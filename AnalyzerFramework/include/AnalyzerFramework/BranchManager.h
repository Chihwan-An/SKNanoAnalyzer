#ifndef BRANCHMANAGER_H
#define BRANCHMANAGER_H

#include <cstddef>
#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "AnalysisException.h"
#include "ColumnSource.h"
#include <AnalyzerFramework/RNTupleSource.h>
#include "RtypesCore.h"

enum class ColumnRequirement { Required, Optional };

class BranchManager;
template <typename T> class ColumnHandle;
template <typename T> class ScalarHandle;

class BranchBase {
public:
    explicit BranchBase(const char *name) : branchName(name) {}
    virtual ~BranchBase() = default;

    virtual void attachRNTuple(SKNano::RNTupleSource *source) {
        clearBinding();
        rntupleSource = source;
        const bool wasActive = active;
        lastEntry = -1;
        fieldAvailable = source && source->hasField(branchName);
        needsRebind = fieldAvailable;
        active = wasActive && fieldAvailable;
        if (active)
            bindRNTuple();
    }

    void setActivationCallback(std::function<void(const std::string &)> callback) {
        activationCallback = std::move(callback);
    }
    void bindActiveBranchSet(std::unordered_set<std::string> *names) {
        activeBranchNames = names;
    }
    void bindEntrySource(const Long64_t *ptr) { entrySource = ptr; }
    void bindEpochSource(const std::uint64_t *ptr) { epochSource = ptr; }

    virtual void reset() { lastEntry = -1; }
    const std::string &name() const noexcept { return branchName; }
    virtual const std::type_info &wrapperType() const = 0;

protected:
    virtual void bindRNTuple() const = 0;
    virtual void clearBinding() const = 0;

    void requireAvailable() const {
        if (fieldAvailable)
            return;
        if (rntupleSource)
            throw SKNano::ConfigError(
                "[BranchManager] missing RNTuple field '" + branchName + "'");
        throw SKNano::ConfigError(
            "[BranchManager] field '" + branchName +
            "' accessed before an RNTuple was attached");
    }

    Long64_t resolveEntry(Long64_t entry) const {
        return entry >= 0 ? entry : (entrySource ? *entrySource : -1);
    }

    std::uint64_t currentEpoch() const noexcept {
        return epochSource ? *epochSource : 0;
    }

    void activate() const {
        if (active)
            return;
        std::cout << "[BranchManager] activating RNTuple field '" << branchName
                  << "'" << std::endl;
        bindRNTuple();
        active = true;
        needsRebind = false;
        if (activeBranchNames)
            activeBranchNames->insert(branchName);
        if (activationCallback)
            activationCallback(branchName);
    }

    std::string branchName;
    SKNano::RNTupleSource *rntupleSource = nullptr;
    bool fieldAvailable = false;
    mutable bool active = false;
    mutable bool needsRebind = false;
    mutable Long64_t lastEntry = -1;
    const Long64_t *entrySource = nullptr;
    const std::uint64_t *epochSource = nullptr;
    std::function<void(const std::string &)> activationCallback;
    std::unordered_set<std::string> *activeBranchNames = nullptr;
};

template <typename T>
class BranchScalar : public BranchBase {
public:
    explicit BranchScalar(const char *name) : BranchBase(name) {}

    const T &get(Long64_t entry = -1) const {
        ensure(entry);
        return value;
    }
    operator T() const { return get(); }
    T &mutableValue() const { return value; }
    bool valid() const noexcept { return fieldAvailable; }

    const std::type_info &wrapperType() const override {
        return typeid(BranchScalar<T>);
    }

    void ensure(Long64_t entry = -1) const {
        requireAvailable();
        if (needsRebind)
            bindRNTuple();
        activate();
        const Long64_t target = resolveEntry(entry);
        if (target < 0 || lastEntry == target)
            return;
        rntupleColumn->read(static_cast<std::uint64_t>(target), &value);
        lastEntry = target;
    }

protected:
    void bindRNTuple() const override {
        if (!rntupleSource || !fieldAvailable)
            return;
        rntupleColumn = rntupleSource->makeScalarColumn(
            branchName, std::type_index(typeid(T)));
        needsRebind = false;
    }

