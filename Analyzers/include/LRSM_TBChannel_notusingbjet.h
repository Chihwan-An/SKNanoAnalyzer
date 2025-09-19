#ifndef LRSM_TBChannel_notusingbjet_h
#define LRSM_TBChannel_notusingbjet_h

#include "AnalyzerCore.h"
#include "SystematicHelper.h"

class LRSM_TBChannel_notusingbjet : public AnalyzerCore {
public:
    LRSM_TBChannel_notusingbjet();
    ~LRSM_TBChannel_notusingbjet();

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
    TString IsoMuTriggerName;
    TString Trigger1;
    TString Trigger2;
    TString Trigger3;
    float TriggerSafePtCut;
    
    // Object ID settings
    RVec<Muon::MuonID> MuonIDs;
    RVec<TString> MuonIDSFKeys;
    RVec<Jet::JetID> JetIDs;
    
    // Physics objects
    RVec<Muon> AllMuons;
    RVec<Jet> AllJets;
    RVec<FatJet> AllFatJets;
    RVec<Jet> jetss;
    RVec<Jet> bjetss;
    RVec<Muon> muon1;
    RVec<Muon> muon2;
    RVec<Muon> muon_overlap_cleaned;
    // Analysis cuts
    struct AnalysisCuts {
        float muon_pt = 50.0;
        float muon_sub_pt = 30.0;
        float muon_eta = 2.5;
        float jet_pt = 30.0;
        float jet_eta = 2.5;
        float fatjet_pt = 350.0;
        float fatjet_eta = 2.5;
        float btag_wp = 0.2347; // ParticleNet medium WP
        float toptag_score = 0.683;
        float wtag_score = 0.959;
        float toptag_mass_low = 120.0;
        float toptag_mass_high = 250.0;
        float wtag_mass_low = 50;
        float wtag_mass_high = 110;
        float deltaR_overlap = 0.4;
        float deltaR_fatjet_overlap = 0.8;
        float dilepton_mass_cut = 0;
    } cuts;
    
    // Weight variables
    float weight_Prefire;
    
    // Systematic helper
    unique_ptr<SystematicHelper> systHelper;
    
    // Helper functions
    RVec<Muon> SelectHighPtMuons(const RVec<Muon>& muons);
    RVec<Jet> SelectBTaggedJets(const RVec<Jet>& jets);
    RVec<FatJet> SelectTopTaggedJets(const RVec<FatJet>& fatjets);
    RVec<FatJet> SelectWTaggedJets(const RVec<FatJet>& fatjets);
    RVec<Muon> RemoveOverlap(const RVec<Muon>& muons, float deltaR_cut = 0.4);
    RVec<Jet> RemoveOverlapWithMuons(const RVec<Jet>& jets, const RVec<Muon>& muons, float deltaR_cut = 0.4);
    RVec<Jet> RemoveOverlapWithFatJets(const RVec<Jet>& jets, const RVec<FatJet>& fatjets, float deltaR_cut = 0.8);
    RVec<FatJet> RemoveOverlapWithMuonsFatJet(const RVec<FatJet>& fatjets, const RVec<Muon>& muons, float deltaR_cut = 0.8);
    bool PassEventSelection(const RVec<Muon>& muons, const RVec<Jet>& bjets, const RVec<FatJet>& topjets);
    bool PassKinematicCuts(const RVec<Muon>& muons);
    bool PassDileptonMassCut(const RVec<Muon>& muons);
    float CalculateWRMass(const RVec<Muon>& muons, const RVec<Jet>& bjets, const RVec<FatJet>& topjets);
    float CalculateNeutrinoMass(const RVec<Muon>& muons, const RVec<Jet>& bjets, const RVec<FatJet>& topjets);
    float CalculateTransverseMass(const Muon& lepton,  const Jet& b1, float met_pt, float met_phi);
};

#endif