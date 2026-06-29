#include "atlas.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

#include "TVector3.h"

atlas::atlas()
    : btagger(JetTagging::JetFlavTagger::ParticleNet),
      btag_wp_name(JetTagging::JetFlavTaggerWP::Medium),
      btag_wp(0.) {}

atlas::~atlas() {}

void atlas::initializeAnalyzer() {
    AllElectrons.clear();
    AllMuons.clear();
    AllJets.clear();
    AllFatJets.clear();

    myCorr = new MyCorrection(DataEra, DataPeriod, IsDATA ? DataStream : MCSample, IsDATA);

    btagger = (Run == 2) ? JetTagging::JetFlavTagger::DeepJet : JetTagging::JetFlavTagger::ParticleNet;
    btag_wp_name = JetTagging::JetFlavTaggerWP::Medium;
    btag_wp = myCorr->GetBTaggingWP(btagger, btag_wp_name);

    setupTriggers();

    const char* sknanoHome = std::getenv("SKNANO_HOME");
    const string sknanoHomeStr = sknanoHome ? string(sknanoHome) : string(".");
    if (IsDATA) {
        systHelper = std::make_unique<SystematicHelper>(sknanoHomeStr + "/docs/noSyst.yaml", DataStream, DataEra);
    } else {
        systHelper = std::make_unique<SystematicHelper>(sknanoHomeStr + "/docs/ExampleSystematic.yaml", MCSample, DataEra);
    }
}

void atlas::executeEvent() {
    AllElectrons = GetAllElectrons();
    AllMuons = GetAllMuons();
    AllJets = GetAllJets();
    AllFatJets = GetAllFatJets();

    for (const auto& syst_dummy : *systHelper) {
        (void)syst_dummy;
        executeEventFromParameter();
    }
}

void atlas::executeEventFromParameter() {
    const TString this_syst = systHelper->getCurrentSysName();
    if (this_syst != "Central") return;

    Event ev = GetEvent();
    Particle met = ev.GetMETVector(Event::MET_Type::PUPPI, Event::MET_Syst::CENTRAL);
    float weight = eventWeight(ev, this_syst);

    FillHist(this_syst + "/Cutflow/Event", 1., weight, 10, 0., 10.);
    if (!PassNoiseFilter(AllJets, ev, Event::MET_Type::PUPPI)) return;
    FillHist(this_syst + "/Cutflow/Event", 2., weight, 10, 0., 10.);

    fillResolved(ev, met, weight, this_syst);
    fillBoosted(ev, met, weight, this_syst);
}

void atlas::setupTriggers() {
    triggers.ResolvedEE.clear();
    triggers.ResolvedMM.clear();
    triggers.ResolvedEM.clear();
    triggers.BoostedElectron.clear();
    triggers.BoostedMuon.clear();

    RVec<TString> electron_triggers;
    RVec<TString> muon_triggers;

    if (DataEra == "2017") {
        electron_triggers = {"HLT_Ele35_WPTight_Gsf", "HLT_Photon200", "HLT_Ele115_CaloIdVT_GsfTrkIdT"};
        muon_triggers = {"HLT_Mu50", "HLT_OldMu100", "HLT_TkMu100"};
    } else if (Run == 2) {
        electron_triggers = {"HLT_Photon200", "HLT_Ele115_CaloIdVT_GsfTrkIdT"};
        muon_triggers = {"HLT_Mu50", "HLT_OldMu100", "HLT_TkMu100"};
    } else {
        electron_triggers = {"HLT_Photon200", "HLT_Ele115_CaloIdVT_GsfTrkIdT"};
        muon_triggers = {"HLT_Mu50", "HLT_CascadeMu100", "HLT_HighPtTkMu100"};
    }

    triggers.ResolvedEE = electron_triggers;
    triggers.ResolvedMM = muon_triggers;
    triggers.ResolvedEM = muon_triggers;
    triggers.BoostedElectron = electron_triggers;
    triggers.BoostedMuon = muon_triggers;
}

float atlas::eventWeight(Event& ev, const TString& syst) {
    float weight = 1.;
    if (!IsDATA) {
        weight *= MCweight();
        weight *= ev.GetTriggerLumi("HLT_Mu50");
        weight *= myCorr->GetPUWeight(ev.nTrueInt(), MyCorrection::variation::nom, syst);
    }
    return weight;
}

