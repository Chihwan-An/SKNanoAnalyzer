#ifndef DY_backup_h
#define DY_backup_h

#include "AnalyzerCore.h"
#include "SystematicHelper.h"

class DY_backup : public AnalyzerCore {
public:
    DY_backup();
    ~DY_backup();

    void initializeAnalyzer();
    void executeEvent();
    void executeEventFromParameter();

    // Analysis flags
    bool RunSyst;
    
    // Trigger settings
    TString IsoMuTriggerName;
    float TriggerSafePtCut;
    
    // Object ID settings
    RVec<Muon::MuonID> MuonIDs;
    RVec<TString> MuonIDSFKeys;
    
    
    // Physics objects
    RVec<Muon> AllMuons;
    RVec<Jet> AllJets;
    
    RVec<Muon> selectedMuons;
    float dilepton_mass;
    
    // Analysis cuts
    struct AnalysisCuts {
        float muon_pt_lead = 28.0;
        float muon_pt_sublead = 10.0;
        float muon_eta = 2.4;
        float deltaR_overlap = 0.4;
        float base_jet_pt = 20.0;
        float jet_eta = 2.4;
    } cuts;
    
    // Systematic helper
    unique_ptr<SystematicHelper> systHelper;
    
    
    // Helper functions
    RVec<Muon> SelectMuons(const RVec<Muon>& muons);
    RVec<Muon> SelectMuonssublead(const RVec<Muon>& muons);
    RVec<Muon> RemoveOverlap(const RVec<Muon>& muons);
    
    
    RVec<Jet> SelectJets(const RVec<Jet> &jets, const TString ID, const float ptmin, const float fetamax);

};

#endif