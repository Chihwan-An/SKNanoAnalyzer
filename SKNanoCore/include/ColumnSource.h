#ifndef SKNanoCore_ColumnSource_h
#define SKNanoCore_ColumnSource_h

#include <cstddef>
#include <cstdint>
#include <stdexcept>

namespace SKNano {

template <typename T>
class ColumnSource;
template <typename T>
class ContiguousView;
class ByteContiguousView;

template <typename T>
class ColumnSource {
public:
    virtual ~ColumnSource() = default;
    virtual T get(std::size_t idx) const = 0;
    virtual std::size_t size() const = 0;
    virtual bool valid() const = 0;
    virtual std::uint64_t epoch() const = 0;
    virtual ContiguousView<T> snapshot() const = 0;
};

template <>
class ColumnSource<bool> {
public:
    virtual ~ColumnSource() = default;
    virtual bool get(std::size_t idx) const = 0;
    virtual std::size_t size() const = 0;
    virtual bool valid() const = 0;
    virtual std::uint64_t epoch() const = 0;
    virtual ByteContiguousView snapshot() const = 0;
};

// A view is valid only for the event epoch at which it was acquired.  In
// release builds callers use data()/size() once and run raw contiguous loops;
// Snapshot boundary access always retains epoch checks. Hot kernels should
// obtain data() once and iterate that raw span without repeated dispatch.
template <typename T>
class ContiguousView {
public:
    ContiguousView() = default;
    ContiguousView(const ColumnSource<T> *source, const T *data,
                   std::size_t size, std::uint64_t epoch)
        : source_(source), data_(data), size_(size), epoch_(epoch) {}

    const T *data() const {
        assertCurrentEpoch();
        return data_;
    }
    std::size_t size() const {
        assertCurrentEpoch();
        return size_;
    }
    bool empty() const { return size() == 0; }
    const T &operator[](std::size_t index) const {
        assertCurrentEpoch();
        if (index >= size_)
            throw std::out_of_range("[ContiguousView] column index out of range");
        return data_[index];
    }

private:
    void assertCurrentEpoch() const {
        if (source_ && source_->epoch() != epoch_)
            throw std::logic_error("[ContiguousView] stale event snapshot access");
    }

    const ColumnSource<T> *source_ = nullptr;
    const T *data_ = nullptr;
    std::size_t size_ = 0;
    std::uint64_t epoch_ = 0;
};

// ROOT stores Bool_t leaves in byte buffers, not std::vector<bool>.  Keep the
// fast path explicitly byte-typed so it never relies on bool representation.
class ByteContiguousView {
public:
    ByteContiguousView() = default;
    ByteContiguousView(const ColumnSource<bool> *source,
                       const std::uint8_t *data, std::size_t size,
                       std::uint64_t epoch)
        : source_(source), data_(data), size_(size), epoch_(epoch) {}

    const std::uint8_t *data() const {
        assertCurrentEpoch();
        return data_;
    }
    std::size_t size() const {
        assertCurrentEpoch();
        return size_;
    }
    bool empty() const { return size() == 0; }
    bool operator[](std::size_t index) const {
        assertCurrentEpoch();
        if (index >= size_)
            throw std::out_of_range("[ByteContiguousView] column index out of range");
        return data_[index] != 0;
    }

private:
    void assertCurrentEpoch() const {
        if (source_ && source_->epoch() != epoch_)
            throw std::logic_error("[ByteContiguousView] stale event snapshot access");
    }

    const ColumnSource<bool> *source_ = nullptr;
    const std::uint8_t *data_ = nullptr;
    std::size_t size_ = 0;
    std::uint64_t epoch_ = 0;
};

} // namespace SKNano

#endif
