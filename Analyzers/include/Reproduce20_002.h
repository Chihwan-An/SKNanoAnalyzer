#ifndef Reproduce20_002_h
#define Reproduce20_002_h

#include "AnalyzerCore.h"
#include "SystematicHelper.h"

class Reproduce20_002 : public AnalyzerCore {
public:
    Reproduce20_002();
    ~Reproduce20_002();

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
        RVec<Electron::ElectronID> Electron_Tight_ID = {Electron::ElectronID::POG_TIGHT};
        RVec<Electron::ElectronID> Electron_Loose_ID = {Electron::ElectronID::POG_LOOSE};
        float Electron_MinPt = 53.;

        RVec<std::string> Ele_Trigger;
        float Ele_Trigger_Safe_Pt_Cut;
        
        std::string Electron_IS_SF_Key = "HEEP";
        std::string Electron_Trigger_SF_Key = "HEEP";
        std::string Electron_FR_ID = "HNWR";
        std::string Electron_FR_Key = "AwayJetPt40";
        std::string Electron_CF_ID = "HNWRTight";
        std::string Electron_CF_Key = "ZToLL";
        bool Electron_UseMini = false;
        bool Electron_UsePtCone = false;
    }el_set;

    struct Muons {
        RVec<Muon> AllMuons;

        std::string TriggerNameForSF_Muon ="Mu50";
        RVec<Muon::MuonID> Muon_Tight_ID = {Muon::MuonID::POG_TIGHT};
        RVec<Muon::MuonID> Muon_Loose_ID = {Muon::MuonID::POG_LOOSE};
        float Muon_MinPt = 53.;

        RVec<std::string> Muon_Trigger;
        float Muon_Trigger_Safe_Pt_Cut;

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

        RVec<Jet::JetID> JetIds;
    }jet_set;

    struct FatJets{
        RVec<FatJet> AllFatJets;
    }fatjet_set;

    struct Gens{
        RVec<Gen> gens;
    }gen_set;



    float dR_Separation = 0.4;
    float FatJet_dR_Separation = 0.8;





};

#endif 
