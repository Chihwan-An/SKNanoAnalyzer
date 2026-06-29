#include "HNWR_BDT_presel.h"

#include <algorithm>
#include <cmath>

HNWR_BDT_presel::HNWR_BDT_presel() {}
HNWR_BDT_presel::~HNWR_BDT_presel() {}

void HNWR_BDT_presel::initializeAnalyzer() {
    el_set.AllElectrons.clear();
    mu_set.AllMuons.clear();
    jet_set.AllJets.clear();
    fatjet_set.AllFatJets.clear();

    myCorr = new MyCorrection(DataEra, DataPeriod, IsDATA ? DataStream : MCSample, IsDATA);

    // Systematic helper: the event-loop iteration yields only Central + the
    // object (evtLoopAgain) variations, i.e. JER_Up/Down and JES_Up/Down for MC.
    // Data uses DataLRSM.yaml (no systematics) -> Central only.
    string SKNANO_HOME = getenv("SKNANO_HOME");
    if (IsDATA) {
        systHelper = std::make_unique<SystematicHelper>(SKNANO_HOME + "/docs/DataLRSM.yaml", DataStream, DataEra);
    } else {
        systHelper = std::make_unique<SystematicHelper>(SKNANO_HOME + "/docs/MCLRSM.yaml", MCSample, DataEra);
    }

    // One resolved + one boosted tree per variation (Central has no suffix).
    for (const auto &syst_dummy : *systHelper) {
        const TString sys = systHelper->getCurrentSysName();
        const TString suffix = (sys == "Central") ? TString("") : ("_" + sys);
        NewTree(TString("BDTTree_resolved") + suffix);
        NewTree(TString("BDTTree_boosted") + suffix);
    }

    mu_set.Muon_Trigger.clear();
    mu_set.Muon_Trigger_Safe_Pt_Cut = 0.;
    el_set.Ele_Trigger.clear();
    el_set.Ele_Trigger_Safe_Pt_Cut = 0.;

    if (DataEra == "2017") {
        mu_set.Muon_Trigger = {"HLT_Mu50", "HLT_OldMu100", "HLT_TkMu100"};
        mu_set.Muon_Trigger_Safe_Pt_Cut = 52.;
        el_set.Ele_Trigger = {"HLT_Ele35_WPTight_Gsf", "HLT_Photon200", "HLT_Ele115_CaloIdVT_GsfTrkIdT"};
        el_set.Ele_Trigger_Safe_Pt_Cut = 38.;
    }
    if (DataEra == "2022" || DataEra == "2022EE" || DataEra == "2023" || DataEra == "2023BPix") {
        mu_set.Muon_Trigger = {"HLT_Mu50", "HLT_CascadeMu100", "HLT_HighPtTkMu100"};
        mu_set.Muon_Trigger_Safe_Pt_Cut = 52.;
        el_set.Ele_Trigger = {"HLT_Photon200", "HLT_Ele115_CaloIdVT_GsfTrkIdT"};
        el_set.Ele_Trigger_Safe_Pt_Cut = 118.;
    }
}

void HNWR_BDT_presel::executeEvent() {
    el_set.AllElectrons   = GetAllElectrons();
    mu_set.AllMuons       = GetAllMuons();
    jet_set.AllJets       = GetAllJets();
    fatjet_set.AllFatJets = GetAllFatJets();

    // Re-run the selection once per object variation (Central + JES/JER up/down);
    // each variation writes into its own BDTTree_<category>[_<syst>] tree.
    for (const auto &syst_dummy : *systHelper) {
        executeEventFromParameter();
    }
}

