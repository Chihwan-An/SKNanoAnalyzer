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
    
    if (DataEra == "2022")
    {
        mu_set.Muon_Trigger = {"HLT_Mu50", "HLT_CascadeMu100", "HLT_HighPtTkMu100"}; 
        mu_set.Muon_Trigger_Safe_Pt_Cut = 52.;
        el_set.Ele_Trigger = {"HLT_Ele32_WPTight_Gsf","HLT_Photon200","HLT_Ele115_CaloIdVT_GsfTrkIdT"};
        el_set.Ele_Trigger_Safe_Pt_Cut = 35.;  
    }
    if (DataEra == "2022EE")
    {
        mu_set.Muon_Trigger = {"HLT_Mu50", "HLT_CascadeMu100", "HLT_HighPtTkMu100"}; 
        mu_set.Muon_Trigger_Safe_Pt_Cut = 52.;
        el_set.Ele_Trigger = {"HLT_Ele32_WPTight_Gsf","HLT_Photon200","HLT_Ele115_CaloIdVT_GsfTrkIdT"};
        el_set.Ele_Trigger_Safe_Pt_Cut = 35.; 
    }
    if (DataEra == "2023")
    {
        mu_set.Muon_Trigger = {"HLT_Mu50", "HLT_CascadeMu100", "HLT_HighPtTkMu100"}; 
        mu_set.Muon_Trigger_Safe_Pt_Cut = 52.;
        el_set.Ele_Trigger = {"HLT_Ele32_WPTight_Gsf","HLT_Photon200","HLT_Ele115_CaloIdVT_GsfTrkIdT"};
        el_set.Ele_Trigger_Safe_Pt_Cut = 35.; 
    }
    if (DataEra == "2023BPix")
    {
        mu_set.Muon_Trigger = {"HLT_Mu50", "HLT_CascadeMu100", "HLT_HighPtTkMu100"}; 
        mu_set.Muon_Trigger_Safe_Pt_Cut = 52.;
        el_set.Ele_Trigger = {"HLT_Ele32_WPTight_Gsf","HLT_Photon200","HLT_Ele115_CaloIdVT_GsfTrkIdT"};
        el_set.Ele_Trigger_Safe_Pt_Cut = 35.; 
    }

    myCorr = new MyCorrection(DataEra, DataPeriod, IsDATA ? DataStream : MCSample, IsDATA);

    
    // Initialize systematic helper
    string SKNANO_HOME = getenv("SKNANO_HOME");
    if (IsDATA) {
        systHelper = std::make_unique<SystematicHelper>(SKNANO_HOME + "/docs/noSyst.yaml", DataStream, DataEra);
    } else {
        systHelper = std::make_unique<SystematicHelper>(SKNANO_HOME + "/docs/ExampleSystematic.yaml", MCSample, DataEra);
    }

}

void Reproduce20_002_copy::executeEvent() {

    el_set.AllElectrons =  GetAllElectrons();
    mu_set.AllMuons = GetAllMuons();
    jet_set.AllJets = GetAllJets();
    fatjet_set.AllFatJets = GetAllFatJets();
    gen_set.gens = GetAllGens();
    // no tune p muon 
    

    // DY pt reweight
    if(MCSample.Contains("DY")){
    // wtf ?
    }
    for (const auto &syst_dummy : *systHelper) {
        executeEventFromParameter();
    }
    // Nvtx ( PU )


    //Parameter -no function of parameter -> use struct 
    

    //Run syst  # 457




}