bool atlas::passAtlasElectronEta(const Electron& electron) const {
    const float abs_sc_eta = std::fabs(electron.scEta());
    if (!(abs_sc_eta < cuts.electron_eta)) return false;
    if (abs_sc_eta > 1.37 && abs_sc_eta < 1.52) return false;
    return true;
}

bool atlas::passResolvedElectron(const Electron& electron) const {
    if (!(electron.Pt() > cuts.resolved_electron_pt)) return false;
    if (!passAtlasElectronEta(electron)) return false;
    if (!electron.PassID(Electron::ElectronID::POG_TIGHT)) return false;
    return true;
}

bool atlas::passBoostedElectron(const Electron& electron) const {
    if (!(electron.Pt() > cuts.boosted_electron_pt)) return false;
    if (!passAtlasElectronEta(electron)) return false;
    if (!electron.PassID(Electron::ElectronID::POG_MEDIUM)) return false;
    return true;
}

bool atlas::passBoostedLeadingElectron(const Electron& electron) const {
    if (!(electron.Pt() > cuts.boosted_leading_lepton_pt)) return false;
    if (!passAtlasElectronEta(electron)) return false;
    if (!(electron.PassID(Electron::ElectronID::POG_HEEP) ||
          electron.PassID(Electron::ElectronID::POG_TIGHT))) return false;
    return true;
}

bool atlas::passResolvedMuon(const Muon& muon) const {
    if (!(muon.Pt() > cuts.resolved_muon_pt)) return false;
    if (!(std::fabs(muon.Eta()) < cuts.muon_eta)) return false;
    if (!muon.PassID(Muon::MuonID::POG_MEDIUM)) return false;
    if (muon.Pt() > 300. && !muon.PassID(Muon::MuonID::POG_GLOBAL_HIGH_PT)) return false;
    if (!muon.PassID(Muon::MuonID::POG_TKISO_TIGHT)) return false;
    return true;
}

bool atlas::passBoostedMuon(const Muon& muon) const {
    if (!(muon.Pt() > cuts.boosted_muon_pt)) return false;
    if (!(std::fabs(muon.Eta()) < cuts.muon_eta)) return false;
    if (!muon.PassID(Muon::MuonID::POG_MEDIUM)) return false;
    return true;
}

bool atlas::passBoostedLeadingMuon(const Muon& muon) const {
    if (!(muon.Pt() > cuts.boosted_leading_lepton_pt)) return false;
    if (!(std::fabs(muon.Eta()) < cuts.muon_eta)) return false;
    if (!muon.PassID(Muon::MuonID::POG_TIGHT)) return false;
    if (!muon.PassID(Muon::MuonID::POG_TKISO_TIGHT)) return false;
    return true;
}

RVec<Electron> atlas::selectResolvedElectrons(const RVec<Electron>& electrons) const {
    RVec<Electron> selected;
    for (const auto& electron : electrons) {
        if (passResolvedElectron(electron)) selected.push_back(electron);
    }
    sort(selected.begin(), selected.end(), PtComparing);
    return selected;
}

RVec<Electron> atlas::selectBoostedElectrons(const RVec<Electron>& electrons) const {
    RVec<Electron> selected;
    for (const auto& electron : electrons) {
        if (passBoostedElectron(electron)) selected.push_back(electron);
    }
    sort(selected.begin(), selected.end(), PtComparing);
    return selected;
}

RVec<Muon> atlas::selectResolvedMuons(const RVec<Muon>& muons) const {
    RVec<Muon> selected;
    for (const auto& muon : muons) {
        if (passResolvedMuon(muon)) selected.push_back(muon);
    }
    sort(selected.begin(), selected.end(), PtComparing);
    return selected;
}

RVec<Muon> atlas::selectBoostedMuons(const RVec<Muon>& muons) const {
    RVec<Muon> selected;
    for (const auto& muon : muons) {
        if (passBoostedMuon(muon)) selected.push_back(muon);
    }
    sort(selected.begin(), selected.end(), PtComparing);
    return selected;
}

RVec<Jet> atlas::selectSmallRJets(const RVec<Jet>& jets, float ptmin) const {
    RVec<Jet> selected;
    for (const auto& jet : jets) {
        if (!(jet.Pt() > ptmin)) continue;
        if (!(std::fabs(jet.Eta()) < cuts.smallr_jet_eta)) continue;
        if (!jet.PassID(Jet::JetID::TIGHTLEPVETO)) continue;
        selected.push_back(jet);
    }
    sort(selected.begin(), selected.end(), PtComparing);
    return selected;
}

