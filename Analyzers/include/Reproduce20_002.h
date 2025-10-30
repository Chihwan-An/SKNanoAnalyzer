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
    

    RVec<Electron> AllElectrons;
    RVec<Muon> AllMuons;
    RVec<Jet> AllJets;
    RVec<FatJet> AllFatJets;
    RVec<Gen> gens;
    
    struct EMU_Set {
        RVec<Muon::MuonID> Muon_Tight_ID = {Muon::MuonID::POG_TIGHT};
        RVec<Muon::MuonID> Muon_Loose_ID = {Muon::MuonID::POG_LOOSE};

        RVec<Electron::ElectronID> Electron_Tight_ID = {Electron::ElectronID::POG_TIGHT};
        RVec<Electron::ElectronID> Electron_Loose_ID = {Electron::ElectronID::POG_LOOSE};
        float Muon_MinPt = 53.;
        float Electron_MinPt = 53.;
        
        float dR_Separation = 0.4;
        float FatJet_dR_Separation = 0.8;
    } emu_set;
    


    RVec<std::string> Ele_Trigger;
    float Ele_Trigger_Safe_Pt_Cut;
    RVec<std::string> Muon_Trigger;
    float Muon_Trigger_Safe_Pt_Cut;

    RVec<Jet::JetID> JetIds;

    // DY pt Reweighting
    double ZPtReweight, ZPtReweight_Up, ZPtReweight_Down;
    double ZPtReweight_QCDScaleUp, ZPtReweight_QCDScaleDown;
    double ZPtReweight_QCDPDFErrorUp, ZPtReweight_QCDPDFErrorDown;
    double ZPtReweight_QCDPDFAlphaSUp, ZPtReweight_QCDPDFAlphaSDown;
    // DY EW Corrections
    double ZPtEWCorr;
    double ZPtEWCorr_E1Up, ZPtEWCorr_E1Down;
    double ZPtEWCorr_E2Up, ZPtEWCorr_E2Down;
    double ZPtEWCorr_E3Up, ZPtEWCorr_E3Down;

};

#endif 
