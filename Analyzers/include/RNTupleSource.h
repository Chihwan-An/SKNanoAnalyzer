#ifndef SKNANO_RNTUPLESOURCE_H
#define SKNANO_RNTUPLESOURCE_H

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <typeindex>

namespace SKNano {

// Type-erased bridge between the analyzer's generated branch wrappers and
// ROOT's typed RNTuple views.  Keeping ROOT's RNTuple templates in the .cc
// file avoids exposing their evolving implementation to every analyzer and
// to rootcling.
struct RNTupleSpan {
    const void *data = nullptr;
    std::size_t size = 0;
};

class RNTupleScalarColumn {
public:
    virtual ~RNTupleScalarColumn() = default;
    virtual void read(std::uint64_t entry, void *destination) const = 0;
};

class RNTupleVectorColumn {
public:
    virtual ~RNTupleVectorColumn() = default;
    virtual RNTupleSpan read(std::uint64_t entry) const = 0;
};

class RNTupleSource {
public:
    RNTupleSource();
    ~RNTupleSource();
    RNTupleSource(const RNTupleSource &) = delete;
    RNTupleSource &operator=(const RNTupleSource &) = delete;
    RNTupleSource(RNTupleSource &&) noexcept;
    RNTupleSource &operator=(RNTupleSource &&) noexcept;

    void open(const std::string &ntupleName, const std::string &fileName,
              bool enableMetrics = false, bool enableClusterCache = true);
    void close();
    bool isOpen() const noexcept;
    std::uint64_t entries() const;
    bool hasField(const std::string &name) const;
    std::string fieldType(const std::string &name) const;
    const std::string &fileName() const noexcept;

    std::shared_ptr<RNTupleScalarColumn>
    makeScalarColumn(const std::string &name, std::type_index outputType) const;
    std::shared_ptr<RNTupleVectorColumn>
    makeVectorColumn(const std::string &name, std::type_index elementType) const;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace SKNano

#endif
