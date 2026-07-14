#ifndef GENVIEW_H
#define GENVIEW_H

#include <cstddef>

#include "EventRange.h"
#include "TLorentzVector.h"
#include "ViewColumns.h"

struct GenSoA {
    ColumnView<float> pt;
    ColumnView<float> eta;
    ColumnView<float> phi;
    ColumnView<float> mass;
    ColumnView<int> pdgId;
    ColumnView<int> status;
    ColumnView<short> motherIdx;
    ColumnView<unsigned short> statusFlags;

    std::size_t size() const { return pt.size(); }
};

class GenView {
public:
    GenView() = default;
    GenView(const GenSoA *storage, std::size_t index)
        : store(storage), idx(index) {}

    bool valid() const { return static_cast<bool>(store) && idx < store->size(); }

    float Pt() const { return store->pt[idx]; }
    float Eta() const { return store->eta[idx]; }
    float Phi() const { return store->phi[idx]; }
    float Mass() const { return store->mass[idx]; }
    int PdgId() const { return store->pdgId[idx]; }
    int PID() const { return PdgId(); }
    int Status() const { return store->status[idx]; }
    int MotherIndex() const { return static_cast<int>(store->motherIdx[idx]); }
    unsigned short StatusFlags() const { return store->statusFlags[idx]; }
    bool isPrompt() const { return StatusFlags() & (1U << 0); }
    bool isDecayedLeptonHadron() const { return StatusFlags() & (1U << 1); }
    bool isTauDecayProduct() const { return StatusFlags() & (1U << 2); }
    bool isPromptTauDecayProduct() const { return StatusFlags() & (1U << 3); }
    bool isDirectTauDecayProduct() const { return StatusFlags() & (1U << 4); }
    bool isDirectPromptTauDecayProduct() const { return StatusFlags() & (1U << 5); }
    bool isDirectHadronDecayProduct() const { return StatusFlags() & (1U << 6); }
    bool isHardProcess() const { return StatusFlags() & (1U << 7); }
    bool fromHardProcess() const { return StatusFlags() & (1U << 8); }
    bool isHardProcessTauDecayProduct() const { return StatusFlags() & (1U << 9); }
    bool isDirectHardProcessTauDecayProduct() const { return StatusFlags() & (1U << 10); }
    bool fromHardProcessBeforeFSR() const { return StatusFlags() & (1U << 11); }
    bool isFirstCopy() const { return StatusFlags() & (1U << 12); }
    bool isLastCopy() const { return StatusFlags() & (1U << 13); }
    bool isLastCopyBeforeFSR() const { return StatusFlags() & (1U << 14); }

    TLorentzVector P4() const {
        TLorentzVector v;
        v.SetPtEtaPhiM(Pt(), Eta(), Phi(), Mass());
        return v;
    }
    float DeltaR(const GenView &other) const { return P4().DeltaR(other.P4()); }

private:
    const GenSoA *store = nullptr;
    std::size_t idx = 0;
};

using GenViewCollection = EventRange<GenSoA, GenView>;

#endif // GENVIEW_H
