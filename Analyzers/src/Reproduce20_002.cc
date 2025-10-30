#include "Reproduce20_002.h"    

Reproduce20_002::Reproduce20_002() {}
Reproduce20_002::~Reproduce20_002() {}  

void Reproduce20_002::initializeAnalyzer() {

    Muon_Trigger.clear();
    Muon_Trigger_Safe_Pt_Cut = 0.;
    Ele_Trigger.clear();
    Ele_Trigger_Safe_Pt_Cut = 0.;
    
    if (DataEra == "2022")
    {
        Muon_Trigger = {"HLT_Mu50", "HLT_CascadeMu100", "HLT_HighPtTkMu100"}; 
        double Muon_Trigger_Safe_Pt_Cut = 52.;
        Ele_Trigger = {"HLT_Ele32_WPTight_Gsf","HLT_Photon200","HLT_Ele115_CaloIdVT_GsfTrkIdT"};
        double Ele_Trigger_Safe_Pt_Cut = 35.;  
    }
    if (DataEra == "2022EE")
    {
        Muon_Trigger = {"HLT_Mu50", "HLT_CascadeMu100", "HLT_HighPtTkMu100"}; 
        double Muon_Trigger_Safe_Pt_Cut = 52.;
        Ele_Trigger = {"HLT_Ele32_WPTight_Gsf","HLT_Photon200","HLT_Ele115_CaloIdVT_GsfTrkIdT"};
        double Ele_Trigger_Safe_Pt_Cut = 35.; 
    }
    if (DataEra == "2023")
    {
        Muon_Trigger = {"HLT_Mu50", "HLT_CascadeMu100", "HLT_HighPtTkMu100"}; 
        double Muon_Trigger_Safe_Pt_Cut = 52.;
        Ele_Trigger = {"HLT_Ele32_WPTight_Gsf","HLT_Photon200","HLT_Ele115_CaloIdVT_GsfTrkIdT"};
        double Ele_Trigger_Safe_Pt_Cut = 35.; 
    }
    if (DataEra == "2023BPix")
    {
        Muon_Trigger = {"HLT_Mu50", "HLT_CascadeMu100", "HLT_HighPtTkMu100"}; 
        double Muon_Trigger_Safe_Pt_Cut = 52.;
        Ele_Trigger = {"HLT_Ele32_WPTight_Gsf","HLT_Photon200","HLT_Ele115_CaloIdVT_GsfTrkIdT"};
        double Ele_Trigger_Safe_Pt_Cut = 35.; 
    }

    // Muon , electron SF 설정 ? 
    ZPtReweight = 1.;
    ZPtReweight_Up = 1.;
    ZPtReweight_Down = 1.;
    ZPtReweight_QCDScaleUp = 1.;
    ZPtReweight_QCDScaleDown = 1.;
    ZPtReweight_QCDPDFErrorUp = 1.;
    ZPtReweight_QCDPDFErrorDown = 1.;
    ZPtReweight_QCDPDFAlphaSUp = 1.;
    ZPtReweight_QCDPDFAlphaSDown = 1.;

  //==== Z-pt EW
    ZPtEWCorr = 1.;
    ZPtEWCorr_E1Up = 1.;
    ZPtEWCorr_E1Down = 1.;
    ZPtEWCorr_E2Up = 1.;
    ZPtEWCorr_E2Down = 1.;
    ZPtEWCorr_E3Up = 1.;
    ZPtEWCorr_E3Down = 1.;

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
    myCorr = new MyCorrection(DataEra, DataPeriod, IsDATA ? DataStream : MCSample, IsDATA);
    
    // Initialize systematic helper
    string SKNANO_HOME = getenv("SKNANO_HOME");
    if (IsDATA) {
        systHelper = std::make_unique<SystematicHelper>(SKNANO_HOME + "/docs/noSyst.yaml", DataStream, DataEra);
    } else {
        systHelper = std::make_unique<SystematicHelper>(SKNANO_HOME + "/docs/ExampleSystematic.yaml", MCSample, DataEra);
    }

}

