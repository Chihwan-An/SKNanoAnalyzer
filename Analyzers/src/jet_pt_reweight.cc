#include "jet_pt_reweight.h"

#include <TFile.h>
#include <TH1D.h>

#include <cmath>
#include <cstdlib>

jet_pt_reweight::jet_pt_reweight() {}
jet_pt_reweight::~jet_pt_reweight() {}

// ---------------------------------------------------------------------------
// setup
// ---------------------------------------------------------------------------
void jet_pt_reweight::initializeAnalyzer() {
    // Triggers, myCorr and the object configuration come from the parent, so
    // the selection below runs against exactly the same cut values as the main
    // analyzer. Only the systematic list is replaced afterwards.
    Reproduce20_002_copy::initializeAnalyzer();

    // R's uncertainty is data statistics, MC statistics and the non-DY cross
    // sections -- nothing else (DY_CORRECTION_SPEC.md A2). Object and theory
    // nuisances were dropped from the band on purpose: they get re-derived
    // automatically when a datacard nuisance moves the reco jet pT and the R
    // lookup lands in a different bin, so carrying them here would double count.
    // A single Central pass is therefore all this analyzer needs.
    string SKNANO_HOME = getenv("SKNANO_HOME");
    systHelper = std::make_unique<SystematicHelper>(
        SKNANO_HOME + "/docs/jet_pt_reweight.yaml",
        IsDATA ? DataStream : MCSample, DataEra);

    // --- the gen Z-pT correction
    fApplyZpt = false;
    if (IsDATA || !IsDYSample()) return;
    if (getenv("DY_ZPT_NO_CORRECTION")) {
        cout << "[jet_pt_reweight] DY_ZPT_NO_CORRECTION is set: filling without "
             << "C. This is the validation mode -- the output should reproduce "
             << "the existing Reproduce20_002_copy DY CR histograms." << endl;
        return;
    }

    const char *env = getenv("DY_ZPT_CORRECTION");
    const TString path = env ? env : kDefaultZptFile;
    // Era-specific by construction. Falling back to the combined curve would
    // quietly reweight one era with another's shape.
    const TString hpath = TString(kZptVariant) + "/" + DataEra + "/ZPTReweight";

    unique_ptr<TFile> f(TFile::Open(path));
    if (!f || f->IsZombie()) {
        cerr << "[jet_pt_reweight] FATAL: cannot open " << path << endl;
        exit(EXIT_FAILURE);
    }
    TH1D *h = dynamic_cast<TH1D *>(f->Get(hpath));
    if (!h) {
        cerr << "[jet_pt_reweight] FATAL: " << hpath << " not found in " << path
             << ". C exists for 2022 and 2022EE only; 2023 / 2023BPix need "
             << "DYGenZpT re-run once their sample lists are settled." << endl;
        exit(EXIT_FAILURE);
    }

    const int n = h->GetNbinsX();
    fZptEdges.resize(n + 1);
    fZptValues.resize(n);
    for (int i = 1; i <= n; i++) {
        fZptEdges[i - 1] = h->GetBinLowEdge(i);
        fZptValues[i - 1] = h->GetBinContent(i);
    }
    fZptEdges[n] = h->GetBinLowEdge(n) + h->GetBinWidth(n);
    fApplyZpt = true;

    cout << "[jet_pt_reweight] C from " << path << ":" << hpath << " -- " << n
         << " bins, " << fZptEdges.front() << "-" << fZptEdges.back() << " GeV"
         << endl;
}

bool jet_pt_reweight::IsDYSample() const {
    return MCSample.Contains("DYMLL");
}

float jet_pt_reweight::GetZptWeight(float gen_zpt) const {
    if (!fApplyZpt || fZptValues.empty() || gen_zpt < 0.) return 1.f;

    // Clamp rather than extrapolate. The top bin is merged and runs to 8 TeV, so
    // in practice nothing lands outside.
    size_t idx = 0;
    if (gen_zpt >= fZptEdges.back()) {
        idx = fZptValues.size() - 1;
    } else {
        while (idx + 1 < fZptValues.size() && gen_zpt >= fZptEdges[idx + 1]) idx++;
    }

    const double c = fZptValues[idx];
    // A 0 would delete the DY event instead of leaving it alone. The delivered
    // curve has no such bin (checked at derivation), so this is a guard, not a
    // routine path.
    if (!std::isfinite(c) || c <= 0.) return 1.f;
    return static_cast<float>(c);
}

