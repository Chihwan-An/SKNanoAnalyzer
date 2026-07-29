#ifndef HNWR_miniiso_scan_h
#define HNWR_miniiso_scan_h

#include "AnalyzerCore.h"
#include "SystematicHelper.h"
#include "Muon.h"
#include "Electron.h"
#include "LHE.h"

class HNWR_miniiso_scan : public AnalyzerCore {
public:
    HNWR_miniiso_scan();
    ~HNWR_miniiso_scan();

    void initializeAnalyzer();
    void executeEvent();
    void executeEventFromParameter();
    bool RunSyst;
    bool RunWRCut;
    // DY correction userflags, for the before/after figures in
    // fig/05_backgrounds/forAN/. Neither flag set is the production default:
    // C(gen Z pT) and R(reco jet pT) both on.
    //   NoDYCorr  both off        -> "before"
    //   ZptOnly   C on, R off     -> "after"
    // The two are mutually exclusive; setting both is a fatal error rather than
    // a silent precedence rule, so a mislabelled output directory cannot happen.
    bool NoDYCorr = false;
    bool ZptOnly = false;
    // Remove AK8 jets overlapping (dR < 0.4) any tight lepton.
    // HNWRAnalyzer does NOT do this, so it is off by default; flip to true to restore.
    bool CleanFatjetWithTightLeptons = false;
    unique_ptr<SystematicHelper> systHelper;
    
    
    struct flags{
        bool RunFake;
        bool RunCF ;
        bool RunSyst ;
        bool PromptLeptononly ;
        bool ApplyDYPtReweight ;
        bool ApplyDYReshape ;
        bool RunXsecSyst ;
        bool Signal ;
        bool CalculateAverageKFactor ;
        bool SignalElectronOnly ;
        bool SignalMuonOnly ;
        bool UseJetPtRwg ;
        bool UseDYCR1Reshape ;
    } flag;

    struct parameters{
        std::map<std::string ,float> Zpt;

        parameters(){
            Zpt["ZPTReweight"] = 1.0;
            Zpt["ZPTReweight_Up"] = 1.0;
            Zpt["ZPTReweight_Down"] = 1.0;
            Zpt["ZPTReweight_QCDScaleUp"] = 1.0;
            Zpt["ZPTReweight_QCDScaleDown"] = 1.0;
            Zpt["ZPTReweight_QCDPDFErrorUp"] = 1.0;
            Zpt["ZPTReweight_QCDPDFErrorDown"] = 1.0;
            Zpt["ZPTReweight_QCDPDFAlphaSUp"] = 1.0;
            Zpt["ZPTReweight_QCDPDFAlphaSDown"] = 1.0;
            Zpt["ZPtEWCorr"] = 1.0;
            Zpt["ZPtEWCorr_E1Up"] = 1.0;
            Zpt["ZPtEWCorr_E1Down"] = 1.0;
            Zpt["ZPtEWCorr_E2Up"] = 1.0;
            Zpt["ZPtEWCorr_E2Down"] = 1.0;
            Zpt["ZPtEWCorr_E3Up"] = 1.0;
            Zpt["ZPtEWCorr_E3Down"] = 1.0;
        }
    } param;
    
    struct Electrons {
        RVec<Electron> AllElectrons;

        std::string TriggerNameForSF_Electron ="WREGammaTrigger";
        RVec<Electron::ElectronID> Electron_Tight_ID = {Electron::ElectronID::POG_HEEP};
        RVec<Electron::ElectronID> Electron_Loose_ID = {Electron::ElectronID::POG_LOOSE};
        //set to custom loose id
        float Electron_MinPt = 53.;

        RVec<std::string> Ele_Trigger;
        float Ele_Trigger_Safe_Pt_Cut;
        
        float Barrel_ID_SF_2023_C = 1.007;
        float Barrel_ID_SF_2023_C_err = 0.004;
        float Barrel_ID_SF_2023_D = 1.009;
        float Barrel_ID_SF_2023_D_err = 0.005;

        float Endcap_ID_SF_2023_C = 0.988;
        float Endcap_ID_SF_2023_C_err = 0.005;
        float Endcap_ID_SF_2023_D = 0.988;
        float Endcap_ID_SF_2023_D_err = 0.004;

        std::string Electron_FR_ID = "HNWR";
        std::string Electron_FR_Key = "AwayJetPt40";
        std::string Electron_CF_ID = "HNWRTight";
        std::string Electron_CF_Key = "ZToLL";
        bool Electron_UseMini = false;
        bool Electron_UsePtCone = false;
        bool isPassCustomLooseID(const Electron& el) const;
        bool isPassCustomTightID(const Electron& el , const HNWR_miniiso_scan::Electrons& eset) const;
        // Loose ID without isolation (matches Python vidNestedWPBitmap with id_level=2, ignoring isolation)
        bool isPassLooseNoIso(const Electron& el) const;
    }el_set;