    void clearBinding() const override { rntupleColumn.reset(); }

private:
    mutable T value{};
    mutable std::shared_ptr<SKNano::RNTupleScalarColumn> rntupleColumn;
};

template <typename T, typename CountT>
class BranchVector : public BranchBase, public SKNano::ColumnSource<T> {
public:
    explicit BranchVector(const char *name) : BranchBase(name) {}
    BranchVector(const char *name, BranchScalar<CountT> &)
        : BranchBase(name) {}

    const std::vector<T> &values(Long64_t entry = -1) const {
        ensure(entry);
        if (rntupleSize == 0)
            compatibilityBuffer.clear();
        else
            compatibilityBuffer.assign(rntupleData, rntupleData + rntupleSize);
        return compatibilityBuffer;
    }

    std::vector<T> &mutableValues(Long64_t entry = -1) const {
        static_cast<void>(values(entry));
        return compatibilityBuffer;
    }

    const T &operator[](std::size_t index) const {
        ensure();
        if (index >= rntupleSize)
            throw std::out_of_range("[BranchVector] column index out of range");
        return rntupleData[index];
    }

    T get(std::size_t index) const override { return (*this)[index]; }
    std::size_t size(Long64_t entry) const {
        ensure(entry);
        return rntupleSize;
    }
    std::size_t size() const override { return size(-1); }
    bool empty(Long64_t entry = -1) const { return size(entry) == 0; }
    bool valid() const override { return fieldAvailable; }
    std::uint64_t epoch() const override { return currentEpoch(); }

    SKNano::ContiguousView<T> snapshot() const override {
        ensure();
        return SKNano::ContiguousView<T>(this, rntupleData, rntupleSize,
                                         currentEpoch());
    }

    const std::type_info &wrapperType() const override {
        return typeid(BranchVector<T, CountT>);
    }

    void ensure(Long64_t entry = -1) const {
        requireAvailable();
        if (needsRebind)
            bindRNTuple();
        activate();
        const Long64_t target = resolveEntry(entry);
        if (target < 0 || lastEntry == target)
            return;
        const auto span = rntupleColumn->read(static_cast<std::uint64_t>(target));
        rntupleData = static_cast<const T *>(span.data);
        rntupleSize = span.size;
        lastEntry = target;
    }

protected:
    void bindRNTuple() const override {
        if (!rntupleSource || !fieldAvailable)
            return;
        rntupleColumn = rntupleSource->makeVectorColumn(
            branchName, std::type_index(typeid(T)));
        rntupleData = nullptr;
        rntupleSize = 0;
        needsRebind = false;
    }

    void clearBinding() const override {
        rntupleColumn.reset();
        rntupleData = nullptr;
        rntupleSize = 0;
    }

private:
    mutable std::shared_ptr<SKNano::RNTupleVectorColumn> rntupleColumn;
    mutable const T *rntupleData = nullptr;
    mutable std::size_t rntupleSize = 0;
    mutable std::vector<T> compatibilityBuffer;
};

