#ifndef ELECTRONVIEW_H
#define ELECTRONVIEW_H

#include <cmath>
#include <cstddef>
#include <functional>
#include <stdexcept>
#include <vector>

#include "EventRange.h"
#include "TLorentzVector.h"
#include "TString.h"
#include "LeptonIDEnums.h"
#include "ViewColumns.h"

/**
 * Lightweight view over nanoAOD electron branches.
 * Mirrors the pieces of the full Electron interface that are
 * required for fast filtering without materialising Electron objects.
 */

struct ElectronSoA {
    ColumnView<float> pt;
    ColumnView<float> eta;
    ColumnView<float> phi;
    ColumnView<float> mass;
    ColumnView<int> charge;
    ColumnView<float> pfRelIso03;
    ColumnView<float> miniPFRelIso;
    ColumnView<float> dxy;
    ColumnView<float> dxyErr;
    ColumnView<float> dz;
    ColumnView<float> dzErr;
    ColumnView<float> ip3d;
    ColumnView<float> sip3d;
    BoolColumnView convVeto;
    ColumnView<unsigned char> lostHits;
    ColumnView<unsigned char> seedGain;
    ColumnView<unsigned char> tightCharge;
    ColumnView<float> sieie;
    ColumnView<float> hoe;
    ColumnView<float> eInvMinusPInv;
    ColumnView<float> dr03EcalRecHitSumEt;
    ColumnView<float> dr03HcalDepth1TowerSumEt;
    ColumnView<float> dr03TkSumPt;
    ColumnView<float> dr03TkSumPtHEEP;
    ColumnView<float> r9;
    ColumnView<float> energyErr;
    BoolColumnView cutBasedHEEP;
    ColumnView<float> promptMVA;
    BoolColumnView mvaIsoWP80;
    BoolColumnView mvaIsoWP90;
    BoolColumnView mvaNoIsoWP80;
    BoolColumnView mvaNoIsoWP90;
    ColumnView<float> mvaIso;
    ColumnView<float> mvaNoIso;
    ColumnView<unsigned char> cutBased;
    ColumnView<unsigned char> genPartFlav;
    ColumnView<short> genPartIdx;
    ColumnView<short> jetIdx;
    ColumnView<float> scEta;
    ColumnView<float> deltaEtaSC;

    std::function<float()> readRho;
    mutable float rho = 0.f;
    mutable bool rhoReady = false;
    mutable bool rhoComputing = false;

    std::size_t size() const { return pt.size(); }

    float getRho() const {
        static_cast<void>(pt.size());
        if (rhoReady)
            return rho;
        if (rhoComputing)
            throw std::logic_error("recursive Electron rho population");
        if (!readRho)
            return 0.f;
        rhoComputing = true;
        try {
            rho = readRho();
            rhoReady = true;
            rhoComputing = false;
            return rho;
        } catch (...) {
            rhoComputing = false;
            throw;
        }
    }
};

class ElectronView {
public:
    using ElectronID = LeptonID::ElectronID;
    using CutBasedWP = LeptonID::ElectronCutBasedWP;
    using EtaRegion = LeptonID::ElectronEtaRegion;

    ElectronView() = default;
    ElectronView(const ElectronSoA *storage, std::size_t index)
        : store(storage), idx(index) {}

    bool valid() const { return static_cast<bool>(store) && idx < store->size(); }

