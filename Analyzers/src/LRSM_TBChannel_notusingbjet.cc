#include "LRSM_TBChannel_notusingbjet.h"

LRSM_TBChannel_notusingbjet::LRSM_TBChannel_notusingbjet() {}
LRSM_TBChannel_notusingbjet::~LRSM_TBChannel_notusingbjet() {}

void LRSM_TBChannel_notusingbjet::initializeAnalyzer() {
    cout << "[LRSM_TBChannel_notusingbjet::initializeAnalyzer] Starting initialization" << endl;
    
    // Check user flags
    RunSyst = HasFlag("RunSyst");
    RunWRCut = HasFlag("RunWRCut");
    
    cout << "[LRSM_TBChannel_notusingbjet::initializeAnalyzer] RunSyst = " << RunSyst << endl;
    cout << "[LRSM_TBChannel_notusingbjet::initializeAnalyzer] RunWRCut = " << RunWRCut << endl;
    
    // Set WR mass cut threshold
    if (RunWRCut) {
        WRCutThreshold = SelectionCuts::WR_CUT_2000;
    } else {
        WRCutThreshold = SelectionCuts::NO_WR_CUT;
    }
    
    // Muon IDs and scale factor keys
    MuonIDs.clear();
    // Use more reasonable muon IDs for standard analysis
    // POG_GLOBAL_HIGH_PT is very restrictive (for >200 GeV muons)
    // For LRSM analysis, use Tight ID + isolation
    // Your data has HighPtId=1, so use POG_TRACKER_HIGH_PT instead of POG_GLOBAL_HIGH_PT
    MuonIDs.push_back(Muon::MuonID::POG_GLOBAL_HIGH_PT);  // This matches your data (HighPtId=2)
    MuonIDs.push_back(Muon::MuonID::POG_TKISO_TIGHT);      // TkIsoId=2
    
    // Alternative: Use standard IDs if high-pT selection isn't critical
    // MuonIDs.push_back(Muon::MuonID::POG_TIGHT);
    // MuonIDs.push_back(Muon::MuonID::POG_PFISO_TIGHT);
    

    // Jet IDs
    JetIDs = {Jet::JetID::NOCUT};
    
    // Era-dependent trigger settings
    if (DataEra == "2016preVFP" || DataEra == "2016postVFP" || DataEra == "2018") {
        IsoMuTriggerName = "HLT_IsoMu27";
        TriggerSafePtCut = 29.;
    } else if (DataEra == "2017") {
        IsoMuTriggerName = "HLT_IsoMu27"; 
        TriggerSafePtCut = 29.;
    } else if (DataEra == "2022") {
        Trigger1  = "HLT_Mu50";
        Trigger2  = "HLT_CascadeMu100";
        Trigger3  = "HLT_HighPtTkMu100";
        TriggerSafePtCut = 52.;
    } else if (DataEra == "2022EE") {
        Trigger1  = "HLT_Mu50";
        Trigger2  = "HLT_CascadeMu100";
        Trigger3  = "HLT_HighPtTkMu100";
        TriggerSafePtCut = 52.;
    } else if (DataEra == "2023") {
        Trigger1  = "HLT_Mu50";
        Trigger2  = "HLT_CascadeMu100";
        Trigger3  = "HLT_HighPtTkMu100";
        TriggerSafePtCut = 52.;
    } else if (DataEra == "2023BPix") {
        Trigger1  = "HLT_Mu50";
        Trigger2  = "HLT_CascadeMu100";
        Trigger3  = "HLT_HighPtTkMu100";
        TriggerSafePtCut = 52.;
    } else {
        cerr << "[LRSM_TBChannel_notusingbjet::initializeAnalyzer] DataEra is not set properly: " << DataEra << endl;
        exit(EXIT_FAILURE);
    }
    
    //cout << "[LRSM_TBChannel_notusingbjet::initializeAnalyzer] IsoMuTriggerName = " << IsoMuTriggerName << endl;
    cout << "[LRSM_TBChannel_notusingbjet::initializeAnalyzer] TriggerSafePtCut = " << TriggerSafePtCut << endl;
    

    /*
    // Initialize corrections
    if (IsDATA){
        if (DataEra == "2022") {
            corr_C = new MyCorrection("2022", "C", MCSample, true);
            corr_D = new MyCorrection("2022", "D", MCSample, true);
            corr_sm = new MyCorrection("2022", "SingleMuon", MCSample, true);

            
        } else if (DataEra == "2022EE") {
            corr_E = new MyCorrection("2022EE", "E", MCSample, true);
            corr_F = new MyCorrection("2022EE", "F", MCSample, true);
            corr_G = new MyCorrection("2022EE", "G", MCSample, true);
        }
        
    } else{
        if (DataEra == "2022") {
            corr_C = new MyCorrection("2022", "C", MCSample, false);
            corr_D = new MyCorrection("2022", "D", MCSample, false);
            corr_sm = new MyCorrection("2022", "SingleMuon", MCSample, false);
        } else if (DataEra == "2022EE") {
            corr_E = new MyCorrection("2022EE", "E", MCSample, false);
            corr_F = new MyCorrection("2022EE", "F", MCSample, false);
            corr_G = new MyCorrection("2022EE", "G", MCSample, false);
        }
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
    
    cout << "[LRSM_TBChannel_notusingbjet::initializeAnalyzer] Initialization complete" << endl;
}

void LRSM_TBChannel_notusingbjet::executeEvent() {
    // Get all physics objects at the beginning to save CPU time
    AllMuons = GetAllMuons();
    AllJets = GetAllJets();
    AllFatJets = GetAllFatJets();
    
    
    
    // Loop over systematic sources
    for (const auto &syst_dummy : *systHelper) {
        executeEventFromParameter();
    }
}

void LRSM_TBChannel_notusingbjet::executeEventFromParameter() {
    const TString this_syst = systHelper->getCurrentSysName();
    
    // Get event information
    Event ev = GetEvent();

    float weight = 1.0;
    for (auto& muon : AllMuons) {
        float muon_scale_sf = myCorr->GetMuonScaleSF(muon, MyCorrection::variation::nom, 0.0);
        muon.SetPtEtaPhiM(muon.Pt() * muon_scale_sf, muon.Eta(), muon.Phi(), muon.M());
    }

    if(!IsDATA){
        weight *= MCweight();
        weight *= ev.GetTriggerLumi("Full");
        
        // Muon Scale Factor로 4-momentum 보정
        /*
        for (auto& jet : AllJets) {
            float jes_sf = myCorr->GetJESSF(0.4, jet.Eta(), jet.Pt(), jet.Phi(), ev.GetRho(), ev.run());
            jet.SetPtEtaPhiM(jet.Pt() * jes_sf, jet.Eta(), jet.Phi(), jet.M());
        }
        
        // FatJet JES 적용
        for (auto& fatjet : AllFatJets) {
            float jes_sf = myCorr->GetJESSF(0.8, fatjet.Eta(), fatjet.Pt(), fatjet.Phi(), ev.GetRho(), ev.run());
            fatjet.SetPtEtaPhiM(fatjet.Pt() * jes_sf, fatjet.Eta(), fatjet.Phi(), fatjet.M());
        }
        
        // JER 적용 (선택적)
        for (auto& jet : AllJets) {
            float jer_sf = myCorr->GetJERSF(jet.Eta(), jet.Pt(), MyCorrection::variation::nom);
            jet.SetPtEtaPhiM(jet.Pt() * jer_sf, jet.Eta(), jet.Phi(), jet.M());
        }*/
    }


    FillHist(this_syst + "/sumSign" + this_syst, sumSign, 1 , 10 , 0 , 1e+11 );
    FillHist(this_syst + "/CutFlow", 0.0, weight, 10, 0., 10.); // Initial event
    FillHist(this_syst + "/CutFlow_wtagging", 0.0, weight, 10, 0., 10.); // Initial event
    // Apply HLT trigger
    if (!(ev.PassTrigger(Trigger1)||ev.PassTrigger(Trigger2)||ev.PassTrigger(Trigger3))) return;
    
    FillHist(this_syst + "/CutFlow", 1.0, weight, 10, 0., 10.); // HLT pass
    FillHist(this_syst + "/CutFlow_wtagging", 1.0, weight, 10, 0., 10.); // HLT pass
    // Copy physics objects for systematic variations
    RVec<Muon> muons = AllMuons;
    RVec<Jet> jets = AllJets;
    RVec<FatJet> fatjets = AllFatJets;
    
    
    
    
    

    // Muon Id pass 
    bool hasGoodMuon = false;
    
    for (const auto& muon : muons) {
        // Debug output to see actual ID values
        FillHist(this_syst + "/MuonhighPtid",muon.PassID(MuonIDs[0]), 1.0, 10, -5., 5.);
        FillHist(this_syst + "/Muonisoid",muon.PassID(MuonIDs[1]), 1.0, 10, -5., 5.);
        
        // Fill additional histograms to understand what IDs are available
        FillHist(this_syst + "/Muon_HighPtId", (int)muon.HighPtId(), 1.0,  10, -5., 5.);
        FillHist(this_syst + "/Muon_TkIsoId", (int)muon.TkIsoId(), 1.0,  10, -5., 5.);
        FillHist(this_syst + "/Muon_TightId", muon.isPOGTightId(), 1.0, 3, 0., 3.);
        FillHist(this_syst + "/Muon_MediumId", muon.isPOGMediumId(), 1.0, 3, 0., 3.);
        FillHist(this_syst + "/Muon_LooseId", muon.isPOGLooseId(), 1.0, 3, 0., 3.);
        
        if (muon.PassID(MuonIDs[1]) and muon.PassID(MuonIDs[0]) ) {
            hasGoodMuon = true;
            break;
        }
    }   
    if (!hasGoodMuon) return;

    // Apply muon selection
    FillHist(this_syst + "/CutFlow", 2.0, weight, 10, 0., 10.); // 2 muons
    FillHist(this_syst + "/CutFlow_wtagging", 2.0, weight, 10, 0., 10.); // 2 muons
    muons = RemoveOverlap(muons);
    // Require more than 2 muons
    if (muons.size() < 2) return;
    
    // Sort muons by pT
    sort(muons.begin(), muons.end(), PtComparing);
    
    // Apply kinematic cuts
    if (!PassKinematicCuts(muons)) return;
    FillHist(this_syst + "/CutFlow_wtagging", 3.0, weight, 10, 0., 10.); // Kinematic cuts
    FillHist(this_syst + "/CutFlow", 3.0, weight, 10, 0., 10.); // Kinematic cuts
    
    // Apply dilepton mass cut
    if (!PassDileptonMassCut(muons)) return;
    
    Muon muon1 = muons[0];
    Muon muon2 = muons[1];
    muon_overlap_cleaned = { muon1, muon2 };
    


    FillHist(this_syst + "/CutFlow", 4.0, weight, 10, 0., 10.); // Dilepton mass cut
    FillHist(this_syst + "/CutFlow_wtagging", 4.0, weight, 10, 0., 10.); // Dilepton mass cut
    
    
    // Select fat jets and remove overlaps
    // FatJet selection - using basic kinematic cuts for now
    RVec<FatJet> selected_fatjets;
    for (const auto& fj : fatjets) {
        if (fj.Pt() > cuts.fatjet_pt && abs(fj.Eta()) < cuts.fatjet_eta) {
            selected_fatjets.push_back(fj);
        }
    }
    fatjets = selected_fatjets;
    FillHist(this_syst + "/FatJetnum", fatjets.size(), 1.0, 10, 0., 10.);
    fatjets = RemoveOverlapWithMuonsFatJet(fatjets, muon_overlap_cleaned);
    FillHist(this_syst + "/FatJetnum_afterOverlap", fatjets.size(), 1.0, 10, 0., 10.);
    RVec<FatJet> topjets = SelectTopTaggedJets(fatjets);
    
    for (const auto& fatjet : fatjets) {
        // Using basic mass cuts for top tagging - update with actual tagger when available
        float toptag_score1 = fatjet.GetTaggerResult(JetTagging::FatJetTaggingtype::ParticleNetWithMass, JetTagging::FatjetTaggingObject::TvsQCD); // placeholder
        float softdrop_mass1 = fatjet.SDMass();
        FillHist(this_syst + "/FatJet_SoftDropMass", softdrop_mass1, 1.0, 100, 0., 1000.);
        FillHist(this_syst + "/FatJet_TopTagScore", toptag_score1, 1.0, 100, 0., 1.);
        FillHist("pt eta  , overlap passed fatjet sdm , top score ",toptag_score1,softdrop_mass1, weight,100,0.,1.,300,0.,300.);
        
    }
    for (const auto& topjet : topjets) {
        float toptag_score2 = topjet.GetTaggerResult(JetTagging::FatJetTaggingtype::ParticleNetWithMass, JetTagging::FatjetTaggingObject::TvsQCD);
        float softdrop_mass2 = topjet.SDMass();
        float topjet_pt = topjet.Pt();
        FillHist(this_syst + "/topJet_SoftDropmass", softdrop_mass2, 1.0, 100, 0., 1000.);
        FillHist(this_syst + "/topJet_TopTagScore", toptag_score2, 1.0, 100, 0., 1.);
        FillHist("top jets topscore & sdm",toptag_score2,softdrop_mass2, weight,100,0.,1.,300,0.,300.);
        FillHist("top jets topscore & pt",toptag_score2,topjet_pt, weight,100,0.,1.,300,0.,3000.);
        FillHist("top jets sdm & pt",softdrop_mass2,topjet_pt, weight,300,0.,300.,300,0.,3000.);
    }
    if (topjets.size()<1){
        for (const auto& fatjet : fatjets) {
        // Using basic mass cuts for top tagging - update with actual tagger when available
        float wtag_score1 = fatjet.GetTaggerResult(JetTagging::FatJetTaggingtype::ParticleNetWithMass, JetTagging::FatjetTaggingObject::WvsQCD); // placeholder
        float wsoftdrop_mass1 = fatjet.SDMass();
        FillHist("no top tagged all fatjet wtagg score sdm", wtag_score1, wsoftdrop_mass1, 1.0, 100, 0., 1., 300, 0., 300.);
        }
        RVec<FatJet> wtagged_fatjet = SelectWTaggedJets(fatjets); 
        if (wtagged_fatjet.size()<1) {
            FillHist("no_toptagg_no_tagg_num",1,1,1,0,1);
            return;
        }
        for (const auto& fatjet : wtagged_fatjet) {
            float wtag_score2 = fatjet.GetTaggerResult(JetTagging::FatJetTaggingtype::ParticleNetWithMass, JetTagging::FatjetTaggingObject::WvsQCD); // placeholder
            float wsoftdrop_mass2 = fatjet.SDMass();
            FillHist("no top tagged , w tagged fatjet wtagg score sdm", wtag_score2, wsoftdrop_mass2, 1.0, 100, 0., 1., 300, 0., 300.);
        }
        FillHist(this_syst + "/CutFlow_wtagging", 5.0, weight, 10, 0., 10.); // w tagged , no top tagged
        FillHist("no top tagged w tagged num",1, 1, 1, 0, 1);
        RVec<FatJet> leading_wtagged_fatjet = {wtagged_fatjet[0]} ;
        float wtagscore = leading_wtagged_fatjet[0].GetTaggerResult(JetTagging::FatJetTaggingtype::ParticleNetWithMass, JetTagging::FatjetTaggingObject::WvsQCD);
        FillHist("Wtagging_score_when_no_toptagged",wtagscore,1,100,0,1);
        jetss = SelectJets(jets, JetIDs[0], cuts.jet_pt, cuts.jet_eta);
        jetss = RemoveOverlapWithMuons(jetss, muon_overlap_cleaned);
        bjetss = SelectBTaggedJets(jetss);
        sort(bjetss.begin(), bjetss.end(), PtComparing);
        FillHist("no top tagged b jet num", bjetss.size(), 1, 5,0,5);
        if (bjetss.size()<2) return;
        FillHist(this_syst + "/CutFlow_wtagging", 6.0, weight, 10, 0., 10.); // b jet and w jet
        float wtagged_topmass = (bjetss[1]+ leading_wtagged_fatjet[0]).M();
        FillHist("no top tagged b jet sublead + W mass", wtagged_topmass,1,1000,0,1000);
        
        
        float wr_mass = (bjetss[0]+ bjetss[1]+leading_wtagged_fatjet[0]+ muon_overlap_cleaned[0]+ muon_overlap_cleaned[1]).M();
        float dilepton_mass = (muon_overlap_cleaned[0] + muon_overlap_cleaned[1]).M();
        
        FillHist("no top tagged w tagged WRMass" , wr_mass, weight, 8000, 0., 8000.);
        FillHist("no top tagged w tagged DileptonMass" , dilepton_mass, weight, 5000, 0., 5000.);
        FillHist( "no top tagged w tagged LeadingMuonPt", muon_overlap_cleaned[0].Pt(), weight, 5000, 0., 5000.);
        FillHist( "no top tagged w tagged SubleadingMuonPt", muon_overlap_cleaned[1].Pt(), weight, 5000, 0., 5000.);
        FillHist( "no top tagged w tagged LeadingBJetPt" , bjetss[0].Pt(), weight, 5000, 0., 5000.);
        FillHist( "no top tagged w tagged SubLeadingBJetPt" , bjetss[1].Pt(), weight, 5000, 0., 5000.);
        FillHist( "no top tagged w tagged LeadingWJetPt" , leading_wtagged_fatjet[0].Pt(), weight, 5000, 0., 5000.);



    }
    if (topjets.size() < 1) return;
    sort(topjets.begin(), topjets.end(), PtComparing);
    RVec<FatJet> leading_topjet = {topjets[0]};
    float leading_topjet_sdm = leading_topjet[0].SDMass();
    float leading_topjet_topscore = leading_topjet[0].GetTaggerResult(JetTagging::FatJetTaggingtype::ParticleNetWithMass, JetTagging::FatjetTaggingObject::TvsQCD);
    float leading_topjet_wscore = leading_topjet[0].GetTaggerResult(JetTagging::FatJetTaggingtype::ParticleNetWithMass, JetTagging::FatjetTaggingObject::WvsQCD);
    float leading_topjet_pt = leading_topjet[0].Pt();
    FillHist("top tagged jet w tagging score", leading_topjet_wscore ,1, 100 ,0,1);
    FillHist("used top jet sdm & topscore",leading_topjet_topscore,leading_topjet_sdm, weight,100,0.,1.,300,0.,300.);
    FillHist("used top jet topscore& pt", leading_topjet_topscore,leading_topjet_pt, weight,100,0.,1.,300,0.,3000.);
    FillHist("used top jet sdm & pt",leading_topjet_sdm,leading_topjet_pt, weight,300,0.,300.,300,0.,3000.);

    FillHist(this_syst + "/CutFlow", 5.0, weight, 10, 0., 10.);
    // Remove overlap between jets and fat jets
    jets = SelectJets(jets, JetIDs[0], cuts.jet_pt, cuts.jet_eta);
    jets = RemoveOverlapWithMuons(jets, muon_overlap_cleaned);
    jets = RemoveOverlapWithFatJets(jets, leading_topjet);
    //RVec<Jet> bjets = SelectBTaggedJets(jets);
    RVec<Jet> bjets = jets;
    sort(bjets.begin(), bjets.end(), PtComparing);
    if (bjets.size() < 1 ) return;
    
    RVec<Jet> leading_bjet = {bjets[0]};
    FillHist(this_syst + "/CutFlow", 6.0, weight, 10, 0., 10.); // b-jet and top-jet
    
    
    
    
    // Calculate invariant masses
    float wr_mass = CalculateWRMass(muon_overlap_cleaned, leading_bjet, leading_topjet);
    float dilepton_mass = (muon_overlap_cleaned[0] + muon_overlap_cleaned[1]).M();
    
    // Apply WR mass cut if requested
   
    
    
    FillHist(this_syst + "/CutFlow", 7.0,weight, 10, 0., 10.); // WR mass cut (if applied)
    string CR = "CR";
    string CROP = "CR_opposite";
    string SR = "SR";
    string test = "test";
    string all = "all_WR";

    //SF apply 
    if (!IsDATA) {
        weight *= myCorr->GetMuonRECOSF(muon_overlap_cleaned, MyCorrection::variation::nom);
        //weight *= myCorr->GetMuonIDSF("NUM_probe_TightRelTkIso_DEN_HighPtProbes", muon_overlap_cleaned, MyCorrection::variation::nom);
        weight *= myCorr->GetPUWeight(ev.nTrueInt(), MyCorrection::variation::nom);
        //weight *= myCorr->GetMuonTriggerSF("NUM_HLT_DEN_HighPtTightRelIsoProbes", muon_overlap_cleaned, MyCorrection::variation::nom);
        weight *= myCorr->GetTopPtReweight(GetAllGens());
    }

    

    float met_pt = ev.GetMETVector(Event::MET_Type::PUPPI,Event::MET_Syst::CENTRAL).Pt();
    float met_phi = ev.GetMETVector(Event::MET_Type::PUPPI,Event::MET_Syst::CENTRAL).Phi();
    float b_l_met_transverse_mass ;
    if (!IsDATA) {
        FillHist(this_syst + "/WRmass vs Met" + all, wr_mass, met_pt, weight, 8000, 0., 8000., 500, 0., 1000.);
        //delta eta , phi , R with top and bjet 
        FillHist(this_syst + "/delta eta with top and bjet" + all, wr_mass,leading_bjet[0].Eta() - leading_topjet[0].Eta(), weight, 8000,0., 8000., 100, -10., 10.);
        FillHist(this_syst + "/delta phi with top and bjet" + all, wr_mass,leading_bjet[0].Phi() - leading_topjet[0].Phi(), weight, 8000,0., 8000., 100, -5., 5.);
        FillHist(this_syst + "/delta R with top and bjet" + all, wr_mass,leading_bjet[0].DeltaR(leading_topjet[0]), weight, 8000,0., 8000., 100, 0., 5.);
        FillHist(this_syst + "/WRMass & DileptonMass" + all, wr_mass, dilepton_mass, weight, 100, 0., 8000., 500, 0., 5000.);
        FillHist(this_syst + "/WRMass & LeadingMuonPt" + all, wr_mass, muon_overlap_cleaned[0].Pt(), weight, 100, 0., 8000., 500, 0., 5000.);
        FillHist(this_syst + "/WRMass & SubleadingMuonPt" + all, wr_mass, muon_overlap_cleaned[1].Pt(), weight, 100, 0., 8000., 500, 0., 5000.);
        FillHist(this_syst + "/WRMass & LeadingBJetPt" + all, wr_mass, leading_bjet[0].Pt(), weight, 100, 0., 8000., 500, 0., 5000.);
        FillHist(all + "/WRMass & LeadingTopJetPt" + all,wr_mass,leading_topjet[0].Pt(), weight,100,0.,8000.,500,0.,5000.);
        // met pt & leading muon pt
        FillHist(this_syst + "/met pt & leading muon pt" + all,met_pt,muon_overlap_cleaned[0].Pt(), weight,100,0.,5000.,500,0.,5000.);
        //delta r with lepton and b jet 
        FillHist(this_syst + "/delta r with lepton and b jet vs MET" + all, met_pt, muon_overlap_cleaned[0].DeltaR(leading_bjet[0]), weight, 100, 0., 1000., 100, 0., 5.);
        FillHist(this_syst + "/delta r with top and b jet vs lepton pt" + all, muon_overlap_cleaned[0].Pt(), leading_topjet[0].DeltaR(leading_bjet[0]), weight, 500, 0., 1000., 100, 0., 5.);
        //lead lepton pt / subleading lepton pt
        FillHist(this_syst + "/delta r t b vs lead lepton / subleading lepton pt" + all, muon_overlap_cleaned[0].Pt() / muon_overlap_cleaned[1].Pt(), leading_topjet[0].DeltaR(leading_bjet[0]), weight, 500, 0., 100., 100, 0., 5.);
        // sub lead pt vs met 
        FillHist(this_syst + "/sub lead pt vs met" + all, muon_overlap_cleaned[1].Pt(), met_pt, weight, 500, 0., 1000., 100, 0., 1000.);
        //counting met 
        if (met_pt < 50){
            FillHist(this_syst + "/met pt " + all, 0, weight, 5, 0, 5);
        }
        if (met_pt >50 && met_pt < 100){
            FillHist(this_syst + "/met pt " + all, 1, weight, 5, 0, 5);
        }
        if (met_pt >100 && met_pt < 200){
            FillHist(this_syst + "/met pt " + all, 2, weight, 5, 0, 5);
        }   
        if (met_pt >200 && met_pt < 300){
            FillHist(this_syst + "/met pt " + all, 3, weight, 5, 0, 5);
        }
        if (met_pt >300 && met_pt < 400){
            FillHist(this_syst + "/met pt " + all, 4, weight, 5, 0, 5);
        }
        // using tb system 
        float tb_mass = (leading_bjet[0] + leading_topjet[0]).M();
        float tb_phi = (leading_bjet[0] + leading_topjet[0]).Phi();
        float tb_eta = (leading_bjet[0] + leading_topjet[0]).Eta();
        float tb_pt = (leading_bjet[0] + leading_topjet[0]).Pt();
        // tb & sublead muon delta r 
        float tb_sublead_muon_delta_r = leading_bjet[0].DeltaR(muon_overlap_cleaned[1]);
        float tb_sublead_muon_delta_phi = tb_phi - muon_overlap_cleaned[1].Phi();
        FillHist(this_syst + "/tb mass" + all, tb_mass, weight, 8000, 0., 8000.);
        FillHist(this_syst + "/tb phi" + all, tb_phi, weight, 100, 0., 5.);
        FillHist(this_syst + "/tb eta" + all, tb_eta, weight, 100, 0., 100.);
        FillHist(this_syst + "/tb pt" + all, tb_pt, weight, 100, 0., 1000.);
        FillHist(this_syst + "/tb sublead muon delta r" + all, tb_sublead_muon_delta_r, weight, 100, 0., 5.);
        FillHist(this_syst + "/tb sublead muon delta phi" + all, tb_sublead_muon_delta_phi, weight, 100, 0., 5.);
        // b l met transverse mass 
        float b_l_met_transverse_mass = CalculateTransverseMass(muon_overlap_cleaned[0], leading_bjet[0], met_pt, met_phi);
        FillHist(this_syst + "/b met transverse mass" + all, b_l_met_transverse_mass, weight, 200, 0., 2000.);
        // CR setting 
        FillHist(this_syst + "/mt vs subleading lepton pt" + all,b_l_met_transverse_mass, muon_overlap_cleaned[1].Pt(), weight, 300, 0., 3000., 400, 0., 4000.);
        FillHist(this_syst + "/mt vs wr mass" + all,b_l_met_transverse_mass, wr_mass, weight, 300, 0., 3000., 800, 0., 8000.);
        if (b_l_met_transverse_mass < 250.00){ // for CR
            if ( muon_overlap_cleaned[1].Pt() < 50 ){
                FillHist(this_syst + "/xsec" + CR, xsec , weight, 100 , 0 , 1000 );
                FillHist(this_syst + "/Bjetnum" + CR, bjets.size(), weight, 10, 0., 10.);
                FillHist(this_syst + "/Topjetnum" + CR, topjets.size(), weight, 10, 0., 10.);
                FillHist(this_syst + "/WRMass_" + CR, wr_mass, weight, 8000, 0., 8000.);
                FillHist(this_syst + "/DileptonMass_" + CR, dilepton_mass, weight, 5000, 0., 5000.);
                FillHist(this_syst + "/LeadingMuonPt_" + CR, muon_overlap_cleaned[0].Pt(), weight, 5000, 0., 5000.);
                FillHist(this_syst + "/SubleadingMuonPt_" + CR, muon_overlap_cleaned[1].Pt(), weight, 5000, 0., 5000.);
                FillHist(this_syst + "/LeadingBJetPt_" + CR, leading_bjet[0].Pt(), weight, 5000, 0., 5000.);
                FillHist(this_syst + "/LeadingTopJetPt_" + CR, leading_topjet[0].Pt(), weight, 5000, 0., 5000.);

                // variable 중에서 어떤게 SR 에 영향 주는 지 2d plot 으로 파악
                FillHist(this_syst + "/WRMass & DileptonMass" + CR, wr_mass, dilepton_mass, weight, 100, 0., 8000., 500, 0., 5000.);
                FillHist(this_syst + "/WRMass & LeadingMuonPt" + CR, wr_mass, muon_overlap_cleaned[0].Pt(), weight, 100, 0., 8000., 500, 0., 5000.);
                FillHist(this_syst + "/WRMass & SubleadingMuonPt" + CR, wr_mass, muon_overlap_cleaned[1].Pt(), weight, 100, 0., 8000., 500, 0., 5000.);
                FillHist(this_syst + "/WRMass & LeadingBJetPt" + CR, wr_mass, leading_bjet[0].Pt(), weight, 100, 0., 8000., 500, 0., 5000.);
                FillHist(CR + "/WRMass & LeadingTopJetPt" + CR,wr_mass,leading_topjet[0].Pt(), weight,100,0.,8000.,500,0.,5000.);
                // met pt & leading muon pt
                FillHist(this_syst + "/met pt & leading muon pt" + CR,met_pt,muon_overlap_cleaned[0].Pt(), weight,100,0.,5000.,500,0.,5000.);
            }
        }

        if (b_l_met_transverse_mass > 250.00){ // for SR
            if ( muon_overlap_cleaned[1].Pt() > 50 ){
                FillHist(this_syst + "/Bjetnum" + SR, bjets.size(), weight, 10, 0., 10.);
                FillHist(this_syst + "/Topjetnum" + SR, topjets.size(), weight, 10, 0., 10.);
                FillHist(this_syst + "/WRMass_" + SR, wr_mass, weight, 8000, 0., 8000.);
                FillHist(this_syst + "/DileptonMass_" + SR, dilepton_mass, weight, 5000, 0., 5000.);
                FillHist(this_syst + "/LeadingMuonPt_" + SR, muon_overlap_cleaned[0].Pt(), weight, 5000, 0., 5000.);
                FillHist(this_syst + "/SubleadingMuonPt_" + SR, muon_overlap_cleaned[1].Pt(), weight, 5000, 0., 5000.);
                FillHist(this_syst + "/LeadingBJetPt_" + SR, leading_bjet[0].Pt(), weight, 5000, 0., 5000.);
                FillHist(this_syst + "/LeadingTopJetPt_" + SR, leading_topjet[0].Pt(), weight, 5000, 0., 5000.);

                FillHist(this_syst + "/WRMass & DileptonMass" + SR, wr_mass, dilepton_mass, weight, 100, 0., 8000., 500, 0., 5000.);
                FillHist(this_syst + "/WRMass & LeadingMuonPt" + SR, wr_mass, muon_overlap_cleaned[0].Pt(), weight, 100, 0., 8000., 500, 0., 5000.);
                FillHist(this_syst + "/WRMass & SubleadingMuonPt" + SR, wr_mass, muon_overlap_cleaned[1].Pt(), weight, 100, 0., 8000., 500, 0., 5000.);
                FillHist(this_syst + "/WRMass & LeadingBJetPt" + SR, wr_mass, leading_bjet[0].Pt(), weight, 100, 0., 8000., 500, 0., 5000.);
                FillHist(this_syst + "/WRMass & LeadingTopJetPt" + SR, wr_mass, leading_topjet[0].Pt(), weight, 100, 0., 8000., 500, 0., 5000.);
                FillHist(this_syst + "/met pt & leading muon pt" + SR, met_pt, muon_overlap_cleaned[0].Pt(), weight, 100, 0., 5000., 500, 0., 5000.);
                }
            FillHist(this_syst + "/Bjetnum" + CROP, bjets.size(), weight, 10, 0., 10.);
            FillHist(this_syst + "/Topjetnum" + CROP, topjets.size(), weight, 10, 0., 10.);
            FillHist(this_syst + "/WRMass_" + CROP, wr_mass, weight, 8000, 0., 8000.);
            FillHist(this_syst + "/DileptonMass_" + CROP, dilepton_mass, weight, 5000, 0., 5000.);
            FillHist(this_syst + "/LeadingMuonPt_" + CROP, muon_overlap_cleaned[0].Pt(), weight, 5000, 0., 5000.);
            FillHist(this_syst + "/SubleadingMuonPt_" + CROP, muon_overlap_cleaned[1].Pt(), weight, 5000, 0., 5000.);
            FillHist(this_syst + "/LeadingBJetPt_" + CROP, leading_bjet[0].Pt(), weight, 5000, 0., 5000.);
            FillHist(this_syst + "/LeadingTopJetPt_" + CROP, leading_topjet[0].Pt(), weight, 5000, 0., 5000.);
            FillHist(this_syst + "/WRMass & DileptonMass" + CROP, wr_mass, dilepton_mass, weight, 100, 0., 8000., 500, 0., 5000.);
            FillHist(this_syst + "/WRMass & LeadingMuonPt" + CROP, wr_mass, muon_overlap_cleaned[0].Pt(), weight, 100, 0., 8000., 500, 0., 5000.);
            FillHist(this_syst + "/WRMass & SubleadingMuonPt" + CROP, wr_mass, muon_overlap_cleaned[1].Pt(), weight, 100, 0., 8000., 500, 0., 5000.);
            FillHist(this_syst + "/WRMass & LeadingBJetPt" + CROP, wr_mass, leading_bjet[0].Pt(), weight, 100, 0., 8000., 500, 0., 5000.);
            FillHist(this_syst + "/WRMass & LeadingTopJetPt" + CROP, wr_mass, leading_topjet[0].Pt(), weight, 100, 0., 8000., 500, 0., 5000.);
            FillHist(this_syst + "/met pt & leading muon pt" + CROP, met_pt, muon_overlap_cleaned[0].Pt(), weight, 100, 0., 5000., 500, 0., 5000.);
            }
        // Apply systematic weights
        //unordered_map<std::string, float> weight_map = systHelper->calculateWeight();
        //for (const auto &w : weight_map) {
        //    TString weight_suffix = w.first;
        //    float total_weight = weight * w.second;
            
            // Fill histograms with systematic weights
        //    FillHist(this_syst + "/WRMass_" + weight_suffix, wr_mass, total_weight, 100, 0., 8000.);
            //FillHist(this_syst + "/DileptonMass_" + weight_suffix, dilepton_mass, total_weight, 100, 0., 8000.);
            //FillHist(this_syst + "/LeadingMuonPt_" + weight_suffix, muons[0].Pt(), total_weight, 100, 0., 8000.);
            //FillHist(this_syst + "/SubleadingMuonPt_" + weight_suffix, muons[1].Pt(), total_weight, 100, 0., 8000.);
            //FillHist(this_syst + "/LeadingBJetPt_" + weight_suffix, leading_bjet[0].Pt(), total_weight, 100, 0., 8000.);
            //FillHist(this_syst + "/LeadingTopJetPt_" + weight_suffix, leading_topjet[0].Pt(), total_weight, 100, 0., 8000.);
        //}
    } 
    if (IsDATA) {
        if ( b_l_met_transverse_mass > 250.00) return;
        if ( muon_overlap_cleaned[1].Pt() < 50 ){
        // Using only wr less than 800 GeV ( for CR )
        FillHist(this_syst + "/Bjetnum" + CR, bjets.size(), 1, 10, 0., 10.);
        FillHist(this_syst + "/Topjetnum" + CR, topjets.size(), 1, 10, 0., 10.);
        FillHist(this_syst + "/WRMass_" + CR, wr_mass, 1, 8000, 0., 8000.);
        FillHist(this_syst + "/DileptonMass_" + CR, dilepton_mass, 1, 5000, 0., 5000.);
        FillHist(this_syst + "/LeadingMuonPt_" + CR, muon_overlap_cleaned[0].Pt(), 1, 5000, 0., 5000.);
        FillHist(this_syst + "/SubleadingMuonPt_" + CR, muon_overlap_cleaned[1].Pt(), 1, 5000, 0., 5000.);
        FillHist(this_syst + "/LeadingBJetPt_" + CR, leading_bjet[0].Pt(), 1, 5000, 0., 5000.);
        FillHist(this_syst + "/LeadingTopJetPt_" + CR, leading_topjet[0].Pt(), 1, 5000, 0., 5000.);

        FillHist(this_syst + "/WRMass & DileptonMass" + CR, wr_mass, dilepton_mass, 1, 100, 0., 8000., 500, 0., 5000.);
        FillHist(this_syst + "/WRMass & LeadingMuonPt" + CR, wr_mass, muon_overlap_cleaned[0].Pt(), 1, 100, 0., 8000., 500, 0., 5000.);
        FillHist(this_syst + "/WRMass & SubleadingMuonPt" + CR, wr_mass, muon_overlap_cleaned[1].Pt(), 1, 100, 0., 8000., 500, 0., 5000.);
        FillHist(this_syst + "/WRMass & LeadingBJetPt" + CR, wr_mass, leading_bjet[0].Pt(), 1, 100, 0., 8000., 500, 0., 5000.);
        FillHist(this_syst + "/WRMass & LeadingTopJetPt" + CR, wr_mass, leading_topjet[0].Pt(), 1, 100, 0., 8000., 500, 0., 5000.);
        FillHist(this_syst + "/met pt & leading muon pt" + CR, met_pt, muon_overlap_cleaned[0].Pt(), 1, 100, 0., 5000., 500, 0., 5000.);
        }
    }
}