template <typename CountT>
class BranchVector<bool, CountT> : public BranchBase,
                                  public SKNano::ColumnSource<bool> {
public:
    explicit BranchVector(const char *name) : BranchBase(name) {}
    BranchVector(const char *name, BranchScalar<CountT> &)
        : BranchBase(name) {}

    bool operator[](std::size_t index) const {
        ensure();
        if (index >= rntupleSize)
            throw std::out_of_range(
                "[BranchVector] bool column index out of range");
        return rntupleData[index] != 0;
    }

    bool get(std::size_t index) const override { return (*this)[index]; }
    std::size_t size(Long64_t entry) const {
        ensure(entry);
        return rntupleSize;
    }
    std::size_t size() const override { return size(-1); }
    bool empty(Long64_t entry = -1) const { return size(entry) == 0; }
    bool valid() const override { return fieldAvailable; }
    std::uint64_t epoch() const override { return currentEpoch(); }

    SKNano::ByteContiguousView snapshot() const override {
        ensure();
        return SKNano::ByteContiguousView(this, rntupleData, rntupleSize,
                                          currentEpoch());
    }

    const std::type_info &wrapperType() const override {
        return typeid(BranchVector<bool, CountT>);
    }

    void ensure(Long64_t entry = -1) const {
        requireAvailable();
        if (needsRebind)
            bindRNTuple();
        activate();
        const Long64_t target = resolveEntry(entry);
        if (target < 0 || lastEntry == target)
            return;
        const auto span = rntupleColumn->read(static_cast<std::uint64_t>(target));
        rntupleData = static_cast<const std::uint8_t *>(span.data);
        rntupleSize = span.size;
        lastEntry = target;
    }

protected:
    void bindRNTuple() const override {
        if (!rntupleSource || !fieldAvailable)
            return;
        rntupleColumn = rntupleSource->makeVectorColumn(
            branchName, std::type_index(typeid(bool)));
        rntupleData = nullptr;
        rntupleSize = 0;
        needsRebind = false;
    }

    void clearBinding() const override {
        rntupleColumn.reset();
        rntupleData = nullptr;
        rntupleSize = 0;
    }

private:
    mutable std::shared_ptr<SKNano::RNTupleVectorColumn> rntupleColumn;
    mutable const std::uint8_t *rntupleData = nullptr;
    mutable std::size_t rntupleSize = 0;
};

class BranchManager {
public:
    BranchManager() = default;
    BranchManager(const BranchManager &) = delete;
    BranchManager &operator=(const BranchManager &) = delete;
    BranchManager(BranchManager &&) = delete;
    BranchManager &operator=(BranchManager &&) = delete;

    void clear() {
        for (auto *branch : branches)
            branch->attachRNTuple(nullptr);
        branches.clear();
        branchesByName.clear();
        ownedBranches.clear();
        requests.clear();
        activeBranchNames.clear();
        rntupleSource = nullptr;
        ++sourceGeneration;
    }

    // Bumped whenever the resolved column layout can change (new file attached,
    // manager cleared).  Typed handles cache their resolved source per
    // generation instead of re-resolving the field name on every access.
    std::uint64_t generation() const noexcept { return sourceGeneration; }

    void bindEntrySource(const Long64_t *ptr) {
        entrySource = ptr;
        for (auto *branch : branches)
            branch->bindEntrySource(ptr);
    }

    void bindEpochSource(const std::uint64_t *ptr) {
        epochSource = ptr;
        for (auto *branch : branches)
            branch->bindEpochSource(ptr);
    }

    void setActivationCallback(std::function<void(const std::string &)> callback) {
        activationCallback = std::move(callback);
        for (auto *branch : branches)
            branch->setActivationCallback(activationCallback);
    }

    void attachRNTuple(SKNano::RNTupleSource *source) {
        rntupleSource = source;
        ++sourceGeneration;
        for (auto *branch : branches) {
            branch->attachRNTuple(source);
            branch->bindEntrySource(entrySource);
            branch->bindEpochSource(epochSource);
        }
        for (const auto &item : requests)
            validateRequest(item.first, item.second);
    }

    template <typename Branch>
    void registerBranch(Branch &branch) {
        const auto existing = branchesByName.find(branch.name());
        if (existing != branchesByName.end()) {
            if (existing->second == &branch)
                return;
            if (existing->second->wrapperType() != branch.wrapperType())
                throw SKNano::ConfigError(
                    "[BranchManager] conflicting wrappers for RNTuple field '" +
                    branch.name() + "'");
            throw SKNano::ConfigError(
                "[BranchManager] duplicate wrapper registration for RNTuple field '" +
                branch.name() + "'");
        }
        branch.reset();
        branch.bindEntrySource(entrySource);
        branch.bindEpochSource(epochSource);
        branch.setActivationCallback(activationCallback);
        branch.bindActiveBranchSet(&activeBranchNames);
        if (rntupleSource)
            branch.attachRNTuple(rntupleSource);
        branches.push_back(&branch);
        branchesByName.emplace(branch.name(), &branch);
    }

    template <typename T>
    void registerScalar(BranchScalar<T> &branch) { registerBranch(branch); }

