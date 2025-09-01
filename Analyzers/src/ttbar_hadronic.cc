#include "ttbar_hadronic.h"

ttbar_hadronic::ttbar_hadronic() {}
ttbar_hadronic::~ttbar_hadronic() {}

void ttbar_hadronic::initializeAnalyzer() {
    cout << "[ttbar_hadronic::initializeAnalyzer] Starting initialization" << endl;
    
    // Check user flags
    RunSyst = HasFlag("RunSyst");
    
    cout << "[ttbar_hadronic::initializeAnalyzer] RunSyst = " << RunSyst << endl;
    
    // Muon IDs and scale factor keys
    MuonIDs.clear();
    MuonIDs.push_back(Muon::MuonID::POG_TIGHT);
    //MuonIDSFKeys = {"NUM_TightID_DEN_TrackerMuons"};
    
    // Jet IDs
    JetIDs = {Jet::JetID::TIGHTLEPVETO};
    
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
    if (!ev.PassTrigger(Trigger)) return;
    FillHist(this_syst + "/CutFlow", 1.0, 1.0, 10, 0., 10.); // HLT pass
    
    RVec<FatJet> selected_fatjets;
    for (const auto& fj : fatjets) {
        if (fj.Pt() > cuts.fatjet_pt && abs(fj.Eta()) < cuts.fatjet_eta) {
            selected_fatjets.push_back(fj);
        }
    }
    fatjets = selected_fatjets;
    RVec<FatJet> topjets = SelectTopTaggedJets(fatjets);
    sort(topjets.begin(), topjets.end(), PtComparing);
    if (topjets.size() < 2) return;
    RVec<FatJet> leading_topjet = {topjets[0]};
    RVec<FatJet> subleading_topjet; = {topjets[1]};
    FillHist(this_syst + "/CutFlow", 2.0, 1.0, 10, 0., 10.); // At least 2 top-tagged fat jets

    

    // Calculate TTbar observables
    TTbar_mass = (leading_topjet[0] + subleading_topjet[0]).M();
    TTbar_pt = (leading_topjet[0] + subleading_topjet[0]).Pt();
    
    // Event weight calculation
    float weight = 1.0;
    if (!IsDATA) {
        weight *= MCweight();
        weight *= ev.GetTriggerLumi("Full");
    }
    
    // Fill histograms
    // top jet pt 
    FillHist(this_syst + "/LeadingTopJetPt", leading_topjet[0].Pt(), weight.0, 4000, 0., 4000.);
    FillHist(this_syst + "/SubleadingTopJetPt", subleading_topjet[0].Pt(), weight, 4000, 0., 4000.);
    FillHist(this_syst + "/TTbarMass", TTbar_mass, weight, 4000, 0., 4000.);
    FillHist(this_syst + "/TTbarPt", TTbar_pt, weight, 4000, 0., 4000.);
    
    
}

// Helper function implementations




RVec<FatJet> LRSM_TBChannel::SelectTopTaggedJets(const RVec<FatJet>& fatjets) {
    RVec<FatJet> toptagged_jets;
    for (const auto& fatjet : fatjets) {
        // Using basic mass cuts for top tagging - update with actual tagger when available
        float toptag_score = fatjet.GetTaggerResult(JetTagging::FatJetTaggingtype::ParticleNetWithMass, JetTagging::FatjetTaggingObject::TvsQCD); // placeholder
        float softdrop_mass = fatjet.SDMass();
        
        if (toptag_score > cuts.toptag_score &&
            softdrop_mass > cuts.toptag_mass_low &&
            softdrop_mass < cuts.toptag_mass_high) {
            toptagged_jets.push_back(fatjet);
        }
    }
    return toptagged_jets;
}