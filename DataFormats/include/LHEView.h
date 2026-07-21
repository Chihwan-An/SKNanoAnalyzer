#ifndef LHEVIEW_H
#define LHEVIEW_H

#include <cstddef>

#include "EventRange.h"
#include "TLorentzVector.h"
#include "ViewColumns.h"

struct LHESoA {
    ColumnView<float> pt;
    ColumnView<float> eta;
    ColumnView<float> phi;
    ColumnView<float> mass;
    ColumnView<int> status;
    ColumnView<int> spin;
    ColumnView<float> incomingPz;
    ColumnView<int> pdgId;

    std::size_t size() const { return pt.size(); }
};

class LHEView {
public:
    LHEView() = default;
    LHEView(const LHESoA *storage, std::size_t index)
        : store_(storage), index_(index) {}

    bool valid() const { return store_ && index_ < store_->size(); }
    float Pt() const { return store_->pt[index_]; }
    float Eta() const { return store_->eta[index_]; }
    float Phi() const { return store_->phi[index_]; }
    float M() const { return store_->mass[index_]; }
    float Mass() const { return M(); }
    int Status() const { return store_->status[index_]; }
    int Spin() const { return store_->spin[index_]; }
    float IncomingPz() const { return store_->incomingPz[index_]; }
    int PdgId() const { return store_->pdgId[index_]; }
    bool IsEmpty() const { return false; }

    TLorentzVector P4() const {
        TLorentzVector value;
        value.SetPtEtaPhiM(Pt(), Eta(), Phi(), M());
        return value;
    }

private:
    const LHESoA *store_ = nullptr;
    std::size_t index_ = 0;
};

using LHEViewCollection = EventRange<LHESoA, LHEView>;

#endif // LHEVIEW_H
