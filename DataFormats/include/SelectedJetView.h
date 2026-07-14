#ifndef SELECTEDJETVIEW_H
#define SELECTEDJETVIEW_H

#include <cstddef>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include "JetView.h"

enum class JetKinematicProjection {
    Corrected,
    Nominal,
    JesUp,
    JesDown,
    JerUp,
    JerDown
};

class SelectedJetView : public JetView {
public:
    SelectedJetView() = default;
    SelectedJetView(const JetSoA *storage, std::size_t index,
                    JetKinematicProjection projection)
        : JetView(storage, index), raw_(storage, index), index_(index),
          projection_(projection) {}

    float Pt() const {
        switch (projection_) {
        case JetKinematicProjection::Corrected: return raw_.CorrectedPt();
        case JetKinematicProjection::Nominal: return raw_.SmearedPtNominal();
        case JetKinematicProjection::JesUp: return raw_.JesPtUp();
        case JetKinematicProjection::JesDown: return raw_.JesPtDown();
        case JetKinematicProjection::JerUp: return raw_.SmearedPtUp();
        case JetKinematicProjection::JerDown: return raw_.SmearedPtDown();
        }
        throw std::logic_error("invalid jet kinematic projection");
    }

    float M() const {
        switch (projection_) {
        case JetKinematicProjection::Corrected: return raw_.CorrectedMass();
        case JetKinematicProjection::Nominal: return raw_.SmearedMassNominal();
        case JetKinematicProjection::JesUp: return raw_.JesMassUp();
        case JetKinematicProjection::JesDown: return raw_.JesMassDown();
        case JetKinematicProjection::JerUp: return raw_.SmearedMassUp();
        case JetKinematicProjection::JerDown: return raw_.SmearedMassDown();
        }
        throw std::logic_error("invalid jet kinematic projection");
    }

    float Mass() const { return M(); }
    int OriginalIndex() const { return static_cast<int>(index_); }

    TLorentzVector P4() const {
        TLorentzVector result;
        result.SetPtEtaPhiM(Pt(), Eta(), Phi(), M());
        return result;
    }
    operator TLorentzVector() const { return P4(); }
    float Px() const { return P4().Px(); }
    float Py() const { return P4().Py(); }
    float Pz() const { return P4().Pz(); }
    float E() const { return P4().E(); }
    TVector3 Vect() const { return P4().Vect(); }
    template <typename Other>
    float DeltaR(const Other &other) const {
        if constexpr (requires { other.P4(); })
            return P4().DeltaR(other.P4());
        else
            return P4().DeltaR(TLorentzVector(other));
    }

private:
    JetView raw_;
    std::size_t index_ = 0;
    JetKinematicProjection projection_ = JetKinematicProjection::Nominal;
};

class SelectedJetViewCollection {
public:
    class const_iterator {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = SelectedJetView;
        using difference_type = std::ptrdiff_t;
        using reference = SelectedJetView;

        const_iterator() = default;
        const_iterator(const SelectedJetViewCollection *owner,
                       std::size_t position)
            : owner_(owner), position_(position) {}
        reference operator*() const { return (*owner_)[position_]; }
        reference operator[](difference_type offset) const {
            return (*owner_)[position_ + offset];
        }
        const_iterator &operator++() { ++position_; return *this; }
        const_iterator operator++(int) { auto copy = *this; ++*this; return copy; }
        const_iterator &operator--() { --position_; return *this; }
        const_iterator &operator+=(difference_type n) { position_ += n; return *this; }
        const_iterator &operator-=(difference_type n) { position_ -= n; return *this; }
        friend const_iterator operator+(const_iterator it, difference_type n) { it += n; return it; }
        friend const_iterator operator-(const_iterator it, difference_type n) { it -= n; return it; }
        friend difference_type operator-(const const_iterator &a, const const_iterator &b) { return static_cast<difference_type>(a.position_) - static_cast<difference_type>(b.position_); }
        friend bool operator==(const const_iterator &a, const const_iterator &b) { return a.owner_ == b.owner_ && a.position_ == b.position_; }
        friend bool operator<(const const_iterator &a, const const_iterator &b) { return a.position_ < b.position_; }
    private:
        const SelectedJetViewCollection *owner_ = nullptr;
        std::size_t position_ = 0;
    };

    SelectedJetViewCollection() = default;
    SelectedJetViewCollection(std::shared_ptr<JetSoA> storage,
                              std::vector<std::size_t> indices,
                              JetKinematicProjection projection)
        : storage_(std::move(storage)), indices_(std::move(indices)),
          projection_(projection) {}

    std::size_t size() const {
        if (storage_)
            static_cast<void>(storage_->size());
        return indices_.size();
    }
    bool empty() const { return size() == 0; }
    SelectedJetView operator[](std::size_t position) const {
        if (position >= size())
            throw std::out_of_range("SelectedJetViewCollection index");
        return SelectedJetView(storage_.get(), indices_[position], projection_);
    }
    SelectedJetView at(std::size_t position) const { return (*this)[position]; }
    std::size_t rawIndex(std::size_t position) const { return indices_.at(position); }
    const std::vector<std::size_t> &indices() const { return indices_; }
    const std::shared_ptr<JetSoA> &storage() const { return storage_; }
    JetKinematicProjection projection() const { return projection_; }
    SelectedJetViewCollection selectPositions(
        const std::vector<std::size_t> &positions) const {
        std::vector<std::size_t> selected;
        selected.reserve(positions.size());
        for (const auto position : positions)
            selected.push_back(indices_.at(position));
        return SelectedJetViewCollection(storage_, std::move(selected),
                                         projection_);
    }
    const_iterator begin() const { return const_iterator(this, 0); }
    const_iterator end() const { return const_iterator(this, size()); }

private:
    std::shared_ptr<JetSoA> storage_;
    std::vector<std::size_t> indices_;
    JetKinematicProjection projection_ = JetKinematicProjection::Nominal;
};

#endif
