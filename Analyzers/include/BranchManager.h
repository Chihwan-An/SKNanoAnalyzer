#ifndef BRANCHMANAGER_H
#define BRANCHMANAGER_H

#include <cstddef>
#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <typeinfo>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include <unordered_set>

#include "AnalysisException.h"
#include "ColumnSource.h"
#include "RNTupleSource.h"
#include "TBranch.h"
#include "TFile.h"
#include "TLeaf.h"
#include "TTree.h"

enum class ColumnRequirement { Required, Optional };

class BranchManager;
template <typename T> class ColumnHandle;
template <typename T> class ScalarHandle;

template <typename T>
const char *ExpectedRootLeafType() {
    if constexpr (std::is_same_v<T, float>)
        return "Float_t";
    else if constexpr (std::is_same_v<T, double>)
        return "Double_t";
    else if constexpr (std::is_same_v<T, int>)
        return "Int_t";
    else if constexpr (std::is_same_v<T, unsigned int>)
        return "UInt_t";
    else if constexpr (std::is_same_v<T, bool>)
        return "Bool_t";
    else if constexpr (std::is_same_v<T, short>)
        return "Short_t";
    else if constexpr (std::is_same_v<T, unsigned short>)
        return "UShort_t";
    else if constexpr (std::is_same_v<T, unsigned char>)
        return "UChar_t";
    else if constexpr (std::is_same_v<T, Long64_t>)
        return "Long64_t";
    else if constexpr (std::is_same_v<T, ULong64_t>)
        return "ULong64_t";
    else
        return nullptr;
}

class BranchBase {
public:
    explicit BranchBase(const char *name) : branchName(name) {}
    virtual ~BranchBase() = default;

    virtual void attach(TTree *tree_) {
        clearBackendBinding();
        rntupleSource = nullptr;
        rntupleFieldAvailable = false;
        tree = tree_;
        branch = tree ? tree->GetBranch(branchName.c_str()) : nullptr;
        const bool wasActive = active; // remember whether this branch had been turned on
        lastEntry = -1;
        needsRebind = branch != nullptr;
        active = wasActive && branch;
        if (tree && branch) {
            validateSchema();
            tree->SetBranchStatus(branchName.c_str(), wasActive ? 1 : 0);
            if (active)
                bindAddress();
        }
    }

    virtual void attachRNTuple(SKNano::RNTupleSource *source) {
        clearBackendBinding();
        tree = nullptr;
        branch = nullptr;
        rntupleSource = source;
        const bool wasActive = active;
        lastEntry = -1;
        rntupleFieldAvailable = source && source->hasField(branchName);
        needsRebind = rntupleFieldAvailable;
        active = wasActive && rntupleFieldAvailable;
        if (rntupleFieldAvailable) {
            validateRNTupleSchema();
            if (active)
                bindRNTuple();
        }
    }

    void setActivationCallback(std::function<void(const std::string &)> callback) {
        activationCallback = std::move(callback);
    }
    void bindActiveBranchSet(std::unordered_set<std::string> *names) {
        activeBranchNames = names;
    }

    virtual void reset() { lastEntry = -1; }

    const std::string &name() const { return branchName; }
    virtual const std::type_info &wrapperType() const = 0;

    void bindEntrySource(const Long64_t *ptr) { entrySource = ptr; }
    void bindEpochSource(const std::uint64_t *ptr) { epochSource = ptr; }
    std::uint64_t currentEpoch() const { return epochSource ? *epochSource : 0; }

protected:
    virtual void bindAddress() const {}
    virtual void bindRNTuple() const {}
    virtual void clearBackendBinding() const {}
    virtual void validateSchema() const {}
    virtual void validateRNTupleSchema() const {}

    void requireLeafType(const TLeaf *leaf, const char *expected,
                         const char *kind) const {
        if (!leaf || !expected || std::string(leaf->GetTypeName()) != expected) {
            const std::string actual = leaf ? leaf->GetTypeName() : "<missing leaf>";
            throw SKNano::ConfigError("[BranchManager] " + std::string(kind) +
                                      " branch '" + branchName + "' has type " +
                                      actual + ", expected " +
                                      (expected ? expected : "<unsupported C++ type>"));
        }
    }

