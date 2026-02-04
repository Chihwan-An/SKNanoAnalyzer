#ifndef SVVIEW_H
#define SVVIEW_H

#include <cstddef>
#include <memory>
#include <vector>

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
    SVView(std::shared_ptr<const SVSoA> storage, std::size_t index)
        : store(std::move(storage)), idx(index) {}

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
    std::shared_ptr<const SVSoA> store;
    std::size_t idx = 0;
};

class SVViewCollection {
public:
    SVViewCollection() = default;
    explicit SVViewCollection(std::shared_ptr<SVSoA> storage)
        : payload(std::move(storage)) {
        if (payload) {
            const std::size_t n = payload->size();
            views.reserve(n);
            for (std::size_t i = 0; i < n; ++i) {
                views.emplace_back(payload, i);
            }
        }
    }

    const SVView &operator[](std::size_t index) const { return views[index]; }
    std::size_t size() const { return views.size(); }
    bool empty() const { return views.empty(); }

    auto begin() const { return views.begin(); }
    auto end() const { return views.end(); }

    const std::shared_ptr<SVSoA> &storage() const { return payload; }

private:
    std::shared_ptr<SVSoA> payload;
    std::vector<SVView> views;
};

#endif // SVVIEW_H