void HNWR_BDT_presel::executeEventFromParameter() {
    const TString this_syst = systHelper->getCurrentSysName();
    const TString dir = this_syst;
    const TString syst_suffix = (this_syst == "Central") ? TString("") : ("_" + this_syst);

    Event ev = GetEvent();

    float weight = 1.0;
    if (!IsDATA) {
        weight *= MCweight();
        weight *= ev.GetTriggerLumi("HLT_Mu50");
    }

    // --- Cutflow bin 0: all events ---
    FillHist(dir + "/Cutflow", 0.0, weight, 10, 0., 10.);

    RVec<Electron> all_electrons = el_set.AllElectrons;
    RVec<Muon>     all_muons     = mu_set.AllMuons;
    RVec<Jet>      jets          = jet_set.AllJets;
    RVec<FatJet>   fatjets       = fatjet_set.AllFatJets;

    // --- JES/JER object variation (MC only): vary the AK4 jets so every
    // jet-derived BDT input (jet pT/mass, mjj, mlljj, ht, st, dR_lep_jet, ...)
    // is recomputed under the variation. MET is left at its nominal value.
    if (!IsDATA) {
        if (this_syst.Contains("JER_Up")) {
            RVec<GenJet> genjets = GetAllGenJets();
            jets = SmearJets(jets, genjets, MyCorrection::variation::up, "total");
        } else if (this_syst.Contains("JER_Down")) {
            RVec<GenJet> genjets = GetAllGenJets();
            jets = SmearJets(jets, genjets, MyCorrection::variation::down, "total");
        } else if (this_syst.Contains("JES_Up")) {
            jets = ScaleJets(jets, MyCorrection::variation::up, "total");
        } else if (this_syst.Contains("JES_Down")) {
            jets = ScaleJets(jets, MyCorrection::variation::down, "total");
        }
    }

    // --- Cutflow bin 1: noise filter ---
    if (!PassNoiseFilter(jets, ev, Event::MET_Type::PUPPI)) return;
    FillHist(dir + "/Cutflow", 1.0, weight, 10, 0., 10.);

    bool pass_trig_muon = ev.PassTrigger(mu_set.Muon_Trigger);
    bool pass_trig_elec = ev.PassTrigger(el_set.Ele_Trigger);
    if (!pass_trig_muon && !pass_trig_elec) return;
    // --- Cutflow bin 2: HLT fired ---
    FillHist(dir + "/Cutflow", 2.0, weight, 10, 0., 10.);


    ///         Leptons       ///
    RVec<Electron> my_electrons = SelectElectrons(all_electrons, "NOCUT", el_set.Electron_MinPt, 2.5);
    RVec<Muon>     my_muons     = SelectMuons(all_muons, "NOCUT", mu_set.Muon_MinPt, 2.4);

    sort(my_electrons.begin(), my_electrons.end(), PtComparing);
    sort(my_muons.begin(), my_muons.end(), PtComparing);

    RVec<Electron*> Loose_electrons, Tight_electrons;
    RVec<Muon*>     Loose_muons, Tight_muons;
    RVec<Lepton*>   Loose_leps, Tight_leps;

    for (unsigned int i = 0; i < my_electrons.size(); i++) {
        Electron& el = my_electrons.at(i);
        if (el.PassID(el_set.Electron_Tight_ID[0])) {
            Tight_electrons.push_back(&el);
            Tight_leps.push_back(&el);
        }

        const bool passLooseNoIso = el_set.isPassLooseNoIso(el);
        const bool passHEEP = el.PassID(Electron::ElectronID::POG_HEEP);
        if (passLooseNoIso || passHEEP) {
            Loose_electrons.push_back(&el);
            Loose_leps.push_back(&el);
        }
    }

    for (unsigned int i = 0; i < my_muons.size(); i++) {
        Muon& mu = my_muons.at(i);
        if (mu.PassID(mu_set.Muon_Tight_ID[0]) && mu.TkRelIso() < mu_set.Muon_Iso_Cut) {
            Tight_muons.push_back(&mu);
            Tight_leps.push_back(&mu);
        }
        if (mu.PassID(mu_set.Muon_Loose_ID[0])) {
            Loose_muons.push_back(&mu);
            Loose_leps.push_back(&mu);
        }
    }

    sort(Tight_leps.begin(), Tight_leps.end(), PtComparingPtr);
    sort(Loose_leps.begin(), Loose_leps.end(), PtComparingPtr);

    RVec<Electron*>& electrons = Tight_electrons;
    RVec<Muon*>&     muons     = Tight_muons;
    RVec<Lepton*>&   leps      = Tight_leps;

    ///         FatJets       ///
    fatjets = Clean_Fatjet_with_tight_leptons(fatjets, Tight_leps);
    jets    = Clean_jet_with_loose_leptons(jets, Loose_leps);

    RVec<FatJet> fatjet_list;
    for (unsigned int i = 0; i < fatjets.size(); i++) {
        FatJet& fj = fatjets.at(i);
        // FatJet preselection for boosted SR: pT/eta/ID only.
        // SDMass is saved as a BDT input variable and is not used as a cut here.
        if ((fj.Pt() > fatjet_set.FatJet_MinPt) && (abs(fj.Eta()) < fatjet_set.FatJet_MaxEta)) {
            if (fj.PassID(fatjet_set.FatJet_ID)) {
                fatjet_list.push_back(fj);
            }
        }
    }
    fatjets = fatjet_list;
    sort(fatjets.begin(), fatjets.end(), PtComparing);

    int n_leptons = electrons.size() + muons.size();
    const int n_loose_leptons = Loose_electrons.size() + Loose_muons.size();
    if (n_leptons < 1) return;
    // --- Cutflow bin 3: >=1 tight lepton ---
    FillHist(dir + "/Cutflow", 3.0, weight, 10, 0., 10.);

    Lepton* LeadLep    = nullptr;
    Lepton* SubLeadLep = nullptr;
    Lepton* ThirdLep   = nullptr;
    int channel = -999;
    bool this_trigger_pass = false;

    ///         Jets       ///
    bool is_jet_veto = AnalyzerCore::PassVetoMap(jets, mu_set.AllMuons, "jetvetomap");
    if (!is_jet_veto) return;
    // --- Cutflow bin 4: jet veto map ---
    FillHist(dir + "/Cutflow", 4.0, weight, 10, 0., 10.);

    RVec<Jet> selected_jets = SelectJets(jets, jet_set.Jet_ID[0], jet_set.Jet_MinPt, jet_set.Jet_MaxEta);
    sort(selected_jets.begin(), selected_jets.end(), PtComparing);

    // SR definitions used for BDT ntuples:
    //
    // Common object/event requirements:
    // - At least one tight lepton after electron/muon eta, pT, and ID selections.
    // - Event trigger must match the leading lepton flavor and trigger-safe pT.
    // - Jets are selected with TIGHTLEPVETO, pT > 40 GeV, and |eta| < 2.4.
    // - Fatjets are selected with pT > 200 GeV, |eta| < 2.4, and Tight ID.
    //
    // Resolved SR:
    // - Exactly two tight same-flavor leptons (EE or MM).
    // - Leading lepton pT > 60 GeV and subleading lepton pT > 53 GeV.
    // - mll > 200 GeV.
    // - At least two selected AK4 jets.
    // - lep1, lep2, jet1, and jet2 are mutually separated by dR > 0.4.
    // - No mlljj cut is applied.
    //
    // Boosted SR:
    // - At least one tight leading lepton with pT > 60 GeV.
    // - At least one additional loose lepton after excluding the leading tight lepton.
    // - The stored lep2 is the first pT-ordered extra loose lepton that forms
    //   a same-flavor pair with lep1 and satisfies mll > 200 GeV.
    // - At least one selected fatjet.
    // - If two AK4 jets exist, the resolved dR requirement must fail; if fewer
    //   than two AK4 jets exist, the event is allowed in boosted SR.
    // - No mlljj cut, loose-lepton-in-fatjet requirement, or OF loose veto is applied.
    //
    // Per-object lepton storage (both SRs), in two groups, no generic lepN branches:
    // - ele*/mu* = the two SIGNAL leptons (lead + sublead) by flavor. Resolved: both
    //   tight; boosted: tight lead + loose sublead. Same-flavor, so they fill ele1/2
    //   or mu1/2.
    // - extra_looseEle1/2, extra_looseMu1/2 = loose leptons that are NOT the two signal leptons,
    //   up to two per flavor (counts nExtraLooseElectron / nExtraLooseMuon). Resolved:
    //   "loose minus the tight pair"; boosted: "loose minus the two used leptons".
    // - SR selection and nElectron/nMuon counts remain on TIGHT leptons;
    //   nLooseElectron/nLooseMuon give the loose multiplicity.

    // Filled below once the resolved/boosted category is known.
    TString tree = TString("BDTTree_boosted") + syst_suffix;
    const float fMissing = -999.f;
    const int iMissing = -999;
    Particle METv = ev.GetMETVector(Event::MET_Type::PUPPI, Event::MET_Syst::CENTRAL);

    auto fillParticle = [&](const TString& prefix, const Particle* obj) {
        SetBranch(tree, prefix + "_pt", obj ? obj->Pt() : fMissing);
        SetBranch(tree, prefix + "_eta", obj ? obj->Eta() : fMissing);
        SetBranch(tree, prefix + "_phi", obj ? obj->Phi() : fMissing);
        SetBranch(tree, prefix + "_mass", obj ? obj->M() : fMissing);
        SetBranch(tree, prefix + "_charge", obj ? obj->Charge() : fMissing);
    };

    auto fillElectron = [&](const TString& prefix, const Electron* el) {
        fillParticle(prefix, el);
        SetBranch(tree, prefix + "_isFilled", el != nullptr);
        SetBranch(tree, prefix + "_dxy", el ? el->dXY() : fMissing);
        SetBranch(tree, prefix + "_dxyErr", el ? el->dXYerr() : fMissing);
        SetBranch(tree, prefix + "_dz", el ? el->dZ() : fMissing);
        SetBranch(tree, prefix + "_dzErr", el ? el->dZerr() : fMissing);
        SetBranch(tree, prefix + "_ip3d", el ? el->IP3D() : fMissing);
        SetBranch(tree, prefix + "_sip3d", el ? el->SIP3D() : fMissing);
        SetBranch(tree, prefix + "_pfRelIso03", el ? el->PfRelIso03() : fMissing);
        SetBranch(tree, prefix + "_miniPFRelIso", el ? el->MiniPFRelIso() : fMissing);
        SetBranch(tree, prefix + "_scEta", el ? el->scEta() : fMissing);
        SetBranch(tree, prefix + "_deltaEtaInSC", el ? el->deltaEtaInSC() : fMissing);
        SetBranch(tree, prefix + "_deltaEtaInSeed", el ? el->deltaEtaInSeed() : fMissing);
        SetBranch(tree, prefix + "_deltaPhiInSC", el ? el->deltaPhiInSC() : fMissing);
        SetBranch(tree, prefix + "_deltaPhiInSeed", el ? el->deltaPhiInSeed() : fMissing);
        SetBranch(tree, prefix + "_ecalPFClusterIso", el ? el->ecalPFClusterIso() : fMissing);
        SetBranch(tree, prefix + "_hcalPFClusterIso", el ? el->hcalPFClusterIso() : fMissing);
        SetBranch(tree, prefix + "_dr03EcalRecHitSumEt", el ? el->dr03EcalRecHitSumEt() : fMissing);
        SetBranch(tree, prefix + "_dr03HcalDepth1TowerSumEt", el ? el->dr03HcalDepth1TowerSumEt() : fMissing);
        SetBranch(tree, prefix + "_dr03TkSumPt", el ? el->dr03TkSumPt() : fMissing);
        SetBranch(tree, prefix + "_dr03TkSumPtHEEP", el ? el->dr03TkSumPtHEEP() : fMissing);
        SetBranch(tree, prefix + "_hoe", el ? el->hoe() : fMissing);
        SetBranch(tree, prefix + "_sieie", el ? el->sieie() : fMissing);
        SetBranch(tree, prefix + "_eInvMinusPInv", el ? el->eInvMinusPInv() : fMissing);
        SetBranch(tree, prefix + "_energyErr", el ? el->energyErr() : fMissing);
        SetBranch(tree, prefix + "_r9", el ? el->r9() : fMissing);
        SetBranch(tree, prefix + "_rho", el ? el->rho() : fMissing);
        SetBranch(tree, prefix + "_scEtOverPt", el ? el->scEtOverPt() : fMissing);
        SetBranch(tree, prefix + "_convVeto", el ? el->ConvVeto() : false);
        SetBranch(tree, prefix + "_cutBased", el ? static_cast<int>(el->CutBased()) : iMissing);
        SetBranch(tree, prefix + "_cutBasedHEEP", el ? el->isCutBasedHEEP() : false);
        SetBranch(tree, prefix + "_lostHits", el ? static_cast<int>(el->LostHits()) : iMissing);
        SetBranch(tree, prefix + "_seedGain", el ? static_cast<int>(el->SeedGain()) : iMissing);
        SetBranch(tree, prefix + "_tightCharge", el ? static_cast<int>(el->TightCharge()) : iMissing);
        SetBranch(tree, prefix + "_jetIdx", el ? static_cast<int>(el->JetIdx()) : iMissing);
        SetBranch(tree, prefix + "_vidNestedWPBitmap", el ? static_cast<int>(el->VidNestedWPBitmap()) : iMissing);
        SetBranch(tree, prefix + "_vidNestedWPBitmapHEEP", el ? static_cast<int>(el->VidNestedWPBitmapHEEP()) : iMissing);
        SetBranch(tree, prefix + "_mvaIso", el ? el->MvaIso() : fMissing);
        SetBranch(tree, prefix + "_mvaNoIso", el ? el->MvaNoIso() : fMissing);
        SetBranch(tree, prefix + "_mvaTTH", el ? el->MvaTTH() : fMissing);
        SetBranch(tree, prefix + "_mvaIsoWP80", el ? el->isMVAIsoWP80() : false);
        SetBranch(tree, prefix + "_mvaIsoWP90", el ? el->isMVAIsoWP90() : false);
        SetBranch(tree, prefix + "_mvaIsoWPLoose", el ? el->isMVAIsoWPLoose() : false);
        SetBranch(tree, prefix + "_mvaNoIsoWP80", el ? el->isMVANoIsoWP80() : false);
        SetBranch(tree, prefix + "_mvaNoIsoWP90", el ? el->isMVANoIsoWP90() : false);
        SetBranch(tree, prefix + "_mvaNoIsoWPLoose", el ? el->isMVANoIsoWPLoose() : false);
        SetBranch(tree, prefix + "_passLooseID", el ? el->PassID(el_set.Electron_Loose_ID[0]) : false);
        SetBranch(tree, prefix + "_passHEEP", el ? el->PassID(Electron::ElectronID::POG_HEEP) : false);
        SetBranch(tree, prefix + "_genPartFlav_truth", el ? static_cast<int>(el->GenPartFlav()) : iMissing);
        SetBranch(tree, prefix + "_genPartIdx_truth", el ? static_cast<int>(el->GenPartIdx()) : iMissing);
    };

    auto fillMuon = [&](const TString& prefix, const Muon* mu) {
        fillParticle(prefix, mu);
        SetBranch(tree, prefix + "_isFilled", mu != nullptr);
        SetBranch(tree, prefix + "_dxy", mu ? mu->dXY() : fMissing);
        SetBranch(tree, prefix + "_dxyErr", mu ? mu->dXYerr() : fMissing);
        SetBranch(tree, prefix + "_dz", mu ? mu->dZ() : fMissing);
        SetBranch(tree, prefix + "_dzErr", mu ? mu->dZerr() : fMissing);
        SetBranch(tree, prefix + "_ip3d", mu ? mu->IP3D() : fMissing);
        SetBranch(tree, prefix + "_sip3d", mu ? mu->SIP3D() : fMissing);
        SetBranch(tree, prefix + "_pfRelIso03", mu ? mu->PfRelIso03() : fMissing);
        SetBranch(tree, prefix + "_pfRelIso04", mu ? mu->PfRelIso04() : fMissing);
        SetBranch(tree, prefix + "_miniPFRelIso", mu ? mu->MiniPFRelIso() : fMissing);
        SetBranch(tree, prefix + "_tkRelIso", mu ? mu->TkRelIso() : fMissing);
        SetBranch(tree, prefix + "_isTracker", mu ? mu->isTracker() : false);
        SetBranch(tree, prefix + "_isGlobal", mu ? mu->isGlobal() : false);
        SetBranch(tree, prefix + "_isStandalone", mu ? mu->isStandalone() : false);
        SetBranch(tree, prefix + "_passLooseID", mu ? mu->PassID(mu_set.Muon_Loose_ID[0]) : false);
        SetBranch(tree, prefix + "_passTightID", mu ? mu->PassID(mu_set.Muon_Tight_ID[0]) : false);
        SetBranch(tree, prefix + "_tightCharge", mu ? static_cast<int>(mu->TightCharge()) : iMissing);
        SetBranch(tree, prefix + "_nTrackerLayers", mu ? mu->nTrackerLayers() : iMissing);
        SetBranch(tree, prefix + "_highPtId", mu ? static_cast<int>(mu->HighPtId()) : iMissing);
        SetBranch(tree, prefix + "_miniIsoId", mu ? static_cast<int>(mu->MiniIsoId()) : iMissing);
        SetBranch(tree, prefix + "_multiIsoId", mu ? static_cast<int>(mu->MultiIsoId()) : iMissing);
        SetBranch(tree, prefix + "_mvaMuId", mu ? static_cast<int>(mu->MvaMuId()) : iMissing);
        SetBranch(tree, prefix + "_pfIsoId", mu ? static_cast<int>(mu->PfIsoId()) : iMissing);
        SetBranch(tree, prefix + "_puppiIsoId", mu ? static_cast<int>(mu->PuppiIsoId()) : iMissing);
        SetBranch(tree, prefix + "_tkIsoId", mu ? static_cast<int>(mu->TkIsoId()) : iMissing);
        SetBranch(tree, prefix + "_softMva", mu ? mu->SoftMva() : fMissing);
        SetBranch(tree, prefix + "_mvaLowPt", mu ? mu->MvaLowPt() : fMissing);
        SetBranch(tree, prefix + "_mvaTTH", mu ? mu->MvaTTH() : fMissing);
        SetBranch(tree, prefix + "_jetIdx", mu ? static_cast<int>(mu->JetIdx()) : iMissing);
        SetBranch(tree, prefix + "_genPartFlav_truth", mu ? static_cast<int>(mu->GenPartFlav()) : iMissing);
        SetBranch(tree, prefix + "_genPartIdx_truth", mu ? static_cast<int>(mu->GenPartIdx()) : iMissing);
    };

    auto fillJet = [&](const TString& prefix, const Jet* jet) {
        fillParticle(prefix, jet);
        SetBranch(tree, prefix + "_isFilled", jet != nullptr);
        SetBranch(tree, prefix + "_rawPt", jet ? jet->GetRawPt() : fMissing);
        SetBranch(tree, prefix + "_originalPt", jet ? jet->GetOriginalPt() : fMissing);
        SetBranch(tree, prefix + "_deepJetB", jet ? jet->GetBTaggerResult(JetTagging::JetFlavTagger::DeepJet) : fMissing);
        SetBranch(tree, prefix + "_pnetB", jet ? jet->GetBTaggerResult(JetTagging::JetFlavTagger::ParticleNet) : fMissing);
        SetBranch(tree, prefix + "_partB", jet ? jet->GetBTaggerResult(JetTagging::JetFlavTagger::ParT) : fMissing);
        SetBranch(tree, prefix + "_deepJetQG", jet ? jet->GetQvGTaggerResult(JetTagging::JetFlavTagger::DeepJet) : fMissing);
        SetBranch(tree, prefix + "_pnetQG", jet ? jet->GetQvGTaggerResult(JetTagging::JetFlavTagger::ParticleNet) : fMissing);
        SetBranch(tree, prefix + "_partQG", jet ? jet->GetQvGTaggerResult(JetTagging::JetFlavTagger::ParT) : fMissing);
        SetBranch(tree, prefix + "_nConstituents", jet ? static_cast<int>(jet->nConstituents()) : iMissing);
        SetBranch(tree, prefix + "_nElectrons", jet ? static_cast<int>(jet->nElectrons()) : iMissing);
        SetBranch(tree, prefix + "_nMuons", jet ? static_cast<int>(jet->nMuons()) : iMissing);
        SetBranch(tree, prefix + "_nSVs", jet ? static_cast<int>(jet->nSVs()) : iMissing);
        SetBranch(tree, prefix + "_chHEF", jet ? jet->chHEF() : fMissing);
        SetBranch(tree, prefix + "_neHEF", jet ? jet->neHEF() : fMissing);
        SetBranch(tree, prefix + "_chEmEF", jet ? jet->chEmEF() : fMissing);
        SetBranch(tree, prefix + "_neEmEF", jet ? jet->neEmEF() : fMissing);
        SetBranch(tree, prefix + "_muEF", jet ? jet->muEF() : fMissing);
    };

    auto fillFatJet = [&](const TString& prefix, const FatJet* fatjet) {
        fillParticle(prefix, fatjet);
        SetBranch(tree, prefix + "_isFilled", fatjet != nullptr);
        SetBranch(tree, prefix + "_rawPt", fatjet ? fatjet->GetRawPt() : fMissing);
        SetBranch(tree, prefix + "_originalPt", fatjet ? fatjet->GetOriginalPt() : fMissing);
        SetBranch(tree, prefix + "_sdMass", fatjet ? fatjet->SDMass() : fMissing);
        SetBranch(tree, prefix + "_lsf3", fatjet ? fatjet->LSF3() : fMissing);
        SetBranch(tree, prefix + "_area", fatjet ? fatjet->Area() : fMissing);
        SetBranch(tree, prefix + "_nConstituents", fatjet ? static_cast<int>(fatjet->NConstituents()) : iMissing);
        SetBranch(tree, prefix + "_nBHadrons", fatjet ? static_cast<int>(fatjet->NBHadrons()) : iMissing);
        SetBranch(tree, prefix + "_nCHadrons", fatjet ? static_cast<int>(fatjet->NCHadrons()) : iMissing);
        SetBranch(tree, prefix + "_pnetWithMassTvsQCD", fatjet ? fatjet->GetTaggerResult(JetTagging::FatJetTaggingtype::ParticleNetWithMass, JetTagging::FatjetTaggingObject::TvsQCD) : fMissing);
        SetBranch(tree, prefix + "_pnetWithMassWvsQCD", fatjet ? fatjet->GetTaggerResult(JetTagging::FatJetTaggingtype::ParticleNetWithMass, JetTagging::FatjetTaggingObject::WvsQCD) : fMissing);
        SetBranch(tree, prefix + "_pnetWithMassHbbvsQCD", fatjet ? fatjet->GetTaggerResult(JetTagging::FatJetTaggingtype::ParticleNetWithMass, JetTagging::FatjetTaggingObject::HbbvsQCD) : fMissing);
        SetBranch(tree, prefix + "_pnetXbbVsQCD", fatjet ? fatjet->GetTaggerResult(JetTagging::FatJetTaggingtype::ParticleNet, JetTagging::FatjetTaggingObject::XbbVsQCD) : fMissing);
        SetBranch(tree, prefix + "_pnetXqqVsQCD", fatjet ? fatjet->GetTaggerResult(JetTagging::FatJetTaggingtype::ParticleNet, JetTagging::FatjetTaggingObject::XqqVsQCD) : fMissing);
        SetBranch(tree, prefix + "_pnetQCD", fatjet ? fatjet->GetTaggerResult(JetTagging::FatJetTaggingtype::ParticleNet, JetTagging::FatjetTaggingObject::QCD) : fMissing);
    };

    auto isSameFlavorPair = [](const Lepton* lep1, const Lepton* lep2) {
        if (!lep1 || !lep2) return false;
        return (lep1->IsElectron() && lep2->IsElectron()) || (lep1->IsMuon() && lep2->IsMuon());
    };
    auto makeSameFlavorPair = [&](const Lepton* lep1, const Lepton* lep2, TLorentzVector& pair) {
        if (!isSameFlavorPair(lep1, lep2)) return false;
        pair = *lep1 + *lep2;
        return true;
    };
    auto passLeadTrigger = [&](const Lepton* lep, int& outChannel) {
        outChannel = -999;
        if (!lep) return false;
        if (lep->IsElectron()) {
            if (lep->Pt() < el_set.Ele_Trigger_Safe_Pt_Cut) return false;
            outChannel = 0;
            return pass_trig_elec;
        }
        if (lep->IsMuon()) {
            if (lep->Pt() < mu_set.Muon_Trigger_Safe_Pt_Cut) return false;
            outChannel = 1;
            return pass_trig_muon;
        }
        return false;
    };

    const bool has2Jets = selected_jets.size() >= 2;
    auto computeFourObjectDR = [&](const Lepton* lep1, const Lepton* lep2,
                                   bool& outLeadJetLep, bool& outSubLeadJetLep,
                                   bool& outTwoLeptons, bool& outTwoJets) {
        outLeadJetLep = false;
        outSubLeadJetLep = false;
        outTwoLeptons = false;
        outTwoJets = false;
        if (!lep1 || !lep2 || !has2Jets) return false;
        outLeadJetLep = (selected_jets[0].DeltaR(*lep1) > dR_Separation) && (selected_jets[0].DeltaR(*lep2) > dR_Separation);
        outSubLeadJetLep = (selected_jets[1].DeltaR(*lep1) > dR_Separation) && (selected_jets[1].DeltaR(*lep2) > dR_Separation);
        outTwoLeptons = (lep1->DeltaR(*lep2) > dR_Separation);
        outTwoJets = (selected_jets[0].DeltaR(selected_jets[1]) > dR_Separation);
        return outLeadJetLep && outSubLeadJetLep && outTwoLeptons && outTwoJets;
    };

    Lepton* resolvedLead = nullptr;
    Lepton* resolvedSubLead = nullptr;
    bool isResolvedLeptonSelection = false;
    int resolvedChannel = -999;
    bool passResolvedTrigger = false;
    if ((n_leptons == 2) && ((electrons.size() == 2 && muons.size() == 0) || (muons.size() == 2 && electrons.size() == 0))) {
        resolvedLead = Tight_leps[0];
        resolvedSubLead = Tight_leps[1];
        isResolvedLeptonSelection = (resolvedLead->Pt() > 60.0) && (resolvedSubLead->Pt() > 53.0);
        passResolvedTrigger = isResolvedLeptonSelection && passLeadTrigger(resolvedLead, resolvedChannel);
    }

    TLorentzVector resolvedDilepton;
    const bool passResolvedMll = passResolvedTrigger && makeSameFlavorPair(resolvedLead, resolvedSubLead, resolvedDilepton) && (resolvedDilepton.M() > 200.0);
    bool resolvedDRLeadJetLep = false;
    bool resolvedDRSubLeadJetLep = false;
    bool resolvedDRTwoLeptons = false;
    bool resolvedDRTwoJets = false;
    const bool passResolvedDRCandidate = computeFourObjectDR(resolvedLead, resolvedSubLead, resolvedDRLeadJetLep, resolvedDRSubLeadJetLep, resolvedDRTwoLeptons, resolvedDRTwoJets);
    bool isResolvedCandidate = passResolvedMll && passResolvedDRCandidate;

    Lepton* boostedLead = Tight_leps.empty() ? nullptr : Tight_leps[0];
    RVec<Lepton*> boostedExtraLooseLeps;
    if (boostedLead) {
        for (unsigned int i = 0; i < Loose_leps.size(); i++) {
            if (Loose_leps[i] == boostedLead) continue;
            boostedExtraLooseLeps.push_back(Loose_leps[i]);
        }
    }
    sort(boostedExtraLooseLeps.begin(), boostedExtraLooseLeps.end(), PtComparingPtr);
    const int n_boosted_extra_loose_leptons = boostedExtraLooseLeps.size();
    Lepton* boostedSubLead = nullptr;
    TLorentzVector boostedDilepton;
    bool hasBoostedDilepton = false;
    if (boostedLead) {
        for (unsigned int i = 0; i < boostedExtraLooseLeps.size(); i++) {
            TLorentzVector candidateDilepton;
            if (!makeSameFlavorPair(boostedLead, boostedExtraLooseLeps[i], candidateDilepton)) continue;
            if (candidateDilepton.M() <= 200.0) continue;
            boostedSubLead = boostedExtraLooseLeps[i];
            boostedDilepton = candidateDilepton;
            hasBoostedDilepton = true;
            break;
        }
    }

    int boostedChannel = -999;
    const bool passBoostedLeptonSelection = boostedLead && (boostedLead->Pt() > 60.0) && (n_boosted_extra_loose_leptons >= 1);
    const bool passBoostedTrigger = passBoostedLeptonSelection && passLeadTrigger(boostedLead, boostedChannel);
    const bool passBoostedMll = passBoostedTrigger && hasBoostedDilepton;

    bool boostedDRLeadJetLep = false;
    bool boostedDRSubLeadJetLep = false;
    bool boostedDRTwoLeptons = false;
    bool boostedDRTwoJets = false;
    const bool passBoostedResolvedDR = computeFourObjectDR(boostedLead, boostedSubLead, boostedDRLeadJetLep, boostedDRSubLeadJetLep, boostedDRTwoLeptons, boostedDRTwoJets);
    const bool passBoostedDRFail = !has2Jets || !passBoostedResolvedDR;
    bool isBoostedCandidate = (!isResolvedCandidate && passBoostedMll && passBoostedDRFail && fatjets.size() >= 1);
    if (!(isResolvedCandidate || isBoostedCandidate)) return;

    LeadLep = isResolvedCandidate ? resolvedLead : boostedLead;
    SubLeadLep = isResolvedCandidate ? resolvedSubLead : boostedSubLead;
    channel = isResolvedCandidate ? resolvedChannel : boostedChannel;
    this_trigger_pass = isResolvedCandidate ? passResolvedTrigger : passBoostedTrigger;
    if (!LeadLep || !SubLeadLep || !this_trigger_pass) return;

    tree = TString(isResolvedCandidate ? "BDTTree_resolved" : "BDTTree_boosted") + syst_suffix;
    // --- Cutflow bin 5: resolved || boosted candidate ---
    FillHist(dir + "/Cutflow", 5.0, weight, 10, 0., 10.);
    // --- Cutflow bin 6: resolved / bin 7: boosted ---
    if (isResolvedCandidate) FillHist(dir + "/Cutflow", 6.0, weight, 10, 0., 10.);
    if (isBoostedCandidate)  FillHist(dir + "/Cutflow", 7.0, weight, 10, 0., 10.);
    FillHist(dir + "/LeadFlavorChannel", channel, weight, 2, 0., 2.);

    // --- Per-object lepton storage, split into two groups (both categories) ------
    // (1) Signal leptons: the two SELECTED leptons (LeadLep, SubLeadLep) stored by
    //     flavor as ele*/mu* (no generic lepN). They are same-flavor by construction,
    //     so they occupy ele1/ele2 or mu1/mu2. For resolved both are tight; for
    //     boosted the lead is tight and the sublead is loose -> this is the only
    //     per-object slot that captures the boosted loose sublead.
    // (2) Extra loose leptons: every loose lepton that is NOT one of the two signal
    //     leptons, stored per flavor up to two each as extra_looseEle1/2, extra_looseMu1/2.
    //     (In resolved the two signal leptons are exactly the tight pair, so this is
    //     "loose minus tight"; in boosted it is "loose minus the two used leptons".)
    RVec<Electron*> Signal_electrons;
    RVec<Muon*>     Signal_muons;
    auto addSignalLepton = [&](Lepton* lep) {
        if (!lep) return;
        if (lep->IsElectron()) Signal_electrons.push_back(static_cast<Electron*>(lep));
        else                   Signal_muons.push_back(static_cast<Muon*>(lep));
    };
    addSignalLepton(LeadLep);
    addSignalLepton(SubLeadLep);

    auto isSignalLepton = [&](const Lepton* lep) {
        return lep && (lep == LeadLep || lep == SubLeadLep);
    };
    RVec<Electron*> ExtraLoose_electrons;
    for (auto* el : Loose_electrons) {
        if (isSignalLepton(el)) continue;
        ExtraLoose_electrons.push_back(el);
    }
    RVec<Muon*> ExtraLoose_muons;
    for (auto* mu : Loose_muons) {
        if (isSignalLepton(mu)) continue;
        ExtraLoose_muons.push_back(mu);
    }
    const int n_extra_loose_electrons = static_cast<int>(ExtraLoose_electrons.size());
    const int n_extra_loose_muons     = static_cast<int>(ExtraLoose_muons.size());

    // (3) Very-loose extra leptons (VL_extra_loose*): built from the RAW lepton
    //     collections with their OWN pt > 20 GeV cut (NOT the signal MinPt cut), kept per
    //     flavor up to two each. These are leptons that FAIL this analysis' loose ID but
    //     still pass a very-loose POG selection:
    //       - Electron: cutBased == LOOSE (==2 exactly), |eta| < 2.5
    //       - Muon: POG looseId == true, |eta| < 2.4
    //     By requiring failure of the analysis loose ID, signal and extra_loose leptons
    //     are excluded automatically (they pass the loose ID).
    RVec<Electron*> VL_extra_electrons;
    for (auto& el : all_electrons) {
        if (el.Pt() <= 20.) continue;
        if (std::fabs(el.Eta()) >= 2.5) continue;
        if (static_cast<int>(el.CutBased()) != 2) continue;
        const bool passAnalysisLoose = el_set.isPassLooseNoIso(el) || el.PassID(Electron::ElectronID::POG_HEEP);
        if (passAnalysisLoose) continue;
        VL_extra_electrons.push_back(&el);
    }
    sort(VL_extra_electrons.begin(), VL_extra_electrons.end(), PtComparingPtr);
    RVec<Muon*> VL_extra_muons;
    for (auto& mu : all_muons) {
        if (mu.Pt() <= 20.) continue;
        if (std::fabs(mu.Eta()) >= 2.4) continue;
        if (!mu.isPOGLooseId()) continue;
        if (mu.PassID(mu_set.Muon_Loose_ID[0])) continue;
        VL_extra_muons.push_back(&mu);
    }
    sort(VL_extra_muons.begin(), VL_extra_muons.end(), PtComparingPtr);
    const int n_vl_extra_electrons = static_cast<int>(VL_extra_electrons.size());
    const int n_vl_extra_muons     = static_cast<int>(VL_extra_muons.size());

    TLorentzVector jj;
    TLorentzVector l1jj;
    TLorentzVector l2jj;
    TLorentzVector l3jj;
    if (has2Jets) {
        jj = selected_jets[0] + selected_jets[1];
        l1jj = *LeadLep + jj;
        l2jj = *SubLeadLep + jj;
        if (ThirdLep) l3jj = *ThirdLep + jj;
    }

    const bool passMllSR = isResolvedCandidate ? passResolvedMll : passBoostedMll;
    bool dRLeadJetLep = false;
    bool dRSubLeadJetLep = false;
    bool dRTwoLeptons = false;
    bool dRTwoJets = false;
    const bool passResolvedDR = computeFourObjectDR(LeadLep, SubLeadLep, dRLeadJetLep, dRSubLeadJetLep, dRTwoLeptons, dRTwoJets);

    float ht = 0.f;
    for (const auto& jet : selected_jets) ht += jet.Pt();
    float lt = LeadLep->Pt() + SubLeadLep->Pt();
    float st = ht + lt;
    const float metPt = METv.Pt();
    const float metPhi = METv.Phi();
    auto transverseMass = [](float pt1, float pt2, float dphi) {
        const double mt2 = 2.0 * pt1 * pt2 * (1.0 - std::cos(dphi));
        return static_cast<float>(std::sqrt(std::max(0.0, mt2)));
    };
    auto fillSameFlavorPair = [&](const TString& pairName, const Lepton* lep1, const Lepton* lep2) {
        TLorentzVector pair;
        const bool hasPair = makeSameFlavorPair(lep1, lep2, pair);
        const TLorentzVector pairjj = hasPair ? pair + jj : TLorentzVector();

        SetBranch(tree, TString("m_") + pairName, hasPair ? static_cast<float>(pair.M()) : fMissing);
        SetBranch(tree, TString("pt_") + pairName, hasPair ? static_cast<float>(pair.Pt()) : fMissing);
        SetBranch(tree, TString("eta_") + pairName, hasPair ? static_cast<float>(pair.Eta()) : fMissing);
        SetBranch(tree, TString("phi_") + pairName, hasPair ? static_cast<float>(pair.Phi()) : fMissing);
        SetBranch(tree, TString("dR_") + pairName, hasPair ? static_cast<float>(lep1->DeltaR(*lep2)) : fMissing);
        SetBranch(tree, TString("dPhi_") + pairName, hasPair ? static_cast<float>(std::fabs(lep1->DeltaPhi(*lep2))) : fMissing);
        SetBranch(tree, TString("chargeProduct_") + pairName, hasPair ? lep1->Charge() * lep2->Charge() : iMissing);
        SetBranch(tree, TString("m_") + pairName + "jj", (hasPair && has2Jets) ? static_cast<float>(pairjj.M()) : fMissing);
        SetBranch(tree, TString("pt_") + pairName + "jj", (hasPair && has2Jets) ? static_cast<float>(pairjj.Pt()) : fMissing);
        SetBranch(tree, TString("dPhi_met_") + pairName, hasPair ? static_cast<float>(std::fabs(METv.DeltaPhi(pair))) : fMissing);
        SetBranch(tree, TString("met_mt_") + pairName, hasPair ? transverseMass(metPt, pair.Pt(), METv.DeltaPhi(pair)) : fMissing);
    };
    auto fillLeptonJetBranches = [&](const TString& lepName, const Lepton* lep) {
        SetBranch(tree, TString("dR_") + lepName + "_j1", (lep && selected_jets.size() >= 1) ? static_cast<float>(lep->DeltaR(selected_jets[0])) : fMissing);
        SetBranch(tree, TString("dR_") + lepName + "_j2", (lep && has2Jets) ? static_cast<float>(lep->DeltaR(selected_jets[1])) : fMissing);
    };
    auto fillLeptonFatJetBranches = [&](const TString& lepName, const Lepton* lep) {
        SetBranch(tree, TString("dR_") + lepName + "_fatjet1", (lep && fatjets.size() >= 1) ? static_cast<float>(lep->DeltaR(fatjets[0])) : fMissing);
        SetBranch(tree, TString("dPhi_") + lepName + "_fatjet1", (lep && fatjets.size() >= 1) ? static_cast<float>(std::fabs(lep->DeltaPhi(fatjets[0]))) : fMissing);
    };
    auto fillLeptonMETBranches = [&](const TString& lepName, const Lepton* lep) {
        SetBranch(tree, TString("dPhi_met_") + lepName, lep ? static_cast<float>(std::fabs(METv.DeltaPhi(*lep))) : fMissing);
        SetBranch(tree, TString("met_mt_") + lepName, lep ? transverseMass(metPt, lep->Pt(), METv.DeltaPhi(*lep)) : fMissing);
    };

    JetTagging::JetFlavTagger btagger = (Run == 2) ? JetTagging::JetFlavTagger::DeepJet : JetTagging::JetFlavTagger::ParticleNet;
    float btagWP = GetBTaggingWP(btagger, JetTagging::JetFlavTaggerWP::Medium);
    int nBJetMedium = 0;
    float maxBTag = fMissing;
    for (const auto& jet : selected_jets) {
        float score = jet.GetBTaggerResult(btagger);
        if (maxBTag == fMissing || score > maxBTag) maxBTag = score;
        if (score > btagWP) nBJetMedium++;
    }

    SetBranch(tree, "passTrigMuon", pass_trig_muon);
    SetBranch(tree, "passTrigElectron", pass_trig_elec);
    SetBranch(tree, "nElectronRaw", static_cast<int>(nElectron));
    SetBranch(tree, "nMuonRaw", static_cast<int>(nMuon));
    SetBranch(tree, "nElectron", static_cast<int>(electrons.size()));
    SetBranch(tree, "nMuon", static_cast<int>(muons.size()));
    SetBranch(tree, "nLepton", n_leptons);
    SetBranch(tree, "nLooseElectron", static_cast<int>(Loose_electrons.size()));
    SetBranch(tree, "nLooseMuon", static_cast<int>(Loose_muons.size()));
    SetBranch(tree, "nLooseLepton", n_loose_leptons);
    SetBranch(tree, "nBoostedExtraLooseLepton", n_boosted_extra_loose_leptons);
    SetBranch(tree, "nExtraLooseElectron", n_extra_loose_electrons);
    SetBranch(tree, "nExtraLooseMuon", n_extra_loose_muons);
    SetBranch(tree, "nVLExtraLooseElectron", n_vl_extra_electrons);
    SetBranch(tree, "nVLExtraLooseMuon", n_vl_extra_muons);
    SetBranch(tree, "nTightElectron", static_cast<int>(Tight_electrons.size()));
    SetBranch(tree, "nTightMuon", static_cast<int>(Tight_muons.size()));
    SetBranch(tree, "nTightLepton", n_leptons);
    SetBranch(tree, "nJet", static_cast<int>(selected_jets.size()));
    SetBranch(tree, "nFatJet", static_cast<int>(fatjets.size()));
    SetBranch(tree, "nBJetMedium", nBJetMedium);
    SetBranch(tree, "maxBTag", maxBTag);
    SetBranch(tree, "channel", channel);
    SetBranch(tree, "weight", weight);
    SetBranch(tree, "passMllSR", passMllSR);
    SetBranch(tree, "passResolvedDR", passResolvedDR);
    SetBranch(tree, "passBoostedDRFail", passBoostedDRFail);
    SetBranch(tree, "passDRLeadJetLep", dRLeadJetLep);
    SetBranch(tree, "passDRSubLeadJetLep", dRSubLeadJetLep);
    SetBranch(tree, "passDRTwoLeptons", dRTwoLeptons);
    SetBranch(tree, "passDRTwoJets", dRTwoJets);
    SetBranch(tree, "isResolvedCandidate", isResolvedCandidate);
    SetBranch(tree, "isBoostedCandidate", isBoostedCandidate);
    SetBranch(tree, "category", isResolvedCandidate ? 0 : 1);  // 0 = resolved, 1 = boosted

    // ele*/mu* = the two SIGNAL leptons (LeadLep, SubLeadLep) by flavor; jets in the
    // same loop. The signal pair is same-flavor, so only one flavor's slots fill.
    // Signal leptons: same-flavor pair, so at most two slots ever fill.
    for (int i = 0; i < 2; i++) {
        fillElectron(Form("ele%d", i + 1), i < static_cast<int>(Signal_electrons.size()) ? Signal_electrons[i] : nullptr);
        fillMuon(Form("mu%d", i + 1), i < static_cast<int>(Signal_muons.size()) ? Signal_muons[i] : nullptr);
    }
    for (int i = 0; i < 4; i++) {
        fillJet(Form("jet%d", i + 1), i < static_cast<int>(selected_jets.size()) ? &selected_jets[i] : nullptr);
    }
    for (int i = 0; i < 2; i++) {
        fillFatJet(Form("fatjet%d", i + 1), i < static_cast<int>(fatjets.size()) ? &fatjets[i] : nullptr);
    }
    // Up to two extra loose leptons per flavor (signal pair excluded; see above).
    for (int i = 0; i < 2; i++) {
        fillElectron(Form("extra_looseEle%d", i + 1), i < n_extra_loose_electrons ? ExtraLoose_electrons[i] : nullptr);
        fillMuon(Form("extra_looseMu%d", i + 1), i < n_extra_loose_muons ? ExtraLoose_muons[i] : nullptr);
    }
    // Up to two very-loose extra leptons per flavor (pt > 20, fail analysis loose; see above).
    for (int i = 0; i < 2; i++) {
        fillElectron(Form("VL_extra_looseEle%d", i + 1), i < n_vl_extra_electrons ? VL_extra_electrons[i] : nullptr);
        fillMuon(Form("VL_extra_looseMu%d", i + 1), i < n_vl_extra_muons ? VL_extra_muons[i] : nullptr);
    }

    TLorentzVector sfL1L2;
    const bool hasSFL1L2 = makeSameFlavorPair(LeadLep, SubLeadLep, sfL1L2);
    const TLorentzVector sfL1L2jj = hasSFL1L2 ? sfL1L2 + jj : TLorentzVector();

    fillSameFlavorPair("l1l2", LeadLep, SubLeadLep);
    fillSameFlavorPair("l1l3", LeadLep, ThirdLep);
    fillSameFlavorPair("l2l3", SubLeadLep, ThirdLep);

    SetBranch(tree, "mll", hasSFL1L2 ? static_cast<float>(sfL1L2.M()) : fMissing);
    SetBranch(tree, "ptll", hasSFL1L2 ? static_cast<float>(sfL1L2.Pt()) : fMissing);
    SetBranch(tree, "etall", hasSFL1L2 ? static_cast<float>(sfL1L2.Eta()) : fMissing);
    SetBranch(tree, "phill", hasSFL1L2 ? static_cast<float>(sfL1L2.Phi()) : fMissing);
    SetBranch(tree, "dRll", hasSFL1L2 ? static_cast<float>(LeadLep->DeltaR(*SubLeadLep)) : fMissing);
    SetBranch(tree, "dPhill", hasSFL1L2 ? static_cast<float>(std::fabs(LeadLep->DeltaPhi(*SubLeadLep))) : fMissing);
    SetBranch(tree, "lepChargeProduct", hasSFL1L2 ? LeadLep->Charge() * SubLeadLep->Charge() : iMissing);

    SetBranch(tree, "mjj", has2Jets ? jj.M() : fMissing);
    SetBranch(tree, "ptjj", has2Jets ? jj.Pt() : fMissing);
    SetBranch(tree, "dRjj", has2Jets ? selected_jets[0].DeltaR(selected_jets[1]) : fMissing);
    SetBranch(tree, "dPhijj", has2Jets ? std::fabs(selected_jets[0].DeltaPhi(selected_jets[1])) : fMissing);
    SetBranch(tree, "mlljj", (hasSFL1L2 && has2Jets) ? static_cast<float>(sfL1L2jj.M()) : fMissing);
    SetBranch(tree, "ptlljj", (hasSFL1L2 && has2Jets) ? static_cast<float>(sfL1L2jj.Pt()) : fMissing);
    SetBranch(tree, "ml1jj", has2Jets ? l1jj.M() : fMissing);
    SetBranch(tree, "ml2jj", has2Jets ? l2jj.M() : fMissing);
    SetBranch(tree, "ml3jj", (ThirdLep && has2Jets) ? l3jj.M() : fMissing);

    // Boosted fit observable: lead lepton + leading fatjet invariant mass.
    // For the boosted topology (collimated N -> l qq merged into one fatjet)
    // this reconstructs the WR resonance, where mlljj is undefined because the
    // event rarely has two resolved AK4 jets.
    const bool hasFatjet = fatjets.size() >= 1;
    const TLorentzVector l1fatjet = hasFatjet ? (*LeadLep + fatjets[0]) : TLorentzVector();
    SetBranch(tree, "ml1fatjet", hasFatjet ? static_cast<float>(l1fatjet.M()) : fMissing);
    SetBranch(tree, "ptl1fatjet", hasFatjet ? static_cast<float>(l1fatjet.Pt()) : fMissing);

    fillLeptonJetBranches("l1", LeadLep);
    fillLeptonJetBranches("l2", SubLeadLep);
    fillLeptonJetBranches("l3", ThirdLep);
    float minDRLepJet = fMissing;
    const Lepton* leptonsForDR[3] = {LeadLep, SubLeadLep, ThirdLep};
    for (const auto* lep : leptonsForDR) {
        if (!lep) continue;
        for (const auto& jet : selected_jets) {
            float dr = lep->DeltaR(jet);
            if (minDRLepJet == fMissing || dr < minDRLepJet) minDRLepJet = dr;
        }
    }
    SetBranch(tree, "minDR_lep_jet", minDRLepJet);

    SetBranch(tree, "ht", ht);
    SetBranch(tree, "lt", lt);
    SetBranch(tree, "st", st);
    SetBranch(tree, "met_pt", metPt);
    SetBranch(tree, "met_phi", metPhi);
    SetBranch(tree, "met_px", METv.Px());
    SetBranch(tree, "met_py", METv.Py());
    SetBranch(tree, "met_over_ht", ht > 0.f ? metPt / ht : fMissing);
    SetBranch(tree, "met_over_lt", lt > 0.f ? metPt / lt : fMissing);
    SetBranch(tree, "met_over_st", st > 0.f ? metPt / st : fMissing);
    fillLeptonMETBranches("l1", LeadLep);
    fillLeptonMETBranches("l2", SubLeadLep);
    fillLeptonMETBranches("l3", ThirdLep);
    SetBranch(tree, "dPhi_met_j1", selected_jets.size() >= 1 ? std::fabs(METv.DeltaPhi(selected_jets[0])) : fMissing);
    SetBranch(tree, "dPhi_met_j2", has2Jets ? std::fabs(METv.DeltaPhi(selected_jets[1])) : fMissing);
    SetBranch(tree, "met_mt_ll", hasSFL1L2 ? transverseMass(metPt, sfL1L2.Pt(), METv.DeltaPhi(sfL1L2)) : fMissing);
    fillLeptonFatJetBranches("l1", LeadLep);
    fillLeptonFatJetBranches("l2", SubLeadLep);
    fillLeptonFatJetBranches("l3", ThirdLep);

    FillTrees(tree);

    /*
    ///         Event preselection       ///
    // Require exactly 2 leptons, lead pT > 60, sublead pT > 53
    if (!((n_leptons == 2) && (leps[0]->Pt() > 60.0) && (leps[1]->Pt() > 53.0))) return;
    // --- Cutflow bin 3: 2 leptons + pT ---
    FillHist(dir + "/Cutflow", 3.0, weight, 10, 0., 10.);

    bool this_trigger_pass = false;
    bool tmp_isEE = false, tmp_isMM = false, tmp_isEM = false;

    if ((electrons.size() == 2) && (muons.size() == 0)) {
        if (electrons[0]->Pt() < el_set.Ele_Trigger_Safe_Pt_Cut) return;
        this_trigger_pass = pass_trig_elec;
        tmp_isEE = true;
    } else if ((muons.size() == 2) && (electrons.size() == 0)) {
        if (muons[0]->Pt() < mu_set.Muon_Trigger_Safe_Pt_Cut) return;
        this_trigger_pass = pass_trig_muon;
        tmp_isMM = true;
    } else if ((muons.size() == 1) && (electrons.size() == 1)) {
        if (muons[0]->Pt() < mu_set.Muon_Trigger_Safe_Pt_Cut) return;
        this_trigger_pass = pass_trig_muon;
        tmp_isEM = true;
    }

    // --- Cutflow bin 4: trigger fired (above safe pT) ---
    if (!this_trigger_pass) return;
    FillHist(dir + "/Cutflow", 4.0, weight, 10, 0., 10.);

    // Channel monitoring
    if (tmp_isEE) FillHist(dir + "/Channel", 0.0, weight, 3, 0., 3.);
    if (tmp_isMM) FillHist(dir + "/Channel", 1.0, weight, 3, 0., 3.);
    if (tmp_isEM) FillHist(dir + "/Channel", 2.0, weight, 3, 0., 3.);

    // --- Resolved preselection: >=2 jets, mutually dR-separated from the 2 leptons ---
    bool IsResolvedEvent = false;
    if (selected_jets.size() >= 2) {
        bool dRLeadJetLep    = (selected_jets[0].DeltaR(*LeadLep) > dR_Separation) && (selected_jets[0].DeltaR(*SubLeadLep) > dR_Separation);
        bool dRSubLeadJetLep = (selected_jets[1].DeltaR(*LeadLep) > dR_Separation) && (selected_jets[1].DeltaR(*SubLeadLep) > dR_Separation);
        bool dRTwoLeptons    = (LeadLep->DeltaR(*SubLeadLep) > dR_Separation);
        bool dRTwoJets       = (selected_jets[0].DeltaR(selected_jets[1]) > dR_Separation);
        if (dRLeadJetLep && dRSubLeadJetLep && dRTwoLeptons && dRTwoJets) IsResolvedEvent = true;
    }

    // --- Boosted preselection: at least one selected fatjet ---
    bool IsBoostedEvent = (fatjets.size() >= 1);

    if (IsResolvedEvent) FillHist(dir + "/Cutflow", 5.0, weight, 10, 0., 10.);
    if (IsBoostedEvent)  FillHist(dir + "/Cutflow", 6.0, weight, 10, 0., 10.);

    // Event passes preselection if it is resolved or boosted
    if (!(IsResolvedEvent || IsBoostedEvent)) return;
    // --- Cutflow bin 7: pass preselection (resolved OR boosted) ---
    FillHist(dir + "/Cutflow", 7.0, weight, 10, 0., 10.);
    */




    // Basic kinematics of preselected events (sanity monitoring)
    FillHist(dir + "/LeadLep_pt", LeadLep->Pt(), weight, 100, 0., 1000.);
    FillHist(dir + "/SubLeadLep_pt", SubLeadLep->Pt(), weight, 100, 0., 1000.);
    FillHist(dir + "/N_Jet", selected_jets.size(), weight, 10, 0., 10.);
    FillHist(dir + "/N_FatJet", fatjets.size(), weight, 10, 0., 10.);
    FillHist(dir + "/Mll", (*LeadLep + *SubLeadLep).M(), weight, 200, 0., 2000.);
}

