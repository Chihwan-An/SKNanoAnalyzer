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

    // One resolved + one boosted tree per variation (Central has no suffix), split
    // by lepton flavor channel: same-flavor EE/MM for SR and CR_DY, opposite-flavor
    // EM/ME for CR_FLV. For CR_FLV_boosted the tag records which flavor is the
    // pT-leading lepton (so ME can fill); CR_FLV_resolved always requires the muon
    // to pass the muon trigger regardless of pT order, so only the EM tree fills
    // (the ME resolved tree is created but stays empty).
    // d is a ROOT directory prefix like "Central/" or "JES_Up/": the tree name carries
    // the directory, and WriteHist() places it there (same layout as the histograms).
    // Only the SR needs a BDT ntuple. The CR (CR_DY / CR_FLV) is saved as
    // per-systematic histograms instead (see executeEventFromParameter), so no CR trees.
    auto makeTrees = [&](const TString& d) {
        for (const TString &ch : {TString("EE"), TString("MM")}) {
            NewTree(d + "SR_" + ch + "_BDTTree_resolved");
            NewTree(d + "SR_" + ch + "_BDTTree_boosted");
        }
    };
    // One tree set per object variation, grouped under a directory named after the
    // variation (Central/, JES_Up/, JES_Down/, JER_Up/, JER_Down/). Weight systematics
    // (Pileup, lepton SFs, XSec muF/muR/PDF/AlphaS) do NOT get their own trees -- they
    // leave the BDT inputs unchanged, so the score is identical and they live as
    // per-event weight branches inside the Central tree (see executeEventFromParameter).
    for (const auto &syst_dummy : *systHelper) {
        const TString sys = systHelper->getCurrentSysName();
        makeTrees(sys + "/");
    }

    // ---- Central-only auxiliary trees (background estimation / validation) ----
    // These carry the FULL BDT branch set (same filler as the SR trees) so the
    // trained boosters can score their events, but only the nominal (Central)
    // variation is written to keep the output size in check:
    //  * CR_DY / CR_FLV BDTTrees: restored CR ntuples for data/MC score-shape
    //    validation (the CRs also keep their per-systematic histograms).
    //  * CR_SS: same-sign resolved pair, 60 < mll < 300 (SR mll cut replaced) --
    //    ATLAS-style SS control region for diboson normalisation + charge-flip /
    //    fake validation. Overlaps the SR above mll > 200; cut at the fit level
    //    if needed (mll branch is stored).
    //  * SR_LL: resolved SR selection on the two LOOSE leptons with at least one
    //    failing tight -- the N_TL/N_LT/N_LL fake-factor application sideband.
    //  * CF_EE_Tree: charge-flip measurement tree (own small branch set): two
    //    tight electrons, 50 < mll < 130, CHARGE-INCLUSIVE, no jet cuts.
    for (const TString &ch : {TString("EE"), TString("MM")}) {
        NewTree("Central/CR_DY_" + ch + "_BDTTree_resolved");
        NewTree("Central/CR_DY_" + ch + "_BDTTree_boosted");
        NewTree("Central/CR_SS_" + ch + "_BDTTree_resolved");
        NewTree("Central/SR_LL_" + ch + "_BDTTree_resolved");
    }
    for (const TString &tag : {TString("EM"), TString("ME")}) {
        NewTree("Central/CR_FLV_" + tag + "_BDTTree_resolved");
        NewTree("Central/CR_FLV_" + tag + "_BDTTree_boosted");
    }
    NewTree("Central/CF_EE_Tree");

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
    AllGens.clear();
    if (!IsDATA) AllGens = GetAllGens();

    // tb decay tag: mark signal events whose LHE record contains a b (|PDG|=5) or
    // top (|PDG|=6) parton. Mirrors Reproduce20_002_copy::SetSignalFlags step (3);
    // recomputed once per event and written as the "is_tb" branch on every BDTTree.
    sig_isTb = false;
    if (!IsDATA) {
        for (const auto &p : GetAllLHEs()) {
            int apid = abs(p.PdgId());
            if (apid == 5 || apid == 6) { sig_isTb = true; break; }
        }
    }

    // Re-run the selection once per object variation (Central + JES/JER up/down);
    // each variation writes into its own BDTTree_<category>[_<syst>] tree.
    for (const auto &syst_dummy : *systHelper) {
        executeEventFromParameter();
    }
}

