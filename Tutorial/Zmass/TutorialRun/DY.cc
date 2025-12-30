#include "DY.h"
#include <utility>

//==== Constructor and Destructor
DY::DY() {}
DY::~DY() {}

//==== Initialize variables
// NOTE: Every local varaible declared in executeEvent() will be initialized for every event, 
// but some variables do not have to be reinitialized for every event.
// for example, we will use the sample trigger and trigger safe pt cut throughout all events.
// For these varibles(which are called global variables) are declared in the TutorialRun.h
// and initialized in initializeAnalyzer step
void DY::initializeAnalyzer() {

    RunSyst = HasFlag("RunSyst");
    //==== Dimuon Z-peak events with muon ID
	// One can define customized Muon ID in DataFormat/src/Muon.C
	// For this tutorial, let's use POG based ID
	// https://muon-wiki.docs.cern.ch/guidelines/recommendations/
    MuonIDs.clear();
    MuonIDs.push_back(Muon::MuonID::POG_TIGHT);
    

    //==== Trigger settings
	// In this tutorial, we will use HLT_IsoMu24_v (HighLevelTrigger_IsolatedMuon ptcut 24 GeV)
	// which is the recommended singlemuon tirgger for year 2022
	// https://muon-wiki.docs.cern.ch/guidelines/corrections/#low-pt-reco-efficiencies
	// NOTE: for each dataset, you should use corresponding triggers
	// for example, SingleMuon -> HLT_IsoMu24_v
	//              DoubleMuon -> ...
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
    //==== *IMPORTANT TO SAVE CPU TIME*
	//==== Every GetMuon() function first collect ALL MINIAOD muons with GetAllMuons() 
	//==== and then check ID booleans
    AllMuons = GetAllMuons();
    AllJets = GetAllJets();
    
    for (const auto &syst_dummy : *systHelper) {
        executeEventFromParameter();
    }
}

void DY::executeEventFromParameter() {
    // === Getting systematic  
    const TString this_syst = systHelper->getCurrentSysName();
    
    Event ev = GetEvent();
    
    //nocuts
    FillHist(this_syst + "/CutFlow", 1, 1 , 10, 0 , 10 )


    //==== muons and jets for this event are one the device memory
		//==== select muons  for current ID POG Tight ID
    RVec<Muon> muons = AllMuons;
    RVec<Jet> jets = AllJets;

    //==== Trigger
		//==== see Dataformat/src/Event.cc
    if (!ev.PassTrigger(IsoMuTriggerName)) return;

    //Cutflow afger trigger
    FillHist(this_syst + "/CutFlow", 2, 1 , 10, 0 , 10 );

    RVec<Muon> selectedMuons;
    selectedMuons = SelectMuons(muons); 

    if ( selectedMuons.size() == 1 ) return;
    sort(selectedMuons.begin(), selectedMuons.end(), PtComparing);
    
    //Cutflow after delete single muon events
    FillHist(this_syst + "/CutFlow", 3, 1 , 10, 0 , 10 );

    if ((selectedMuons[0].Pt() < cuts.muon_pt_lead)) return;
    if ((selectedMuons[1].Pt() < cuts.muon_pt_sublead)) return;
    
    //Cutflow after leading and subleading pt cuts
    FillHist(this_syst + "/CutFlow", 4, 1 , 10, 0 , 10 );

    if (selectedMuons[0].Charge() * selectedMuons[1].Charge() > 0) return; //opposite sign
    
    //Cutflow after OS selection
    FillHist(this_syst + "/CutFlow", 5, 1 , 10, 0 , 10 );

    float dilepton_mass = (selectedMuons[0] + selectedMuons[1]).M();
    float dilepton_pt = (selectedMuons[0] + selectedMuons[1]).Pt();
    float dilepton_eta = (selectedMuons[0] + selectedMuons[1]).Eta();
    float dilepton_phi = (selectedMuons[0] + selectedMuons[1]).Phi();

    float weight = 1.0;
    if (!IsDATA) {
        // MC
			//==== MCs are normalized to match their xsec [1/pb]
			//==== to get the full luminosity, just pass "Full"
			// const double gen_weight = ev.MCweight()*weight_norm_1invpb;
        weight *= MCweight();
        weight *= ev.GetTriggerLumi("Full");
    }

    FillHist(this_syst + "/DileptonMass", dilepton_mass, weight, 3000, 0., 3000.);
    FillHist(this_syst + "/DileptonPt", dilepton_pt, weight, 2000, 0., 2000.);
    FillHist(this_syst + "/DileptonEta", dilepton_eta, weight, 100, -5., 5.);
    FillHist(this_syst + "/DileptonPhi", dilepton_phi, weight, 64, -3.2, 3.2);
    FillHist(this_syst + "/LeadingMuonPt", selectedMuons[0].Pt(), weight, 500, 0., 500.);
    FillHist(this_syst + "/SubleadingMuonPt", selectedMuons[1].Pt(), weight, 500, 0., 500.);


    //==== Problem 1. 
        //==== Make a table to check how many events and corresponding statistical errors
        //==== passing the on-Z region.
        //==== Do the errors follow the Gaussian distribution? (stat err ~ sqrt(events))
		
	//==== Problem 2. 
        //==== Check the kinematic distributions of decay products.
        //==== To be more specific, compare the data & MC distribution for leading and subleading muons.
		//==== Some variables can be used to suppress non-DY backgrounds (such as ttbar process)
		//==== Can you implement additional cuts to suppress backgrounds further?
        //==== Check the sensitivity (s/sqrt(b)) before and after giving each cut.
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