RVec<FatJet> atlas::selectLargeRJets(const RVec<FatJet>& fatjets) const {
    RVec<FatJet> selected;
    for (const auto& fatjet : fatjets) {
        if (!(fatjet.Pt() > cuts.boosted_large_r_jet_pt)) continue;
        if (!(std::fabs(fatjet.Eta()) < cuts.boosted_large_r_jet_eta)) continue;
        if (!fatjet.PassID("Tight")) continue;
        selected.push_back(fatjet);
    }
    sort(selected.begin(), selected.end(), PtComparing);
    return selected;
}

RVec<Lepton*> atlas::makeLeptonPtrs(RVec<Electron>& electrons, RVec<Muon>& muons) const {
    RVec<Lepton*> leptons;
    for (auto& electron : electrons) leptons.push_back(&electron);
    for (auto& muon : muons) leptons.push_back(&muon);
    sort(leptons.begin(), leptons.end(), PtComparingPtr);
    return leptons;
}

RVec<Jet> atlas::cleanJetsWithLeptons(const RVec<Jet>& jets, const RVec<Lepton*>& leptons, float dr) const {
    RVec<Jet> cleaned;
    for (const auto& jet : jets) {
        bool overlaps = false;
        for (const auto& lepton : leptons) {
            if (jet.DeltaR(*lepton) < dr) {
                overlaps = true;
                break;
            }
        }
        if (!overlaps) cleaned.push_back(jet);
    }
    return cleaned;
}

int atlas::countBTaggedJets(const RVec<Jet>& jets) const {
    int n_btag = 0;
    for (const auto& jet : jets) {
        if (isBTagged(jet)) ++n_btag;
    }
    return n_btag;
}

bool atlas::isBTagged(const Jet& jet) const {
    return jet.GetBTaggerResult(btagger) > btag_wp;
}

bool atlas::passResolvedTrigger(const Event& ev, const TString& channel) const {
    if (channel == "EE") return ev.PassTrigger(triggers.ResolvedEE);
    if (channel == "MM") return ev.PassTrigger(triggers.ResolvedMM);
    if (channel == "EM") return ev.PassTrigger(triggers.ResolvedEM);
    return false;
}

bool atlas::passBoostedTrigger(const Event& ev, const Lepton& leading_lepton) const {
    if (leading_lepton.IsElectron()) return ev.PassTrigger(triggers.BoostedElectron);
    if (leading_lepton.IsMuon()) return ev.PassTrigger(triggers.BoostedMuon);
    return false;
}

float atlas::calculateCosTheta(const Lepton& lepton, const Particle& met) const {
    constexpr double mW = 80.4;
    const double px_l = lepton.Px();
    const double py_l = lepton.Py();
    const double pz_l = lepton.Pz();
    const double e_l = lepton.E();
    const double px_nu = met.Px();
    const double py_nu = met.Py();
    const double met_pt = met.Pt();
    const double mu = 0.5 * mW * mW + px_l * px_nu + py_l * py_nu;
    const double a = e_l * e_l - pz_l * pz_l;
    const double b = -2. * mu * pz_l;
    const double c = e_l * e_l * met_pt * met_pt - mu * mu;

    double pz_nu = 0.;
    if (std::fabs(a) > 1.e-6) {
        const double discriminant = b * b - 4. * a * c;
        if (discriminant >= 0.) {
            const double sqrt_disc = std::sqrt(discriminant);
            const double sol1 = (-b + sqrt_disc) / (2. * a);
            const double sol2 = (-b - sqrt_disc) / (2. * a);
            pz_nu = (std::fabs(sol1) < std::fabs(sol2)) ? sol1 : sol2;
        } else {
            pz_nu = -b / (2. * a);
        }
    }

    TLorentzVector neutrino;
    neutrino.SetPxPyPzE(px_nu, py_nu, pz_nu, std::sqrt(px_nu * px_nu + py_nu * py_nu + pz_nu * pz_nu));
    TLorentzVector w_boson = lepton + neutrino;
    TVector3 boost = w_boson.BoostVector();
    if (boost.Mag() <= 0.) return 0.;

    TLorentzVector lepton_in_w = lepton;
    lepton_in_w.Boost(-boost);
    return lepton_in_w.Vect().Unit().Dot(boost.Unit());
}

TString atlas::chargeCategory(const Lepton& lep1, const Lepton& lep2) const {
    return (lep1.Charge() * lep2.Charge() > 0.) ? "SS" : "OS";
}