// Helper function implementations



RVec<Jet> LRSM_TBChannel_notusingbjet::SelectBTaggedJets(const RVec<Jet>& jets) {
    RVec<Jet> btagged_jets;
    for (const auto& jet : jets) {
        if (jet.GetBTaggerResult(JetTagging::JetFlavTagger::ParticleNet) > cuts.btag_wp) {
            btagged_jets.push_back(jet); 
        }
    }
    return btagged_jets;
}

RVec<FatJet> LRSM_TBChannel_notusingbjet::SelectTopTaggedJets(const RVec<FatJet>& fatjets) {
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


RVec<FatJet> LRSM_TBChannel_notusingbjet::SelectWTaggedJets(const RVec<FatJet>& fatjets) {
    RVec<FatJet> wtagged_jets;
    for (const auto& fatjet : fatjets) {
        // Using basic mass cuts for top tagging - update with actual tagger when available
        float wtag_score = fatjet.GetTaggerResult(JetTagging::FatJetTaggingtype::ParticleNetWithMass, JetTagging::FatjetTaggingObject::WvsQCD); // placeholder
        float softdrop_mass = fatjet.SDMass();
        
        if (wtag_score > cuts.wtag_score &&
            softdrop_mass > cuts.wtag_mass_low &&
            softdrop_mass < cuts.wtag_mass_high) {
            wtagged_jets.push_back(fatjet);
        }
    }
    return wtagged_jets;
}

RVec<Muon> LRSM_TBChannel_notusingbjet::RemoveOverlap(const RVec<Muon>& muons, float deltaR_cut) {
    RVec<Muon> cleaned_muons;
    for (size_t i = 0; i < muons.size(); ++i) {
        bool overlaps = false;
        for (size_t j = i + 1; j < muons.size(); ++j) {
            if (muons[i].DeltaR(muons[j]) < deltaR_cut) {
                overlaps = true;
                break;
            }
        }
        if (!overlaps) {
            cleaned_muons.push_back(muons[i]);
        }
    }
    return cleaned_muons;
}

RVec<Jet> LRSM_TBChannel_notusingbjet::RemoveOverlapWithMuons(const RVec<Jet>& jets, const RVec<Muon>& muons, float deltaR_cut) {
    RVec<Jet> cleaned_jets;
    for (const auto& jet : jets) {
        bool overlaps = false;
        for (const auto& muon : muons) {
            if (jet.DeltaR(muon) < deltaR_cut) {
                overlaps = true;
                break;
            }
        }
        if (!overlaps) {
            cleaned_jets.push_back(jet);
        }
    }
    return cleaned_jets;
}

RVec<Jet> LRSM_TBChannel_notusingbjet::RemoveOverlapWithFatJets(const RVec<Jet>& jets, const RVec<FatJet>& fatjets, float deltaR_cut) {
    RVec<Jet> cleaned_jets;
    for (const auto& jet : jets) {
        bool overlaps = false;
        for (const auto& fatjet : fatjets) {
            if (jet.DeltaR(fatjet) < deltaR_cut) {
                overlaps = true;
                break;
            }
        }
        if (!overlaps) {
            cleaned_jets.push_back(jet);
        }
    }
    return cleaned_jets;
}

RVec<FatJet> LRSM_TBChannel_notusingbjet::RemoveOverlapWithMuonsFatJet(const RVec<FatJet>& fatjets, const RVec<Muon>& muons, float deltaR_cut) {
    RVec<FatJet> cleaned_fatjets;
    for (const auto& fatjet : fatjets) {
        bool overlaps = false;
        for (const auto& muon : muons) {
            if (fatjet.DeltaR(muon) < deltaR_cut) {
                overlaps = true;
                break;
            }
        }
        if (!overlaps) {
            cleaned_fatjets.push_back(fatjet);
        }
    }
    return cleaned_fatjets;
}

bool LRSM_TBChannel_notusingbjet::PassKinematicCuts(const RVec<Muon>& muons) {
    if (muons.size() < 2) return false;
    
    // Leading muon pT cut
    if (muons[0].Pt() <= cuts.muon_pt ) return false;
    if (muons[1].Pt() <= cuts.muon_sub_pt ) return false;

    
    // Eta cuts
    for (const auto& muon : muons) {
        if (fabs(muon.Eta()) >= cuts.muon_eta) return false;
    }
    
    return true;
}

bool LRSM_TBChannel_notusingbjet::PassDileptonMassCut(const RVec<Muon>& muons) {
    if (muons.size() < 2) return false;
    
    float dilepton_mass = (muons[0] + muons[1]).M();
    return dilepton_mass > cuts.dilepton_mass_cut;
}

float LRSM_TBChannel_notusingbjet::CalculateWRMass(const RVec<Muon>& muons, const RVec<Jet>& bjets, const RVec<FatJet>& topjets) {
    if (muons.size() < 2 || bjets.size() < 1 || topjets.size() < 1) return -1.0;
    
    Particle wr_candidate = muons[0] + muons[1] + bjets[0] + topjets[0];
    return wr_candidate.M();
}

float LRSM_TBChannel_notusingbjet::CalculateTransverseMass(const Muon& lepton,  const Jet& b1, float met_pt, float met_phi) {
// Calculate transverse energies for all visible particles
float et_lepton = lepton.Et();
float et_b1 = b1.Et();
float et_met = met_pt; // MET is already transverse, assume massless

// Sum all transverse energies
float et_total = et_lepton + et_b1 + et_met;

// Calculate total transverse momentum vector
float px_total = lepton.Px() + b1.Px() + met_pt * cos(met_phi);
float py_total = lepton.Py() + b1.Py() + met_pt * sin(met_phi);

float pt_total = sqrt(px_total*px_total + py_total*py_total);

// Transverse mass: mt = sqrt(Et_total^2 - pt_total^2)
return sqrt(et_total*et_total - pt_total*pt_total);
}