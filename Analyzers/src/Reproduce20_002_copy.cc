#include "Reproduce20_002_copy.h"    

Reproduce20_002_copy::Reproduce20_002_copy() {}
Reproduce20_002_copy::~Reproduce20_002_copy() {}  

void Reproduce20_002_copy::initializeAnalyzer() {
    // if signal ..  # 26 
    // kfactor  # 51
    el_set.AllElectrons.clear();
    mu_set.AllMuons.clear();
    jet_set.AllJets.clear();
    fatjet_set.AllFatJets.clear();
    gen_set.gens.clear();
    
    mu_set.Muon_Trigger.clear();
    mu_set.Muon_Trigger_Safe_Pt_Cut = 0.;
    el_set.Ele_Trigger.clear();
    el_set.Ele_Trigger_Safe_Pt_Cut = 0.;

    if ( DataEra=="2017")
    {
        mu_set.Muon_Trigger = {"HLT_Mu50", "HLT_OldMu100", "HLT_TkMu100"}; 
        mu_set.Muon_Trigger_Safe_Pt_Cut = 52.;
        el_set.Ele_Trigger = {"HLT_Ele35_WPTight_Gsf","HLT_Photon200","HLT_Ele115_CaloIdVT_GsfTrkIdT"};
        el_set.Ele_Trigger_Safe_Pt_Cut = 38.;  
    }

    if (DataEra == "2022")
    {
        mu_set.Muon_Trigger = {"HLT_Mu50", "HLT_CascadeMu100", "HLT_HighPtTkMu100"}; 
        mu_set.Muon_Trigger_Safe_Pt_Cut = 52.;
        el_set.Ele_Trigger = {"HLT_Photon200","HLT_Ele115_CaloIdVT_GsfTrkIdT"};
        el_set.Ele_Trigger_Safe_Pt_Cut = 118.;  
        //el_set.Ele_Trigger = {"HLT_Ele30_WPTight_Gsf","HLT_Photon200","HLT_Ele115_CaloIdVT_GsfTrkIdT"};
        //el_set.Ele_Trigger_Safe_Pt_Cut = 32.;  
    }
    if (DataEra == "2022EE")
    {
        mu_set.Muon_Trigger = {"HLT_Mu50", "HLT_CascadeMu100", "HLT_HighPtTkMu100"}; 
        mu_set.Muon_Trigger_Safe_Pt_Cut = 52.;
        //el_set.Ele_Trigger = {"HLT_Ele32_WPTight_Gsf","HLT_Photon200","HLT_Ele115_CaloIdVT_GsfTrkIdT"};
        //el_set.Ele_Trigger_Safe_Pt_Cut = 35.;  
        //el_set.Ele_Trigger = {"HLT_Ele30_WPTight_Gsf","HLT_Photon200","HLT_Ele115_CaloIdVT_GsfTrkIdT"};
        //el_set.Ele_Trigger_Safe_Pt_Cut = 32.;  
    }
    if (DataEra == "2023")
    {
        mu_set.Muon_Trigger = {"HLT_Mu50", "HLT_CascadeMu100", "HLT_HighPtTkMu100"}; 
        mu_set.Muon_Trigger_Safe_Pt_Cut = 52.;
        el_set.Ele_Trigger = {"HLT_Photon200","HLT_Ele115_CaloIdVT_GsfTrkIdT"};
        el_set.Ele_Trigger_Safe_Pt_Cut = 118.;   
        //el_set.Ele_Trigger = {"HLT_Photon200","HLT_Ele115_CaloIdVT_GsfTrkIdT"};
        //el_set.Ele_Trigger_Safe_Pt_Cut = 117.; 
    }
    if (DataEra == "2023BPix")
    {
        mu_set.Muon_Trigger = {"HLT_Mu50", "HLT_CascadeMu100", "HLT_HighPtTkMu100"}; 
        mu_set.Muon_Trigger_Safe_Pt_Cut = 52.;
        el_set.Ele_Trigger = {"HLT_Photon200","HLT_Ele115_CaloIdVT_GsfTrkIdT"};
        el_set.Ele_Trigger_Safe_Pt_Cut = 118.;  
        //el_set.Ele_Trigger = {"HLT_Ele30_WPTight_Gsf","HLT_Photon200","HLT_Ele115_CaloIdVT_GsfTrkIdT"};
        //el_set.Ele_Trigger_Safe_Pt_Cut = 32.; 
    }

    myCorr = new MyCorrection(DataEra, DataPeriod, IsDATA ? DataStream : MCSample, IsDATA);

    
    // Initialize systematic helper
    string SKNANO_HOME = getenv("SKNANO_HOME");
    if (IsDATA) {
        systHelper = std::make_unique<SystematicHelper>(SKNANO_HOME + "/docs/DataLRSM.yaml", DataStream, DataEra);
    } else {
        systHelper = std::make_unique<SystematicHelper>(SKNANO_HOME + "/docs/MCLRSM.yaml", MCSample, DataEra);
    }

}

void Reproduce20_002_copy::executeEvent() {

    el_set.AllElectrons =  GetAllElectrons();
    mu_set.AllMuons = GetAllMuons();
    jet_set.AllJets = GetAllJets();
    fatjet_set.AllFatJets = GetAllFatJets();
    gen_set.gens = GetAllGens();

    std::unordered_map<std::string, std::variant<std::function<float(MyCorrection::variation, TString)>, std::function<float()>>> weight_function_map;
    
    for (const auto &syst_dummy : *systHelper) {
        executeEventFromParameter();
    }

}