TString atlas::resolvedChannelName(const TString& flavor, const TString& charge) const {
    if (charge == "SS") return TString("rSRSS2") + flavor;
    return TString("rSROS2") + flavor;
}

void atlas::fillCutflowBin(const TString& histname, int step, float weight, int nbins) {
    FillHist(histname, step, weight, nbins, 0., nbins);
    FillHist(histname + "_raw", step, 1., nbins, 0., nbins);
}

void atlas::fillResolved(const Event& ev, const Particle& met, float weight, const TString& syst) {
    (void)met;
    FillHist(syst + "/Resolved/Cutflow_all", 1., weight, 12, 0., 12.);
    const RVec<TString> resolved_sr_cutflows = {"rSROS2e", "rSROS2mu", "rSRSS2e", "rSRSS2mu"};
    for (const auto& region : resolved_sr_cutflows) {
        fillCutflowBin(syst + "/Resolved/Cutflow/" + region, 1, weight, 12);
    }

    RVec<Electron> electrons = selectResolvedElectrons(AllElectrons);
    RVec<Muon> muons = selectResolvedMuons(AllMuons);
    RVec<Lepton*> leptons_for_cleaning = makeLeptonPtrs(electrons, muons);
    RVec<Jet> cleaned_jets = cleanJetsWithLeptons(AllJets, leptons_for_cleaning, cuts.overlap_dr);
    RVec<Jet> smallr_jets_for_bveto = selectSmallRJets(cleaned_jets, cuts.resolved_smallr_jet_pt);
    RVec<Jet> signal_jets = selectSmallRJets(cleaned_jets, cuts.resolved_jet_pt);

    FillHist(syst + "/Resolved/N_electrons", electrons.size(), weight, 8, 0., 8.);
    FillHist(syst + "/Resolved/N_muons", muons.size(), weight, 8, 0., 8.);
    FillHist(syst + "/Resolved/N_smallR_jets_pt20", smallr_jets_for_bveto.size(), weight, 12, 0., 12.);
    FillHist(syst + "/Resolved/N_smallR_jets_pt100", signal_jets.size(), weight, 12, 0., 12.);
    FillHist(syst + "/Resolved/N_btag_jets", countBTaggedJets(smallr_jets_for_bveto), weight, 8, 0., 8.);

    auto process_candidate = [&](const TString& channel, const TString& flavor, Lepton& lep1, Lepton& lep2) {
        const TString charge = chargeCategory(lep1, lep2);
        const bool is_same_flavor_sr_channel = (channel == "EE" || channel == "MM");
        const TString sr_region = is_same_flavor_sr_channel ? resolvedChannelName(flavor, charge) : TString("rCROSemu");
        auto fill_sr_cutflow = [&](int step) {
            if (is_same_flavor_sr_channel) fillCutflowBin(syst + "/Resolved/Cutflow/" + sr_region, step, weight, 12);
        };

        FillHist(syst + "/Resolved/Cutflow_all", 2., weight, 12, 0., 12.);
        fill_sr_cutflow(2); // exact lepton flavor and charge category
        if (!(lep1.Pt() > cuts.resolved_leading_lepton_pt)) return;
        FillHist(syst + "/Resolved/Cutflow_all", 3., weight, 12, 0., 12.);
        fill_sr_cutflow(3); // leading lepton pT
        if (!passResolvedTrigger(ev, channel)) return;
        FillHist(syst + "/Resolved/Cutflow_all", 4., weight, 12, 0., 12.);
        fill_sr_cutflow(4); // trigger
        if (signal_jets.size() < 2) return;
        FillHist(syst + "/Resolved/Cutflow_all", 5., weight, 12, 0., 12.);
        fill_sr_cutflow(5); // at least two pT>100 small-R jets
        if (countBTaggedJets(smallr_jets_for_bveto) != 0) return;
        FillHist(syst + "/Resolved/Cutflow_all", 6., weight, 12, 0., 12.);
        fill_sr_cutflow(6); // b-veto

        const double mll = (lep1 + lep2).M();
        const double mjj = (signal_jets[0] + signal_jets[1]).M();
        const double ht = lep1.Pt() + lep2.Pt() + signal_jets[0].Pt() + signal_jets[1].Pt();

        FillHist(syst + "/Resolved/Base/mll", mll, weight, 100, 0., 1000.);
        FillHist(syst + "/Resolved/Base/mjj", mjj, weight, 100, 0., 2000.);
        FillHist(syst + "/Resolved/Base/HT", ht, weight, 100, 0., 3000.);

        if (charge == "OS") {
            if (channel == "EM") {
                if (mll > cuts.resolved_mll_sr && mjj > cuts.resolved_mjj && ht > cuts.resolved_ht) {
                    fillResolvedCandidate("rCROSemu", lep1, lep2, signal_jets[0], signal_jets[1], weight, syst);
                }
                return;
            }

            if (mll > cuts.resolved_mll_sr) {
                FillHist(syst + "/Resolved/Cutflow_all", 7., weight, 12, 0., 12.);
                fill_sr_cutflow(7); // SR mll
                fill_sr_cutflow(8); // SS dR requirement, no-op for OS
                if (mjj > cuts.resolved_mjj) {
                    FillHist(syst + "/Resolved/Cutflow_all", 8., weight, 12, 0., 12.);
                    fill_sr_cutflow(9); // mjj
                    if (ht > cuts.resolved_ht) {
                        FillHist(syst + "/Resolved/Cutflow_all", 9., weight, 12, 0., 12.);
                        fill_sr_cutflow(10); // HT
                        fill_sr_cutflow(11); // final SR
                        fillResolvedCandidate(resolvedChannelName(flavor, charge), lep1, lep2, signal_jets[0], signal_jets[1], weight, syst);
                    }
                }
            } else if (mll > 60. && mll < 110. && mjj > cuts.resolved_mjj && ht > cuts.resolved_ht) {
                fillResolvedCandidate(TString("rCROS2") + flavor, lep1, lep2, signal_jets[0], signal_jets[1], weight, syst);
            } else if (mll > 110. && mll < cuts.resolved_mll_sr && mjj > cuts.resolved_mjj && ht > cuts.resolved_ht) {
                fillResolvedCandidate(TString("rVROS2") + flavor, lep1, lep2, signal_jets[0], signal_jets[1], weight, syst);
            }
            return;
        }

        if (channel == "EM") return;
        if (mll > cuts.resolved_mll_sr) {
            FillHist(syst + "/Resolved/Cutflow_all", 7., weight, 12, 0., 12.);
            fill_sr_cutflow(7); // SR mll
            if (!(lep1.DeltaR(lep2) < cuts.resolved_ss_max_drll)) return;
            fill_sr_cutflow(8); // SS dRll
            if (mjj > cuts.resolved_mjj) {
                FillHist(syst + "/Resolved/Cutflow_all", 8., weight, 12, 0., 12.);
                fill_sr_cutflow(9); // mjj
                if (ht > cuts.resolved_ht) {
                    FillHist(syst + "/Resolved/Cutflow_all", 9., weight, 12, 0., 12.);
                    fill_sr_cutflow(10); // HT
                    fill_sr_cutflow(11); // final SR
                    fillResolvedCandidate(resolvedChannelName(flavor, charge), lep1, lep2, signal_jets[0], signal_jets[1], weight, syst);
                }
            }
        }

        const bool ee_crss_mass = (flavor == "e" && mll > 110. && mll < 300.);
        const bool mm_crss_mass = (flavor == "mu" && mll > 60. && mll < 300.);
        if (ee_crss_mass || mm_crss_mass) {
            fillResolvedCandidate(TString("rCRSS2") + flavor, lep1, lep2, signal_jets[0], signal_jets[1], weight, syst);
        } else if (mll > 300. && mll < cuts.resolved_mll_sr) {
            fillResolvedCandidate(TString("rVRSS2") + flavor, lep1, lep2, signal_jets[0], signal_jets[1], weight, syst);
        }
    };

    if (electrons.size() == 2 && muons.empty()) {
        process_candidate("EE", "e", electrons[0], electrons[1]);
    }
    if (muons.size() == 2 && electrons.empty()) {
        process_candidate("MM", "mu", muons[0], muons[1]);
    }
    if (electrons.size() == 1 && muons.size() == 1) {
        Lepton* lep1 = &electrons[0];
        Lepton* lep2 = &muons[0];
        if (lep2->Pt() > lep1->Pt()) std::swap(lep1, lep2);
        process_candidate("EM", "emu", *lep1, *lep2);
    }
}

