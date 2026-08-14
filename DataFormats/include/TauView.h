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
    ColumnView<float> rawDeepTau2018v2p5VSe;
    ColumnView<float> rawDeepTau2018v2p5VSjet;
    ColumnView<float> rawDeepTau2018v2p5VSmu;

    std::size_t size() const { return pt.size(); }
};

enum class TauWP : unsigned char {
    None = 0, VVVLoose = 1, VVLoose = 2, VLoose = 3, Loose = 4,
    Medium = 5, Tight = 6, VTight = 7, VVTight = 8
};

// vsMu discriminator has only four WPs
enum class TauWPvsMu : unsigned char {
    None = 0, VLoose = 1, Loose = 2, Medium = 3, Tight = 4
};

inline const char *ToCorrectionString(TauWP wp) {
    switch (wp) {
    case TauWP::VVVLoose: return "VVVLoose";
    case TauWP::VVLoose:  return "VVLoose";
    case TauWP::VLoose:   return "VLoose";
    case TauWP::Loose:    return "Loose";
    case TauWP::Medium:   return "Medium";
    case TauWP::Tight:    return "Tight";
    case TauWP::VTight:   return "VTight";
    case TauWP::VVTight:  return "VVTight";
    case TauWP::None:     return "None";
    }
    return "None";
}

inline const char *ToCorrectionString(TauWPvsMu wp) {
    switch (wp) {
    case TauWPvsMu::VLoose: return "VLoose";
    case TauWPvsMu::Loose:  return "Loose";
    case TauWPvsMu::Medium: return "Medium";
    case TauWPvsMu::Tight:  return "Tight";
    case TauWPvsMu::None:   return "None";
    }
    return "None";
}

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

    // Raw DeepTau discriminator outputs. Return -1 when the column is not
    // present, following the GenPartIdx() convention above.
    float RawVsJet() const {
        return store_->rawDeepTau2018v2p5VSjet.available()
                   ? store_->rawDeepTau2018v2p5VSjet[index_]
                   : -1.f;
    }
    float RawVsE() const {
        return store_->rawDeepTau2018v2p5VSe.available()
                   ? store_->rawDeepTau2018v2p5VSe[index_]
                   : -1.f;
    }
    float RawVsMu() const {
        return store_->rawDeepTau2018v2p5VSmu.available()
                   ? store_->rawDeepTau2018v2p5VSmu[index_]
                   : -1.f;
    }

    bool passVVVLIDvJet() const { return jetId() >= 1; }
    bool passVVLIDvJet() const { return jetId() >= 2; }
    bool passVLIDvJet() const { return jetId() >= 3; }
    bool passLIDvJet() const { return jetId() >= 4; }
    bool passMIDvJet() const { return jetId() >= 5; }
    bool passTIDvJet() const { return jetId() >= 6; }
    bool passVTIDvJet() const { return jetId() >= 7; }
    bool passVVTIDvJet() const { return jetId() >= 8; }
    bool passVVVLIDvEl() const { return electronId() >= 1; }
    bool passVVLIDvEl() const { return electronId() >= 2; }
    bool passVLIDvEl() const { return electronId() >= 3; }
    bool passLIDvEl() const { return electronId() >= 4; }
    bool passMIDvEl() const { return electronId() >= 5; }
    bool passTIDvEl() const { return electronId() >= 6; }
    bool passVTIDvEl() const { return electronId() >= 7; }
    bool passVVTIDvEl() const { return electronId() >= 8; }
    bool passVLIDvMu() const { return muonId() >= 1; }
    bool passLIDvMu() const { return muonId() >= 2; }
    bool passMIDvMu() const { return muonId() >= 3; }
    bool passTIDvMu() const { return muonId() >= 4; }

    struct ID {
        TauWP vsJet = TauWP::None;
        TauWP vsE = TauWP::None;
        TauWPvsMu vsMu = TauWPvsMu::None;
        bool requireNewDM = true;
        float maxDz = 0.2f;
    };

    bool PassID(const ID &id) const {
        if (id.requireNewDM && !idDecayModeNewDMs())
            return false;
        if (std::abs(dZ()) >= id.maxDz)
            return false;
        if (jetId() < static_cast<unsigned char>(id.vsJet))
            return false;
        if (electronId() < static_cast<unsigned char>(id.vsE))
            return false;
        if (muonId() < static_cast<unsigned char>(id.vsMu))
            return false;
        return true;
    }

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

#endif
