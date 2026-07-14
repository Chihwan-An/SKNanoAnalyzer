#ifndef SKNanoCore_EventBlock_h
#define SKNanoCore_EventBlock_h

#include "AnalysisException.h"
#include "ExecutionPlan.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <vector>

namespace SKNano {

class EventBlock;

template <typename T>
class RaggedEventView {
public:
    RaggedEventView() = default;
    const T *data() const { assertCurrent(); return data_; }
    std::size_t size() const { assertCurrent(); return size_; }
    bool empty() const { return size() == 0; }
    const T &operator[](std::size_t index) const {
        assertCurrent();
        if (index >= size_)
            throw EventDataError("[RaggedEventView] index out of range");
        return data_[index];
    }
private:
    friend class EventBlock;
    RaggedEventView(const std::uint64_t *epochSource, std::uint64_t epoch,
                    const T *data, std::size_t size)
        : epochSource_(epochSource), epoch_(epoch), data_(data), size_(size) {}
    void assertCurrent() const {
        if (epochSource_ && *epochSource_ != epoch_)
            throw LogicError("[RaggedEventView] stale block view access");
    }
    const std::uint64_t *epochSource_ = nullptr;
    std::uint64_t epoch_ = 0;
    const T *data_ = nullptr;
    std::size_t size_ = 0;
};

class EventBlock {
public:
    explicit EventBlock(std::size_t memoryBudgetBytes = 64 * 1024 * 1024,
                        std::size_t maxEvents = 256)
        : memoryBudgetBytes_(memoryBudgetBytes), maxEvents_(maxEvents) {
        if (memoryBudgetBytes_ == 0 || maxEvents_ == 0)
            throw ConfigError("[EventBlock] memory budget and max events must be positive");
    }

    template <typename T>
    void registerColumn(ColumnId id) {
        if (building_)
            throw LogicError(
                "[EventBlock] columns cannot be registered while building");
        const auto key = id.value();
        if (!id)
            throw ConfigError("[EventBlock] invalid ColumnId");
        const auto found = columns_.find(key);
        if (found != columns_.end()) {
            if (found->second->type() != typeid(T))
                throw ConfigError("[EventBlock] conflicting type for ColumnId");
            return;
        }
        columns_.emplace(key, std::make_unique<TypedColumn<T>>());
    }

    void begin(std::uint64_t treeToken, std::int64_t firstEntry) {
        reset();
        treeToken_ = treeToken;
        firstEntry_ = firstEntry;
        building_ = true;
    }

    template <typename T>
    bool canAppend(ColumnId id, std::size_t count) const {
        const auto &column = typedColumn<T>(id);
        if (count > (static_cast<std::size_t>(-1) - sizeof(std::size_t)) /
                        sizeof(T))
            return false;
        const std::size_t additional = count * sizeof(T) + sizeof(std::size_t);
        return !eventReservationActive_ &&
               column.eventCount() == eventCount_ && eventCount_ < maxEvents_ &&
               bytesUsed() + additional <= memoryBudgetBytes_;
    }

    bool canBeginEvent(std::size_t payloadBytes) const noexcept {
        if (!building_ || eventReservationActive_ || eventCount_ >= maxEvents_)
            return false;
        const std::size_t offsetBytes = columns_.size() * sizeof(std::size_t);
        if (payloadBytes > memoryBudgetBytes_ ||
            offsetBytes > memoryBudgetBytes_ - payloadBytes)
            return false;
        return bytesUsed() <= memoryBudgetBytes_ - payloadBytes - offsetBytes;
    }

    // Reserve the complete event before appending any column. This makes a
    // capacity rejection transactional: the block is unchanged on false.
    bool beginEvent(std::uint64_t treeToken, std::int64_t entry,
                    std::size_t payloadBytes) {
        if (!building_ || treeToken != treeToken_)
            throw LogicError("[EventBlock] event crossed a tree/file boundary");
        if (entry != firstEntry_ + static_cast<std::int64_t>(eventCount_))
            throw LogicError("[EventBlock] event entries must be contiguous");
        if (!canBeginEvent(payloadBytes))
            return false;
        eventReservationActive_ = true;
        reservedPayloadBytes_ = payloadBytes;
        appendedPayloadBytes_ = 0;
        return true;
    }

    template <typename T>
    void append(ColumnId id, const T *data, std::size_t count) {
        if (!building_)
            throw LogicError("[EventBlock] begin() was not called");
        auto &column = typedColumn<T>(id);
        if (column.eventCount() != eventCount_)
            throw LogicError("[EventBlock] column appended twice or out of order");
        if (count > static_cast<std::size_t>(-1) / sizeof(T))
            throw EventDataError("[EventBlock] column byte size overflow");
        const std::size_t payloadBytes = count * sizeof(T);
        if (eventReservationActive_) {
            if (payloadBytes > reservedPayloadBytes_ - appendedPayloadBytes_)
                throw EventDataError(
                    "[EventBlock] event exceeded its payload reservation");
        } else if (!canAppend<T>(id, count)) {
            throw EventDataError("[EventBlock] block capacity exceeded");
        }
        if (count && !data)
            throw EventDataError("[EventBlock] null non-empty column input");
        column.append(data, count);
        if (eventReservationActive_)
            appendedPayloadBytes_ += payloadBytes;
    }