float jet_pt_reweight::GetGenZpT() const {
    // Transcribed from DYGenZpT.cc. The two must not drift: C is a function of
    // this quantity, so looking it up with a different definition silently
    // applies the wrong correction.
    RVec<Gen> hard;
    for (const auto &gen : gen_set.gens) {
        const int abspid = abs(gen.PID());
        if (abspid != 11 && abspid != 13 && abspid != 15) continue;
        if (!gen.isHardProcess()) continue;
        hard.emplace_back(gen);
    }
    if (hard.size() != 2) return -1.;
    return (hard.at(0) + hard.at(1)).Pt();
}

// ---------------------------------------------------------------------------
// event loop
// ---------------------------------------------------------------------------
void jet_pt_reweight::executeEvent() {
    el_set.AllElectrons = GetAllElectrons();
    mu_set.AllMuons = GetAllMuons();
    jet_set.AllJets = GetAllJets();
    fatjet_set.AllFatJets = GetAllFatJets();
    gen_set.gens = GetAllGens();
    lhe_set.lhe_parts = GetAllLHEs();

    const TString this_syst = "Central";

    Event ev = GetEvent();

    float weight = 1.0;
    if (!IsDATA) {
        weight *= MCweight();
        weight *= ev.GetTriggerLumi("HLT_Mu50");
    }

    // --- C(gen Z pT). Event-level scalar, independent of the reco category, so
    // it multiplies the weight once here and reaches both DY CRs.
    float zpt_weight = 1.f;
    if (fApplyZpt) {
        const float gen_zpt = GetGenZpT();
        if (gen_zpt < 0.) {
            cerr << "[jet_pt_reweight] FATAL: no isHardProcess lepton pair "
                 << "(run " << RunNumber << ", event " << EventNumber
                 << ", sample " << MCSample << ", era " << DataEra << "). "
                 << "C was derived assuming 100% coverage of this definition."
                 << endl;
            exit(EXIT_FAILURE);
        }
        zpt_weight = GetZptWeight(gen_zpt);
        weight *= zpt_weight;
        FillHist(this_syst + "/genZpt_all", gen_zpt, 1.0, 141, 0., 2000.);
        FillHist(this_syst + "/C_applied", zpt_weight, 1.0, 200, 0., 2.);
    }

    RVec<Electron> electrons = el_set.AllElectrons;
    RVec<Muon> muons = mu_set.AllMuons;
    RVec<Jet> jets = jet_set.AllJets;
    RVec<FatJet> fatjets = fatjet_set.AllFatJets;

    // Nominal JER smearing is not pre-applied in NanoAOD and must be applied to
    // every MC event, exactly as the parent does. No JES/JER variations here --
    // this analyzer runs Central only.
    if (!IsDATA) {
        RVec<GenJet> genjets = GetAllGenJets();
        jets = SmearJets(jets, genjets, MyCorrection::variation::nom, "total");
        RVec<GenJet> genjetsak8 = GetAllGenJetAK8();
        fatjets = SmearFatJets(fatjets, genjetsak8, MyCorrection::variation::nom, "total");
    }

    if (!PassNoiseFilter(jets, ev, Event::MET_Type::PUPPI)) return;

    const bool pass_trig_muon = ev.PassTrigger(mu_set.Muon_Trigger);
    const bool pass_trig_elec = ev.PassTrigger(el_set.Ele_Trigger);

    // --- leptons
    RVec<Electron> my_electrons = SelectElectrons(electrons, "NOCUT", el_set.Electron_MinPt, 2.5);
    RVec<Muon> my_muons = SelectMuons(muons, "NOCUT", mu_set.Muon_MinPt, 2.4);
    sort(my_electrons.begin(), my_electrons.end(), PtComparing);
    sort(my_muons.begin(), my_muons.end(), PtComparing);

    RVec<Electron *> Loose_electrons, Tight_electrons;
    RVec<Muon *> Loose_muons, Tight_muons;
    RVec<Lepton *> Tight_leps_el, Tight_leps_mu, Tight_leps;
    RVec<Lepton *> Loose_leps_el, Loose_leps_mu, Loose_leps;

    for (unsigned int i = 0; i < my_electrons.size(); i++) {
        Electron &el = my_electrons.at(i);
        if (el.PassID(el_set.Electron_Tight_ID[0])) {
            Tight_electrons.push_back(&el);
            Tight_leps_el.push_back(&el);
            Tight_leps.push_back(&el);
        }
        // Loose = loose-without-isolation OR HEEP, matching the parent.
        const bool passLooseNoIso = el_set.isPassLooseNoIso(el);
        const bool passHEEP = el.PassID(Electron::ElectronID::POG_HEEP);
        if (passLooseNoIso || passHEEP) {
            Loose_electrons.push_back(&el);
            Loose_leps_el.push_back(&el);
            Loose_leps.push_back(&el);
        }
    }
    for (unsigned int i = 0; i < my_muons.size(); i++) {
        Muon &mu = my_muons.at(i);
        if (mu.PassID(mu_set.Muon_Tight_ID[0]) && (mu.TkRelIso() < mu_set.Muon_Iso_Cut)) {
            Tight_muons.push_back(&mu);
            Tight_leps_mu.push_back(&mu);
            Tight_leps.push_back(&mu);
        }
        if (mu.PassID(mu_set.Muon_Loose_ID[0])) {
            Loose_muons.push_back(&mu);
            Loose_leps_mu.push_back(&mu);
            Loose_leps.push_back(&mu);
        }
    }

    if (CleanFatjetWithTightLeptons) fatjets = Clean_Fatjet_with_tight_leptons(fatjets, Tight_leps);
    jets = Clean_jet_with_loose_leptons(jets, Loose_leps);

    // --- fatjets
    RVec<FatJet> fatjet_list, lsf;
    for (unsigned int i = 0; i < fatjets.size(); i++) {
        FatJet &fj = fatjets.at(i);
        if ((fj.Pt() > fatjet_set.FatJet_MinPt) && (abs(fj.Eta()) < fatjet_set.FatJet_MaxEta) &&
            (fj.SDMass() > fatjet_set.FatJet_SDM)) {
            if (fj.PassID(fatjet_set.FatJet_ID)) {
                fatjet_list.push_back(fj);
                if (fj.LSF3() > fatjet_set.Fatjet_LSF) lsf.push_back(fj);
            }
        }
    }
    fatjets = fatjet_list;
    RVec<FatJet> fatjets_LSF = lsf;
    sort(fatjets.begin(), fatjets.end(), PtComparing);
    sort(fatjets_LSF.begin(), fatjets_LSF.end(), PtComparing);

    sort(Tight_leps.begin(), Tight_leps.end(), PtComparingPtr);
    const int n_Tight_leptons = Tight_electrons.size() + Tight_muons.size();

    // --- jets
    if (!AnalyzerCore::PassVetoMap(jets, mu_set.AllMuons, "jetvetomap")) return;
    RVec<Jet> selected_jets = SelectJets(jets, jet_set.Jet_ID[0], jet_set.Jet_MinPt, jet_set.Jet_MaxEta);
    sort(selected_jets.begin(), selected_jets.end(), PtComparing);

    // --- scale factors. Registered per category the same way the parent does,
    // because R is a ratio of data to MC and an SF that is missing on one side
    // does not cancel.
    std::unordered_map<std::string,
                       std::variant<std::function<float(MyCorrection::variation, TString)>,
                                    std::function<float()>>> weight_function_map;
    auto dummy_sf = [](MyCorrection::variation var, TString source) -> float { return 1.0; };
    for (const char *k : {"PU_Weight", "E_Id_Weight", "E_Reco_Weight", "E_Trig_Weight",
                          "M_Id_Weight", "M_Reco_Weight", "M_Trig_Weight", "M_Iso_Weight"})
        weight_function_map[k] = dummy_sf;

    if (!IsDATA) {
        weight_function_map["PU_Weight"] = [&](MyCorrection::variation var, TString source) {
            return myCorr->GetPUWeight(ev.nTrueInt(), var);
        };
    }

    // --- resolved DY CR ------------------------------------------------------
    bool is_Resolved_DY_EE = false, is_Resolved_DY_MM = false;
    float Resolve_DYCREEleadjetpt = 0., Resolve_DYCRMMleadjetpt = 0.;
    bool IsResolvedEvent = false;

    bool this_trigger_pass = false;
    bool tmp_isEE = false, tmp_isMM = false;

    if ((n_Tight_leptons == 2) &&
        (Tight_leps[0]->Pt() > (Tight_leps[0]->IsElectron() ? 130.0 : 60.0)) &&
        (Tight_leps[1]->Pt() > 53.0)) {

        if ((Tight_electrons.size() == 2) && (Tight_muons.size() == 0)) {
            if (Tight_electrons[0]->Pt() < el_set.Ele_Trigger_Safe_Pt_Cut) return;
            this_trigger_pass = pass_trig_elec;
            tmp_isEE = true;
            if (!IsDATA) {
                weight_function_map["E_Id_Weight"] = [&](MyCorrection::variation var, TString source) -> float {
                    return GetElectronHEEPIDSF_TnP(Tight_electrons[0]->scEta(), Tight_electrons[0]->Pt(), var) *
                           GetElectronHEEPIDSF_TnP(Tight_electrons[1]->scEta(), Tight_electrons[1]->Pt(), var);
                };
                weight_function_map["E_Reco_Weight"] = [&](MyCorrection::variation var, TString source) {
                    return myCorr->GetElectronRECOSF(Tight_electrons[0]->scEta(), Tight_electrons[0]->Pt(),
                                                     Tight_electrons[0]->Phi(), var) *
                           myCorr->GetElectronRECOSF(Tight_electrons[1]->scEta(), Tight_electrons[1]->Pt(),
                                                     Tight_electrons[1]->Phi(), var);
                };
                if (DataEra != "2017") {
                    weight_function_map["E_Trig_Weight"] = [&](MyCorrection::variation var, TString source) {
                        return GetElectronTriggerSF_TnP(Tight_electrons[0]->scEta(),
                                                        Tight_electrons[0]->Pt(), var);
                    };
                }
            }
        } else if ((Tight_muons.size() == 2) && (Tight_electrons.size() == 0)) {
            if (Tight_muons[0]->Pt() < mu_set.Muon_Trigger_Safe_Pt_Cut) return;
            this_trigger_pass = pass_trig_muon;
            tmp_isMM = true;
            if (!IsDATA) {
                weight_function_map["M_Id_Weight"] = [&](MyCorrection::variation var, TString source) -> float {
                    if (DataEra == "2017") return 1.0;
                    return myCorr->GetMuonIDSF("NUM_HighPtID_DEN_GlobalMuonProbes", *Tight_muons[0], var) *
                           myCorr->GetMuonIDSF("NUM_HighPtID_DEN_GlobalMuonProbes", *Tight_muons[1], var);
                };
                weight_function_map["M_Reco_Weight"] = [&](MyCorrection::variation var, TString source) {
                    return myCorr->GetMuonRECOSF(*Tight_muons[0], var) *
                           myCorr->GetMuonRECOSF(*Tight_muons[1], var);
                };
                weight_function_map["M_Trig_Weight"] = [&](MyCorrection::variation var, TString source) -> float {
                    if (DataEra == "2017") return 1.0;
                    RVec<Muon *> trig_muons;
                    trig_muons.push_back(Tight_muons[0]);
                    trig_muons.push_back(Tight_muons[1]);
                    return myCorr->GetMuonTriggerSF("NUM_HLT_DEN_HighPtLooseRelIsoProbes", trig_muons, var);
                };
                weight_function_map["M_Iso_Weight"] = [&](MyCorrection::variation var, TString source) -> float {
                    if (DataEra == "2017") return 1.0;
                    return myCorr->GetMuonIDSF("NUM_probe_LooseRelTkIso_DEN_HighPtProbes", *Tight_muons[0], var) *
                           myCorr->GetMuonIDSF("NUM_probe_LooseRelTkIso_DEN_HighPtProbes", *Tight_muons[1], var);
                };
            }
        }

        if (this_trigger_pass && (tmp_isEE || tmp_isMM) && selected_jets.size() >= 2) {
            Lepton *LeadLep = Tight_leps[0];
            Lepton *SubLeadLep = Tight_leps[1];
            const bool dRLeadJetLep = (selected_jets[0].DeltaR(*Tight_leps[0]) > 0.4) &&
                                      (selected_jets[0].DeltaR(*Tight_leps[1]) > 0.4);
            const bool dRSubLeadJetLep = (selected_jets[1].DeltaR(*Tight_leps[0]) > 0.4) &&
                                         (selected_jets[1].DeltaR(*Tight_leps[1]) > 0.4);
            const bool dRTwoLepton = (LeadLep->DeltaR(*SubLeadLep) > 0.4);
            const bool dRTwoJets = (selected_jets[0].DeltaR(selected_jets[1]) > 0.4);

            if (dRLeadJetLep && dRSubLeadJetLep && dRTwoLepton && dRTwoJets) {
                IsResolvedEvent = true;
                Particle WRCand = *LeadLep + *SubLeadLep + selected_jets[0] + selected_jets[1];
                const double dilepton_mass = (*LeadLep + *SubLeadLep).M();
                const bool DiLepMassLT150 = (dilepton_mass >= 60.) && (dilepton_mass < 150.);

                // The DY CR itself: 60 < m(ll) < 150 and m(lljj) > 800.
                if (DiLepMassLT150 && WRCand.M() > 800.0) {
                    if (tmp_isEE) {
                        is_Resolved_DY_EE = true;
                        Resolve_DYCREEleadjetpt = selected_jets[0].Pt();
                    } else if (tmp_isMM) {
                        is_Resolved_DY_MM = true;
                        Resolve_DYCRMMleadjetpt = selected_jets[0].Pt();
                    }
                }
            }
        }
    }

    // --- boosted DY CR -------------------------------------------------------
    bool is_Boosted_DY_EE = false, is_Boosted_DY_MM = false;
    float Boost_DYCREEfatjetpt = 0., Boost_DYCRMMfatjetpt = 0.;

    if (!IsResolvedEvent) {
        if ((n_Tight_leptons > 0) &&
            (Tight_leps[0]->Pt() > (Tight_leps[0]->IsElectron() ? 130.0 : 60.0))) {

            Lepton *LeadLep = Tight_leps[0];
            bool this_trigger_pass_boost = false;
            bool is_tmp_lead_el = false, is_tmp_lead_mu = false;

            if (LeadLep->IsElectron()) {
                if (LeadLep->Pt() < el_set.Ele_Trigger_Safe_Pt_Cut) return;
                is_tmp_lead_el = true;
                this_trigger_pass_boost = pass_trig_elec;
                if (!IsDATA) {
                    weight_function_map["E_Id_Weight"] = [&](MyCorrection::variation var, TString source) -> float {
                        return GetElectronHEEPIDSF_TnP(Tight_electrons[0]->scEta(), Tight_electrons[0]->Pt(), var);
                    };
                }
            } else if (LeadLep->IsMuon()) {
                if (LeadLep->Pt() < mu_set.Muon_Trigger_Safe_Pt_Cut) return;
                is_tmp_lead_mu = true;
                this_trigger_pass_boost = pass_trig_muon;
            }

            if (this_trigger_pass_boost) {
                RVec<Lepton *> Loose_SF_leps = is_tmp_lead_el ? Loose_leps_el : Loose_leps_mu;

                // Same-flavour loose partner giving 60 < m(ll) < 150.
                Lepton *LowMllLooseLepton = nullptr;
                for (unsigned int i = 0; i < Loose_SF_leps.size(); i++) {
                    if (Loose_SF_leps[i] == LeadLep) continue;
                    const double dilep_mass = (*LeadLep + *Loose_SF_leps[i]).M();
                    if ((dilep_mass > 60) && (dilep_mass < 150)) {
                        LowMllLooseLepton = Loose_SF_leps[i];
                        break;
                    }
                }

                if (LowMllLooseLepton) {
                    if (!IsDATA) {
                        if (is_tmp_lead_el) {
                            weight_function_map["E_Reco_Weight"] = [&](MyCorrection::variation var, TString source) {
                                return myCorr->GetElectronRECOSF(((Electron *)LeadLep)->scEta(), LeadLep->Pt(),
                                                                 LeadLep->Phi(), var);
                            };
                            if (DataEra != "2017") {
                                weight_function_map["E_Trig_Weight"] = [&](MyCorrection::variation var, TString source) {
                                    return GetElectronTriggerSF_TnP(Tight_electrons[0]->scEta(),
                                                                    Tight_electrons[0]->Pt(), var);
                                };
                            }
                        }
                        if (is_tmp_lead_mu) {
                            weight_function_map["M_Id_Weight"] = [&](MyCorrection::variation var, TString source) -> float {
                                if (DataEra == "2017") return 1.0;
                                return myCorr->GetMuonIDSF("NUM_HighPtID_DEN_GlobalMuonProbes", *Tight_muons[0], var);
                            };
                            weight_function_map["M_Reco_Weight"] = [&](MyCorrection::variation var, TString source) {
                                return myCorr->GetMuonRECOSF(*Tight_muons[0], var);
                            };
                            weight_function_map["M_Trig_Weight"] =
                                [&, LowMllLooseLepton](MyCorrection::variation var, TString source) -> float {
                                if (DataEra == "2017") return 1.0;
                                RVec<Muon *> trig_muons;
                                trig_muons.push_back(Tight_muons[0]);
                                trig_muons.push_back((Muon *)LowMllLooseLepton);
                                return myCorr->GetMuonTriggerSF("NUM_HLT_DEN_HighPtLooseRelIsoProbes", trig_muons, var);
                            };
                            weight_function_map["M_Iso_Weight"] = [&](MyCorrection::variation var, TString source) {
                                return myCorr->GetMuonIDSF("NUM_probe_LooseRelTkIso_DEN_HighPtProbes",
                                                           *Tight_muons[0], var);
                            };
                        }
                    }

                    // First fatjet back-to-back with the lead lepton wins, as in
                    // the parent: the loop breaks out through the same branch.
                    for (unsigned int i = 0; i < fatjets.size(); i++) {
                        FatJet this_fatjet = fatjets.at(i);
                        if (abs(LeadLep->DeltaPhi(this_fatjet)) <= 2.0) continue;

                        Particle Ncand;
                        if (this_fatjet.DeltaR(*LowMllLooseLepton) < 0.8) Ncand = this_fatjet;
                        else Ncand = this_fatjet + *LowMllLooseLepton;

                        Particle WRCand = *LeadLep + Ncand;
                        if (WRCand.M() > 800.0) {
                            if (is_tmp_lead_el) {
                                is_Boosted_DY_EE = true;
                                Boost_DYCREEfatjetpt = this_fatjet.Pt();
                            } else if (is_tmp_lead_mu) {
                                is_Boosted_DY_MM = true;
                                Boost_DYCRMMfatjetpt = this_fatjet.Pt();
                            }
                        }
                    }
                }
            }
        }
    }

    // --- fill ----------------------------------------------------------------
    // Binning matches Reproduce20_002_copy exactly, so the validation run can be
    // compared bin by bin and make_dy_jetpt_ratio.py needs no changes.
    systHelper->assignWeightFunctionMap(weight_function_map);

    std::vector<std::pair<std::string, float>> fill_targets;
    if (!IsDATA) {
        for (const auto &[sn, sf_val] : systHelper->calculateWeight())
            fill_targets.push_back({sn, weight * sf_val});
    } else {
        fill_targets.push_back({std::string(this_syst), 1.0f});
    }

    for (const auto &[syst_name, final_weight] : fill_targets) {
        if (is_Resolved_DY_EE)
            FillHist(syst_name + "/DYCR_Resolved_EE_leading_jet_pt", Resolve_DYCREEleadjetpt,
                     final_weight, 200, 0., 2000.);
        if (is_Resolved_DY_MM)
            FillHist(syst_name + "/DYCR_Resolved_MM_leading_jet_pt", Resolve_DYCRMMleadjetpt,
                     final_weight, 200, 0., 2000.);
        if (is_Boosted_DY_EE)
            FillHist(syst_name + "/DYCR_Boosted_EE_leading_fatjet_pt", Boost_DYCREEfatjetpt,
                     final_weight, 2000, 0., 2000.);
        if (is_Boosted_DY_MM)
            FillHist(syst_name + "/DYCR_Boosted_MM_leading_fatjet_pt", Boost_DYCRMMfatjetpt,
                     final_weight, 2000, 0., 2000.);

        // gen Z pT inside each region. This is what answers whether the m(ll)
        // dependence of C matters: C is derived at the Z peak, and below about
        // 40 GeV it differs by 13 sigma between the mass slices
        // (DY_CORRECTION_SPEC.md B1-d). If the CR and SR populations sit above
        // that, the peak-derived curve carries over; these histograms measure it.
        if (fApplyZpt) {
            const float gen_zpt = GetGenZpT();
            if (is_Resolved_DY_EE || is_Resolved_DY_MM)
                FillHist(syst_name + "/DYCR_Resolved_genZpt", gen_zpt, final_weight, 141, 0., 2000.);
            if (is_Boosted_DY_EE || is_Boosted_DY_MM)
                FillHist(syst_name + "/DYCR_Boosted_genZpt", gen_zpt, final_weight, 141, 0., 2000.);
        }
    }
}
