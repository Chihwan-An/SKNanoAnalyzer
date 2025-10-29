#include "Reproduce20_002.h"    

Reproduce20_002::Reproduce20_002() {}
Reproduce20_002::~Reproduce20_002() {}  

void Reproduce20_002::initializeAnalyzer() {

    Muon_Trigger.clear();
    Muon_Trigger_Safe_Pt_Cut.clear();
    Ele_Trigger.clear();
    Ele_Trigger_Safe_Pt_Cut.clear();
    
    if (DataEra == "2022")
    {
        Muon_Trigger = {"HLT_Mu50", "HLT_CascadeMu100", "HLT_HighPtTkMu100"}; 
        Muon_Trigger_Safe_Pt_Cut = 52.;
        Ele_Trigger = {"HLT_Ele32_WPTight_Gsf","HLT_Photon200","HLT_Ele115_CaloIdVT_GsfTrkIdT"}
        Ele_Trigger_Safe_Pt_Cut = 35.;  
    }
    if (DataEra == "2022EE")
    {
        Muon_Trigger = {"HLT_Mu50", "HLT_CascadeMu100", "HLT_HighPtTkMu100"}; 
        Muon_Trigger_Safe_Pt_Cut = 52.;
        Ele_Trigger = {"HLT_Ele32_WPTight_Gsf","HLT_Photon200","HLT_Ele115_CaloIdVT_GsfTrkIdT"}
        Ele_Trigger_Safe_Pt_Cut = 35.; 
    }
    if (DataEra == "2023")
    {
        Muon_Trigger = {"HLT_Mu50", "HLT_CascadeMu100", "HLT_HighPtTkMu100"}; 
        Muon_Trigger_Safe_Pt_Cut = 52.;
        Ele_Trigger = {"HLT_Ele32_WPTight_Gsf","HLT_Photon200","HLT_Ele115_CaloIdVT_GsfTrkIdT"}
        Ele_Trigger_Safe_Pt_Cut = 35.; 
    }
    if (DataEra == "2023BPix")
    {
        Muon_Trigger = {"HLT_Mu50", "HLT_CascadeMu100", "HLT_HighPtTkMu100"}; 
        Muon_Trigger_Safe_Pt_Cut = 52.;
        Ele_Trigger = {"HLT_Ele32_WPTight_Gsf","HLT_Photon200","HLT_Ele115_CaloIdVT_GsfTrkIdT"}
        Ele_Trigger_Safe_Pt_Cut = 35.; 
    }
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




    // PU Reweight # 188



    //DY reshape  # 209

}

void Reproduce20_002::executeEvent() {

    Allelectrons =  GetAllElectrons();
    Allmuons = GetAllMuons();
    Alljets = GetAllJets();
    Allfatjets = GetAllFatJets();
    // no tune p muon 
    

    // DY pt reweight

    // Nvtx ( PU )


    //Parameter
    AnalyzerParameter param;
    param.clear();

    param.Name = "HNWR";
    param.MCCorrrectionIgnoreNoHist = false;

    param.Electron_Tight_ID = "HNWRTight";
    param.Electron_Loose_ID = "HNWRLoose";
    param.Electron_ID_SF_Key = "HEEP";
    param.Electron_Trigger_SF_Key = "HEEP";
    param.Electron_FR_ID = "HNWR";
    param.Electron_FR_Key = "AwayJetPt40";
    param.Electron_CF_ID = "HNWRTight";
    param.Electron_CF_Key = "ZToLL";
    param.Electron_UseMini = false;
    param.Electron_UsePtCone = false;
    param.Electron_MinPt = 53.;

    param.Muon_Tight_ID = "HNWRTight";
    param.Muon_Loose_ID = "HNWRLoose";
    param.Muon_RECO_SF_Key = "HighPtMuonRecoSF";
    param.Muon_ID_SF_Key = "NUM_HighPtID_DEN_genTracks";
    param.Muon_ISO_SF_Key = "NUM_LooseRelTkIso_DEN_HighPtIDandIPCut";
    param.Muon_Trigger_SF_Key = "POGHighPtLooseTrkIso";
    param.Muon_FR_ID = "HNWR";
    param.Muon_FR_Key = "AwayJetPt40";
    param.Muon_CF_ID = "HNWRTight";
    param.Muon_CF_Key = "ZToLL";
    param.Muon_UseMini = false;
    param.Muon_UsePtCone = false;
    param.Muon_UseTuneP = true;
    param.Muon_MinPt = 53.;

    param.Jet_ID = "HN";
    param.FatJet_ID = "HN";

    param.dRSeparation = 0.4;
    param.FatJet_dRSeparation = 0.8;

    executeEventFromParameter(param);

    //Run syst  # 457




}

