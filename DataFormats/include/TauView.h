#ifndef TAUVIEW_H
#define TAUVIEW_H

#include <cmath>
#include <cstddef>

#include "EventRange.h"
#include "TLorentzVector.h"
#include "TString.h"
#include "ViewColumns.h"

struct TauSoA {
    ColumnView<float> pt;
    ColumnView<float> eta;
    ColumnView<float> phi;
    ColumnView<float> mass;
    ColumnView<float> dxy;
    ColumnView<float> dz;
    ColumnView<short> charge;
    ColumnView<unsigned char> decayMode;
    ColumnView<unsigned char> genPartFlav;
    ColumnView<short> genPartIdx;
    BoolColumnView idDecayModeNewDMs;
    ColumnView<unsigned char> idDeepTau2018v2p5VSe;
    ColumnView<unsigned char> idDeepTau2018v2p5VSjet;
    ColumnView<unsigned char> idDeepTau2018v2p5VSmu;

    std::size_t size() const { return pt.size(); }
};

class TauView {
public:
    TauView() = default;
    TauView(const TauSoA *storage, std::size_t index)
        : store_(storage), index_(index) {}

    bool valid() const { return store_ && index_ < store_->size(); }
    float Pt() const { return store_->pt[index_]; }
    float Eta() const { return store_->eta[index_]; }
    float Phi() const { return store_->phi[index_]; }
    float M() const { return store_->mass[index_]; }
    float dXY() const { return store_->dxy[index_]; }
    float dZ() const { return store_->dz[index_]; }
    short Charge() const { return store_->charge[index_]; }
    unsigned char DecayMode() const { return store_->decayMode[index_]; }
    unsigned char GenPartFlav() const { return store_->genPartFlav[index_]; }
    short GenPartIdx() const {
        return store_->genPartIdx.available() ? store_->genPartIdx[index_] : -1;
    }
    bool idDecayModeNewDMs() const { return store_->idDecayModeNewDMs[index_]; }

    bool passVVVLIDvJet() const { return jetId() == 1; }
    bool passVVLIDvJet() const { return jetId() == 2; }
    bool passVLIDvJet() const { return jetId() == 3; }
    bool passLIDvJet() const { return jetId() == 4; }
    bool passMIDvJet() const { return jetId() == 5; }
    bool passTIDvJet() const { return jetId() == 6; }
    bool passVTIDvJet() const { return jetId() == 7; }
    bool passVVTIDvJet() const { return jetId() == 8; }
    bool passVVVLIDvEl() const { return electronId() == 1; }
    bool passVVLIDvEl() const { return electronId() == 2; }
    bool passVLIDvEl() const { return electronId() == 3; }
    bool passLIDvEl() const { return electronId() == 4; }
    bool passMIDvEl() const { return electronId() == 5; }
    bool passTIDvEl() const { return electronId() == 6; }
    bool passVTIDvEl() const { return electronId() == 7; }
    bool passVVTIDvEl() const { return electronId() == 8; }
    bool passVLIDvMu() const { return muonId() == 1; }
    bool passLIDvMu() const { return muonId() == 2; }
    bool passMIDvMu() const { return muonId() == 3; }
    bool passTIDvMu() const { return muonId() == 4; }

    bool PassID(const TString &id) const {
        if (id == "NoCut")
            return true;
        if (id == "TestID")
            return idDecayModeNewDMs() && std::abs(dZ()) < 0.2f &&
                   passTIDvEl() && passTIDvJet() && passTIDvMu();
        return false;
    }

    TLorentzVector P4() const {
        TLorentzVector value;
        value.SetPtEtaPhiM(Pt(), Eta(), Phi(), M());
        return value;
    }

private:
    unsigned char jetId() const { return store_->idDeepTau2018v2p5VSjet[index_]; }
    unsigned char electronId() const { return store_->idDeepTau2018v2p5VSe[index_]; }
    unsigned char muonId() const { return store_->idDeepTau2018v2p5VSmu[index_]; }

    const TauSoA *store_ = nullptr;
    std::size_t index_ = 0;
};

using TauViewCollection = EventRange<TauSoA, TauView>;

#endif // TAUVIEW_H
