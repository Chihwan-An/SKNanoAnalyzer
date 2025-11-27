#ifndef GENJETVIEW_H
#define GENJETVIEW_H

#include <cstddef>
#include <memory>
#include <vector>

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
    GenJetView(std::shared_ptr<const GenJetSoA> storage, std::size_t index)
        : store(std::move(storage)), idx(index) {}

    bool valid() const { return static_cast<bool>(store) && idx < store->size(); }

    float Pt() const { return store->pt[idx]; }
    float Eta() const { return store->eta[idx]; }
    float Phi() const { return store->phi[idx]; }
    float Mass() const { return store->mass[idx]; }
    short PartonFlavour() const { return store->partonFlavour[idx]; }
    unsigned char HadronFlavour() const { return store->hadronFlavour[idx]; }

    TLorentzVector P4() const {
        TLorentzVector v;
        v.SetPtEtaPhiM(Pt(), Eta(), Phi(), Mass());
        return v;
    }

private:
    std::shared_ptr<const GenJetSoA> store;
    std::size_t idx = 0;
};

class GenJetViewCollection {
public:
    GenJetViewCollection() = default;
    explicit GenJetViewCollection(std::shared_ptr<GenJetSoA> payload)
        : storage_(std::move(payload)) {
        if (storage_) {
            const std::size_t n = storage_->size();
            views.reserve(n);
            for (std::size_t i = 0; i < n; ++i) {
                views.emplace_back(storage_, i);
            }
        }
    }

    const GenJetView &operator[](std::size_t index) const { return views[index]; }
    std::size_t size() const { return views.size(); }
    bool empty() const { return views.empty(); }

    auto begin() const { return views.begin(); }
    auto end() const { return views.end(); }

    const std::shared_ptr<GenJetSoA> &storage() const { return storage_; }

private:
    std::shared_ptr<GenJetSoA> storage_;
    std::vector<GenJetView> views;
};

#endif // GENJETVIEW_H
