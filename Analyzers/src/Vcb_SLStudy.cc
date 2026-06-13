#include "Vcb_SLStudy.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>

namespace {

template <typename T>
void PushUnique(std::vector<T> &values, const T &value) {
  if (std::find(values.begin(), values.end(), value) == values.end())
    values.push_back(value);
}

} // namespace

Vcb_SLStudy::Vcb_SLStudy() = default;

void Vcb_SLStudy::initializeAnalyzer() {
  Vcb::initializeAnalyzer();
  ConfigureStudyFlags();
}

void Vcb_SLStudy::ConfigureStudyFlags() {
  if (studyConfigInitialized_)
    return;

  const bool clean_with_tight = HasFlag("CleanJetsWithTightLeptons");
  const bool clean_with_loose = HasFlag("CleanJetsWithLooseLeptons");
  if (static_cast<int>(clean_with_tight) + static_cast<int>(clean_with_loose) >
      1) {
    throw std::runtime_error(
        "[Vcb_SLStudy::ConfigureStudyFlags] Choose only one jet-cleaning mode");
  }

  const bool jet_id_tight = HasFlag("JetIDTight");
  const bool jet_id_tight_lepton_veto =
      HasFlag("JetIDTightLeptonVeto") || HasFlag("JetIDTightLeptonID");
  if (static_cast<int>(jet_id_tight) +
          static_cast<int>(jet_id_tight_lepton_veto) >
      1) {
    throw std::runtime_error(
        "[Vcb_SLStudy::ConfigureStudyFlags] Choose only one jet ID flag");
  }

  const bool loose_muon_id_tight = HasFlag("LooseMuonIDTight");
  const bool loose_muon_id_medium = HasFlag("LooseMuonIDMedium");
  const bool loose_muon_id_loose = HasFlag("LooseMuonIDLoose");
  const bool loose_muon_id_prompt =
      HasFlag("LooseMuonIDPrompt") || HasFlag("LooseMuonIDPromptMVA");
  if (static_cast<int>(loose_muon_id_tight) +
          static_cast<int>(loose_muon_id_medium) +
          static_cast<int>(loose_muon_id_loose) +
          static_cast<int>(loose_muon_id_prompt) >
      1) {
    throw std::runtime_error(
        "[Vcb_SLStudy::ConfigureStudyFlags] Choose only one loose-muon ID "
        "flag");
  }

  const bool loose_muon_iso_tight = HasFlag("LooseMuonIsoTight");
  const bool loose_muon_iso_loose = HasFlag("LooseMuonIsoLoose");
  const bool loose_muon_iso_none = HasFlag("LooseMuonIsoNone");
  if (static_cast<int>(loose_muon_iso_tight) +
          static_cast<int>(loose_muon_iso_loose) +
          static_cast<int>(loose_muon_iso_none) >
      1) {
    throw std::runtime_error(
        "[Vcb_SLStudy::ConfigureStudyFlags] Choose only one loose-muon "
        "isolation flag");
  }

  const bool require_met20 = HasFlag("RequireMET20") || HasFlag("MET20Cut");
  const bool skip_met20 = HasFlag("SkipMET20");
  if (static_cast<int>(require_met20) + static_cast<int>(skip_met20) > 1) {
    throw std::runtime_error(
        "[Vcb_SLStudy::ConfigureStudyFlags] Choose only one MET20 flag");
  }

  if (HasFlag("CleanJetsWithLooseLeptons")) {
    studyConfig_.jetCleaningMode = JetCleaningMode::LooseLeptons;
    studyConfig_.jetCleaningLabel = "LooseLeptons";
  }

  if (HasFlag("JetIDTightLeptonVeto") || HasFlag("JetIDTightLeptonID")) {
    studyConfig_.jetID = Jet::JetID::TIGHTLEPVETO;
    studyConfig_.jetIDLabel = "TightLeptonVeto";
  }

  if (require_met20) {
    studyConfig_.requireMET20 = true;
    studyConfig_.metLabel = "RequireMET20";
  }

  if (HasFlag("LooseMuonIDMedium")) {
    studyConfig_.looseMuonID = Muon::MuonID::POG_MEDIUM;
    studyConfig_.looseMuonLabel = "MediumID+TightIso";
  } else if (HasFlag("LooseMuonIDLoose")) {
    studyConfig_.looseMuonID = Muon::MuonID::POG_LOOSE;
    studyConfig_.looseMuonLabel = "LooseID+TightIso";
  } else if (HasFlag("LooseMuonIDPrompt") ||
             HasFlag("LooseMuonIDPromptMVA")) {
    studyConfig_.looseMuonID = Muon::MuonID::POG_PROMPTMVA_WP0p64;
    studyConfig_.looseMuonLabel = "PromptMVA+TightIso";
  } else if (HasFlag("LooseMuonIDTight")) {
    studyConfig_.looseMuonID = Muon::MuonID::POG_TIGHT;
    studyConfig_.looseMuonLabel = "TightID+TightIso";
  }

  if (HasFlag("LooseMuonIsoLoose")) {
    studyConfig_.looseMuonIso = LooseMuonIsolationMode::Loose;
    studyConfig_.looseMuonLabel = studyConfig_.looseMuonLabel.substr(
        0, studyConfig_.looseMuonLabel.find('+'));
    studyConfig_.looseMuonLabel += "+LooseIso";
  } else if (HasFlag("LooseMuonIsoNone")) {
    studyConfig_.looseMuonIso = LooseMuonIsolationMode::None;
    studyConfig_.looseMuonLabel = studyConfig_.looseMuonLabel.substr(
        0, studyConfig_.looseMuonLabel.find('+'));
    studyConfig_.looseMuonLabel += "+NoIso";
  } else if (HasFlag("LooseMuonIsoTight")) {
    studyConfig_.looseMuonIso = LooseMuonIsolationMode::Tight;
  }

  std::cout << "[Vcb_SLStudy::initializeAnalyzer] looseMuon="
            << studyConfig_.looseMuonLabel
            << ", jetCleaning=" << studyConfig_.jetCleaningLabel
            << ", jetID=" << studyConfig_.jetIDLabel
            << ", MET=" << studyConfig_.metLabel << std::endl;
  if (studyConfig_.jetCleaningMode == JetCleaningMode::TightLeptons &&
      (HasFlag("LooseMuonIDTight") || HasFlag("LooseMuonIDMedium") ||
       HasFlag("LooseMuonIDLoose") || HasFlag("LooseMuonIDPrompt") ||
       HasFlag("LooseMuonIDPromptMVA") || HasFlag("LooseMuonIsoTight") ||
       HasFlag("LooseMuonIsoLoose") || HasFlag("LooseMuonIsoNone"))) {
    std::cout << "[Vcb_SLStudy::initializeAnalyzer] Loose-muon cleaning "
                 "flags are inactive unless CleanJetsWithLooseLeptons is set"
              << std::endl;
  }

  studyConfigInitialized_ = true;
}

