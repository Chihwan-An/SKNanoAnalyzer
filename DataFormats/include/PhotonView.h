#ifndef PHOTONVIEW_H
#define PHOTONVIEW_H

#include <cmath>
#include <cstddef>

#include "AnalysisException.h"
#include "EventRange.h"
#include "TLorentzVector.h"
#include "TString.h"
#include "ViewColumns.h"

struct PhotonSoA {
    ColumnView<float> pt;
    ColumnView<float> eta;
    ColumnView<float> phi;
    ColumnView<float> sieie;
    ColumnView<float> hoe;
    ColumnView<float> energyRaw;
    BoolColumnView pixelSeed;
    BoolColumnView isScEtaEB;
    BoolColumnView isScEtaEE;
    BoolColumnView mvaIdWP80;
    BoolColumnView mvaIdWP90;
    ColumnView<unsigned char> cutBased;
    ColumnView<float> mvaId;

    std::size_t size() const { return pt.size(); }
};

class PhotonView {
public:
    enum class EtaRegion { IB, OB, GAP, EC };
    enum class WorkingPoint { NONE, VETO, LOOSE, MEDIUM, TIGHT };

    PhotonView() = default;
    PhotonView(const PhotonSoA *storage, std::size_t index)
        : store_(storage), index_(index) {}

    bool valid() const { return store_ && index_ < store_->size(); }
    float Pt() const { return store_->pt[index_]; }
    float Eta() const { return store_->eta[index_]; }
    float Phi() const { return store_->phi[index_]; }
    float M() const { return 0.f; }
    float scEta() const { return Eta(); }
    float energy() const { return Pt() * std::cosh(Eta()); }
    float sieie() const { return store_->sieie[index_]; }
    float hoe() const { return store_->hoe[index_]; }
    float energyRaw() const {
        return store_->energyRaw.available() ? store_->energyRaw[index_] : 0.f;
    }
    bool pixelSeed() const { return store_->pixelSeed[index_]; }
    bool isScEtaEB() const { return store_->isScEtaEB[index_]; }
    bool isScEtaEE() const { return store_->isScEtaEE[index_]; }
    bool isMVAIDWP80() const { return store_->mvaIdWP80[index_]; }
    bool isMVAIDWP90() const { return store_->mvaIdWP90[index_]; }
    WorkingPoint CutBased() const {
        return static_cast<WorkingPoint>(store_->cutBased[index_]);
    }
    float MvaID() const { return store_->mvaId[index_]; }

    EtaRegion etaRegion() const {
        const float eta = std::abs(scEta());
        if (eta < 0.8f) return EtaRegion::IB;
        if (eta < 1.444f) return EtaRegion::OB;
        if (eta < 1.566f) return EtaRegion::GAP;
        return EtaRegion::EC;
    }

    bool PassID(const TString &id) const {
        if (etaRegion() == EtaRegion::GAP)
            return false;
        if (id == "POGVeto") return CutBased() == WorkingPoint::VETO;
        if (id == "POGLoose") return CutBased() == WorkingPoint::LOOSE;
        if (id == "POGMedium") return CutBased() == WorkingPoint::MEDIUM;
        if (id == "POGTight") return CutBased() == WorkingPoint::TIGHT;
        if (id == "POGMVAIDWP80") return isMVAIDWP80();
        if (id == "POGMVAIDWP90") return isMVAIDWP90();
        throw SKNano::LogicError("[PhotonView::PassID] " +
                                 std::string(id.Data()) +
                                 " is not implemented");
    }

    TLorentzVector P4() const {
        TLorentzVector value;
        value.SetPtEtaPhiM(Pt(), Eta(), Phi(), 0.f);
        return value;
    }

private:
    const PhotonSoA *store_ = nullptr;
    std::size_t index_ = 0;
};

using PhotonViewCollection = EventRange<PhotonSoA, PhotonView>;

#endif // PHOTONVIEW_H