    void finishEvent(std::uint64_t treeToken, std::int64_t entry) {
        if (!building_ || treeToken != treeToken_)
            throw LogicError("[EventBlock] event crossed a tree/file boundary");
        if (entry != firstEntry_ + static_cast<std::int64_t>(eventCount_))
            throw LogicError("[EventBlock] event entries must be contiguous");
        for (const auto &item : columns_) {
            if (item.second->eventCount() != eventCount_ + 1)
                throw LogicError("[EventBlock] not every registered column was appended");
        }
        if (eventReservationActive_ &&
            appendedPayloadBytes_ != reservedPayloadBytes_)
            throw LogicError(
                "[EventBlock] event payload reservation was not fully consumed");
        ++eventCount_;
        eventReservationActive_ = false;
        reservedPayloadBytes_ = 0;
        appendedPayloadBytes_ = 0;
    }

    void seal() {
        if (!building_)
            throw LogicError("[EventBlock] block is not being built");
        if (eventReservationActive_)
            throw LogicError("[EventBlock] cannot seal an unfinished event");
        building_ = false;
    }

    template <typename T>
    RaggedEventView<T> event(ColumnId id, std::size_t eventIndex) const {
        if (building_)
            throw LogicError("[EventBlock] seal block before reading");
        const auto &column = typedColumn<T>(id);
        if (eventIndex >= eventCount_)
            throw EventDataError("[EventBlock] event index out of range");
        const std::size_t begin = column.offsets[eventIndex];
        const std::size_t end = column.offsets[eventIndex + 1];
        const T *data = column.values.empty() ? nullptr
                                              : column.values.data() + begin;
        return RaggedEventView<T>(&epoch_, epoch_, data,
                                  end - begin);
    }

    void reset() {
        ++epoch_;
        for (auto &item : columns_)
            item.second->clear();
        eventCount_ = 0;
        firstEntry_ = -1;
        treeToken_ = 0;
        building_ = false;
        eventReservationActive_ = false;
        reservedPayloadBytes_ = 0;
        appendedPayloadBytes_ = 0;
    }

    std::size_t eventCount() const noexcept { return eventCount_; }
    std::size_t bytesUsed() const {
        std::size_t result = 0;
        for (const auto &item : columns_)
            result += item.second->bytesUsed();
        return result;
    }
    std::uint64_t treeToken() const noexcept { return treeToken_; }
    std::int64_t firstEntry() const noexcept { return firstEntry_; }
    std::uint64_t epoch() const noexcept { return epoch_; }

private:
    struct ColumnBase {
        virtual ~ColumnBase() = default;
        virtual const std::type_info &type() const = 0;
        virtual std::size_t eventCount() const = 0;
        virtual std::size_t bytesUsed() const = 0;
        virtual void clear() = 0;
    };

    template <typename T>
    struct TypedColumn final : ColumnBase {
        std::vector<T> values;
        std::vector<std::size_t> offsets{0};
        const std::type_info &type() const override { return typeid(T); }
        std::size_t eventCount() const override { return offsets.size() - 1; }
        std::size_t bytesUsed() const override {
            return values.size() * sizeof(T) + offsets.size() * sizeof(std::size_t);
        }
        void append(const T *data, std::size_t count) {
            if (count)
                values.insert(values.end(), data, data + count);
            offsets.push_back(values.size());
        }
        void clear() override { values.clear(); offsets.assign(1, 0); }
    };

    template <typename T>
    TypedColumn<T> &typedColumn(ColumnId id) {
        return const_cast<TypedColumn<T> &>(
            static_cast<const EventBlock *>(this)->typedColumn<T>(id));
    }
    template <typename T>
    const TypedColumn<T> &typedColumn(ColumnId id) const {
        const auto found = columns_.find(id.value());
        if (found == columns_.end())
            throw ConfigError("[EventBlock] unregistered ColumnId");
        auto *typed = dynamic_cast<TypedColumn<T> *>(found->second.get());
        if (!typed)
            throw ConfigError("[EventBlock] ColumnId type mismatch");
        return *typed;
    }

    std::unordered_map<std::uint32_t, std::unique_ptr<ColumnBase>> columns_;
    std::size_t memoryBudgetBytes_;
    std::size_t maxEvents_;
    std::size_t eventCount_ = 0;
    std::uint64_t treeToken_ = 0;
    std::int64_t firstEntry_ = -1;
    std::uint64_t epoch_ = 1;
    bool building_ = false;
    bool eventReservationActive_ = false;
    std::size_t reservedPayloadBytes_ = 0;
    std::size_t appendedPayloadBytes_ = 0;
};

} // namespace SKNano

#endif
