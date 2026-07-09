#ifndef Electron_h
#define Electron_h

#include <bitset>
#include <memory>

#include "TString.h"
#include "Lepton.h"
#include "LeptonIDEnums.h"

class AnalyzerCore;

// Need update
// Missing variables (compared to SKFlat)
// energy / resolution corrections & errors
// ID variables: j_e2x5OverE5x5, j_e1x5OverE5x5, j_ecalPFClusterIso, j_hcalPFClusterIso, j_dr03HcalTowerSumEt
// SuperCluster: phi, E
// others: j_ea

class Electron : public Lepton {
public:
    Electron();
    ~Electron();

    enum class Property {
        PfRelIso03,
        MiniPFRelIso,
        Dxy,
        DxyErr,
        Dz,
        DzErr,
        Ip3d,
        Sip3d,
        ConvVeto,
        LostHits,
        SeedGain,
        TightCharge,
        Sieie,
        Hoe,
        EInvMinusPInv,
        Dr03EcalRecHitSumEt,
        Dr03HcalDepth1TowerSumEt,
        Dr03TkSumPt,
        Dr03TkSumPtHEEP,
        R9,
        EnergyErr,
        CutBasedHEEP,
        PromptMVA,
        MvaIsoWP80,
        MvaIsoWP90,
        MvaNoIsoWP80,
        MvaNoIsoWP90,
        MvaIso,
        MvaNoIso,
        CutBased,
        GenPartFlav,
        GenPartIdx,
        JetIdx,
        Count
    };

    using EnsureCallback = void (*)(void*, Electron&, Property);
    void AttachLazyPayload(void *context, EnsureCallback callback, int index) const;
    void DetachLazyPayload() const;
    bool HasLazyPayload() const { return static_cast<bool>(lazy_); }

    using ETAREGION = LeptonID::ElectronEtaRegion;
    inline ETAREGION etaRegion() const {
        if (fabs(scEta()) < 0.8)        return ETAREGION::IB;
        else if (fabs(scEta()) < 1.444) return ETAREGION::OB;
        else if (fabs(scEta()) < 1.566) return ETAREGION::GAP;
        else return ETAREGION::EC;
    }

    using ElectronID = LeptonID::ElectronID;

    void SetConvVeto(bool convVeto) { j_convVeto = convVeto; markLoaded(Property::ConvVeto); }
    inline bool ConvVeto() const { ensure(Property::ConvVeto); return j_convVeto; }

    void SetScEta(float scEta) { j_scEta = scEta; }
    inline float scEta() const { return j_scEta; }

    void SetDeltaEtaInSC(float deltaEtaInSC) { j_deltaEtaInSC = deltaEtaInSC; }
    inline float deltaEtaInSC() const { return j_deltaEtaInSC; }

    void SetDeltaPhiInSC(float deltaPhiInSC) { j_deltaPhiInSC = deltaPhiInSC; }
    inline float deltaPhiInSC() const { return j_deltaPhiInSC; }

    void SetDeltaEtaInSeed(float deltaEtaInSeed) { j_deltaEtaInSeed = deltaEtaInSeed; }
    inline float deltaEtaInSeed() const { return j_deltaEtaInSeed; }

    void SetDeltaPhiInSeed(float deltaPhiInSeed) { j_deltaPhiInSeed = deltaPhiInSeed; }
    inline float deltaPhiInSeed() const { return j_deltaPhiInSeed; }

    void SetLostHits(unsigned char lostHits) { j_lostHits = lostHits; markLoaded(Property::LostHits); }
    inline unsigned char LostHits() const { ensure(Property::LostHits); return j_lostHits; }

    void SetPFClusterIso(float ecalPFClusterIso, float hcalPFClusterIso) {
        j_ecalPFClusterIso = ecalPFClusterIso;
        j_hcalPFClusterIso = hcalPFClusterIso;
    }
    inline float ecalPFClusterIso() const { return j_ecalPFClusterIso; }
    inline float hcalPFClusterIso() const { return j_hcalPFClusterIso; }
    void SetSeedGain(unsigned char seedGain) { j_seedGain = seedGain; markLoaded(Property::SeedGain); }
    inline unsigned char SeedGain() const { ensure(Property::SeedGain); return j_seedGain; }

    void SetTightCharge(unsigned char tightCharge) { j_tightCharge = tightCharge; markLoaded(Property::TightCharge); }
    inline unsigned char TightCharge() const { ensure(Property::TightCharge); return j_tightCharge; }

    void SetSieie(float sieie) { j_sieie = sieie; markLoaded(Property::Sieie); }
    inline float sieie() const { ensure(Property::Sieie); return j_sieie; }

