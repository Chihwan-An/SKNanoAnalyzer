#ifndef GENJETVIEW_H
#define GENJETVIEW_H

#include <cstddef>

#include "EventRange.h"
#include "TLorentzVector.h"
#include "ViewColumns.h"

struct GenJetSoA {
    ColumnView<float> pt;
    ColumnView<float> eta;
    ColumnView<float> phi;
    ColumnView<float> mass;
    ColumnView<short> partonFlavour;
    ColumnView<unsigned char> hadronFlavour;

    std::size_t size() const { return pt.size(); }
};

class GenJetView {
public:
    GenJetView() = default;
    GenJetView(const GenJetSoA *storage, std::size_t index)
        : store(storage), idx(index) {}

    bool valid() const { return static_cast<bool>(store) && idx < store->size(); }

    float Pt() const { return store->pt[idx]; }
    float Eta() const { return store->eta[idx]; }
    float Phi() const { return store->phi[idx]; }
    float Mass() const { return store->mass[idx]; }
    short PartonFlavour() const { return store->partonFlavour[idx]; }
    short partonFlavour() const { return PartonFlavour(); }
    unsigned char HadronFlavour() const { return store->hadronFlavour[idx]; }
    unsigned char hadronFlavour() const { return HadronFlavour(); }

    TLorentzVector P4() const {
        TLorentzVector v;
        v.SetPtEtaPhiM(Pt(), Eta(), Phi(), Mass());
        return v;
    }
    template <typename Other>
    float DeltaR(const Other &other) const { return P4().DeltaR(other.P4()); }

private:
    const GenJetSoA *store = nullptr;
    std::size_t idx = 0;
};

using GenJetViewCollection = EventRange<GenJetSoA, GenJetView>;

#endif // GENJETVIEW_H
