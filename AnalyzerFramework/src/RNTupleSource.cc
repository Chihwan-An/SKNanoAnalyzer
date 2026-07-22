#include <AnalyzerFramework/RNTupleSource.h>

#include "AnalysisException.h"

#include <ROOT/RNTupleDescriptor.hxx>
#include <ROOT/RNTupleReadOptions.hxx>
#include <ROOT/RNTupleReader.hxx>
#include <ROOT/RNTupleTypes.hxx>
#include <ROOT/RNTupleView.hxx>
#include <ROOT/RVec.hxx>

#include <algorithm>
#include <cstdint>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeindex>
#include <utility>
#include <vector>

namespace SKNano {
namespace {

template <typename DiskT, typename OutputT = DiskT>
class ScalarColumn final : public RNTupleScalarColumn {
public:
    ScalarColumn(ROOT::RNTupleReader &reader, const std::string &name)
        : view_(reader.GetView<DiskT>(name)) {}

    void read(std::uint64_t entry, void *destination) const override {
        *static_cast<OutputT *>(destination) =
            static_cast<OutputT>(view_(entry));
    }

private:
    mutable ROOT::RNTupleView<DiskT> view_;
};

template <typename T>
class VectorColumn final : public RNTupleVectorColumn {
public:
    VectorColumn(ROOT::RNTupleReader &reader, const std::string &name)
        : view_(reader.GetView<ROOT::VecOps::RVec<T>>(name)) {}

    RNTupleSpan read(std::uint64_t entry) const override {
        const auto &values = view_(entry);
        return {values.data(), values.size()};
    }

private:
    mutable ROOT::RNTupleView<ROOT::VecOps::RVec<T>> view_;
};

// RVec<bool> deliberately does not expose a bool* suitable for the analyzer's
// byte-contiguous Bool_t contract. Materialize only this specialization; all
// arithmetic vector fields remain zero-copy views into RNTuple pages.
class BoolVectorColumn final : public RNTupleVectorColumn {
public:
    BoolVectorColumn(ROOT::RNTupleReader &reader, const std::string &name)
        : view_(reader.GetView<ROOT::VecOps::RVec<bool>>(name)) {}