void atlas::fillBoosted(const Event& ev, const Particle& met, float weight, const TString& syst) {
    FillHist(syst + "/Boosted/Cutflow_all", 1., weight, 12, 0., 12.);
    const RVec<TString> boosted_sr_cutflows = {"bSR1e", "bSR2e", "bSR2mu"};
    for (const auto& region : boosted_sr_cutflows) {
        fillCutflowBin(syst + "/Boosted/Cutflow/" + region, 1, weight, 12);
    }

    RVec<Electron> electrons = selectBoostedElectrons(AllElectrons);
    RVec<Muon> muons = selectBoostedMuons(AllMuons);
    RVec<Lepton*> leptons = makeLeptonPtrs(electrons, muons);
    RVec<Jet> cleaned_smallr = cleanJetsWithLeptons(AllJets, leptons, cuts.overlap_dr);
    RVec<Jet> smallr_jets_for_bveto = selectSmallRJets(cleaned_smallr, cuts.resolved_smallr_jet_pt);
    RVec<FatJet> large_r_jets = selectLargeRJets(AllFatJets);

    FillHist(syst + "/Boosted/N_electrons", electrons.size(), weight, 8, 0., 8.);
    FillHist(syst + "/Boosted/N_muons", muons.size(), weight, 8, 0., 8.);
    FillHist(syst + "/Boosted/N_largeR_jets", large_r_jets.size(), weight, 8, 0., 8.);
    FillHist(syst + "/Boosted/N_btag_jets", countBTaggedJets(smallr_jets_for_bveto), weight, 8, 0., 8.);

    const bool is_bsr1e_topology = (electrons.size() == 1 && muons.empty());
    const bool is_bsr2e_topology = (electrons.size() == 2 && muons.empty());
    const bool is_bsr2mu_topology = (muons.size() == 2 && electrons.empty());
    auto fill_boosted_cutflow = [&](const TString& region, int step) {
        fillCutflowBin(syst + "/Boosted/Cutflow/" + region, step, weight, 12);
    };

    if (is_bsr1e_topology) fill_boosted_cutflow("bSR1e", 2);     // exact 1e0mu
    if (is_bsr2e_topology) fill_boosted_cutflow("bSR2e", 2);     // exact 2e0mu
    if (is_bsr2mu_topology) fill_boosted_cutflow("bSR2mu", 2);   // exact 0e2mu

    if (large_r_jets.size() != 1) return;
    FillHist(syst + "/Boosted/Cutflow_all", 2., weight, 12, 0., 12.);
    if (is_bsr1e_topology) fill_boosted_cutflow("bSR1e", 3);     // exactly one large-R jet
    if (is_bsr2e_topology) fill_boosted_cutflow("bSR2e", 3);
    if (is_bsr2mu_topology) fill_boosted_cutflow("bSR2mu", 3);
    if (countBTaggedJets(smallr_jets_for_bveto) != 0) return;
    FillHist(syst + "/Boosted/Cutflow_all", 3., weight, 12, 0., 12.);
    if (is_bsr1e_topology) fill_boosted_cutflow("bSR1e", 4);     // b-veto
    if (is_bsr2e_topology) fill_boosted_cutflow("bSR2e", 4);
    if (is_bsr2mu_topology) fill_boosted_cutflow("bSR2mu", 4);

    FatJet& large_r_jet = large_r_jets[0];

    if (is_bsr1e_topology) {
        Electron& electron = electrons[0];
        if (!passBoostedLeadingElectron(electron)) return;
        FillHist(syst + "/Boosted/Cutflow_all", 4., weight, 12, 0., 12.);
        fill_boosted_cutflow("bSR1e", 5); // leading electron pT and ID
        if (!passBoostedTrigger(ev, electron)) return;
        FillHist(syst + "/Boosted/Cutflow_all", 5., weight, 12, 0., 12.);
        fill_boosted_cutflow("bSR1e", 6); // trigger
        if (!(std::fabs(electron.DeltaPhi(large_r_jet)) > cuts.boosted_dphi_lep_large_r_jet)) return;
        FillHist(syst + "/Boosted/Cutflow_all", 6., weight, 12, 0., 12.);
        fill_boosted_cutflow("bSR1e", 7); // dPhi(l1,J)

        const float cos_theta = calculateCosTheta(electron, met);
        const float wr_mass = (electron + large_r_jet).M();
        FillHist(syst + "/Boosted/b1e/cosTheta", cos_theta, weight, 80, -1., 1.);
        FillHist(syst + "/Boosted/b1e/MET", met.Pt(), weight, 100, 0., 500.);
        FillHist(syst + "/Boosted/b1e/dEta_J_l1", std::fabs(large_r_jet.Eta() - electron.Eta()), weight, 80, 0., 4.);

        if (!(met.Pt() < cuts.boosted_met)) return;
        fill_boosted_cutflow("bSR1e", 8); // MET
        if (!(std::fabs(cos_theta) > cuts.boosted_cos_theta)) {
            fillBoostedCandidate("bCRW1e", electron + large_r_jet, large_r_jet, weight, syst);
            return;
        }
        fill_boosted_cutflow("bSR1e", 9); // |cosTheta|
        if (!(std::fabs(large_r_jet.Eta() - electron.Eta()) < cuts.boosted_delta_eta_e_large_r_jet)) return;
        fill_boosted_cutflow("bSR1e", 10); // dEta(J,e)
        FillHist(syst + "/Boosted/Cutflow_all", 7., weight, 12, 0., 12.);
        fill_boosted_cutflow("bSR1e", 11); // final SR
        fillBoostedCandidate("bSR1e", electron + large_r_jet, large_r_jet, weight, syst);
        if (wr_mass > 2000. && wr_mass < 3000.) {
            fillBoostedCandidate("bVR1e", electron + large_r_jet, large_r_jet, weight, syst);
        }
    }

    if (is_bsr2e_topology) {
        if (!passBoostedLeadingElectron(electrons[0])) return;
        FillHist(syst + "/Boosted/Cutflow_all", 4., weight, 12, 0., 12.);
        fill_boosted_cutflow("bSR2e", 5); // leading electron pT and ID
        if (!passBoostedTrigger(ev, electrons[0])) return;
        FillHist(syst + "/Boosted/Cutflow_all", 5., weight, 12, 0., 12.);
        fill_boosted_cutflow("bSR2e", 6); // trigger
        if (!(std::fabs(electrons[0].DeltaPhi(large_r_jet)) > cuts.boosted_dphi_lep_large_r_jet)) return;
        FillHist(syst + "/Boosted/Cutflow_all", 6., weight, 12, 0., 12.);
        fill_boosted_cutflow("bSR2e", 7); // dPhi(l1,J)

        const Particle dilepton = electrons[0] + electrons[1];
        FillHist(syst + "/Boosted/b2e/mll", dilepton.M(), weight, 100, 0., 1000.);
        FillHist(syst + "/Boosted/b2e/MET", met.Pt(), weight, 100, 0., 500.);
        if (!(met.Pt() < cuts.boosted_met)) return;
        fill_boosted_cutflow("bSR2e", 8); // MET
        if (dilepton.M() > 120. && dilepton.M() < cuts.boosted_mll) {
            fillBoostedCandidate("bVRZ2e", dilepton + large_r_jet, large_r_jet, weight, syst);
            return;
        }
        if (!(dilepton.M() > cuts.boosted_mll)) return;
        fill_boosted_cutflow("bSR2e", 9); // mll
        FillHist(syst + "/Boosted/Cutflow_all", 7., weight, 12, 0., 12.);
        fill_boosted_cutflow("bSR2e", 10); // final SR
        fillBoostedCandidate("bSR2e", dilepton + large_r_jet, large_r_jet, weight, syst);
        const float wr_mass = (dilepton + large_r_jet).M();
        if (wr_mass > 1000. && wr_mass < 2000.) {
            fillBoostedCandidate("bCRZ2e", dilepton + large_r_jet, large_r_jet, weight, syst);
        } else if (wr_mass > 2000. && wr_mass < 3000.) {
            fillBoostedCandidate("bVR2e", dilepton + large_r_jet, large_r_jet, weight, syst);
        }
    }

    if (is_bsr2mu_topology) {
        if (!passBoostedLeadingMuon(muons[0])) return;
        FillHist(syst + "/Boosted/Cutflow_all", 4., weight, 12, 0., 12.);
        fill_boosted_cutflow("bSR2mu", 5); // leading muon pT and ID
        if (!passBoostedTrigger(ev, muons[0])) return;
        FillHist(syst + "/Boosted/Cutflow_all", 5., weight, 12, 0., 12.);
        fill_boosted_cutflow("bSR2mu", 6); // trigger
        if (!(std::fabs(muons[0].DeltaPhi(large_r_jet)) > cuts.boosted_dphi_lep_large_r_jet)) return;
        FillHist(syst + "/Boosted/Cutflow_all", 6., weight, 12, 0., 12.);
        fill_boosted_cutflow("bSR2mu", 7); // dPhi(l1,J)

        const Particle dilepton = muons[0] + muons[1];
        FillHist(syst + "/Boosted/b2mu/mll", dilepton.M(), weight, 100, 0., 1000.);
        FillHist(syst + "/Boosted/b2mu/ptll", dilepton.Pt(), weight, 100, 0., 1000.);
        if (dilepton.M() > 120. && dilepton.M() < cuts.boosted_mll && dilepton.Pt() > cuts.boosted_dilepton_pt) {
            fillBoostedCandidate("bVRZ2mu", dilepton + large_r_jet, large_r_jet, weight, syst);
            return;
        }
        if (!(dilepton.M() > cuts.boosted_mll)) return;
        fill_boosted_cutflow("bSR2mu", 8); // mll
        if (!(dilepton.Pt() > cuts.boosted_dilepton_pt)) return;
        fill_boosted_cutflow("bSR2mu", 9); // ptll
        FillHist(syst + "/Boosted/Cutflow_all", 7., weight, 12, 0., 12.);
        fill_boosted_cutflow("bSR2mu", 10); // final SR
        fillBoostedCandidate("bSR2mu", dilepton + large_r_jet, large_r_jet, weight, syst);
        const float wr_mass = (dilepton + large_r_jet).M();
        if (wr_mass > 1000. && wr_mass < 2000.) {
            fillBoostedCandidate("bCRZ2mu", dilepton + large_r_jet, large_r_jet, weight, syst);
        } else if (wr_mass > 2000. && wr_mass < 3000.) {
            fillBoostedCandidate("bVR2mu", dilepton + large_r_jet, large_r_jet, weight, syst);
        }
    }
}