    void SetHoe(float hoe) { j_hoe = hoe; markLoaded(Property::Hoe); }
    inline float hoe() const { ensure(Property::Hoe); return j_hoe; }

    void SetEInvMinusPInv(float eInvMinusPInv) { j_eInvMinusPInv = eInvMinusPInv; markLoaded(Property::EInvMinusPInv); }
    inline float eInvMinusPInv() const { ensure(Property::EInvMinusPInv); return j_eInvMinusPInv; }

    void SetDr03EcalRecHitSumEt(float dr03EcalRecHitSumEt) { j_dr03EcalRecHitSumEt = dr03EcalRecHitSumEt; markLoaded(Property::Dr03EcalRecHitSumEt); }
    inline float dr03EcalRecHitSumEt() const { ensure(Property::Dr03EcalRecHitSumEt); return j_dr03EcalRecHitSumEt; }

    void SetDr03HcalDepth1TowerSumEt(float dr03HcalDepth1TowerSumEt) { j_dr03HcalDepth1TowerSumEt = dr03HcalDepth1TowerSumEt; markLoaded(Property::Dr03HcalDepth1TowerSumEt); }
    inline float dr03HcalDepth1TowerSumEt() const { ensure(Property::Dr03HcalDepth1TowerSumEt); return j_dr03HcalDepth1TowerSumEt; }

    void SetDr03TkSumPt(float dr03TkSumPt) { j_dr03TkSumPt = dr03TkSumPt; markLoaded(Property::Dr03TkSumPt); }
    inline float dr03TkSumPt() const { ensure(Property::Dr03TkSumPt); return j_dr03TkSumPt; }

    void SetDr03TkSumPtHEEP(float dr03TkSumPtHEEP) { j_dr03TkSumPtHEEP = dr03TkSumPtHEEP; markLoaded(Property::Dr03TkSumPtHEEP); }
    inline float dr03TkSumPtHEEP() const { ensure(Property::Dr03TkSumPtHEEP); return j_dr03TkSumPtHEEP; }

    void SetR9(float r9) { j_r9 = r9; markLoaded(Property::R9); }
    inline float r9() const { ensure(Property::R9); return j_r9; }

    void SetGenPartFlav(unsigned char genPartFlav) { j_genPartFlav = genPartFlav; markLoaded(Property::GenPartFlav); }
    inline unsigned char GenPartFlav() const { ensure(Property::GenPartFlav); return j_genPartFlav; }

    void SetGenPartIdx(short genPartIdx) { j_genPartIdx = genPartIdx; markLoaded(Property::GenPartIdx); }
    inline short GenPartIdx() const { ensure(Property::GenPartIdx); return j_genPartIdx; }

    void SetJetIdx(short jetIdx) { j_jetIdx = jetIdx; markLoaded(Property::JetIdx); }
    inline short JetIdx() const { ensure(Property::JetIdx); return j_jetIdx; }

    void SetRho(float rho) { j_rho = rho; }
    inline float rho() const { return j_rho; }

    void SetVidNestedWPBitmap(int bitmap) { j_vidNestedWPBitmap = bitmap; }
    inline int VidNestedWPBitmap() const { return j_vidNestedWPBitmap; }

    void SetVidNestedWPBitmapHEEP(int bitmap) { j_vidNestedWPBitmapHEEP = bitmap; }
    inline int VidNestedWPBitmapHEEP() const { return j_vidNestedWPBitmapHEEP; }

    void SetScEtOverPt(float scEtOverPt) { j_scEtOverPt = scEtOverPt; }
    inline float scEtOverPt() const { return j_scEtOverPt; }

    void SetEnergyErr(float energyErr) { j_energyErr = energyErr; markLoaded(Property::EnergyErr); }
    inline float energyErr() const { ensure(Property::EnergyErr); return j_energyErr; }

    void SetEnergyResUnc(float dEsigmaUp, float dEsigmaDown) {
        j_dEsigmaUp = dEsigmaUp;
        j_dEsigmaDown = dEsigmaDown;
    }
    inline float dEsigmaUp() const { return j_dEsigmaUp; }
    inline float dEsigmaDown() const { return j_dEsigmaDown; }

