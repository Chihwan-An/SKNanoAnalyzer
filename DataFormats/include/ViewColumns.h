#ifndef VIEWCOLUMNS_H
#define VIEWCOLUMNS_H

#include <cstddef>
#include <cstdint>

#include "AnalysisException.h"
#include "ColumnSource.h"

// Generic column view referencing a BranchVector without materialising data.
template <typename T>
class ColumnView {
public:
    using BranchType = SKNano::ColumnSource<T>;

    ColumnView() = default;

    void bind(const BranchType *branch) {
        data = branch;
        boundEpoch = data ? data->epoch() : 0;
    }

    T operator[](std::size_t idx) const {
        assertCurrentEpoch();
        requireAvailable();
        return data->get(idx);
    }

    std::size_t size() const {
        assertCurrentEpoch();
        requireAvailable();
        return data->size();
    }
    bool empty() const { return size() == 0; }

    SKNano::ContiguousView<T> snapshot() const {
        assertCurrentEpoch();
        requireAvailable();
        return data->snapshot();
    }

    bool available() const noexcept { return data && data->valid(); }
    const BranchType *branch() const { return data; }

private:
    void requireAvailable() const {
        if (!data)
            throw SKNano::ConfigError("[ColumnView] unbound column access");
        if (!data->valid())
            throw SKNano::ConfigError("[ColumnView] unavailable column access");
    }

    void assertCurrentEpoch() const {
        if (data && data->epoch() != boundEpoch)
            throw SKNano::LogicError("[ColumnView] stale event view access");
    }
    const BranchType *data = nullptr;
    std::uint64_t boundEpoch = 0;
};

// Boolean specialisation using the dedicated BranchVector<bool> implementation.
class BoolColumnView {
public:
    using BranchType = SKNano::ColumnSource<bool>;

    BoolColumnView() = default;

    void bind(const BranchType *branch) {
        data = branch;
        boundEpoch = data ? data->epoch() : 0;
    }

    bool operator[](std::size_t idx) const {
        assertCurrentEpoch();
        requireAvailable();
        return data->get(idx);
    }

    std::size_t size() const {
        assertCurrentEpoch();
        requireAvailable();
        return data->size();
    }
    bool empty() const { return size() == 0; }

    SKNano::ByteContiguousView snapshot() const {
        assertCurrentEpoch();
        requireAvailable();
        return data->snapshot();
    }

    bool available() const noexcept { return data && data->valid(); }
    const BranchType *branch() const { return data; }

private:
    void requireAvailable() const {
        if (!data)
            throw SKNano::ConfigError("[BoolColumnView] unbound column access");
        if (!data->valid())
            throw SKNano::ConfigError(
                "[BoolColumnView] unavailable column access");
    }

    void assertCurrentEpoch() const {
        if (data && data->epoch() != boundEpoch)
            throw SKNano::LogicError("[BoolColumnView] stale event view access");
    }
    const BranchType *data = nullptr;
    std::uint64_t boundEpoch = 0;
};

#endif // VIEWCOLUMNS_H
