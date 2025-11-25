#include "DY.h"
#include <utility>

DY::DY() {}
DY::~DY() {}

void DY::initializeAnalyzer() {
    cout << "[DY::initializeAnalyzer] Starting initialization" << endl;
    
    // Check user flags
    RunSyst = HasFlag("RunSyst");
    
    cout << "[DY::initializeAnalyzer] RunSyst = " << RunSyst << endl;
    
    // Muon IDs and scale factor keys
    MuonIDs.clear();
    MuonIDs.push_back(Muon::MuonID::POG_TKISO_LOOSE);
    //MuonIDSFKeys = {"NUM_TightID_DEN_TrackerMuons"};
    
    // Jet IDs
    
    
    // Era-dependent trigger settings
    if (DataEra == "2016preVFP" || DataEra == "2016postVFP" || DataEra == "2018") {
        IsoMuTriggerName = "HLT_IsoMu24";
        TriggerSafePtCut = 26.;
    } else if (DataEra == "2017") {
        IsoMuTriggerName = "HLT_IsoMu24"; 
        TriggerSafePtCut = 26.;
    } else if (DataEra == "2022") {
        IsoMuTriggerName = "HLT_IsoMu24";
        TriggerSafePtCut = 26.;
    } else if (DataEra == "2022EE") {
        IsoMuTriggerName = "HLT_IsoMu24";
        TriggerSafePtCut = 26.;
    } else if (DataEra == "2023") {
        IsoMuTriggerName = "HLT_IsoMu24";
        TriggerSafePtCut = 26.;
    } else if (DataEra == "2023BPix") {
        IsoMuTriggerName = "HLT_IsoMu24";
        TriggerSafePtCut = 26.; 
    } else {
        cerr << "[DY::initializeAnalyzer] DataEra is not set properly: " << DataEra << endl;
        exit(EXIT_FAILURE);
    }
    
    cout << "[DY::initializeAnalyzer] IsoMuTriggerName = " << IsoMuTriggerName << endl;
    cout << "[DY::initializeAnalyzer] TriggerSafePtCut = " << TriggerSafePtCut << endl;
    
    // Initialize corrections
    myCorr = new MyCorrection(DataEra, DataPeriod, IsDATA ? DataStream : MCSample, IsDATA);
    
    // Initialize systematic helper
    string SKNANO_HOME = getenv("SKNANO_HOME");
    if (IsDATA) {
        systHelper = std::make_unique<SystematicHelper>(SKNANO_HOME + "/docs/noSyst.yaml", DataStream, DataEra);
    } else {
        systHelper = std::make_unique<SystematicHelper>(SKNANO_HOME + "/docs/ExampleSystematic.yaml", MCSample, DataEra);
    }
    
    cout << "[DY::initializeAnalyzer] Initialization complete" << endl;
}

void DY::executeEvent() {
    // Get all physics objects at the beginning to save CPU time
    AllMuons = GetAllMuons();
    //AllJets = GetAllJets();
    
    // Loop over systematic sources
    for (const auto &syst_dummy : *systHelper) {
        executeEventFromParameter();
    }
}