    struct Muons {
        RVec<Muon> AllMuons;

        std::string TriggerNameForSF_Muon ="Mu50";
        RVec<Muon::MuonID> Muon_Tight_ID = {Muon::MuonID::POG_GLOBAL_HIGH_PT};
        RVec<Muon::MuonID> Muon_Loose_ID = {Muon::MuonID::POG_GLOBAL_HIGH_PT}; //Loose and tight only differ in isolation
        float Muon_MinPt = 53.;

        RVec<std::string> Muon_Trigger;
        float Muon_Trigger_Safe_Pt_Cut;
        
        float Muon_TkrelISO ;
        float Muon_Iso_Cut = 0.1;
        std::string Muon_IS_SF_Key = "HighPtMuonIDSF";
        std::string Muon_Trigger_SF_Key = "POGHighPtLooseTrkIso";
        std::string Muon_FR_ID = "HNWR";
        std::string Muon_FR_Key = "AwayJetPt40";
        std::string Muon_CF_ID = "HNWRTight";
        std::string Muon_CF_Key = "ZToLL";
        bool Muon_UseMini = false;
        bool Muon_UsePtCone = false;
        bool Muon_UseTuneP = true;
    }mu_set;

    struct Jets{
        RVec<Jet> AllJets;
        //TString Jet_ID ="TIGHTLEPVETO";
        RVec<Jet::JetID> Jet_ID = {Jet::JetID::TIGHTLEPVETO};
        float Jet_MinPt = 40.;
        float Jet_MaxEta = 2.5;
        RVec<Jet::JetID> JetIds;
        RVec<Jet>cleanedjet_with_tight_leptons;
        RVec<Jet>cleanedjet_with_loose_leptons;
        RVec<Jet>JetVetoed;
    }jet_set;

    struct FatJets{
        RVec<FatJet> AllFatJets;
        TString FatJet_ID ="Tight";
        float Fatjet_LSF = 0.75; // not used in HNWR_miniiso_scan (replaced by Sublead_MiniIso)
        // scan build: the miniIso gate is a pass-through so that the boosted SR is the
        // common baseline with NEITHER discriminant applied. Both LSF3 and miniIso are
        // then scannable from the SR histograms (see SR_Boosted_*_lsf3_vs_miniiso).
        float Sublead_MiniIso = 999.;
        float FatJet_MinPt = 200.;
        float FatJet_MaxEta = 2.5;
        float FatJet_SDM = 40;
    }fatjet_set;

    struct Gens{
        RVec<Gen> gens;
    }gen_set;

    struct LHECollection{
        RVec<LHE> lhe_parts;
    }lhe_set;

    // Signal categorization flags (set per event in executeEvent)
    bool sig_isSignal   = false;
    bool sig_isOffshell = false;
    bool sig_isOnshell  = false;
    bool sig_isTb       = false;

    // Compute the signal / offshell / onshell / tb flags from gens, LHE and sample name
    void SetSignalFlags();
    // Fill the signal SR cutflow copies (_offshell / _onshell / _tb) next to the base cutflow.
    // Only fills for the Central systematic.
    void FillSignalCutflow(const TString &this_syst, bool isResolved, double binN, float weight);

    // Electron trigger scale factor: constant per era, split barrel/endcap at SC
    // |eta| = 1.4442 (EGM high-pT trigger T&P, fit range pT > 130 GeV).
    // Returns 1.0 for eras without a measurement (e.g. 2017).
    float GetElectronTriggerSF_TnP(double eta, double pt, MyCorrection::variation var) const;

    // High-pT electron ID scale factor from POG/EGM electronID_highPt.json.gz
    // ("Electron-ID-SF", WP "Tight"), binned in (signed SC eta, pt); pt bins start
    // at 100 GeV with clamp flow. Returns 1.0 for eras without the JSON (e.g. 2017).
    float GetElectronHEEPIDSF_TnP(double eta, double pt, MyCorrection::variation var) const;

    // Mini-isolation (miniPFRelIso_all < 0.1) scale factor for the subleading loose
    // lepton cut in the boosted SR, from the egamma-tnp Tag&Probe measurement
    // (2022/2022EE/2023/2023BPix), binned in (el_pt, el_eta). Electrons only —
    // the muon-channel measurement is not available yet. Returns 1.0 for eras
    // without a measurement (e.g. 2017).
    //
    // NOT applied as an event weight yet: the miniIso cut replaces the LSF3 cut in
    // this analyzer, but the corresponding SF is left out for now (LSF_Weight stays
    // at 1). Kept here so the table is ready when it is turned on.
    float GetElectronMiniIsoSF_TnP(double eta, double pt, MyCorrection::variation var) const;