std::vector<std::size_t> Vcb_SLStudy::BuildLooseMuonCleaningIndices(
    const std::vector<std::size_t> &signal_muon_indices) const {
  std::vector<std::size_t> cleaning_muons =
      SelectMuonIndices(AllMuonViews, studyConfig_.looseMuonID, Muon_Veto_Pt,
                        Muon_Veto_Eta);

  switch (studyConfig_.looseMuonIso) {
  case LooseMuonIsolationMode::Tight:
    cleaning_muons =
        SelectMuonIndices(AllMuonViews, cleaning_muons,
                          Muon::MuonID::POG_PFISO_TIGHT, Muon_Veto_Pt,
                          Muon_Veto_Eta);
    break;
  case LooseMuonIsolationMode::Loose:
    cleaning_muons =
        SelectMuonIndices(AllMuonViews, cleaning_muons,
                          Muon::MuonID::POG_PFISO_LOOSE, Muon_Veto_Pt,
                          Muon_Veto_Eta);
    break;
  case LooseMuonIsolationMode::None:
    break;
  }

  for (const auto idx : signal_muon_indices) {
    PushUnique(cleaning_muons, idx);
  }
  std::sort(cleaning_muons.begin(), cleaning_muons.end());
  cleaning_muons.erase(
      std::unique(cleaning_muons.begin(), cleaning_muons.end()),
      cleaning_muons.end());
  return cleaning_muons;
}

