#ifndef ExampleRun_h
#define ExampleRun_h

#include "AnalyzerCore.h"
#include "MuonView.h"
#include "SystematicHelper.h"


class ExampleRun : public AnalyzerCore {
public:
    void initializeAnalyzer();
    void executeEvent();
    void executeEventFromParameter();

    bool RunSyst;
    bool RunNewPDF;
    bool RunXSecSyst;

    TString IsoMuTriggerName;
    float TriggerSafePtCut;

    RVec<TString> MuonIDSFKeys;
    RVec<Muon::MuonID> MuonIDs;
    RVec<Muon> AllMuons;
    MuonViewCollection AllMuonViews;
    RVec<Jet> AllJets;
    Event ev;

    float weight_Prefire;

    unique_ptr<SystematicHelper> systHelper;

    ExampleRun();
    ~ExampleRun();
};

#endif