    void requireAvailable() const {
        if (branch || rntupleFieldAvailable)
            return;
        if (tree || rntupleSource)
            throw SKNano::ConfigError("[BranchManager] Missing NanoAOD branch '" +
                                      branchName + "' in current tree");
        throw SKNano::ConfigError("[BranchManager] Branch '" + branchName +
                                  "' accessed before a tree was attached");
    }

    bool usesRNTuple() const noexcept {
        return rntupleSource && rntupleFieldAvailable;
    }

    Long64_t resolveEntry(Long64_t entry) const {
        if (entry >= 0)
            return entry;
        return entrySource ? *entrySource : -1;
    }

    void recordActivation() const {
        if (activeBranchNames)
            activeBranchNames->insert(branchName);
        if (activationCallback)
            activationCallback(branchName);
    }

    std::string branchName;
    TTree *tree = nullptr;
    TBranch *branch = nullptr;
    SKNano::RNTupleSource *rntupleSource = nullptr;
    bool rntupleFieldAvailable = false;
    mutable bool active = false;
    mutable Long64_t lastEntry = -1;
    const Long64_t *entrySource = nullptr;
    const std::uint64_t *epochSource = nullptr;
    mutable bool needsRebind = false;
    std::function<void(const std::string &)> activationCallback;
    std::unordered_set<std::string> *activeBranchNames = nullptr;
};

template <typename T>
class BranchScalar : public BranchBase {
public:
    explicit BranchScalar(const char *name) : BranchBase(name) {}

    void bindAddress() const override {
        if (branch)
        {
            branch->SetAddress(&value);
            needsRebind = false;
        }
    }

    void bindRNTuple() const override {
        if (!rntupleSource || !rntupleFieldAvailable)
            return;
        rntupleColumn = rntupleSource->makeScalarColumn(
            branchName, std::type_index(typeid(T)));
        needsRebind = false;
    }

    void clearBackendBinding() const override { rntupleColumn.reset(); }

    void validateSchema() const override {
        TLeaf *leaf = branch->GetLeaf(branchName.c_str());
        requireLeafType(leaf, ExpectedRootLeafType<T>(), "scalar");
        if (leaf->GetLeafCount() || leaf->GetLenStatic() != 1) {
            throw SKNano::ConfigError("[BranchManager] branch '" + branchName +
                                      "' is not scalar");
        }
    }

    const T &get(Long64_t entry = -1) const {
        ensure(entry);
        return value;
    }

    operator T() const { return get(); }

    T &mutableValue() const { return value; }

    bool valid() const { return branch != nullptr || rntupleFieldAvailable; }
    const std::type_info &wrapperType() const override {
        return typeid(BranchScalar<T>);
    }

    void ensure(Long64_t entry = -1) const {
        requireAvailable();
        if (needsRebind) {
            if (usesRNTuple())
                bindRNTuple();
            else
                bindAddress();
        }
        const Long64_t target = resolveEntry(entry);
        if (target < 0)
            return;
        if (!active) {
            std::cout << "[BranchManager] activating branch '" << branchName
                      << "'" << std::endl;
            if (usesRNTuple())
                bindRNTuple();
            else {
                tree->SetBranchStatus(branchName.c_str(), 1);
                branch->SetAddress(&value);
            }
            active = true;
            needsRebind = false;
            recordActivation();
        }
        if (lastEntry == target)
            return;
        if (usesRNTuple())
            rntupleColumn->read(static_cast<std::uint64_t>(target), &value);
        else
            branch->GetEntry(target);
        lastEntry = target;
    }

private:
    mutable T value{};
    mutable std::shared_ptr<SKNano::RNTupleScalarColumn> rntupleColumn;
};

template <typename T, typename CountT>
class BranchVector : public BranchBase, public SKNano::ColumnSource<T> {
public:
    BranchVector(const char *name, BranchScalar<CountT> &count)
        : BranchBase(name), countBranch(count) {}

    void bindAddress() const override {
        if (!branch)
            return;
        boundAddress = nullptr;
        if (capacity > 0 && !buffer.empty())
            setAddress(buffer.data());
        else
            setAddress(&zeroValue);
        needsRebind = false;
    }

    void bindRNTuple() const override {
        if (!rntupleSource || !rntupleFieldAvailable)
            return;
        rntupleColumn = rntupleSource->makeVectorColumn(
            branchName, std::type_index(typeid(T)));
        rntupleData = nullptr;
        rntupleSize = 0;
        needsRebind = false;
    }