std::vector<std::size_t> Vcb_SLStudy::BuildElectronCleaningIndices(
    const std::vector<std::size_t> &signal_electron_indices,
    const std::vector<std::size_t> &loose_electron_indices) const {
  std::vector<std::size_t> cleaning_electrons = loose_electron_indices;
  for (const auto idx : signal_electron_indices) {
    PushUnique(cleaning_electrons, idx);
  }
  std::sort(cleaning_electrons.begin(), cleaning_electrons.end());
  cleaning_electrons.erase(
      std::unique(cleaning_electrons.begin(), cleaning_electrons.end()),
      cleaning_electrons.end());
  return cleaning_electrons;
}

bool Vcb_SLStudy::PassBaseLineSelection(bool remove_flavtagging_cut,
                                        bool loose_cut) {
  if (!studyConfigInitialized_)
    ConfigureStudyFlags();

  Clear();
  FillCutFlow(0);

  if (channel == Channel::Mu)
    if (!ev.PassTrigger(Mu_Trigger[DataEra.Data()]))
      return false;
  if (channel == Channel::El)
    if (!ev.PassTrigger(El_Trigger[DataEra.Data()]))
      return false;
  FillCutFlow(1);

  if (!PassJetVetoMap(AllJetViews))
    return false;
  FillCutFlow(2);

  if (!PassMetFilter(AllJetViews, ev))
    return false;
  FillCutFlow(3);

  std::vector<std::size_t> Muons_Veto_indices = SelectMuonIndices(
      AllMuonViews, Muon_Veto_ID, Muon_Veto_Pt, Muon_Veto_Eta);
  Muons_Veto_indices =
      SelectMuonIndices(AllMuonViews, Muons_Veto_indices, Muon_Veto_Iso,
                        Muon_Veto_Pt, Muon_Veto_Eta);
  std::vector<std::size_t> Muons_indices;
  if (HasFlag("Skim")) {
    Muons_indices.clear();
  } else {
    Muons_indices =
        SelectMuonIndices(AllMuonViews, Muons_Veto_indices, Muon_Tight_ID,
                          Muon_Tight_Pt[DataEra.Data()], Muon_Tight_Eta);
  }

  Muons_indices =
      SelectMuonIndices(AllMuonViews, Muons_indices, Muon_Tight_Iso,
                        Muon_Tight_Pt[DataEra.Data()], Muon_Tight_Eta);

  std::vector<std::size_t> Electron_Veto_indices = SelectElectronIndices(
      AllElectronViews, Electron_Veto_ID, Electron_Veto_Pt, Electron_Veto_Eta);

  std::vector<std::size_t> Electrons_indices;
  if (HasFlag("Skim")) {
    Electrons_indices.clear();
  } else {
    Electrons_indices = SelectElectronIndices(
        AllElectronViews, Electron_Veto_indices, Electron_Tight_ID,
        Electron_Tight_Pt[DataEra.Data()], Electron_Tight_Eta);
  }

  if (HasFlag("Skim")) {
    auto select_tight_muons = [&](Muon::MuonID id, bool require_iso) {
      std::vector<std::size_t> indices = SelectMuonIndices(
          AllMuonViews, id, Muon_Tight_Pt[DataEra.Data()], Muon_Tight_Eta);
      if (require_iso) {
        indices =
            SelectMuonIndices(AllMuonViews, indices, Muon_Tight_Iso,
                              Muon_Tight_Pt[DataEra.Data()], Muon_Tight_Eta);
      }
      return indices;
    };

    auto select_tight_electrons = [&](Electron::ElectronID id) {
      return SelectElectronIndices(AllElectronViews, id,
                                   Electron_Tight_Pt[DataEra.Data()],
                                   Electron_Tight_Eta);
    };

    auto count_extra_loose = [](const std::vector<std::size_t> &loose,
                                const std::vector<std::size_t> &tight) {
      std::size_t count = 0;
      for (const auto idx : loose) {
        if (std::find(tight.begin(), tight.end(), idx) == tight.end())
          ++count;
      }
      return count;
    };

    const std::vector<std::size_t> mu_tight_pog =
        select_tight_muons(Muon::MuonID::POG_TIGHT, true);
    const std::vector<std::size_t> mu_tight_prompt =
        select_tight_muons(Muon::MuonID::POG_PROMPTMVA_WP0p64, false);
    const std::vector<std::size_t> el_tight_wp80 =
        select_tight_electrons(Electron::ElectronID::POG_MVAISO_WP80);
    const std::vector<std::size_t> el_tight_prompt =
        select_tight_electrons(Electron::ElectronID::POG_PROMPTMVA_MEDIUM);

    auto pass_mu_case = [&](const std::vector<std::size_t> &mu_tight,
                            const std::vector<std::size_t> &el_tight) {
      if (mu_tight.size() != 1)
        return false;
      if (!el_tight.empty())
        return false;
      if (!Electron_Veto_indices.empty())
        return false;
      if (count_extra_loose(Muons_Veto_indices, mu_tight) != 0)
        return false;
      return true;
    };

    auto pass_el_case = [&](const std::vector<std::size_t> &mu_tight,
                            const std::vector<std::size_t> &el_tight) {
      if (el_tight.size() != 1)
        return false;
      if (!mu_tight.empty())
        return false;
      if (!Muons_Veto_indices.empty())
        return false;
      if (count_extra_loose(Electron_Veto_indices, el_tight) != 0)
        return false;
      return true;
    };

    if (channel == Channel::Mu) {
      if (pass_mu_case(mu_tight_pog, el_tight_wp80)) {
        Muons_indices = mu_tight_pog;
        Electrons_indices = el_tight_wp80;
      } else if (pass_mu_case(mu_tight_prompt, el_tight_prompt)) {
        Muons_indices = mu_tight_prompt;
        Electrons_indices = el_tight_prompt;
      } else {
        return false;
      }
    } else if (channel == Channel::El) {
      if (pass_el_case(mu_tight_pog, el_tight_wp80)) {
        Muons_indices = mu_tight_pog;
        Electrons_indices = el_tight_wp80;
      } else if (pass_el_case(mu_tight_prompt, el_tight_prompt)) {
        Muons_indices = mu_tight_prompt;
        Electrons_indices = el_tight_prompt;
      } else {
        return false;
      }
    }
  }

  if (channel == Channel::El) {
    if (!HasFlag("Skim")) {
      if (!(Electrons_indices.size() == 1 && Muons_Veto_indices.size() == 0 &&
            Muons_indices.size() == 0 && Electron_Veto_indices.size() == 1))
        return false;
    }
    Muons = MaterializeMuons(AllMuonViews, Muons_indices);
    Electrons = MaterializeElectrons(AllElectronViews, Electrons_indices);
    lepton = Electrons[0];
    leptons.push_back(lepton);
  }
  if (channel == Channel::Mu) {
    if (!HasFlag("Skim")) {
      if (!(Muons_indices.size() == 1 && Electron_Veto_indices.size() == 0 &&
            Electrons_indices.size() == 0 && Muons_Veto_indices.size() == 1))
        return false;
    }
    Muons = MaterializeMuons(AllMuonViews, Muons_indices);
    if (!HasFlag("Skim")) {
      if (!Muons[0].PassID(Muon::MuonID::POG_PROMPTMVA_WP0p64)) {
        return false;
      }
    }
    Electrons = MaterializeElectrons(AllElectronViews, Electrons_indices);
    lepton = Muons[0];
    leptons.push_back(lepton);
  }
  FillCutFlow(4);

  MyCorrection::variation jesVar = MyCorrection::variation::nom;
  MyCorrection::variation jerVar = MyCorrection::variation::nom;
  const std::string systTarget = systHelper->getCurrentIterSysTarget();
  const TString systSource = systHelper->getCurrentIterSysSource();
  const MyCorrection::variation systVar = systHelper->getCurrentIterVariation();

  MET = ev.GetMETVector(Event::MET_Type::PUPPI);
  bool doJetPropagation = true;
  if (systTarget.find("Jet_En") != std::string::npos) {
    const bool doBreakdown = HasFlag("doBreakdown");
    if (!PrepareJetJESVariations(AllJetViews, systSource, doBreakdown))
      return false;
    if (!IsDATA)
      jesVar = systVar;
  } else if (systTarget == "Jet_Res") {
    if (!IsDATA)
      jerVar = systVar;
  } else if (systTarget == "UE") {
    MET = ev.GetMETVector(Event::MET_Type::PUPPI, systVar,
                          Event::MET_Syst::UE);
    doJetPropagation = false;
  }

  if (doJetPropagation)
    PropagateJetSystToMET(AllJetViews, MET, jesVar, jerVar);
  FillCutFlow(5);

  if (studyConfig_.requireMET20 && MET.Pt() < 20.f)
    return false;

  float Jet_Pt_Cut = loose_cut ? SL_Jet_Pt_cut - 5.f : SL_Jet_Pt_cut;
  std::vector<std::size_t> jetIndices =
      SelectJetIndices(AllJetViews, studyConfig_.jetID, Jet_Pt_Cut,
                       Jet_Eta_cut, jesVar, jerVar);

  const std::vector<std::size_t> cleaningMuonIndices =
      studyConfig_.jetCleaningMode == JetCleaningMode::LooseLeptons
          ? BuildLooseMuonCleaningIndices(Muons_indices)
          : Muons_indices;
  const std::vector<std::size_t> cleaningElectronIndices =
      studyConfig_.jetCleaningMode == JetCleaningMode::LooseLeptons
          ? BuildElectronCleaningIndices(Electrons_indices,
                                         Electron_Veto_indices)
          : Electrons_indices;

  jetIndices = JetsVetoLeptonInside(AllJetViews, jetIndices, AllElectronViews,
                                    cleaningElectronIndices, AllMuonViews,
                                    cleaningMuonIndices, Jet_Veto_DR);

  if (HasFlag("QuadJet")) {
    if (jetIndices.size() < 4)
      return false;
  } else {
    if (jetIndices.size() < 3)
      return false;
  }

  Jets = MaterializeJets(AllJetViews, jetIndices, jesVar, jerVar);
  std::sort(Jets.begin(), Jets.end(), PtComparing);

  HT = GetHT(Jets);
  n_jets = Jets.size();
  FillCutFlow(6);

  UpdateAllJetTaggingCaches(AllJetViews, jetIndices);
  short bWP_work = loose_cut ? 0 : 1;
  short cWP_work = loose_cut ? 0 : 1;

  for (const auto &jet : Jets) {
    if (GetPassedBTaggingWP(jet) >= bWP_work) n_b_tagged_jets++;
    if (GetPassedCTaggingWP(jet) >= cWP_work) n_c_tagged_jets++;
    
    if (!IsDATA) {
      if (std::abs(jet.hadronFlavour()) == 5) n_hadronFlav_b_jets++;
      if (std::abs(jet.hadronFlavour()) == 4) n_hadronFlav_c_jets++;
    }
  }
  n_hf_jets = n_b_tagged_jets + n_c_tagged_jets;
  FillCutFlow(7);

  if ((n_b_tagged_jets < 1 || n_hf_jets < 3) && !remove_flavtagging_cut)
    return false;
  FillCutFlow(8);

  SetTTbarId();
  SetSystematicLambda();
  return true;
}
