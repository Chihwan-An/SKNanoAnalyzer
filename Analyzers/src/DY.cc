#include "DY.h"
#include <utility>

DY::DY() {}
DY::~DY() {}

void DY::initializeAnalyzer() {
    // Check user flags
    RunSyst = HasFlag("RunSyst");
    
    cout << "[DY::initializeAnalyzer] RunSyst = " << RunSyst << endl;
    
    MuonIDs.clear();
    MuonIDs.push_back(Muon::MuonID::POG_TIGHT);
    


    if (DataEra == "2022") {
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
    } else if (DataEra == "2024") {
	IsoMuTriggerName = "HLT_IsoMu24";
        TriggerSafePtCut = 26.;	      
    } else {
        cerr << "[DY::initializeAnalyzer] DataEra is not set properly: " << DataEra << endl;
        exit(EXIT_FAILURE);
    }
    
    myCorr = new MyCorrection(DataEra, DataPeriod, IsDATA ? DataStream : MCSample, IsDATA);
    

    string SKNANO_HOME = getenv("SKNANO_HOME");
    if (IsDATA) {
        systHelper = std::make_unique<SystematicHelper>(SKNANO_HOME + "/docs/noSyst.yaml", DataStream, DataEra);
    } else {
        systHelper = std::make_unique<SystematicHelper>(SKNANO_HOME + "/docs/ExampleSystematic.yaml", MCSample, DataEra);
    }
    
    
}

void DY::executeEvent() {
    
    AllMuons = GetAllMuons();
    AllJets = GetAllJets();
    
    for (const auto &syst_dummy : *systHelper) {
        executeEventFromParameter();
    }
}

void DY::executeEventFromParameter() {
    const TString this_syst = systHelper->getCurrentSysName();
    
    
    Event ev = GetEvent();
    
    
    RVec<Muon> muons = AllMuons;
    RVec<Jet> jets = AllJets;
    
    if (!ev.PassTrigger(IsoMuTriggerName)) return;

    RVec<Muon> selectedMuons;
    selectedMuons = SelectMuons(muons); 

    if ( selectedMuons.size() == 1 ) return;
    sort(selectedMuons.begin(), selectedMuons.end(), PtComparing);

    if ((selectedMuons[0].Pt() < cuts.muon_pt_lead)) return;
    if ((selectedMuons[1].Pt() < cuts.muon_pt_sublead)) return;

    if (selectedMuons[0].Charge() * selectedMuons[1].Charge() > 0) return; //opposite sign
    
    float dilepton_mass = (selectedMuons[0] + selectedMuons[1]).M();
    float dilepton_pt = (selectedMuons[0] + selectedMuons[1]).Pt();

    float weight = 1.0;
    if (!IsDATA) {
        weight *= MCweight();
        weight *= ev.GetTriggerLumi("Full");
    }

    FillHist(this_syst + "/DileptonMass", dilepton_mass, weight, 3000, 0., 3000.);
    FillHist(this_syst + "/DileptonPt", dilepton_pt, weight, 2000, 0., 2000.);
    FillHist(this_syst + "/LeadingMuonPt", selectedMuons[0].Pt(), weight, 500, 0., 500.);
    FillHist(this_syst + "/SubleadingMuonPt", selectedMuons[1].Pt(), weight, 500, 0., 500.);
        
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