void Reproduce20_002_copy::executeEventFromParameter() {
    const TString this_syst = systHelper->getCurrentSysName();
    FillHist(this_syst + "/Cutflow_for_reseolved_SR", 1.0 , 1.0, 10, 0., 10.);
    FillHist(this_syst + "/Cutflow_for_Boosted_SR", 1.0 , 1.0, 13, 0., 13.);
    Event ev = GetEvent();
    Particle METv = ev.GetMETVector(Event::MET_Type::PUPPI,Event::MET_Syst::CENTRAL);
    FillHist(this_syst + "/Cutflow_for_reseolved_SR", 2.0 , 1.0, 10, 0., 10.);
    FillHist(this_syst + "/Cutflow_for_Boosted_SR", 2.0 , 1.0, 13, 0., 13.);
    
    std::unordered_map<std::string, std::variant<std::function<float(MyCorrection::variation, TString)>, std::function<float()>>> weight_function_map;
    auto dummy_sf = [](MyCorrection::variation var, TString source) -> float { return 1.0; };

    weight_function_map["PU_Weight"]     = dummy_sf;
    // Electron Targets
    weight_function_map["E_Id_Weight"]   = dummy_sf;
    weight_function_map["E_Reco_Weight"] = dummy_sf;
    weight_function_map["E_Trig_Weight"] = dummy_sf;

    // Muon Targets
    weight_function_map["M_Id_Weight"]   = dummy_sf;
    weight_function_map["M_Reco_Weight"] = dummy_sf;
    weight_function_map["M_Trig_Weight"] = dummy_sf;

    // Jet Targets (object variations, not weight variations)
    weight_function_map["JER_Variation"] = dummy_sf;
    weight_function_map["JES_Variation"] = dummy_sf;
    weight_function_map["M_Iso_Weight"]  = dummy_sf;




    weight_function_map["PU_Weight"] = [&](MyCorrection::variation var, TString source) {
        return myCorr->GetPUWeight(ev.nTrueInt(), var);
    };
    
    bool debug1(false);
    //Event Bool Categorization

    // Resolved
    bool is_Resolved_DY_EE(false);
    bool is_Resolved_DY_MM(false);

    bool is_Resolved_Flav_EM(false);

    bool is_Resolved_SR_EE(false);
    bool is_Resolved_SR_EE_SS(false);
    bool is_Resolved_SR_EE_OS(false);

    bool is_Resolved_SR_MM(false);
    bool is_Resolved_SR_MM_SS(false);
    bool is_Resolved_SR_MM_OS(false);

    // Boosted
    bool is_Boosted_DY_EE(false);
    bool is_Boosted_DY_MM(false);

    bool is_Boosted_Flav_EMJ(false);
    bool is_Boosted_Flav_MEJ(false);

    bool is_Boosted_SR_EE(false);
    bool is_Boosted_SR_EE_SS(false);
    bool is_Boosted_SR_EE_OS(false);

    bool is_Boosted_SR_MM(false);
    bool is_Boosted_SR_MM_SS(false);
    bool is_Boosted_SR_MM_OS(false);
    
    //Event values


    //Resolved 
    float Resolve_DYCREEpt = 0.;
    float Resolve_DYCRMMmass = 0.;
    float Resolve_DYCRMMeta = 0.;
    float Resolve_DYCRMMphi = 0.;
    float Resolve_DYCREEleadjetpt = 0.;
    float Resolve_DYCREEleadjeteta = 0.;
    float Resolve_DYCREEleadjetphi = 0.;
    float Resolve_DYCREEsubleadjetpt = 0.;
    float Resolve_DYCREEsubleadjeteta = 0.;
    float Resolve_DYCREEsubleadjetphi = 0.;
    float Resolve_DYCREEj1j2mass = 0.;
    float Resolve_DYCREEj1j2pt = 0.;
    float Resolve_DYCREEj1j2eta = 0.;
    float Resolve_DYCREEj1j2phi = 0.;
    float Resolve_DYCREEl1j1j2mass = 0.;
    float Resolve_DYCREEl1j1j2pt = 0.;
    float Resolve_DYCREEl1j1j2eta = 0.;
    float Resolve_DYCREEl1j1j2phi = 0.;
    float Resolve_DYCREEl2j1j2mass = 0.;
    float Resolve_DYCREEl2j1j2pt = 0.;
    float Resolve_DYCREEl2j1j2eta = 0.;
    float Resolve_DYCREEl2j1j2phi = 0.;
    float Resolve_DYCREEmlljj = 0.;
    float Resolve_DYCREEmlljjpt = 0.;
    float Resolve_DYCREEmlljjeta = 0.;
    float Resolve_DYCREEmlljjphi = 0.;
    float Resolve_DYCREEleadleppt = 0.;
    float Resolve_DYCREEleadlepeta = 0.;
    float Resolve_DYCREEleadlepphi = 0.;
    float Resolve_DYCREEsubleadleppt = 0.;
    float Resolve_DYCREEsubleadlepeta = 0.;
    float Resolve_DYCREEsubleadlepphi = 0.;
    float Resolve_DYCREEjetnum = 0.;
    float Resolve_DYCREEpunum = 0.;

    float Resolve_DYCRMMpt = 0.;
    float Resolve_DYCRMMmass = 0.;
    float Resolve_DYCRMMeta = 0.;
    float Resolve_DYCRMMphi = 0.;
    float Resolve_DYCRMMleadjetpt = 0.;
    float Resolve_DYCRMMleadjeteta = 0.;
    float Resolve_DYCRMMleadjetphi = 0.;
    float Resolve_DYCRMMsubleadjetpt = 0.;
    float Resolve_DYCRMMsubleadjeteta = 0.;
    float Resolve_DYCRMMsubleadjetphi = 0.;
    float Resolve_DYCRMMj1j2mass = 0.;
    float Resolve_DYCRMMj1j2pt = 0.;
    float Resolve_DYCRMMj1j2eta = 0.;
    float Resolve_DYCRMMj1j2phi = 0.;
    float Resolve_DYCRMMl1j1j2mass = 0.;
    float Resolve_DYCRMMl1j1j2pt = 0.;
    float Resolve_DYCRMMl1j1j2eta = 0.;
    float Resolve_DYCRMMl1j1j2phi = 0.;
    float Resolve_DYCRMMl2j1j2mass = 0.;
    float Resolve_DYCRMMl2j1j2pt = 0.;
    float Resolve_DYCRMMl2j1j2eta = 0.;
    float Resolve_DYCRMMl2j1j2phi = 0.;
    float Resolve_DYCRMMmlljj = 0.;
    float Resolve_DYCRMMmlljjpt = 0.;
    float Resolve_DYCRMMmlljjeta = 0.;
    float Resolve_DYCRMMmlljjphi = 0.;
    float Resolve_DYCRMMleadleppt = 0.;
    float Resolve_DYCRMMleadlepeta = 0.;
    float Resolve_DYCRMMleadlepphi = 0.;
    float Resolve_DYCRMMsubleadleppt = 0.;
    float Resolve_DYCRMMsubleadlepeta = 0.;
    float Resolve_DYCRMMsubleadlepphi = 0.;
    float Resolve_DYCRMMjetnum = 0.;
    float Resolve_DYCRMMpunum = 0.;

    float Resolve_FlavCRpt = 0.;
    float Resolve_FlavCRmass = 0.;
    float Resolve_FlavCReta = 0.;
    float Resolve_FlavCRphi = 0.;
    float Resolve_FlavCRleadjetpt = 0.;
    float Resolve_FlavCRleadjeteta = 0.;
    float Resolve_FlavCRleadjetphi = 0.;
    float Resolve_FlavCRsubleadjetpt = 0.;
    float Resolve_FlavCRsubleadjeteta = 0.;
    float Resolve_FlavCRsubleadjetphi = 0.;
    float Resolve_FlavCRj1j2mass = 0.;
    float Resolve_FlavCRj1j2pt = 0.;
    float Resolve_FlavCRj1j2eta = 0.;
    float Resolve_FlavCRj1j2phi = 0.;
    float Resolve_FlavCRl1j1j2mass = 0.;
    float Resolve_FlavCRl1j1j2pt = 0.;
    float Resolve_FlavCRl1j1j2eta = 0.;
    float Resolve_FlavCRl1j1j2phi = 0.;
    float Resolve_FlavCRl2j1j2mass = 0.;
    float Resolve_FlavCRl2j1j2pt = 0.;
    float Resolve_FlavCRl2j1j2eta = 0.;
    float Resolve_FlavCRl2j1j2phi = 0.;
    float Resolve_FlavCRmlljj = 0.;
    float Resolve_FlavCRmlljjpt = 0.;
    float Resolve_FlavCRmlljjeta = 0.;
    float Resolve_FlavCRmlljjphi = 0.;
    float Resolve_FlavCRleadleppt = 0.;
    float Resolve_FlavCRleadlepeta = 0.;
    float Resolve_FlavCRleadlepphi = 0.;
    float Resolve_FlavCRsubleadleppt = 0.;
    float Resolve_FlavCRsubleadlepeta = 0.;
    float Resolve_FlavCRsubleadlepphi = 0.;
    float Resolve_FlavCRjetnum = 0.;
    float Resolve_FlavCRpunum = 0.;

    float Resolve_SREEpt = 0.;
    float Resolve_SREEmass = 0.;
    float Resolve_SREEeta = 0.;
    float Resolve_SREEphi = 0.;
    float Resolve_SREEleadjetpt = 0.;
    float Resolve_SREEleadjeteta = 0.;
    float Resolve_SREEleadjetphi = 0.;
    float Resolve_SREEsubleadjetpt = 0.;
    float Resolve_SREEsubleadjeteta = 0.;
    float Resolve_SREEsubleadjetphi = 0.;
    float Resolve_SREEj1j2mass = 0.;
    float Resolve_SREEj1j2pt = 0.;
    float Resolve_SREEj1j2eta = 0.;
    float Resolve_SREEj1j2phi = 0.;
    float Resolve_SREEl1j1j2mass = 0.;
    float Resolve_SREEl1j1j2pt = 0.;
    float Resolve_SREEl1j1j2eta = 0.;
    float Resolve_SREEl1j1j2phi = 0.;
    float Resolve_SREEl2j1j2mass = 0.;
    float Resolve_SREEl2j1j2pt = 0.;
    float Resolve_SREEl2j1j2eta = 0.;
    float Resolve_SREEl2j1j2phi = 0.;
    float Resolve_SREEmlljj = 0.;
    float Resolve_SREEmlljjpt = 0.;
    float Resolve_SREEmlljjeta = 0.;
    float Resolve_SREEmlljjphi = 0.;
    float Resolve_SREEleadleppt = 0.;
    float Resolve_SREEleadlepeta = 0.;
    float Resolve_SREEleadlepphi = 0.;
    float Resolve_SREEsubleadleppt = 0.;
    float Resolve_SREEsubleadlepeta = 0.;
    float Resolve_SREEsubleadlepphi = 0.;
    float Resolve_DYSREEjetnum = 0.;
    float Resolve_DYSREEpunum = 0.;

    float Resolve_SRMMpt = 0.;
    float Resolve_SRMMmass = 0.;
    float Resolve_SRMMeta = 0.;
    float Resolve_SRMMphi = 0.;
    float Resolve_SRMMleadjetpt = 0.;
    float Resolve_SRMMleadjeteta = 0.;
    float Resolve_SRMMleadjetphi = 0.;
    float Resolve_SRMMsubleadjetpt = 0.;
    float Resolve_SRMMsubleadjeteta = 0.;
    float Resolve_SRMMsubleadjetphi = 0.;
    float Resolve_SRMMj1j2mass = 0.;
    float Resolve_SRMMj1j2pt = 0.;
    float Resolve_SRMMj1j2eta = 0.;
    float Resolve_SRMMj1j2phi = 0.;
    float Resolve_SRMMl1j1j2mass = 0.;
    float Resolve_SRMMl1j1j2pt = 0.;
    float Resolve_SRMMl1j1j2eta = 0.;
    float Resolve_SRMMl1j1j2phi = 0.;
    float Resolve_SRMMl2j1j2mass = 0.;
    float Resolve_SRMMl2j1j2pt = 0.;
    float Resolve_SRMMl2j1j2eta = 0.;
    float Resolve_SRMMl2j1j2phi = 0.;
    float Resolve_SRMMmlljj = 0.;
    float Resolve_SRMMmlljjpt = 0.;
    float Resolve_SRMMmlljjeta = 0.;
    float Resolve_SRMMmlljjphi = 0.;
    float Resolve_SRMMleadleppt = 0.;
    float Resolve_SRMMleadlepeta = 0.;
    float Resolve_SRMMleadlepphi = 0.;
    float Resolve_SRMMsubleadleppt = 0.;
    float Resolve_SRMMsubleadlepeta = 0.;
    float Resolve_SRMMsubleadlepphi = 0.;
    float Resolve_DYSRMMjetnum = 0.;
    float Resolve_DYSRMMpunum = 0.;


    float Boost_DYCREEpt = 0.;
    float Boost_DYCREEmass = 0.;
    float Boost_DYCREEeta = 0.;
    float Boost_DYCREEphi = 0.;
    float Boost_DYCREEfatjetpt = 0.;
    float Boost_DYCREEfatjeteta = 0.;
    float Boost_DYCREEfatjetphi = 0.;
    float Boost_DYCREEfatjetSDM = 0.;
    float Boost_DYCREEmlljj = 0.;
    float Boost_DYCREEmll = 0.;
    float Boost_DYCREEWRpt = 0.;
    float Boost_DYCREEWReta = 0.;
    float Boost_DYCREEWRphi = 0.;
    float Boost_DYCREEleadleppt = 0.;
    float Boost_DYCREEleadlepeta = 0.;
    float Boost_DYCREEleadlepphi = 0.;
    float Boost_DYCREEsubleadleppt = 0.;
    float Boost_DYCREEsubleadlepeta = 0.;
    float Boost_DYCREEsubleadlepphi = 0.;
    float Boost_DYCREEfatjet_lsf3 = 0.;
    float Boost_DYCREEdeltaR_leadlep_fatjet = 0.;
    float Boost_DYCREEdphi_leadlep_fatjet = 0.;
    float Boost_DYCREEpileup_num = 0.;
    float Boost_DYCREEjet_num = 0.;

    float Boost_DYCRMMpt = 0.;
    float Boost_DYCRMMmass = 0.;
    float Boost_DYCRMMeta = 0.;
    float Boost_DYCRMMphi = 0.;
    float Boost_DYCRMMfatjetpt = 0.;
    float Boost_DYCRMMfatjeteta = 0.;
    float Boost_DYCRMMfatjetphi = 0.;
    float Boost_DYCRMMfatjetSDM = 0.;
    float Boost_DYCRMMmlljj = 0.;
    float Boost_DYCRMMmll = 0.;
    float Boost_DYCRMMWRpt = 0.;
    float Boost_DYCRMMWReta = 0.;
    float Boost_DYCRMMWRphi = 0.;
    float Boost_DYCRMMleadleppt = 0.;
    float Boost_DYCRMMleadlepeta = 0.;
    float Boost_DYCRMMleadlepphi = 0.;
    float Boost_DYCRMMsubleadleppt = 0.;
    float Boost_DYCRMMsubleadlepeta = 0.;
    float Boost_DYCRMMsubleadlepphi = 0.;
    float Boost_DYCRMMfatjet_lsf3 = 0.;
    float Boost_DYCRMMdeltaR_leadlep_fatjet = 0.;
    float Boost_DYCRMMdphi_leadlep_fatjet = 0.;
    float Boost_DYCRMMpileup_num = 0.;
    float Boost_DYCRMMjet_num = 0.;

    float Boost_SREEpt = 0.;
    float Boost_SREEmass = 0.;
    float Boost_SREEeta = 0.;
    float Boost_SREEphi = 0.;
    float Boost_SREEfatjetpt = 0.;
    float Boost_SREEfatjeteta = 0.;
    float Boost_SREEfatjetphi = 0.;
    float Boost_SREEfatjetSDM = 0.;
    float Boost_SREEmlljj = 0.;
    float Boost_SREEmll = 0.;
    float Boost_SREEWRpt = 0.;
    float Boost_SREEWReta = 0.;
    float Boost_SREEWRphi = 0.;
    float Boost_SREEleadleppt = 0.;
    float Boost_SREEleadlepeta = 0.;
    float Boost_SREEleadlepphi = 0.;
    float Boost_SREEsubleadleppt = 0.;
    float Boost_SREEsubleadlepeta = 0.;
    float Boost_SREEsubleadlepphi = 0.;
    float Boost_SREEfatjet_lsf3 = 0.;
    float Boost_SREEdeltaR_leadlep_fatjet = 0.;
    float Boost_SREEdphi_leadlep_fatjet = 0.;
    float Boost_SREEpileup_num = 0.;
    float Boost_SREEjet_num = 0.;

    float Boost_SRMMpt = 0.;
    float Boost_SRMMmass = 0.;
    float Boost_SRMMeta = 0.;
    float Boost_SRMMphi = 0.;
    float Boost_SRMMfatjetpt = 0.;
    float Boost_SRMMfatjeteta = 0.;
    float Boost_SRMMfatjetphi = 0.;
    float Boost_SRMMfatjetSDM = 0.;
    float Boost_SRMMmlljj = 0.;
    float Boost_SRMMmll = 0.;
    float Boost_SRMMWRpt = 0.;
    float Boost_SRMMWReta = 0.;
    float Boost_SRMMWRphi = 0.;
    float Boost_SRMMleadleppt = 0.;
    float Boost_SRMMleadlepeta = 0.;
    float Boost_SRMMleadlepphi = 0.;
    float Boost_SRMMsubleadleppt = 0.;
    float Boost_SRMMsubleadlepeta = 0.;
    float Boost_SRMMsubleadlepphi = 0.;
    float Boost_SRMMfatjet_lsf3 = 0.;
    float Boost_SRMMdeltaR_leadlep_fatjet = 0.;
    float Boost_SRMMdphi_leadlep_fatjet = 0.;
    float Boost_SRMMpileup_num = 0.;
    float Boost_SRMMjet_num = 0.;

    float Boost_FlavEMJpt = 0.;
    float Boost_FlavEMJmass = 0.;
    float Boost_FlavEMJeta = 0.;
    float Boost_FlavEMJphi = 0.;
    float Boost_FlavEMJfatjetpt = 0.;
    float Boost_FlavEMJfatjeteta = 0.;
    float Boost_FlavEMJfatjetphi = 0.;
    float Boost_FlavEMJSDM = 0.;
    float Boost_FlavEMJmlljj = 0.;
    float Boost_FlavEMJmll = 0.;
    float Boost_FlavEMJWRpt = 0.;
    float Boost_FlavEMJWReta = 0.;
    float Boost_FlavEMJWRphi = 0.;
    float Boost_FlavEMJleadleppt = 0.;
    float Boost_FlavEMJleadlepeta = 0.;
    float Boost_FlavEMJleadlepphi = 0.;
    float Boost_FlavEMJsubleadleppt = 0.;
    float Boost_FlavEMJsubleadlepeta = 0.;
    float Boost_FlavEMJsubleadlepphi = 0.;
    float Boost_FlavEMJfatjet_lsf3 = 0.;
    float Boost_FlavEMJdeltaR_leadlep_fatjet = 0.;
    float Boost_FlavEMJdphi_leadlep_fatjet = 0.;
    float Boost_FlavEMJpileup_num = 0.;
    float Boost_FlavEMJjet_num = 0.;

    float Boost_FlavMEJpt = 0.;
    float Boost_FlavMEJmass = 0.;
    float Boost_FlavMEJeta = 0.;
    float Boost_FlavMEJphi = 0.;
    float Boost_FlavMEJfatjetpt = 0.;
    float Boost_FlavMEJfatjeteta = 0.;
    float Boost_FlavMEJfatjetphi = 0.;
    float Boost_FlavMEJSDM = 0.;
    float Boost_FlavMEJmlljj = 0.;
    float Boost_FlavMEJmll = 0.;
    float Boost_FlavMEJWRpt = 0.;
    float Boost_FlavMEJWReta = 0.;
    float Boost_FlavMEJWRphi = 0.;
    float Boost_FlavMEJleadleppt = 0.;
    float Boost_FlavMEJleadlepeta = 0.;
    float Boost_FlavMEJleadlepphi = 0.;
    float Boost_FlavMEJsubleadleppt = 0.;
    float Boost_FlavMEJsubleadlepeta = 0.;
    float Boost_FlavMEJsubleadlepphi = 0.;
    float Boost_FlavMEJfatjet_lsf3 = 0.;
    float Boost_FlavMEJdeltaR_leadlep_fatjet = 0.;
    float Boost_FlavMEJdphi_leadlep_fatjet = 0.;
    float Boost_FlavMEJpileup_num = 0.;
    float Boost_FlavMEJjet_num = 0.;


    
    float weight = 1.0;
    float norm_weight = 1.0;
    RVec<Muon> temp_two_muon;
    //float heep_ID_weight = 1.0;

    if(!IsDATA){
        weight *= MCweight();
        weight *= ev.GetTriggerLumi("Full");
    }
    FillHist(this_syst + "/Cutflow_for_skim", 2.0 , 1.0, 10, 0., 10.);
    //Event selection
    RVec<Electron> electrons = el_set.AllElectrons;
    RVec<Muon> muons = mu_set.AllMuons;
    RVec<Jet> jets = jet_set.AllJets;

    // Apply JES and JER systematic variations
    if (!IsDATA) {
        // Apply JER systematics (requires GenJets for smearing)
        for (const auto& jet : jets) {
            FillHist(this_syst + "/JESJER_before_jetpt",jet.Pt(), weight, 2000, 0., 2000.);
        }
        if (this_syst.Contains("JER_Up")) {
            RVec<GenJet> genjets = GetAllGenJets();
            jets = SmearJets(jets, genjets, MyCorrection::variation::up, "total");
        } else if (this_syst.Contains("JER_Down")) {
            RVec<GenJet> genjets = GetAllGenJets();
            jets = SmearJets(jets, genjets, MyCorrection::variation::down, "total");
        }
        // Apply JES systematics
        else if (this_syst.Contains("JES_Up")) {
            jets = ScaleJets(jets, MyCorrection::variation::up, "total");
        } else if (this_syst.Contains("JES_Down")) {
            jets = ScaleJets(jets, MyCorrection::variation::down, "total");
        }
        if (this_syst.Contains("JER_Up") || this_syst.Contains("JER_Down") || this_syst.Contains("JES_Up") || this_syst.Contains("JES_Down")) {
            for (const auto& jet : jets) {
                FillHist(this_syst + "/JESJER_after_jetpt",jet.Pt(), weight, 2000, 0., 2000.);
            }
        }
    }

    RVec<FatJet> fatjets = fatjet_set.AllFatJets;
    if (!PassNoiseFilter(jets,ev,Event::MET_Type::PUPPI)) return;
    FillHist(this_syst + "/Cutflow_for_skim", 3.0 , 1.0, 10, 0., 10.);
    bool pass_trig_muon = ev.PassTrigger(mu_set.Muon_Trigger);
    bool pass_trig_elec = ev.PassTrigger(el_set.Ele_Trigger);


    std::string nom = "nom";
    
    ///         Leptons       ///
    
    RVec<Electron> my_electrons = SelectElectrons(electrons, "NOCUT" , el_set.Electron_MinPt, 2.4); 
    RVec<Muon> my_muons = SelectMuons(muons, "NOCUT" , mu_set.Muon_MinPt, 2.4); 
    
    sort (my_electrons.begin(), my_electrons.end(), PtComparing);
    sort (my_muons.begin(), my_muons.end(), PtComparing);

    RVec<Electron *> Loose_electrons , Tight_electrons;
    RVec<Muon *> Loose_muons , Tight_muons;
    RVec<Lepton *> Tight_leps_el , Tight_leps_mu , Tight_leps;
    RVec<Lepton *> Loose_leps_el , Loose_leps_mu , Loose_leps;
    
    float el_tight_and_loose = 0.;
    float el_tight_but_fail_loose = 0.;
    float el_loose_and_fail_tight = 0.;
    float el_loose_but_tight = 0.;
    

    for (unsigned int i=0 ; i< my_electrons.size(); i ++) {
        Electron & el = my_electrons.at(i);
        //if (el_set.isPassCustomTightID(el, el_set)) {
        if (el.PassID(el_set.Electron_Tight_ID[0])) {
            Tight_electrons.push_back(&el);
            Tight_leps_el.push_back( &el);
            Tight_leps.push_back(&el);
            if (el_set.isPassCustomLooseID(el)) {
                el_tight_and_loose += 1.;
            } else {
                el_tight_but_fail_loose += 10.;
            }
        }

        // Loose ID: Match Python selectLooseElectrons logic
        // Pass if: (Loose WP without isolation) OR (HEEP)
        bool passLooseNoIso = el_set.isPassLooseNoIso(el);
        bool passHEEP = el.PassID(Electron::ElectronID::POG_HEEP);

        if (passLooseNoIso || passHEEP) {
        //if (el_set.isPassCustomLooseID(el)){  // OLD
        //if (el.PassID(el_set.Electron_Loose_ID[0])) {  // OLD
            Loose_electrons.push_back(&el);
            Loose_leps_el.push_back(&el);
            Loose_leps.push_back(&el);
            if (!el_set.isPassCustomTightID(el, el_set)) {
                el_loose_and_fail_tight += 100.;
            } else {
                el_loose_but_tight += 1000.;
            }
        }
    }
    
    FillHist(this_syst + "/Electron_Tight_and_Loose_ID_Check", el_tight_and_loose + el_tight_but_fail_loose + el_loose_and_fail_tight + el_loose_but_tight , weight, 1111, 0., 1111.);
    
    for (unsigned int i=0 ; i< my_muons.size(); i ++) {
        Muon & mu = my_muons.at(i);

        float tkRelIso = mu.TkRelIso();
        
        if ((mu.PassID(mu_set.Muon_Tight_ID[0]))&&( tkRelIso < 0.1) ){ //global high pt id 
            Tight_muons.push_back(&mu);
            Tight_leps_mu.push_back( &mu);
            Tight_leps.push_back(&mu);
        }
        if (mu.PassID(mu_set.Muon_Loose_ID[0])) {
            Loose_muons.push_back(&mu);
            Loose_leps_mu.push_back(&mu);
            Loose_leps.push_back(&mu);
        }
    }
    

    /// FatJets ///


    RVec<FatJet> fatjet_list ;
    RVec<FatJet> lsf ;

    FillHist(this_syst + "/Fatjet_num_total", fatjets.size() , weight, 10, 0., 10.);
    
    FillHist(this_syst + "/Jet_num_total_before_clean_looselep", jets.size() , weight, 10, 0., 10.);
    FillHist(this_syst + "/Fatjet_num_total_before_clean_tightlep", fatjets.size() , weight, 10, 0., 10.);
    // Leptons cleaned with fatjet(tight lepton) and jets(loose lepton)
    // HNWRAnalyzer does NOT clean fatjets with tight leptons → removed to match
    fatjets = Clean_Fatjet_with_tight_leptons(fatjets, Tight_leps);
    jets = Clean_jet_with_loose_leptons(jets, Loose_leps);
    FillHist(this_syst + "/Jet_num_total_after_clean_looselep", jets.size() , weight, 10, 0., 10.);
    FillHist(this_syst + "/Fatjet_num_total_after_clean_tightlep", fatjets.size() , weight, 10, 0., 10.);

    

    for (unsigned int i=0 ; i< fatjets.size(); i ++) {
        
        FatJet & fj = fatjets.at(i);
        if ((fj.Pt() > fatjet_set.FatJet_MinPt) && (abs(fj.Eta())<fatjet_set.FatJet_MaxEta) && (fj.SDMass() > fatjet_set.FatJet_SDM) ) {
            
            if (fj.PassID(fatjet_set.FatJet_ID)) {
                fatjet_list.push_back(fj);
                if (fj.LSF3() >fatjet_set.Fatjet_LSF) {
                    lsf.push_back(fj);
                }
            }
        }
    }
    FillHist(this_syst + "/Cutflow_for_skim", 4.0 , 1.0, 10, 0., 10.);
    fatjets = fatjet_list;
    RVec<FatJet> fatjets_LSF = lsf;

    sort (fatjets.begin(), fatjets.end(), PtComparing);
    sort (fatjets_LSF.begin(), fatjets_LSF.end(), PtComparing);
    
    FillHist(this_syst + "/Fatjet_num_aftercut", fatjets.size() , weight, 10, 0., 10.);
    FillHist(this_syst + "/Fatjet_LSF_num_aftercut", fatjets_LSF.size() , weight, 10, 0., 10.);
    
    

    sort (Tight_leps.begin(), Tight_leps.end(), PtComparingPtr);

    int n_Loose_leptons  = Loose_electrons.size() + Loose_muons.size();
    int n_Tight_leptons  = Tight_electrons.size() + Tight_muons.size();
    
    FillHist(this_syst + "/N_Loose_Lepton", n_Loose_leptons , weight, 10, 0., 10.);
    FillHist(this_syst + "/N_Tight_Lepton", n_Tight_leptons , weight, 10, 0., 10.);
    FillHist(this_syst + "/Tightleps_pt_lead", Tight_leps.size() >0 ? Tight_leps[0]->Pt() : 0. , weight, 100, 0., 500.);
    FillHist(this_syst + "/Tightleps_pt_sublead", Tight_leps.size() >1 ? Tight_leps[1]->Pt() : 0. , weight, 100, 0., 500.);


    ///         Jets       ///


    
    //jet veto 
    bool is_jet_veto = AnalyzerCore::PassVetoMap(jets, mu_set.AllMuons, "jetvetomap");
    if (!(is_jet_veto) ) return;
    
    
    FillHist(this_syst + "/Cutflow_for_skim", 6.0 , 1.0, 10, 0., 10.);
    FillHist(this_syst + "/Non_Selected_Jetnum", jets.size(), 1.0, 20, 0., 20.);
    RVec<Jet> selected_jets = SelectJets(jets, jet_set.Jet_ID[0] , jet_set.Jet_MinPt, jet_set.Jet_MaxEta);
    sort (selected_jets.begin(), selected_jets.end(), PtComparing);
    FillHist(this_syst + "/Selected_Jetnum", selected_jets.size(), 1.0, 20,-10,10.);

    

    // clean lsf fatjet with jet 
    //selected_jets = Clean_LSF_FatJet_with_jets(fatjets_LSF, selected_jets); /// 일단 젯 중에서 lsf fatjet이랑 겹치는거 제거  
    // clean fatjet with jet 
    //fatjets = Clean_Jets_with_fatjets(selected_jets,fatjets); // lsf fatjet 이랑 겹치는 젯 을 제외한 젯 중에서 팻젯이랑 겹치는경우 팻젯 제거 
    //FillHist(this_syst + "/num_of_jet_after_cleaning_lsffatjet", selected_jets.size(), 1.0, 10, 0., 10.);
    //FillHist(this_syst + "/num_of_fatjet_after_cleaningjet", fatjets.size(), 1.0, 10, 0., 10.);







    // Requires 2 tight leptons , l1 > 60 
    // Def of resolved event 
    bool IsResolvedEvent = false;
    bool this_trigger_pass(false);
    bool tmp_isEE(false), tmp_isMM(false), tmp_isEM(false);
    FillHist(this_syst + "/Cutflow_for_skim", 7.0 , 1.0, 10, 0., 10.);
    if ( (n_Tight_leptons == 2 ) && (Tight_leps[0]->Pt() > 60.0)  && (Tight_leps[1]->Pt() > 53.0)) {
        
        
        if ( (Tight_electrons.size() == 2) && ( Tight_muons.size() == 0 )) {
            if (Tight_electrons[0]->Pt() < el_set.Ele_Trigger_Safe_Pt_Cut) return;
            FillHist(this_syst + "/Cutflow_for_reseolved_SR", 3.0 , 1.0, 10, 0., 10.);
            FillHist(this_syst + "/CutFlow", 3.0, weight, 20,-10,10.); // 2 tight leptons with pT cut
            this_trigger_pass = pass_trig_elec;
            tmp_isEE = true;
            
            weight_function_map["E_Id_Weight"] = [&](MyCorrection::variation var, TString source) -> float {

            double eta1 = abs(Tight_electrons[0]->Eta());
            double eta2 = abs(Tight_electrons[1]->Eta());
            float EB_syst = 0.004; 
            float EE_syst = 0.005;

            auto get_single_electron_sf = [&](double eta, float base_sf) -> float {
            bool isEB = (eta < 1.444);
            bool isEE = (eta > 1.566 && eta < 2.5);

            float current_syst = 0.0;
        
            if (isEB)      current_syst = EB_syst;
            else if (isEE) current_syst = EE_syst;
            else           return base_sf; // Gap 영역 등은 오차 적용 안 함 (혹은 1.0 반환)

        
            if (var == MyCorrection::variation::up) {
                return base_sf + current_syst;
            } else if (var == MyCorrection::variation::down) {
                return base_sf - current_syst;
            } else {
                return base_sf; // Nominal
            }
        };

        float sf1_base = 1.0;
        if      (eta1 < 1.444)                       sf1_base = el_set.Barrel_ID_SF_2023_C;
        else if (eta1 > 1.566 && eta1 < 2.5)         sf1_base = el_set.Endcap_ID_SF_2023_C;

        float sf2_base = 1.0;
        if      (eta2 < 1.444)                       sf2_base = el_set.Barrel_ID_SF_2023_C;
        else if (eta2 > 1.566 && eta2 < 2.5)         sf2_base = el_set.Endcap_ID_SF_2023_C;

        return get_single_electron_sf(eta1, sf1_base) * get_single_electron_sf(eta2, sf2_base);
        };
    }
    
        else if ( (Tight_muons.size() == 2) && ( Tight_electrons.size() == 0 )) {
            if (Tight_muons[0]->Pt() < mu_set.Muon_Trigger_Safe_Pt_Cut) return;
            FillHist(this_syst + "/Cutflow_for_reseolved_SR", 3.0 , 1.0, 10, 0., 10.);
            this_trigger_pass = pass_trig_muon;
            tmp_isMM = true;
            FillHist(this_syst + "/tightmuons", 2 , weight, 5, 0., 5.);
        }
        else if ( (Tight_muons.size() == 1) && ( Tight_electrons.size() == 1 )) {
            if (Tight_muons[0]->Pt() < mu_set.Muon_Trigger_Safe_Pt_Cut) return;
            FillHist(this_syst + "/Cutflow_for_reseolved_SR", 3.0 , 1.0, 10, 0., 10.);
            this_trigger_pass = pass_trig_muon;
            tmp_isEM = true;
            
            weight_function_map["E_Id_Weight"] = [&](MyCorrection::variation var, TString source) -> float {

            double eta1 = abs(Tight_electrons[0]->Eta());
            
            float EB_syst = 0.004; 
            float EE_syst = 0.005;

            auto get_single_electron_sf = [&](double eta, float base_sf) -> float {
            bool isEB = (eta < 1.444);
            bool isEE = (eta > 1.566 && eta < 2.5);

            float current_syst = 0.0;
        
            if (isEB)      current_syst = EB_syst;
            else if (isEE) current_syst = EE_syst;
            else           return base_sf; // Gap 영역 등은 오차 적용 안 함 (혹은 1.0 반환)
            
        
            if (var == MyCorrection::variation::up) {
                return base_sf + current_syst;
            } else if (var == MyCorrection::variation::down) {
                return base_sf - current_syst;
            } else {
                return base_sf; // Nominal
            }
        };

        float sf1_base = 1.0;
        if      (eta1 < 1.444)                       sf1_base = el_set.Barrel_ID_SF_2023_C;
        else if (eta1 > 1.566 && eta1 < 2.5)         sf1_base = el_set.Endcap_ID_SF_2023_C;

        return get_single_electron_sf(eta1, sf1_base);
        };
        /*
        weight_function_map["M_Id_Weight"] = [&](MyCorrection::variation var, TString source) -> float   {
            if (DataEra=="2017") return 1.0;
            return (myCorr->GetMuonIDSF("NUM_HighPtID_DEN_GlobalMuonProbes", *Tight_muons[0], var)) ;
        };
        */
            FillHist(this_syst + "/tightmuons", 3 , weight, 5, 0., 5.);
        }

        

        if (this_trigger_pass) {
            FillHist(this_syst + "/Cutflow_for_reseolved_SR", 4.0 , 1.0, 10, 0., 10.);
            // needs 2 jets 
            Lepton *LeadLep = Tight_leps[0];
            Lepton *SubLeadLep = Tight_leps[1];
            
            float LeadLepCharge = LeadLep->Charge();
            float SubLeadLepCharge = SubLeadLep->Charge();
            bool dRLeadJetLepon(false), dRSubLeadJetLepon(false), dRTwoLetpton(false), dRTwoJets(false);
            if (selected_jets.size() >= 2) {
                FillHist(this_syst + "/Cutflow_for_reseolved_SR", 5.0 , 1.0, 10, 0., 10.);
            dRLeadJetLepon = (selected_jets[0].DeltaR(*Tight_leps[0]) > 0.4) && (selected_jets[0].DeltaR(*Tight_leps[1]) > 0.4);
            dRSubLeadJetLepon = (selected_jets[1].DeltaR(*Tight_leps[0]) > 0.4) && (selected_jets[1].DeltaR(*Tight_leps[1]) > 0.4);
            dRTwoLetpton = (LeadLep->DeltaR(*SubLeadLep) > 0.4);
            dRTwoJets = (selected_jets[0].DeltaR(selected_jets[1]) > 0.4);
            }
            FillHist(this_syst + "/Jetnumber_before_resolved_selection1", selected_jets.size(), weight, 20,-10,10.);
                FillHist(this_syst + "/Jetnumber_before_resolved_selection2", selected_jets.size(), weight, 20,-10,10.);
                if ((selected_jets.size() >= 2 )&&(dRLeadJetLepon)&&(dRSubLeadJetLepon)&&(dRTwoLetpton)&&(dRTwoJets)) { 
                    FillHist(this_syst + "/Jetnumber_before_resolved_selection3", selected_jets.size(), weight, 20,-10,10.);
                    FillHist(this_syst + "/Cutflow_for_reseolved_SR", 6.0 , 1.0, 10, 0., 10.);
                    IsResolvedEvent = true;
                    // Mass calculation 
                    Particle WRCand = *LeadLep + *SubLeadLep + selected_jets[0] + selected_jets[1];
                    
                    double dilepton_mass = ( *LeadLep + *SubLeadLep ).M();
                    double dilepton_pt = ( *LeadLep + *SubLeadLep ).Pt();
                    bool DiLepMassGT200 = ( dilepton_mass > 200.0 );
                    bool DiLepMassGT400 = ( dilepton_mass > 400.0 );
                    bool DiLepMassLT150 = ( dilepton_mass >= 60. ) && ( dilepton_mass < 150. );
                    
                    bool DiLepMass60to100  = (dilepton_mass >= 60.) && (dilepton_mass < 100.);
                    bool DiLepMass100to150 = (dilepton_mass >= 100.) && (dilepton_mass < 150.);
                    bool DiLepMass200to400  = (dilepton_mass >= 200.) && (dilepton_mass < 400.);
                    double trigger_sf_SingleElectron = 1.0;
                    double trigger_sf_SingleMuon = 1.0;


                    
                
                // Trigger SF
                if(!IsDATA){
                    if(tmp_isEE){
                        
                        weight_function_map["E_Reco_Weight"] = [&](MyCorrection::variation var, TString source)  {
                            return (myCorr->GetElectronRECOSF(Tight_electrons[0]->Eta(), Tight_electrons[0]->Pt(), Tight_electrons[0]->Phi(),var)) * myCorr->GetElectronRECOSF(Tight_electrons[1]->Eta(), Tight_electrons[1]->Pt(), Tight_electrons[1]->Phi(),var);
                        };
                        if (DataEra != "2017") {
                        weight_function_map["E_Trig_Weight"] = [&](MyCorrection::variation var, TString source)  {
                            float scalefactor = 1 ; 
                            float upscale = 1;
                            float downscale = 1; 
                            bool has_correct_region = false ;
                            if (Tight_electrons[0]->Pt() > 130) {
                                if (abs(Tight_electrons[0]->Eta())< 1.444) { //barrel 
                                    has_correct_region = true ;

                                    if (DataEra == "2022") {
                                        scalefactor = 0.995;
                                        upscale = 0.004;
                                        downscale = 0.004;
                                    }
                                    if (DataEra == "2022EE" ){
                                        scalefactor = 0.990;
                                        upscale = 0.007;
                                        downscale = 0.007;
                                    }
                                    if (DataEra == "2023" ) {
                                        scalefactor = 0.992;
                                        upscale = 0.006;
                                        downscale = 0.006;
                                    }
                                    if (DataEra == "2023BPix"){
                                        scalefactor = 0.993;
                                        upscale = 0.001;
                                        downscale = 0.001;
                                    }
                                }
                                else if ((abs((Tight_electrons[0]->Eta()> 1.556))) or (abs((Tight_electrons[0]->Eta() < 2.5 )))){
                                    has_correct_region = true ;
                                    
                                    if (DataEra == "2022") {
                                        scalefactor = 0.991;
                                        upscale = 0.009;
                                        downscale = 0.009;
                                    }
                                    if (DataEra == "2022EE" ){
                                        scalefactor = 0.981;
                                        upscale = 0.017;
                                        downscale = 0.017;
                                    }
                                    if (DataEra == "2023" ) {
                                        scalefactor = 0.979;
                                        upscale = 0.019;
                                        downscale = 0.019;
                                    }
                                    if (DataEra == "2023BPix"){
                                        scalefactor = 0.978;
                                        upscale = 0.019;
                                        downscale = 0.019;
                                    }
                                }
                                
                                }
                                if (has_correct_region) {
                                    debug1 = true;
                                    if (var == MyCorrection::variation::up){
                                        scalefactor = scalefactor + upscale;
                                    }
                                    if(var == MyCorrection::variation::down){
                                        scalefactor = scalefactor - downscale;
                                    }
                                    cout<<"Trigger SF: " << scalefactor << " (up: " << upscale << ", down: " << downscale << ")" << endl;
                                }
                            
                            return scalefactor;
                            };
                        }
                        
                    }
                    if(tmp_isMM){
                        /*
                        weight_function_map["M_Id_Weight"] = [&](MyCorrection::variation var, TString source) -> float   {
                            if (DataEra=="2017") return 1.0;
                            return  (myCorr->GetMuonIDSF("NUM_HighPtID_DEN_GlobalMuonProbes", *Tight_muons[0], var))*(myCorr->GetMuonIDSF("NUM_HighPtID_DEN_GlobalMuonProbes", *Tight_muons[1], var));
                        };
                        
                        //float MuonIDSF = (myCorr->GetMuonIDSF("NUM_HighPtID_DEN_GlobalMuonProbes", *Tight_muons[0]))*(myCorr->GetMuonIDSF("NUM_HighPtID_DEN_GlobalMuonProbes", *Tight_muons[1]));
                        weight_function_map["M_Reco_Weight"] = [&](MyCorrection::variation var, TString source)  {
                            return (myCorr->GetMuonRECOSF(*Tight_muons[0], var) * myCorr->GetMuonRECOSF(*Tight_muons[1], var));
                        };
                        //float MuonRECOSF = (myCorr->GetMuonRECOSF(*Tight_muons[0]) * myCorr->GetMuonRECOSF(*Tight_muons[1]));
                        // Fix: build trig_muons inside lambda to avoid dangling reference
                        // (local RVec goes out of scope before lambda is evaluated at systHelper->calculateWeight())
                        weight_function_map["M_Trig_Weight"] = [&](MyCorrection::variation var, TString source) -> float {
                            RVec<Muon*> trig_muons;
                            trig_muons.push_back(Tight_muons[0]);
                            trig_muons.push_back(Tight_muons[1]);
                            if (DataEra=="2017") return 1.0;
                            return (myCorr->GetMuonTriggerSF("NUM_HLT_DEN_HighPtLooseRelIsoProbes",trig_muons, var));
                        };
                        
                        weight_function_map["M_Iso_Weight"] = [&](MyCorrection::variation var, TString source) -> float  {
                            if (DataEra=="2017") return 1.0;
                            return  (myCorr->GetMuonIDSF("NUM_probe_LooseRelTkIso_DEN_HighPtProbes",*Tight_muons[0], var))*(myCorr->GetMuonIDSF("NUM_probe_LooseRelTkIso_DEN_HighPtProbes",*Tight_muons[1], var));
                        };
                        */
                        //float MuonISOSF = (myCorr->GetMuonIDSF("NUM_probe_LooseRelTkIso_DEN_HighPtProbes",*Tight_muons[0]))*(myCorr->GetMuonIDSF("NUM_probe_LooseRelTkIso_DEN_HighPtProbes",*Tight_muons[1]));
                        if ( DiLepMassLT150 && WRCand.M() > 800.0 ) {
                        
                        }
                    }
                    if(tmp_isEM){
                        
                        weight_function_map["E_Reco_Weight"] = [&](MyCorrection::variation var, TString source)  {
                            return    (myCorr->GetElectronRECOSF(Tight_electrons[0]->Eta(), Tight_electrons[0]->Pt(), Tight_electrons[0]->Phi(),var)) ;
                        };
                        weight_function_map["M_Reco_Weight"] = [&](MyCorrection::variation var, TString source)  {
                            return  myCorr->GetMuonRECOSF(*Tight_muons[0], var);
                        };
                        
                        weight_function_map["M_Trig_Weight"] = [&](MyCorrection::variation var, TString source) -> float {
                            if (DataEra=="2017") return 1.0;
                            return   (myCorr->GetMuonTriggerSF("NUM_HLT_DEN_HighPtLooseRelIsoProbes",*Tight_muons[0], var));
                        };
                        
                        weight_function_map["M_Iso_Weight"] = [&](MyCorrection::variation var, TString source) -> float  {
                            if (DataEra=="2017") return 1.0;
                            return   (myCorr->GetMuonIDSF("NUM_probe_LooseRelTkIso_DEN_HighPtProbes",*Tight_muons[0], var));
                        };
                    }   
                }

                // DY CR 60 < ll < 150 , lljj > 800 
                if ( DiLepMassLT150 && WRCand.M() > 800.0 ) {
                    if (tmp_isEE) {
                        is_Resolved_DY_EE = true;
                        
                        //PU&object weight 적용
                        Resolve_DYCREEpt = dilepton_pt;
                        Resolve_DYCREEleadjetpt = selected_jets[0].Pt();
                        Resolve_DYCREEsubleadjetpt = selected_jets[1].Pt();
                        Resolve_DYCREEmlljj = WRCand.M();
                        Resolve_SREEleadleppt = LeadLep->Pt();
                        Resolve_SREEsubleadleppt = SubLeadLep->Pt();
                    }
                    else if (tmp_isMM) {
                        
                        is_Resolved_DY_MM = true;
                        Resolve_DYCRMMpt = dilepton_pt;
                        Resolve_DYCRMMleadjetpt = selected_jets[0].Pt();
                        Resolve_DYCRMMsubleadjetpt = selected_jets[1].Pt();
                        Resolve_DYCRMMmlljj = WRCand.M();
                        Resolve_SRMMleadleppt = LeadLep->Pt();
                        Resolve_SRMMsubleadleppt = SubLeadLep->Pt();
                    }
                    else if (tmp_isEM) {

                    }
                }
            // DY CR1 ( 60 < mll < 100)
                if ( DiLepMass60to100 && WRCand.M() > 800.0 ){
                    if (tmp_isEE)
                    {
                    }
                    if (tmp_isMM)
                    {
                    }
                }
            // DY CR2 ( 100 < mll < 150)
                if ( DiLepMass100to150 && WRCand.M() > 800.0 ){
                    if (tmp_isEE)
                    {
                    }
                    if (tmp_isMM)
                    {
                    }
                }
                // Flavor CR
                if ( DiLepMassGT400 && WRCand.M() > 800.0 ){

                    if (tmp_isEM)
                    {
                    is_Resolved_Flav_EM = true;
                    Resolve_FlavCRpt = dilepton_pt;
                    Resolve_FlavCRleadjetpt = selected_jets[0].Pt();
                    Resolve_FlavCRsubleadjetpt = selected_jets[1].Pt();
                    Resolve_FlavCRmlljj = WRCand.M();
                    Resolve_FlavCRleadleppt = LeadLep->Pt();
                    Resolve_FlavCRsubleadleppt = SubLeadLep->Pt();
                    }
                }
                // low mass CR 
                if (DiLepMassGT200 && WRCand.M() < 800.0 ){
                    if (tmp_isEE)
                    {
                    }
                    if (tmp_isMM)
                    {
                    }
                    }
                FillHist(this_syst + "/Cutflow_for_reseolved_SR", 7.0 , 1.0, 10, 0., 10.);
                FillHist(this_syst + "/Cutflow_for_reseolved_SR", 8.0 , 1.0, 10, 0., 10.);
                //Resovled SR
                if (DiLepMassGT400 && WRCand.M() > 800.0) {
                    if (!IsDATA){
                        if (tmp_isEE){
                            is_Resolved_SR_EE = true;
                            FillHist(this_syst + "/Cutflow_for_reseolved_SR", 9.0 , 1.0, 10, 0., 10.);
                            Resolve_SREEpt = dilepton_pt;
                            Resolve_SREEleadjetpt = selected_jets[0].Pt();
                            Resolve_SREEsubleadjetpt = selected_jets[1].Pt();
                            Resolve_SREEmlljj = WRCand.M();
                            Resolve_SREEleadleppt = LeadLep->Pt();
                            Resolve_SREEsubleadleppt = SubLeadLep->Pt();
                            // charge
                            if ( LeadLepCharge * SubLeadLepCharge > 0 ) {
                                is_Resolved_SR_EE_SS = true;
                            }
                            else {
                                is_Resolved_SR_EE_OS = true;
                            }
                        }
                        if (tmp_isMM){
                            FillHist(this_syst + "/Cutflow_for_reseolved_SR", 9.0 , 1.0, 10, 0., 10.);
                            is_Resolved_SR_MM = true;
                            Resolve_SRMMpt = dilepton_pt;
                            Resolve_SRMMleadjetpt = selected_jets[0].Pt();
                            Resolve_SRMMsubleadjetpt = selected_jets[1].Pt();
                            Resolve_SRMMmlljj = WRCand.M();
                            Resolve_SRMMleadleppt = LeadLep->Pt();
                            Resolve_SRMMsubleadleppt = SubLeadLep->Pt();
                        // charge
                            if ( LeadLepCharge * SubLeadLepCharge > 0 ) {
                                is_Resolved_SR_MM_SS = true;
                            }
                            else {
                                is_Resolved_SR_MM_OS = true;
                        }
                    }
                }
            }// # 1192 -> Resolved ends 
            // dr < 0.4
        //} // 2jets 
    }
    }
}
    
    if (!IsResolvedEvent){
        FillHist(this_syst + "/Cutflow_for_Boosted_SR", 3.0 , 1.0, 13, 0., 13.);
        FillHist(this_syst + "/Boost_tightlepnum", n_Tight_leptons , weight, 10, 0., 10.);

        FillHist(this_syst + "/Cutflow_for_e_mujet", 1.0 , 1.0, 20, 0., 20.);
        FillHist(this_syst + "/Cutflow_for_mu_ejet", 1.0 , 1.0, 20, 0., 20.);

        FillHist(this_syst + "/Boost_cutflow_DY", 1 , weight, 20,-10,10.);
        FillHist(this_syst + "/Boost_cutflow_FLV", 1 , weight, 20,-10,10.);
        if ((n_Tight_leptons >0 ) && (Tight_leps[0]->Pt() > 60.0)) {
            FillHist(this_syst + "/Boost_cutflow_DY", 2 , weight, 20,-10,10.);
            FillHist(this_syst + "/Boost_cutflow_FLV", 2 , weight, 20,-10,10.);
            bool this_trigger_pass_boost(false);
            bool is_tmp_lead_el(false), is_tmp_lead_mu(false);
            Lepton * LeadLep = Tight_leps[0];
            FillHist(this_syst + "/Cutflow_for_e_mujet", 2.0 , 1.0, 20, 0., 20.);
            FillHist(this_syst + "/Cutflow_for_mu_ejet", 2.0 , 1.0, 20, 0., 20.);
            if ( LeadLep->IsElectron() ) {
                FillHist(this_syst + "/Cutflow_for_e_mujet", 3.0 , 1.0, 20, 0., 20.);
                if (LeadLep->Pt() < el_set.Ele_Trigger_Safe_Pt_Cut) return;
                FillHist(this_syst + "/Cutflow_for_e_mujet", 4.0 , 1.0,20, 0., 20.);
                FillHist(this_syst + "/Cutflow_for_Boosted_SR", 4.0 , 1.0, 13, 0., 13.);
                is_tmp_lead_el = true;
                this_trigger_pass_boost = pass_trig_elec;
                weight_function_map["E_Id_Weight"] = [&](MyCorrection::variation var, TString source) -> float {

                double eta1 = abs(Tight_electrons[0]->Eta());
            
                float EB_syst = 0.004; 
                float EE_syst = 0.005;

                auto get_single_electron_sf = [&](double eta, float base_sf) -> float {
                bool isEB = (eta < 1.444);
                bool isEE = (eta > 1.566 && eta < 2.5);

                float current_syst = 0.0;
        
                if (isEB)      current_syst = EB_syst;
                else if (isEE) current_syst = EE_syst;
                else           return base_sf; // Gap 영역 등은 오차 적용 안 함 (혹은 1.0 반환)

        
                if (var == MyCorrection::variation::up) {
                    return base_sf + current_syst;
                } else if (var == MyCorrection::variation::down) {
                    return base_sf - current_syst;
                } else {
                    return base_sf; // Nominal
                }
                };

            float sf1_base = 1.0;
            if      (eta1 < 1.444)                       sf1_base = el_set.Barrel_ID_SF_2023_C;
            else if (eta1 > 1.566 && eta1 < 2.5)         sf1_base = el_set.Endcap_ID_SF_2023_C;

            return get_single_electron_sf(eta1, sf1_base) ;
                };
            }
            else if ( LeadLep->IsMuon()){
                FillHist(this_syst + "/Cutflow_for_mu_ejet", 3.0 , 1.0, 20, 0., 20.);
                if (LeadLep->Pt() < mu_set.Muon_Trigger_Safe_Pt_Cut) return;
                FillHist(this_syst + "/Cutflow_for_mu_ejet", 4.0 , 1.0, 20, 0., 20.);
                is_tmp_lead_mu = true;
                this_trigger_pass_boost = pass_trig_muon;
                FillHist(this_syst + "/Check_is_tmp_lead_muon_ok", 1 , weight, 5, 0., 5.);
                FillHist(this_syst + "/Cutflow_for_Boosted_SR", 4.0 , 1.0, 13, 0., 13.);
            }
            //checked ok
            if (this_trigger_pass_boost){
                if(is_tmp_lead_el){
                FillHist(this_syst + "/Cutflow_for_e_mujet", 5.0 , 1.0, 20, 0., 20.);
                }
                if(is_tmp_lead_mu){
                FillHist(this_syst + "/Cutflow_for_mu_ejet", 5.0 , 1.0, 20, 0., 20.);
                }
                FillHist(this_syst + "/Cutflow_for_Boosted_SR", 5.0 , 1.0, 13, 0., 13.);
                FillHist(this_syst + "/Boost_cutflow_DY", 3 , weight,  20,-10,10.);
                FillHist(this_syst + "/Boost_cutflow_FLV", 3 , weight, 20,-10,10.);
                RVec<Lepton *> Loose_SF_leps = is_tmp_lead_el ? Loose_leps_el : Loose_leps_mu;
                RVec<Lepton *> Loose_OF_leps = is_tmp_lead_el ? Loose_leps_mu : Loose_leps_el;
                Lepton * LowMllLooseLepton ; 
                bool has_lowmll(false);
                float lowmllmass ;
                //check num of SF loose lepton for low mll num
                
                Muon looseMuon ;
                for (unsigned int i=0 ; i< Loose_SF_leps.size(); i ++) {
                    if (Loose_SF_leps[i] == LeadLep) continue;
                    double dilep_mass = (*LeadLep + *Loose_SF_leps[i]).M();
                    
                    if ((dilep_mass > 60) && (dilep_mass < 150 )) {
                        FillHist(this_syst + "/Dilep_mass_cutflow3to4test", dilep_mass, weight, 200, 0., 2000.);
                        has_lowmll = true;
                        lowmllmass = dilep_mass;
                        LowMllLooseLepton = Loose_SF_leps[i];
                        if (is_tmp_lead_mu){
                            Muon * looseMuon = (Muon *)LowMllLooseLepton;
                        }
                        break;
                    }
                }
                
                // Boosted CR selection with low mll ( DY CR)
                if (has_lowmll){
                    
                    FillHist(this_syst + "/Boost_cutflow_DY", 4 , weight, 20,-10,10.);
                    float fatjet_DYBoost_num = 0;
                    for (unsigned int i=0 ; i< fatjets.size(); i++) {
                        FatJet this_fatjet = fatjets.at(i);
                        FillHist(this_syst + "/deltaPhi_LeadLep_Fatjet", abs( LeadLep->DeltaPhi(this_fatjet)) , weight, 100, 0., 3.5);
                        if (abs( LeadLep->DeltaPhi(this_fatjet))>2.0) {
                            FillHist(this_syst + "/Boost_cutflow_DY", 5 , weight, 20,-10,10.);
                            FatJet HNFatJet = this_fatjet;
                            Particle Ncand;
                            bool looselepton_infatjet = false;
                            fatjet_DYBoost_num++;
                        // if loose lepton is inside of fatjet
                            if (this_fatjet.DeltaR( *LowMllLooseLepton)<0.8) {
                                Ncand = this_fatjet;
                                FillHist(this_syst + "/has_looselepton_insidefatjet", 1 , weight, 5, 0., 5.);
                                looselepton_infatjet = true;
                            }
                            else { // if loose lepton is outside of fatjet
                                Ncand = HNFatJet + *LowMllLooseLepton;
                                FillHist(this_syst + "/has_looselepton_outsidefatjet", 1 , weight, 5, 0., 5.);
                                FillHist(this_syst + "/mass_looselepton_fatjet_outside", Ncand.M() , weight, 8000, 0., 8000.);
                                FillHist(this_syst +"/numofhnfatjet_DY", HNFatJet.SDMass() , weight, 10000, 0., 10000.);
                            }
                            Particle WRCand;
                            WRCand = *LeadLep + Ncand;
                            double Ncandmasss = (Ncand).M();
                            double leadlepmass = LeadLep->M();
                            double leadleppt = LeadLep->Pt();
                            FillHist(this_syst + "/LeadLep_pt_DY", leadleppt , weight, 1000, 0., 1000.);
                            FillHist(this_syst + "/LeadLep_mass_DY", leadlepmass , weight, 5000, 0., 5000.);
                            FillHist(this_syst + "/WRCandmass", Ncandmasss , weight, 8000, 0., 8000.);
                            FillHist(this_syst + "/WRCand_mass_boosted_DY", WRCand.M() , weight, 8000, 0., 8000.);
                            if ( WRCand.M() > 800.0 )  {
                                FillHist(this_syst + "/Boost_cutflow_DY", 6 , weight, 20,-10,10.);
                                // DY CR
                                FillHist(this_syst + "/DY_CR_Fatjet_SDMass", HNFatJet.SDMass() , weight, 10000, 0., 10000.);
                                if (!IsDATA){
                                    //SF
                                    if(is_tmp_lead_el){
                                        
                                            weight_function_map["E_Reco_Weight"] = [&](MyCorrection::variation var, TString source) {
                                            return (myCorr->GetElectronRECOSF(LeadLep->Eta(), LeadLep->Pt(), LeadLep->Phi(),var)) ;};
                                            if (DataEra != "2017") {
                                            weight_function_map["E_Trig_Weight"] = [&](MyCorrection::variation var, TString source)  {
                                                float scalefactor = 1 ; 
                                                float upscale = 1;
                                                float downscale = 1; 
                                                bool has_correct_region = false ;
                                                if (Tight_electrons[0]->Pt() > 130) {
                                                    if (abs(Tight_electrons[0]->Eta())< 1.444) { //barrel 
                                                        has_correct_region = true ;
                                                    if (DataEra == "2022") {
                                                        scalefactor = 0.995;
                                                        upscale = 0.004;
                                                        downscale = 0.004;
                                                        }
                                                    if (DataEra == "2022EE" ){
                                                        scalefactor = 0.990;
                                                        upscale = 0.007;
                                                        downscale = 0.007;
                                                        }
                                                    if (DataEra == "2023" ) {
                                                        scalefactor = 0.992;
                                                        upscale = 0.006;
                                                        downscale = 0.006;
                                                        }
                                                    if (DataEra == "2023BPix"){
                                                        scalefactor = 0.993;
                                                        upscale = 0.001;
                                                        downscale = 0.001;
                                                        }
                                                    }
                                                else if ((abs((Tight_electrons[0]->Eta()> 1.556))) or (abs((Tight_electrons[0]->Eta() < 2.5 )))){
                                                    has_correct_region = true ;
                                                    if (DataEra == "2022") {
                                                        scalefactor = 0.991;
                                                        upscale = 0.009;
                                                        downscale = 0.009;
                                                        }
                                                    if (DataEra == "2022EE" ){
                                                        scalefactor = 0.981;
                                                        upscale = 0.017;
                                                        downscale = 0.017;
                                                        }
                                                    if (DataEra == "2023" ) {
                                                        scalefactor = 0.979;
                                                        upscale = 0.019;
                                                        downscale = 0.019;
                                                        }
                                                    if (DataEra == "2023BPix"){
                                                        scalefactor = 0.978;
                                                        upscale = 0.019;
                                                        downscale = 0.019;
                                                        }
                                                    }
                                                }
                                            if (has_correct_region){
                                                debug1 = true;
                                                if (var == MyCorrection::variation::up){
                                                    scalefactor = scalefactor + upscale;
                                                    }
                                                if(var == MyCorrection::variation::down){
                                                    scalefactor = scalefactor - downscale;
                                                    }
                                                }
                                        return scalefactor;
                                            };
                                        }
                                        
                                    }
                                    if(is_tmp_lead_mu){
                                        /*
                                        weight_function_map["M_Id_Weight"] = [&](MyCorrection::variation var, TString source) -> float  {
                                            if (DataEra=="2017") return 1.0;
                                            return (myCorr->GetMuonIDSF("NUM_HighPtID_DEN_GlobalMuonProbes", *Tight_muons[0],var));};
                                        weight_function_map["M_Reco_Weight"] = [&](MyCorrection::variation var, TString source) {
                                            return (myCorr->GetMuonRECOSF(*Tight_muons[0],var)) ;};
                                        
                                        // Fix: capture LowMllLooseLepton by value to avoid dangling reference
                                        // (LowMllLooseLepton goes out of scope before lambda is evaluated at systHelper->calculateWeight())
                                        
                                        weight_function_map["M_Trig_Weight"] = [&, LowMllLooseLepton](MyCorrection::variation var, TString source) -> float {
                                            RVec<Muon*> trig_muons;
                                            trig_muons.push_back(Tight_muons[0]);
                                            if (LowMllLooseLepton) {
                                                trig_muons.push_back( (Muon*)LowMllLooseLepton );
                                            }
                                            if (DataEra=="2017") return 1.0;
                                        return myCorr->GetMuonTriggerSF("NUM_HLT_DEN_HighPtLooseRelIsoProbes", trig_muons, var);};
                                        
                                        weight_function_map["M_Iso_Weight"] = [&](MyCorrection::variation var, TString source) {
                                            return (myCorr->GetMuonIDSF("NUM_probe_LooseRelTkIso_DEN_HighPtProbes",*Tight_muons[0],var));};
                                        */
                                    }
                                }
                                
                                if (is_tmp_lead_el){ // ee 
                                        
                                        is_Boosted_DY_EE = true;
                                        
                                        Boost_DYCREEpt = (*LeadLep + *LowMllLooseLepton).Pt();
                                        Boost_DYCREEfatjetpt = HNFatJet.Pt();
                                        Boost_DYCREEmlljj = WRCand.M();
                                        Boost_DYCREEleadleppt = LeadLep->Pt();
                                        Boost_DYCREEsubleadleppt = LowMllLooseLepton->Pt();
                                        if (looselepton_infatjet){
                                            FillHist(this_syst + "/Boosted_DY_CR_EE_looselepton_infatjet_Fatjet_SDMass", HNFatJet.SDMass() , weight, 10000, 0., 10000.);
                                        }
                                        else{
                                            FillHist(this_syst + "/Boosted_DY_CR_EE_looselepton_outsidefatjet_Fatjet_SDMass", HNFatJet.SDMass() , weight, 10000, 0., 10000.);
                                        }
                                    }
                                    if (is_tmp_lead_mu){// mumu
                                        
                                        is_Boosted_DY_MM = true;
                                        Boost_DYCRMMpt = (*LeadLep + *LowMllLooseLepton).Pt();
                                        Boost_DYCRMMfatjetpt = HNFatJet.Pt();
                                        Boost_DYCRMMmlljj = WRCand.M();
                                        Boost_DYCRMMleadleppt = LeadLep->Pt();
                                        Boost_DYCRMMsubleadleppt = LowMllLooseLepton->Pt();
                                        if (looselepton_infatjet){
                                            FillHist(this_syst + "/Boosted_DY_CR_MM_looselepton_infatjet_Fatjet_SDMass", HNFatJet.SDMass() , weight, 10000, 0., 10000.);
                                        }
                                        else{
                                            FillHist(this_syst + "/Boosted_DY_CR_MM_looselepton_outsidefatjet_Fatjet_SDMass", HNFatJet.SDMass() , weight, 10000, 0., 10000.);
                                        }
                                    }
                                
                                if (lowmllmass < 100){
                            // Boosted CR1    
                                    if (is_tmp_lead_el){ // ee 
                                    }
                                    if (is_tmp_lead_mu){// mumu
                                    }
                                }
                                else{ // 100 < lowmllmass < 150
                                    if (is_tmp_lead_el){ // ee 
                                    }
                                    if (is_tmp_lead_mu){// mumu
                                    }
                            // Boosted CR2
                                }
                            }
                            break;
                        }// lead lep fat jet backto back end 
                    }
                    FillHist(this_syst + "/fatjet_DYBoost_num", fatjet_DYBoost_num , weight, 10, 0., 10.);
                } // has low mll end
            // dont have low mll ( > 150 )
                else{
                    if (is_tmp_lead_el){
                    FillHist(this_syst + "/Cutflow_for_e_mujet", 6.0 , 1.0, 20, 0., 20.);
                    FillHist(this_syst + "/Cutflow_for_e_mujet", 7.0 , 1.0,20, 0., 20.);
                    }
                    if (is_tmp_lead_mu){
                    FillHist(this_syst + "/Cutflow_for_mu_ejet", 6.0 , 1.0, 20, 0., 20.);
                    FillHist(this_syst + "/Cutflow_for_mu_ejet", 7.0 , 1.0, 20, 0., 20.);
                    }
                    
                    FillHist(this_syst + "/Cutflow_for_Boosted_SR", 6.0 , 1.0, 13, 0., 13.);
                    FillHist(this_syst + "/Boost_cutflow_FLV", 4 , weight, 20,-10,10.);
                    bool hasawaymergedfatjet = false;
                    FatJet Ncand;
                    Particle WRCand;
                    FatJet HNFatJet;

                    for (unsigned int i=0 ; i<fatjets_LSF.size(); i++) {
                        FatJet this_fatjet = fatjets_LSF.at(i);
                        FillHist(this_syst + "/deltaPhi_LeadLep_Fatjet_FLV", abs( LeadLep->DeltaPhi(this_fatjet)) , weight, 100, 0., 3.5);
                        if (abs( LeadLep->DeltaPhi(this_fatjet))>2.0) {
                            
                            HNFatJet = this_fatjet;
                            Ncand = HNFatJet;
                            WRCand = *LeadLep + Ncand;
                            hasawaymergedfatjet = true;
                            break;
                        }
                    }
                    // 이거 만족하는것 중에 리딩 골라야 하는거 아닌가?
                    //if  lead lep delta phi cut > 2.0 with fatjet
                    if (hasawaymergedfatjet) {
                        if (is_tmp_lead_el){
                        FillHist(this_syst + "/Cutflow_for_e_mujet", 8.0 , 1.0, 20, 0., 20.);
                        }
                        if (is_tmp_lead_mu){
                        FillHist(this_syst + "/Cutflow_for_mu_ejet", 8.0 , 1.0, 20, 0., 20.);
                        }
                        FillHist(this_syst + "/Cutflow_for_Boosted_SR", 7.0 , 1.0,13, 0., 13.);
                        FillHist(this_syst + "/Boost_cutflow_FLV", 5 , weight, 20,-10,10.);
                        bool hassflooselepton(false);
                        bool hasoflooselepton(false);
                        Lepton *SFLooseLepton = nullptr;
                        Lepton *OFLooseLepton = nullptr;
                        FillHist(this_syst +"/numofhnfatjet_FLV", HNFatJet.SDMass() , weight, 10000, 0., 10000.); 
                    //num SF loose lepton inside fatjet for high mll
                        
                        
                        for (unsigned int k=0 ; k< Loose_SF_leps.size(); k ++) {
                            if (LeadLep->DeltaR( *Loose_SF_leps[k])<0.01) continue;
                            FillHist(this_syst + "/Loosesflepton_pt_beforeDR", Loose_SF_leps.at(k)->Pt() , weight, 100, 0., 500.);
                            if ( Loose_SF_leps.at(k)->Pt() < 53.0) continue;
                            FillHist(this_syst + "/Loosesfleptondr", HNFatJet.DeltaR(*Loose_SF_leps[k]) , weight, 100, 0., 5.);
                            if (HNFatJet.DeltaR(*Loose_SF_leps[k]) < 0.8) {
                                hassflooselepton = true;
                                SFLooseLepton = Loose_SF_leps[k];
                                break;
                            }
                        }
                        
                    //num OF loose lepton inside fatjet for high mll

                        for (unsigned int m=0 ; m< Loose_OF_leps.size(); m ++) {
                            if (LeadLep->DeltaR( *Loose_OF_leps[m])<0.01) continue;
                            FillHist(this_syst + "/Looseoflepton_pt_beforeDR", Loose_OF_leps.at(m)->Pt() , weight, 100, 0., 500.);
                            if ( Loose_OF_leps.at(m)->Pt() < 53.0) continue;
                            FillHist(this_syst + "/Looseofleptondr", HNFatJet.DeltaR(*Loose_OF_leps[m]) , weight, 100, 0., 5.);
                            if (HNFatJet.DeltaR(*Loose_OF_leps[m]) < 0.8) {
                                hasoflooselepton = true;
                                OFLooseLepton = Loose_OF_leps[m];
                                break;
                            }
                        }
                        // 여기 둘다 다 모아 놓고 플레이버가 안에 다른게 있으면 탈락 , 다 같으면 리딩만 골라야
                        //그래서 SF 인 경우 OF 인 경우 모두 같은 플레이버 들어있으니까 리딩만 골라야?
                        // Veto tight lepton 
                        int NExtraTightLepton(0);
                            for (unsigned int i=0 ; i< Tight_leps.size(); i ++) {
                                if ( Tight_leps[i] == LeadLep || Tight_leps[i] == SFLooseLepton || Tight_leps[i] == OFLooseLepton ) continue;
                                NExtraTightLepton++;
                            }
                        
                        bool hasnoextralep = (NExtraTightLepton == 0);
                        bool WRMassGT800 = ( WRCand.M() > 800.0 );
                        if (hasnoextralep ){
                            FillHist(this_syst + "/Cutflow_for_Boosted_SR", 8.0 , 1.0, 13, 0., 13.);
                            // tight fatjet 밖 한개 , loose lepton same flavor 안에 
                            FillHist(this_syst + "/Boost_cutflow_FLV", 6 , weight, 20, -10., 10.);
                            if (hassflooselepton) {
                                FillHist(this_syst + "/Cutflow_for_Boosted_SR", 9.0 , 1.0, 13, 0., 13.);
                                FillHist(this_syst + "/Boost_cutflow_FLV", 7 , weight, 20,-10,10.);
                                if (!hasoflooselepton){
                                    FillHist(this_syst + "/Cutflow_for_Boosted_SR", 10.0 , 1.0, 13, 0., 13.);
                                    FillHist(this_syst + "/Boost_cutflow_FLV", 8 , weight, 20,-10,10.);
                        
                        //    if(tmp_IsLeadM){
                        //==== In this case, the loose ID is HighPt ID muon.
                        //==== we want to apply the lepton scale factors to these muons
                        //Muon *looseMuon = (Muon *)SFLooseLepton;
                        //ForSF_muons.push_back( looseMuon );
                        //    }
                                    if ( (*LeadLep + *SFLooseLepton).M() >200.0 ) {
                                        FillHist(this_syst + "/Cutflow_for_Boosted_SR", 11.0 , 1.0, 13, 0., 13.);
                                        //charge 
                                        float LeadLepCharge = LeadLep->Charge();
                                        float SFLooseLeptonCharge = SFLooseLepton->Charge();
                                        FillHist(this_syst + "/Boost_cutflow_FLV", 9 , weight, 20,-10,10.);
                                        // Boosted DY SR
                                        if (WRMassGT800) {
                                            if(!IsDATA){
                                                //SF
                                                if(is_tmp_lead_el){
                                                    FillHist (this_syst + "/Cutflow_for_Boosted_SR", 12.0 , 1.0, 13, 0., 13.);
                                                    
                                                    weight_function_map["E_Reco_Weight"] = [&](MyCorrection::variation var, TString source)  {
                                                    return (myCorr->GetElectronRECOSF(LeadLep->Eta(), LeadLep->Pt(), LeadLep->Phi(),var)) ;};
                                                    if (DataEra != "2017") {
                                                    weight_function_map["E_Trig_Weight"] = [&](MyCorrection::variation var, TString source)  {
                                                        float scalefactor = 1 ; 
                                                        float upscale = 1;
                                                        float downscale = 1; 
                                                        bool has_correct_region = false ;
                                                        if (Tight_electrons[0]->Pt() > 130) {
                                                            if (abs(Tight_electrons[0]->Eta())< 1.444) { //barrel 
                                                                has_correct_region = true ;
                                                                if (DataEra == "2022") {
                                                                    scalefactor = 0.995;
                                                                    upscale = 0.004;
                                                                    downscale = 0.004;
                                                                    }
                                                                if (DataEra == "2022EE" ){
                                                                    scalefactor = 0.990;
                                                                    upscale = 0.007;
                                                                    downscale = 0.007;
                                                                    }
                                                                if (DataEra == "2023" ) {
                                                                    scalefactor = 0.992;
                                                                    upscale = 0.006;
                                                                    downscale = 0.006;
                                                                    }
                                                                if (DataEra == "2023BPix"){
                                                                    scalefactor = 0.993;
                                                                    upscale = 0.001;
                                                                    downscale = 0.001;
                                                                    }
                                                                }
                                                            else if ((abs((Tight_electrons[0]->Eta()> 1.556))) or (abs((Tight_electrons[0]->Eta() < 2.5 )))){
                                                                has_correct_region = true ;
                                                                if (DataEra == "2022") {
                                                                    scalefactor = 0.991;
                                                                    upscale = 0.009;
                                                                    downscale = 0.009;
                                                                    }
                                                                if (DataEra == "2022EE" ){
                                                                    scalefactor = 0.981;
                                                                    upscale = 0.017;
                                                                    downscale = 0.017;
                                                                    }
                                                                if (DataEra == "2023" ) {
                                                                    scalefactor = 0.979;
                                                                    upscale = 0.019;
                                                                    downscale = 0.019;
                                                                    }
                                                                if (DataEra == "2023BPix"){
                                                                    scalefactor = 0.978;
                                                                    upscale = 0.019;
                                                                    downscale = 0.019;
                                                                    }
                                                                }
                                                            }
                                                        if (has_correct_region){
                                                            debug1 = true;
                                                            if (var == MyCorrection::variation::up){
                                                                scalefactor = scalefactor + upscale;
                                                                }
                                                            if(var == MyCorrection::variation::down){
                                                                scalefactor = scalefactor - downscale;
                                                                }
                                                            }
                                                        
                                                        return scalefactor;
                                                        };
                                                    }

                                                    
                                                    
                                                }
                                                if(is_tmp_lead_mu){
                                                    FillHist (this_syst + "/Cutflow_for_Boosted_SR", 12.0 , 1.0, 13, 0., 13.);
                                                    
                                                    //temp_two_muon.clear();
                                                    //Muon * SFLooseLepton_mu = (Muon *)SFLooseLepton;
                                                    //temp_two_muon.push_back(*SFLooseLepton_mu);

                                                    /*
                                                    weight_function_map["M_Id_Weight"] = [&](MyCorrection::variation var, TString source) -> float   {
                                                    if (DataEra=="2017") return 1.0;
                                                    return (myCorr->GetMuonIDSF("NUM_HighPtID_DEN_GlobalMuonProbes", *Tight_muons[0],var));};
                                                    weight_function_map["M_Reco_Weight"] = [&](MyCorrection::variation var, TString source)  {
                                                    return (myCorr->GetMuonRECOSF(*Tight_muons[0],var));};
                                                    
                                                    // Fix: capture SFLooseLepton by value to avoid dangling reference
                                                    // (SFLooseLepton goes out of scope before lambda is evaluated at systHelper->calculateWeight())
                                                    
                                                    weight_function_map["M_Trig_Weight"] = [&, SFLooseLepton](MyCorrection::variation var, TString source) -> float {
                                                        RVec<Muon*> trig_muons;
                                                        trig_muons.push_back(Tight_muons[0]);
                                                        trig_muons.push_back((Muon*)SFLooseLepton);
                                                        if (DataEra=="2017") return 1.0;
                                                    return myCorr->GetMuonTriggerSF("NUM_HLT_DEN_HighPtLooseRelIsoProbes", trig_muons, var);};
                                                    
                                                    weight_function_map["M_Iso_Weight"] = [&](MyCorrection::variation var, TString source) -> float {
                                                    if (DataEra=="2017") return 1.0;
                                                        return (myCorr->GetMuonIDSF("NUM_probe_LooseRelTkIso_DEN_HighPtProbes",*Tight_muons[0],var));};
                                                    */
                                                    
                                                }
                                            
                                                if (is_tmp_lead_el) {
                                                    //SF 
                                                    
                                                    is_Boosted_SR_EE = true;
                                                    Boost_SREEpt = (*LeadLep + *SFLooseLepton).Pt();
                                                    Boost_SREEfatjetpt = HNFatJet.Pt();
                                                    Boost_SREEmlljj = WRCand.M();
                                                    Boost_SREEleadleppt = LeadLep->Pt();
                                                    Boost_SREEsubleadleppt = SFLooseLepton->Pt();
                                                    //charge 
                                                    if ( LeadLepCharge * SFLooseLeptonCharge > 0 ) {
                                                        is_Boosted_SR_EE_SS = true;
                                                        
                                                    }
                                                    else {
                                                        is_Boosted_SR_EE_OS = true;
                                                        
                                                    }
                                                }
                                                if (is_tmp_lead_mu) {
                                                    
                                                    is_Boosted_SR_MM = true;
                                                    Boost_SRMMpt = (*LeadLep + *SFLooseLepton).Pt();
                                                    Boost_SRMMfatjetpt = HNFatJet.Pt();
                                                    Boost_SRMMmlljj = WRCand.M();
                                                    Boost_SRMMleadleppt = LeadLep->Pt();
                                                    Boost_SRMMsubleadleppt = SFLooseLepton->Pt();
                                                    
                                                    //charge
                                                    if ( LeadLepCharge * SFLooseLeptonCharge > 0 ) {
                                                        is_Boosted_SR_MM_SS = true;

                                                    }
                                                    else {
                                                        is_Boosted_SR_MM_OS = true;
                                                        
                                                    }
                                                }                                            
                                            }
                                        }
                                        else{ // low wr CR
                                            if (is_tmp_lead_el) {
                                                // Boosted low WR ee CR
                                            }
                                            else if (is_tmp_lead_mu) {
                                                // Boosted low WR mumu CR
                                            }
                                        }
                                        // Boosted DY CR
                                    }
                                }
                            }
                    if(is_tmp_lead_el){
                    FillHist(this_syst + "/Cutflow_for_e_mujet", 9.0 , 1.0,20, 0., 20.);
                    }
                    if(is_tmp_lead_mu){
                    FillHist(this_syst + "/Cutflow_for_mu_ejet", 9.0 , 1.0, 20, 0., 20.);
                    }
                    // tight fatjet 밖 한개 , loose lepton oppo flavor 안에
                        if (!hassflooselepton){
                            if(is_tmp_lead_el){
                                FillHist(this_syst + "/Cutflow_for_e_mujet", 10.0 , 1.0,20, 0., 20.);
                            }
                            if(is_tmp_lead_mu){
                                FillHist(this_syst + "/Cutflow_for_mu_ejet", 10.0 , 1.0, 20, 0., 20.);
                            }
                            FillHist(this_syst + "/Boost_cutflow_FLV", -6 , weight, 20, -10, 10.);
                            if (hasoflooselepton){
                                if(is_tmp_lead_el){
                                    FillHist(this_syst + "/Cutflow_for_e_mujet", 11.0 , 1.0,20, 0., 20.);
                                }
                                if(is_tmp_lead_mu){
                                    FillHist(this_syst + "/Cutflow_for_mu_ejet", 11.0 , 1.0, 20, 0., 20.);
                                }   
                                // Boosted Flavor CR
                                if ((*LeadLep + *OFLooseLepton).M() > 200.0) {
                                    if(is_tmp_lead_el){
                                        FillHist(this_syst + "/Cutflow_for_e_mujet", 12.0 , 1.0,20, 0., 20.);
                                    }
                                    if(is_tmp_lead_mu){
                                        FillHist(this_syst + "/Cutflow_for_mu_ejet", 12.0 , 1.0, 20, 0., 20.);
                                    }   
                                    FillHist(this_syst + "/Boost_cutflow_FLV", -7 , weight, 20, -10, 10.);
                                    if (WRMassGT800) {
                                        if(is_tmp_lead_el){
                                            FillHist(this_syst + "/Cutflow_for_e_mujet", 13.0 , 1.0,20, 0., 20.);
                                        }
                                    if(is_tmp_lead_mu){
                                            FillHist(this_syst + "/Cutflow_for_mu_ejet", 13.0 , 1.0, 20, 0., 20.);
                                        }   
                                        FillHist(this_syst + "/Boost_cutflow_FLV", -8 , weight, 20, -10, 10.);
                                        if(!IsDATA){
                                            if(is_tmp_lead_el){
                                                        //float ElIDSF = heep_ID_weight;
                                                        cout<<"ok1"<<endl;
                                                        
                                                        weight_function_map["E_Reco_Weight"] = [&](MyCorrection::variation var, TString source)  {
                                                        return (myCorr->GetElectronRECOSF(LeadLep->Eta(), LeadLep->Pt(), LeadLep->Phi(),var)) ;};
                                                        if (DataEra != "2017") {
                                                        weight_function_map["E_Trig_Weight"] = [&](MyCorrection::variation var, TString source)  {
                                                            float scalefactor = 1 ; 
                                                            float upscale = 1;
                                                            float downscale = 1; 
                                                            bool has_correct_region = false ;
                                                                if (Tight_electrons[0]->Pt() > 130) {
                                                                    if (abs(Tight_electrons[0]->Eta())< 1.444) { //barrel 
                                                                        has_correct_region = true ;
                                                                        if (DataEra == "2022") {
                                                                            scalefactor = 0.995;
                                                                            upscale = 0.004;
                                                                            downscale = 0.004;
                                                                            }
                                                                        if (DataEra == "2022EE" ){
                                                                            scalefactor = 0.990;
                                                                            upscale = 0.007;
                                                                            downscale = 0.007;
                                                                            }
                                                                        if (DataEra == "2023" ) {
                                                                            scalefactor = 0.992;
                                                                            upscale = 0.006;
                                                                            downscale = 0.006;
                                                                            }
                                                                        if (DataEra == "2023BPix"){
                                                                            scalefactor = 0.993;
                                                                            upscale = 0.001;
                                                                            downscale = 0.001;
                                                                            }
                                                                        }
                                                                    else if ((abs((Tight_electrons[0]->Eta()> 1.556))) or (abs((Tight_electrons[0]->Eta() < 2.5 )))){
                                                                        has_correct_region = true ;
                                                                        if (DataEra == "2022") {
                                                                            scalefactor = 0.991;
                                                                            upscale = 0.009;
                                                                            downscale = 0.009;
                                                                            }
                                                                        if (DataEra == "2022EE" ){
                                                                            scalefactor = 0.981;
                                                                            upscale = 0.017;
                                                                            downscale = 0.017;
                                                                            }
                                                                        if (DataEra == "2023" ) {
                                                                            scalefactor = 0.979;
                                                                            upscale = 0.019;
                                                                            downscale = 0.019;
                                                                            }
                                                                        if (DataEra == "2023BPix"){
                                                                            scalefactor = 0.978;
                                                                            upscale = 0.019;
                                                                            downscale = 0.019;
                                                                            }
                                                                        }
                                                                    }
                                                                if (has_correct_region){
                                                                    debug1 = true;
                                                                    if (var == MyCorrection::variation::up){
                                                                        scalefactor = scalefactor + upscale;
                                                                        }
                                                                    if(var == MyCorrection::variation::down){
                                                                        scalefactor = scalefactor - downscale;
                                                                        }
                                                                    }
                                                            return scalefactor ;
                                                        };
                                                    }
                                                }
                                                if(is_tmp_lead_mu){
                                                    cout<<"ok11"<<endl;
                                                    /*
                                                    weight_function_map["M_Id_Weight"] = [&](MyCorrection::variation var, TString source) -> float   {
                                                    if (DataEra=="2017") return 1.0;
                                                    return (myCorr->GetMuonIDSF("NUM_HighPtID_DEN_GlobalMuonProbes", *Tight_muons[0],var));};
                                                    weight_function_map["M_Reco_Weight"] = [&](MyCorrection::variation var, TString source)  {
                                                    return (myCorr->GetMuonRECOSF(*Tight_muons[0],var));};
                                                    
                                                    weight_function_map["M_Trig_Weight"] = [&](MyCorrection::variation var, TString source) -> float {
                                                    if (DataEra=="2017") return 1.0;
                                                        return (myCorr->GetMuonTriggerSF("NUM_HLT_DEN_HighPtLooseRelIsoProbes",*Tight_muons[0],var));};
                                                    
                                                    weight_function_map["M_Iso_Weight"] = [&](MyCorrection::variation var, TString source) -> float {
                                                        if (DataEra=="2017") return 1.0;
                                                    return (myCorr->GetMuonIDSF("NUM_probe_LooseRelTkIso_DEN_HighPtProbes",*Tight_muons[0],var));};
                                                    */
                                                }
                                        }
                                        if (is_tmp_lead_el) {
                                            cout<<"ok2"<<endl;
                                            is_Boosted_Flav_EMJ = true;
                                            Boost_FlavEMJpt = (*LeadLep + *OFLooseLepton).Pt();
                                            Boost_FlavEMJleadfatjetpt = HNFatJet.Pt();
                                            Boost_FlavEMJmlljj = WRCand.M();
                                            Boost_FlavEMJSDM = HNFatJet.SDMass();
                                            Boost_FlavEMJleadleppt = LeadLep->Pt();
                                            Boost_FlavEMJsubleadleppt = OFLooseLepton->Pt();
                                            //eta 
                                            Boost_FlavEMJleadlepeta = LeadLep->Eta();
                                            Boost_FlavEMJsubleadlepeta = OFLooseLepton->Eta();
                                            Boost_FlavEMJfatjeteta = HNFatJet.Eta();
                                            Boost_FlavEMJfatjetphi = HNFatJet.Phi();

                                            Boost_FlavEMJdeltaR_leadlep_fatjet = LeadLep->DeltaR(HNFatJet);


                                            // lsf
                                            Boost_FlavEMJleadlep_lsf = HNFatJet.LSF3();
                                            
                                            //mll
                                            Boost_FlavEMJmll = (*LeadLep + *OFLooseLepton).M();
                                            // fatjet pt 
                                            Boost_FlavEMJfatjetpt = HNFatJet.Pt();
                                            // wr pt 
                                            Boost_FlavEMJWRpt = WRCand.Pt();
                                        }
                                        else if (is_tmp_lead_mu) {
                                            // Boosted Flavor CR
                                            cout<<"ok22"<<endl;
                                            is_Boosted_Flav_MEJ = true;
                                            Boost_FlavMEJpt = (*LeadLep + *OFLooseLepton).Pt();
                                            Boost_FlavMEJleadfatjetpt = HNFatJet.Pt();
                                            Boost_FlavMEJmlljj = WRCand.M();
                                            Boost_FlavMEJSDM = HNFatJet.SDMass();
                                            Boost_FlavMEJleadleppt = LeadLep->Pt();
                                            Boost_FlavMEJsubleadleppt = OFLooseLepton->Pt();
                                            
                                            Boost_FlavMEJleadlepeta = LeadLep->Eta();
                                            Boost_FlavMEJsubleadlepeta = OFLooseLepton->Eta();
                                            Boost_FlavMEJfatjeteta = HNFatJet.Eta();
                                            Boost_FlavMEJfatjetphi = HNFatJet.Phi();

                                            Boost_FlavMEJdeltaR_leadlep_fatjet = LeadLep->DeltaR(HNFatJet);


                                            // lsf
                                            Boost_FlavMEJleadlep_lsf = HNFatJet.LSF3();
                                            
                                            //mll
                                            Boost_FlavMEJmll = (*LeadLep + *OFLooseLepton).M();
                                            // fatjet pt 
                                            Boost_FlavMEJfatjetpt = HNFatJet.Pt();
                                            // wr pt 
                                            Boost_FlavMEJWRpt = WRCand.Pt();
                                            

                                        }
                                        // B    oosted Flavor CR
                                    }
                                    else {
                                            if (is_tmp_lead_el) {
                                        // Boosted low WR Flavor CR
                                        
                                        }
                                            else if (is_tmp_lead_mu) {
                                            // Boosted low WR Flavor CR
                                        
                                        }
                                    }
                                }
                            }
                        }
                    } // has no extra lepton finish 
                } // leading lepton back to back fatjet end   
            } // dont have low mll end
        }
        } // boost lead lep pt cut end
    }// boost selected event end
    systHelper->assignWeightFunctionMap(weight_function_map);
    //bool exist_data = ((is_Resolved_DY_EE) || (is_Resolved_DY_MM) || (is_Resolved_SR_EE) || (is_Resolved_SR_EE_SS) || (is_Resolved_SR_EE_OS) || (is_Resolved_SR_MM) || (is_Resolved_SR_MM_SS) || (is_Resolved_SR_MM_OS) || (is_Resolved_Flav_EM) || (is_Boosted_DY_EE) || (is_Boosted_DY_MM) || (is_Boosted_SR_EE) || (is_Boosted_SR_EE_SS) || (is_Boosted_SR_EE_OS) || (is_Boosted_Flav_EMJ) || (is_Boosted_Flav_MEJ) );
    //if (!exist_data) return;
    if(!IsDATA) {
        auto weight_map = systHelper->calculateWeight(); 
        for (const auto& [syst_name, sf_val] : weight_map) {
            //if (syst_name != "Central") continue ;
            float final_weight = weight * sf_val; //weight -> mc, lumi weight
            if ((syst_name.find("ElectronTrig") != std::string::npos) && (debug1)) {
                cout<<"Systematic: "<<syst_name<<" SF value: "<<sf_val<<" Final weight: "<<final_weight<<endl;
            }
            //resolved
            //DY CR
            if (is_Resolved_DY_EE) {
                FillHist(syst_name + "/Obj_PU_Corr_DYCR_Resolved_EE_pt", Resolve_DYCREEpt , final_weight,100, 0., 1000.);
                FillHist(syst_name + "/Obj_PU_Corr_DYCR_Resolved_EE_leading_jet_pt", Resolve_DYCREEleadjetpt, final_weight, 200, 0., 2000.);
                FillHist(syst_name + "/Obj_PU_Corr_DYCR_Resolved_EE_subleading_jet_pt", Resolve_DYCREEsubleadjetpt, final_weight, 200, 0., 2000.);
                FillHist(syst_name + "/Obj_PU_Corr_DYCR_Resolved_EE_mlljj", Resolve_DYCREEmlljj, final_weight, 800, 0., 8000.);
                FillHist(syst_name + "/Obj_PU_Corr_DYCR_Resolved_EE_leading_lep_pt", Resolve_DYCREEleadleppt, final_weight, 2000, 0., 2000.);
                FillHist(syst_name + "/Obj_PU_Corr_DYCR_Resolved_EE_subleading_lep_pt", Resolve_DYCREEsubleadleppt, final_weight, 2000, 0., 2000.);
                
            }
            if (is_Resolved_DY_MM) {
                if (syst_name == "Central") {
                    /*//cout
                    <<"weight without pu "<<norm_weight
                    <<"weight with only pu"<<weight
                    <<"sf_val"<<sf_val
                    << " final weight "<<final_weight<<endl;
                if ((is_Resolved_DY_EE) or (is_Resolved_SR_EE_SS) or (is_Resolved_SR_EE_OS) or (is_Resolved_SR_MM) or (is_Resolved_SR_MM_SS) or (is_Resolved_SR_MM_OS) or (is_Resolved_Flav_EM) or (is_Boosted_DY_EE) or (is_Boosted_DY_MM) or (is_Boosted_SR_EE) or (is_Boosted_SR_EE_SS) or (is_Boosted_SR_EE_OS) or (is_Boosted_SR_MM) or (is_Boosted_SR_MM_SS) or (is_Boosted_SR_MM_OS) or (is_Boosted_Flav_EMJ) or (is_Boosted_Flav_MEJ)){
                    //cout<<"DY MM CR event passes also other region"<<endl;    
                }
                 */   
                }
                
                FillHist(syst_name + "/Obj_PU_Corr_DYCR_Resolved_MM_pt", Resolve_DYCRMMpt, final_weight,100, 0., 1000.);
                FillHist(syst_name + "/Obj_PU_Corr_DYCR_Resolved_MM_leading_jet_pt", Resolve_DYCRMMleadjetpt, final_weight, 200, 0., 2000.);
                FillHist(syst_name + "/Obj_PU_Corr_DYCR_Resolved_MM_subleading_jet_pt", Resolve_DYCRMMsubleadjetpt, final_weight, 200, 0., 2000.);
                FillHist(syst_name + "/Obj_PU_Corr_DYCR_Resolved_MM_mlljj", Resolve_DYCRMMmlljj, final_weight, 800, 0., 8000.);
                FillHist(syst_name + "/Obj_PU_Corr_DYCR_Resolved_MM_leading_lep_pt", Resolve_DYCRMMleadleppt, final_weight, 2000, 0., 2000.);
                FillHist(syst_name + "/Obj_PU_Corr_DYCR_Resolved_MM_subleading_lep_pt", Resolve_DYCRMMsubleadleppt, final_weight, 2000, 0., 2000.);
            }
            //SR
            if (is_Resolved_SR_EE) {
                FillHist(syst_name + "/Obj_PU_Corr_SR_Resolved_EE_ll_pt", Resolve_SREEpt, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/Obj_PU_Corr_SR_Resolved_EE_leading_jet_pt", Resolve_SREEleadjetpt, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/Obj_PU_Corr_SR_Resolved_EE_subleading_jet_pt", Resolve_SREEsubleadjetpt, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/Obj_PU_Corr_SR_Resolved_EE_mlljj", Resolve_SREEmlljj, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/Obj_PU_Corr_SR_Resolved_EE_leading_lep_pt", Resolve_SREEleadleppt, final_weight, 2000, 0., 2000.);
                FillHist(syst_name + "/Obj_PU_Corr_SR_Resolved_EE_subleading_lep_pt", Resolve_SREEsubleadleppt, final_weight, 2000, 0., 2000.);
            }
            if(is_Resolved_SR_EE_SS) {
                FillHist(syst_name + "/Obj_PU_Corr_SR_Resolved_EE_SS_ll_pt", Resolve_SREEpt, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/Obj_PU_Corr_SR_Resolved_EE_SS_leading_jet_pt", Resolve_SREEleadjetpt, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/Obj_PU_Corr_SR_Resolved_EE_SS_subleading_jet_pt", Resolve_SREEsubleadjetpt, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/Obj_PU_Corr_SR_Resolved_EE_SS_mlljj", Resolve_SREEmlljj, final_weight, 8000, 0., 8000.);
            }
            if(is_Resolved_SR_EE_OS) {
                FillHist(syst_name + "/Obj_PU_Corr_SR_Resolved_EE_OS_ll_pt", Resolve_SREEpt, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/Obj_PU_Corr_SR_Resolved_EE_OS_leading_jet_pt", Resolve_SREEleadjetpt, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/Obj_PU_Corr_SR_Resolved_EE_OS_subleading_jet_pt", Resolve_SREEsubleadjetpt, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/Obj_PU_Corr_SR_Resolved_EE_OS_mlljj", Resolve_SREEmlljj, final_weight, 8000, 0., 8000.);
            }
            if (is_Resolved_SR_MM) {
                FillHist(syst_name + "/Obj_PU_Corr_SR_Resolved_MM_ll_pt", Resolve_SRMMpt, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/Obj_PU_Corr_SR_Resolved_MM_leading_jet_pt", Resolve_SRMMleadjetpt, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/Obj_PU_Corr_SR_Resolved_MM_subleading_jet_pt", Resolve_SRMMsubleadjetpt, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/Obj_PU_Corr_SR_Resolved_MM_mlljj", Resolve_SRMMmlljj, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/Obj_PU_Corr_SR_Resolved_MM_leading_lep_pt", Resolve_SRMMleadleppt, final_weight, 2000, 0., 2000.);
                FillHist(syst_name + "/Obj_PU_Corr_SR_Resolved_MM_subleading_lep_pt", Resolve_SRMMsubleadleppt, final_weight, 2000, 0., 2000.);
            }
            if (is_Resolved_SR_MM_SS) {
                FillHist(syst_name + "/Obj_PU_Corr_SR_Resolved_MM_SS_ll_pt", Resolve_SRMMpt, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/Obj_PU_Corr_SR_Resolved_MM_SS_leading_jet_pt", Resolve_SRMMleadjetpt, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/Obj_PU_Corr_SR_Resolved_MM_SS_subleading_jet_pt", Resolve_SRMMsubleadjetpt, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/Obj_PU_Corr_SR_Resolved_MM_SS_mlljj", Resolve_SRMMmlljj, final_weight, 8000, 0., 8000.);
            }
            if (is_Resolved_SR_MM_OS) {
                FillHist(syst_name + "/Obj_PU_Corr_SR_Resolved_MM_OS_ll_pt", Resolve_SRMMpt, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/Obj_PU_Corr_SR_Resolved_MM_OS_leading_jet_pt", Resolve_SRMMleadjetpt, final_weight, 8000,    0., 8000.);
                FillHist(syst_name + "/Obj_PU_Corr_SR_Resolved_MM_OS_subleading_jet_pt", Resolve_SRMMsubleadjetpt, final_weight, 8000,  0., 8000.); 
                //SS OS 필요 
            }

            //Flav
            if(is_Resolved_Flav_EM) {
                FillHist(syst_name + "/Obj_PU_Corr_Flav_CR_Resolved_EM_ll_pt", Resolve_FlavCRpt, final_weight, 1000, 0., 1000.);
                FillHist(syst_name + "/Obj_PU_Corr_Flav_CR_Resolved_EM_leading_jet_pt", Resolve_FlavCRleadjetpt, final_weight, 2000, 0., 2000.);
                FillHist(syst_name + "/Obj_PU_Corr_Flav_CR_Resolved_EM_subleading_jet_pt", Resolve_FlavCRsubleadjetpt, final_weight, 2000, 0., 2000.);
                FillHist(syst_name + "/Obj_PU_Corr_Flav_CR_Resolved_EM_mlljj", Resolve_FlavCRmlljj, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/Obj_PU_Corr_Flav_CR_Resolved_EM_leading_lep_pt", Resolve_FlavCRleadleppt, final_weight, 2000, 0., 2000.);
                FillHist(syst_name + "/Obj_PU_Corr_Flav_CR_Resolved_EM_subleading_lep_pt", Resolve_FlavCRsubleadleppt, final_weight, 2000, 0., 2000.);
            }
            //Boosted 
            
            //DY CR
            if(is_Boosted_DY_EE) {
                FillHist(syst_name + "/Obj_PU_pt(ll)_boosted_DY_CR_EE", Boost_DYCREEpt, final_weight, 1000, 0., 1000.);
                FillHist(syst_name + "/Obj_PU_leading_fatjet_pt_boosted_DY_CR_EE", Boost_DYCREEfatjetpt, final_weight, 2000, 0., 2000.);
                FillHist(syst_name + "/Obj_PU_m(lljj)_boosted_DY_CR_EE", Boost_DYCREEmlljj, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/Obj_PU_leading_lep_pt_boosted_DY_CR_EE", Boost_DYCREEleadleppt, final_weight, 2000, 0., 2000.);
                FillHist(syst_name + "/Obj_PU_subleading_lep_pt_boosted_DY_CR_EE", Boost_DYCREEsubleadleppt, final_weight, 2000, 0., 2000.);
            }
            if(is_Boosted_DY_MM) {
                FillHist(syst_name + "/Obj_PU_pt(ll)_boosted_DY_CR_MM", Boost_DYCRMMpt, final_weight, 1000, 0., 1000.);
                FillHist(syst_name + "/Obj_PU_leading_fatjet_pt_boosted_DY_CR_MM", Boost_DYCRMMfatjetpt, final_weight, 2000, 0., 2000.);
                FillHist(syst_name + "/Obj_PU_m(lljj)_boosted_DY_CR_MM", Boost_DYCRMMmlljj, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/Obj_PU_leading_lep_pt_boosted_DY_CR_MM", Boost_DYCRMMleadleppt, final_weight, 2000, 0., 2000.);
                FillHist(syst_name + "/Obj_PU_subleading_lep_pt_boosted_DY_CR_MM", Boost_DYCRMMsubleadleppt, final_weight, 2000, 0., 2000.);
            }

            //SR
            if(is_Boosted_SR_EE) {
                FillHist(syst_name + "/Obj_PU_boosted_Dilepton_Pt_ee", Boost_SREEpt, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/Obj_PU_leading_fatjet_pt_boosted_ee", Boost_SREEfatjetpt, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/Obj_PU_boosted_WRMass_ee", Boost_SREEmlljj, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/Obj_PU_boosted_LeadingLepPt_ee", Boost_SREEleadleppt, final_weight, 2000, 0., 2000.);
                FillHist(syst_name + "/Obj_PU_boosted_SubLeadingLepPt_ee", Boost_SREEsubleadleppt, final_weight, 2000, 0., 2000.);
            }
            if(is_Boosted_SR_EE_SS) {
                FillHist(syst_name + "/Obj_PU_SR_boosted_Dilepton_Pt_ee_SS", Boost_SREEpt, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/Obj_PU_SR_leading_fatjet_pt_boosted_ee_SS", Boost_SREEfatjetpt, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/Obj_PU_SR_boosted_WRMass_ee_SS", Boost_SREEmlljj, final_weight, 8000, 0., 8000.);
            }
            if(is_Boosted_SR_EE_OS) {
                FillHist(syst_name + "/Obj_PU_SR_boosted_Dilepton_Pt_ee_OS", Boost_SREEpt, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/Obj_PU_SR_leading_fatjet_pt_boosted_ee_OS", Boost_SREEfatjetpt, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/Obj_PU_SR_boosted_WRMass_ee_OS", Boost_SREEmlljj, final_weight, 8000, 0., 8000.);
            }
            if(is_Boosted_SR_MM) {

                FillHist(syst_name + "/Obj_PU_SR_boosted_Dilepton_Pt_mumu", Boost_SRMMpt, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/Obj_PU_SR_leading_fatjet_pt_boosted_mumu", Boost_SRMMfatjetpt, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/Obj_PU_SR_boosted_WRMass_mumu", Boost_SRMMmlljj, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/Obj_PU_boosted_LeadingLepPt_mumu", Boost_SRMMleadleppt, final_weight, 2000, 0., 2000.);
                FillHist(syst_name + "/Obj_PU_boosted_SubLeadingLepPt_mumu", Boost_SRMMsubleadleppt, final_weight, 2000, 0., 2000.);
            }
            if(is_Boosted_SR_MM_SS) {
                FillHist(syst_name + "/Obj_PU_SR_boosted_Dilepton_Pt_mumu_SS", Boost_SRMMpt, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/Obj_PU_SR_leading_fatjet_pt_boosted_mumu_SS", Boost_SRMMfatjetpt, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/Obj_PU_SR_boosted_WRMass_mumu_SS", Boost_SRMMmlljj, final_weight, 8000, 0., 8000.);
            }
            if(is_Boosted_SR_MM_OS) {
                FillHist(syst_name + "/Obj_PU_SR_boosted_Dilepton_Pt_mumu_OS", Boost_SRMMpt, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/Obj_PU_SR_leading_fatjet_pt_boosted_mumu_OS", Boost_SRMMfatjetpt, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/Obj_PU_SR_boosted_WRMass_mumu_OS", Boost_SRMMmlljj, final_weight, 8000, 0., 8000.);
            }
            
            // Flav
            if(is_Boosted_Flav_EMJ) {
                cout<<"ok3"<<endl;
                FillHist(syst_name + "/Obj_PU_pt(ll)_boosted_e_mujet_Flavor_CR", Boost_FlavEMJpt, final_weight, 1000, 0., 1000.);
                FillHist(syst_name + "/Obj_PU_leading_fatjet_pt_boosted_e_mujet_Flavor_CR", Boost_FlavEMJleadfatjetpt, final_weight, 2000, 0., 2000.);
                FillHist(syst_name + "/Obj_PU_m(lljj)_boosted_e_mujet_Flavor_CR", Boost_FlavEMJmlljj, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/Obj_PU_Boosted_Flavor_CR_e_mujet_Fatjet_SDMass", Boost_FlavEMJSDM , final_weight, 10000, 0., 10000.);
                FillHist(syst_name + "/Obj_PU_Boosted_Flavor_CR_e_mujet_LeadingLepPt", Boost_FlavEMJleadleppt , final_weight, 2000, 0., 2000.);
                FillHist(syst_name + "/Obj_PU_Boosted_Flavor_CR_e_mujet_SubLeadingLepPt", Boost_FlavEMJsubleadleppt , final_weight, 2000, 0., 2000.);

                FillHist(syst_name + "/Obj_PU_Boosted_Flavor_CR_e_mujet_LeadingLepEta", Boost_FlavEMJleadlepeta , final_weight, 100, -2.5, 2.5);
                FillHist(syst_name + "/Obj_PU_Boosted_Flavor_CR_e_mujet_SubLeadingLepEta", Boost_FlavEMJsubleadlepeta , final_weight, 100, -2.5, 2.5);
                FillHist(syst_name + "/Obj_PU_Boosted_Flavor_CR_e_mujet_Fatjet_Eta", Boost_FlavEMJfatjeteta , final_weight, 100, -2.5, 2.5);
                FillHist(syst_name + "/Obj_PU_Boosted_Flavor_CR_e_mujet_Fatjet_Phi", Boost_FlavEMJfatjetphi , final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/Obj_PU_Boosted_Flavor_CR e_mujet_DeltaR_LeadLep_Fatjet", Boost_FlavEMJdeltaR_leadlep_fatjet , final_weight, 100, 0., 5.);
                FillHist(syst_name + "/Obj_PU_Boosted_Flavor_CR e_mujet_LeadLep_LSF", Boost_FlavEMJleadlep_lsf , final_weight, 100, 0., 1.);    
                FillHist(syst_name + "/Obj_PU_Boosted_Flavor_CR e_mujet_mll", Boost_FlavEMJmll , final_weight, 1000, 0., 1000.);
                FillHist(syst_name + "/Obj_PU_Boosted_Flavor_CR e_mujet_Fatjet_pt", Boost_FlavEMJfatjetpt , final_weight, 1000, 0., 1000.);
                FillHist(syst_name + "/Obj_PU_Boosted_Flavor_CR e_mujet_WRpt", Boost_FlavEMJWRpt , final_weight, 1000, 0., 1000.);
            }
            if(is_Boosted_Flav_MEJ) {
                cout<<"ok33"<<endl;
                FillHist(syst_name + "/Obj_PU_pt(ll)_boosted_mu_ejets_Flavor_CR", Boost_FlavMEJpt, final_weight, 1000, 0., 1000.);
                FillHist(syst_name + "/Obj_PU_leading_fatjet_pt_boosted_mu_ejets_Flavor_CR", Boost_FlavMEJleadfatjetpt, final_weight, 2000, 0., 2000.);
                FillHist(syst_name + "/Obj_PU_m(lljj)_boosted_mu_ejets_Flavor_CR", Boost_FlavMEJmlljj, final_weight, 8000, 0., 8000.);
                FillHist(syst_name + "/Obj_PU_Boosted_Flavor_CR_mu_ejets_Fatjet_SDMass", Boost_FlavMEJSDM , final_weight, 10000, 0., 10000.);
                FillHist(syst_name + "/Obj_PU_Boosted_Flavor_CR_mu_ejets_LeadingLepPt", Boost_FlavMEJleadleppt , final_weight, 2000, 0., 2000.);
                FillHist(syst_name + "/Obj_PU_Boosted_Flavor_CR_mu_ejets_SubLeadingLepPt", Boost_FlavMEJsubleadleppt , final_weight, 2000, 0., 2000.);

                FillHist(syst_name + "/Obj_PU_Boosted_Flavor_CR_mu_ejets_LeadingLepEta", Boost_FlavMEJleadlepeta , final_weight, 100, -2.5, 2.5);
                FillHist(syst_name + "/Obj_PU_Boosted_Flavor_CR_mu_ejets_SubLeadingLepEta", Boost_FlavMEJsubleadlepeta , final_weight, 100, -2.5, 2.5);
                FillHist(syst_name + "/Obj_PU_Boosted_Flavor_CR_mu_ejets_Fatjet_Eta", Boost_FlavMEJfatjeteta , final_weight, 100, -2.5, 2.5);
                FillHist(syst_name + "/Obj_PU_Boosted_Flavor_CR_mu_ejets_Fatjet_Phi", Boost_FlavMEJfatjetphi , final_weight, 100, -3.14, 3.14);
                FillHist(syst_name + "/Obj_PU_Boosted_Flavor_CR mu_ejets_DeltaR_LeadLep_Fatjet", Boost_FlavMEJdeltaR_leadlep_fatjet , final_weight, 100, 0., 5.);
                FillHist(syst_name + "/Obj_PU_Boosted_Flavor_CR mu_ejets_LeadLep_LSF", Boost_FlavMEJleadlep_lsf , final_weight, 100, 0., 1.);    
                FillHist(syst_name + "/Obj_PU_Boosted_Flavor_CR mu_ejets_mll", Boost_FlavMEJmll , final_weight, 1000, 0., 1000.);
                FillHist(syst_name + "/Obj_PU_Boosted_Flavor_CR mu_ejets_Fatjet_pt", Boost_FlavMEJfatjetpt , final_weight, 1000, 0., 1000.);
                FillHist(syst_name + "/Obj_PU_Boosted_Flavor_CR mu_ejets_WRpt", Boost_FlavMEJWRpt , final_weight, 1000, 0., 1000.);
            }
            // syst_name: "Central", "PU_Weight_Up" 등
            // sf_val: 해당 케이스의 SF 값 (예: 0.99)
        }
    } 
    
    else { //DATA
        //resolved
            //DY CR
            if (is_Resolved_DY_EE) {
                FillHist(this_syst + "/Obj_PU_Corr_DYCR_Resolved_EE_pt", Resolve_DYCREEpt , 1,100, 0., 1000.);
                FillHist(this_syst + "/Obj_PU_Corr_DYCR_Resolved_EE_leading_jet_pt", Resolve_DYCREEleadjetpt, 1, 200, 0., 2000.);
                FillHist(this_syst + "/Obj_PU_Corr_DYCR_Resolved_EE_subleading_jet_pt", Resolve_DYCREEsubleadjetpt, 1, 200, 0., 2000.);
                FillHist(this_syst + "/Obj_PU_Corr_DYCR_Resolved_EE_mlljj", Resolve_DYCREEmlljj, 1, 800, 0., 8000.);
                FillHist(this_syst + "/Obj_PU_Corr_DYCR_Resolved_EE_leading_lep_pt", Resolve_DYCREEleadleppt, 1, 2000, 0., 2000.);
                FillHist(this_syst + "/Obj_PU_Corr_DYCR_Resolved_EE_subleading_lep_pt", Resolve_DYCREEsubleadleppt, 1, 2000, 0., 2000.);
            }
            if (is_Resolved_DY_MM) {
                FillHist(this_syst + "/Obj_PU_Corr_DYCR_Resolved_MM_pt", Resolve_DYCRMMpt, 1,100, 0., 1000.);
                FillHist(this_syst + "/Obj_PU_Corr_DYCR_Resolved_MM_leading_jet_pt", Resolve_DYCRMMleadjetpt, 1, 200, 0., 2000.);
                FillHist(this_syst + "/Obj_PU_Corr_DYCR_Resolved_MM_subleading_jet_pt", Resolve_DYCRMMsubleadjetpt, 1, 200, 0., 2000.);
                FillHist(this_syst + "/Obj_PU_Corr_DYCR_Resolved_MM_mlljj", Resolve_DYCRMMmlljj, 1, 800, 0., 8000.);
                FillHist(this_syst + "/Obj_PU_Corr_DYCR_Resolved_MM_leading_lep_pt", Resolve_DYCRMMleadleppt, 1, 2000, 0., 2000.);
                FillHist(this_syst + "/Obj_PU_Corr_DYCR_Resolved_MM_subleading_lep_pt", Resolve_DYCRMMsubleadleppt, 1, 2000, 0., 2000.);
            }
            //SR
            if (is_Resolved_SR_EE) {
                FillHist(this_syst + "/Obj_PU_Corr_SR_Resolved_EE_ll_pt", Resolve_SREEpt, 1, 8000, 0., 8000.);
                FillHist(this_syst + "/Obj_PU_Corr_SR_Resolved_EE_leading_jet_pt", Resolve_SREEleadjetpt, 1, 8000, 0., 8000.);
                FillHist(this_syst + "/Obj_PU_Corr_SR_Resolved_EE_subleading_jet_pt", Resolve_SREEsubleadjetpt, 1, 8000, 0., 8000.);
                FillHist(this_syst + "/Obj_PU_Corr_SR_Resolved_EE_mlljj", Resolve_SREEmlljj, 1, 8000, 0., 8000.);
                FillHist(this_syst + "/Obj_PU_Corr_SR_Resolved_EE_leading_lep_pt", Resolve_SREEleadleppt, 1, 2000, 0., 2000.);
                FillHist(this_syst + "/Obj_PU_Corr_SR_Resolved_EE_subleading_lep_pt", Resolve_SREEsubleadleppt, 1, 2000, 0., 2000.);
            }
            if(is_Resolved_SR_EE_SS) {
                FillHist(this_syst + "/Obj_PU_Corr_SR_Resolved_EE_SS_ll_pt", Resolve_SREEpt, 1, 8000, 0., 8000.);
                FillHist(this_syst + "/Obj_PU_Corr_SR_Resolved_EE_SS_leading_jet_pt", Resolve_SREEleadjetpt, 1, 8000, 0., 8000.);
                FillHist(this_syst + "/Obj_PU_Corr_SR_Resolved_EE_SS_subleading_jet_pt", Resolve_SREEsubleadjetpt, 1, 8000, 0., 8000.);
                FillHist(this_syst + "/Obj_PU_Corr_SR_Resolved_EE_SS_mlljj", Resolve_SREEmlljj, 1, 8000, 0., 8000.);
            }
            if(is_Resolved_SR_EE_OS) {
                FillHist(this_syst + "/Obj_PU_Corr_SR_Resolved_EE_OS_ll_pt", Resolve_SREEpt, 1, 8000, 0., 8000.);
                FillHist(this_syst + "/Obj_PU_Corr_SR_Resolved_EE_OS_leading_jet_pt", Resolve_SREEleadjetpt, 1, 8000, 0., 8000.);
                FillHist(this_syst + "/Obj_PU_Corr_SR_Resolved_EE_OS_subleading_jet_pt", Resolve_SREEsubleadjetpt, 1, 8000, 0., 8000.);
                FillHist(this_syst + "/Obj_PU_Corr_SR_Resolved_EE_OS_mlljj", Resolve_SREEmlljj, 1, 8000, 0., 8000.);
            }
            if (is_Resolved_SR_MM) {
                FillHist(this_syst + "/Obj_PU_Corr_SR_Resolved_MM_ll_pt", Resolve_SRMMpt, 1, 8000, 0., 8000.);
                FillHist(this_syst + "/Obj_PU_Corr_SR_Resolved_MM_leading_jet_pt", Resolve_SRMMleadjetpt, 1, 8000, 0., 8000.);
                FillHist(this_syst + "/Obj_PU_Corr_SR_Resolved_MM_subleading_jet_pt", Resolve_SRMMsubleadjetpt, 1, 8000, 0., 8000.);
                FillHist(this_syst + "/Obj_PU_Corr_SR_Resolved_MM_mlljj", Resolve_SRMMmlljj, 1, 8000, 0., 8000.);
                FillHist(this_syst + "/Obj_PU_Corr_SR_Resolved_MM_leading_lep_pt", Resolve_SRMMleadleppt, 1, 2000, 0., 2000.);
                FillHist(this_syst + "/Obj_PU_Corr_SR_Resolved_MM_subleading_lep_pt", Resolve_SRMMsubleadleppt, 1, 2000, 0., 2000.);
            }
            if (is_Resolved_SR_MM_SS) {
                FillHist(this_syst + "/Obj_PU_Corr_SR_Resolved_MM_SS_ll_pt", Resolve_SRMMpt, 1, 8000, 0., 8000.);
                FillHist(this_syst + "/Obj_PU_Corr_SR_Resolved_MM_SS_leading_jet_pt", Resolve_SRMMleadjetpt, 1, 8000, 0., 8000.);
                FillHist(this_syst + "/Obj_PU_Corr_SR_Resolved_MM_SS_subleading_jet_pt", Resolve_SRMMsubleadjetpt, 1, 8000, 0., 8000.);
                FillHist(this_syst + "/Obj_PU_Corr_SR_Resolved_MM_SS_mlljj", Resolve_SRMMmlljj, 1, 8000, 0., 8000.);
            }
            if (is_Resolved_SR_MM_OS) {
                FillHist(this_syst + "/Obj_PU_Corr_SR_Resolved_MM_OS_ll_pt", Resolve_SRMMpt, 1, 8000, 0., 8000.);
                FillHist(this_syst + "/Obj_PU_Corr_SR_Resolved_MM_OS_leading_jet_pt", Resolve_SRMMleadjetpt, 1, 8000,    0., 8000.);
                FillHist(this_syst + "/Obj_PU_Corr_SR_Resolved_MM_OS_subleading_jet_pt", Resolve_SRMMsubleadjetpt, 1, 8000,  0., 8000.); 
                //SS OS 필요 
            }

            //Flav
            if(is_Resolved_Flav_EM) {
                FillHist(this_syst + "/Obj_PU_Corr_Flav_CR_Resolved_EM_ll_pt", Resolve_FlavCRpt, 1, 1000, 0., 1000.);
                FillHist(this_syst + "/Obj_PU_Corr_Flav_CR_Resolved_EM_leading_jet_pt", Resolve_FlavCRleadjetpt, 1, 2000, 0., 2000.);
                FillHist(this_syst + "/Obj_PU_Corr_Flav_CR_Resolved_EM_subleading_jet_pt", Resolve_FlavCRsubleadjetpt, 1, 2000, 0., 2000.);
                FillHist(this_syst + "/Obj_PU_Corr_Flav_CR_Resolved_EM_mlljj", Resolve_FlavCRmlljj, 1, 8000, 0., 8000.);
                FillHist(this_syst + "/Obj_PU_Corr_Flav_CR_Resolved_EM_leading_lep_pt", Resolve_FlavCRleadleppt, 1, 2000, 0., 2000.);
                FillHist(this_syst + "/Obj_PU_Corr_Flav_CR_Resolved_EM_subleading_lep_pt", Resolve_FlavCRsubleadleppt, 1, 2000, 0., 2000.);
            }
            //Boosted 
            
            //DY CR
            if(is_Boosted_DY_EE) {
                FillHist(this_syst + "/Obj_PU_pt(ll)_boosted_DY_CR_EE", Boost_DYCREEpt, 1, 1000, 0., 1000.);
                FillHist(this_syst + "/Obj_PU_leading_fatjet_pt_boosted_DY_CR_EE", Boost_DYCREEfatjetpt, 1, 2000, 0., 2000.);
                FillHist(this_syst + "/Obj_PU_m(lljj)_boosted_DY_CR_EE", Boost_DYCREEmlljj, 1, 8000, 0., 8000.);
                FillHist(this_syst + "/Obj_PU_leading_lep_pt_boosted_DY_CR_EE", Boost_DYCREEleadleppt, 1, 2000, 0., 2000.);
                FillHist(this_syst + "/Obj_PU_subleading_lep_pt_boosted_DY_CR_EE", Boost_DYCREEsubleadleppt, 1, 2000, 0., 2000.);
            }
            if(is_Boosted_DY_MM) {
                FillHist(this_syst + "/Obj_PU_pt(ll)_boosted_DY_CR_MM", Boost_DYCRMMpt, 1, 1000, 0., 1000.);
                FillHist(this_syst + "/Obj_PU_leading_fatjet_pt_boosted_DY_CR_MM", Boost_DYCRMMfatjetpt, 1, 2000, 0., 2000.);
                FillHist(this_syst + "/Obj_PU_m(lljj)_boosted_DY_CR_MM", Boost_DYCRMMmlljj, 1, 8000, 0., 8000.);
                FillHist(this_syst + "/Obj_PU_leading_lep_pt_boosted_DY_CR_MM", Boost_DYCRMMleadleppt, 1, 2000, 0., 2000.);
                FillHist(this_syst + "/Obj_PU_subleading_lep_pt_boosted_DY_CR_MM", Boost_DYCRMMsubleadleppt, 1, 2000, 0., 2000.);
            }

            //SR
            if(is_Boosted_SR_EE) {
                FillHist(this_syst + "/Obj_PU_boosted_Dilepton_Pt_ee", Boost_SREEpt, 1, 8000, 0., 8000.);
                FillHist(this_syst + "/Obj_PU_leading_fatjet_pt_boosted_ee", Boost_SREEfatjetpt, 1, 8000, 0., 8000.);
                FillHist(this_syst + "/Obj_PU_boosted_WRMass_ee", Boost_SREEmlljj, 1, 8000, 0., 8000.);
                FillHist(this_syst + "/Obj_PU_boosted_LeadingLepPt_ee", Boost_SREEleadleppt, 1, 2000, 0., 2000.);
                FillHist(this_syst + "/Obj_PU_boosted_SubLeadingLepPt_ee", Boost_SREEsubleadleppt, 1, 2000, 0., 2000.);
            }
            if(is_Boosted_SR_EE_SS) {
                FillHist(this_syst + "/Obj_PU_SR_boosted_Dilepton_Pt_ee_SS", Boost_SREEpt, 1, 8000, 0., 8000.);
                FillHist(this_syst + "/Obj_PU_SR_leading_fatjet_pt_boosted_ee_SS", Boost_SREEfatjetpt, 1, 8000, 0., 8000.);
                FillHist(this_syst + "/Obj_PU_SR_boosted_WRMass_ee_SS", Boost_SREEmlljj, 1, 8000, 0., 8000.);
            }
            if(is_Boosted_SR_EE_OS) {
                FillHist(this_syst + "/Obj_PU_SR_boosted_Dilepton_Pt_ee_OS", Boost_SREEpt, 1, 8000, 0., 8000.);
                FillHist(this_syst + "/Obj_PU_SR_leading_fatjet_pt_boosted_ee_OS", Boost_SREEfatjetpt, 1, 8000, 0., 8000.);
                FillHist(this_syst + "/Obj_PU_SR_boosted_WRMass_ee_OS", Boost_SREEmlljj, 1, 8000, 0., 8000.);
            }
            if(is_Boosted_SR_MM) {
                FillHist(this_syst + "/Obj_PU_SR_boosted_Dilepton_Pt_mumu", Boost_SRMMpt, 1, 8000, 0., 8000.);
                FillHist(this_syst + "/Obj_PU_SR_leading_fatjet_pt_boosted_mumu", Boost_SRMMfatjetpt, 1, 8000, 0., 8000.);
                FillHist(this_syst + "/Obj_PU_SR_boosted_WRMass_mumu", Boost_SRMMmlljj, 1, 8000, 0., 8000.);
                FillHist(this_syst + "/Obj_PU_boosted_LeadingLepPt_mumu", Boost_SRMMleadleppt, 1, 2000, 0., 2000.);
                FillHist(this_syst + "/Obj_PU_boosted_SubLeadingLepPt_mumu", Boost_SRMMsubleadleppt, 1, 2000, 0., 2000.);
            }
            if(is_Boosted_SR_MM_SS) {
                FillHist(this_syst + "/Obj_PU_SR_boosted_Dilepton_Pt_mumu_SS", Boost_SRMMpt, 1, 8000, 0., 8000.);
                FillHist(this_syst + "/Obj_PU_SR_leading_fatjet_pt_boosted_mumu_SS", Boost_SRMMfatjetpt, 1, 8000, 0., 8000.);
                FillHist(this_syst + "/Obj_PU_SR_boosted_WRMass_mumu_SS", Boost_SRMMmlljj, 1, 8000, 0., 8000.);
            }
            if(is_Boosted_SR_MM_OS) {
                FillHist(this_syst + "/Obj_PU_SR_boosted_Dilepton_Pt_mumu_OS", Boost_SRMMpt, 1, 8000, 0., 8000.);
                FillHist(this_syst + "/Obj_PU_SR_leading_fatjet_pt_boosted_mumu_OS", Boost_SRMMfatjetpt, 1, 8000, 0., 8000.);
                FillHist(this_syst + "/Obj_PU_SR_boosted_WRMass_mumu_OS", Boost_SRMMmlljj, 1, 8000, 0., 8000.);
            }
            
            // Flav
            if(is_Boosted_Flav_EMJ) {
                FillHist(this_syst + "/Obj_PU_pt(ll)_boosted_e_mujet_Flavor_CR", Boost_FlavEMJpt, 1, 1000, 0., 1000.);
                FillHist(this_syst + "/Obj_PU_leading_fatjet_pt_boosted_e_mujet_Flavor_CR", Boost_FlavEMJleadfatjetpt, 1, 2000, 0., 2000.);
                FillHist(this_syst + "/Obj_PU_m(lljj)_boosted_e_mujet_Flavor_CR", Boost_FlavEMJmlljj, 1, 8000, 0., 8000.);
                FillHist(this_syst + "/Obj_PU_Boosted_Flavor_CR_e_mujet_Fatjet_SDMass", Boost_FlavEMJSDM , 1, 10000, 0., 10000.);
                FillHist(this_syst + "/Obj_PU_Boosted_Flavor_CR_e_mujet_LeadingLepPt", Boost_FlavEMJleadleppt , 1, 2000, 0., 2000.);
                FillHist(this_syst + "/Obj_PU_Boosted_Flavor_CR_e_mujet_SubLeadingLepPt", Boost_FlavEMJsubleadleppt , 1, 2000, 0., 2000.);

                FillHist(this_syst + "/Obj_PU_Boosted_Flavor_CR_e_mujet_LeadingLepEta", Boost_FlavEMJleadlepeta , 1, 100, -2.5, 2.5);
                FillHist(this_syst + "/Obj_PU_Boosted_Flavor_CR_e_mujet_SubLeadingLepEta", Boost_FlavEMJsubleadlepeta , 1, 100, -2.5, 2.5);
                FillHist(this_syst + "/Obj_PU_Boosted_Flavor_CR_e_mujet_Fatjet_Eta", Boost_FlavEMJfatjeteta , 1, 100, -2.5, 2.5);
                FillHist(this_syst + "/Obj_PU_Boosted_Flavor_CR_e_mujet_Fatjet_Phi", Boost_FlavEMJfatjetphi , 1, 100, -3.14, 3.14);
                FillHist(this_syst + "/Obj_PU_Boosted_Flavor_CR_e_mujet_DeltaR_LeadLep_Fatjet", Boost_FlavEMJdeltaR_leadlep_fatjet , 1, 100, 0., 5.);
                FillHist(this_syst + "/Obj_PU_Boosted_Flavor_CR_e_mujet_LeadLep_LSF", Boost_FlavEMJleadlep_lsf , 1, 100, 0., 1.);    
                FillHist(this_syst + "/Obj_PU_Boosted_Flavor_CR_e_mujet_mll", Boost_FlavEMJmll , 1, 1000, 0., 1000.);
                FillHist(this_syst + "/Obj_PU_Boosted_Flavor_CR_e_mujet_Fatjet_pt", Boost_FlavEMJfatjetpt , 1, 1000, 0., 1000.);
                FillHist(this_syst + "/Obj_PU_Boosted_Flavor_CR_e_mujet_WRpt", Boost_FlavEMJWRpt , 1, 1000, 0., 1000.);
            }
            if(is_Boosted_Flav_MEJ) {
                FillHist(this_syst + "/Obj_PU_pt(ll)_boosted_mu_ejets_Flavor_CR", Boost_FlavMEJpt, 1, 1000, 0., 1000.);
                FillHist(this_syst + "/Obj_PU_leading_fatjet_pt_boosted_mu_ejets_Flavor_CR", Boost_FlavMEJleadfatjetpt, 1, 2000, 0., 2000.);
                FillHist(this_syst + "/Obj_PU_m(lljj)_boosted_mu_ejets_Flavor_CR", Boost_FlavMEJmlljj, 1, 8000, 0., 8000.);
                FillHist(this_syst + "/Obj_PU_Boosted_Flavor_CR_mu_ejets_Fatjet_SDMass", Boost_FlavMEJSDM , 1, 10000, 0., 10000.);
                FillHist(this_syst + "/Obj_PU_Boosted_Flavor_CR_mu_ejets_LeadingLepPt", Boost_FlavMEJleadleppt , 1, 2000, 0., 2000.);
                FillHist(this_syst + "/Obj_PU_Boosted_Flavor_CR_mu_ejets_SubLeadingLepPt", Boost_FlavMEJsubleadleppt , 1, 2000, 0., 2000.);

                FillHist(this_syst + "/Obj_PU_Boosted_Flavor_CR_mu_ejets_LeadingLepEta", Boost_FlavMEJleadlepeta , 1, 100, -2.5, 2.5);
                FillHist(this_syst + "/Obj_PU_Boosted_Flavor_CR_mu_ejets_SubLeadingLepEta", Boost_FlavMEJsubleadlepeta , 1, 100, -2.5, 2.5);
                FillHist(this_syst + "/Obj_PU_Boosted_Flavor_CR_mu_ejets_Fatjet_Eta", Boost_FlavMEJfatjeteta , 1, 100, -2.5, 2.5);
                FillHist(this_syst + "/Obj_PU_Boosted_Flavor_CR_mu_ejets_Fatjet_Phi", Boost_FlavMEJfatjetphi , 1, 100, -3.14, 3.14);
                FillHist(this_syst + "/Obj_PU_Boosted_Flavor_CR_mu_ejets_DeltaR_LeadLep_Fatjet", Boost_FlavMEJdeltaR_leadlep_fatjet , 1, 100, 0., 5.);
                FillHist(this_syst + "/Obj_PU_Boosted_Flavor_CR_mu_ejets_LeadLep_LSF", Boost_FlavMEJleadlep_lsf , 1, 100, 0., 1.);    
                FillHist(this_syst + "/Obj_PU_Boosted_Flavor_CR_mu_ejets_mll", Boost_FlavMEJmll , 1, 1000, 0., 1000.);
                FillHist(this_syst + "/Obj_PU_Boosted_Flavor_CR_mu_ejets_Fatjet_pt", Boost_FlavMEJfatjetpt , 1, 1000, 0., 1000.);
                FillHist(this_syst + "/Obj_PU_Boosted_Flavor_CR_mu_ejets_WRpt", Boost_FlavMEJWRpt , 1, 1000, 0., 1000.);
            }
            // syst_name: "Central", "PU_Weight_Up" 등
    }
}


    // double counting check?  # 1670

    // Higmass info # 1722 

    // veto HEM  ? # 1748 

    // Fill histograms for main variables # 1771 
    // end ## 1984
    

bool Reproduce20_002_copy::Electrons::isPassCustomTightID(const Electron& el, const Reproduce20_002_copy::Electrons& eset) const {
    if (fabs(el.scEta()) < 1.566) {
        return el.PassID(eset.Electron_Tight_ID[0]);
    }
    int heepbit = el.VidNestedWPBitmapHEEP();
    if ( !((heepbit & 3775) == 3775) ) return false;
    float scE =  (el.scEtOverPt()+1) * el.Pt();
    double cutValue_HoverE = ( -0.4 + 0.4 * fabs(el.scEta()) ) * el.rho() / scE + 0.05;
    if(! (el.hoe()<cutValue_HoverE) ) return false;

    //==== new EM+Had_depth1 cut
    //double cutValue_emhaddep1 = UncorrPt() > 50. ? 2.5 + 0.03 * (UncorrPt()-50.) +                        0.28 * Rho() : 2.5 +                        0.28 * Rho(); // original cut
    double cutValue_emhaddep1 = el.Pt() > 50. ? 2.5 + 0.03 * (el.Pt()-50.) + (0.15 + 0.07*fabs(el.scEta())) * el.rho() : 2.5 + (0.15 + 0.07*fabs(el.scEta())) * el.rho();
    if(! ( el.dr03EcalRecHitSumEt() + el.dr03HcalDepth1TowerSumEt() < cutValue_emhaddep1 ) ) return false;
    return true;
}

bool Reproduce20_002_copy::Electrons::isPassCustomLooseID(const Electron& el) const {
    //if (!(el.hoe() < 0.5)) return false;

    if (fabs(el.scEta()) <= 1.479){
    
        if(!(el.hoe() < 0.05 + 1.16/el.E() + 0.0324*el.rho()/el.E())) return false;
        if (!(el.sieie() < 0.0112)) return false;
        if (!(fabs(el.deltaEtaInSeed()) < 0.00377)) return false;
        if (!(fabs(el.deltaPhiInSC()) < 0.0884)) return false;
        if (!(fabs(el.eInvMinusPInv()) < 0.193)) return false;
        if (!(el.LostHits() <= 1)) return false;
        if (!(el.ConvVeto())) return false;
        return true;
    }
    else {
        if(!(el.hoe() < 0.0441 + 2.54/el.E() + 0.183*el.rho()/el.E())) return false;
        if (!(el.sieie() < 0.0425)) return false;
        if (!(fabs(el.deltaEtaInSeed()) < 0.00674)) return false;
        if (!(fabs(el.deltaPhiInSC()) < 0.169)) return false;
        if (!(fabs(el.eInvMinusPInv()) < 0.111)) return false;
        if (!(el.LostHits() <= 1)) return false;
        if (!(el.ConvVeto())) return false;
        return true;
    }
    return true ;
}

bool Reproduce20_002_copy::Electrons::isPassLooseNoIso(const Electron& el) const {
    // Matches Python selectLooseElectrons logic:
    // Evaluate vidNestedWPBitmap with id_level=2 (Loose WP), ignoring isolation (cut index 7)
    //
    // vidNestedWPBitmap contains 10 cuts, each encoded in 3 bits:
    //   0: MinPtCut
    //   1: GsfEleSCEtaMultiRangeCut
    //   2: GsfEleDEtaInSeedCut
    //   3: GsfEleDPhiInCut
    //   4: GsfEleFull5x5SigmaIEtaIEtaCut
    //   5: GsfEleHadronicOverEMEnergyScaledCut
    //   6: GsfEleEInverseMinusPInverseCut
    //   7: GsfEleRelPFIsoScaledCut (ISOLATION - ignored)
    //   8: GsfEleConversionVetoCut
    //   9: GsfEleMissingHitsCut
    //
    // Each 3-bit value: 0=fail, 1=Veto, 2=Loose, 3=Medium, 4=Tight

    int bitmap = el.VidNestedWPBitmap();

    const int n_cuts = 10;
    const int bits_per_cut = 3;
    const int ignore_cut = 7;      // Isolation cut index
    const int id_level = 2;        // Loose working point
    const int mask = (1 << bits_per_cut) - 1;  // 0b111 = 7

    for (int cut_nr = 0; cut_nr < n_cuts; cut_nr++) {
        if (cut_nr == ignore_cut) continue;  // Skip isolation

        int value = (bitmap >> (cut_nr * bits_per_cut)) & mask;
        if (value < id_level) return false;
    }
    return true;
}

RVec<FatJet> Reproduce20_002_copy::Clean_Fatjet_with_tight_leptons(const RVec<FatJet> & fatjets, const RVec<Lepton *> & tight_leps) {
    RVec<FatJet> cleanedfatjets;
    for (unsigned int i=0 ; i< fatjets.size(); i ++) {
        FatJet fatjet = fatjets.at(i);
        bool isDRtoLepton(false);
        for (unsigned int j=0 ; j< tight_leps.size(); j ++) {
            Lepton * lep = tight_leps.at(j);
            if ( fatjet.DeltaR(*lep) < 0.4 ) {
                isDRtoLepton = true;
                break;
            }
        }
        if (!isDRtoLepton) {
            cleanedfatjets.push_back(fatjet);
        }
    }
    return cleanedfatjets;
}

RVec<Jet> Reproduce20_002_copy::Clean_jet_with_loose_leptons(const RVec<Jet> & jets, const RVec<Lepton *> & loose_leps) {
    RVec<Jet> cleanedjets;
    for (unsigned int i=0 ; i< jets.size(); i ++) {
        Jet jet = jets.at(i);
        bool isDRtoLepton(false);
        for (unsigned int j=0 ; j< loose_leps.size(); j ++) {
            Lepton * lep = loose_leps.at(j);
            if ( jet.DeltaR(*lep) < 0.4 ) {
                isDRtoLepton = true;
                break;
            }
        }
        if (!isDRtoLepton) {
            cleanedjets.push_back(jet);
        }
    }
    return cleanedjets;
}

RVec<Jet> Reproduce20_002_copy::Clean_LSF_FatJet_with_jets(const RVec<FatJet> & fatjets, const RVec<Jet> & jets) {
    RVec<Jet> cleanedjets;
    for (unsigned int i=0 ; i< jets.size(); i ++) {
        Jet jet = jets.at(i);
        bool isDRtoFatJet(false);
        for (unsigned int j=0 ; j< fatjets.size(); j ++) {
            FatJet fatjet = fatjets.at(j);
            if ( fatjet.DeltaR(jet) < 0.8 ) {
                isDRtoFatJet = true;
                break;
            }
        }
        if (!isDRtoFatJet) {
            cleanedjets.push_back(jet);
        }
    }
    return cleanedjets;
}

RVec<FatJet> Reproduce20_002_copy::Clean_Jets_with_fatjets(const RVec<Jet> & jets, const RVec<FatJet> & fatjets) {
    RVec<FatJet> cleanedfatjets;
    for (unsigned int i=0 ; i< fatjets.size(); i ++) {
        FatJet fatjet = fatjets.at(i);
        bool isDRtoJet(false);
        for (unsigned int j=0 ; j< jets.size(); j ++) {
            Jet jet = jets.at(j);
            if ( fatjet.DeltaR(jet) < 0.8 ) {
                isDRtoJet = true;
                break;
            }
        }
        if (!isDRtoJet) {
            cleanedfatjets.push_back(fatjet);
        }
    }
    return cleanedfatjets;
}