    void clearBackendBinding() const override {
        rntupleColumn.reset();
        rntupleData = nullptr;
        rntupleSize = 0;
    }

    void validateSchema() const override {
        TLeaf *leaf = branch->GetLeaf(branchName.c_str());
        requireLeafType(leaf, ExpectedRootLeafType<T>(), "vector");
        TLeaf *countLeaf = leaf ? leaf->GetLeafCount() : nullptr;
        if (!countLeaf || countLeaf->GetName() != countBranch.name()) {
            throw SKNano::ConfigError("[BranchManager] vector branch '" + branchName +
                                      "' has incompatible count branch");
        }
        requireLeafType(countLeaf, ExpectedRootLeafType<CountT>(), "vector count");
    }

    const std::vector<T> &values(Long64_t entry = -1) const {
        ensure(entry);
        if (usesRNTuple()) {
            if (rntupleSize == 0)
                compatibilityBuffer.clear();
            else
                compatibilityBuffer.assign(rntupleData,
                                           rntupleData + rntupleSize);
            return compatibilityBuffer;
        }
        return buffer;
    }

    std::vector<T> &mutableValues(Long64_t entry = -1) const {
        ensure(entry);
        if (usesRNTuple()) {
            if (rntupleSize == 0)
                compatibilityBuffer.clear();
            else
                compatibilityBuffer.assign(rntupleData,
                                           rntupleData + rntupleSize);
            return compatibilityBuffer;
        }
        return buffer;
    }

    const T &operator[](std::size_t idx) const {
        ensure();
        if (usesRNTuple()) {
            if (idx >= rntupleSize)
                throw std::out_of_range("[BranchVector] column index out of range");
            return rntupleData[idx];
        }
        return buffer.at(idx);
    }

    T get(std::size_t idx) const override { return (*this)[idx]; }

    std::size_t size(Long64_t entry) const {
        ensure(entry);
        return usesRNTuple() ? rntupleSize : buffer.size();
    }

    std::size_t size() const override { return size(-1); }

    bool empty(Long64_t entry = -1) const {
        return size(entry) == 0;
    }

    bool valid() const override { return branch != nullptr || rntupleFieldAvailable; }
    std::uint64_t epoch() const override { return currentEpoch(); }
    SKNano::ContiguousView<T> snapshot() const override {
        ensure();
        const T *data = usesRNTuple() ? rntupleData : buffer.data();
        const std::size_t count = usesRNTuple() ? rntupleSize : buffer.size();
        return SKNano::ContiguousView<T>(this, data, count,
                                         currentEpoch());
    }
    const std::type_info &wrapperType() const override {
        return typeid(BranchVector<T, CountT>);
    }

    void ensure(Long64_t entry = -1) const {
        requireAvailable();
        if (needsRebind) {
            if (usesRNTuple())
                bindRNTuple();
            else
                bindAddress();
        }
        if (!active) {
            std::cout << "[BranchManager] activating branch '" << branchName
                      << "'" << std::endl;
            if (usesRNTuple())
                bindRNTuple();
            else
                tree->SetBranchStatus(branchName.c_str(), 1);
            active = true;
            capacity = 0;
            recordActivation();
        }

        const Long64_t target = resolveEntry(entry);
        if (target < 0)
            return;

        if (usesRNTuple()) {
            if (lastEntry == target)
                return;
            const auto span = rntupleColumn->read(
                static_cast<std::uint64_t>(target));
            rntupleData = static_cast<const T *>(span.data);
            rntupleSize = span.size;
            lastEntry = target;
            return;
        }

        countBranch.ensure(target);
        auto rawSize = countBranch.mutableValue();
        if (rawSize < 0)
            rawSize = 0;
        std::size_t needed = static_cast<std::size_t>(rawSize);

        if (needed == 0)
        {
            buffer.clear();
            setAddress(&zeroValue);
            capacity = 0;
            if (lastEntry != target)
            {
                branch->GetEntry(target);
                lastEntry = target;
            }
            return;
        }

        if (buffer.capacity() < needed)
            buffer.reserve(needed);
        buffer.resize(needed);
        setAddress(buffer.data());
        capacity = buffer.capacity();

        if (lastEntry == target)
            return;
        branch->GetEntry(target);
        lastEntry = target;
    }

private:
    BranchScalar<CountT> &countBranch;
    mutable std::vector<T> buffer;
    mutable std::size_t capacity = 0;
    mutable T zeroValue{};
    mutable void *boundAddress = nullptr;
    mutable std::shared_ptr<SKNano::RNTupleVectorColumn> rntupleColumn;
    mutable const T *rntupleData = nullptr;
    mutable std::size_t rntupleSize = 0;
    mutable std::vector<T> compatibilityBuffer;