void Reproduce20_002_copy::executeEventFromParameter() {
    const TString this_syst = systHelper->getCurrentSysName();

    Event ev = GetEvent();
    Particle METv = ev.GetMETVector(Event::MET_Type::PUPPI,Event::MET_Syst::CENTRAL);
    float PU_Weight = myCorr->GetPUWeight(ev.nTrueInt(),MyCorrection::variation::nom,this_syst);
    float weight = 1.0;
    float norm_weight = 1.0;
    if(!IsDATA){
        weight *= MCweight();
        weight *= ev.GetTriggerLumi("Full");
        weight *= PU_Weight;
        norm_weight *= MCweight();
        norm_weight *= ev.GetTriggerLumi("Full");
        // weight_norm_1invpb??

        // Z pt reweight

        // signal -> Kfactor 
    }

    //  No cut 

    FillHist(this_syst + "/CutFlow", 0.0, weight, 20,-10,10.); // Initial event


    //Event selection 
    RVec<Electron> electrons = el_set.AllElectrons;
    RVec<Muon> muons = mu_set.AllMuons;
    RVec<Jet> jets = jet_set.AllJets;
    RVec<FatJet> fatjets = fatjet_set.AllFatJets;
    if (!PassNoiseFilter(jets,ev,Event::MET_Type::PUPPI)) return;

    FillHist(this_syst + "/CutFlow", 1.0, weight, 20,-10,10.); // Noise filter pass
    //float MuonIDSF = myCorr->GetMuonIDSF("NUM_HighPtID_DEN_GlobalMuonProbes", muons, MyCorrection::variation::nom);
    bool pass_trig_muon = ev.PassTrigger(mu_set.Muon_Trigger);
    bool pass_trig_elec = ev.PassTrigger(el_set.Ele_Trigger);

    FillHist(this_syst + "/CutFlow", 2.0, weight, 20,-10,10.); // HLT pass

    
    





    std::string nom = "nom";
    ///         Leptons       ///
    /*
    for (unsigned int i=0 ; i< muons.size(); i ++) 
        {
        Muon & mu = muons.at(i);
        myCorr->GetMuonScaleSF(mu,nom,mu.Pt());
    }    
    */
    RVec<Electron> my_electrons = SelectElectrons(electrons, "NOCUT" , el_set.Electron_MinPt, 2.4); //ID in 429
    RVec<Muon> my_muons = SelectMuons(muons, "NOCUT" , mu_set.Muon_MinPt, 2.4); //ID in 446
    
    RVec<FatJet> fatjet_list ;
    RVec<FatJet> lsf ;

    FillHist(this_syst + "/Fatjet_num_total", fatjets.size() , weight, 10, 0., 10.);
    
    for (unsigned int i=0 ; i< fatjets.size(); i ++) {
        FatJet & fj = fatjets.at(i);
        if ((fj.Pt() > fatjet_set.FatJet_MinPt) && (abs(fj.Eta())<fatjet_set.FatJet_MaxEta) && (fj.SDMass() > fatjet_set.FatJet_SDM) ) {
            FillHist(this_syst + "fatjet_cutflow", 1 , weight, 10, 0., 10.);
            if (fj.PassID(fatjet_set.FatJet_ID)) return;
                
                fatjet_list.push_back(fj);
                FillHist(this_syst + "/Fatjet_num_preLSF", 1 , weight, 10, 0., 10.);
                FillHist(this_syst + "/Fatjet_lsf_preLSF", fj.LSF3() , weight, 100, 0., 1.);
                if (fj.LSF3() >fatjet_set.Fatjet_LSF) {
                    lsf.push_back(fj);
                    FillHist(this_syst + "/Fatjet_num_LSF", 1 , weight, 10, 0., 10.);

                //}
            }
        }
    }
    fatjets = fatjet_list;
    RVec<FatJet> fatjets_LSF = lsf;

    sort (fatjets.begin(), fatjets.end(), PtComparing);
    sort (fatjets_LSF.begin(), fatjets_LSF.end(), PtComparing);
    
    

    //prompt only 구현 필요    
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
        // Loose ID
        if (el_set.isPassCustomLooseID(el)){
            Loose_electrons.push_back(&el);
            Loose_leps_el.push_back(&el);
            Loose_leps.push_back(&el);
            if (!el.PassID(el_set.Electron_Tight_ID[0])) {
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
    sort (Tight_leps.begin(), Tight_leps.end(), PtComparingPtr);

    int n_Loose_leptons  = Loose_electrons.size() + Loose_muons.size();
    int n_Tight_leptons  = Tight_electrons.size() + Tight_muons.size();
    FillHist(this_syst + "/N_Loose_Lepton", n_Loose_leptons , weight, 10, 0., 10.);
    FillHist(this_syst + "/N_Tight_Lepton", n_Tight_leptons , weight, 10, 0., 10.);
    FillHist(this_syst + "/Tightleps_pt_lead", Tight_leps.size() >0 ? Tight_leps[0]->Pt() : 0. , weight, 100, 0., 500.);
    FillHist(this_syst + "/Tightleps_pt_sublead", Tight_leps.size() >1 ? Tight_leps[1]->Pt() : 0. , weight, 100, 0., 500.);


    ///         Jets       ///



    // sepration lepton - jets 
    jet_set.cleanedjet_with_tight_leptons = Clean_jet_with_tight_leptons(jet_set.AllJets, Tight_leps);
    

    FillHist(this_syst + "/Non_Selected_Jetnum", jets.size(), 1.0, 20, 0., 20.);
    RVec<Jet> selected_jets = SelectJets(jet_set.cleanedjet_with_tight_leptons, jet_set.Jet_ID[0] , jet_set.Jet_MinPt, jet_set.Jet_MaxEta);
    sort (selected_jets.begin(), selected_jets.end(), PtComparing);
    FillHist(this_syst + "/Selected_Jetnum", selected_jets.size(), 1.0, 20,-10,10.);

    ///         FatJets       ///
    

    // # of bjets 
    /*
    int NBJets=0;
    for(unsigned int i=0; i<jets.size(); i++){
        if( mcCorr->IsBTagged_2a(JetTagging::Parameters(JetTagging::DeepCSV, JetTagging::Medium, JetTagging::incl, JetTagging::comb), jets.at(i), SystDir_BTag) ){
        NBJets++;
        }
    */


    // sum of pt 
    double HT(0.);
    for(const auto &jet: jet_set.AllJets){
        HT += jet.Pt();
    }


    // Requires 2 tight leptons , l1 > 60 
    // Def of resolved event 
    bool IsResolvedEvent = false;
    bool this_trigger_pass(false);
    bool tmp_isEE(false), tmp_isMM(false), tmp_isEM(false);
    if ( (n_Tight_leptons == 2 ) && (Tight_leps[0]->Pt() > 60.0)  && (Tight_leps[1]->Pt() > 53.0)) {
        
        FillHist(this_syst + "/CutFlow", 3.0, weight, 20,-10,10.); // 2 tight leptons with pT cut
        
        if ( (Tight_electrons.size() == 2) && ( Tight_muons.size() == 0 )) {
            this_trigger_pass = pass_trig_elec;
            tmp_isEE = true;
            FillHist(this_syst + "/tightmuons", 1 , weight, 5, 0., 5.);
        }
        else if ( (Tight_muons.size() == 2) && ( Tight_electrons.size() == 0 )) {
            this_trigger_pass = pass_trig_muon;
            tmp_isMM = true;
            FillHist(this_syst + "/tightmuons", 2 , weight, 5, 0., 5.);
        }
        else if ( (Tight_muons.size() == 1) && ( Tight_electrons.size() == 1 )) {
            this_trigger_pass = pass_trig_muon;
            tmp_isEM = true;
            FillHist(this_syst + "/tightmuons", 3 , weight, 5, 0., 5.);
        }

        if (this_trigger_pass) {
            // needs 2 jets 
            Lepton *LeadLep = Tight_leps[0];
            Lepton *SubLeadLep = Tight_leps[1];
            
            float LeadLepCharge = LeadLep->Charge();
            float SubLeadLepCharge = SubLeadLep->Charge();

            bool dRLeadJetLepon = (selected_jets[0].DeltaR(*Tight_leps[0]) > 0.4) && (selected_jets[0].DeltaR(*Tight_leps[1]) > 0.4);
            bool dRSubLeadJetLepon = (selected_jets[1].DeltaR(*Tight_leps[0]) > 0.4) && (selected_jets[1].DeltaR(*Tight_leps[1]) > 0.4);
            bool dRTwoLetpton = (LeadLep->DeltaR(*SubLeadLep) > 0.4);
            bool dRTwoJets = (selected_jets[0].DeltaR(selected_jets[1]) > 0.4);
            FillHist(this_syst + "/Jetnumber_before_resolved_selection1", selected_jets.size(), weight, 20,-10,10.);
            //if (selected_jets.size() >= 2 ) {
                FillHist(this_syst + "/Jetnumber_before_resolved_selection2", selected_jets.size(), weight, 20,-10,10.);
                FillHist(this_syst + "/CutFlow", 4.0, weight, 20,-10,10.); // 2 jets 
                if ((selected_jets.size() >= 2 )&&(dRLeadJetLepon)&&(dRSubLeadJetLepon)&&(dRTwoLetpton)&&(dRTwoJets)) { 
                    FillHist(this_syst + "/Jetnumber_before_resolved_selection3", selected_jets.size(), weight, 20,-10,10.);
                    IsResolvedEvent = true;
                    FillHist(this_syst + "/CutFlow", 5.0, weight, 20,-10,10.); // 2 jets
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
                // e , mu id , reco , ISO SF
                
                // Trigger SF
                // DY reshape ?
                // saving plots for each CR 
                
                // DY CR 60 < ll < 150 , lljj > 800 
                if ( DiLepMassLT150 && WRCand.M() > 800.0 ) {
                    FillHist(this_syst + "/CutFlow", 6.0, weight, 20,-10,10.); // DY CR pass
                    FillHist(this_syst + "/DYCR_Resolved_ll_pt", dilepton_pt, weight, 100, 0., 1000.);
                    FillHist(this_syst + "/DYCR_Resolved_leading_jet_pt", selected_jets[0].Pt(), weight, 200, 0., 2000.);
                    FillHist(this_syst + "/DYCR_Resolved_subleading_jet_pt", selected_jets[1].Pt(), weight, 200, 0., 2000.);
                    FillHist(this_syst + "/DYCR_Resolved_mlljj", WRCand.M(), weight, 800, 0., 8000.);
                    if (tmp_isEE) {
                        //PU weight 적용
                        FillHist(this_syst + "/DYCR_Resolved_EE_pt", dilepton_pt, weight, 100, 0., 1000.);
                        FillHist(this_syst + "/DYCR_Resolved_EE_leading_jet_pt", selected_jets[0].Pt(), weight, 200, 0., 2000.);
                        FillHist(this_syst + "/DYCR_Resolved_EE_subleading_jet_pt", selected_jets[1].Pt(), weight, 200, 0., 2000.);
                        FillHist(this_syst + "/DYCR_Resolved_EE_mlljj", WRCand.M(), weight, 800, 0., 8000.);
                        //no pu weight
                        FillHist(this_syst + "/No_PU_DYCR_Resolved_EE_pt", dilepton_pt, norm_weight, 100, 0., 1000.);
                        FillHist(this_syst + "/No_PU_DYCR_Resolved_EE_leading_jet_pt", selected_jets[0].Pt(), norm_weight, 200, 0., 2000.);
                        FillHist(this_syst + "/No_PU_DYCR_Resolved_EE_subleading_jet_pt", selected_jets[1].Pt(), norm_weight, 200, 0., 2000.);
                        FillHist(this_syst + "/No_PU_DYCR_Resolved_EE_mlljj", WRCand.M(), norm_weight, 800, 0., 8000.);
                    }
                    else if (tmp_isMM) {
                        FillHist(this_syst + "/DYCR_Resolved_MM_pt",dilepton_pt, weight, 100, 0., 1000.);
                        FillHist(this_syst + "/DYCR_Resolved_MM_leading_jet_pt", selected_jets[0].Pt(), weight, 200, 0., 2000.);
                        FillHist(this_syst + "/DYCR_Resolved_MM_subleading_jet_pt", selected_jets[1].Pt(), weight, 200, 0., 2000.);
                        FillHist(this_syst + "/DYCR_Resolved_MM_mlljj", WRCand.M(), weight, 800, 0., 8000.);

                        FillHist(this_syst + "/No_PU_DYCR_Resolved_MM_pt",dilepton_pt, norm_weight, 100, 0., 1000.);
                        FillHist(this_syst + "/No_PU_DYCR_Resolved_MM_leading_jet_pt", selected_jets[0].Pt(), norm_weight, 200, 0., 2000.);
                        FillHist(this_syst + "/No_PU_DYCR_Resolved_MM_subleading_jet_pt", selected_jets[1].Pt(), norm_weight, 200, 0., 2000.);
                        FillHist(this_syst + "/No_PU_DYCR_Resolved_MM_mlljj", WRCand.M(), norm_weight, 800, 0., 8000.);
                    }
                    else if (tmp_isEM) {
                        FillHist(this_syst + "/DYCR_Resolved_EM_pt", dilepton_pt, weight, 100, 0., 1000.);
                        FillHist(this_syst + "/DYCR_Resolved_EM_leading_jet_pt", selected_jets[0].Pt(), weight, 200, 0., 2000.);
                        FillHist(this_syst + "/DYCR_Resolved_EM_subleading_jet_pt", selected_jets[1].Pt(), weight, 200, 0., 2000.);
                        FillHist(this_syst + "/DYCR_Resolved_EM_mlljj", WRCand.M(), weight, 800, 0., 8000.);

                        FillHist(this_syst + "/No_PU_DYCR_Resolved_EM_pt", dilepton_pt, norm_weight, 100, 0., 1000.);
                        FillHist(this_syst + "/No_PU_DYCR_Resolved_EM_leading_jet_pt", selected_jets[0].Pt(), norm_weight, 200, 0., 2000.);
                        FillHist(this_syst + "/No_PU_DYCR_Resolved_EM_subleading_jet_pt", selected_jets[1].Pt(), norm_weight, 200, 0., 2000.);
                        FillHist(this_syst + "/No_PU_DYCR_Resolved_EM_mlljj", WRCand.M(), norm_weight, 800, 0., 8000.);
                    }
                }
            // DY CR1 ( 60 < mll < 100)
                if ( DiLepMass60to100 && WRCand.M() > 800.0 ){
                    if (tmp_isEE)
                    {
                        FillHist(this_syst + "/DYCR1_Resolved_EE_ll_pt", dilepton_pt, weight, 100, 0., 1000.);
                        FillHist(this_syst + "/DYCR1_Resolved_EE_leading_jet_pt", selected_jets[0].Pt(), weight, 200, 0., 2000.);
                        FillHist(this_syst + "/DYCR1_Resolved_EE_subleading_jet_pt", selected_jets[1].Pt(), weight, 200, 0., 2000.);
                        FillHist(this_syst + "/DYCR1_Resolved_EE_mlljj", WRCand.M(), weight, 800, 0., 8000.);
                    }
                    if (tmp_isMM)
                    {
                        FillHist(this_syst + "/DYCR1_Resolved_MM_ll_pt", dilepton_pt, weight, 100, 0., 1000.);
                        FillHist(this_syst + "/DYCR1_Resolved_MM_leading_jet_pt", selected_jets[0].Pt(), weight, 200, 0., 2000.);
                        FillHist(this_syst + "/DYCR1_Resolved_MM_subleading_jet_pt", selected_jets[1].Pt(), weight, 200, 0., 2000.);
                        FillHist(this_syst + "/DYCR1_Resolved_MM_mlljj", WRCand.M(), weight, 800, 0., 8000.);
                    }
                }
            // DY CR2 ( 100 < mll < 150)
                if ( DiLepMass100to150 && WRCand.M() > 800.0 ){
                    if (tmp_isEE)
                    {
                        FillHist(this_syst + "/DYCR2_Resolved_EE_ll_pt", dilepton_pt, weight, 100, 0., 1000.);
                        FillHist(this_syst + "/DYCR2_Resolved_EE_leading_jet_pt", selected_jets[0].Pt(), weight, 200, 0., 2000.);
                        FillHist(this_syst + "/DYCR2_Resolved_EE_subleading_jet_pt", selected_jets[1].Pt(), weight, 200, 0., 2000.);
                        FillHist(this_syst + "/DYCR2_Resolved_EE_mlljj", WRCand.M(), weight, 800, 0., 8000.);
                    }
                    if (tmp_isMM)
                    {
                        FillHist(this_syst + "/DYCR2_Resolved_MM_ll_pt", dilepton_pt, weight, 100, 0., 1000.);
                        FillHist(this_syst + "/DYCR2_Resolved_MM_leading_jet_pt", selected_jets[0].Pt(), weight, 200, 0., 2000.);
                        FillHist(this_syst + "/DYCR2_Resolved_MM_subleading_jet_pt", selected_jets[1].Pt(), weight, 200, 0., 2000.);
                        FillHist(this_syst + "/DYCR2_Resolved_MM_mlljj", WRCand.M(), weight, 800, 0., 8000.);
                    }
                }
                // Flavor CR
                if ( DiLepMassGT400 && WRCand.M() > 800.0 ){
                    if (tmp_isEM)
                    {
                        FillHist(this_syst + "/CutFlow", 6.0 , weight, 20,-10,10.); // Flavor CR pass
                        FillHist(this_syst + "/FlavorCR_Resolved_ll_pt", dilepton_pt, weight, 100, 0., 1000.);
                        FillHist(this_syst + "/FlavorCR_Resolved_leading_jet_pt", selected_jets[0].Pt(), weight, 200, 0., 2000.);
                        FillHist(this_syst + "/FlavorCR_Resolved_subleading_jet_pt", selected_jets[1].Pt(), weight, 200, 0., 2000.);
                        FillHist(this_syst + "/FlavorCR_Resolved_mlljj", WRCand.M(), weight, 800, 0., 8000.);

                        FillHist(this_syst + "/NO_PU_FlavorCR_Resolved_ll_pt", dilepton_pt, norm_weight, 100, 0., 1000.);
                        FillHist(this_syst + "/NO_PU_FlavorCR_Resolved_leading_jet_pt", selected_jets[0].Pt(), norm_weight, 200, 0., 2000.);
                        FillHist(this_syst + "/NO_PU_FlavorCR_Resolved_subleading_jet_pt", selected_jets[1].Pt(), norm_weight, 200, 0., 2000.);
                        FillHist(this_syst + "/NO_PU_FlavorCR_Resolved_mlljj", WRCand.M(), norm_weight, 800, 0., 8000.);
                    }
                }
                // low mass CR 
                if (DiLepMassGT200 && WRCand.M() < 800.0 ){
                    if (tmp_isEE)
                    {
                        FillHist(this_syst + "/LowMassCR_Resolved_EE_ll_pt", dilepton_pt, weight, 100, 0., 1000.);
                        FillHist(this_syst + "/LowMassCR_Resolved_leading_jet_pt", selected_jets[0].Pt(), weight, 200, 0., 2000.);
                        FillHist(this_syst + "/LowMassCR_Resolved_subleading_jet_pt", selected_jets[1].Pt(), weight, 200, 0., 2000.);
                        FillHist(this_syst + "/LowMassCR_Resolved_mlljj", WRCand.M(), weight, 800, 0., 8000.);
                    }
                    if (tmp_isMM)
                    {
                        FillHist(this_syst + "/LowMassCR_Resolved_MM_ll_pt", dilepton_pt, weight, 100, 0., 1000.);
                        FillHist(this_syst + "/LowMassCR_Resolved_leading_jet_pt", selected_jets[0].Pt(), weight, 200, 0., 2000.);
                        FillHist(this_syst + "/LowMassCR_Resolved_subleading_jet_pt", selected_jets[1].Pt(), weight, 200, 0., 2000.);
                        FillHist(this_syst + "/LowMassCR_Resolved_mlljj", WRCand.M(), weight, 800, 0., 8000.);
                    }
                    }
                
                //Resovled SR
                if (DiLepMassGT400 && WRCand.M() > 800.0) {
                    if (!IsDATA){
                        if (tmp_isEE){
                            FillHist(this_syst + "/SR_Resolved_EE_ll_pt", dilepton_pt, weight, 8000, 0., 8000.);
                            FillHist(this_syst + "/SR_Resolved_EE_leading_jet_pt", selected_jets[0].Pt(), weight, 8000, 0., 8000.);
                            FillHist(this_syst + "/SR_Resolved_EE_subleading_jet_pt", selected_jets[1].Pt(), weight, 8000, 0., 8000.);
                            FillHist(this_syst + "/SR_Resolved_EE_mlljj", WRCand.M(), weight, 8000, 0., 8000.);

                            FillHist(this_syst + "/NO_PU_SR_Resolved_EE_ll_pt", dilepton_pt, norm_weight, 8000, 0., 8000.);
                            FillHist(this_syst + "/NO_PU_SR_Resolved_EE_leading_jet_pt", selected_jets[0].Pt(), norm_weight, 8000, 0., 8000.);
                            FillHist(this_syst + "/NO_PU_SR_Resolved_EE_subleading_jet_pt", selected_jets[1].Pt(), norm_weight, 8000, 0., 8000.);
                            FillHist(this_syst + "/NO_PU_SR_Resolved_EE_mlljj", WRCand.M(), norm_weight, 8000, 0., 8000.);
                            // charge
                            if ( LeadLepCharge * SubLeadLepCharge > 0 ) {
                                FillHist(this_syst + "/SR_Resolved_EE_ll_pt_SS", dilepton_pt, weight, 8000, 0., 8000.);
                                FillHist(this_syst + "/SR_Resolved_EE_leading_jet_pt_SS", selected_jets[0].Pt(), weight, 8000, 0., 8000.);
                                FillHist(this_syst + "/SR_Resolved_EE_subleading_jet_pt_SS", selected_jets[1].Pt(), weight, 8000, 0., 8000.);
                                FillHist(this_syst + "/SR_Resolved_EE_mlljj_SS", WRCand.M(), weight, 8000, 0., 8000.);
                            }
                            else {
                                FillHist(this_syst + "/SR_Resolved_EE_ll_pt_OS", dilepton_pt, weight, 8000, 0., 8000.);
                                FillHist(this_syst + "/SR_Resolved_EE_leading_jet_pt_OS", selected_jets[0].Pt(), weight, 8000, 0., 8000.);
                                FillHist(this_syst + "/SR_Resolved_EE_subleading_jet_pt_OS", selected_jets[1].Pt(), weight, 8000, 0., 8000.);
                                FillHist(this_syst + "/SR_Resolved_EE_mlljj_OS", WRCand.M(), weight, 8000, 0., 8000.);
                            }
                        }
                        if (tmp_isMM){
                            FillHist(this_syst + "/SR_Resolved_MM_ll_pt", dilepton_pt, weight, 8000, 0., 8000.);
                            FillHist(this_syst + "/SR_Resolved_MM_leading_jet_pt", selected_jets[0].Pt(), weight, 8000, 0., 8000.);
                            FillHist(this_syst + "/SR_Resolved_MM_subleading_jet_pt", selected_jets[1].Pt(), weight, 8000, 0., 8000.);
                            FillHist(this_syst + "/SR_Resolved_MM_mlljj", WRCand.M(), weight, 8000, 0., 8000.);

                            FillHist(this_syst + "/NO_PU_SR_Resolved_MM_ll_pt", dilepton_pt, norm_weight, 8000, 0., 8000.);
                            FillHist(this_syst + "/NO_PU_SR_Resolved_MM_leading_jet_pt", selected_jets[0].Pt(), norm_weight, 8000, 0., 8000.);
                            FillHist(this_syst + "/NO_PU_SR_Resolved_MM_subleading_jet_pt", selected_jets[1].Pt(), norm_weight, 8000, 0., 8000.);
                            FillHist(this_syst + "/NO_PU_SR_Resolved_MM_mlljj", WRCand.M(), norm_weight, 8000, 0., 8000.);
                        // charge
                            if ( LeadLepCharge * SubLeadLepCharge > 0 ) {
                                FillHist(this_syst + "/SR_Resolved_MM_ll_pt_SS", dilepton_pt, weight, 8000, 0., 8000.);
                                FillHist(this_syst + "/SR_Resolved_MM_leading_jet_pt_SS", selected_jets[0].Pt(), weight, 8000, 0., 8000.);
                                FillHist(this_syst + "/SR_Resolved_MM_subleading_jet_pt_SS", selected_jets[1].Pt(), weight, 8000, 0., 8000.);
                                FillHist(this_syst + "/SR_Resolved_MM_mlljj_SS", WRCand.M(), weight, 8000, 0., 8000.);
                            }
                            else {
                                FillHist(this_syst + "/SR_Resolved_MM_ll_pt_OS", dilepton_pt, weight, 8000, 0., 8000.);
                                FillHist(this_syst + "/SR_Resolved_MM_leading_jet_pt_OS", selected_jets[0].Pt(), weight, 8000, 0., 8000.);
                                FillHist(this_syst + "/SR_Resolved_MM_subleading_jet_pt_OS", selected_jets[1].Pt(), weight, 8000, 0., 8000.);
                                FillHist(this_syst + "/SR_Resolved_MM_mlljj_OS", WRCand.M(), weight, 8000, 0., 8000.);
                        }
                    }
                }
            }// # 1192 -> Resolved ends 
            }// dr < 0.4
        //} // 2jets 
    }
    }
    
    
    if (!IsResolvedEvent){
        FillHist(this_syst + "/Boost_tightlepnum", n_Tight_leptons , weight, 10, 0., 10.);
        FillHist(this_syst + "/Boost_cutflow_DY", 1 , weight, 20,-10,10.);
        FillHist(this_syst + "/Boost_cutflow_FLV", 1 , weight, 20,-10,10.);
        if ((n_Tight_leptons >0 ) && (Tight_leps[0]->Pt() > 60.0)) {
            FillHist(this_syst + "/Boost_cutflow_DY", 2 , weight, 20,-10,10.);
            FillHist(this_syst + "/Boost_cutflow_FLV", 2 , weight, 20,-10,10.);
            bool this_trigger_pass_boost(false);
            bool is_tmp_lead_el(false), is_tmp_lead_mu(false);
            Lepton * LeadLep = Tight_leps[0];
            if ( LeadLep->IsElectron() ) {
                is_tmp_lead_el = true;
                this_trigger_pass_boost = pass_trig_elec;
            }
            else if ( LeadLep->IsMuon()){
                is_tmp_lead_mu = true;
                this_trigger_pass_boost = pass_trig_muon;
                FillHist(this_syst + "/Check_is_tmp_lead_muon_ok", 1 , weight, 5, 0., 5.);
            }
            
            if (this_trigger_pass_boost){
                FillHist(this_syst + "/Boost_cutflow_DY", 3 , weight,  20,-10,10.);
                FillHist(this_syst + "/Boost_cutflow_FLV", 3 , weight, 20,-10,10.);
                RVec<Lepton *> Loose_SF_leps = is_tmp_lead_el ? Loose_leps_el : Loose_leps_mu;
                RVec<Lepton *> Loose_OF_leps = is_tmp_lead_el ? Loose_leps_mu : Loose_leps_el;
                Lepton * LowMllLooseLepton ; 
                bool has_lowmll(false);
                float lowmllmass ;
                //check num of SF loose lepton for low mll num
                int n_loose_sf_lep = 0;
                for (unsigned int i=0 ; i< Loose_SF_leps.size(); i ++) {
                    if (Loose_SF_leps[i] == LeadLep) continue;
                    double dilep_mass = (*LeadLep + *Loose_SF_leps[i]).M();
                    if ((dilep_mass > 60) && (dilep_mass < 150 )) {
                        n_loose_sf_lep++;
                    }
                }
                FillHist(this_syst + "/Boost_SF_looselepton_num", n_loose_sf_lep , weight, 10, 0., 10.);

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
                                bool looselepton_infatjet = true;
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
                                if (is_tmp_lead_el){ // ee 
                                        FillHist(this_syst + "/pt(ll)_boosted_DY_CR_EE", (*LeadLep + *LowMllLooseLepton).Pt(), weight, 1000, 0., 1000.);
                                        FillHist(this_syst + "/leading_fatjet_pt_boosted_DY_CR_EE", HNFatJet.Pt(), weight, 2000, 0., 2000.);
                                        FillHist(this_syst + "/m(lljj)_boosted_DY_CR_EE", WRCand.M(), weight, 8000, 0., 8000.);

                                        FillHist(this_syst + "/NO_PU_pt(ll)_boosted_DY_CR_EE", (*LeadLep + *LowMllLooseLepton).Pt(), norm_weight, 1000, 0., 1000.);
                                        FillHist(this_syst + "/leading_fatjet_pt_boosted_DY_CR_EE", HNFatJet.Pt(), norm_weight, 2000, 0., 2000.);
                                        FillHist(this_syst + "/m(lljj)_boosted_DY_CR_EE", WRCand.M(), norm_weight, 8000, 0., 8000.);
                                        if (looselepton_infatjet){
                                            FillHist(this_syst + "/Boosted_DY_CR_EE_looselepton_infatjet_Fatjet_SDMass", HNFatJet.SDMass() , weight, 10000, 0., 10000.);
                                        }
                                        else{
                                            FillHist(this_syst + "/Boosted_DY_CR_EE_looselepton_outsidefatjet_Fatjet_SDMass", HNFatJet.SDMass() , weight, 10000, 0., 10000.);
                                        }
                                    }
                                    if (is_tmp_lead_mu){// mumu
                                        FillHist(this_syst + "/pt(ll)_boosted_DY_CR_MM", (*LeadLep + *LowMllLooseLepton).Pt(), weight, 1000, 0., 1000.);
                                        FillHist(this_syst + "/leading_fatjet_pt_boosted_DY_CR_MM", HNFatJet.Pt(), weight, 2000, 0., 2000.);
                                        FillHist(this_syst + "/m(lljj)_boosted_DY_CR_MM", WRCand.M(), weight, 8000, 0., 8000.);

                                        FillHist(this_syst + "/NO_PU_pt(ll)_boosted_DY_CR_MM", (*LeadLep + *LowMllLooseLepton).Pt(), norm_weight, 1000, 0., 1000.);
                                        FillHist(this_syst + "/NO_PU_leading_fatjet_pt_boosted_DY_CR_MM", HNFatJet.Pt(), norm_weight, 2000, 0., 2000.);
                                        FillHist(this_syst + "/NO_PU_m(lljj)_boosted_DY_CR_MM", WRCand.M(), norm_weight, 8000, 0., 8000.);
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
                                        FillHist(this_syst + "/pt(ll)_boosted_DY_CR1_EE", (*LeadLep + *LowMllLooseLepton).Pt(), weight, 1000, 0., 1000.);
                                        FillHist(this_syst + "/leading_fatjet_pt_boosted_DY_CR1_EE", HNFatJet.Pt(), weight, 2000, 0., 2000.);
                                        FillHist(this_syst + "/m(lljj)_boosted_DY_CR1_EE", WRCand.M(), weight, 8000, 0., 8000.);
                                    }
                                    if (is_tmp_lead_mu){// mumu
                                        FillHist(this_syst + "/pt(ll)_boosted_DY_CR1_MM", (*LeadLep + *LowMllLooseLepton).Pt(), weight, 1000, 0., 1000.);
                                        FillHist(this_syst + "/leading_fatjet_pt_boosted_DY_CR1_MM", HNFatJet.Pt(), weight, 2000, 0., 2000.);
                                        FillHist(this_syst + "/m(lljj)_boosted_DY_CR1_MM", WRCand.M(), weight, 8000, 0., 8000.);
                                    }
                                }
                                else{ // 100 < lowmllmass < 150
                                    if (is_tmp_lead_el){ // ee 
                                        FillHist(this_syst + "/pt(ll)_boosted_DY_CR2_EE", (*LeadLep + *LowMllLooseLepton).Pt(), weight, 1000, 0., 1000.);
                                        FillHist(this_syst + "/leading_fatjet_pt_boosted_DY_CR2_EE", HNFatJet.Pt(), weight, 2000, 0., 2000.);
                                        FillHist(this_syst + "/m(lljj)_boosted_DY_CR2_EE", WRCand.M(), weight, 8000, 0., 8000.);
                                    }
                                    if (is_tmp_lead_mu){// mumu
                                        FillHist(this_syst + "/pt(ll)_boosted_DY_CR2_MM", (*LeadLep + *LowMllLooseLepton).Pt(), weight, 1000, 0., 1000.);
                                        FillHist(this_syst + "/leading_fatjet_pt_boosted_DY_CR2_MM", HNFatJet.Pt(), weight, 2000, 0., 2000.);
                                        FillHist(this_syst + "/m(lljj)_boosted_DY_CR2_MM", WRCand.M(), weight, 8000, 0., 8000.);
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
                    FillHist(this_syst + "/Boost_cutflow_FLV", 4 , weight, 20,-10,10.);
                    bool hasawaymergedfatjet = false;
                    FatJet Ncand;
                    Particle WRCand;
                    FatJet HNFatJet;
                    int num_of_fatjet_lsf_dphi_cut = 0;
                    for (unsigned int i=0 ; i<fatjets_LSF.size(); i++) {
                        FatJet this_fatjet = fatjets_LSF.at(i);
                        if (abs( LeadLep->DeltaPhi(this_fatjet))>2.0) {
                            num_of_fatjet_lsf_dphi_cut += 1;
                        }
                    }
                    FillHist(this_syst + "/num_of_fatjet_lsf_dphi_cut", num_of_fatjet_lsf_dphi_cut , weight, 10, 0., 10.);
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
                        FillHist(this_syst + "/Boost_cutflow_FLV", 5 , weight, 20,-10,10.);
                        bool hassflooselepton(false);
                        bool hasoflooselepton(false);
                        Lepton *SFLooseLepton;
                        Lepton *OFLooseLepton;
                        FillHist(this_syst +"/numofhnfatjet_FLV", HNFatJet.SDMass() , weight, 10000, 0., 10000.); 
                    //num SF loose lepton inside fatjet for high mll
                        int num_of_sf_looselepton_infatjet = 0;
                        for (unsigned int k=0 ; k< Loose_SF_leps.size(); k ++) {
                            if (LeadLep->DeltaR( *Loose_SF_leps[k])<0.01) continue;
                            if ( Loose_SF_leps.at(k)->Pt() < 53.0) continue;
                            if (HNFatJet.DeltaR(*Loose_SF_leps[k]) < 0.8) {
                                num_of_sf_looselepton_infatjet += 1;
                            }
                        }
                        
                        FillHist(this_syst + "/num_of_sf_looselepton", num_of_sf_looselepton_infatjet , weight, 10, 0., 10.);
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
                        int num_of_of_looselepton_infatjet = 0;
                        for (unsigned int m=0 ; m< Loose_OF_leps.size(); m ++) {
                            if (LeadLep->DeltaR( *Loose_OF_leps[m])<0.01) continue;
                            if ( Loose_OF_leps.at(m)->Pt() < 53.0) continue;
                            if (HNFatJet.DeltaR(*Loose_OF_leps[m]) < 0.8) {
                                num_of_of_looselepton_infatjet -= 1;
                            }
                        }
                        float of_sf_looselepton_both = num_of_of_looselepton_infatjet * num_of_sf_looselepton_infatjet;
                        if (of_sf_looselepton_both == 0){
                            FillHist(this_syst + "/num_of_both_of_looselepton",num_of_of_looselepton_infatjet , weight, 10, 0., 10.);
                        }
                        FillHist(this_syst + "/of_sf_looselepton_both", of_sf_looselepton_both , weight, 20, -10., 10.);

                        FillHist(this_syst + "/num_of_of_looselepton", num_of_of_looselepton_infatjet , weight, 10, 0., 10.);
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
                            // tight fatjet 밖 한개 , loose lepton same flavor 안에 
                            FillHist(this_syst + "/Boost_cutflow_FLV", 6 , weight, 20, -10., 10.);
                            if (hassflooselepton) {
                                FillHist(this_syst + "/Boost_cutflow_FLV", 7 , weight, 20,-10,10.);
                                if (!hasoflooselepton){
                                    FillHist(this_syst + "/Boost_cutflow_FLV", 8 , weight, 20,-10,10.);
                        
                        //    if(tmp_IsLeadM){
                        //==== In this case, the loose ID is HighPt ID muon.
                        //==== we want to apply the lepton scale factors to these muons
                        //Muon *looseMuon = (Muon *)SFLooseLepton;
                        //ForSF_muons.push_back( looseMuon );
                        //    }
                                    if ( (*LeadLep + *SFLooseLepton).M() >200.0 ) {
                                        //charge 
                                        float LeadLepCharge = LeadLep->Charge();
                                        float SFLooseLeptonCharge = SFLooseLepton->Charge();
                                        FillHist(this_syst + "/Boost_cutflow_FLV", 9 , weight, 20,-10,10.);
                                        // Boosted DY SR
                                        if (WRMassGT800) {
                                            if(!IsDATA){
                                                if (is_tmp_lead_el) {
                                                    FillHist(this_syst + "/SR_boosted_Dilepton_Pt_ee", (*LeadLep + *SFLooseLepton).Pt(), weight, 8000, 0., 8000.);
                                                    FillHist(this_syst + "/SR_leading_fatjet_pt_boosted_ee", HNFatJet.Pt(), weight, 8000, 0., 8000.);
                                                    FillHist(this_syst + "/SR_boosted_WRMass_ee", WRCand.M(), weight, 8000, 0., 8000.);
                                                    //charge 
                                                    if ( LeadLepCharge * SFLooseLeptonCharge > 0 ) {
                                                        FillHist(this_syst + "/SR_boosted_Dilepton_Pt_ee_SS", (*LeadLep + *SFLooseLepton).Pt(), weight, 8000, 0., 8000.);
                                                        FillHist(this_syst + "/SR_leading_fatjet_pt_boosted_ee_SS", HNFatJet.Pt(), weight, 8000, 0., 8000.);
                                                        FillHist(this_syst + "/SR_boosted_WRMass_ee_SS", WRCand.M(), weight, 8000, 0., 8000.);
                                                    }
                                                    else {
                                                        FillHist(this_syst + "/SR_boosted_Dilepton_Pt_ee_OS", (*LeadLep + *SFLooseLepton).Pt(), weight, 8000, 0., 8000.);
                                                        FillHist(this_syst + "/SR_leading_fatjet_pt_boosted_ee_OS", HNFatJet.Pt(), weight, 8000, 0., 8000.);
                                                        FillHist(this_syst + "/SR_boosted_WRMass_ee_OS", WRCand.M(), weight, 8000, 0., 8000.);
                                                    }
                                                }
                                            else if (is_tmp_lead_mu) {
                                                    FillHist(this_syst + "/SR_boosted_Dilepton_Pt_mumu", (*LeadLep + *SFLooseLepton).Pt(), weight, 8000, 0., 8000.);
                                                    FillHist(this_syst + "/SR_leading_fatjet_pt_boosted_mumu", HNFatJet.Pt(), weight, 8000, 0., 8000.);
                                                    FillHist(this_syst + "/SR_boosted_WRMass_mumu", WRCand.M(), weight, 8000, 0., 8000.);
                                                    //charge
                                                    if ( LeadLepCharge * SFLooseLeptonCharge > 0 ) {
                                                        FillHist(this_syst + "/SR_boosted_Dilepton_Pt_mumu_SS", (*LeadLep + *SFLooseLepton).Pt(), weight, 8000, 0., 8000.);
                                                        FillHist(this_syst + "/SR_leading_fatjet_pt_boosted_mumu_SS", HNFatJet.Pt(), weight, 8000, 0., 8000.);
                                                        FillHist(this_syst + "/SR_boosted_WRMass_mumu_SS", WRCand.M(), weight, 8000, 0., 8000.);
                                                    }
                                                    else {
                                                        FillHist(this_syst + "/SR_boosted_Dilepton_Pt_mumu_OS", (*LeadLep + *SFLooseLepton).Pt(), weight, 8000, 0., 8000.);
                                                        FillHist(this_syst + "/SR_leading_fatjet_pt_boosted_mumu_OS", HNFatJet.Pt(), weight, 8000, 0., 8000.);
                                                        FillHist(this_syst + "/SR_boosted_WRMass_mumu_OS", WRCand.M(), weight, 8000, 0., 8000.);
                                                    }
                                                }                                            
                                            }
                                        }
                                        else{ // low wr CR
                                            if (is_tmp_lead_el) {
                                                // Boosted low WR ee CR
                                                FillHist(this_syst + "/pt(ll)_boosted_low_WR_ee_Flavor_CR", (*LeadLep + *SFLooseLepton).Pt(), weight, 1000, 0., 1000.);
                                                FillHist(this_syst + "/leading_fatjet_pt_boosted_low_WR_ee_Flavor_CR", HNFatJet.Pt(), weight, 2000, 0., 2000.);
                                                FillHist(this_syst + "/m(lljj)_boosted_low_WR_ee_Flavor_CR", WRCand.M(), weight, 8000, 0., 8000.);
                                            }
                                            else if (is_tmp_lead_mu) {
                                                // Boosted low WR mumu CR
                                                FillHist(this_syst + "/pt(ll)_boosted_low_WR_mumu_Flavor_CR", (*LeadLep + *SFLooseLepton).Pt(), weight, 1000, 0., 1000.);
                                                FillHist(this_syst + "/leading_fatjet_pt_boosted_low_WR_mumu_Flavor_CR", HNFatJet.Pt(), weight, 2000, 0., 2000.);
                                                FillHist(this_syst + "/m(lljj)_boosted_low_WR_mumu_Flavor_CR", WRCand.M(), weight, 8000, 0., 8000.);
                                            }
                                        }
                                        // Boosted DY CR
                                    }
                                }
                            }

                    // tight fatjet 밖 한개 , loose lepton oppo flavor 안에
                        if (!hassflooselepton){
                            FillHist(this_syst + "/Boost_cutflow_FLV", -6 , weight, 20, -10, 10.);
                            if (hasoflooselepton){
                                // Boosted Flavor CR
                                if ((*LeadLep + *OFLooseLepton).M() > 200.0) {
                                    FillHist(this_syst + "/Boost_cutflow_FLV", -7 , weight, 20, -10, 10.);
                                    if (WRMassGT800) {
                                        FillHist(this_syst + "/Boost_cutflow_FLV", -8 , weight, 20, -10, 10.);
                                        if (is_tmp_lead_el) {
                                            // Boosted Flavor CR
                                            FillHist(this_syst + "/pt(ll)_boosted_e_mujet_Flavor_CR", (*LeadLep + *OFLooseLepton).Pt(), weight, 1000, 0., 1000.);
                                            FillHist(this_syst + "/leading_fatjet_pt_boosted_e_mujet_Flavor_CR", HNFatJet.Pt(), weight, 2000, 0., 2000.);
                                            FillHist(this_syst + "/m(lljj)_boosted_e_mujet_Flavor_CR", WRCand.M(), weight, 8000, 0., 8000.);
                                            FillHist(this_syst + "/Boosted_Flavor_CR_e_mujet_Fatjet_SDMass", HNFatJet.SDMass() , weight, 10000, 0., 10000.);

                                            FillHist(this_syst + "/NO_PU_pt(ll)_boosted_e_mujet_Flavor_CR", (*LeadLep + *OFLooseLepton).Pt(), norm_weight, 1000, 0., 1000.);
                                            FillHist(this_syst + "/NO_PU_leading_fatjet_pt_boosted_e_mujet_Flavor_CR", HNFatJet.Pt(), norm_weight, 2000, 0., 2000.);
                                            FillHist(this_syst + "/NO_PU_m(lljj)_boosted_e_mujet_Flavor_CR", WRCand.M(), norm_weight, 8000, 0., 8000.);
                                            FillHist(this_syst + "/NO_PU_Boosted_Flavor_CR_e_mujet_Fatjet_SDMass", HNFatJet.SDMass() , norm_weight, 10000, 0., 10000.);
                                        }
                                        else if (is_tmp_lead_mu) {
                                            // Boosted Flavor CR
                                            FillHist(this_syst + "/pt(ll)_boosted_mu_ejets_Flavor_CR", (*LeadLep + *OFLooseLepton).Pt(), weight, 1000, 0., 1000.);
                                            FillHist(this_syst + "/leading_fatjet_pt_boosted_mu_ejets_Flavor_CR", HNFatJet.Pt(), weight, 2000, 0., 2000.);
                                            FillHist(this_syst + "/m(lljj)_boosted_mu_ejets_Flavor_CR", WRCand.M(), weight, 8000, 0., 8000.);
                                            FillHist(this_syst + "/Boosted_Flavor_CR_mu_ejets_Fatjet_SDMass", HNFatJet.SDMass() , weight, 10000, 0., 10000.);
                                        }
                                        // B    oosted Flavor CR
                                    }
                                    else {
                                            if (is_tmp_lead_el) {
                                        // Boosted low WR Flavor CR
                                            FillHist(this_syst + "/pt(ll)_boosted_low_WR_e_mujet_Flavor_CR", (*LeadLep + *OFLooseLepton).Pt(), weight, 1000, 0., 1000.);
                                            FillHist(this_syst + "/leading_fatjet_pt_boosted_low_WR_e_mujet_Flavor_CR", HNFatJet.Pt(), weight, 2000, 0., 2000.);
                                            FillHist(this_syst + "/m(lljj)_boosted_low_WR_e_mujet_Flavor_CR", WRCand.M(), weight, 8000, 0., 8000.);
                                        }
                                            else if (is_tmp_lead_mu) {
                                            // Boosted low WR Flavor CR
                                            FillHist(this_syst + "/pt(ll)_boosted_low_WR_mu_ejets_Flavor_CR", (*LeadLep + *OFLooseLepton).Pt(), weight, 1000, 0., 1000.);
                                            FillHist(this_syst + "/leading_fatjet_pt_boosted_low_WR_mu_ejets_Flavor_CR", HNFatJet.Pt(), weight, 2000, 0., 2000.);
                                            FillHist(this_syst + "/m(lljj)_boosted_low_WR_mu_ejets_Flavor_CR", WRCand.M(), weight, 8000, 0., 8000.);
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
}

    // double counting check?  # 1670

    // Higmass info # 1722 

    // veto HEM  ? # 1748 

    // Fill histograms for main variables # 1771 
    // end ## 1984
    


bool Reproduce20_002_copy::Electrons::isPassCustomLooseID(const Electron& el) const {
    if (!(el.hoe() < 0.5)) return false;
    
    if (fabs(el.scEta()) <= 1.479){
        if (!(el.sieie() < 0.0112)) return false;
        if (!(fabs(el.deltaEtaInSC()) < 0.00377)) return false;
        if (!(fabs(el.deltaPhiInSeed()) < 0.0884)) return false;
        if (!(fabs(el.eInvMinusPInv()) < 0.193)) return false;
        if (!(el.LostHits() <= 1)) return false;
        if (!(el.ConvVeto())) return false;
        return true;
    }
    else {
        if (!(el.sieie() < 0.0425)) return false;
        if (!(fabs(el.deltaEtaInSC()) < 0.00674)) return false;
        if (!(fabs(el.deltaPhiInSeed()) < 0.169)) return false;
        if (!(fabs(el.eInvMinusPInv()) < 0.111)) return false;
        if (!(el.LostHits() <= 1)) return false;
        if (!(el.ConvVeto())) return false;
        return true;
    }
    return true ;
}

RVec<Jet> Reproduce20_002_copy::Clean_jet_with_tight_leptons(const RVec<Jet> & jets, const RVec<Lepton *> & tight_leps) {
    RVec<Jet> cleanedjets;
    for (unsigned int i=0 ; i< jets.size(); i ++) {
        Jet jet = jets.at(i);
        bool isDRtoTightLepton(false);
        for (unsigned int j=0 ; j< tight_leps.size(); j ++) {
            Lepton * lep = tight_leps.at(j);
            if ( jet.DeltaR(*lep) < dR_Separation ) {
                isDRtoTightLepton = true;
                break;
            }
        }
        if (!isDRtoTightLepton) {
            cleanedjets.push_back(jet);
        }
    }
    return cleanedjets;
}

