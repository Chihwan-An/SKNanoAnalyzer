#ifndef SKNanoCore_EventArena_h
#define SKNanoCore_EventArena_h

#include "AnalysisException.h"

#include <cstddef>
#include <cstdint>
#include <memory_resource>
#include <new>
#include <type_traits>
#include <vector>

namespace SKNano {

template <typename T>
class ArenaArray {
public:
    ArenaArray() = default;
    T *data() const { assertCurrent(); return data_; }
    std::size_t size() const { assertCurrent(); return size_; }
    T &operator[](std::size_t index) const {
        assertCurrent();
        if (index >= size_)
            throw EventDataError("[ArenaArray] index out of range");
        return data_[index];
    }
private:
    friend class EventArena;
    ArenaArray(const std::uint64_t *epochSource, std::uint64_t epoch,
               T *data, std::size_t size)
        : epochSource_(epochSource), epoch_(epoch), data_(data), size_(size) {}
    void assertCurrent() const {
        if (epochSource_ && *epochSource_ != epoch_)
            throw LogicError("[ArenaArray] stale arena allocation access");
    }
    const std::uint64_t *epochSource_ = nullptr;
    std::uint64_t epoch_ = 0;
    T *data_ = nullptr;
    std::size_t size_ = 0;
};

class EventArena {
public:
    explicit EventArena(std::size_t initialBytes = 1024 * 1024)
        : buffer_(initialBytes),
          resource_(buffer_.data(), buffer_.size()) {
        if (initialBytes == 0)
            throw ConfigError("[EventArena] initial size must be positive");
    }
    EventArena(const EventArena &) = delete;
    EventArena &operator=(const EventArena &) = delete;

    std::pmr::memory_resource *resource() noexcept { return &resource_; }

    template <typename T>
    ArenaArray<T> allocateArray(std::size_t size,
                                std::size_t alignment = 64) {
        static_assert(std::is_trivially_destructible_v<T>,
                      "EventArena arrays must not require destruction");
        if (alignment < alignof(T))
            alignment = alignof(T);
        T *data = size ? static_cast<T *>(
                            resource_.allocate(size * sizeof(T), alignment))
                       : nullptr;
        return ArenaArray<T>(&epoch_, epoch_, data, size);
    }

    template <typename T>
    std::pmr::vector<T> vector() {
        return std::pmr::vector<T>(&resource_);
    }

    void reset() {
        ++epoch_;
        resource_.release();
    }

    std::uint64_t epoch() const noexcept { return epoch_; }

private:
    std::vector<std::byte> buffer_;
    std::pmr::monotonic_buffer_resource resource_;
    std::uint64_t epoch_ = 1;
};

} // namespace SKNano

#endif
