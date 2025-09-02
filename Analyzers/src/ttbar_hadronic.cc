#include "ttbar_hadronic.h"

ttbar_hadronic::ttbar_hadronic() {}
ttbar_hadronic::~ttbar_hadronic() {}

void ttbar_hadronic::initializeAnalyzer() {
    cout << "[ttbar_hadronic::initializeAnalyzer] Starting initialization" << endl;
    
    // Check user flags
    RunSyst = HasFlag("RunSyst");
    
    cout << "[ttbar_hadronic::initializeAnalyzer] RunSyst = " << RunSyst << endl;
    
    // Muon IDs and scale factor keys
    
    //MuonIDSFKeys = {"NUM_TightID_DEN_TrackerMuons"};
    
    // Jet IDs
    
    
    // Era-dependent trigger settings
    if (DataEra == "2016preVFP" || DataEra == "2016postVFP" || DataEra == "2018") {
        Trigger = "HLT_PFHT400_SixPFJet32";
        TriggerSafePtCut = 35.;
    } else if (DataEra == "2017") {
        Trigger = "HLT_PFHT400_SixPFJet32"; 
        TriggerSafePtCut = 35.;
    } else if (DataEra == "2022") {
        Trigger = "HLT_PFHT400_SixPFJet32";
        TriggerSafePtCut = 35.;
    } else if (DataEra == "2022EE") {
        Trigger = "HLT_PFHT400_SixPFJet32";
        TriggerSafePtCut = 35.;
    } else if (DataEra == "2023") {
        Trigger = "HLT_PFHT400_SixPFJet32";
        TriggerSafePtCut = 35.;
    } else if (DataEra == "2023BPix") {
        Trigger = "HLT_PFHT400_SixPFJet32";
        TriggerSafePtCut = 35.; 
    } else {
        cerr << "[ttbar_hadronic::initializeAnalyzer] DataEra is not set properly: " << DataEra << endl;
        exit(EXIT_FAILURE);
    }
    
    cout << "[ttbar_hadronic::initializeAnalyzer] Trigger = " << Trigger << endl;
    cout << "[ttbar_hadronic::initializeAnalyzer] TriggerSafePtCut = " << TriggerSafePtCut << endl;
    
    // Initialize corrections
    myCorr = new MyCorrection(DataEra, DataPeriod, IsDATA ? DataStream : MCSample, IsDATA);
    
    // Initialize systematic helper
    string SKNANO_HOME = getenv("SKNANO_HOME");
    if (IsDATA) {
        systHelper = std::make_unique<SystematicHelper>(SKNANO_HOME + "/docs/noSyst.yaml", DataStream, DataEra);
    } else {
        systHelper = std::make_unique<SystematicHelper>(SKNANO_HOME + "/docs/ExampleSystematic.yaml", MCSample, DataEra);
    }
    
    cout << "[ttbar_hadronic::initializeAnalyzer] Initialization complete" << endl;
}

void ttbar_hadronic::executeEvent() {
    // Get all physics objects at the beginning to save CPU time
    
    AllJets = GetAllJets();
    AllFatJets = GetAllFatJets();
    // Loop over systematic sources
    for (const auto &syst_dummy : *systHelper) {
        executeEventFromParameter();
    }
}