void HNWR_BDT_presel::executeEventFromParameter() {
    const TString this_syst = systHelper->getCurrentSysName();
    const TString dir = this_syst;

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

    // Gen-level charge of the dR<0.1-matched status-1 lepton: -1/+1, or -999
    // when unmatched or data. PID +11/+13 is the NEGATIVE lepton.
    const int iMissingGen = -999;
    auto genChargeOf = [&](const Lepton* lep) -> int {
        if (!lep || IsDATA || AllGens.empty()) return iMissingGen;
        const Gen matched = GetGenMatchedLepton(*lep, AllGens);
        if (matched.PID() == 0) return iMissingGen;
        return matched.PID() > 0 ? -1 : +1;
    };

    // --- Charge-flip measurement tree (Central only; data AND MC) --------------
    // AN-19-206 sec.8.2 recipe needs a CHARGE-INCLUSIVE Z-window dielectron
    // sample: analysis trigger + two tight electrons (trigger-safe lead pT,
    // sublead > 53) + 50 < mll < 130. No jet or charge requirement. The rate is
    // measured on MC truth (genCharge vs charge), the scale factor from the
    // data SS/weighted-OS ratio in the same window. `weight` is gen x lumi
    // (no lepton SFs), matching the SR trees' raw weight branch.
    if (this_syst == "Central" && Tight_electrons.size() == 2 && Tight_muons.size() == 0) {
        Electron* cf1 = Tight_electrons[0];
        Electron* cf2 = Tight_electrons[1];
        const bool cf_pt_ok = (cf1->Pt() > el_set.Ele_Trigger_Safe_Pt_Cut) && (cf2->Pt() > 53.0);
        if (pass_trig_elec && cf_pt_ok) {
            const TLorentzVector cfPair = *cf1 + *cf2;
            if (cfPair.M() > 50.0 && cfPair.M() < 130.0) {
                const TString cfTree = "Central/CF_EE_Tree";
                for (int i = 0; i < 2; i++) {
                    Electron* el = (i == 0) ? cf1 : cf2;
                    const TString p = Form("ele%d", i + 1);
                    SetBranch(cfTree, p + "_pt", el->Pt());
                    SetBranch(cfTree, p + "_eta", el->Eta());
                    SetBranch(cfTree, p + "_scEta", el->scEta());
                    SetBranch(cfTree, p + "_phi", el->Phi());
                    SetBranch(cfTree, p + "_charge", el->Charge());
                    SetBranch(cfTree, p + "_tightCharge", static_cast<int>(el->TightCharge()));
                    SetBranch(cfTree, p + "_genCharge", genChargeOf(el));
                    SetBranch(cfTree, p + "_genPartFlav_truth", static_cast<int>(el->GenPartFlav()));
                }
                SetBranch(cfTree, "m_l1l2", static_cast<float>(cfPair.M()));
                SetBranch(cfTree, "pt_l1l2", static_cast<float>(cfPair.Pt()));
                SetBranch(cfTree, "isSS", cf1->Charge() * cf2->Charge() > 0);
                SetBranch(cfTree, "nJet", static_cast<int>(selected_jets.size()));
                SetBranch(cfTree, "weight", weight);
                FillTrees(cfTree);
            }
        }
    }

    // Region-aware jet<->fatjet cross-cleaning (leptons already cleaned above, so they
    // keep top priority). Build both topology variants up front:
    //  - Resolved: AK4 jets take priority -> keep jets, drop fatjets overlapping a jet.
    //  - Boosted:  fatjets take priority  -> keep fatjets, drop jets overlapping a fatjet.
    // The right variant is selected once the topology is known (see below).
    RVec<FatJet> fatjets_resolved      = Clean_Fatjet_with_jets(fatjets, selected_jets);
    RVec<Jet>    selected_jets_boosted = Clean_jet_with_fatjets(selected_jets, fatjets);

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
    auto passResolveEMuTrigger = [&](const Lepton* lep, int& outChannel) {
        outChannel = -999;
        if (!lep) return false;
        
        if (lep->IsMuon()) {
            if (lep->Pt() < mu_set.Muon_Trigger_Safe_Pt_Cut) return false;
            outChannel = 1;
            return pass_trig_muon;
        }
        return false;
    };
    // Resolved uses the jet-priority collection (selected_jets); boosted uses the
    // fatjet-priority collection (selected_jets_boosted). The jet collection is passed
    // explicitly so each topology evaluates the 4-object dR against the right jets.
    const bool has2Jets_boosted = selected_jets_boosted.size() >= 2;
    auto computeFourObjectDR = [&](const RVec<Jet>& jetcol, const Lepton* lep1, const Lepton* lep2,
                                   bool& outLeadJetLep, bool& outSubLeadJetLep,
                                   bool& outTwoLeptons, bool& outTwoJets) {
        outLeadJetLep = false;
        outSubLeadJetLep = false;
        outTwoLeptons = false;
        outTwoJets = false;
        if (!lep1 || !lep2 || jetcol.size() < 2) return false;
        outLeadJetLep = (jetcol[0].DeltaR(*lep1) > dR_Separation) && (jetcol[0].DeltaR(*lep2) > dR_Separation);
        outSubLeadJetLep = (jetcol[1].DeltaR(*lep1) > dR_Separation) && (jetcol[1].DeltaR(*lep2) > dR_Separation);
        outTwoLeptons = (lep1->DeltaR(*lep2) > dR_Separation);
        outTwoJets = (jetcol[0].DeltaR(jetcol[1]) > dR_Separation);
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
    const bool passResolvedDRCandidate = computeFourObjectDR(selected_jets, resolvedLead, resolvedSubLead, resolvedDRLeadJetLep, resolvedDRSubLeadJetLep, resolvedDRTwoLeptons, resolvedDRTwoJets);
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
    const bool passBoostedResolvedDR = computeFourObjectDR(selected_jets_boosted, boostedLead, boostedSubLead, boostedDRLeadJetLep, boostedDRSubLeadJetLep, boostedDRTwoLeptons, boostedDRTwoJets);
    const bool passBoostedDRFail = !has2Jets_boosted || !passBoostedResolvedDR;
    bool isBoostedCandidate = (!isResolvedCandidate && passBoostedMll && passBoostedDRFail && fatjets.size() >= 1);

    // --- CR_DY resolved: same-flavor, 60 < mll < 150 ---
    const double lowMllMin = 60.0;
    const double lowMllMax = 150.0;
    auto isOppositeFlavorPair = [](const Lepton* lep1, const Lepton* lep2) {
        if (!lep1 || !lep2) return false;
        return (lep1->IsElectron() && lep2->IsMuon()) || (lep1->IsMuon() && lep2->IsElectron());
    };
    TLorentzVector resolvedDYDilepton;
    bool isResolvedDYCR = false;
    if (!isResolvedCandidate && passResolvedTrigger && passResolvedDRCandidate && resolvedLead && resolvedSubLead) {
        if (makeSameFlavorPair(resolvedLead, resolvedSubLead, resolvedDYDilepton))
            isResolvedDYCR = (resolvedDYDilepton.M() > lowMllMin) && (resolvedDYDilepton.M() < lowMllMax);
    }

    // --- CR_FLV resolved: opposite-flavor, mll > 200. The pt>60/pt>53 cuts still
    // follow pT order (Tight_leps[0]/[1]) like every other region -> either lepton
    // may be the pT-leading one. But trigger is judged on the muon specifically
    // (there is exactly one, since electrons.size()==1 && muons.size()==1), not on
    // whichever lepton happens to be pT-leading: the muon need not be the larger of
    // the two, it just has to pass pt>=safe-cut and the muon trigger. Unlike
    // boosted, this is NOT split by which lepton is pT-leading: resolvedFlavLead/
    // SubLead are fixed as electron/muon so the flavor tag below always resolves to
    // a single "EM" tree.
    Lepton* resolvedFlavLead = nullptr;
    Lepton* resolvedFlavSubLead = nullptr;
    int resolvedFlavChannel = -999;
    bool passResolvedFlavTrigger = false;
    bool resolvedFlavDRLeadJetLep = false, resolvedFlavDRSubLeadJetLep = false;
    bool resolvedFlavDRTwoLeptons = false, resolvedFlavDRTwoJets = false;
    TLorentzVector resolvedFlavDilepton;
    bool isResolvedFlavCR = false;
    if (!isResolvedCandidate && !isResolvedDYCR && n_leptons == 2 && electrons.size() == 1 && muons.size() == 1) {
        Lepton* flavPtLead = Tight_leps[0];
        Lepton* flavPtSubLead = Tight_leps[1];
        Electron* flavElectron = electrons[0];
        Muon*     flavMuon     = muons[0];
        const bool flav_pt_ok = (flavPtLead->Pt() > 60.0) && (flavPtSubLead->Pt() > 53.0);
        passResolvedFlavTrigger = flav_pt_ok && passResolveEMuTrigger(flavMuon, resolvedFlavChannel);
        if (passResolvedFlavTrigger) {
            resolvedFlavLead = flavElectron;
            resolvedFlavSubLead = flavMuon;
            resolvedFlavDilepton = *resolvedFlavLead + *resolvedFlavSubLead;
            const bool passFlavDR = computeFourObjectDR(selected_jets, resolvedFlavLead, resolvedFlavSubLead,
                resolvedFlavDRLeadJetLep, resolvedFlavDRSubLeadJetLep,
                resolvedFlavDRTwoLeptons, resolvedFlavDRTwoJets);
            isResolvedFlavCR = (resolvedFlavDilepton.M() > 200.0) && passFlavDR;
        }
    }

    // --- Boosted CR: DY (SF, low mll) and FLV (OF, high mll) ---
    Lepton* boostedDYSubLead = nullptr;
    Lepton* boostedFlavSubLead = nullptr;
    TLorentzVector boostedDYDilepton;
    TLorentzVector boostedFlavDilepton;
    bool hasBoostedDYDilepton = false;
    bool hasBoostedFlavDilepton = false;
    if (boostedLead) {
        for (unsigned int i = 0; i < boostedExtraLooseLeps.size(); i++) {
            TLorentzVector cand = *boostedLead + *boostedExtraLooseLeps[i];
            if (!hasBoostedDYDilepton && isSameFlavorPair(boostedLead, boostedExtraLooseLeps[i]) &&
                cand.M() > lowMllMin && cand.M() < lowMllMax) {
                boostedDYSubLead = boostedExtraLooseLeps[i];
                boostedDYDilepton = cand;
                hasBoostedDYDilepton = true;
            }
            if (!hasBoostedFlavDilepton && isOppositeFlavorPair(boostedLead, boostedExtraLooseLeps[i]) &&
                cand.M() > 200.0) {
                boostedFlavSubLead = boostedExtraLooseLeps[i];
                boostedFlavDilepton = cand;
                hasBoostedFlavDilepton = true;
            }
            if (hasBoostedDYDilepton && hasBoostedFlavDilepton) break;
        }
    }
    bool boostedDYDRLeadJetLep = false, boostedDYDRSubLeadJetLep = false;
    bool boostedDYDRTwoLeptons = false, boostedDYDRTwoJets = false;
    const bool passBoostedDYResolvedDR = computeFourObjectDR(selected_jets_boosted, boostedLead, boostedDYSubLead,
        boostedDYDRLeadJetLep, boostedDYDRSubLeadJetLep, boostedDYDRTwoLeptons, boostedDYDRTwoJets);
    const bool passBoostedDYDRFail = !has2Jets_boosted || !passBoostedDYResolvedDR;
    bool boostedFlavDRLeadJetLep = false, boostedFlavDRSubLeadJetLep = false;
    bool boostedFlavDRTwoLeptons = false, boostedFlavDRTwoJets = false;
    const bool passBoostedFlavResolvedDR = computeFourObjectDR(selected_jets_boosted, boostedLead, boostedFlavSubLead,
        boostedFlavDRLeadJetLep, boostedFlavDRSubLeadJetLep, boostedFlavDRTwoLeptons, boostedFlavDRTwoJets);
    const bool passBoostedFlavDRFail = !has2Jets_boosted || !passBoostedFlavResolvedDR;
    const bool isBoostedDYCR = (!isResolvedCandidate && !isResolvedDYCR && !isResolvedFlavCR &&
        passBoostedTrigger && hasBoostedDYDilepton && passBoostedDYDRFail && fatjets.size() >= 1);
    const bool isBoostedFlavCR = (!isResolvedCandidate && !isResolvedDYCR && !isResolvedFlavCR &&
        !isBoostedDYCR && passBoostedTrigger && hasBoostedFlavDilepton && passBoostedFlavDRFail && fatjets.size() >= 1);

    // --- Auxiliary categories (Central only) -----------------------------------
    auto isTightLepton = [&](const Lepton* lep) {
        return lep && std::find(Tight_leps.begin(), Tight_leps.end(), lep) != Tight_leps.end();
    };
    // CR_SS: the resolved SR selection with the mll > 200 cut replaced by
    // 60 < mll < 300 and the pair required SAME-SIGN. Sources the SS-category
    // background fit (prompt-SS diboson normalisation; charge-flip / fake
    // validation). Overlaps the SS part of the SR for mll in (200, 300) -- the
    // mll branch is stored, so restrict at the fit level if needed.
    bool isSSCREvent = false;
    if (this_syst == "Central" && passResolvedTrigger && passResolvedDRCandidate &&
        resolvedLead && resolvedSubLead &&
        resolvedLead->Charge() * resolvedSubLead->Charge() > 0) {
        TLorentzVector ssPair;
        if (makeSameFlavorPair(resolvedLead, resolvedSubLead, ssPair) &&
            ssPair.M() > 60.0 && ssPair.M() < 300.0) {
            isSSCREvent = true;
        }
    }
    // SR_LL: the resolved SR selection evaluated on the two LOOSE leptons, with
    // at least one of them FAILING tight -- the N_TL/N_LT/N_LL sideband the
    // fake-factor method reweights into the SR. (Loose-not-tight: electrons fail
    // HEEP but pass loose-no-iso; muons pass HighPt ID but fail TkRelIso<0.1.)
    // Tight-tight events are exactly the SR tree, so they are excluded here.
    Lepton* llLead = nullptr;
    Lepton* llSubLead = nullptr;
    int llChannel = -999;
    bool isLLEvent = false;
    if (this_syst == "Central" && n_loose_leptons == 2) {
        Lepton* l0 = Loose_leps[0];
        Lepton* l1 = Loose_leps[1];
        const bool notBothTight = !(isTightLepton(l0) && isTightLepton(l1));
        TLorentzVector llPair;
        if (notBothTight && (l0->Pt() > 60.0) && (l1->Pt() > 53.0) &&
            makeSameFlavorPair(l0, l1, llPair) && llPair.M() > 200.0) {
            bool llDR1 = false, llDR2 = false, llDR3 = false, llDR4 = false;
            if (passLeadTrigger(l0, llChannel) &&
                computeFourObjectDR(selected_jets, l0, l1, llDR1, llDR2, llDR3, llDR4)) {
                llLead = l0;
                llSubLead = l1;
                isLLEvent = true;
            }
        }
    }

    const bool anyMainRegion = isResolvedCandidate || isBoostedCandidate || isResolvedDYCR ||
                               isResolvedFlavCR || isBoostedDYCR || isBoostedFlavCR;
    if (!(anyMainRegion || isSSCREvent || isLLEvent)) return;

    // The ntuple fill below runs once for the (unique) main region and once per
    // matching AUXILIARY category (CR_SS / SR_LL), each with its own lepton pair,
    // tree, and topology-committed jet collections. The lambda parameters
    // deliberately SHADOW the outer LeadLep/SubLeadLep/channel/... so the filling
    // code below is the original main-region code unchanged; the caller passes the
    // topology-committed collections (resolved: jets win / boosted: fatjets win).
    // isMainFill gates the cutflow/LeadFlavorChannel histograms so auxiliary fills
    // never double count them.
    auto fillNtuple = [&](const TString& regionPrefix, Lepton* LeadLep, Lepton* SubLeadLep,
                          int channel, bool this_trigger_pass, bool isResolved,
                          RVec<Jet> selected_jets, RVec<FatJet> fatjets, bool isMainFill) {
    if (!LeadLep || !SubLeadLep || !this_trigger_pass) return;
    const bool has2Jets = selected_jets.size() >= 2;

    // Flavor channel tag: EE/MM for same-flavor (SR, CR_DY), EM/ME for the
    // opposite-flavor CR_FLV (ordered lead->sublead). For CR_FLV_resolved,
    // resolvedFlavLead/SubLead are fixed to electron/muon above, so this always
    // resolves to "EM"; for CR_FLV_boosted, LeadLep/SubLeadLep still follow the
    // actual pT order, so both EM and ME can occur.
    TString chTag;
    if (LeadLep->IsElectron() && SubLeadLep->IsElectron())      chTag = "EE";
    else if (LeadLep->IsMuon() && SubLeadLep->IsMuon())         chTag = "MM";
    else if (LeadLep->IsElectron() && SubLeadLep->IsMuon())     chTag = "EM";
    else                                                        chTag = "ME";
    const TString tree = dir + "/" + regionPrefix + chTag + "_" + (isResolved ? "BDTTree_resolved" : "BDTTree_boosted");
    // Region + flavor + topology namespace for the monitoring/CR histograms,
    // mirroring the tree name: e.g. SR_EE_resolved, CR_DY_MM_boosted, CR_FLV_EM_resolved.
    const TString topoName = isResolved ? "resolved" : "boosted";
    const TString regionChanCat = regionPrefix + chTag + "_" + topoName;
    if (isMainFill) {
    // --- Cutflow bin 5: any SR/CR candidate passed ---
    FillHist(dir + "/Cutflow", 5.0, weight, 10, 0., 10.);
    // --- Cutflow bin 6: resolved / bin 7: boosted (SR only) ---
    if (isResolvedCandidate) FillHist(dir + "/Cutflow", 6.0, weight, 10, 0., 10.);
    if (isBoostedCandidate)  FillHist(dir + "/Cutflow", 7.0, weight, 10, 0., 10.);
    FillHist(dir + "/LeadFlavorChannel", channel, weight, 2, 0., 2.);
    }

    // ==========================================================================
    //  WEIGHT SYSTEMATICS (stored as per-event weight branches, not extra trees)
    //  Mirrors BDT_CR.cc's scale-factor weight functions and adds the XSec(theory)
    //  weight variations (muF, muR, PDF, alpha_S). These leave the BDT inputs
    //  unchanged, so the score is identical and no separate tree/evaluation is needed.
    //  During the "Central" iteration calculateWeight() returns {Central + every
    //  weight-syst Up/Down}; each is written below as a weight_<name> branch. For a
    //  JES/JER object iteration it returns only that tree's SF-included nominal weight.
    // ==========================================================================
    // Leptons entering the scale factors: the two selected signal leptons.
    RVec<Electron*> selectedElectronsForSF;
    RVec<Muon*>     selectedMuonsForSF;
    auto collectLeptonForSF = [&](Lepton* lep) {
        if (!lep) return;
        if (lep->IsElectron())  selectedElectronsForSF.push_back(static_cast<Electron*>(lep));
        else if (lep->IsMuon()) selectedMuonsForSF.push_back(static_cast<Muon*>(lep));
    };
    collectLeptonForSF(LeadLep);
    collectLeptonForSF(SubLeadLep);

    std::unordered_map<std::string, std::variant<std::function<float(MyCorrection::variation, TString)>, std::function<float()>>> weight_function_map;
    auto dummy_sf = [](MyCorrection::variation var, TString source) -> float { (void)var; (void)source; return 1.0f; };

    weight_function_map["PU_Weight"] = [&](MyCorrection::variation var, TString source) -> float {
        (void)source;
        return myCorr->GetPUWeight(ev.nTrueInt(), var);
    };
    weight_function_map["E_Id_Weight"] = [&](MyCorrection::variation var, TString source) -> float {
        (void)source;
        float sf = 1.0f;
        for (const auto* el : selectedElectronsForSF) {
            const float absEta = std::fabs(el->Eta());
            float base = 1.0f, unc = 0.0f;
            if (DataEra == "2023") {
                if (absEta < 1.444)                      { base = 1.007f; unc = 0.004f; }
                else if (absEta > 1.566 && absEta < 2.5) { base = 0.988f; unc = 0.005f; }
            } else if (DataEra == "2023BPix") {
                if (absEta < 1.444)                      { base = 1.009f; unc = 0.005f; }
                else if (absEta > 1.566 && absEta < 2.5) { base = 0.988f; unc = 0.004f; }
            }
            if (var == MyCorrection::variation::up)   base += unc;
            if (var == MyCorrection::variation::down) base -= unc;
            sf *= base;
        }
        return sf;
    };
    weight_function_map["E_Reco_Weight"] = [&](MyCorrection::variation var, TString source) -> float {
        (void)source;
        float sf = 1.0f;
        for (const auto* el : selectedElectronsForSF)
            sf *= myCorr->GetElectronRECOSF(std::fabs(el->Eta()), el->Pt(), el->Phi(), var);
        return sf;
    };
    weight_function_map["E_Trig_Weight"] = [&](MyCorrection::variation var, TString source) -> float {
        (void)source;
        if (!LeadLep->IsElectron() || DataEra == "2017") return 1.0f;
        const Electron* leadElectron = static_cast<Electron*>(LeadLep);
        const float absEta = std::fabs(leadElectron->Eta());
        if (leadElectron->Pt() <= 130.) return 1.0f;
        float sf = 1.0f, unc = 0.0f;
        if (absEta < 1.444) {
            if (DataEra == "2022")         { sf = 0.995f; unc = 0.004f; }
            else if (DataEra == "2022EE")  { sf = 0.990f; unc = 0.007f; }
            else if (DataEra == "2023")    { sf = 0.992f; unc = 0.006f; }
            else if (DataEra == "2023BPix"){ sf = 0.993f; unc = 0.001f; }
        } else if (absEta > 1.566 && absEta < 2.5) {
            if (DataEra == "2022")         { sf = 0.991f; unc = 0.009f; }
            else if (DataEra == "2022EE")  { sf = 0.981f; unc = 0.017f; }
            else if (DataEra == "2023")    { sf = 0.979f; unc = 0.019f; }
            else if (DataEra == "2023BPix"){ sf = 0.978f; unc = 0.019f; }
        }
        if (var == MyCorrection::variation::up)   sf += unc;
        if (var == MyCorrection::variation::down) sf -= unc;
        return sf;
    };
    weight_function_map["M_Id_Weight"] = [&](MyCorrection::variation var, TString source) -> float {
        (void)source;
        if (DataEra == "2017") return 1.0f;
        float sf = 1.0f;
        for (const auto* mu : selectedMuonsForSF)
            sf *= myCorr->GetMuonIDSF("NUM_HighPtID_DEN_GlobalMuonProbes", *mu, var);
        return sf;
    };
    weight_function_map["M_Reco_Weight"] = [&](MyCorrection::variation var, TString source) -> float {
        (void)source;
        float sf = 1.0f;
        for (const auto* mu : selectedMuonsForSF)
            sf *= myCorr->GetMuonRECOSF(*mu, var);
        return sf;
    };
    weight_function_map["M_Trig_Weight"] = [&](MyCorrection::variation var, TString source) -> float {
        (void)source;
        if (!LeadLep->IsMuon() || DataEra == "2017" || selectedMuonsForSF.empty()) return 1.0f;
        RVec<Muon*> trigMuons;
        for (auto* mu : selectedMuonsForSF) trigMuons.push_back(mu);
        return myCorr->GetMuonTriggerSF("NUM_HLT_DEN_HighPtLooseRelIsoProbes", trigMuons, var);
    };
    weight_function_map["M_Iso_Weight"] = [&](MyCorrection::variation var, TString source) -> float {
        (void)source;
        if (DataEra == "2017") return 1.0f;
        float sf = 1.0f;
        for (const auto* mu : selectedMuonsForSF)
            sf *= myCorr->GetMuonIDSF("NUM_probe_LooseRelTkIso_DEN_HighPtProbes", *mu, var);
        return sf;
    };
    weight_function_map["JER_Variation"] = dummy_sf;
    weight_function_map["JES_Variation"] = dummy_sf;

    // XSec(theory) weight systematics (reuse AnalyzerCore::GetScaleVariation + LHE PDF weights).
    // muF: vary factorization scale, keep renormalization scale nominal.
    weight_function_map["ScaleWeight_muF"] = [&](MyCorrection::variation var, TString source) -> float {
        (void)source;
        return GetScaleVariation(var, MyCorrection::variation::nom);
    };
    // muR: vary renormalization scale, keep factorization scale nominal.
    weight_function_map["ScaleWeight_muR"] = [&](MyCorrection::variation var, TString source) -> float {
        (void)source;
        return GetScaleVariation(MyCorrection::variation::nom, var);
    };
    // PDF envelope: Hessian sum in quadrature over members 1..100 (LHEPdfWeight[0] is central == 1).
    weight_function_map["PDF_Weight"] = [&](MyCorrection::variation var, TString source) -> float {
        (void)source;
        if (nLHEPdfWeight < 103) return 1.0f;
        const float w0 = LHEPdfWeight[0];
        float sumSq = 0.0f;
        for (int i = 1; i <= 100; i++) { const float dw = LHEPdfWeight[i] - w0; sumSq += dw * dw; }
        const float deltaPDF = std::sqrt(sumSq);
        if (var == MyCorrection::variation::up)   return w0 + deltaPDF;
        if (var == MyCorrection::variation::down) return w0 - deltaPDF;
        return 1.0f;
    };
    // alpha_S: dedicated PDF members (101 = down, 102 = up).
    weight_function_map["AlphaS_Weight"] = [&](MyCorrection::variation var, TString source) -> float {
        (void)source;
        if (nLHEPdfWeight < 103) return 1.0f;
        if (var == MyCorrection::variation::up)   return LHEPdfWeight[102];
        if (var == MyCorrection::variation::down) return LHEPdfWeight[101];
        return 1.0f;
    };

    std::unordered_map<std::string, float> weight_map;
    if (!IsDATA) {
        systHelper->assignWeightFunctionMap(weight_function_map);
        weight_map = systHelper->calculateWeight();
    }

    // ==========================================================================
    //  BRANCH FILLING
    //  Everything above selects the event and fixes the region/topology/tree.
    //  Everything below only computes derived quantities and fills the ntuple.
    // ==========================================================================
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
        SetBranch(tree, prefix + "_genCharge", genChargeOf(el));
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
        SetBranch(tree, prefix + "_genCharge", genChargeOf(mu));
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

    const bool passMllSR = isResolvedCandidate ? passResolvedMll : (isBoostedCandidate ? passBoostedMll : false);
    bool dRLeadJetLep = false;
    bool dRSubLeadJetLep = false;
    bool dRTwoLeptons = false;
    bool dRTwoJets = false;
    const bool passResolvedDR = computeFourObjectDR(selected_jets, LeadLep, SubLeadLep, dRLeadJetLep, dRSubLeadJetLep, dRTwoLeptons, dRTwoJets);

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

    // Dilepton pair: computed from any-flavor combination so CR_FLV gets real mll values.
    const TLorentzVector dilep = *LeadLep + *SubLeadLep;
    const bool isSFpair = isSameFlavorPair(LeadLep, SubLeadLep);
    const TLorentzVector dilepjj = has2Jets ? dilep + jj : TLorentzVector();
    // Keep sfL1L2 alias so fillSameFlavorPair ("l1l2") still works (returns -999 for OF pairs).
    TLorentzVector sfL1L2;
    const bool hasSFL1L2 = makeSameFlavorPair(LeadLep, SubLeadLep, sfL1L2);
    const TLorentzVector sfL1L2jj = hasSFL1L2 ? sfL1L2 + jj : TLorentzVector();

    // The SR BDT ntuple is written for EVERY object variation; the auxiliary
    // trees (CR_DY / CR_FLV / CR_SS / SR_LL) exist only under Central/ (their
    // events still fill the per-systematic CR histograms below).
    const bool writeTree = (regionPrefix == "SR_") || (this_syst == "Central");
    if (writeTree) {
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
    // Signal truth tag: 1 if the WR* decay chain contains a b (|PDG|=5) or top
    // (|PDG|=6) LHE parton (tb decay), 0 otherwise (light-quark decay / data / bkg).
    // Not a BDT input feature -- only used to split the signal score by decay mode.
    SetBranch(tree, "is_tb", sig_isTb ? 1 : 0);
    // Weight systematics stored as per-event branches (MC only): weight_Central is the
    // SF-included nominal, and each weight_<SystName>_Up/_Down is the full event weight
    // with that one systematic varied -- Pileup / Electron{ID,Reco,Trig} /
    // Muon{ID,Reco,Trig,Iso} and the XSec(theory) ScaleWeight_muF/muR, PDF, AlphaS.
    // The BDT inputs are unchanged, so downstream evaluates the score once (on nominal
    // features) and fills each variation's histogram with the matching weight branch.
    // On a JES/JER object tree weight_map holds only that tree's SF-included nominal.
    // The raw "weight" branch above (gen x lumi, no SF) is left unchanged.
    for (const auto& [wname, wval] : weight_map) {
        SetBranch(tree, TString("weight_") + wname.c_str(), weight * wval);
    }
    SetBranch(tree, "passMllSR", passMllSR);
    SetBranch(tree, "passResolvedDR", passResolvedDR);
    SetBranch(tree, "passBoostedDRFail", passBoostedDRFail);
    SetBranch(tree, "passDRLeadJetLep", dRLeadJetLep);
    SetBranch(tree, "passDRSubLeadJetLep", dRSubLeadJetLep);
    SetBranch(tree, "passDRTwoLeptons", dRTwoLeptons);
    SetBranch(tree, "passDRTwoJets", dRTwoJets);
    SetBranch(tree, "isResolvedCandidate", isResolvedCandidate);
    SetBranch(tree, "isBoostedCandidate", isBoostedCandidate);
    SetBranch(tree, "isResolvedDYCR", isResolvedDYCR);
    SetBranch(tree, "isResolvedFlavCR", isResolvedFlavCR);
    SetBranch(tree, "isBoostedDYCR", isBoostedDYCR);
    SetBranch(tree, "isBoostedFlavCR", isBoostedFlavCR);
    SetBranch(tree, "category", isResolved ? 0 : 1);  // 0 = resolved, 1 = boosted
    // region: 0 = SR, 1 = CR_DY, 2 = CR_FLV
    const int region_val = (regionPrefix == "SR_") ? 0 : (regionPrefix == "CR_DY_") ? 1 : 2;
    SetBranch(tree, "region", region_val);
    SetBranch(tree, "isSameFlavor", isSFpair);

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

    fillSameFlavorPair("l1l2", LeadLep, SubLeadLep);
    fillSameFlavorPair("l1l3", LeadLep, ThirdLep);
    fillSameFlavorPair("l2l3", SubLeadLep, ThirdLep);

    // mll/dRll etc. use any-flavor dilep so CR_FLV events are not -999.
    SetBranch(tree, "mll", static_cast<float>(dilep.M()));
    SetBranch(tree, "ptll", static_cast<float>(dilep.Pt()));
    SetBranch(tree, "etall", static_cast<float>(dilep.Eta()));
    SetBranch(tree, "phill", static_cast<float>(dilep.Phi()));
    SetBranch(tree, "dRll", static_cast<float>(LeadLep->DeltaR(*SubLeadLep)));
    SetBranch(tree, "dPhill", static_cast<float>(std::fabs(LeadLep->DeltaPhi(*SubLeadLep))));
    SetBranch(tree, "lepChargeProduct", LeadLep->Charge() * SubLeadLep->Charge());

    SetBranch(tree, "mjj", has2Jets ? jj.M() : fMissing);
    SetBranch(tree, "ptjj", has2Jets ? jj.Pt() : fMissing);
    SetBranch(tree, "dRjj", has2Jets ? selected_jets[0].DeltaR(selected_jets[1]) : fMissing);
    SetBranch(tree, "dPhijj", has2Jets ? std::fabs(selected_jets[0].DeltaPhi(selected_jets[1])) : fMissing);
    SetBranch(tree, "mlljj", has2Jets ? static_cast<float>(dilepjj.M()) : fMissing);
    SetBranch(tree, "ptlljj", has2Jets ? static_cast<float>(dilepjj.Pt()) : fMissing);
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
    } // end writeTree BDT ntuple

    // --- CR histograms (CR_DY / CR_FLV): save mlljj, mll, HT, jet number ---
    // Saved per systematic, one directory per variation (Central, Pileup_Up,
    // ElectronID_Up, ..., ScaleWeight_muF_Up, PDF_Up, AlphaS_Up, and the JES/JER object
    // variations), matching Reproduce20_002_copy: the weight-only systematics come from
    // weight_map during the Central iteration, and JES/JER from their own object passes.
    if (regionPrefix == "CR_DY_" || regionPrefix == "CR_FLV_") {
        std::vector<std::pair<TString, float>> fill_targets;
        if (IsDATA) {
            fill_targets.push_back({dir, weight});
        } else {
            for (const auto& [sn, sf_val] : weight_map)
                fill_targets.push_back({TString(sn.c_str()), weight * sf_val});
        }
        for (const auto& [syst_name, fw] : fill_targets) {
            const TString crName = syst_name + "/" + regionChanCat;
            if (isResolved) {
                // Resolved: WR candidate mass = ll + jj
                if (has2Jets) FillHist(crName + "/mlljj", dilepjj.M(), fw, 200, 0., 4000.);
            } else {
                // Boosted: WR candidate mass = lead lepton + leading fatjet
                if (fatjets.size() >= 1)
                    FillHist(crName + "/mlfatjet", (*LeadLep + fatjets[0]).M(), fw, 200, 0., 4000.);
            }
            FillHist(crName + "/mll", dilep.M(), fw, 200, 0., 2000.);
            FillHist(crName + "/HT", ht, fw, 200, 0., 4000.);
            FillHist(crName + "/N_Jet", selected_jets.size(), fw, 20, 0., 20.);
        }
    }

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




    // Basic kinematics of preselected events (sanity monitoring), namespaced by
    // region + flavor + topology, e.g. Central/SR_EE_resolved/LeadLep_pt.
    const TString monName = dir + "/" + regionChanCat;
    FillHist(monName + "/LeadLep_pt", LeadLep->Pt(), weight, 100, 0., 1000.);
    FillHist(monName + "/SubLeadLep_pt", SubLeadLep->Pt(), weight, 100, 0., 1000.);
    FillHist(monName + "/N_Jet", selected_jets.size(), weight, 10, 0., 10.);
    FillHist(monName + "/N_FatJet", fatjets.size(), weight, 10, 0., 10.);
    FillHist(monName + "/Mll", (*LeadLep + *SubLeadLep).M(), weight, 200, 0., 2000.);
    }; // end fillNtuple

    // ---- main region: at most one of the six flags is set (mutually exclusive) --
    if (anyMainRegion) {
        TString regionPrefix;
        bool isResolved;
        if (isResolvedCandidate || isBoostedCandidate) {
            regionPrefix = "SR_";
            isResolved = isResolvedCandidate;
            LeadLep = isResolvedCandidate ? resolvedLead : boostedLead;
            SubLeadLep = isResolvedCandidate ? resolvedSubLead : boostedSubLead;
            channel = isResolvedCandidate ? resolvedChannel : boostedChannel;
            this_trigger_pass = isResolvedCandidate ? passResolvedTrigger : passBoostedTrigger;
        } else if (isResolvedDYCR || isBoostedDYCR) {
            regionPrefix = "CR_DY_";
            isResolved = isResolvedDYCR;
            LeadLep = isResolvedDYCR ? resolvedLead : boostedLead;
            SubLeadLep = isResolvedDYCR ? resolvedSubLead : boostedDYSubLead;
            channel = isResolvedDYCR ? resolvedChannel : boostedChannel;
            this_trigger_pass = isResolvedDYCR ? passResolvedTrigger : passBoostedTrigger;
        } else {
            regionPrefix = "CR_FLV_";
            isResolved = isResolvedFlavCR;
            LeadLep = isResolvedFlavCR ? resolvedFlavLead : boostedLead;
            SubLeadLep = isResolvedFlavCR ? resolvedFlavSubLead : boostedFlavSubLead;
            channel = isResolvedFlavCR ? resolvedFlavChannel : boostedChannel;
            this_trigger_pass = isResolvedFlavCR ? passResolvedFlavTrigger : passBoostedTrigger;
        }
        // Commit to the topology-cleaned collections (resolved: AK4 jets win,
        // fatjets cleaned against jets; boosted: fatjets win, jets cleaned).
        fillNtuple(regionPrefix, LeadLep, SubLeadLep, channel, this_trigger_pass, isResolved,
                   isResolved ? selected_jets : selected_jets_boosted,
                   isResolved ? fatjets_resolved : fatjets, true);
    }
    // ---- auxiliary categories (Central only; may overlap the main region) ------
    if (isSSCREvent)
        fillNtuple("CR_SS_", resolvedLead, resolvedSubLead, resolvedChannel,
                   true, true, selected_jets, fatjets_resolved, false);
    if (isLLEvent)
        fillNtuple("SR_LL_", llLead, llSubLead, llChannel,
                   true, true, selected_jets, fatjets_resolved, false);
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

// Resolved priority: AK4 jets win, so drop any fatjet overlapping a selected jet.
RVec<FatJet> HNWR_BDT_presel::Clean_Fatjet_with_jets(const RVec<FatJet>& fatjets, const RVec<Jet>& jets) {
    RVec<FatJet> cleanedfatjets;
    for (unsigned int i = 0; i < fatjets.size(); i++) {
        const FatJet& fatjet = fatjets.at(i);
        bool isDRtoJet = false;
        for (unsigned int j = 0; j < jets.size(); j++) {
            if (fatjet.DeltaR(jets.at(j)) < dR_JetFatJet) { isDRtoJet = true; break; }
        }
        if (!isDRtoJet) cleanedfatjets.push_back(fatjet);
    }
    return cleanedfatjets;
}

// Boosted priority: fatjets win, so drop any AK4 jet overlapping a selected fatjet.
RVec<Jet> HNWR_BDT_presel::Clean_jet_with_fatjets(const RVec<Jet>& jets, const RVec<FatJet>& fatjets) {
    RVec<Jet> cleanedjets;
    for (unsigned int i = 0; i < jets.size(); i++) {
        const Jet& jet = jets.at(i);
        bool isDRtoFatJet = false;
        for (unsigned int j = 0; j < fatjets.size(); j++) {
            if (jet.DeltaR(fatjets.at(j)) < dR_JetFatJet) { isDRtoFatJet = true; break; }
        }
        if (!isDRtoFatJet) cleanedjets.push_back(jet);
    }
    return cleanedjets;
}
