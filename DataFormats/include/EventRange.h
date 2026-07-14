#ifndef EVENTRANGE_H
#define EVENTRANGE_H

#include <cstddef>
#include <iterator>
#include <memory>
#include <utility>
#include <vector>

#include "AnalysisException.h"

// Event-scoped range over a struct-of-arrays store.  The range owns only the
// small per-event storage descriptor/derived lanes; elements are non-owning
// references produced on demand.  This avoids a vector<View> and one
// shared_ptr reference count per physics object.
template <typename Storage, typename Ref>
class EventRange {
public:
    using storage_type = Storage;
    using value_type = Ref;
    using size_type = std::size_t;

    EventRange() = default;
    explicit EventRange(std::shared_ptr<Storage> storage)
        : storage_(std::move(storage)), identity_(true) {}
    EventRange(std::shared_ptr<Storage> storage, std::vector<size_type> indices)
        : storage_(std::move(storage)), indices_(std::move(indices)),
          identity_(false) {}

    Ref operator[](size_type position) const {
        checkPosition(position);
        return Ref(storage_.get(), rawIndex(position));
    }

    Ref at(size_type position) const { return (*this)[position]; }

    size_type size() const {
        if (!storage_)
            return 0;
        assertCurrentEvent();
        return identity_ ? storage_->size() : indices_.size();
    }

    bool empty() const { return size() == 0; }

    size_type rawIndex(size_type position) const {
        return identity_ ? position : indices_[position];
    }

    const std::shared_ptr<Storage> &storage() const { return storage_; }

    class const_iterator {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = Ref;
        using difference_type = std::ptrdiff_t;
        using reference = Ref;
        using pointer = void;

        const_iterator() = default;
        const_iterator(const EventRange *range, size_type position)
            : range_(range), position_(position) {}

        Ref operator*() const { return (*range_)[position_]; }
        Ref operator[](difference_type offset) const {
            return (*range_)[static_cast<size_type>(
                static_cast<difference_type>(position_) + offset)];
        }
        const_iterator &operator++() { ++position_; return *this; }
        const_iterator operator++(int) { auto copy = *this; ++*this; return copy; }
        const_iterator &operator--() { --position_; return *this; }
        const_iterator operator--(int) { auto copy = *this; --*this; return copy; }
        const_iterator &operator+=(difference_type offset) {
            position_ = static_cast<size_type>(
                static_cast<difference_type>(position_) + offset);
            return *this;
        }
        const_iterator &operator-=(difference_type offset) { return *this += -offset; }

        friend const_iterator operator+(const_iterator it, difference_type offset) {
            it += offset; return it;
        }
        friend const_iterator operator+(difference_type offset, const_iterator it) {
            it += offset; return it;
        }
        friend const_iterator operator-(const_iterator it, difference_type offset) {
            it -= offset; return it;
        }
        friend difference_type operator-(const const_iterator &left,
                                         const const_iterator &right) {
            return static_cast<difference_type>(left.position_) -
                   static_cast<difference_type>(right.position_);
        }
        friend bool operator==(const const_iterator &left, const const_iterator &right) {
            return left.range_ == right.range_ && left.position_ == right.position_;
        }
        friend bool operator!=(const const_iterator &left, const const_iterator &right) {
            return !(left == right);
        }
        friend bool operator<(const const_iterator &left, const const_iterator &right) {
            return left.position_ < right.position_;
        }
        friend bool operator>(const const_iterator &left, const const_iterator &right) {
            return right < left;
        }
        friend bool operator<=(const const_iterator &left, const const_iterator &right) {
            return !(right < left);
        }
        friend bool operator>=(const const_iterator &left, const const_iterator &right) {
            return !(left < right);
        }

    private:
        const EventRange *range_ = nullptr;
        size_type position_ = 0;
    };

    const_iterator begin() const { return const_iterator(this, 0); }
    const_iterator end() const { return const_iterator(this, size()); }

private:
    void assertCurrentEvent() const {
        // Every SoA has a primary column-backed size().  Besides returning the
        // cardinality this performs the epoch check, including for accesses
        // that subsequently read a derived lane.
        static_cast<void>(storage_->size());
    }

    void checkPosition(size_type position) const {
        const size_type count = size();
        if (position >= count)
            throw SKNano::LogicError("[EventRange] object index out of range");
    }

    std::shared_ptr<Storage> storage_;
    std::vector<size_type> indices_;
    bool identity_ = false;
};

#endif // EVENTRANGE_H