    void setAddress(void *address) const {
        if (boundAddress == address)
            return;
        branch->SetAddress(address);
        boundAddress = address;
    }
};

template <typename CountT>
class BranchVector<bool, CountT> : public BranchBase, public SKNano::ColumnSource<bool> {
public:
    BranchVector(const char *name, BranchScalar<CountT> &count)
        : BranchBase(name), countBranch(count) {}

    void bindAddress() const override {
        if (!branch)
            return;
        boundAddress = nullptr;
        if (capacity > 0 && !storage.empty())
            setAddress(storage.data());
        else
            setAddress(&zeroValue);
        needsRebind = false;
    }

    void bindRNTuple() const override {
        if (!rntupleSource || !rntupleFieldAvailable)
            return;
        rntupleColumn = rntupleSource->makeVectorColumn(
            branchName, std::type_index(typeid(bool)));
        rntupleData = nullptr;
        rntupleSize = 0;
        needsRebind = false;
    }

    void clearBackendBinding() const override {
        rntupleColumn.reset();
        rntupleData = nullptr;
        rntupleSize = 0;
    }

    void validateSchema() const override {
        TLeaf *leaf = branch->GetLeaf(branchName.c_str());
        requireLeafType(leaf, ExpectedRootLeafType<bool>(), "vector");
        TLeaf *countLeaf = leaf ? leaf->GetLeafCount() : nullptr;
        if (!countLeaf || countLeaf->GetName() != countBranch.name()) {
            throw SKNano::ConfigError("[BranchManager] vector branch '" + branchName +
                                      "' has incompatible count branch");
        }
        requireLeafType(countLeaf, ExpectedRootLeafType<CountT>(), "vector count");
    }

    bool operator[](std::size_t idx) const {
        ensure();
        if (usesRNTuple()) {
            if (idx >= rntupleSize)
                throw std::out_of_range("[BranchVector] bool column index out of range");
            return rntupleData[idx] != 0;
        }
        return static_cast<bool>(storage.at(idx));
    }

    bool get(std::size_t idx) const override { return (*this)[idx]; }

    std::size_t size(Long64_t entry) const {
        ensure(entry);
        return usesRNTuple() ? rntupleSize : storage.size();
    }

    std::size_t size() const override { return size(-1); }

    bool empty(Long64_t entry = -1) const {
        return size(entry) == 0;
    }

    bool valid() const override { return branch != nullptr || rntupleFieldAvailable; }
    std::uint64_t epoch() const override { return currentEpoch(); }
    SKNano::ByteContiguousView snapshot() const override {
        ensure();
        const auto *data = usesRNTuple() ? rntupleData : storage.data();
        const std::size_t count = usesRNTuple() ? rntupleSize : storage.size();
        return SKNano::ByteContiguousView(this, data, count,
                                          currentEpoch());
    }
    const std::type_info &wrapperType() const override {
        return typeid(BranchVector<bool, CountT>);
    }

    void ensure(Long64_t entry = -1) const {
        requireAvailable();
        if (needsRebind) {
            if (usesRNTuple())
                bindRNTuple();
            else
                bindAddress();
        }
        if (!active) {
            if (usesRNTuple())
                bindRNTuple();
            else
                tree->SetBranchStatus(branchName.c_str(), 1);
            std::cout << "[BranchManager] activating branch '" << branchName << "'" << std::endl;
            active = true;
            capacity = 0;
            recordActivation();
        }

        const Long64_t target = resolveEntry(entry);
        if (target < 0)
            return;

        if (usesRNTuple()) {
            if (lastEntry == target)
                return;
            const auto span = rntupleColumn->read(
                static_cast<std::uint64_t>(target));
            rntupleData = static_cast<const std::uint8_t *>(span.data);
            rntupleSize = span.size;
            lastEntry = target;
            return;
        }

        countBranch.ensure(target);
        auto rawSize = countBranch.mutableValue();
        if (rawSize < 0)
            rawSize = 0;
        std::size_t needed = static_cast<std::size_t>(rawSize);

        if (needed == 0)
        {
            storage.clear();
            setAddress(&zeroValue);
            capacity = 0;
            if (lastEntry != target)
            {
                branch->GetEntry(target);
                lastEntry = target;
            }
            return;
        }

        if (storage.capacity() < needed)
            storage.reserve(needed);
        storage.resize(needed);
        setAddress(storage.data());
        capacity = storage.capacity();

        if (lastEntry == target)
            return;
        branch->GetEntry(target);
        lastEntry = target;
    }

private:
    BranchScalar<CountT> &countBranch;
    mutable std::vector<unsigned char> storage;
    mutable std::size_t capacity = 0;
    mutable unsigned char zeroValue = 0;
    mutable void *boundAddress = nullptr;
    mutable std::shared_ptr<SKNano::RNTupleVectorColumn> rntupleColumn;
    mutable const std::uint8_t *rntupleData = nullptr;
    mutable std::size_t rntupleSize = 0;