void ttbar_hadronic::executeEventFromParameter() {
    const TString this_syst = systHelper->getCurrentSysName();
    
    // Get event information
    Event ev = GetEvent();
    FillHist(this_syst + "/CutFlow", 0.0, 1.0, 10, 0., 10.); // Initial event
    
    RVec<Jet> jets = AllJets;
    RVec<FatJet> fatjets = AllFatJets;
    // Apply HLT trigger (HLT_PFHT400_SixPFJet32)
    //if (!ev.PassTrigger(Trigger)) return;  // not using HLT 
    FillHist(this_syst + "/CutFlow", 1.0, 1.0, 10, 0., 10.); // HLT pass
    
    RVec<FatJet> selected_fatjets;
    for (const auto& fj : fatjets) {
        if (fj.Pt() > cuts.fatjet_pt && abs(fj.Eta()) < cuts.fatjet_eta) {
            selected_fatjets.push_back(fj);
        }
    }
    fatjets = selected_fatjets;
    RVec<FatJet> topjets_wp1 = SelectTopTaggedJets_wp1(fatjets);
    RVec<FatJet> topjets_wp2 = SelectTopTaggedJets_wp2(fatjets);
    RVec<FatJet> topjets_wp3 = SelectTopTaggedJets_wp3(fatjets);
    
    sort(topjets_wp1.begin(), topjets_wp1.end(), PtComparing);
    sort(topjets_wp2.begin(), topjets_wp2.end(), PtComparing);
    sort(topjets_wp3.begin(), topjets_wp3.end(), PtComparing);

    float weight = 1.0;
    if (!IsDATA) {
        weight *= MCweight();
        weight *= ev.GetTriggerLumi("Full");
    }

    float TTbar_mass = (leading_topjet[0] + subleading_topjet[0]).M();
    float TTbar_pt = (leading_topjet[0] + subleading_topjet[0]).Pt();
    float leading_topjet_pt = leading_topjet[0].Pt();
    float subleading_topjet_pt = subleading_topjet[0].Pt();

    FillHist("/workingpoint_passed",0.,weight,5,0.,4.); // no tagging wp passed
    FillHist(this_syst + "/LeadingTopJetPt_wp0", leading_topjet_pt, weight, 4000, 0., 4000.);
    FillHist(this_syst + "/SubleadingTopJetPt_wp0", subleading_topjet_pt, weight, 4000, 0., 4000.);
    FillHist(this_syst + "/TTbarMass_wp0", TTbar_mass, weight, 4000, 0., 4000.);
    FillHist(this_syst + "/TTbarPt_wp0", TTbar_pt, weight, 4000, 0., 4000.);
    if (topjets_wp1.size() < 2) return;

    FillHist("/workingpoint_passed",1.,weight,5,0.,4.); // tagging wp1 not passed
    FillHist(this_syst + "/LeadingTopJetPt_wp1", leading_topjet_pt, weight, 4000, 0., 4000.);
    FillHist(this_syst + "/SubleadingTopJetPt_wp1", subleading_topjet_pt, weight, 4000, 0., 4000.);
    FillHist(this_syst + "/TTbarMass_wp1", TTbar_mass, weight, 4000, 0., 4000.);
    FillHist(this_syst + "/TTbarPt_wp1", TTbar_pt, weight, 4000, 0., 4000.);

    if ((topjets_wp1.size() > 1) && (topjets_wp2.size() < 2)) {
        leading_topjet = {topjets_wp1[0]};
        subleading_topjet = {topjets_wp1[1]};
        FillHist("/workingpoint_passed",2.,weight,5,0.,4.); // wp1 passed, wp2 not passed
        FillHist(this_syst + "/LeadingTopJetPt_wp2", leading_topjet_pt, weight, 4000, 0., 4000.);
        FillHist(this_syst + "/SubleadingTopJetPt_wp2", subleading_topjet_pt, weight, 4000, 0., 4000.);
        FillHist(this_syst + "/TTbarMass_wp2", TTbar_mass, weight, 4000, 0., 4000.);
        FillHist(this_syst + "/TTbarPt_wp2", TTbar_pt, weight, 4000, 0., 4000.);
    }

    if ((topjets_wp2.size() > 1) && (topjets_wp3.size() < 2)) {
        leading_topjet = {topjets_wp2[0]};
        subleading_topjet = {topjets_wp2[1]};
        FillHist("/workingpoint_passed",3.,weight,5,0.,4.); // wp2 passed, wp3 not passed
        FillHist(this_syst + "/LeadingTopJetPt_wp3", leading_topjet_pt, weight, 4000, 0., 4000.);
        FillHist(this_syst + "/SubleadingTopJetPt_wp3", subleading_topjet_pt, weight, 4000, 0., 4000.);
        FillHist(this_syst + "/TTbarMass_wp3", TTbar_mass, weight, 4000,0., 4000.);
    }

    if (topjets_wp3.size() > 1) {
        leading_topjet = {topjets_wp3[0]};
        subleading_topjet = {topjets_wp3[1]};
        FillHist("/workingpoint_passed",4.,weight,5,0.,4.); // wp3 passed
        FillHist(this_syst + "/LeadingTopJetPt_wp4", leading_topjet_pt, weight, 4000, 0., 4000.);
        FillHist(this_syst + "/SubleadingTopJetPt_wp4", subleading_topjet_pt, weight, 4000, 0., 4000.);
        FillHist(this_syst + "/TTbarMass_wp4", TTbar_mass, weight, 4000, 0., 4000.);
        FillHist(this_syst + "/TTbarPt_wp4", TTbar_pt, weight, 4000, 0., 4000.);
    }



}

// Helper function implementations




RVec<FatJet> ttbar_hadronic::SelectTopTaggedJets_wp1(const RVec<FatJet>& fatjets) {
    RVec<FatJet> toptagged_jets;
    for (const auto& fatjet : fatjets) {
        // Using basic mass cuts for top tagging - update with actual tagger when available
        float toptag_score = fatjet.GetTaggerResult(JetTagging::FatJetTaggingtype::ParticleNetWithMass, JetTagging::FatjetTaggingObject::TvsQCD); // placeholder
        float softdrop_mass = fatjet.SDMass();
        
        if (toptag_score > cuts.toptag_score1 &&
            softdrop_mass > cuts.toptag_mass_low &&
            softdrop_mass < cuts.toptag_mass_high) {
            toptagged_jets.push_back(fatjet);
        }
    }
    return toptagged_jets;
}
RVec<FatJet> ttbar_hadronic::SelectTopTaggedJets_wp2(const RVec<FatJet>& fatjets) {
    RVec<FatJet> toptagged_jets;
    for (const auto& fatjet : fatjets) {
        // Using basic mass cuts for top tagging - update with actual tagger when available
        float toptag_score = fatjet.GetTaggerResult(JetTagging::FatJetTaggingtype::ParticleNetWithMass, JetTagging::FatjetTaggingObject::TvsQCD); // placeholder
        float softdrop_mass = fatjet.SDMass();
        
        if (toptag_score > cuts.toptag_score2 &&
            softdrop_mass > cuts.toptag_mass_low &&
            softdrop_mass < cuts.toptag_mass_high) {
            toptagged_jets.push_back(fatjet);
        }
    }
    return toptagged_jets;
}
RVec<FatJet> ttbar_hadronic::SelectTopTaggedJets_wp3(const RVec<FatJet>& fatjets) {
    RVec<FatJet> toptagged_jets;
    for (const auto& fatjet : fatjets) {
        // Using basic mass cuts for top tagging - update with actual tagger when available
        float toptag_score = fatjet.GetTaggerResult(JetTagging::FatJetTaggingtype::ParticleNetWithMass, JetTagging::FatjetTaggingObject::TvsQCD); // placeholder
        float softdrop_mass = fatjet.SDMass();
        
        if (toptag_score > cuts.toptag_score3 &&
            softdrop_mass > cuts.toptag_mass_low &&
            softdrop_mass < cuts.toptag_mass_high) {
            toptagged_jets.push_back(fatjet);
        }
    }
    return toptagged_jets;
}