    RNTupleSpan read(std::uint64_t entry) const override {
        const auto &values = view_(entry);
        bytes_.resize(values.size());
        std::transform(values.begin(), values.end(), bytes_.begin(),
                       [](bool value) { return static_cast<std::uint8_t>(value); });
        return {bytes_.data(), bytes_.size()};
    }

private:
    mutable ROOT::RNTupleView<ROOT::VecOps::RVec<bool>> view_;
    mutable std::vector<std::uint8_t> bytes_;
};

template <typename T>
bool Is(std::type_index type) {
    return type == std::type_index(typeid(T));
}

template <typename T>
std::shared_ptr<RNTupleScalarColumn>
MakeOrdinaryScalar(ROOT::RNTupleReader &reader, const std::string &name) {
    return std::make_shared<ScalarColumn<T>>(reader, name);
}

template <typename OutputT>
std::shared_ptr<RNTupleScalarColumn>
MakeCardinalityScalar(ROOT::RNTupleReader &reader, const std::string &name,
                      const std::string &fieldType) {
    if (fieldType.find("uint64_t") != std::string::npos)
        return std::make_shared<
            ScalarColumn<ROOT::RNTupleCardinality<std::uint64_t>, OutputT>>(
            reader, name);
    return std::make_shared<
        ScalarColumn<ROOT::RNTupleCardinality<std::uint32_t>, OutputT>>(
        reader, name);
}

bool IsCardinality(const std::string &fieldType) {
    return fieldType.find("RNTupleCardinality") != std::string::npos;
}

} // namespace

class RNTupleSource::Impl {
public:
    std::unique_ptr<ROOT::RNTupleReader> reader;
    std::string file;
};

RNTupleSource::RNTupleSource() : impl_(std::make_unique<Impl>()) {}
RNTupleSource::~RNTupleSource() = default;
RNTupleSource::RNTupleSource(RNTupleSource &&) noexcept = default;
RNTupleSource &RNTupleSource::operator=(RNTupleSource &&) noexcept = default;

void RNTupleSource::open(const std::string &ntupleName,
                         const std::string &fileName, bool enableMetrics,
                         bool enableClusterCache) {
    ROOT::RNTupleReadOptions options;
    options.SetEnableMetrics(enableMetrics);
    options.SetClusterCache(enableClusterCache
        ? ROOT::RNTupleReadOptions::EClusterCache::kOn
        : ROOT::RNTupleReadOptions::EClusterCache::kOff);
    try {
        auto reader = ROOT::RNTupleReader::Open(ntupleName, fileName, options);
        impl_->reader = std::move(reader);
        impl_->file = fileName;
    } catch (const std::exception &error) {
        throw SKNano::ConfigError("[RNTupleSource] cannot open '" + ntupleName +
                                  "' in " + fileName + ": " + error.what());
    }
}

void RNTupleSource::close() {
    impl_->reader.reset();
    impl_->file.clear();
}

bool RNTupleSource::isOpen() const noexcept {
    return static_cast<bool>(impl_->reader);
}

std::uint64_t RNTupleSource::entries() const {
    if (!impl_->reader)
        throw SKNano::LogicError("[RNTupleSource] no RNTuple is open");
    return impl_->reader->GetNEntries();
}

bool RNTupleSource::hasField(const std::string &name) const {
    if (!impl_->reader)
        return false;
    return impl_->reader->GetDescriptor().FindFieldId(name) !=
           ROOT::kInvalidDescriptorId;
}

std::string RNTupleSource::fieldType(const std::string &name) const {
    if (!impl_->reader)
        throw SKNano::LogicError("[RNTupleSource] no RNTuple is open");
    const auto &descriptor = impl_->reader->GetDescriptor();
    const auto id = descriptor.FindFieldId(name);
    if (id == ROOT::kInvalidDescriptorId)
        return {};
    return descriptor.GetFieldDescriptor(id).GetTypeName();
}

const std::string &RNTupleSource::fileName() const noexcept {
    return impl_->file;
}

std::shared_ptr<RNTupleScalarColumn>
RNTupleSource::makeScalarColumn(const std::string &name,
                                std::type_index outputType) const {
    if (!impl_->reader)
        throw SKNano::LogicError("[RNTupleSource] no RNTuple is open");
    if (!hasField(name))
        throw SKNano::ConfigError("[RNTupleSource] missing field '" + name +
                                  "' in " + impl_->file);

    auto &reader = *impl_->reader;
    const auto diskType = fieldType(name);
    try {
        if (IsCardinality(diskType)) {
            if (Is<int>(outputType))
                return MakeCardinalityScalar<int>(reader, name, diskType);
            if (Is<unsigned int>(outputType))
                return MakeCardinalityScalar<unsigned int>(reader, name, diskType);
            if (Is<short>(outputType))
                return MakeCardinalityScalar<short>(reader, name, diskType);
            if (Is<unsigned short>(outputType))
                return MakeCardinalityScalar<unsigned short>(reader, name, diskType);
            if (Is<unsigned char>(outputType))
                return MakeCardinalityScalar<unsigned char>(reader, name, diskType);
            if (Is<Long64_t>(outputType))
                return MakeCardinalityScalar<Long64_t>(reader, name, diskType);
            if (Is<ULong64_t>(outputType))
                return MakeCardinalityScalar<ULong64_t>(reader, name, diskType);
        } else {
            if (Is<float>(outputType)) return MakeOrdinaryScalar<float>(reader, name);
            if (Is<double>(outputType)) return MakeOrdinaryScalar<double>(reader, name);
            if (Is<int>(outputType)) return MakeOrdinaryScalar<int>(reader, name);
            if (Is<unsigned int>(outputType)) return MakeOrdinaryScalar<unsigned int>(reader, name);
            if (Is<bool>(outputType)) return MakeOrdinaryScalar<bool>(reader, name);
            if (Is<short>(outputType)) return MakeOrdinaryScalar<short>(reader, name);
            if (Is<unsigned short>(outputType)) return MakeOrdinaryScalar<unsigned short>(reader, name);
            if (Is<unsigned char>(outputType)) return MakeOrdinaryScalar<unsigned char>(reader, name);
            if (Is<Long64_t>(outputType)) return MakeOrdinaryScalar<Long64_t>(reader, name);
            if (Is<ULong64_t>(outputType)) return MakeOrdinaryScalar<ULong64_t>(reader, name);
        }
    } catch (const std::exception &error) {
        throw SKNano::ConfigError("[RNTupleSource] scalar field '" + name +
                                  "' has incompatible type " + diskType +
                                  " in " + impl_->file + ": " + error.what());
    }
    throw SKNano::ConfigError("[RNTupleSource] unsupported scalar mapping for field '" +
                              name + "' (disk type " + diskType + ")");
}

std::shared_ptr<RNTupleVectorColumn>
RNTupleSource::makeVectorColumn(const std::string &name,
                                std::type_index elementType) const {
    if (!impl_->reader)
        throw SKNano::LogicError("[RNTupleSource] no RNTuple is open");
    if (!hasField(name))
        throw SKNano::ConfigError("[RNTupleSource] missing field '" + name +
                                  "' in " + impl_->file);
    try {
        auto &reader = *impl_->reader;
        if (Is<float>(elementType)) return std::make_shared<VectorColumn<float>>(reader, name);
        if (Is<double>(elementType)) return std::make_shared<VectorColumn<double>>(reader, name);
        if (Is<int>(elementType)) return std::make_shared<VectorColumn<int>>(reader, name);
        if (Is<unsigned int>(elementType)) return std::make_shared<VectorColumn<unsigned int>>(reader, name);
        if (Is<bool>(elementType)) return std::make_shared<BoolVectorColumn>(reader, name);
        if (Is<short>(elementType)) return std::make_shared<VectorColumn<short>>(reader, name);
        if (Is<unsigned short>(elementType)) return std::make_shared<VectorColumn<unsigned short>>(reader, name);
        if (Is<unsigned char>(elementType)) return std::make_shared<VectorColumn<unsigned char>>(reader, name);
        if (Is<Long64_t>(elementType)) return std::make_shared<VectorColumn<Long64_t>>(reader, name);
        if (Is<ULong64_t>(elementType)) return std::make_shared<VectorColumn<ULong64_t>>(reader, name);
    } catch (const std::exception &error) {
        throw SKNano::ConfigError("[RNTupleSource] vector field '" + name +
                                  "' has incompatible type " + fieldType(name) +
                                  " in " + impl_->file + ": " + error.what());
    }
    throw SKNano::ConfigError("[RNTupleSource] unsupported vector mapping for field '" +
                              name + "' (disk type " + fieldType(name) + ")");
}

} // namespace SKNano
