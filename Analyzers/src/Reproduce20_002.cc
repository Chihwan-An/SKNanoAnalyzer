#include "Reproduce20_002.h"    

Reproduce20_002::Reproduce20_002() {}
Reproduce20_002::~Reproduce20_002() {}  

void Reproduce20_002::initializeAnalyzer() {
    // if signal ..  # 26 
    // kfactor  # 51
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

    // B tagging parameters ..?  # 177
    
    /*
    // PU Reweight # 188
    if(!IsDATA){
        TString datapath = getenv("DATA_DIR");
        TString PUfname = datapath+"/"+TString::Itoa(DataYear,10)+"/PileUp/PUReweight_"+TString::Itoa(DataYear,10)+".root";
        TString PUhname = "PUReweight_"+TString::Itoa(DataYear,10);
        TFile *file_PUReweight = new TFile(PUfname);
        hist_PUReweight = (TH1D *)file_PUReweight->Get(PUhname);
        hist_PUReweight_Up = (TH1D *)file_PUReweight->Get(PUhname+"_Up");
        hist_PUReweight_Down = (TH1D *)file_PUReweight->Get(PUhname+"_Down");
    }


    //DY reshape  # 209
    if(!IsDATA){
    TString datapath = getenv("DATA_DIR");
    //==== QCD
    TFile *file_DYPtReweight = new TFile(datapath+"/"+TString::Itoa(DataYear,10)+"/HNWRDYPtReweight/ZPtQCDWithError.root");
    hist_DYPt_PDFError = (TH1D *)file_DYPtReweight->Get("h_Ratio_PDFError");
    hist_DYPt_ScaleUp = (TH1D *)file_DYPtReweight->Get("h_Ratio_ScaleUp");
    hist_DYPt_ScaleDown = (TH1D *)file_DYPtReweight->Get("h_Ratio_ScaleDown");
    hist_DYPt_PDFAlphaSUp = (TH1D *)file_DYPtReweight->Get("h_Ratio_AlphaSUp");
    hist_DYPt_PDFAlphaSDown = (TH1D *)file_DYPtReweight->Get("h_Ratio_AlphaSDown");

    //==== EW
    TFile *file_DYPtEWCorr = new TFile(datapath+"/"+TString::Itoa(DataYear,10)+"/HNWRDYPtReweight/ZPtEWCorr.root");
    hist_DYPtEWCorr = (TH1D *)file_DYPtEWCorr->Get("hist_v");
    hist_DYPtEWCorrE1 = (TH1D *)file_DYPtEWCorr->Get("hist_e1");
    hist_DYPtEWCorrE2 = (TH1D *)file_DYPtEWCorr->Get("hist_e2");
    hist_DYPtEWCorrE3 = (TH1D *)file_DYPtEWCorr->Get("hist_e3");

    //==== Shape
    TString tmp_IsJetPt = "";
    if(UseJetPtRwg) tmp_IsJetPt = "JetPt";
    TString             filename_DYReshape = "DYReshape"+tmp_IsJetPt+"_"     +TString::Itoa(DataYear,10)+".root";
    if(UseDYCR1Reshape) filename_DYReshape = "DYReshape"+tmp_IsJetPt+"DYCR1_"+TString::Itoa(DataYear,10)+".root";

    cout << "[HNWRAnalyzer::initializeAnalyzer()] filename_DYReshape = " << filename_DYReshape << endl;
    TFile *file_DYReshape = new TFile(datapath+"/"+TString::Itoa(DataYear,10)+"/HNWRDYReshape/"+filename_DYReshape);
    hist_DYReshape_Resolved_ratio_AllCh = (TH1D *)file_DYReshape->Get("Resolved_ratio_AllCh");
    hist_DYReshape_Resolved_EEOnlyRatio = (TH1D *)file_DYReshape->Get("Resolved_EEOnlyRatio");
    hist_DYReshape_Resolved_MuMuOnlyRatio = (TH1D *)file_DYReshape->Get("Resolved_MuMuOnlyRatio");
    hist_DYReshape_Boosted_ratio_AllCh = (TH1D *)file_DYReshape->Get("Boosted_ratio_AllCh");
    hist_DYReshape_Boosted_EEOnlyRatio = (TH1D *)file_DYReshape->Get("Boosted_EEOnlyRatio");
    hist_DYReshape_Boosted_MuMuOnlyRatio = (TH1D *)file_DYReshape->Get("Boosted_MuMuOnlyRatio");
    }   
    */
    
    
    // Initialize systematic helper
    string SKNANO_HOME = getenv("SKNANO_HOME");
    if (IsDATA) {
        systHelper = std::make_unique<SystematicHelper>(SKNANO_HOME + "/docs/noSyst.yaml", DataStream, DataEra);
    } else {
        systHelper = std::make_unique<SystematicHelper>(SKNANO_HOME + "/docs/ExampleSystematic.yaml", MCSample, DataEra);
    }

}