//================ Helper functions ================//

bool HNWR_BDT_presel::Electrons::isPassCustomTightID(const Electron& el, const HNWR_BDT_presel::Electrons& eset) const {
    if (fabs(el.scEta()) < 1.566) {
        return el.PassID(eset.Electron_Tight_ID[0]);
    }
    int heepbit = el.VidNestedWPBitmapHEEP();
    if (!((heepbit & 3775) == 3775)) return false;
    float scE = (el.scEtOverPt() + 1) * el.Pt();
    double cutValue_HoverE = (-0.4 + 0.4 * fabs(el.scEta())) * el.rho() / scE + 0.05;
    if (!(el.hoe() < cutValue_HoverE)) return false;
    double cutValue_emhaddep1 = el.Pt() > 50. ? 2.5 + 0.03 * (el.Pt() - 50.) + (0.15 + 0.07 * fabs(el.scEta())) * el.rho() : 2.5 + (0.15 + 0.07 * fabs(el.scEta())) * el.rho();
    if (!(el.dr03EcalRecHitSumEt() + el.dr03HcalDepth1TowerSumEt() < cutValue_emhaddep1)) return false;
    return true;
}

bool HNWR_BDT_presel::Electrons::isPassCustomLooseID(const Electron& el) const {
    if (fabs(el.scEta()) <= 1.479) {
        if (!(el.hoe() < 0.05 + 1.28 / el.E() + 0.0422 * el.rho() / el.E())) return false;
        if (!(el.sieie() < 0.00107)) return false;
        if (!(fabs(el.deltaEtaInSeed()) < 0.00691)) return false;
        if (!(fabs(el.deltaPhiInSC()) < 0.175)) return false;
        if (!(fabs(el.eInvMinusPInv()) < 0.138)) return false;
        if (!(el.LostHits() <= 1)) return false;
        if (!(el.ConvVeto())) return false;
        return true;
    } else {
        if (!(el.hoe() < 0.05 + 2.3 / el.E() + 0.262 * el.rho() / el.E())) return false;
        if (!(el.sieie() < 0.0275)) return false;
        if (!(fabs(el.deltaEtaInSeed()) < 0.0121)) return false;
        if (!(fabs(el.deltaPhiInSC()) < 0.228)) return false;
        if (!(fabs(el.eInvMinusPInv()) < 0.127)) return false;
        if (!(el.LostHits() <= 1)) return false;
        if (!(el.ConvVeto())) return false;
        return true;
    }
}

