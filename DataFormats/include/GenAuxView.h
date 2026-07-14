#ifndef GENAUXVIEW_H
#define GENAUXVIEW_H

#include <cstddef>

#include "EventRange.h"
#include "TLorentzVector.h"
#include "ViewColumns.h"

template <typename Storage>
class GenKinematicRef {
public:
    GenKinematicRef() = default;
    GenKinematicRef(const Storage *storage, std::size_t index)
        : store_(storage), index_(index) {}

    bool valid() const { return store_ && index_ < store_->size(); }
    float Pt() const { return store_->pt[index_]; }
    float Eta() const { return store_->eta[index_]; }
    float Phi() const { return store_->phi[index_]; }
    float M() const { return store_->mass[index_]; }
    float Mass() const { return M(); }
    TLorentzVector P4() const {
        TLorentzVector value;
        value.SetPtEtaPhiM(Pt(), Eta(), Phi(), M());
        return value;
    }

protected:
    const Storage *store_ = nullptr;
    std::size_t index_ = 0;
};

struct GenDressedLeptonSoA {
    ColumnView<float> pt;
    ColumnView<float> eta;
    ColumnView<float> phi;
    ColumnView<float> mass;
    ColumnView<int> pdgId;
    BoolColumnView hasTauAnc;
    std::size_t size() const { return pt.size(); }
};

class GenDressedLeptonView
    : public GenKinematicRef<GenDressedLeptonSoA> {
public:
    using GenKinematicRef::GenKinematicRef;
    int PdgId() const { return this->store_->pdgId[this->index_]; }
    bool HasTauAnc() const { return this->store_->hasTauAnc[this->index_]; }
};
using GenDressedLeptonViewCollection =
    EventRange<GenDressedLeptonSoA, GenDressedLeptonView>;

struct GenIsolatedPhotonSoA {
    ColumnView<float> pt;
    ColumnView<float> eta;
    ColumnView<float> phi;
    ColumnView<float> mass;
    std::size_t size() const { return pt.size(); }
};
using GenIsolatedPhotonView = GenKinematicRef<GenIsolatedPhotonSoA>;
using GenIsolatedPhotonViewCollection =
    EventRange<GenIsolatedPhotonSoA, GenIsolatedPhotonView>;

struct GenVisTauSoA {
    ColumnView<float> pt;
    ColumnView<float> eta;
    ColumnView<float> phi;
    ColumnView<float> mass;
    ColumnView<short> charge;
    ColumnView<short> genPartIdxMother;
    ColumnView<unsigned char> status;
    std::size_t size() const { return pt.size(); }
};

class GenVisTauView : public GenKinematicRef<GenVisTauSoA> {
public:
    using GenKinematicRef::GenKinematicRef;
    short Charge() const { return this->store_->charge[this->index_]; }
    short GenPartIdxMother() const {
        return this->store_->genPartIdxMother[this->index_];
    }
    unsigned char Status() const { return this->store_->status[this->index_]; }
};
using GenVisTauViewCollection = EventRange<GenVisTauSoA, GenVisTauView>;

#endif // GENAUXVIEW_H