    void setAddress(void *address) const {
        if (boundAddress == address)
            return;
        branch->SetAddress(address);
        boundAddress = address;
    }
};

class BranchManager {
public:
    BranchManager() = default;
    BranchManager(const BranchManager &) = delete;
    BranchManager &operator=(const BranchManager &) = delete;
    BranchManager(BranchManager &&) = delete;
    BranchManager &operator=(BranchManager &&) = delete;

    void clear() {
        branches.clear();
        branchesByName.clear();
        ownedBranches.clear();
        requests.clear();
        activeBranchNames.clear();
        tree = nullptr;
        rntupleSource = nullptr;
    }

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

    void attachTree(TTree *t) {
        tree = t;
        rntupleSource = nullptr;
        for (auto *branch : branches) {
            branch->attach(tree);
            branch->bindEntrySource(entrySource);
            branch->bindEpochSource(epochSource);
        }
        for (const auto &item : requests)
            validateRequest(item.first, item.second);
    }

    void attachRNTuple(SKNano::RNTupleSource *source) {
        tree = nullptr;
        rntupleSource = source;
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
            if (existing->second->wrapperType() != branch.wrapperType()) {
                throw SKNano::ConfigError(
                    "[BranchManager] Conflicting wrappers for branch '" +
                    branch.name() + "'");
            }
            throw SKNano::ConfigError(
                "[BranchManager] Duplicate wrapper registration for branch '" +
                branch.name() + "'");
        }
        branch.reset();
        branch.bindEntrySource(entrySource);
        branch.bindEpochSource(epochSource);
        branch.setActivationCallback(activationCallback);
        branch.bindActiveBranchSet(&activeBranchNames);
        if (tree)
            branch.attach(tree);
        else if (rntupleSource)
            branch.attachRNTuple(rntupleSource);
        branches.push_back(&branch);
        branchesByName.emplace(branch.name(), &branch);
    }

    template <typename T>
    void registerScalar(BranchScalar<T> &branch) { registerBranch(branch); }

    template <typename T>
    BranchScalar<T> *findScalar(const std::string &name) const {
        const auto it = branchesByName.find(name);
        if (it == branchesByName.end())
            return nullptr;
        return dynamic_cast<BranchScalar<T> *>(it->second);
    }

    template <typename T>
    BranchScalar<T> &getOrCreateScalar(const std::string &name) {
        if (auto *existing = findScalar<T>(name))
            return *existing;
        if (branchesByName.find(name) != branchesByName.end()) {
            throw SKNano::ConfigError(
                "[BranchManager] Type conflict while resolving branch '" + name +
                "'");
        }
        auto owned = std::make_unique<BranchScalar<T>>(name.c_str());
        auto *raw = owned.get();
        ownedBranches.emplace_back(std::move(owned));
        registerScalar(*raw);
        return *raw;
    }

    template <typename T, typename CountT>
    void registerVector(BranchVector<T, CountT> &branch) { registerBranch(branch); }

    template <typename CountT>
    void registerVector(BranchVector<bool, CountT> &branch) { registerBranch(branch); }

    template <typename T>
    ColumnHandle<T> getColumnHandle(
        const std::string &name,
        ColumnRequirement requirement = ColumnRequirement::Required);

    template <typename T>
    ScalarHandle<T> getScalarHandle(
        const std::string &name,
        ColumnRequirement requirement = ColumnRequirement::Required);

    template <typename T>
    SKNano::ColumnSource<T> *resolveColumn(const std::string &name,
                                           ColumnRequirement requirement) {
        registerRequest<T>(name, true, requirement);
        if (!available(name)) {
            if (requirement == ColumnRequirement::Required)
                throwMissing(name);
            return nullptr;
        }

        const auto existing = branchesByName.find(name);
        if (existing != branchesByName.end()) {
            auto *source = dynamic_cast<SKNano::ColumnSource<T> *>(existing->second);
            if (!source) {
                throw SKNano::ConfigError(
                    "[BranchManager] Type or cardinality conflict while resolving vector branch '" +
                    name + "'");
            }
            return source;
        }

        if (rntupleSource) {
            throw SKNano::ConfigError(
                "[BranchManager] dynamic RNTuple vector field '" + name +
                "' must be present in the generated input schema");
        }
        TLeaf *leaf = requirePhysicalLeaf(name);
        TLeaf *countLeaf = leaf->GetLeafCount();
        if (!countLeaf) {
            throw SKNano::ConfigError("[BranchManager] branch '" + name +
                                      "' is not a variable-length vector");
        }
        const std::string countName = countLeaf->GetName();
        const std::string countType = countLeaf->GetTypeName();
        if (countType == "Int_t")
            return createVector<T, int>(name, countName);
        if (countType == "UInt_t")
            return createVector<T, unsigned int>(name, countName);
        if (countType == "Short_t")
            return createVector<T, short>(name, countName);
        if (countType == "UShort_t")
            return createVector<T, unsigned short>(name, countName);
        if (countType == "UChar_t")
            return createVector<T, unsigned char>(name, countName);
        if (countType == "Long64_t")
            return createVector<T, Long64_t>(name, countName);
        if (countType == "ULong64_t")
            return createVector<T, ULong64_t>(name, countName);
        throw SKNano::ConfigError(
            "[BranchManager] vector branch '" + name +
            "' has unsupported count leaf type " + countType);
    }

    template <typename T>
    BranchScalar<T> *resolveScalar(const std::string &name,
                                   ColumnRequirement requirement) {
        registerRequest<T>(name, false, requirement);
        if (!available(name)) {
            if (requirement == ColumnRequirement::Required)
                throwMissing(name);
            return nullptr;
        }
        return &getOrCreateScalar<T>(name);
    }

    bool available(const std::string &name) const {
        return (tree && tree->GetBranch(name.c_str())) ||
               (rntupleSource && rntupleSource->hasField(name));
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
        std::string leafType;
        bool vector = false;
        ColumnRequirement requirement = ColumnRequirement::Optional;
    };

    std::string treeContext() const {
        if (rntupleSource)
            return rntupleSource->fileName() + ":RNTuple";
        if (!tree)
            return "<no tree>";
        const TFile *file = tree->GetCurrentFile();
        return std::string(file ? file->GetName() : "<memory>") + ":" +
               tree->GetName();
    }

    [[noreturn]] void throwMissing(const std::string &name) const {
        throw SKNano::ConfigError("[BranchManager] required branch '" + name +
                                  "' is missing in " + treeContext());
    }

    TLeaf *requirePhysicalLeaf(const std::string &name) const {
        TBranch *physical = tree ? tree->GetBranch(name.c_str()) : nullptr;
        TLeaf *leaf = physical ? physical->GetLeaf(name.c_str()) : nullptr;
        if (!leaf)
            throw SKNano::ConfigError("[BranchManager] branch '" + name +
                                      "' has no matching leaf in " + treeContext());
        return leaf;
    }

    void validateRequest(const std::string &name, const Request &request) const {
        if (!tree && !rntupleSource)
            return;
        if (rntupleSource) {
            if (!rntupleSource->hasField(name)) {
                if (request.requirement == ColumnRequirement::Required)
                    throwMissing(name);
                return;
            }
            try {
                if (request.vector) {
                    const auto existing = branchesByName.find(name);
                    if (existing == branchesByName.end())
                        throw SKNano::ConfigError(
                            "RNTuple vector is absent from generated schema");
                    // The concrete wrapper validates its element type when it
                    // creates the lazy RNTuple view on first access.
                } else {
                    const auto existing = branchesByName.find(name);
                    if (existing == branchesByName.end())
                        throw SKNano::ConfigError(
                            "RNTuple scalar is absent from generated schema");
                }
            } catch (const std::exception &error) {
                throw SKNano::ConfigError(
                    "[BranchManager] incompatible RNTuple request for field '" +
                    name + "' in " + treeContext() + ": " + error.what());
            }
            return;
        }
        TBranch *physical = tree->GetBranch(name.c_str());
        if (!physical) {
            if (request.requirement == ColumnRequirement::Required)
                throwMissing(name);
            return;
        }
        TLeaf *leaf = requirePhysicalLeaf(name);
        if (request.leafType != leaf->GetTypeName()) {
            throw SKNano::ConfigError("[BranchManager] branch '" + name +
                                      "' has type " + leaf->GetTypeName() +
                                      ", expected " + request.leafType + " in " +
                                      treeContext());
        }
        const bool physicalVector = leaf->GetLeafCount() != nullptr;
        if (physicalVector != request.vector ||
            (!physicalVector && leaf->GetLenStatic() != 1)) {
            throw SKNano::ConfigError(
                "[BranchManager] scalar/vector cardinality mismatch for branch '" +
                name + "' in " + treeContext());
        }
    }

    template <typename T>
    void registerRequest(const std::string &name, bool vector,
                         ColumnRequirement requirement) {
        const char *expected = ExpectedRootLeafType<T>();
        if (!expected) {
            throw SKNano::ConfigError(
                "[BranchManager] unsupported C++ column type for branch '" + name +
                "'");
        }
        Request requested{expected, vector, requirement};
        const auto priorIt = requests.find(name);
        if (priorIt != requests.end()) {
            if (priorIt->second.leafType != requested.leafType ||
                priorIt->second.vector != requested.vector) {
                throw SKNano::ConfigError(
                    "[BranchManager] conflicting typed handle request for branch '" +
                    name + "'");
            }
            if (requirement == ColumnRequirement::Required)
                requested.requirement = ColumnRequirement::Required;
            else
                requested.requirement = priorIt->second.requirement;
        }
        validateRequest(name, requested);
        if (priorIt == requests.end())
            requests.emplace(name, std::move(requested));
        else
            priorIt->second = std::move(requested);
    }

    template <typename T, typename CountT>
    SKNano::ColumnSource<T> *createVector(const std::string &name,
                                          const std::string &countName) {
        auto &count = getOrCreateScalar<CountT>(countName);
        auto owned = std::make_unique<BranchVector<T, CountT>>(name.c_str(), count);
        auto *raw = owned.get();
        ownedBranches.emplace_back(std::move(owned));
        try {
            registerVector(*raw);
        } catch (...) {
            ownedBranches.pop_back();
            throw;
        }
        return raw;
    }

    TTree *tree = nullptr;
    SKNano::RNTupleSource *rntupleSource = nullptr;
    const Long64_t *entrySource = nullptr;
    const std::uint64_t *epochSource = nullptr;
    std::function<void(const std::string &)> activationCallback;
    std::vector<BranchBase *> branches;
    std::unordered_map<std::string, BranchBase *> branchesByName;
    std::vector<std::unique_ptr<BranchBase>> ownedBranches;
    std::unordered_map<std::string, Request> requests;
    std::unordered_set<std::string> activeBranchNames;
};

template <typename T>
class ColumnHandle {
public:
    bool available() const { return manager && manager->available(branchName); }

    T operator[](std::size_t index) const {
        return requireSource()->get(index);
    }

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

    SKNano::ColumnSource<T> *requireSource() const {
        if (!manager)
            throw SKNano::LogicError("[ColumnHandle] detached handle access");
        auto *source = manager->resolveColumn<T>(branchName, requirement);
        if (!source) {
            throw SKNano::ConfigError(
                "[ColumnHandle] optional branch '" + branchName +
                "' accessed without checking available()");
        }
        return source;
    }

    BranchManager *manager = nullptr;
    std::string branchName;
    ColumnRequirement requirement = ColumnRequirement::Required;
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
        auto *source = manager->resolveScalar<T>(branchName, requirement);
        if (!source) {
            throw SKNano::ConfigError(
                "[ScalarHandle] optional branch '" + branchName +
                "' accessed without checking available()");
        }
        return source;
    }

    BranchManager *manager = nullptr;
    std::string branchName;
    ColumnRequirement requirement = ColumnRequirement::Required;
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