    RVec<FatJet> Clean_Fatjet_with_tight_leptons(const RVec<FatJet> & fatjets, const RVec<Lepton *> & tight_leps) ;
    RVec<Jet> Clean_jet_with_loose_leptons(const RVec<Jet> & jets, const RVec<Lepton *> & loose_leps) ;
    RVec<Muon *> Clean_Fatjet_with_tight_muons(const RVec<FatJet> & fatjets, const RVec<Muon *> & tight_mus) ;
    RVec<Muon *> Clean_jet_with_loose_muons(const RVec<Jet> & jets, const RVec<Muon *> & loose_mus) ;
    RVec<Electron *> Clean_Fatjet_with_tight_electrons(const RVec<FatJet> & fatjets, const RVec<Electron *> & tight_els) ;
    RVec<Electron *> Clean_jet_with_loose_electrons(const RVec<Jet> & jets, const RVec<Electron *> & loose_els) ; 
    
    RVec<Jet> Clean_LSF_FatJet_with_jets(const RVec<FatJet> & fatjets, const RVec<Jet> & jets) ;
    RVec<FatJet> Clean_Jets_with_fatjets(const RVec<Jet> & jets, const RVec<FatJet> & fatjets) ;

    // ------------------------------------------------------------------------
    // DY corrections. Two event weights on DY MC only:
    //
    //     w *= C(genZpT) * R(recoLeadingJetPt)
    //
    // C is the gen-level NLO/LO Z-pT ratio, a per-event scalar, so it goes into
    // `weight` through the systematic machinery and reaches every region.
    // R is data-driven, from the DY CR, and depends on the reco category --
    // resolved reads the leading AK4 pT, boosted the leading AK8 pT. The two
    // categories are NOT exclusive here (is_Resolved_DY_* and is_Boosted_DY_*
    // can both be set), so R must not be folded into `weight`; it multiplies the
    // fill weight per block instead.
    //
    // Derivation: fig/05_backgrounds/scripts/make_dy_{zpt_nlo_lo,jetpt_ratio}.py.
    // Both currently exist for 2022 and 2022EE only.
    // ------------------------------------------------------------------------
    struct DYCorrections {
        bool loaded = false;
        bool apply = false;            // false for data and every non-DY sample
        // R only. `apply` gates both corrections; this gates R alone, so the
        // ZptOnly userflag can leave C on. Meaningless when apply is false.
        bool apply_r = false;

        // C(genZpT): nominal plus the three theory variations, all on one binning
        std::vector<double> zpt_edges;
        std::map<std::string, std::vector<double>> zpt;   // key -> bin values

        // MC statistical error of C. make_dy_zpt_nlo_lo.py keeps it in the bin
        // errors of the nominal ZPTReweight histogram and nowhere else -- the
        // ZPTReweight_Up/Down pair is the theory quadrature sum, not this.
        std::vector<double> zpt_stat;

        // R(jet pT), rebinned curves only. Native binning has R < 0 at high pT.
        std::vector<double> r_edges_res, r_val_res, r_sig_res;
        std::vector<double> r_edges_boo, r_val_boo, r_sig_boo;

        // Number of usable bins per category, i.e. how many DYReshape nuisances
        // there are. Bin 0 sits below the jet pT selection and is undefined.
        int n_nuis_res = 0, n_nuis_boo = 0;
    } dycorr;

    void LoadDYCorrections();

    // Sum of the two isHardProcess charged leptons. Must stay identical to
    // DYGenZpT.cc, which is the definition C was derived against. -1 if absent.
    float GetGenZpT() const;

    // C for one variation key ("ZPTReweight", "ZPTReweight_QCDScaleUp", ...).
    // Clamps above the top edge; returns 1 for a non-finite or non-positive
    // entry, never 0 -- a 0 would delete the event rather than leave it alone.
    float GetZptWeight(float gen_zpt, const std::string &key) const;

    // MC statistical error of C in the bin `gen_zpt` falls in, as an absolute
    // (not relative) sigma. 0 where C itself is undefined, so that C +- sigma
    // collapses back onto the nominal there.
    float GetZptStat(float gen_zpt) const;

    // R for a reco category. `nuis_bin` >= 0 shifts that one bin by
    // `dir` * sigma and leaves the others at nominal, which is how the per-bin
    // DYReshape nuisances are built; -1 is the nominal curve.
    float GetJetPtR(bool resolved, float pt, int nuis_bin = -1, int dir = 0) const;

    // DY samples the corrections apply to: the HT-binned LO set the background
    // estimate uses. DYJets (amcatnloFXFX) is excluded -- it was the numerator
    // when C was derived, so correcting it would be circular.
    bool IsDYSample() const;
    

    float dR_Separation = 0.4;
    float FatJet_dR_Separation = 0.8;




};

#endif 