    template <typename T, typename CountT>
    void registerVector(BranchVector<T, CountT> &branch) {
        registerBranch(branch);
    }

    template <typename T>
    BranchScalar<T> *findScalar(const std::string &name) const {
        const auto it = branchesByName.find(name);
        return it == branchesByName.end()
                   ? nullptr
                   : dynamic_cast<BranchScalar<T> *>(it->second);
    }

    template <typename T>
    BranchScalar<T> &getOrCreateScalar(const std::string &name) {
        if (auto *existing = findScalar<T>(name))
            return *existing;
        if (branchesByName.find(name) != branchesByName.end())
            throw SKNano::ConfigError(
                "[BranchManager] type conflict while resolving RNTuple field '" +
                name + "'");
        auto owned = std::make_unique<BranchScalar<T>>(name.c_str());
        auto *raw = owned.get();
        ownedBranches.emplace_back(std::move(owned));
        registerScalar(*raw);
        return *raw;
    }

    template <typename T>
    SKNano::ColumnSource<T> &getOrCreateVector(const std::string &name) {
        auto existing = branchesByName.find(name);
        if (existing != branchesByName.end()) {
            auto *source =
                dynamic_cast<SKNano::ColumnSource<T> *>(existing->second);
            if (!source)
                throw SKNano::ConfigError(
                    "[BranchManager] type or cardinality conflict while "
                    "resolving RNTuple vector field '" + name + "'");
            return *source;
        }
        auto owned = std::make_unique<BranchVector<T, Int_t>>(name.c_str());
        auto *raw = owned.get();
        ownedBranches.emplace_back(std::move(owned));
        registerVector(*raw);
        return *raw;
    }

    template <typename T>
    ColumnHandle<T> getColumnHandle(
        const std::string &name,
        ColumnRequirement requirement = ColumnRequirement::Required);

    template <typename T>
    ScalarHandle<T> getScalarHandle(
        const std::string &name,
        ColumnRequirement requirement = ColumnRequirement::Required);

    template <typename T>
    SKNano::ColumnSource<T> *resolveColumn(
        const std::string &name, ColumnRequirement requirement) {
        registerRequest<T>(name, true, requirement);
        if (!available(name)) {
            if (requirement == ColumnRequirement::Required)
                throwMissing(name);
            return nullptr;
        }
        return &getOrCreateVector<T>(name);
    }

    template <typename T>
    BranchScalar<T> *resolveScalar(
        const std::string &name, ColumnRequirement requirement) {
        registerRequest<T>(name, false, requirement);
        if (!available(name)) {
            if (requirement == ColumnRequirement::Required)
                throwMissing(name);
            return nullptr;
        }
        return &getOrCreateScalar<T>(name);
    }

    bool available(const std::string &name) const {
        return rntupleSource && rntupleSource->hasField(name);
    }

    template <typename Branch>
    void reset(Branch &branch) { branch.reset(); }

    void resetAll() {
        for (auto *branch : branches)
            branch->reset();
    }

    const std::unordered_set<std::string> &getActiveBranches() const noexcept {
        return activeBranchNames;
    }

private:
    struct Request {
        std::type_index type;
        bool vector = false;
        ColumnRequirement requirement = ColumnRequirement::Optional;
    };

    std::string context() const {
        return rntupleSource ? rntupleSource->fileName() + ":RNTuple"
                             : "<no RNTuple>";
    }

    [[noreturn]] void throwMissing(const std::string &name) const {
        throw SKNano::ConfigError(
            "[BranchManager] required RNTuple field '" + name +
            "' is missing in " + context());
    }

