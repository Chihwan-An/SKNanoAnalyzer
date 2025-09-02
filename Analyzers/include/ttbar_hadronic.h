#ifndef ttbar_hadronic_h
#define ttbar_hadronic_h

#include "AnalyzerCore.h"
#include "SystematicHelper.h"

class ttbar_hadronic : public AnalyzerCore {
public:
    ttbar_hadronic();
    ~ttbar_hadronic();

    void initializeAnalyzer();
    void executeEvent();
    void executeEventFromParameter();

    // Analysis flags
    bool RunSyst;
    bool RunWRCut;
    
    // Selection parameters
    enum class SelectionCuts {
        NO_WR_CUT = 0,
        WR_CUT_2000 = 2000
    };
    
    SelectionCuts WRCutThreshold;
    
    // Trigger settings
    TString Trigger;
    float TriggerSafePtCut;
    
    // Object ID settings
    
    
    // Physics objects
    
    RVec<Jet> AllJets;
    RVec<FatJet> AllFatJets;
    RVec<FatJet> fatjets;
    RVec<FatJet> leading_topjet;
    RVec<FatJet> subleading_topjet;
    
    
    // Analysis cuts
    struct AnalysisCuts {
        float fatjet_pt = 350.0;
        float fatjet_eta = 2.5;
        float toptag_score1 = 0.5;
        float toptag_score2 = 0.7;
        float toptag_score3 = 0.9;
        float toptag_mass_low = 130.0;
        float toptag_mass_high = 250.0;
        
    } cuts;
    
    // Weight variables
    float weight_Prefire;
    
    // Systematic helper
    unique_ptr<SystematicHelper> systHelper;
    
    // Helper functions
    
    RVec<FatJet> SelectTopTaggedJets_wp1(const RVec<FatJet>& fatjets);
    RVec<FatJet> SelectTopTaggedJets_wp2(const RVec<FatJet>& fatjets);
    RVec<FatJet> SelectTopTaggedJets_wp3(const RVec<FatJet>& fatjets);
    
};

#endif