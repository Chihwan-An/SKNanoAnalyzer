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
    void executeEventFromParameter(AnalyzerParameter param);

    RVec<Electron> AllElectrons;
    RVec<Muon> AllMuons;
    RVec<Jet> AllJets;
    RVec<FatJet> AllFatJets;
    

    RVec<Tstring> Ele_Trigger;
    float Ele_Trigger_Safe_Pt_Cut;
    RVec<Tstring> Muon_Trigger;
    float Muon_Trigger_Safe_Pt_Cut;


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