    void validateRequest(const std::string &name, const Request &request) const {
        if (!rntupleSource)
            return;
        if (!rntupleSource->hasField(name)) {
            if (request.requirement == ColumnRequirement::Required)
                throwMissing(name);
            return;
        }
        auto existing = branchesByName.find(name);
        if (existing == branchesByName.end()) {
            if (request.vector)
                static_cast<void>(const_cast<BranchManager *>(this)
                                      ->getOrCreateVectorForRequest(
                                          name, request.type));
            else
                return;
            existing = branchesByName.find(name);
        }
        if (request.vector) {
            const bool compatible =
                (request.type == std::type_index(typeid(bool)) &&
                 dynamic_cast<SKNano::ColumnSource<bool> *>(existing->second)) ||
                (request.type == std::type_index(typeid(float)) &&
                 dynamic_cast<SKNano::ColumnSource<float> *>(existing->second)) ||
                (request.type == std::type_index(typeid(double)) &&
                 dynamic_cast<SKNano::ColumnSource<double> *>(existing->second)) ||
                (request.type == std::type_index(typeid(int)) &&
                 dynamic_cast<SKNano::ColumnSource<int> *>(existing->second)) ||
                (request.type == std::type_index(typeid(unsigned int)) &&
                 dynamic_cast<SKNano::ColumnSource<unsigned int> *>(existing->second)) ||
                (request.type == std::type_index(typeid(short)) &&
                 dynamic_cast<SKNano::ColumnSource<short> *>(existing->second)) ||
                (request.type == std::type_index(typeid(unsigned short)) &&
                 dynamic_cast<SKNano::ColumnSource<unsigned short> *>(existing->second)) ||
                (request.type == std::type_index(typeid(unsigned char)) &&
                 dynamic_cast<SKNano::ColumnSource<unsigned char> *>(existing->second)) ||
                (request.type == std::type_index(typeid(Long64_t)) &&
                 dynamic_cast<SKNano::ColumnSource<Long64_t> *>(existing->second)) ||
                (request.type == std::type_index(typeid(ULong64_t)) &&
                 dynamic_cast<SKNano::ColumnSource<ULong64_t> *>(existing->second));
            if (!compatible)
                throw SKNano::ConfigError(
                    "[BranchManager] incompatible RNTuple vector request for field '" +
                    name + "' in " + context());
        }
    }

    template <typename T>
    void registerRequest(const std::string &name, bool vector,
                         ColumnRequirement requirement) {
        Request requested{std::type_index(typeid(T)), vector, requirement};
        const auto prior = requests.find(name);
        if (prior != requests.end()) {
            if (prior->second.type != requested.type ||
                prior->second.vector != requested.vector)
                throw SKNano::ConfigError(
                    "[BranchManager] conflicting typed handle request for RNTuple field '" +
                    name + "'");
            if (prior->second.requirement == ColumnRequirement::Required)
                requested.requirement = ColumnRequirement::Required;
        }
        validateRequest(name, requested);
        if (prior == requests.end())
            requests.emplace(name, requested);
        else
            prior->second = requested;
    }

    BranchBase *getOrCreateVectorForRequest(const std::string &name,
                                            std::type_index type) {
#define SKNANO_DYNAMIC_VECTOR(cpp_type)                                      \
        if (type == std::type_index(typeid(cpp_type))) {                     \
            static_cast<void>(getOrCreateVector<cpp_type>(name));            \
            return branchesByName.at(name);                                  \
        }
        SKNANO_DYNAMIC_VECTOR(bool)
        SKNANO_DYNAMIC_VECTOR(float)
        SKNANO_DYNAMIC_VECTOR(double)
        SKNANO_DYNAMIC_VECTOR(int)
        SKNANO_DYNAMIC_VECTOR(unsigned int)
        SKNANO_DYNAMIC_VECTOR(short)
        SKNANO_DYNAMIC_VECTOR(unsigned short)
        SKNANO_DYNAMIC_VECTOR(unsigned char)
        SKNANO_DYNAMIC_VECTOR(Long64_t)
        SKNANO_DYNAMIC_VECTOR(ULong64_t)
#undef SKNANO_DYNAMIC_VECTOR
        throw SKNano::ConfigError(
            "[BranchManager] unsupported dynamic vector type for field '" +
            name + "'");
    }

    SKNano::RNTupleSource *rntupleSource = nullptr;
    const Long64_t *entrySource = nullptr;
    const std::uint64_t *epochSource = nullptr;
    std::function<void(const std::string &)> activationCallback;
    std::vector<BranchBase *> branches;
    std::unordered_map<std::string, BranchBase *> branchesByName;
    std::vector<std::unique_ptr<BranchBase>> ownedBranches;
    std::unordered_map<std::string, Request> requests;
    std::unordered_set<std::string> activeBranchNames;
    std::uint64_t sourceGeneration = 1;
};

