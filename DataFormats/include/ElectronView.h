#ifndef ELECTRONVIEW_H
#define ELECTRONVIEW_H

#include <cmath>
#include <cstddef>
#include <memory>
#include <vector>

#include "TLorentzVector.h"
#include "TString.h"
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

    std::vector<float> rho;
    std::vector<float> dEsigmaUp;
    std::vector<float> dEsigmaDown;
    std::vector<float> ecalPFClusterIso;
    std::vector<float> hcalPFClusterIso;
    std::vector<float> deltaEtaSeed;
    std::vector<float> deltaPhiSC;
    std::vector<float> deltaPhiSeed;

    std::size_t size() const { return pt.size(); }
};

class ElectronView {
public:
    enum class ElectronID {
        NOCUT,
        POG_VETO,
        POG_LOOSE,
        POG_MEDIUM,
        POG_TIGHT,
        POG_HEEP,
        POG_MVAISO_WP80,
        POG_MVAISO_WP90,
        POG_MVANOISO_WP80,
        POG_MVANOISO_WP90,
        HCTOWA_TIGHT,
        HCTOWA_LOOSE_RUN2,
        HCTOWA_LOOSE_RUN3
    };

    enum class CutBasedWP : unsigned char {
        NONE = 0,
        VETO = 1,
        LOOSE = 2,
        MEDIUM = 3,
        TIGHT = 4
    };

    enum class EtaRegion {
        IB,
        OB,
        GAP,
        EC
    };

    ElectronView() = default;
    ElectronView(std::shared_ptr<const ElectronSoA> storage, std::size_t index)
        : store(std::move(storage)), idx(index) {}

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
    unsigned char GenPartFlav() const { return store->genPartFlav[idx]; }
    short GenPartIdx() const { return store->genPartIdx[idx]; }
    short JetIdx() const { return store->jetIdx[idx]; }
    float ScEta() const { return store->scEta[idx]; }
    std::size_t rawIndex() const { return idx; }
    float deltaEtaInSC() const { return store->deltaEtaSC[idx]; }
    float deltaEtaInSeed() const { return idx < store->deltaEtaSeed.size() ? store->deltaEtaSeed[idx] : 0.f; }
    float deltaPhiInSC() const { return idx < store->deltaPhiSC.size() ? store->deltaPhiSC[idx] : 0.f; }
    float deltaPhiInSeed() const { return idx < store->deltaPhiSeed.size() ? store->deltaPhiSeed[idx] : 0.f; }
    float ecalPFClusterIso() const { return idx < store->ecalPFClusterIso.size() ? store->ecalPFClusterIso[idx] : -999.f; }
    float hcalPFClusterIso() const { return idx < store->hcalPFClusterIso.size() ? store->hcalPFClusterIso[idx] : -999.f; }
    float Rho() const { return idx < store->rho.size() ? store->rho[idx] : 0.f; }
    float dEsigmaUp() const { return idx < store->dEsigmaUp.size() ? store->dEsigmaUp[idx] : -999.f; }
    float dEsigmaDown() const { return idx < store->dEsigmaDown.size() ? store->dEsigmaDown[idx] : -999.f; }

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

    std::shared_ptr<const ElectronSoA> store;
    std::size_t idx = 0;
};

class ElectronViewCollection {
public:
    ElectronViewCollection() = default;
    explicit ElectronViewCollection(std::shared_ptr<ElectronSoA> storage, bool skipCrack = true);

    const ElectronView &operator[](std::size_t index) const { return views[index]; }
    std::size_t size() const { return views.size(); }
    bool empty() const { return views.empty(); }

    auto begin() const { return views.begin(); }
    auto end() const { return views.end(); }

    const std::shared_ptr<ElectronSoA> &storage() const { return storage_; }

private:
    std::shared_ptr<ElectronSoA> storage_;
    std::vector<ElectronView> views;
};

#endif // ELECTRONVIEW_H