void Reproduce20_002::executeEvent() {

    AllElectrons =  GetAllElectrons();
    AllMuons = GetAllMuons();
    AllJets = GetAllJets();
    AllFatJets = GetAllFatJets();
    gens = GetAllGens();
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
    Particle METv = ev.GetMETVector(Event::MET_Type::PUPPI,Event::MET_Syst::CENTRAL);
    float weight = 1.0;
    if(!IsDATA){
        weight *= MCweight();
        weight *= ev.GetTriggerLumi("Full");
        // weight_norm_1invpb??

        // Z pt reweight

        // signal -> Kfactor 
    }

    //  No cut 

    FillHist(this_syst + "/CutFlow", 0.0, weight, 10, 0., 10.); // Initial event
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
    RVec<Electron> electrons = AllElectrons;
    RVec<Muon> muons = AllMuons;
    RVec<Jet> jets = AllJets;
    RVec<FatJet> fatjets = AllFatJets;
    if (!PassNoiseFilter(AllJets,ev,Event::MET_Type::PUPPI)) return;

    FillHist(this_syst + "/CutFlow", 1.0, weight, 10, 0., 10.); // Noise filter pass

    bool pass_trig_muon = ev.PassTrigger(Muon_Trigger);
    bool pass_trig_elec = ev.PassTrigger(Ele_Trigger);

    FillHist(this_syst + "/CutFlow", 2.0, weight, 10, 0., 10.); // HLT pass

    
    

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




    ///         Leptons       ///
    RVec<Electron> my_electrons = SelectElectrons(electrons, "NOCUT" , emu_set.Electron_MinPt, 2.4);
    RVec<Muon> my_muons = SelectMuons(muons, "NOCUT" , emu_set.Muon_MinPt, 2.4);

    //prompt only 구현 필요    
    sort (my_electrons.begin(), my_electrons.end(), PtComparing);
    sort (my_muons.begin(), my_muons.end(), PtComparing);

    RVec<Electron *> Loose_electrons , Tight_electrons;
    RVec<Muon *> Loose_muons , Tight_muons;
    RVec<Lepton *> Tight_leps_el , Tight_leps_mu , Tight_leps;
    RVec<Lepton *> Loose_leps_el , Loose_leps_mu , Loose_leps;

    for (unsigned int i=0 ; i< my_electrons.size(); i ++) {
        Electron & el = my_electrons.at(i);
        if (el.PassID(emu_set.Electron_Tight_ID[0])) {
            Tight_electrons.push_back(&el);
            Tight_leps_el.push_back( &el);
            Tight_leps.push_back(&el);

        }
        else if (el.PassID(emu_set.Electron_Loose_ID[0])) {
            Loose_electrons.push_back(&el);
            Loose_leps_el.push_back(&el);
            Loose_leps.push_back(&el);
        }
    }
    for (unsigned int i=0 ; i< my_muons.size(); i ++) {
        Muon & mu = my_muons.at(i);
        if (mu.PassID(emu_set.Muon_Tight_ID[0])) {
            Tight_muons.push_back(&mu);
            Tight_leps_mu.push_back( &mu);
            Tight_leps.push_back(&mu);
        }
        else if (mu.PassID(emu_set.Muon_Loose_ID[0])) {
            Loose_muons.push_back(&mu);
            Loose_leps_mu.push_back(&mu);
            Loose_leps.push_back(&mu);
        }
    }
    sort (Tight_leps.begin(), Tight_leps.end(), PtComparingPtr);

    int n_Loose_leptons  = Loose_electrons.size() + Loose_muons.size();
    int n_Tight_leptons  = Tight_electrons.size() + Tight_muons.size();
    ///         Jets       ///

    JetIds = {Jet::JetID::LOOSE};
    RVec<Jet> selected_jets = SelectJets(jets, JetIds[0] , 40.0, 2.4);
    sort (selected_jets.begin(), selected_jets.end(), PtComparing);

    ///         FatJets       ///
    
    //RVec<FatJet> selected_fatjets = SelectFatJets(fatjets, loose , 200.0, 2.4);
    //sort (selected_fatjets.begin(), selected_fatjets.end(), PtComparing);

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
    for(const auto &jet: AllJets){
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

    bool this_trigger_pass(false);
    bool tmp_isEE(false), tmp_isMM(false), tmp_isEM(false);
    if (Tight_muons.size() == 0 ) return;
    if ( (n_Tight_leptons == 2 ) && (Tight_muons[0]->Pt() > 60.0) ) {

        FillHist(this_syst + "/CutFlow", 3.0, weight, 10, 0., 10.); // 2 tight leptons with pT cut
        
        if ( (Tight_electrons.size() == 2) && ( Tight_muons.size() == 0 )) {
            this_trigger_pass = true;
            tmp_isEE = true;
        }
        else if ( (Tight_muons.size() == 2) && ( Tight_electrons.size() == 0 )) {
            this_trigger_pass = true;
            tmp_isMM = true;
        }
        else if ( (Tight_muons.size() == 1) && ( Tight_electrons.size() == 1 )) {
            this_trigger_pass = true;
            tmp_isEM = true;
        }

        if (this_trigger_pass) {
            // needs 2 jets 
            if (selected_jets.size() >= 2 ){
                FillHist(this_syst + "/CutFlow", 4.0, weight, 10, 0., 10.); // 2 jets
                Lepton *LeadLep = Tight_leps[0];
                Lepton *SubLeadLep = Tight_leps[1];

                // needs seperation 

                bool dRLeadJetLepton = ((selected_jets[0].DeltaR(*LeadLep) > 0.4) &&  (selected_jets[0].DeltaR(*SubLeadLep) > 0.4) );
                bool dRSubLeadJetLepton = ((selected_jets[1].DeltaR(*LeadLep) > 0.4) &&  (selected_jets[1].DeltaR(*SubLeadLep) > 0.4) );
                bool dRTwoLepton = (LeadLep->DeltaR(*SubLeadLep) > 0.4);
                bool dRTwoJets = (selected_jets[0].DeltaR(selected_jets[1]) > 0.4);

                

                if ( dRLeadJetLepton && dRSubLeadJetLepton && dRTwoLepton && dRTwoJets ){
                    bool IsResolvedEvent(true);
                    FillHist(this_syst + "/CutFlow", 5.0, weight, 10, 0., 10.); // dR cuts pass
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

                // e , mu id , reco , ISO SF
                
                // Trigger SF

                // DY reshape ?

                // saving plots for each CR 
                
                // DY CR ll< 150 , lljj > 800 
                if ( DiLepMassLT150 && WRCand.M() > 800.0 ) {
                    FillHist(this_syst + "/CutFlow", 6.0, weight, 10, 0., 10.); // DY CR pass
                    if (tmp_isEE) {
                        FillHist(this_syst + "/DYCR_Resolved_EE_pt", dilepton_pt, weight, 100, 0., 1000.);
                    }
                    else if (tmp_isMM) {
                        FillHist(this_syst + "/DYCR_Resolved_MM_pt",dilepton_pt, weight, 100, 0., 1000.);
                    }
                    else if (tmp_isEM) {
                        FillHist(this_syst + "/DYCR_Resolved_EM_pt", dilepton_pt, weight, 100, 0., 1000.);
                    }
                }
            }




                


            }
        }// # 1192 -> Resolved ends .
    // # 1660  -> Boost event ends .

    // double counting check?  # 1670

    // Higmass info # 1722 

    // veto HEM  ? # 1748 

    // Fill histograms for main variables # 1771 
    // end ## 1984
    }
}


        





    