    // Boolean IDs
    enum class BooleanID {NONE, MVAISOWP80, MVAISOWP90, MVAISOWPL, MVANOISOWP80, MVANOISOWP90, MVANOISOWPL, CUTBASEDHEEP};
    void SetBIDBit(BooleanID id, bool idbit);
    inline bool isMVAIsoWP80() const { ensure(Property::MvaIsoWP80); return j_mvaIso_WP80; }
    inline bool isMVAIsoWP90() const { ensure(Property::MvaIsoWP90); return j_mvaIso_WP90; }
    inline bool isMVAIsoWPLoose() const { return j_mvaIso_WPL; }
    inline bool isMVANoIsoWP80() const { ensure(Property::MvaNoIsoWP80); return j_mvaNoIso_WP80; }
    inline bool isMVANoIsoWP90() const { ensure(Property::MvaNoIsoWP90); return j_mvaNoIso_WP90; }
    inline bool isMVANoIsoWPLoose() const { return j_mvaNoIso_WPL; }
    inline bool isCutBasedHEEP() const { ensure(Property::CutBasedHEEP); return j_cutBased_HEEP; }


    // cut-based ID
    enum class CutBasedID {NONE, CUTBASED};
    using WORKINGPOINT = LeptonID::ElectronCutBasedWP;
    void SetCBIDBit(CutBasedID id, unsigned int value);
    inline WORKINGPOINT CutBased() const { ensure(Property::CutBased); return static_cast<WORKINGPOINT>(j_cutBased); }

    // Private IDs
    bool Pass_CaloIdL_TrackIdL_IsoVL() const;
    bool Pass_HcToWABaseline() const;
    bool Pass_HcToWALooseRun2() const;
    bool Pass_HcToWALooseRun3() const;
    bool Pass_HcToWATight() const;
    // MVA scores
    enum class MVATYPE {NONE, MVAISO, MVANOISO, MVAPROMPT};
    void SetMVA(MVATYPE type, float score);
    inline float MvaIso() const { ensure(Property::MvaIso); return j_mvaIso; }
    inline float MvaNoIso() const { ensure(Property::MvaNoIso); return j_mvaNoIso; }
    inline float MvaTTH() const { ensure(Property::PromptMVA); return j_mvaPrompt; }

    // ID helper functions
    bool PassID(const TString ID) const;
    bool PassID(ElectronID ID) const;

    float PfRelIso03() const { ensure(Property::PfRelIso03); return Lepton::PfRelIso03(); }
    float MiniPFRelIso() const { ensure(Property::MiniPFRelIso); return Lepton::MiniPFRelIso(); }
    float dXY() const { ensure(Property::Dxy); return Lepton::dXY(); }
    float dXYerr() const { ensure(Property::DxyErr); return Lepton::dXYerr(); }
    float dZ() const { ensure(Property::Dz); return Lepton::dZ(); }
    float dZerr() const { ensure(Property::DzErr); return Lepton::dZerr(); }
    float IP3D() const { ensure(Property::Ip3d); return Lepton::IP3D(); }
    float SIP3D() const { ensure(Property::Sip3d); return Lepton::SIP3D(); }

private:
    void ensure(Property property) const;
    void markLoaded(Property property) const;

    struct LazyPayload {
        void *context = nullptr;
        EnsureCallback callback = nullptr;
        int index = -1;
        mutable std::bitset<static_cast<std::size_t>(Property::Count)> loaded;
    };

    mutable std::bitset<static_cast<std::size_t>(Property::Count)> cachedProperties_;
    mutable std::shared_ptr<LazyPayload> lazy_;

    // uncertainties
    float j_energyErr; 
    float j_dEsigmaUp, j_dEsigmaDown;
    // ID variables
    bool j_convVeto;
    unsigned char j_lostHits, j_seedGain, j_tightCharge;
    float j_sieie, j_hoe, j_eInvMinusPInv;
    float j_dr03EcalRecHitSumEt, j_dr03HcalDepth1TowerSumEt, j_dr03TkSumPt, j_dr03TkSumPtHEEP; // no j_e2x5OverE5x5, j_e1x5OverE5x5, j_calPFClusterIso, j_hcalPFClusterIso, j_dr03HcalTowerSumEt

    // SuperCluster
    float j_scEta;
    float j_deltaEtaInSC, j_deltaEtaInSeed, j_deltaPhiInSC, j_deltaPhiInSeed; // missing phi and E?
    float j_ecalPFClusterIso, j_hcalPFClusterIso;

    // IDs
    bool j_mvaIso_WP80, j_mvaIso_WP90, j_mvaIso_WPL, j_mvaNoIso_WP80, j_mvaNoIso_WP90, j_mvaNoIso_WPL, j_cutBased_HEEP;
    unsigned char j_cutBased;
    
    float j_mvaIso, j_mvaNoIso, j_mvaPrompt;

    // others
    float j_r9;
    float j_rho;
    int j_vidNestedWPBitmap;
    int j_vidNestedWPBitmapHEEP;
    float j_scEtOverPt;
    short j_genPartIdx;
    unsigned char j_genPartFlav;
    short j_jetIdx;
    friend class AnalyzerCore;
    ClassDef(Electron, 1);
};

#endif