void Reproduce20_002::executeEvent() {

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

void Reproduce20_002::executeEventFromParameter() {
    const TString this_syst = systHelper->getCurrentSysName();

    Event ev = GetEvent();
    //Particle METv = ev.GetMETVector(Event::MET_Type::PUPPI,Event::MET_Syst::CENTRAL);
    float weight = 1.0;
    if(!IsDATA){
        weight *= MCweight();
        weight *= ev.GetTriggerLumi("Full");
        // weight_norm_1invpb??

        // Z pt reweight

        // signal -> Kfactor 
    }

    //  No cut 

    FillHist(this_syst + "/CutFlow", 0.0, weight, 20,-10,10.); // Initial event
    /*if(RunXsecSyst && param.syst_ == AnalyzerParameter::Central){
    double normweight = 1./sumW/PDFWeights_Error->at(0) * this_kfactor/this_avg_kfactor;
    for(unsigned int i=0; i<PDFWeights_Scale->size(); i++){
        FillHist("XsecSyst_Den/PDFWeights_Scale_"+TString::Itoa(i,10)+"_XsecSyst_Den", 0., PDFWeights_Scale->at(i)*ev.MCweight()*normweight, 1, 0., 1.);
    }
    for(unsigned int i=0; i<PDFWeights_Error->size(); i++){
        FillHist("XsecSyst_Den/PDFWeights_Error_"+TString::Itoa(i,10)+"_XsecSyst_Den", 0., PDFWeights_Error->at(i)*ev.MCweight()*normweight, 1, 0., 1.);
        if(PDFWeights_Error->at(i)>0){
        FillHist("XsecSyst_Den/PDFWeights_Error_"+TString::Itoa(i,10)+"_XsecSyst_Den_Positive", 0., PDFWeights_Error->at(i)*ev.MCweight()*normweight, 1, 0., 1.);
        }
        else{
        FillHist("XsecSyst_Den/PDFWeights_Error_"+TString::Itoa(i,10)+"_XsecSyst_Den_Negative", 0., PDFWeights_Error->at(i)*ev.MCweight()*normweight, 1, 0., 1.);
        }
    }
    for(unsigned int i=0; i<PDFWeights_AlphaS->size(); i++){
        FillHist("XsecSyst_Den/PDFWeights_AlphaS_"+TString::Itoa(i,10)+"_XsecSyst_Den", 0., PDFWeights_AlphaS->at(i)*ev.MCweight()*normweight, 1, 0., 1.);
    }
    }*/
    

    //Event selection 
    RVec<Electron> electrons = el_set.AllElectrons;
    RVec<Muon> muons = mu_set.AllMuons;
    RVec<Jet> jets = jet_set.AllJets;
    RVec<FatJet> fatjets = fatjet_set.AllFatJets;
    if (!PassNoiseFilter(jets,ev,Event::MET_Type::PUPPI)) return;

    FillHist(this_syst + "/CutFlow", 1.0, weight, 20,-10,10.); // Noise filter pass

    bool pass_trig_muon = ev.PassTrigger(mu_set.Muon_Trigger);
    bool pass_trig_elec = ev.PassTrigger(el_set.Ele_Trigger);

    FillHist(this_syst + "/CutFlow", 2.0, weight, 20,-10,10.); // HLT pass

    
    

    /*  int SystDir_MuonRecoSF(0);
    int SystDir_ElectronRecoSF(0);

    int SystDir_MuonIDSF(0);
    int SystDir_ElectronIDSF(0);

    int SystDir_MuonISOSF(0);

    int SystDir_MuonTriggerSF(0);
    int SystDir_ElectronTriggerSF(0);

    int SystDir_LSFSF(0);
    int SystDir_PU(0);

    int SystDir_DYReshape(0);
    string SystDir_BTag("central");

    if(param.syst_ == AnalyzerParameter::Central){

    }
    else if(param.syst_ == AnalyzerParameter::JetResUp){
        this_AllJets = SmearJets( this_AllJets, +1 );
        this_AllFatJets = SmearFatJets( this_AllFatJets, +1 );
    }
    else if(param.syst_ == AnalyzerParameter::JetResDown){
        this_AllJets = SmearJets( this_AllJets, -1 );
        this_AllFatJets = SmearFatJets( this_AllFatJets, -1 );
    }
    else if(param.syst_ == AnalyzerParameter::JetEnUp){
        this_AllJets = ScaleJets( this_AllJets, +1 );
        this_AllFatJets = ScaleFatJets( this_AllFatJets, +1 );
    }
    else if(param.syst_ == AnalyzerParameter::JetEnDown){
        this_AllJets = ScaleJets( this_AllJets, -1 );
        this_AllFatJets = ScaleFatJets( this_AllFatJets, -1 );
    }
    else if(param.syst_ == AnalyzerParameter::JetMassUp){
        this_AllFatJets = ScaleSDMassFatJets( this_AllFatJets, +1 );
    }
    else if(param.syst_ == AnalyzerParameter::JetMassDown){
        this_AllFatJets = ScaleSDMassFatJets( this_AllFatJets, -1 );
    }
    else if(param.syst_ == AnalyzerParameter::MuonRecoSFUp){
        SystDir_MuonRecoSF = +1;
    }
    else if(param.syst_ == AnalyzerParameter::MuonRecoSFDown){
        SystDir_MuonRecoSF = -1;
    }
    else if(param.syst_ == AnalyzerParameter::MuonEnUp){
        this_AllMuons = ScaleMuons( this_AllMuons, +1 );
        this_AllTunePMuons = ScaleMuons( this_AllTunePMuons , +1 );
    }
    else if(param.syst_ == AnalyzerParameter::MuonEnDown){
        this_AllMuons = ScaleMuons( this_AllMuons, -1 );
        this_AllTunePMuons = ScaleMuons( this_AllTunePMuons , -1 );
    }
    else if(param.syst_ == AnalyzerParameter::MuonIDSFUp){
        SystDir_MuonIDSF = +1;
    }
    else if(param.syst_ == AnalyzerParameter::MuonIDSFDown){
        SystDir_MuonIDSF = -1;
    }
    else if(param.syst_ == AnalyzerParameter::MuonISOSFUp){
        SystDir_MuonISOSF = +1;
    }
    else if(param.syst_ == AnalyzerParameter::MuonISOSFDown){
        SystDir_MuonISOSF = -1;
    }
    else if(param.syst_ == AnalyzerParameter::MuonTriggerSFUp){
        SystDir_MuonTriggerSF = +1;
    }
    else if(param.syst_ == AnalyzerParameter::MuonTriggerSFDown){
        SystDir_MuonTriggerSF = -1;
    }
    else if(param.syst_ == AnalyzerParameter::ElectronRecoSFUp){
        SystDir_ElectronRecoSF = +1;
    }
    else if(param.syst_ == AnalyzerParameter::ElectronRecoSFDown){
        SystDir_ElectronRecoSF = -1;
    }
    else if(param.syst_ == AnalyzerParameter::ElectronResUp){
        this_AllElectrons = SmearElectrons( this_AllElectrons, +1 );
    }
    else if(param.syst_ == AnalyzerParameter::ElectronResDown){
        this_AllElectrons = SmearElectrons( this_AllElectrons, -1 );
    }
    else if(param.syst_ == AnalyzerParameter::ElectronEnUp){
        this_AllElectrons = ScaleElectrons( this_AllElectrons, +1 );
    }
    else if(param.syst_ == AnalyzerParameter::ElectronEnDown){
        this_AllElectrons = ScaleElectrons( this_AllElectrons, -1 );
    }
    else if(param.syst_ == AnalyzerParameter::ElectronIDSFUp){
        SystDir_ElectronIDSF = +1;
    }
    else if(param.syst_ == AnalyzerParameter::ElectronIDSFDown){
        SystDir_ElectronIDSF = -1;
    }
    else if(param.syst_ == AnalyzerParameter::ElectronTriggerSFUp){
        SystDir_ElectronTriggerSF = +1;
    }
    else if(param.syst_ == AnalyzerParameter::ElectronTriggerSFDown){
        SystDir_ElectronTriggerSF = -1;
    }
    else if(param.syst_ == AnalyzerParameter::LSFSFUp){
        SystDir_LSFSF = +1;
    }
    else if(param.syst_ == AnalyzerParameter::LSFSFDown){
        SystDir_LSFSF = -1;
    }
    else if(param.syst_ == AnalyzerParameter::PUUp){
        SystDir_PU = +1;
    }
    else if(param.syst_ == AnalyzerParameter::PUDown){
        SystDir_PU = -1;
    }
    else if(param.syst_ == AnalyzerParameter::ZPtRwUp){

    }
    else if(param.syst_ == AnalyzerParameter::ZPtRwDown){

    }
    else if(param.syst_ == AnalyzerParameter::ZPtRwQCDScaleUp){

    }
    else if(param.syst_ == AnalyzerParameter::ZPtRwQCDScaleDown){

    }
    else if(param.syst_ == AnalyzerParameter::ZPtRwQCDPDFErrorUp){

    }
    else if(param.syst_ == AnalyzerParameter::ZPtRwQCDPDFErrorDown){

    }
    else if(param.syst_ == AnalyzerParameter::ZPtRwQCDPDFAlphaSUp){

    }
    else if(param.syst_ == AnalyzerParameter::ZPtRwQCDPDFAlphaSDown){

    }
    else if(param.syst_ == AnalyzerParameter::ZPtRwEW1Up){

    }
    else if(param.syst_ == AnalyzerParameter::ZPtRwEW1Down){

    }
    else if(param.syst_ == AnalyzerParameter::ZPtRwEW2Up){

    }
    else if(param.syst_ == AnalyzerParameter::ZPtRwEW2Down){

    }
    else if(param.syst_ == AnalyzerParameter::ZPtRwEW3Up){

    }
    else if(param.syst_ == AnalyzerParameter::ZPtRwEW3Down){

    }
    else if(param.syst_ == AnalyzerParameter::PrefireUp){

    }
    else if(param.syst_ == AnalyzerParameter::PrefireDown){

    }
    else if(param.syst_ == AnalyzerParameter::DYReshapeSystUp){
        SystDir_DYReshape = +1;
    }
    else if(param.syst_ == AnalyzerParameter::DYReshapeSystDown){
        SystDir_DYReshape = -1;
    }
    else if(param.syst_ == AnalyzerParameter::BTagUp){
        SystDir_BTag = "up";
    }
    else if(param.syst_ == AnalyzerParameter::BTagDown){
        SystDir_BTag = "down";
    }
    else{
        cerr << "[HNWRAnalyzer::executeEventFromParameter] Wrong syst : param.syst_ = " << param.syst_ << endl;
        exit(EXIT_FAILURE);
    }*/

    // PU rewieght # 799



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
        if ((fj.Pt() > fatjet_set.FatJet_MinPt) && (abs(fj.Eta())<fatjet_set.FatJet_MaxEta)) {
            FillHist(this_syst + "fatjet_cutflow", 1 , weight, 10, 0., 10.);
            //if (fj.PassID(fatjet_set.FatJet_ID[0])) {
                
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

    for (unsigned int i=0 ; i< my_electrons.size(); i ++) {
        Electron & el = my_electrons.at(i);
        if (el.PassID(el_set.Electron_Tight_ID[0])) {
            Tight_electrons.push_back(&el);
            Tight_leps_el.push_back( &el);
            Tight_leps.push_back(&el);

        }
        else if (el.PassID(el_set.Electron_Loose_ID[0])) {
            Loose_electrons.push_back(&el);
            Loose_leps_el.push_back(&el);
            Loose_leps.push_back(&el);
        }
    }
    for (unsigned int i=0 ; i< my_muons.size(); i ++) {
        Muon & mu = my_muons.at(i);

        float tkRelIso = mu.TkRelIso();
        
        if ((mu.PassID(mu_set.Muon_Tight_ID[0]))&&( tkRelIso < 0.1) ){ //global high pt id 
            Tight_muons.push_back(&mu);
            Tight_leps_mu.push_back( &mu);
            Tight_leps.push_back(&mu);
        }
        else if (mu.PassID(mu_set.Muon_Loose_ID[0])) {
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
    jet_set.cleanedjet_with_loose_leptons = Clean_jet_with_loose_leptons(jet_set.AllJets, Loose_leps);

    FillHist(this_syst + "/Non_Selected_Jetnum", jets.size(), 1.0, 20, 0., 20.);
    jet_set.JetIds = {Jet::JetID::NOCUT};
    RVec<Jet> selected_jets = SelectJets(jet_set.cleanedjet_with_tight_leptons, jet_set.JetIds[0] , 40.0, 2.4);
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

    // lepton SF 

    // double this_trigger_SF(1.0);

    // Categorization 
    //  뭐임 이게 
    /*
    map<TString, bool> map_bool_To_Region;
    FatJet HNFatJet;
    Particle WRCand;
    Particle NCand;
    Particle NCand_1, NCand_2;

    //==== leps_for_plot
    //==== - If Resolved, at(0) is Tight Leading, at(1) is Tight Subleading
    //==== - If Boosted, at(0) is Tight Leading, at(1) is Loose inside the AK8jet
    vector<Lepton *> leps_for_plot;
    int NExtraLooseElectron(0),NExtraLooseMuon(0),NExtraLooseLepton(0);
    int NExtraTightElectron(0),NExtraTightMuon(0),NExtraTightLepton(0);

    //==== Check Resolved first

    bool IsResolvedEvent = false;
    bool IsResolved_SR_EE(false), IsResolved_SR_MM(false), IsResolved_SR_EM(false);
    bool IsResolved_LowWRCR_EE(false), IsResolved_LowWRCR_MM(false), IsResolved_LowWRCR_EM(false);
    bool IsResolved_DYCR_EE(false), IsResolved_DYCR_MM(false), IsResolved_DYCR_EM(false);
    */


    // Requires 2 tight leptons , l1 > 60 
    // Def of resolved event 
    bool IsResolvedEvent = false;
    bool this_trigger_pass(false);
    bool tmp_isEE(false), tmp_isMM(false), tmp_isEM(false);
    if ( (n_Tight_leptons == 2 ) && (Tight_leps[0]->Pt() > 60.0)  && (Tight_leps[1]->Pt() > 53.0)) {
        IsResolvedEvent = true;
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
            if (selected_jets.size() >= 2 ){
                FillHist(this_syst + "/CutFlow", 4.0, weight, 20,-10,10.); // 2 jets
                Lepton *LeadLep = Tight_leps[0];
                Lepton *SubLeadLep = Tight_leps[1];

                
                
                FillHist(this_syst + "/CutFlow", 5.0, weight, 20,-10,10.); // dR cuts pass
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


                //SF application 
                /*
                if (!IsDATA) {
                    for (unsigned int i=0 ; i< Tight_electrons.size()){
                        double thisrecosf = myCorr->GetElectronRECOSF( Tight_electrons[i]->Eta(), Tight_electrons[i]->Pt() , Tight_electrons[i]->Phi(), "nom" );    
                        double thisidsf = myCorr->GetElectronIDSF( Tight_electrons[i]->Eta(), Tight_electrons[i]->Pt() , Tight_electrons[i]->Phi(), "nom" );
                        weight *= thisrecosf * thisidsf;    
                    }
                    for (unsigned int i=0 ; i< Tight_muons.size()){
                        double thisrecosf = myCorr->GetMuonRecoSF( Tight_muons[i], "nom" );    
                        double thisidsf = myCorr->GetMuonIDSF( Tight_muons[i], "nom" );
                        weight *= thisrecosf * thisidsf;
                    }
                    //trigger SF
                    trigger_sf

                
                }
                */

                    

                    
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
                    FillHist(this_syst + "/DYCR_Resolved_EE_pt", dilepton_pt, weight, 100, 0., 1000.);
                    FillHist(this_syst + "/DYCR_Resolved_EE_leading_jet_pt", selected_jets[0].Pt(), weight, 200, 0., 2000.);
                    FillHist(this_syst + "/DYCR_Resolved_EE_subleading_jet_pt", selected_jets[1].Pt(), weight, 200, 0., 2000.);
                    FillHist(this_syst + "/DYCR_Resolved_EE_mlljj", WRCand.M(), weight, 800, 0., 8000.);
                }
                else if (tmp_isMM) {
                    FillHist(this_syst + "/DYCR_Resolved_MM_pt",dilepton_pt, weight, 100, 0., 1000.);
                    FillHist(this_syst + "/DYCR_Resolved_MM_leading_jet_pt", selected_jets[0].Pt(), weight, 200, 0., 2000.);
                    FillHist(this_syst + "/DYCR_Resolved_MM_subleading_jet_pt", selected_jets[1].Pt(), weight, 200, 0., 2000.);
                    FillHist(this_syst + "/DYCR_Resolved_MM_mlljj", WRCand.M(), weight, 800, 0., 8000.);
                }
                else if (tmp_isEM) {
                    FillHist(this_syst + "/DYCR_Resolved_EM_pt", dilepton_pt, weight, 100, 0., 1000.);
                    FillHist(this_syst + "/DYCR_Resolved_EM_leading_jet_pt", selected_jets[0].Pt(), weight, 200, 0., 2000.);
                    FillHist(this_syst + "/DYCR_Resolved_EM_subleading_jet_pt", selected_jets[1].Pt(), weight, 200, 0., 2000.);
                    FillHist(this_syst + "/DYCR_Resolved_EM_mlljj", WRCand.M(), weight, 800, 0., 8000.);
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
        }// # 1192 -> Resolved ends 
    
    }
    }
    /////////
    //if ( (n_Tight_leptons == 2 ) && (Tight_leps[0]->Pt() > 60.0)  && (Tight_leps[1]->Pt() > 53.0)) {
    //    IsResolvedEvent = true;
    ///////////Resolved selections 

    
    if (!IsResolvedEvent){

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
                    for (unsigned int i=0 ; i< fatjets.size(); i++) {
                        FatJet this_fatjet = fatjets.at(i);
                        FillHist(this_syst + "/deltaPhi_LeadLep_Fatjet", abs( LeadLep->DeltaPhi(this_fatjet)) , weight, 100, 0., 3.5);
                        if (abs( LeadLep->DeltaPhi(this_fatjet))>2.0) {
                            FillHist(this_syst + "/Boost_cutflow_DY", 5 , weight, 20,-10,10.);
                            FatJet HNFatJet = this_fatjet;
                            Particle Ncand;
                        // if loose lepton is inside of fatjet
                            if (this_fatjet.DeltaR( *LowMllLooseLepton)<0.8) {
                                Ncand = this_fatjet;
                                FillHist(this_syst + "has_looselepton_insidefatjet", 1 , weight, 5, 0., 5.);
                            }
                            else { // if loose lepton is outside of fatjet
                                Ncand = HNFatJet + *LowMllLooseLepton;
                                FillHist(this_syst + "has_looselepton_outsidefatjet", 1 , weight, 5, 0., 5.);
                                FillHist(this_syst + "/mass_looselepton_fatjet_outside", Ncand.M() , weight, 8000, 0., 8000.);
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
                                if (is_tmp_lead_el){ // ee 
                                        FillHist(this_syst + "/pt(ll)_boosted_DY_CR_EE", (*LeadLep + *LowMllLooseLepton).Pt(), weight, 1000, 0., 1000.);
                                        FillHist(this_syst + "/leading_fatjet_pt_boosted_DY_CR_EE", HNFatJet.Pt(), weight, 2000, 0., 2000.);
                                        FillHist(this_syst + "/m(lljj)_boosted_DY_CR_EE", WRCand.M(), weight, 8000, 0., 8000.);
                                    }
                                    if (is_tmp_lead_mu){// mumu
                                        FillHist(this_syst + "/pt(ll)_boosted_DY_CR_MM", (*LeadLep + *LowMllLooseLepton).Pt(), weight, 1000, 0., 1000.);
                                        FillHist(this_syst + "/leading_fatjet_pt_boosted_DY_CR_MM", HNFatJet.Pt(), weight, 2000, 0., 2000.);
                                        FillHist(this_syst + "/m(lljj)_boosted_DY_CR_MM", WRCand.M(), weight, 8000, 0., 8000.);
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
                } // has low mll end
            
            
            // dont have low mll
                else{
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

                    //if  lead lep delta phi cut > 2.0 with fatjet
                    if (hasawaymergedfatjet) {
                        FillHist(this_syst + "/Boost_cutflow_FLV", 5 , weight, 20,-10,10.);
                        bool hassflooselepton(false);
                        bool hasoflooselepton(false);

                        Lepton *SFLooseLepton;
                        Lepton *OFLooseLepton;
                        FillHist(this_syst +"/numofhnfatjet_FLV", HNFatJet.SDMass() , weight, 10000, 0., 10000.);
                    //SF loose lepton inside fatjet
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

                    //OF loose lepton inside fatjet
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
                                        FillHist(this_syst + "/Boost_cutflow_FLV", 9 , weight, 20,-10,10.);
                                        if (WRMassGT800) {
                                            /////////******************************///////////////////// */
                            /////////******************************///////////////////// */        SR /////////******************************///////////////////// *//////////******************************///////////////////// *//////////******************************///////////////////// *
                            /////////******************************///////////////////// */        SR /////////******************************///////////////////// *//////////******************************///////////////////// *//////////******************************///////////////////// *
                            /////////******************************///////////////////// */        SR /////////******************************///////////////////// *//////////******************************///////////////////// *//////////******************************///////////////////// *
                            /////////******************************///////////////////// */        SR /////////******************************///////////////////// *//////////******************************///////////////////// *//////////******************************///////////////////// *
                            /////////******************************///////////////////// */        SR /////////******************************///////////////////// *//////////******************************///////////////////// *//////////******************************///////////////////// *
                            // /
                                            if (is_tmp_lead_el) {
                                                // Boosted DY SR
                                            }
                                            else if (is_tmp_lead_mu) {
                                                // Boosted DY SR
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
                                        }
                                        else if (is_tmp_lead_mu) {
                                            // Boosted Flavor CR
                                            FillHist(this_syst + "/pt(ll)_boosted_mu_ejets_Flavor_CR", (*LeadLep + *OFLooseLepton).Pt(), weight, 1000, 0., 1000.);
                                            FillHist(this_syst + "/leading_fatjet_pt_boosted_mu_ejets_Flavor_CR", HNFatJet.Pt(), weight, 2000, 0., 2000.);
                                            FillHist(this_syst + "/m(lljj)_boosted_mu_ejets_Flavor_CR", WRCand.M(), weight, 8000, 0., 8000.);
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
    



        




RVec<Jet> Reproduce20_002::Clean_jet_with_tight_leptons(const RVec<Jet> & jets, const RVec<Lepton *> & tight_leps) {
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

RVec<Jet> Reproduce20_002::Clean_jet_with_loose_leptons(const RVec<Jet> & jets, const RVec<Lepton *> & loose_leps) {
    RVec<Jet> cleanedjets;
    for (unsigned int i=0 ; i< jets.size(); i ++) {
        Jet jet = jets.at(i);
        bool isDRtoLooseLepton(false);
        for (unsigned int j=0 ; j< loose_leps.size(); j ++) {
            Lepton * lep = loose_leps.at(j);
            if ( jet.DeltaR(*lep) < dR_Separation ) {
                isDRtoLooseLepton = true;
                break;
            }
        }
        if (!isDRtoLooseLepton) {
            cleanedjets.push_back(jet);
        }
    }
    return cleanedjets;
}
    







