#include "BDT_CR.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <functional>
#include <initializer_list>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

BDT_CR::BDT_CR() {}
BDT_CR::~BDT_CR() {}

void BDT_CR::initializeAnalyzer() {
    el_set.AllElectrons.clear();
    mu_set.AllMuons.clear();
    jet_set.AllJets.clear();
    fatjet_set.AllFatJets.clear();

    myCorr = new MyCorrection(DataEra, DataPeriod, IsDATA ? DataStream : MCSample, IsDATA);

    string SKNANO_HOME = getenv("SKNANO_HOME");
    if (IsDATA) {
        systHelper = std::make_unique<SystematicHelper>(SKNANO_HOME + "/docs/DataLRSM.yaml", DataStream, DataEra);
    } else {
        systHelper = std::make_unique<SystematicHelper>(SKNANO_HOME + "/docs/MCLRSM.yaml", MCSample, DataEra);
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

void BDT_CR::executeEvent() {
    el_set.AllElectrons = GetAllElectrons();
    mu_set.AllMuons     = GetAllMuons();
    jet_set.AllJets     = GetAllJets();
    fatjet_set.AllFatJets = GetAllFatJets();

    for (const auto& syst_dummy : *systHelper) {
        (void)syst_dummy;
        const std::string this_syst = systHelper->getCurrentSysName();
        // CR plots use only Central object kinematics plus weight-only up/down systematics.
        // JES/JER object variations are intentionally skipped here.
        if (this_syst.find("JER") != std::string::npos || this_syst.find("JES") != std::string::npos) continue;
        executeEventFromParameter();
    }
}

void BDT_CR::executeEventFromParameter() {
    const TString dir = systHelper->getCurrentSysName();

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

    // --- Cutflow bin 1: noise filter ---
    if (!PassNoiseFilter(jets, ev, Event::MET_Type::PUPPI)) return;
    FillHist(dir + "/Cutflow", 1.0, weight, 10, 0., 10.);

    bool pass_trig_muon = ev.PassTrigger(mu_set.Muon_Trigger);
    bool pass_trig_elec = ev.PassTrigger(el_set.Ele_Trigger);
    if (!pass_trig_muon && !pass_trig_elec) return;


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

    ///         FatJets       ///
    fatjets = Clean_Fatjet_with_tight_leptons(fatjets, Tight_leps);
    jets    = Clean_jet_with_loose_leptons(jets, Loose_leps);

    RVec<FatJet> fatjet_list;
    for (unsigned int i = 0; i < fatjets.size(); i++) {
        FatJet& fj = fatjets.at(i);
        // FatJet preselection for boosted CR: pT/eta/ID only.
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
    if (n_leptons < 1) return;
    FillHist(dir + "/Cutflow", 3.0, weight, 10, 0., 10.);

    Lepton* LeadLep    = nullptr;
    Lepton* SubLeadLep = nullptr;
    int channel = -999;
    bool this_trigger_pass = false;

    ///         Jets       ///
    bool is_jet_veto = AnalyzerCore::PassVetoMap(jets, mu_set.AllMuons, "jetvetomap");
    if (!is_jet_veto) return;
    // --- Cutflow bin 2: jet veto map ---
    FillHist(dir + "/Cutflow", 2.0, weight, 10, 0., 10.);

    RVec<Jet> selected_jets = SelectJets(jets, jet_set.Jet_ID[0], jet_set.Jet_MinPt, jet_set.Jet_MaxEta);
    sort(selected_jets.begin(), selected_jets.end(), PtComparing);

    // CR definitions used for Reproduce-style histograms:
    //
    // Common object/event requirements:
    // - At least one tight lepton after electron/muon eta, pT, and ID selections.
    // - Event trigger must match the leading lepton flavor and trigger-safe pT.
    // - Jets are selected with TIGHTLEPVETO, pT > 40 GeV, and |eta| < 2.4.
    // - Fatjets are selected with pT > 200 GeV, |eta| < 2.4, and Tight ID.
    //
    // Resolved CR topology:
    // - Exactly two tight leptons.
    // - Leading lepton pT > 60 GeV and subleading lepton pT > 53 GeV.
    // - At least two selected AK4 jets.
    // - lep1, lep2, jet1, and jet2 are mutually separated by dR > 0.4.
    // - DY CR: same-flavor pair with 60 < mll < 150 GeV.
    // - Flavor CR: opposite-flavor pair with mll > 200 GeV.
    // - No mlljj/WRMass > 800 cut is applied.
    //
    // Boosted CR topology:
    // - At least one tight leading lepton with pT > 60 GeV.
    // - At least one additional loose lepton after excluding the leading tight lepton.
    // - At least one selected fatjet.
    // - If two AK4 jets exist, the resolved dR requirement must fail; if fewer
    //   than two AK4 jets exist, the event is allowed in boosted CR.
    // - DY CR: first pT-ordered same-flavor extra loose lepton with 60 < mll < 150 GeV.
    // - Flavor CR: first pT-ordered opposite-flavor extra loose lepton with mll > 200 GeV.
    // - No mlljj/WRMass > 800 cut, loose-lepton-in-fatjet requirement, or OF loose veto is applied.

    auto isSameFlavorPair = [](const Lepton* lep1, const Lepton* lep2) {
        if (!lep1 || !lep2) return false;
        return (lep1->IsElectron() && lep2->IsElectron()) || (lep1->IsMuon() && lep2->IsMuon());
    };
    auto isOppositeFlavorPair = [](const Lepton* lep1, const Lepton* lep2) {
        if (!lep1 || !lep2) return false;
        return (lep1->IsElectron() && lep2->IsMuon()) || (lep1->IsMuon() && lep2->IsElectron());
    };
    auto makeLeptonPair = [](const Lepton* lep1, const Lepton* lep2, TLorentzVector& pair) {
        if (!lep1 || !lep2) return false;
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

    const double lowMllMin = 60.0;
    const double lowMllMax = 150.0;
    const double highMllMin = 200.0;

    Lepton* resolvedLead = nullptr;
    Lepton* resolvedSubLead = nullptr;
    bool isResolvedLeptonSelection = false;
    int resolvedChannel = -999;
    bool passResolvedTrigger = false;
    if (n_leptons == 2) {
        resolvedLead = Tight_leps[0];
        resolvedSubLead = Tight_leps[1];
        isResolvedLeptonSelection = (resolvedLead->Pt() > 60.0) && (resolvedSubLead->Pt() > 53.0);
        passResolvedTrigger = isResolvedLeptonSelection && passLeadTrigger(resolvedLead, resolvedChannel);
    }

    TLorentzVector resolvedDilepton;
    const bool hasResolvedDilepton = passResolvedTrigger && makeLeptonPair(resolvedLead, resolvedSubLead, resolvedDilepton);
    const bool resolvedIsSameFlavor = isSameFlavorPair(resolvedLead, resolvedSubLead);
    const bool resolvedIsOppositeFlavor = isOppositeFlavorPair(resolvedLead, resolvedSubLead);
    const bool passResolvedLowMll = hasResolvedDilepton && (resolvedDilepton.M() > lowMllMin) && (resolvedDilepton.M() < lowMllMax);
    const bool passResolvedHighMll = hasResolvedDilepton && (resolvedDilepton.M() > highMllMin);
    bool resolvedDRLeadJetLep = false;
    bool resolvedDRSubLeadJetLep = false;
    bool resolvedDRTwoLeptons = false;
    bool resolvedDRTwoJets = false;
    const bool passResolvedDRCandidate = computeFourObjectDR(resolvedLead, resolvedSubLead, resolvedDRLeadJetLep, resolvedDRSubLeadJetLep, resolvedDRTwoLeptons, resolvedDRTwoJets);
    const bool isResolvedDYCR = passResolvedDRCandidate && resolvedIsSameFlavor && passResolvedLowMll;
    const bool isResolvedFlavCR = passResolvedDRCandidate && resolvedIsOppositeFlavor && passResolvedHighMll;
    bool isResolvedCandidate = isResolvedDYCR || isResolvedFlavCR;

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
    Lepton* boostedDYSubLead = nullptr;
    Lepton* boostedFlavSubLead = nullptr;
    bool hasBoostedDYDilepton = false;
    bool hasBoostedFlavDilepton = false;
    if (boostedLead) {
        for (unsigned int i = 0; i < boostedExtraLooseLeps.size(); i++) {
            TLorentzVector candidateDilepton;
            if (!makeLeptonPair(boostedLead, boostedExtraLooseLeps[i], candidateDilepton)) continue;
            const double candidateMll = candidateDilepton.M();
            if (!hasBoostedDYDilepton &&
                isSameFlavorPair(boostedLead, boostedExtraLooseLeps[i]) &&
                (candidateMll > lowMllMin) && (candidateMll < lowMllMax)) {
                boostedDYSubLead = boostedExtraLooseLeps[i];
                hasBoostedDYDilepton = true;
            }
            if (!hasBoostedFlavDilepton &&
                isOppositeFlavorPair(boostedLead, boostedExtraLooseLeps[i]) &&
                (candidateMll > highMllMin)) {
                boostedFlavSubLead = boostedExtraLooseLeps[i];
                hasBoostedFlavDilepton = true;
            }
            if (hasBoostedDYDilepton && hasBoostedFlavDilepton) break;
        }
    }

    int boostedChannel = -999;
    const bool passBoostedLeptonSelection = boostedLead && (boostedLead->Pt() > 60.0) && (n_boosted_extra_loose_leptons >= 1);
    const bool passBoostedTrigger = passBoostedLeptonSelection && passLeadTrigger(boostedLead, boostedChannel);

    bool boostedDYDRLeadJetLep = false;
    bool boostedDYDRSubLeadJetLep = false;
    bool boostedDYDRTwoLeptons = false;
    bool boostedDYDRTwoJets = false;
    const bool passBoostedDYResolvedDR = computeFourObjectDR(boostedLead, boostedDYSubLead, boostedDYDRLeadJetLep, boostedDYDRSubLeadJetLep, boostedDYDRTwoLeptons, boostedDYDRTwoJets);
    const bool passBoostedDYDRFail = !has2Jets || !passBoostedDYResolvedDR;

    bool boostedFlavDRLeadJetLep = false;
    bool boostedFlavDRSubLeadJetLep = false;
    bool boostedFlavDRTwoLeptons = false;
    bool boostedFlavDRTwoJets = false;
    const bool passBoostedFlavResolvedDR = computeFourObjectDR(boostedLead, boostedFlavSubLead, boostedFlavDRLeadJetLep, boostedFlavDRSubLeadJetLep, boostedFlavDRTwoLeptons, boostedFlavDRTwoJets);
    const bool passBoostedFlavDRFail = !has2Jets || !passBoostedFlavResolvedDR;

    const bool isBoostedDYCR = (!isResolvedCandidate && passBoostedTrigger && hasBoostedDYDilepton && passBoostedDYDRFail && fatjets.size() >= 1);
    const bool isBoostedFlavCR = (!isResolvedCandidate && !isBoostedDYCR && passBoostedTrigger && hasBoostedFlavDilepton && passBoostedFlavDRFail && fatjets.size() >= 1);
    bool isBoostedCandidate = isBoostedDYCR || isBoostedFlavCR;
    if (!(isResolvedCandidate || isBoostedCandidate)) return;

    Lepton* boostedSubLead = isBoostedDYCR ? boostedDYSubLead : (isBoostedFlavCR ? boostedFlavSubLead : nullptr);

    LeadLep = isResolvedCandidate ? resolvedLead : boostedLead;
    SubLeadLep = isResolvedCandidate ? resolvedSubLead : boostedSubLead;
    channel = isResolvedCandidate ? resolvedChannel : boostedChannel;
    this_trigger_pass = isResolvedCandidate ? passResolvedTrigger : passBoostedTrigger;
    if (!LeadLep || !SubLeadLep || !this_trigger_pass) return;

    FillHist(dir + "/LeadFlavorChannel", channel, weight, 2, 0., 2.);

    TLorentzVector selectedDilepton;
    makeLeptonPair(LeadLep, SubLeadLep, selectedDilepton);

    const bool is_Resolved_DY_EE = isResolvedDYCR && LeadLep->IsElectron();
    const bool is_Resolved_DY_MM = isResolvedDYCR && LeadLep->IsMuon();
    const bool is_Resolved_Flav_EM = isResolvedFlavCR;
    const bool is_Boosted_DY_EE = isBoostedDYCR && LeadLep->IsElectron();
    const bool is_Boosted_DY_MM = isBoostedDYCR && LeadLep->IsMuon();
    const bool is_Boosted_Flav_EMJ = isBoostedFlavCR && LeadLep->IsElectron();
    const bool is_Boosted_Flav_MEJ = isBoostedFlavCR && LeadLep->IsMuon();

    RVec<Electron*> selectedElectronsForSF;
    RVec<Muon*> selectedMuonsForSF;
    auto collectLeptonForSF = [&](Lepton* lep) {
        if (!lep) return;
        if (lep->IsElectron()) selectedElectronsForSF.push_back((Electron*)lep);
        if (lep->IsMuon()) selectedMuonsForSF.push_back((Muon*)lep);
    };
    collectLeptonForSF(LeadLep);
    collectLeptonForSF(SubLeadLep);

    std::unordered_map<std::string, std::variant<std::function<float(MyCorrection::variation, TString)>, std::function<float()>>> weight_function_map;
    auto dummy_sf = [](MyCorrection::variation var, TString source) -> float {
        (void)var;
        (void)source;
        return 1.0;
    };

    weight_function_map["PU_Weight"] = [&](MyCorrection::variation var, TString source) -> float {
        (void)source;
        return myCorr->GetPUWeight(ev.nTrueInt(), var);
    };
    weight_function_map["E_Id_Weight"] = [&](MyCorrection::variation var, TString source) -> float {
        (void)source;
        float sf = 1.0;
        for (const auto* el : selectedElectronsForSF) {
            const float absEta = std::fabs(el->Eta());
            float base = 1.0;
            float unc = 0.0;
            if (DataEra == "2023") {
                if (absEta < 1.444) {
                    base = 1.007;
                    unc = 0.004;
                } else if (absEta > 1.566 && absEta < 2.5) {
                    base = 0.988;
                    unc = 0.005;
                }
            } else if (DataEra == "2023BPix") {
                if (absEta < 1.444) {
                    base = 1.009;
                    unc = 0.005;
                } else if (absEta > 1.566 && absEta < 2.5) {
                    base = 0.988;
                    unc = 0.004;
                }
            }

            if (var == MyCorrection::variation::up) base += unc;
            if (var == MyCorrection::variation::down) base -= unc;
            sf *= base;
        }
        return sf;
    };
    weight_function_map["E_Reco_Weight"] = [&](MyCorrection::variation var, TString source) -> float {
        (void)source;
        float sf = 1.0;
        for (const auto* el : selectedElectronsForSF) {
            sf *= myCorr->GetElectronRECOSF(std::fabs(el->Eta()), el->Pt(), el->Phi(), var);
        }
        return sf;
    };
    weight_function_map["E_Trig_Weight"] = [&](MyCorrection::variation var, TString source) -> float {
        (void)source;
        if (!LeadLep->IsElectron() || DataEra == "2017") return 1.0;

        const Electron* leadElectron = (Electron*)LeadLep;
        const float absEta = std::fabs(leadElectron->Eta());
        if (leadElectron->Pt() <= 130.) return 1.0;

        float sf = 1.0;
        float unc = 0.0;
        if (absEta < 1.444) {
            if (DataEra == "2022") {
                sf = 0.995;
                unc = 0.004;
            } else if (DataEra == "2022EE") {
                sf = 0.990;
                unc = 0.007;
            } else if (DataEra == "2023") {
                sf = 0.992;
                unc = 0.006;
            } else if (DataEra == "2023BPix") {
                sf = 0.993;
                unc = 0.001;
            }
        } else if (absEta > 1.566 && absEta < 2.5) {
            if (DataEra == "2022") {
                sf = 0.991;
                unc = 0.009;
            } else if (DataEra == "2022EE") {
                sf = 0.981;
                unc = 0.017;
            } else if (DataEra == "2023") {
                sf = 0.979;
                unc = 0.019;
            } else if (DataEra == "2023BPix") {
                sf = 0.978;
                unc = 0.019;
            }
        }

        if (var == MyCorrection::variation::up) sf += unc;
        if (var == MyCorrection::variation::down) sf -= unc;
        return sf;
    };

    weight_function_map["M_Id_Weight"] = [&](MyCorrection::variation var, TString source) -> float {
        (void)source;
        if (DataEra == "2017") return 1.0;
        float sf = 1.0;
        for (const auto* mu : selectedMuonsForSF) {
            sf *= myCorr->GetMuonIDSF("NUM_HighPtID_DEN_GlobalMuonProbes", *mu, var);
        }
        return sf;
    };
    weight_function_map["M_Reco_Weight"] = [&](MyCorrection::variation var, TString source) -> float {
        (void)source;
        float sf = 1.0;
        for (const auto* mu : selectedMuonsForSF) {
            sf *= myCorr->GetMuonRECOSF(*mu, var);
        }
        return sf;
    };
    weight_function_map["M_Trig_Weight"] = [&](MyCorrection::variation var, TString source) -> float {
        (void)source;
        if (!LeadLep->IsMuon() || DataEra == "2017" || selectedMuonsForSF.empty()) return 1.0;
        RVec<Muon*> trigMuons;
        for (auto* mu : selectedMuonsForSF) trigMuons.push_back(mu);
        return myCorr->GetMuonTriggerSF("NUM_HLT_DEN_HighPtLooseRelIsoProbes", trigMuons, var);
    };
    weight_function_map["M_Iso_Weight"] = [&](MyCorrection::variation var, TString source) -> float {
        (void)source;
        if (DataEra == "2017") return 1.0;
        float sf = 1.0;
        for (const auto* mu : selectedMuonsForSF) {
            sf *= myCorr->GetMuonIDSF("NUM_probe_LooseRelTkIso_DEN_HighPtProbes", *mu, var);
        }
        return sf;
    };
    weight_function_map["JER_Variation"] = dummy_sf;
    weight_function_map["JES_Variation"] = dummy_sf;

    systHelper->assignWeightFunctionMap(weight_function_map);

    std::vector<std::pair<TString, float>> fill_targets;
    if (!IsDATA) {
        const auto weight_map = systHelper->calculateWeight();
        for (const auto& [syst_name, sf_val] : weight_map) {
            if (syst_name.find("JER") != std::string::npos || syst_name.find("JES") != std::string::npos) continue;
            fill_targets.push_back({TString(syst_name.c_str()), weight * sf_val});
        }
    } else {
        fill_targets.push_back({dir, 1.0});
    }

    auto fillResolved = [&](const TString& histDir, float finalWeight, const char* pfx) {
        TLorentzVector jj = selected_jets[0] + selected_jets[1];
        TLorentzVector lljj = selectedDilepton + jj;
        TLorentzVector l1j1j2 = *LeadLep + jj;
        TLorentzVector l2j1j2 = *SubLeadLep + jj;

        FillHist(histDir + "/" + pfx + "_pt", selectedDilepton.Pt(), finalWeight, 100, 0., 1000.);
        FillHist(histDir + "/" + pfx + "_leading_jet_pt", selected_jets[0].Pt(), finalWeight, 200, 0., 2000.);
        FillHist(histDir + "/" + pfx + "_subleading_jet_pt", selected_jets[1].Pt(), finalWeight, 200, 0., 2000.);
        FillHist(histDir + "/" + pfx + "_mlljj", lljj.M(), finalWeight, 800, 0., 8000.);
        FillHist(histDir + "/" + pfx + "_leading_lep_pt", LeadLep->Pt(), finalWeight, 2000, 0., 2000.);
        FillHist(histDir + "/" + pfx + "_subleading_lep_pt", SubLeadLep->Pt(), finalWeight, 2000, 0., 2000.);
        FillHist(histDir + "/" + pfx + "_mass", selectedDilepton.M(), finalWeight, 100, 0., 1000.);
        FillHist(histDir + "/" + pfx + "_eta", selectedDilepton.Eta(), finalWeight, 100, -5., 5.);
        FillHist(histDir + "/" + pfx + "_phi", selectedDilepton.Phi(), finalWeight, 100, -3.14, 3.14);
        FillHist(histDir + "/" + pfx + "_leading_jet_eta", selected_jets[0].Eta(), finalWeight, 100, -2.5, 2.5);
        FillHist(histDir + "/" + pfx + "_leading_jet_phi", selected_jets[0].Phi(), finalWeight, 100, -3.14, 3.14);
        FillHist(histDir + "/" + pfx + "_subleading_jet_eta", selected_jets[1].Eta(), finalWeight, 100, -2.5, 2.5);
        FillHist(histDir + "/" + pfx + "_subleading_jet_phi", selected_jets[1].Phi(), finalWeight, 100, -3.14, 3.14);
        FillHist(histDir + "/" + pfx + "_j1j2_mass", jj.M(), finalWeight, 100, 0., 1000.);
        FillHist(histDir + "/" + pfx + "_j1j2_pt", jj.Pt(), finalWeight, 100, 0., 1000.);
        FillHist(histDir + "/" + pfx + "_j1j2_eta", jj.Eta(), finalWeight, 100, -5., 5.);
        FillHist(histDir + "/" + pfx + "_j1j2_phi", jj.Phi(), finalWeight, 100, -3.14, 3.14);
        FillHist(histDir + "/" + pfx + "_l1j1j2_mass", l1j1j2.M(), finalWeight, 200, 0., 2000.);
        FillHist(histDir + "/" + pfx + "_l1j1j2_pt", l1j1j2.Pt(), finalWeight, 200, 0., 2000.);
        FillHist(histDir + "/" + pfx + "_l1j1j2_eta", l1j1j2.Eta(), finalWeight, 100, -5., 5.);
        FillHist(histDir + "/" + pfx + "_l1j1j2_phi", l1j1j2.Phi(), finalWeight, 100, -3.14, 3.14);
        FillHist(histDir + "/" + pfx + "_l2j1j2_mass", l2j1j2.M(), finalWeight, 200, 0., 2000.);
        FillHist(histDir + "/" + pfx + "_l2j1j2_pt", l2j1j2.Pt(), finalWeight, 200, 0., 2000.);
        FillHist(histDir + "/" + pfx + "_l2j1j2_eta", l2j1j2.Eta(), finalWeight, 100, -5., 5.);
        FillHist(histDir + "/" + pfx + "_l2j1j2_phi", l2j1j2.Phi(), finalWeight, 100, -3.14, 3.14);
        FillHist(histDir + "/" + pfx + "_mlljj_pt", lljj.Pt(), finalWeight, 200, 0., 2000.);
        FillHist(histDir + "/" + pfx + "_mlljj_eta", lljj.Eta(), finalWeight, 100, -5., 5.);
        FillHist(histDir + "/" + pfx + "_mlljj_phi", lljj.Phi(), finalWeight, 100, -3.14, 3.14);
        FillHist(histDir + "/" + pfx + "_leading_lep_eta", LeadLep->Eta(), finalWeight, 100, -2.5, 2.5);
        FillHist(histDir + "/" + pfx + "_leading_lep_phi", LeadLep->Phi(), finalWeight, 100, -3.14, 3.14);
        FillHist(histDir + "/" + pfx + "_subleading_lep_eta", SubLeadLep->Eta(), finalWeight, 100, -2.5, 2.5);
        FillHist(histDir + "/" + pfx + "_subleading_lep_phi", SubLeadLep->Phi(), finalWeight, 100, -3.14, 3.14);
        FillHist(histDir + "/" + pfx + "_jetnum", static_cast<float>(selected_jets.size()), finalWeight, 20, 0., 20.);
        FillHist(histDir + "/" + pfx + "_punum", ev.nTrueInt(), finalWeight, 80, 0., 80.);
        FillHist(histDir + "/" + pfx + "_pv", ev.nPV(), finalWeight, 80, 0., 80.);
        FillHist(histDir + "/" + pfx + "_pvgood", ev.nPVsGood(), finalWeight, 80, 0., 80.);
    };

    auto fillBoostedDY = [&](const TString& histDir, float finalWeight, const char* pfx) {
        const FatJet& HNFatJet = fatjets[0];
        TLorentzVector ncand = HNFatJet;
        if (HNFatJet.DeltaR(*SubLeadLep) >= 0.8) ncand += *SubLeadLep;
        TLorentzVector wr = *LeadLep + ncand;

        FillHist(histDir + "/" + pfx + "_ll_pt", selectedDilepton.Pt(), finalWeight, 1000, 0., 1000.);
        FillHist(histDir + "/" + pfx + "_leading_fatjet_pt", HNFatJet.Pt(), finalWeight, 2000, 0., 2000.);
        FillHist(histDir + "/" + pfx + "_mlljj", wr.M(), finalWeight, 8000, 0., 8000.);
        FillHist(histDir + "/" + pfx + "_leading_lep_pt", LeadLep->Pt(), finalWeight, 2000, 0., 2000.);
        FillHist(histDir + "/" + pfx + "_subleading_lep_pt", SubLeadLep->Pt(), finalWeight, 2000, 0., 2000.);
        FillHist(histDir + "/" + pfx + "_mass", selectedDilepton.M(), finalWeight, 100, 0., 1000.);
        FillHist(histDir + "/" + pfx + "_eta", selectedDilepton.Eta(), finalWeight, 100, -5., 5.);
        FillHist(histDir + "/" + pfx + "_phi", selectedDilepton.Phi(), finalWeight, 100, -3.14, 3.14);
        FillHist(histDir + "/" + pfx + "_fatjet_eta", HNFatJet.Eta(), finalWeight, 100, -2.4, 2.4);
        FillHist(histDir + "/" + pfx + "_fatjet_phi", HNFatJet.Phi(), finalWeight, 100, -3.14, 3.14);
        FillHist(histDir + "/" + pfx + "_fatjet_SDM", HNFatJet.SDMass(), finalWeight, 100, 0., 200.);
        FillHist(histDir + "/" + pfx + "_mll", selectedDilepton.M(), finalWeight, 100, 0., 1000.);
        FillHist(histDir + "/" + pfx + "_WR_pt", wr.Pt(), finalWeight, 200, 0., 2000.);
        FillHist(histDir + "/" + pfx + "_WR_eta", wr.Eta(), finalWeight, 100, -5., 5.);
        FillHist(histDir + "/" + pfx + "_WR_phi", wr.Phi(), finalWeight, 100, -3.14, 3.14);
        FillHist(histDir + "/" + pfx + "_leading_lep_eta", LeadLep->Eta(), finalWeight, 100, -2.5, 2.5);
        FillHist(histDir + "/" + pfx + "_leading_lep_phi", LeadLep->Phi(), finalWeight, 100, -3.14, 3.14);
        FillHist(histDir + "/" + pfx + "_subleading_lep_eta", SubLeadLep->Eta(), finalWeight, 100, -2.5, 2.5);
        FillHist(histDir + "/" + pfx + "_subleading_lep_phi", SubLeadLep->Phi(), finalWeight, 100, -3.14, 3.14);
        FillHist(histDir + "/" + pfx + "_fatjet_lsf3", HNFatJet.LSF3(), finalWeight, 100, 0., 1.);
        FillHist(histDir + "/" + pfx + "_deltaR_leadlep_fatjet", LeadLep->DeltaR(HNFatJet), finalWeight, 100, 0., 5.);
        FillHist(histDir + "/" + pfx + "_dphi_leadlep_fatjet", std::fabs(LeadLep->DeltaPhi(HNFatJet)), finalWeight, 100, 0., 3.14);
        FillHist(histDir + "/" + pfx + "_punum", ev.nTrueInt(), finalWeight, 80, 0., 80.);
        FillHist(histDir + "/" + pfx + "_jetnum", static_cast<float>(selected_jets.size()), finalWeight, 20, 0., 20.);
        FillHist(histDir + "/" + pfx + "_pv", ev.nPV(), finalWeight, 80, 0., 80.);
        FillHist(histDir + "/" + pfx + "_pvgood", ev.nPVsGood(), finalWeight, 80, 0., 80.);
    };

    auto fillBoostedFlav = [&](const TString& histDir, float finalWeight, const char* pfx) {
        const FatJet& HNFatJet = fatjets[0];
        TLorentzVector wr = *LeadLep + HNFatJet;

        FillHist(histDir + "/" + pfx + "_ll_pt", selectedDilepton.Pt(), finalWeight, 1000, 0., 1000.);
        FillHist(histDir + "/" + pfx + "_leading_fatjet_pt", HNFatJet.Pt(), finalWeight, 2000, 0., 2000.);
        FillHist(histDir + "/" + pfx + "_mlljj", wr.M(), finalWeight, 8000, 0., 8000.);
        FillHist(histDir + "/" + pfx + "_fatjet_SDM", HNFatJet.SDMass(), finalWeight, 10000, 0., 10000.);
        FillHist(histDir + "/" + pfx + "_leading_lep_pt", LeadLep->Pt(), finalWeight, 2000, 0., 2000.);
        FillHist(histDir + "/" + pfx + "_subleading_lep_pt", SubLeadLep->Pt(), finalWeight, 2000, 0., 2000.);
        FillHist(histDir + "/" + pfx + "_leading_lep_eta", LeadLep->Eta(), finalWeight, 100, -2.5, 2.5);
        FillHist(histDir + "/" + pfx + "_subleading_lep_eta", SubLeadLep->Eta(), finalWeight, 100, -2.5, 2.5);
        FillHist(histDir + "/" + pfx + "_fatjet_eta", HNFatJet.Eta(), finalWeight, 100, -2.5, 2.5);
        FillHist(histDir + "/" + pfx + "_fatjet_phi", HNFatJet.Phi(), finalWeight, 100, -3.14, 3.14);
        FillHist(histDir + "/" + pfx + "_deltaR_leadlep_fatjet", LeadLep->DeltaR(HNFatJet), finalWeight, 100, 0., 5.);
        FillHist(histDir + "/" + pfx + "_leadlep_lsf", HNFatJet.LSF3(), finalWeight, 100, 0., 1.);
        FillHist(histDir + "/" + pfx + "_mll", selectedDilepton.M(), finalWeight, 1000, 0., 1000.);
        FillHist(histDir + "/" + pfx + "_fatjet_pt", HNFatJet.Pt(), finalWeight, 1000, 0., 1000.);
        FillHist(histDir + "/" + pfx + "_WR_pt", wr.Pt(), finalWeight, 1000, 0., 1000.);
        FillHist(histDir + "/" + pfx + "_mass", selectedDilepton.M(), finalWeight, 100, 0., 2000.);
        FillHist(histDir + "/" + pfx + "_eta", selectedDilepton.Eta(), finalWeight, 100, -5., 5.);
        FillHist(histDir + "/" + pfx + "_phi", selectedDilepton.Phi(), finalWeight, 100, -3.14, 3.14);
        FillHist(histDir + "/" + pfx + "_WR_eta", wr.Eta(), finalWeight, 100, -5., 5.);
        FillHist(histDir + "/" + pfx + "_WR_phi", wr.Phi(), finalWeight, 100, -3.14, 3.14);
        FillHist(histDir + "/" + pfx + "_leading_lep_phi", LeadLep->Phi(), finalWeight, 100, -3.14, 3.14);
        FillHist(histDir + "/" + pfx + "_subleading_lep_phi", SubLeadLep->Phi(), finalWeight, 100, -3.14, 3.14);
        FillHist(histDir + "/" + pfx + "_fatjet_lsf3", HNFatJet.LSF3(), finalWeight, 100, 0., 1.);
        FillHist(histDir + "/" + pfx + "_dphi_leadlep_fatjet", std::fabs(LeadLep->DeltaPhi(HNFatJet)), finalWeight, 100, 0., 3.14);
        FillHist(histDir + "/" + pfx + "_punum", ev.nTrueInt(), finalWeight, 80, 0., 80.);
        FillHist(histDir + "/" + pfx + "_pv", ev.nPV(), finalWeight, 80, 0., 80.);
        FillHist(histDir + "/" + pfx + "_pvgood", ev.nPVsGood(), finalWeight, 80, 0., 80.);
        FillHist(histDir + "/" + pfx + "_jetnum", static_cast<float>(selected_jets.size()), finalWeight, 20, 0., 20.);
    };

    for (const auto& [histDir, finalWeight] : fill_targets) {
        for (auto [cond, pfx] : std::initializer_list<std::pair<bool, const char*>>{
                 {is_Resolved_DY_EE, "DYCR_Resolved_EE"},
                 {is_Resolved_DY_MM, "DYCR_Resolved_MM"},
                 {is_Resolved_Flav_EM, "FlavCR_Resolved_EM"},
             }) {
            if (cond) fillResolved(histDir, finalWeight, pfx);
        }

        for (auto [cond, pfx] : std::initializer_list<std::pair<bool, const char*>>{
                 {is_Boosted_DY_EE, "DYCR_Boosted_EE"},
                 {is_Boosted_DY_MM, "DYCR_Boosted_MM"},
             }) {
            if (cond) fillBoostedDY(histDir, finalWeight, pfx);
        }

        for (auto [cond, pfx] : std::initializer_list<std::pair<bool, const char*>>{
                 {is_Boosted_Flav_EMJ, "FlavCR_Boosted_EMJ"},
                 {is_Boosted_Flav_MEJ, "FlavCR_Boosted_MEJ"},
             }) {
            if (cond) fillBoostedFlav(histDir, finalWeight, pfx);
        }

        FillHist(histDir + "/LeadLep_pt", LeadLep->Pt(), finalWeight, 100, 0., 1000.);
        FillHist(histDir + "/SubLeadLep_pt", SubLeadLep->Pt(), finalWeight, 100, 0., 1000.);
        FillHist(histDir + "/N_Jet", selected_jets.size(), finalWeight, 10, 0., 10.);
        FillHist(histDir + "/N_FatJet", fatjets.size(), finalWeight, 10, 0., 10.);
        FillHist(histDir + "/Mll", selectedDilepton.M(), finalWeight, 200, 0., 2000.);
    }
}

//================ Helper functions ================//

bool BDT_CR::Electrons::isPassCustomTightID(const Electron& el, const BDT_CR::Electrons& eset) const {
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

bool BDT_CR::Electrons::isPassCustomLooseID(const Electron& el) const {
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

bool BDT_CR::Electrons::isPassLooseNoIso(const Electron& el) const {
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

RVec<FatJet> BDT_CR::Clean_Fatjet_with_tight_leptons(const RVec<FatJet>& fatjets, const RVec<Lepton*>& tight_leps) {
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

RVec<Jet> BDT_CR::Clean_jet_with_loose_leptons(const RVec<Jet>& jets, const RVec<Lepton*>& loose_leps) {
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
