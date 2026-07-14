#ifndef TRIGOBJVIEW_H
#define TRIGOBJVIEW_H

#include <cstddef>

#include "EventRange.h"
#include "Rtypes.h"
#include "TLorentzVector.h"
#include "ViewColumns.h"

// Struct-of-arrays backing storage for trigger objects.
struct TrigObjSoA {
    ColumnView<float> pt;
    ColumnView<float> eta;
    ColumnView<float> phi;
    ColumnView<unsigned short> id;
    ColumnView<ULong64_t> filterBits;
    ColumnView<short> l1charge;
    ColumnView<int> l1iso;
    ColumnView<float> l1pt;
    ColumnView<float> l1pt2;
    ColumnView<float> l2pt;

    int run = 2;

    std::size_t size() const { return pt.size(); }
};

class TrigObjView {
public:
    TrigObjView() = default;
    TrigObjView(const TrigObjSoA *storage, std::size_t index)
        : store(storage), idx(index) {}

    bool valid() const { return static_cast<bool>(store) && idx < store->size(); }

    float Pt() const { return store->pt[idx]; }
    float Eta() const { return store->eta[idx]; }
    float Phi() const { return store->phi[idx]; }
    unsigned short Id() const { return store->id[idx]; }
    ULong64_t FilterBits() const { return store->filterBits[idx]; }
    short L1Charge() const { return store->l1charge[idx]; }
    int L1Iso() const { return store->l1iso[idx]; }
    float L1Pt() const { return store->l1pt[idx]; }
    float L1Pt2() const { return store->l1pt2[idx]; }
    float L2Pt() const { return store->l2pt[idx]; }
    int Run() const { assertCurrentEvent(); return store->run; }

    TLorentzVector P4() const {
        TLorentzVector v;
        v.SetPtEtaPhiM(Pt(), Eta(), Phi(), 0.);
        return v;
    }

private:
    void assertCurrentEvent() const { static_cast<void>(store->size()); }
    const TrigObjSoA *store = nullptr;
    std::size_t idx = 0;
};

using TrigObjViewCollection = EventRange<TrigObjSoA, TrigObjView>;

#endif // TRIGOBJVIEW_H
