#ifndef HNWR_h
#define HNWR_h

#include "AnalyzerCore.h"
#include "SystematicHelper.h"
#include "Muon.h"
#include "Electron.h"
#include "LHE.h"

class HNWR : public AnalyzerCore {
public:
    HNWR();
    ~HNWR();

    void initializeAnalyzer();
    void executeEvent();
    void executeEventFromParameter();
    bool RunSyst;
    bool RunWRCut;
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
        bool isPassCustomTightID(const Electron& el , const HNWR::Electrons& eset) const;
        // Loose ID without isolation (matches Python vidNestedWPBitmap with id_level=2, ignoring isolation)
        bool isPassLooseNoIso(const Electron& el) const;

        // 2024 EGamma T&P scale factors (CMS Preliminary, 109 fb^-1, Alpana_WR_LRSM_update_22June2026 slides 2 & 6)
        // 2D (pt, signed eta) binned lookup, replacing the flat 2023 barrel/endcap constants above.
        float GetHEEP2024SF(float pt, float eta, MyCorrection::variation var) const;
        float GetTrigger2024SF(float pt, float eta, MyCorrection::variation var) const;
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
        FatJet::FatJetID FatJet_ID = FatJet::FatJetID::TIGHT;
        float Fatjet_LSF = 0.75;
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

    // Electron trigger (Target_Trigger_OR: HLT_Ele30_WPTight_Gsf | HLT_Photon200 | HLT_Ele115_CaloIdVT_GsfTrkIdT)
    // scale factor from the egamma-tnp Tag&Probe measurement (2022/2022EE/2023/2023BPix),
    // binned in (el_pt, el_eta). Returns 1.0 for eras without a measurement (e.g. 2017).
    float GetElectronTriggerSF_TnP(double eta, double pt, MyCorrection::variation var) const;

    RVec<FatJet> Clean_Fatjet_with_tight_leptons(const RVec<FatJet> & fatjets, const RVec<Lepton *> & tight_leps) ;
    RVec<Jet> Clean_jet_with_loose_leptons(const RVec<Jet> & jets, const RVec<Lepton *> & loose_leps) ;
    RVec<Muon *> Clean_Fatjet_with_tight_muons(const RVec<FatJet> & fatjets, const RVec<Muon *> & tight_mus) ;
    RVec<Muon *> Clean_jet_with_loose_muons(const RVec<Jet> & jets, const RVec<Muon *> & loose_mus) ;
    RVec<Electron *> Clean_Fatjet_with_tight_electrons(const RVec<FatJet> & fatjets, const RVec<Electron *> & tight_els) ;
    RVec<Electron *> Clean_jet_with_loose_electrons(const RVec<Jet> & jets, const RVec<Electron *> & loose_els) ; 
    
    RVec<Jet> Clean_LSF_FatJet_with_jets(const RVec<FatJet> & fatjets, const RVec<Jet> & jets) ;
    RVec<FatJet> Clean_Jets_with_fatjets(const RVec<Jet> & jets, const RVec<FatJet> & fatjets) ;
    

    float dR_Separation = 0.4;
    float FatJet_dR_Separation = 0.8;




};

#endif 