template <typename T>
class ColumnHandle {
public:
    bool available() const { return manager && manager->available(branchName); }
    T operator[](std::size_t index) const { return requireSource()->get(index); }
    std::size_t size() const { return requireSource()->size(); }
    bool empty() const { return size() == 0; }
    SKNano::ContiguousView<T> snapshot() const {
        return requireSource()->snapshot();
    }
    const SKNano::ColumnSource<T> *source() const { return requireSource(); }

private:
    friend class BranchManager;
    ColumnHandle(BranchManager *manager_, std::string branchName_,
                 ColumnRequirement requirement_)
        : manager(manager_), branchName(std::move(branchName_)),
          requirement(requirement_) {}

    // Resolving a column by name costs a request validation plus dynamic_cast
    // chains, so the result is cached for as long as the manager keeps the same
    // column layout.  A new file (or a cleared manager) bumps the generation and
    // forces a full re-resolve, which is what keeps optional fields honest when
    // their availability differs between files.
    SKNano::ColumnSource<T> *requireSource() const {
        if (!manager)
            throw SKNano::LogicError("[ColumnHandle] detached handle access");
        const auto currentGeneration = manager->generation();
        if (cachedSource && cachedGeneration == currentGeneration)
            return cachedSource;
        auto *source = manager->resolveColumn<T>(branchName, requirement);
        if (!source)
            throw SKNano::ConfigError(
                "[ColumnHandle] optional RNTuple field '" + branchName +
                "' accessed without checking available()");
        cachedSource = source;
        cachedGeneration = currentGeneration;
        return source;
    }

    BranchManager *manager = nullptr;
    std::string branchName;
    ColumnRequirement requirement = ColumnRequirement::Required;
    mutable SKNano::ColumnSource<T> *cachedSource = nullptr;
    mutable std::uint64_t cachedGeneration = 0;
};

template <typename T>
class ScalarHandle {
public:
    bool available() const { return manager && manager->available(branchName); }
    const T &get() const { return requireSource()->get(); }
    operator T() const { return get(); }
    const BranchScalar<T> *source() const { return requireSource(); }

private:
    friend class BranchManager;
    ScalarHandle(BranchManager *manager_, std::string branchName_,
                 ColumnRequirement requirement_)
        : manager(manager_), branchName(std::move(branchName_)),
          requirement(requirement_) {}

    BranchScalar<T> *requireSource() const {
        if (!manager)
            throw SKNano::LogicError("[ScalarHandle] detached handle access");
        const auto currentGeneration = manager->generation();
        if (cachedSource && cachedGeneration == currentGeneration)
            return cachedSource;
        auto *source = manager->resolveScalar<T>(branchName, requirement);
        if (!source)
            throw SKNano::ConfigError(
                "[ScalarHandle] optional RNTuple field '" + branchName +
                "' accessed without checking available()");
        cachedSource = source;
        cachedGeneration = currentGeneration;
        return source;
    }

    BranchManager *manager = nullptr;
    std::string branchName;
    ColumnRequirement requirement = ColumnRequirement::Required;
    mutable BranchScalar<T> *cachedSource = nullptr;
    mutable std::uint64_t cachedGeneration = 0;
};

template <typename T>
ColumnHandle<T> BranchManager::getColumnHandle(
    const std::string &name, ColumnRequirement requirement) {
    registerRequest<T>(name, true, requirement);
    if (requirement == ColumnRequirement::Required || available(name))
        static_cast<void>(resolveColumn<T>(name, requirement));
    return ColumnHandle<T>(this, name, requirement);
}

template <typename T>
ScalarHandle<T> BranchManager::getScalarHandle(
    const std::string &name, ColumnRequirement requirement) {
    registerRequest<T>(name, false, requirement);
    if (requirement == ColumnRequirement::Required || available(name))
        static_cast<void>(resolveScalar<T>(name, requirement));
    return ScalarHandle<T>(this, name, requirement);
}

#endif // BRANCHMANAGER_H