void Reproduce20_002::executeEventFromParameter(AnalyzerParameter param) {
    const TString this_syst = systHelper->getCurrentSysName();

    Event ev = GetEvent();
    Particle METv = ev.GetMETVector();
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

    if (!PassNoiseFilter()) return;

    FillHist(this_syst + "/CutFlow", 1.0, weight, 10, 0., 10.); // Noise filter pass

    if (!(ev.PassTrigger(Muon_Trigger))) return;

    FillHist(this_syst + "/CutFlow", 2.0, weight, 10, 0., 10.); // HLT pass

    RVec<Electron> electrons = Allelectrons;
    RVec<Muon> muons = Allmuons;
    RVec<Jet> jets = Alljets;
    RVec<FatJet> fatjets = Allfatjets;
    

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
    RVec<Electron> my_electrons = SelectElectrons(electrons, "HNWRLT" , param.Electron_MinPt, 2.4);
    RVec<Muon> my_muons = SelectMuons(muons, "HNWRLT" , param.Muon_MinPt, 2.4);

    //prompt only 구현 필요    
    sort (my_electrons.begin(), my_electrons.end(), PtComparing);
    sort (my_muons.begin(), my_muons.end(), PtComparing);

    RVec<Electron *> Loose_electrons , Tight_electrons;
    RVec<Muon *> Loose_muons , Tight_muons;
    RVec<Lepton *> Tight_leps_el , Tight_leps_mu , Tight_leps;
    RVec<Lepton *> Loose_leps_el , Loose_leps_mu , Loose_leps;

    for (unsigned int i=0 ; i< my_electrons.size()l; i ++) {
        Electron & el = my_electrons.at(i);
        if (el.PassID(param.Electron_Tight_ID)) {
            Tight_electrons.push_back(&el);
            Tight_leps_el.push_back( &el);
            Tight_leps.push_back(&el);

        }
        else if (el.PassID(param.Electron_Loose_ID)) {
            Loose_electrons.push_back(&el);
            Loose_leps_el.push_back(&el);
            Loose_leps.push_back(&el);
        }
    }
    for (unsigned int i=0 ; i< my_muons.size()l; i ++) {
        Muon & mu = my_muons.at(i);
        if (mu.PassID(param.Muon_Tight_ID)) {
            Tight_muons.push_back(&mu);
            Tight_leps_mu.push_back( &mu);
            Tight_leps.push_back(&mu);
        }
        else if (mu.PassID(param.Muon_Loose_ID)) {
            Loose_muons.push_back(&mu);
            Loose_leps_mu.push_back(&mu);
            Loose_leps.push_back(&mu);
        }
    }
    sort (Tight_leps.begin(), Tight_leps.end(), PtComparingPtr);

    int n_Loose_leptons  = Loose_electrons.size() + Loose_muons.size();
    int n_Tight_leptons  = Tight_electrons.size() + Tight_muons.size();
    ///         Jets       ///


    RVec<Jet> selected_jets = SelectJets(jets, param.Jet_ID , 40.0, 2.4);
    sort (selected_jets.begin(), selected_jets.end(), PtComparing);

    ///         FatJets       ///

    RVec<FatJet> selected_fatjets = SelectFatJets(fatjets, param.FatJet_ID , 200.0, 2.4);
    sort (selected_fatjets.begin(), selected_fatjets.end(), PtComparing);

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

    double this_trigger_SF(1.0);

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

    if ( (n_Tight_leptons.size() ==2 ) && (Tight_muons[0].Pt() > 60.0) ) {

        FillHist(this_syst + "/CutFlow", 3.0, weight, 10, 0., 10.); // 2 tight leptons with pT cut
        
        if ( (Tight_electrons.size() == 2) && ( Tight_muons.size() == 0 )) {
            this_trigger_pass = true;
        }
        else if ( (Tight_muons.size() == 2) && ( Tight_electrons.size() == 0 )) {
            this_trigger_pass = true;
        }
        else if ( (Tight_muons.size() == 1) && ( Tight_electrons.size() == 1 )) {
            this_trigger_pass = true;
        }

        if (this_trigger_pass) {
            // needs 2 jets 
            if (jets.size() >= 2 ){
                Lepton *LeadLep = Tight_leps[0];
                Lepton *SubLeadLep = Tight_leps[1];

                // needs seperation 

                bool dRLeadJetLepton = ((jets[0].DeltaR(*LeadLep) > 0.4) &&  (jets[0].DeltaR(*SubLeadLep) > 0.4) );
                bool dRSubLeadJetLepton = ((jets[1].DeltaR(*LeadLep) > 0.4) &&  (jets[1].DeltaR(*SubLeadLep) > 0.4) );
                bool dRTwoLepton = (LeadLep->DeltaR(*SubLeadLep) > 0.4);
                bool dRTwoJets = (jets[0].DeltaR(jets[1]) > 0.4);

                

                if ( dRLeadJetLepton && dRSubLeadJetLepton && dRTwoLepton && dRTwoJets ){
                    bool IsResolvedEvent(true);
                    
                    // Mass calculation 

                }

                


            }
        }// # 1192 -> Resolved ends .
        





    




}