void DY::executeEventFromParameter() {
    const TString this_syst = systHelper->getCurrentSysName();
    
    // Get event information
    Event ev = GetEvent();
    FillHist(this_syst + "/CutFlow", 0.0, 1.0, 10, 0., 10.); // Initial event
    
    RVec<Muon> muons = AllMuons;
    //RVec<Jet> jets = AllJets;
    

    
    if (!ev.PassTrigger(IsoMuTriggerName)) return;
    FillHist(this_syst + "/CutFlow", 1.0, 1.0, 10, 0., 10.); // HLT pass
    
    
    // Select muons
    RVec<Muon> selectedMuons;
    selectedMuons = SelectMuons(muons); //  eta id cut
    FillHist(this_syst + "/CutFlow", 2.0, 1.0, 10, 0., 10.);
    FillHist(this_syst + "/nSelectedMuons", selectedMuons.size(), 1.0, 10, 0., 10.);
    if (selectedMuons.size() < 2) return;
    FillHist(this_syst + "/CutFlow", 3.0, 1.0, 10, 0., 10.); 
    
    //pt sort 
    sort(selectedMuons.begin(), selectedMuons.end(), PtComparing);


    FillHist(this_syst + "/CutFlow", 4.0, 1.0, 10, 0., 10.);
    // pt cut     
    if ((selectedMuons[0].Pt() < cuts.muon_pt_lead)) return;
    if ((selectedMuons[1].Pt() < cuts.muon_pt_sublead)) return;
    FillHist(this_syst + "/CutFlow", 5.0, 1.0, 10, 0., 10.);
    if (selectedMuons[0].Charge() * selectedMuons[1].Charge() > 0) return; //opposite sign
    FillHist(this_syst + "/CutFlow", 6.0, 1.0, 10, 0., 10.);
    
    float dilepton_mass = (selectedMuons[0] + selectedMuons[1]).M();
    float dilepton_pt = (selectedMuons[0] + selectedMuons[1]).Pt();

    
    
    
    // Event weight calculation
    float weight = 1.0;
    if (!IsDATA) {
        weight *= MCweight();
        weight *= ev.GetTriggerLumi("Full");
    }
    
    // Fill histograms
    FillHist(this_syst + "/DileptonMass", dilepton_mass, weight, 3000, 0., 3000.);
    FillHist(this_syst + "/DileptonPt", dilepton_pt, weight, 2000, 0., 2000.);
    FillHist(this_syst + "/LeadingMuonPt", selectedMuons[0].Pt(), weight, 500, 0., 500.);
    FillHist(this_syst + "/SubleadingMuonPt", selectedMuons[1].Pt(), weight, 500, 0., 500.);
    /*
    // Jet constrains
    RVec<Jet> selectedJets = AnalyzerCore::SelectJets(jets, Jet::JetID::NOCUT, cuts.base_jet_pt, cuts.jet_eta);
    
    if (selectedJets.size() < 1) return;
    
    sort(selectedJets.begin(), selectedJets.end(), PtComparing);
    
    FillHist(this_syst + "/LeadingJetPt", selectedJets[0].Pt(), weight, 2000, 0., 2000.);

    if (selectedJets[0].Pt() > 30.0) {
        FillHist(this_syst + "/jetpt_ov_30_mll", dilepton_mass, weight, 3000, 0., 3000.); // Leading jet pT > 30 GeV
        FillHist(this_syst + "/jetpt_ov_30_ptll", dilepton_pt, weight, 2000, 0., 2000.); // Leading jet pT > 30 GeV
    }
    if (selectedJets[0].Pt() > 50.0) {
        FillHist(this_syst + "/jetpt_ov_50_mll", dilepton_mass, weight, 3000, 0., 3000.); // Leading jet pT > 50 GeV
        FillHist(this_syst + "/jetpt_ov_50_ptll", dilepton_pt, weight, 2000, 0., 2000.); // Leading jet pT > 50 GeV
    }
    if (selectedJets[0].Pt() > 70.0) {
        FillHist(this_syst + "/jetpt_ov_70_mll", dilepton_mass, weight, 3000, 0., 3000.); // Leading jet pT > 70 GeV
        FillHist(this_syst + "/jetpt_ov_70_ptll", dilepton_pt, weight, 2000, 0., 2000.); // Leading jet pT > 70 GeV
    }
    if (selectedJets[0].Pt() > 100.0) {
        FillHist(this_syst + "/jetpt_ov_100_mll", dilepton_mass, weight, 3000, 0., 3000.); // Leading jet pT > 100 GeV
        FillHist(this_syst + "/jetpt_ov_100_ptll", dilepton_pt, weight, 2000, 0., 2000.); // Leading jet pT > 100 GeV
    }
        */
}

// Helper function implementations

RVec<Muon> DY::SelectMuons(const RVec<Muon>& muons) {
    RVec<Muon> selected_muons;
    for (const auto& muon : muons) {
        if (abs(muon.Eta()) < cuts.muon_eta && 
            muon.PassID(MuonIDs[0])) {
            selected_muons.push_back(muon);
        }
    }
    return selected_muons;
}

RVec<Muon> DY::SelectMuonssublead(const RVec<Muon>& muons) {
    RVec<Muon> selected_muons;
    for (const auto& muon : muons) {
        if (muon.Pt() > cuts.muon_pt_sublead && 
            abs(muon.Eta()) < cuts.muon_eta &&
            muon.PassID(MuonIDs[0])) {
            selected_muons.push_back(muon);
        }
    }
    return selected_muons;
}



RVec<Muon> DY::RemoveOverlap(const RVec<Muon>& muons) {
    RVec<Muon> cleaned_muons;
    for (const auto& muon : muons) {
        bool overlaps = false;
        for (const auto& other_muon : muons) {
            if (muon.DeltaR(other_muon) < cuts.deltaR_overlap && muon != other_muon) {
                overlaps = true;
                break;
            }
        }
        if (!overlaps) {
            cleaned_muons.push_back(muon);
        }
    }
    return cleaned_muons;
}