    float Pt() const { return store->pt[idx]; }
    float Eta() const { return store->eta[idx]; }
    float Phi() const { return store->phi[idx]; }
    float M() const { return store->mass[idx]; }
    int Charge() const { return store->charge[idx]; }
    float PfRelIso03() const { return store->pfRelIso03[idx]; }
    float MiniPFRelIso() const { return store->miniPFRelIso[idx]; }
    float dXY() const { return store->dxy[idx]; }
    float dXYerr() const { return store->dxyErr[idx]; }
    float dZ() const { return store->dz[idx]; }
    float dZerr() const { return store->dzErr[idx]; }
    float IP3D() const { return store->ip3d[idx]; }
    float SIP3D() const { return store->sip3d[idx]; }
    bool ConvVeto() const { return store->convVeto[idx]; }
    unsigned char LostHits() const { return store->lostHits[idx]; }
    unsigned char SeedGain() const { return store->seedGain[idx]; }
    unsigned char TightCharge() const { return store->tightCharge[idx]; }
    float sieie() const { return store->sieie[idx]; }
    float hoe() const { return store->hoe[idx]; }
    float eInvMinusPInv() const { return store->eInvMinusPInv[idx]; }
    float dr03EcalRecHitSumEt() const { return store->dr03EcalRecHitSumEt[idx]; }
    float dr03HcalDepth1TowerSumEt() const { return store->dr03HcalDepth1TowerSumEt[idx]; }
    float dr03TkSumPt() const { return store->dr03TkSumPt[idx]; }
    float dr03TkSumPtHEEP() const { return store->dr03TkSumPtHEEP[idx]; }
    float r9() const { return store->r9[idx]; }
    float energyErr() const { return store->energyErr[idx]; }
    bool isCutBasedHEEP() const { return store->cutBasedHEEP[idx]; }
    float PromptMVA() const { return store->promptMVA[idx]; }
    bool isMVAIsoWP80() const { return store->mvaIsoWP80[idx]; }
    bool isMVAIsoWP90() const { return store->mvaIsoWP90[idx]; }
    bool isMVANoIsoWP80() const { return store->mvaNoIsoWP80[idx]; }
    bool isMVANoIsoWP90() const { return store->mvaNoIsoWP90[idx]; }
    float MvaIso() const { return store->mvaIso[idx]; }
    float MvaNoIso() const { return store->mvaNoIso[idx]; }
    CutBasedWP CutBased() const { return static_cast<CutBasedWP>(store->cutBased[idx]); }
    unsigned char GenPartFlav() const {
        return store->genPartFlav.available() ? store->genPartFlav[idx] : 0;
    }
    short GenPartIdx() const {
        return store->genPartIdx.available() ? store->genPartIdx[idx] : -1;
    }
    short JetIdx() const { return store->jetIdx[idx]; }
    float ScEta() const { return store->scEta[idx]; }
    std::size_t rawIndex() const { return idx; }
    float deltaEtaInSC() const { return store->deltaEtaSC[idx]; }
    float deltaEtaInSeed() const { assertCurrentEvent(); return -999.f; }
    float deltaPhiInSC() const { assertCurrentEvent(); return -999.f; }
    float deltaPhiInSeed() const { assertCurrentEvent(); return -999.f; }
    float ecalPFClusterIso() const { assertCurrentEvent(); return -999.f; }
    float hcalPFClusterIso() const { assertCurrentEvent(); return -999.f; }
    float Rho() const { assertCurrentEvent(); return store->getRho(); }
    float dEsigmaUp() const { assertCurrentEvent(); return -999.f; }
    float dEsigmaDown() const { assertCurrentEvent(); return -999.f; }

    EtaRegion etaRegion() const;

    TLorentzVector P4() const {
        TLorentzVector v;
        v.SetPtEtaPhiM(Pt(), Eta(), Phi(), M());
        return v;
    }

    bool PassID(ElectronID id) const;
    bool PassID(const TString &id) const;

private:
    bool Pass_CaloIdL_TrackIdL_IsoVL() const;
    bool Pass_HcToWABaseline() const;
    bool Pass_HcToWATight() const;
    bool Pass_HcToWALooseRun2() const;
    bool Pass_HcToWALooseRun3() const;

    void assertCurrentEvent() const { static_cast<void>(store->size()); }
    const ElectronSoA *store = nullptr;
    std::size_t idx = 0;
};

class ElectronViewCollection : public EventRange<ElectronSoA, ElectronView> {
public:
    using Base = EventRange<ElectronSoA, ElectronView>;
    using Base::Base;
    ElectronViewCollection() = default;
    explicit ElectronViewCollection(std::shared_ptr<ElectronSoA> storage,
                                    bool skipCrack);
};

#endif // ELECTRONVIEW_H
