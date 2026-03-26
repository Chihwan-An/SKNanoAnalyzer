#include "Skim_20002.h"    

Skim_20002::Skim_20002() {
    newtree = NULL; 
}

Skim_20002::~Skim_20002() {}

void Skim_20002::initializeAnalyzer() {
    GetOutfile()->cd();
    newtree = fChain->CloneTree(0);

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
        el_set.Ele_Trigger = {"HLT_Photon200","HLT_Ele115_CaloIdVT_GsfTrkIdT"};
        el_set.Ele_Trigger_Safe_Pt_Cut = 118.;  
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
    //if (IsDATA) {
    //    systHelper = std::make_unique<SystematicHelper>(SKNANO_HOME + "/docs/noSyst.yaml", DataStream, DataEra);
    //} else {
    //    systHelper = std::make_unique<SystematicHelper>(SKNANO_HOME + "/docs/ExampleSystematic.yaml", MCSample, DataEra);
    //}
    


}

void Skim_20002::executeEvent() {
    el_set.AllElectrons =  GetAllElectrons();
    mu_set.AllMuons = GetAllMuons();
    jet_set.AllJets = GetAllJets();
    fatjet_set.AllFatJets = GetAllFatJets();
    gen_set.gens = GetAllGens();

    std::unordered_map<std::string, std::variant<std::function<float(MyCorrection::variation, TString)>, std::function<float()>>> weight_function_map;
    
    executeEventFromParameter();
    //for (const auto &syst_dummy : *systHelper) {
    //    executeEventFromParameter();
    //}

}

void Skim_20002::executeEventFromParameter() {
    const TString this_syst = systHelper->getCurrentSysName();
    cout<<"Processing systematic variation: " << this_syst << endl;    
    newtree->Fill();

}

void Skim_20002::WriteHist() {
    GetOutfile()->cd();
    newtree->Write();

}


bool Skim_20002::Electrons::isPassCustomTightID(const Electron& el, const Skim_20002::Electrons& eset) const {
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

bool Skim_20002::Electrons::isPassCustomLooseID(const Electron& el) const {
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

bool Skim_20002::Electrons::isPassLooseNoIso(const Electron& el) const {
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

RVec<FatJet> Skim_20002::Clean_Fatjet_with_tight_leptons(const RVec<FatJet> & fatjets, const RVec<Lepton *> & tight_leps) {
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

RVec<Jet> Skim_20002::Clean_jet_with_loose_leptons(const RVec<Jet> & jets, const RVec<Lepton *> & loose_leps) {
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

RVec<Jet> Skim_20002::Clean_LSF_FatJet_with_jets(const RVec<FatJet> & fatjets, const RVec<Jet> & jets) {
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

RVec<FatJet> Skim_20002::Clean_Jets_with_fatjets(const RVec<Jet> & jets, const RVec<FatJet> & fatjets) {
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