bool HNWR_BDT_presel::Electrons::isPassLooseNoIso(const Electron& el) const {
    int bitmap = el.VidNestedWPBitmap();
    const int n_cuts = 10;
    const int bits_per_cut = 3;
    const int ignore_cut = 7;   // isolation cut index (ignored)
    const int id_level = 2;     // Loose working point
    const int mask = (1 << bits_per_cut) - 1;
    for (int cut_nr = 0; cut_nr < n_cuts; cut_nr++) {
        if (cut_nr == ignore_cut) continue;
        int value = (bitmap >> (cut_nr * bits_per_cut)) & mask;
        if (value < id_level) return false;
    }
    return true;
}

RVec<FatJet> HNWR_BDT_presel::Clean_Fatjet_with_tight_leptons(const RVec<FatJet>& fatjets, const RVec<Lepton*>& tight_leps) {
    RVec<FatJet> cleanedfatjets;
    for (unsigned int i = 0; i < fatjets.size(); i++) {
        FatJet fatjet = fatjets.at(i);
        bool isDRtoLepton = false;
        for (unsigned int j = 0; j < tight_leps.size(); j++) {
            if (fatjet.DeltaR(*tight_leps.at(j)) < 0.4) { isDRtoLepton = true; break; }
        }
        if (!isDRtoLepton) cleanedfatjets.push_back(fatjet);
    }
    return cleanedfatjets;
}

RVec<Jet> HNWR_BDT_presel::Clean_jet_with_loose_leptons(const RVec<Jet>& jets, const RVec<Lepton*>& loose_leps) {
    RVec<Jet> cleanedjets;
    for (unsigned int i = 0; i < jets.size(); i++) {
        Jet jet = jets.at(i);
        bool isDRtoLepton = false;
        for (unsigned int j = 0; j < loose_leps.size(); j++) {
            if (jet.DeltaR(*loose_leps.at(j)) < 0.4) { isDRtoLepton = true; break; }
        }
        if (!isDRtoLepton) cleanedjets.push_back(jet);
    }
    return cleanedjets;
}
