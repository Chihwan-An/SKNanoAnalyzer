#ifndef SVVIEW_H
#define SVVIEW_H

#include <cstddef>

#include "EventRange.h"
#include "TLorentzVector.h"
#include "ViewColumns.h"

struct SVSoA {
    ColumnView<float> pt;
    ColumnView<float> eta;
    ColumnView<float> phi;
    ColumnView<float> mass;
    ColumnView<short> charge;
    ColumnView<float> chi2;
    ColumnView<float> dlen;
    ColumnView<float> dlenSig;
    ColumnView<float> dxy;
    ColumnView<float> dxySig;
    ColumnView<float> ndof;
    ColumnView<unsigned char> ntracks;
    ColumnView<float> pAngle;
    ColumnView<float> x;
    ColumnView<float> y;
    ColumnView<float> z;

    std::size_t size() const { return pt.size(); }
};

class SVView {
public:
    SVView() = default;
    SVView(const SVSoA *storage, std::size_t index)
        : store(storage), idx(index) {}

    bool valid() const { return static_cast<bool>(store) && idx < store->size(); }

    float Pt() const { return store->pt[idx]; }
    float Eta() const { return store->eta[idx]; }
    float Phi() const { return store->phi[idx]; }
    float Mass() const { return store->mass[idx]; }
    short Charge() const { return store->charge[idx]; }
    float Chi2() const { return store->chi2[idx]; }
    float Dlen() const { return store->dlen[idx]; }
    float DlenSig() const { return store->dlenSig[idx]; }
    float Dxy() const { return store->dxy[idx]; }
    float DxySig() const { return store->dxySig[idx]; }
    float Ndof() const { return store->ndof[idx]; }
    unsigned char NTracks() const { return store->ntracks[idx]; }
    float PAngle() const { return store->pAngle[idx]; }
    float X() const { return store->x[idx]; }
    float Y() const { return store->y[idx]; }
    float Z() const { return store->z[idx]; }

    TLorentzVector P4() const {
        TLorentzVector v;
        v.SetPtEtaPhiM(Pt(), Eta(), Phi(), Mass());
        return v;
    }

private:
    const SVSoA *store = nullptr;
    std::size_t idx = 0;
};

using SVViewCollection = EventRange<SVSoA, SVView>;

#endif // SVVIEW_H