void atlas::fillResolvedCandidate(const TString& base,
                                  const Lepton& lep1,
                                  const Lepton& lep2,
                                  const Jet& jet1,
                                  const Jet& jet2,
                                  float weight,
                                  const TString& syst) {
    const Particle dilepton = lep1 + lep2;
    const Particle dijet = jet1 + jet2;
    const Particle wr_candidate = dilepton + dijet;
    const float ht = lep1.Pt() + lep2.Pt() + jet1.Pt() + jet2.Pt();
    const TString path = syst + "/Resolved/" + base;

    RVec<float> mlljj_bins = {0., 500., 1000., 1500., 2000., 2500., 3000., 3500., 4000., 4500., 5000., 8000.};
    RVec<float> mjj_bins = {0., 500., 1000., 1500., 2000., 2500., 3000., 8000.};
    RVec<float> ht_bins = {400., 600., 1000., 1500., 2200., 8000.};

    FillHist(path + "/yield", 1., weight, 2, 0., 2.);
    FillHist(path + "/mll", dilepton.M(), weight, 160, 0., 4000.);
    FillHist(path + "/mjj", dijet.M(), weight, 120, 0., 3000.);
    FillHist(path + "/mlljj", wr_candidate.M(), weight, 160, 0., 8000.);
    FillHist(path + "/HT", ht, weight, 160, 0., 8000.);
    FillHist(path + "/pt_l1", lep1.Pt(), weight, 160, 0., 4000.);
    FillHist(path + "/pt_l2", lep2.Pt(), weight, 160, 0., 4000.);
    FillHist(path + "/pt_j1", jet1.Pt(), weight, 160, 0., 4000.);
    FillHist(path + "/pt_j2", jet2.Pt(), weight, 160, 0., 4000.);
    FillHist(path + "/dRll", lep1.DeltaR(lep2), weight, 80, 0., 5.);
    FillHist(path + "/mlljj_paper_bins", wr_candidate.M(), weight, mlljj_bins);
    FillHist(path + "/mjj_paper_bins", dijet.M(), weight, mjj_bins);
    FillHist(path + "/HT_paper_bins", ht, weight, ht_bins);
}

void atlas::fillBoostedCandidate(const TString& base,
                                 const Particle& wr_candidate,
                                 const FatJet& large_r_jet,
                                 float weight,
                                 const TString& syst) {
    const TString path = syst + "/Boosted/" + base;
    RVec<float> mass_bins = {1000., 2000., 3000., 4000., 10000.};

    FillHist(path + "/yield", 1., weight, 2, 0., 2.);
    FillHist(path + "/mWR", wr_candidate.M(), weight, 160, 0., 8000.);
    FillHist(path + "/mWR_paper_bins", wr_candidate.M(), weight, mass_bins);
    FillHist(path + "/largeR_pt", large_r_jet.Pt(), weight, 160, 0., 4000.);
    FillHist(path + "/largeR_mass", large_r_jet.M(), weight, 120, 0., 600.);
    FillHist(path + "/largeR_sdmass", large_r_jet.SDMass(), weight, 120, 0., 600.);
}
