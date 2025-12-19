#ifndef TRIGOBJVIEW_H
#define TRIGOBJVIEW_H

#include <cstddef>
#include <memory>
#include <vector>

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
    TrigObjView(std::shared_ptr<const TrigObjSoA> storage, std::size_t index)
        : store(std::move(storage)), idx(index) {}

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
    int Run() const { return store->run; }

    TLorentzVector P4() const {
        TLorentzVector v;
        v.SetPtEtaPhiM(Pt(), Eta(), Phi(), 0.);
        return v;
    }

private:
    std::shared_ptr<const TrigObjSoA> store;
    std::size_t idx = 0;
};

class TrigObjViewCollection {
public:
    TrigObjViewCollection() = default;
    explicit TrigObjViewCollection(std::shared_ptr<TrigObjSoA> payload)
        : storage_(std::move(payload)) {
        if (storage_) {
            const std::size_t n = storage_->size();
            views.reserve(n);
            for (std::size_t i = 0; i < n; ++i) {
                views.emplace_back(storage_, i);
            }
        }
    }

    const TrigObjView &operator[](std::size_t index) const { return views[index]; }
    std::size_t size() const { return views.size(); }
    bool empty() const { return views.empty(); }

    auto begin() const { return views.begin(); }
    auto end() const { return views.end(); }

    const std::shared_ptr<TrigObjSoA> &storage() const { return storage_; }

private:
    std::shared_ptr<TrigObjSoA> storage_;
    std::vector<TrigObjView> views;
};

#endif // TRIGOBJVIEW_H
