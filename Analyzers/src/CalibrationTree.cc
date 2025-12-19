#include "CalibrationTree.h"
#include "BranchManager.h"
#include "GenView.h"
#include "Jet.h"
#include "JetTaggingParameter.h"
#include "Muon.h"
#include "MyCorrection.h"
#include "TBranch.h"
#include "TEntryList.h"
#include "TEntryListArray.h"
#include "TObjArray.h"
#include "VcbParameters.h"
#include <TLorentzVector.h>
#include <algorithm>
#include <cctype>
#include <string>
#include <unordered_map>
#include <vector>

CalibrationTree::CalibrationTree() {}

void CalibrationTree::initializeAnalyzer() {
  SetChannel();
  string SKNANO_HOME = std::getenv("SKNANO_HOME");
  if (!IsDATA) {
    TString json_path = SKNANO_HOME + "/ModellingPatch/" + MCSample.Data() +
                        "_" + DataEra.Data() + "_summary.json";
    load_modelling_json(json_path);
  }
  if (IsDATA) {
    systHelper = std::make_unique<SystematicHelper>(
        SKNANO_HOME + "/AnalyzerTools/noSyst.yaml", DataStream, DataEra);
  } else {

    systHelper = std::make_unique<SystematicHelper>(
        SKNANO_HOME + "/AnalyzerTools/"
                      "CalibrationSystematic.yaml",
        MCSample, DataEra);
  }
  myCorr = new MyCorrection(DataEra, DataPeriod, IsDATA ? DataStream : MCSample,
                            IsDATA);
}

void CalibrationTree::SetChannel() {
  if (HasFlag("Skim")) {
    std::cout << "Skimming mode detected, will iterate over channel"
              << std::endl;
  } else if (HasFlag("TTDilep")) {
    channel = Channel::TTDilep;
  } else if (HasFlag("WCharm")) {
    channel = Channel::WCharm;
  } else if (HasFlag("DYLight")) {
    channel = Channel::DYLight;
  } else if (HasFlag("TTSemilep")) {
    channel = Channel::TTSemilep;
  } else {
    throw std::runtime_error(
        "Channel not specified! Please set one of the channel flags.");
  }
}

void CalibrationTree::FillHistogramsAtThisPoint(std::string_view histPrefix,
                                                float weight) {
  const std::string base(histPrefix);
  std::string name;
  name.reserve(base.size() + 64);

  auto fill1d = [&](std::string_view suffix, float value, float w, int nbin,
                    float xmin, float xmax) {
    name.assign(base);
    name.push_back('/');
    name.append(suffix);
    FillHist(name, value, w, nbin, xmin, xmax);
  };

  auto fill2d = [&](std::string_view suffix, float x, float y, float w,
                    int nbinx, float xmin, float xmax, int nbiny, float ymin,
                    float ymax) {
    name.assign(base);
    name.push_back('/');
    name.append(suffix);
    FillHist(name, x, y, w, nbinx, xmin, xmax, nbiny, ymin, ymax);
  };

  fill1d("MET_Pt", MET.Pt(), weight, 100, 0, 500);
  fill1d("MET_Phi", MET.Phi(), weight, 64, -3.2, 3.2);

  for (size_t idx = 0; idx < Jets.size(); ++idx) {
    const Jet &jet = Jets[idx];
    const short hardflav = IsDATA ? -1 : abs(jet.hadronFlavour());
    fill1d("Jet_" + to_string(idx) + "_Pt", jet.Pt(), weight, 100, 0, 500);
    fill1d("Jet_" + to_string(idx) + "_Eta", jet.Eta(), weight, 50, -2.5, 2.5);
    fill1d("Jet_" + to_string(idx) + "_Phi", jet.Phi(), weight, 64, -3.2, 3.2);
    fill1d("Jet_" + to_string(idx) + "_UParT_B",
           jet.GetTaggerResult(JetTagging::JetFlavTagger::ParT,
                               JetTagging::JetFlavTaggerScoreType::B),
           weight, 50, 0.f, 1.f);
    fill1d("Jet_" + to_string(idx) + "_UParT_CvB",
           jet.GetTaggerResult(JetTagging::JetFlavTagger::ParT,
                               JetTagging::JetFlavTaggerScoreType::CvB),
           weight, 50, 0.f, 1.f);
    fill1d("Jet_" + to_string(idx) + "_UParT_CvL",
           jet.GetTaggerResult(JetTagging::JetFlavTagger::ParT,
                               JetTagging::JetFlavTaggerScoreType::CvL),
           weight, 50, 0.f, 1.f);
    fill1d("Jet_" + to_string(idx) + "_UParT_CvNotB",
           jet.GetTaggerResult(JetTagging::JetFlavTagger::ParT,
                               JetTagging::JetFlavTaggerScoreType::CvNotB),
           weight, 50, 0.f, 1.f);
    fill1d("Jet_" + to_string(idx) + "_UParT_SvCB",
           jet.GetTaggerResult(JetTagging::JetFlavTagger::ParT,
                               JetTagging::JetFlavTaggerScoreType::SvCB),
           weight, 50, 0.f, 1.f);
    fill1d("Jet_" + to_string(idx) + "_UParT_SvUDG",
           jet.GetTaggerResult(JetTagging::JetFlavTagger::ParT,
                               JetTagging::JetFlavTaggerScoreType::SvUDG),
           weight, 50, 0.f, 1.f);
    fill1d("Jet_" + to_string(idx) + "_UParT_probUDG",
           jet.GetTaggerResult(JetTagging::JetFlavTagger::ParT,
                               JetTagging::JetFlavTaggerScoreType::probUDG),
           weight, 50, 0.f, 1.f);
    fill1d("Jet_" + to_string(idx) + "_UParT_probB",
           jet.GetTaggerResult(JetTagging::JetFlavTagger::ParT,
                               JetTagging::JetFlavTaggerScoreType::probB),
           weight, 50, 0.f, 1.f);
    fill1d("Jet_" + to_string(idx) + "_UParT_probBB",
           jet.GetTaggerResult(JetTagging::JetFlavTagger::ParT,
                               JetTagging::JetFlavTaggerScoreType::probBB),
           weight, 50, 0.f, 1.f);
    fill1d("Jet_" + to_string(idx) + "_hadronFlavour",
           static_cast<float>(hardflav), weight, 7, -1.f, 6.f);
  }
  for (size_t idx = 0; idx < Electrons.size(); ++idx) {
    const Electron &ele = Electrons[idx];
    fill1d("Electron_" + to_string(idx) + "_Pt", ele.Pt(), weight, 100, 0, 500);
    fill1d("Electron_" + to_string(idx) + "_Eta", ele.Eta(), weight, 50, -2.5,
           2.5);
    fill1d("Electron_" + to_string(idx) + "_Phi", ele.Phi(), weight, 64, -3.2,
           3.2);
  }

  for (size_t idx = 0; idx < Muons.size(); ++idx) {
    const Muon &mu = Muons[idx];
    fill1d("Muon_" + to_string(idx) + "_Pt", mu.Pt(), weight, 100, 0, 500);
    fill1d("Muon_" + to_string(idx) + "_Eta", mu.Eta(), weight, 50, -2.5, 2.5);
    fill1d("Muon_" + to_string(idx) + "_Phi", mu.Phi(), weight, 64, -3.2, 3.2);
  }
  if (Muons.size() >= 2) {
    TLorentzVector ZCand = Muons[0] + Muons[1];
    fill1d("Dimuon_Mass", ZCand.M(), weight, 100, 70, 110);
  }
}

void CalibrationTree::FillTreeAtThisPoint(
    std::string_view treePrefix, float MCNormalizationWeight,
    const std::unordered_map<std::string, float> &weight_map) {

  auto sanitize_branch_name = [](std::string_view raw) -> std::string {
    std::string sanitized;
    sanitized.reserve(raw.size());
    for (char c : raw) {
      const unsigned char uc = static_cast<unsigned char>(c);
      if (std::isalnum(uc) || c == '_') {
        sanitized.push_back(c);
      } else {
        sanitized.push_back('_');
      }
    }
    while (!sanitized.empty() && sanitized.front() == '_') {
      sanitized.erase(sanitized.begin());
    }
    while (!sanitized.empty() && sanitized.back() == '_') {
      sanitized.pop_back();
    }
    if (sanitized.empty())
      sanitized = "Tree";
    return sanitized;
  };

  const std::string tree_name = sanitize_branch_name(treePrefix);
  NewTree(tree_name, {}, {"*"});
  for (size_t i = 0; i < Jets.size(); ++i) {
    SetBranch(tree_name, "Jet_" + std::to_string(i) + "_Pt", Jets[i].Pt());
    SetBranch(tree_name, "Jet_" + std::to_string(i) + "_UParT_B",
              Jets[i].GetTaggerResult(JetTagging::JetFlavTagger::ParT,
                                      JetTagging::JetFlavTaggerScoreType::B));
    SetBranch(tree_name, "Jet_" + std::to_string(i) + "_UParT_CvB",
              Jets[i].GetTaggerResult(JetTagging::JetFlavTagger::ParT,
                                      JetTagging::JetFlavTaggerScoreType::CvB));
    SetBranch(tree_name, "Jet_" + std::to_string(i) + "_UParT_CvL",
              Jets[i].GetTaggerResult(JetTagging::JetFlavTagger::ParT,
                                      JetTagging::JetFlavTaggerScoreType::CvL));
    SetBranch(
        tree_name, "Jet_" + std::to_string(i) + "_UParT_CvNotB",
        Jets[i].GetTaggerResult(JetTagging::JetFlavTagger::ParT,
                                JetTagging::JetFlavTaggerScoreType::CvNotB));
    SetBranch(
        tree_name, "Jet_" + std::to_string(i) + "_UParT_SvCB",
        Jets[i].GetTaggerResult(JetTagging::JetFlavTagger::ParT,
                                JetTagging::JetFlavTaggerScoreType::SvCB));
    SetBranch(
        tree_name, "Jet_" + std::to_string(i) + "_UParT_SvUDG",
        Jets[i].GetTaggerResult(JetTagging::JetFlavTagger::ParT,
                                JetTagging::JetFlavTaggerScoreType::SvUDG));
    SetBranch(
        tree_name, "Jet_" + std::to_string(i) + "_UParT_probUDG",
        Jets[i].GetTaggerResult(JetTagging::JetFlavTagger::ParT,
                                JetTagging::JetFlavTaggerScoreType::probUDG));
    SetBranch(
        tree_name, "Jet_" + std::to_string(i) + "_UParT_probB",
        Jets[i].GetTaggerResult(JetTagging::JetFlavTagger::ParT,
                                JetTagging::JetFlavTaggerScoreType::probB));
    SetBranch(
        tree_name, "Jet_" + std::to_string(i) + "_UParT_probBB",
        Jets[i].GetTaggerResult(JetTagging::JetFlavTagger::ParT,
                                JetTagging::JetFlavTaggerScoreType::probBB));
    SetBranch(tree_name, "Jet_" + std::to_string(i) + "_hadronFlavour",
              IsDATA ? -1 : abs(Jets[i].hadronFlavour()));
  }
  for (const auto &kv : weight_map) {
    SetBranch(tree_name, "weight_" + kv.first, kv.second);
  }
  SetBranch(tree_name, "MCNormalization", MCNormalizationWeight);
  FillTrees(tree_name);
}

void CalibrationTree::executeEvent() {
  AllMuonViews = GetAllMuonViews();
  AllElectronViews = GetAllElectronViews();
  AllJetViews = GetAllJetViews();
  ev = GetEvent();
  for (const auto &syst_dummy : *systHelper) {
    leptons.clear();
    executeEventFromParameter();
  }
}

void CalibrationTree::SetSystematicLambda(bool remove_flavtagging_sf) {
  std::unordered_map<
      std::string,
      std::variant<std::function<float(MyCorrection::variation, TString)>,
                   std::function<float()>>>
      weight_function_map;
  std::function<float(MyCorrection::variation, TString)> mu_id_lambda =
      [&](MyCorrection::variation syst, TString /*source*/) {
        return myCorr->GetMuonIDSF(Mu_ID_SF_Key[DataEra.Data()], Muons, syst);
      };
  std::function<float(MyCorrection::variation, TString)> mu_iso_lambda =
      [&](MyCorrection::variation syst, TString source) {
        return myCorr->GetMuonISOSF(Mu_Iso_SF_Key[DataEra.Data()], Muons, syst,
                                    source);
      };
  std::function<float(MyCorrection::variation, TString)> mu_trigger_lambda =
      [&](MyCorrection::variation syst, TString source) {
        return LeptonTriggerWeight(false, syst, source);
      };
  std::function<float(MyCorrection::variation, TString)> el_id_lambda =
      [&](MyCorrection::variation syst, TString /*source*/) {
        return myCorr->GetElectronIDSF(El_ID_SF_Key[DataEra.Data()], Electrons,
                                       syst);
      };
  std::function<float(MyCorrection::variation, TString)> el_recosf_lambda =
      [&](MyCorrection::variation syst, TString /*source*/) {
        return myCorr->GetElectronRECOSF(Electrons, syst);
      };
  std::function<float(MyCorrection::variation, TString)> el_trigger_lambda =
      [&](MyCorrection::variation syst, TString source) {
        return LeptonTriggerWeight(true, syst, source);
      };

  std::function<float(MyCorrection::variation, TString)> pileup_lambda =
      [&](MyCorrection::variation syst, TString source) {
        return myCorr->GetPUWeight(ev.nTrueInt(), syst, source);
      };
  auto get_subproc_name = [this]() {
    if (IsDATA)
      return "data";
    else if (MCSample.Contains("TT")) {
      const int gent = genTtbarId % 100;
      if (gent >= 51 && gent <= 56)
        return "BB";
      else if (gent >= 41 && gent <= 46)
        return "CC";
      else
        return "JJ";
    } else
      return MCSample.Data();
  };
  std::function<float(MyCorrection::variation, TString)> MuF_lambda =
      [this, get_subproc_name](MyCorrection::variation syst,
                               TString /*source*/) {
        const std::string subproc_name = get_subproc_name();
        switch (syst) {
        case MyCorrection::variation::up:
          return GetScaleVariation(MyCorrection::variation::up,
                                   MyCorrection::variation::nom) *
                 modelling_patches[subproc_name].patch_ScaleVariation[4];
        case MyCorrection::variation::down:
          return GetScaleVariation(MyCorrection::variation::down,
                                   MyCorrection::variation::nom) *
                 modelling_patches[subproc_name].patch_ScaleVariation[3];
        default:
          return 1.f;
        }
      };

  std::function<float(MyCorrection::variation, TString)> MuR_lambda =
      [this, get_subproc_name](MyCorrection::variation syst,
                               TString /*source*/) {
        const std::string subproc_name = get_subproc_name();
        switch (syst) {
        case MyCorrection::variation::up:
          return GetScaleVariation(MyCorrection::variation::nom,
                                   MyCorrection::variation::up) *
                 modelling_patches[subproc_name].patch_ScaleVariation[6];
        case MyCorrection::variation::down:
          return GetScaleVariation(MyCorrection::variation::nom,
                                   MyCorrection::variation::down) *
                 modelling_patches[subproc_name].patch_ScaleVariation[1];
        default:
          return 1.f;
        }
      };

  std::function<float(MyCorrection::variation, TString)> ISR_lambda =
      [this, get_subproc_name](MyCorrection::variation syst,
                               TString /*source*/) {
        const std::string subproc_name = get_subproc_name();
        switch (syst) {
        case MyCorrection::variation::up:
          return GetPSWeight(MyCorrection::variation::up,
                             MyCorrection::variation::nom) *
                 modelling_patches[subproc_name].patch_PSVariation[0];
        case MyCorrection::variation::down:
          return GetPSWeight(MyCorrection::variation::down,
                             MyCorrection::variation::nom) *
                 modelling_patches[subproc_name].patch_PSVariation[2];
        default:
          return 1.f;
        }
      };

  std::function<float(MyCorrection::variation, TString)> FSR_lambda =
      [this, get_subproc_name](MyCorrection::variation syst,
                               TString /*source*/) {
        switch (syst) {
        case MyCorrection::variation::up:
          return GetPSWeight(MyCorrection::variation::nom,
                             MyCorrection::variation::up) *
                 modelling_patches[get_subproc_name()].patch_PSVariation[1];
        case MyCorrection::variation::down:
          return GetPSWeight(MyCorrection::variation::nom,
                             MyCorrection::variation::down) *
                 modelling_patches[get_subproc_name()].patch_PSVariation[3];
        default:
          return 1.f;
        }
      };

  std::function<float(MyCorrection::variation, TString)> BTag_lambda =
      [&](MyCorrection::variation syst, TString source) {
        float weight = 1.f;
        weight *= myCorr->GetBTaggingSF(
            Jets, JetTagging::JetTaggingSFMethod::shape, syst, source);
        weight *= myCorr->GetBTaggingR(Jets, Sample_Shorthand[MCSample.Data()],
                                       syst, source);
        return weight;
      };

  std::function<float(MyCorrection::variation, TString)> dummy_lambda =
      [&](MyCorrection::variation, TString /*source*/) { return 1.f; };

  std::function<float()> dummy_oneside_lambda = [&]() { return 1.f; };

  weight_function_map["Mu_ID"] = mu_id_lambda;
  weight_function_map["Mu_Iso"] = mu_iso_lambda;
  weight_function_map["Mu_Trig"] = mu_trigger_lambda;
  weight_function_map["El_ID"] = el_id_lambda;
  weight_function_map["El_Reco"] = el_recosf_lambda;
  weight_function_map["El_Trig"] = el_trigger_lambda;
  weight_function_map["Pileup"] = pileup_lambda;
  weight_function_map["MuF"] = MuF_lambda;
  weight_function_map["MuR"] = MuR_lambda;
  weight_function_map["ISR"] = ISR_lambda;
  weight_function_map["FSR"] = FSR_lambda;
  // weight_function_map["BFrag"] = dummy_lambda;

  if (remove_flavtagging_sf)
    weight_function_map["btag"] = dummy_lambda;
  else
    weight_function_map["btag"] = dummy_lambda;
  systHelper->assignWeightFunctionMap(weight_function_map);
}

void CalibrationTree::Clear() {
  HT = 0;
  n_jets = 0;
  n_b_tagged_jets = 0;
  n_c_tagged_jets = 0;
  n_hf_jets = 0;
  n_partonFlav_b_jets = 0;
  n_partonFlav_c_jets = 0;
  leptons.clear();
  Jets.clear();
  MET = Particle();
}

void CalibrationTree::executeEventFromParameter() {
  Clear();

  if (!PassBaseLineSelection())
    return;

  SetSystematicLambda();

  const std::string channel_str = GetChannelString(channel).Data();
  const std::string base_path = channel_str + "/Inclusive/";
  const std::string current_iter_name =
      systHelper ? systHelper->getCurrentSysName() : "Central";
  const std::string current_iter_prefix = base_path + current_iter_name + "/";

  std::string sample_postfix = Sample_Shorthand[MCSample.Data()];
  // -------------------------
  // DATA: Inclusive/Central/data_obs 만 채움
  // -------------------------
  if (IsDATA) {
    const std::unordered_map<std::string, float> data_weights = {
        {"Central", 1.f}};
    FillHistogramsAtThisPoint(base_path + "Central/data_obs", 1.f);
    FillTreeAtThisPoint(current_iter_prefix + "data_obs", 1.f, data_weights);

    return;
  }

  const float normalization = MCNormalization();
  const auto weight_map = systHelper->calculateWeight(false);
  FillTreeAtThisPoint(current_iter_prefix + Sample_Shorthand[MCSample.Data()],
                      normalization, weight_map);
  for (const auto &kv : weight_map) {
    const std::string &syst = kv.first;
    const float w = kv.second;
    FillHistogramsAtThisPoint(base_path + syst + "/" + sample_postfix,
                              w * normalization);
  }
}

float CalibrationTree::LeptonTriggerWeight(bool isEle,
                                           const MyCorrection::variation syst,
                                           const TString & /*source*/) {

  if (Muons.size() == 1 && Electrons.size() == 0)
    return myCorr->GetMuonTriggerSF(Mu_Trigger_SF_Key[DataEra.Data()], Muons[0],
                                    syst);

  else if (Muons.size() == 0 && Electrons.size() == 1)
    return myCorr->GetElectronTriggerSF(El_Trigger_SF_Key[DataEra.Data()],
                                        lepton.Eta(), lepton.Pt(), lepton.Phi(),
                                        syst);
  else if (Muons.size() == 1 && Electrons.size() == 1) {
    float mu_sf = myCorr->GetMuonTriggerSF(Mu_Trigger_SF_Key[DataEra.Data()],
                                           Muons[0], syst);
    float el_sf = myCorr->GetElectronTriggerSF(
        El_Trigger_SF_Key[DataEra.Data()], Electrons[0].Eta(),
        Electrons[0].Pt(), Electrons[0].Phi(), syst);
    return mu_sf * el_sf;
  }

  else if (Muons.size() == 2 && Electrons.size() == 0) {
    return myCorr->GetMuonTriggerSF(Mu_Trigger_SF_Key[DataEra.Data()], Muons[0],
                                    syst);
  }

  else
    throw std::runtime_error("Invalid lepton configuration for trigger "
                             "weight calculation. nMuons: " +
                             std::to_string(Muons.size()) + ", nElectrons: " +
                             std::to_string(Electrons.size()));
}

bool CalibrationTree::PassBaseLineSelection() {
  switch (channel) {
  case Channel::TTDilep:
    return PassTTDilepBaselineSelection();
  case Channel::WCharm:
    return PassWCharmBaselineSelection();
  case Channel::DYLight:
    return PassDYLightBaselineSelection();
  case Channel::TTSemilep:
    return PassTTSemilepBaselineSelection();
  default:
    throw std::runtime_error("Unknown channel for baseline selection.");
  }
}

bool CalibrationTree::PassTTDilepBaselineSelection() {
  if (!(ev.PassTrigger(Mu_Trigger[DataEra.Data()]) &&
        ev.PassTrigger(El_Trigger[DataEra.Data()])))
    return false;
  if (!PassJetVetoMap(AllJetViews, AllMuonViews))
    return false;
  if (!PassMetFilter(AllJetViews, ev))
    return false;
  std::vector<size_t> loose_muon_indices = SelectMuonIndices(
      AllMuonViews, Muon_Veto_ID, Muon_Veto_Pt, Muon_Veto_Eta);
  loose_muon_indices =
      SelectMuonIndices(AllMuonViews, loose_muon_indices, Muon_Veto_Iso,
                        Muon_Veto_Pt, Muon_Veto_Eta);
  std::vector<size_t> tight_muon_indices =
      SelectMuonIndices(AllMuonViews, loose_muon_indices, Muon_Tight_ID,
                        Muon_Tight_Pt[DataEra.Data()], Muon_Tight_Eta);
  tight_muon_indices =
      SelectMuonIndices(AllMuonViews, tight_muon_indices, Muon_Tight_Iso,
                        Muon_Tight_Pt[DataEra.Data()], Muon_Tight_Eta);
  std::vector<size_t> loose_electron_indices = SelectElectronIndices(
      AllElectronViews, Electron_Veto_ID, Electron_Veto_Pt, Electron_Veto_Eta);
  std::vector<size_t> tight_electron_indices = SelectElectronIndices(
      AllElectronViews, loose_electron_indices, Electron_Tight_ID,
      Electron_Tight_Pt[DataEra.Data()], Electron_Tight_Eta);
  if (!(tight_muon_indices.size() == 1 && tight_electron_indices.size() == 1 &&
        loose_muon_indices.size() == 1 && loose_electron_indices.size() == 1))
    return false;
  if ((AllMuonViews[tight_muon_indices[0]].Charge() *
       AllElectronViews[tight_electron_indices[0]].Charge()) > 0)
    return false;
  Muons = MaterializeMuons(AllMuonViews, tight_muon_indices);
  Electrons = MaterializeElectrons(AllElectronViews, tight_electron_indices);

  MET = ev.GetMETVector(Event::MET_Type::PUPPI);
  std::sort(Jets.begin(), Jets.end(), PtComparing);

  TLorentzVector p4_nominal(0, 0, 0, 0);
  TLorentzVector p4_shifted(0, 0, 0, 0);
  for (const auto &jetView : AllJetViews) {
    TLorentzVector v;
    v.SetPtEtaPhiM(jetView.Pt(), jetView.Eta(), jetView.Phi(), jetView.Mass());
    p4_nominal += v;
  }

  if (systHelper->getCurrentIterSysTarget().find("Jet_En") !=
      std::string::npos) {
    const bool doBreakdown = HasFlag("doBreakdown");
    const TString srcT = systHelper->getCurrentIterSysSource();
    if (doBreakdown) {
      if (srcT.EqualTo("total", TString::kIgnoreCase))
        return false;
      ApplyJetScaleVariation(AllJetViews, srcT);
    } else {
      if (!srcT.EqualTo("total", TString::kIgnoreCase))
        return false;
      ApplyJetScaleVariation(AllJetViews, "total");
    }

    if (systHelper->getCurrentIterVariation() == MyCorrection::variation::up) {
      for (const auto &jetView : AllJetViews) {
        TLorentzVector v;
        v.SetPtEtaPhiM(jetView.JesPtUp(), jetView.Eta(), jetView.Phi(),
                       jetView.JesMassUp());
        p4_shifted += v;
      }
    } else if (systHelper->getCurrentIterVariation() ==
               MyCorrection::variation::down) {
      for (const auto &jetView : AllJetViews) {
        TLorentzVector v;
        v.SetPtEtaPhiM(jetView.JesPtDown(), jetView.Eta(), jetView.Phi(),
                       jetView.JesMassDown());
        p4_shifted += v;
      }
    }
  } else if (systHelper->getCurrentIterSysTarget() == "Jet_Res") {
    MET = ev.GetMETVector(Event::MET_Type::PUPPI,
                          systHelper->getCurrentIterVariation(),
                          Event::MET_Syst::JER);
    p4_shifted = p4_nominal;
  } else if (systHelper->getCurrentIterSysTarget() == "UE") {
    MET = ev.GetMETVector(Event::MET_Type::PUPPI,
                          systHelper->getCurrentIterVariation(),
                          Event::MET_Syst::UE);
    p4_shifted = p4_nominal;
  } else {
    for (const auto &jetView : AllJetViews) {
      TLorentzVector v;
      v.SetPtEtaPhiM(jetView.SmearedPtNominal(), jetView.Eta(), jetView.Phi(),
                     jetView.SmearedMassNominal());
      p4_shifted += v;
    }
  }

  {
    TLorentzVector delta = p4_shifted - p4_nominal;
    MET.SetXYZM(MET.Px() - delta.Px(), MET.Py() - delta.Py(), 0., 0.);
  }
  if (MET.Pt() < 40.)
    return false;
  MyCorrection::variation jesVar = MyCorrection::variation::nom;
  MyCorrection::variation jerVar = MyCorrection::variation::nom;
  if (!IsDATA) {
    if (systHelper->getCurrentIterSysTarget().find("Jet_En") !=
        std::string::npos) {
      jesVar = systHelper->getCurrentIterVariation();
    } else if (systHelper->getCurrentIterSysTarget() == "Jet_Res") {
      jerVar = systHelper->getCurrentIterVariation();
    }
  }
  std::vector<size_t> jet_indices = SelectJetIndices(
      AllJetViews, Jet::JetID::TIGHT, 25., 2.4, jesVar, jerVar);
  jet_indices = JetsVetoLeptonInside(AllJetViews, jet_indices, AllElectronViews,
                                     tight_electron_indices, AllMuonViews,
                                     tight_muon_indices, 0.4);
  if (jet_indices.size() != 2)
    return false;
  Jets = MaterializeJets(AllJetViews, jet_indices, jesVar, jerVar);
  std::sort(Jets.begin(), Jets.end(), PtComparing);
  return true;
}

bool CalibrationTree::PassDYLightBaselineSelection() {
  if (!(ev.PassTrigger(Mu_Trigger[DataEra.Data()])))
    return false;
  if (!PassJetVetoMap(AllJetViews, AllMuonViews))
    return false;
  if (!PassMetFilter(AllJetViews, ev))
    return false;
  std::vector<size_t> loose_muon_indices = SelectMuonIndices(
      AllMuonViews, Muon_Veto_ID, Muon_Veto_Pt, Muon_Veto_Eta);
  loose_muon_indices =
      SelectMuonIndices(AllMuonViews, loose_muon_indices, Muon_Veto_Iso,
                        Muon_Veto_Pt, Muon_Veto_Eta);
  std::vector<size_t> tight_muon_indices =
      SelectMuonIndices(AllMuonViews, loose_muon_indices, Muon_Tight_ID,
                        Muon_Tight_Pt[DataEra.Data()], Muon_Tight_Eta);
  tight_muon_indices =
      SelectMuonIndices(AllMuonViews, tight_muon_indices, Muon_Tight_Iso,
                        Muon_Tight_Pt[DataEra.Data()], Muon_Tight_Eta);
  std::vector<size_t> loose_electron_indices = SelectElectronIndices(
      AllElectronViews, Electron_Veto_ID, Electron_Veto_Pt, Electron_Veto_Eta);
  std::vector<size_t> tight_electron_indices = SelectElectronIndices(
      AllElectronViews, loose_electron_indices, Electron_Tight_ID,
      Electron_Tight_Pt[DataEra.Data()], Electron_Tight_Eta);
  if (!(tight_muon_indices.size() == 2 && tight_electron_indices.size() == 0 &&
        loose_muon_indices.size() == 2 && loose_electron_indices.size() == 0))
    return false;
  if ((AllMuonViews[tight_muon_indices[0]].Charge() *
       AllMuonViews[tight_muon_indices[1]].Charge()) > 0)
    return false;
  Muons = MaterializeMuons(AllMuonViews, tight_muon_indices);
  std::sort(Muons.begin(), Muons.end(), PtComparing);
  Particle ZCand = Muons[0] + Muons[1];
  if (ZCand.M() < 81. || ZCand.M() > 101.)
    return false;

  MET = ev.GetMETVector(Event::MET_Type::PUPPI);

  TLorentzVector p4_nominal(0, 0, 0, 0);
  TLorentzVector p4_shifted(0, 0, 0, 0);
  for (const auto &jetView : AllJetViews) {
    TLorentzVector v;
    v.SetPtEtaPhiM(jetView.Pt(), jetView.Eta(), jetView.Phi(), jetView.Mass());
    p4_nominal += v;
  }

  if (systHelper->getCurrentIterSysTarget().find("Jet_En") !=
      std::string::npos) {
    const bool doBreakdown = HasFlag("doBreakdown");
    const TString srcT = systHelper->getCurrentIterSysSource();
    if (doBreakdown) {
      if (srcT.EqualTo("total", TString::kIgnoreCase))
        return false;
      ApplyJetScaleVariation(AllJetViews, srcT);
    } else {
      if (!srcT.EqualTo("total", TString::kIgnoreCase))
        return false;
      ApplyJetScaleVariation(AllJetViews, "total");
    }

    if (systHelper->getCurrentIterVariation() == MyCorrection::variation::up) {
      for (const auto &jetView : AllJetViews) {
        TLorentzVector v;
        v.SetPtEtaPhiM(jetView.JesPtUp(), jetView.Eta(), jetView.Phi(),
                       jetView.JesMassUp());
        p4_shifted += v;
      }
    } else if (systHelper->getCurrentIterVariation() ==
               MyCorrection::variation::down) {
      for (const auto &jetView : AllJetViews) {
        TLorentzVector v;
        v.SetPtEtaPhiM(jetView.JesPtDown(), jetView.Eta(), jetView.Phi(),
                       jetView.JesMassDown());
        p4_shifted += v;
      }
    }
  } else if (systHelper->getCurrentIterSysTarget() == "Jet_Res") {
    MET = ev.GetMETVector(Event::MET_Type::PUPPI,
                          systHelper->getCurrentIterVariation(),
                          Event::MET_Syst::JER);
    p4_shifted = p4_nominal;
  } else if (systHelper->getCurrentIterSysTarget() == "UE") {
    MET = ev.GetMETVector(Event::MET_Type::PUPPI,
                          systHelper->getCurrentIterVariation(),
                          Event::MET_Syst::UE);
    p4_shifted = p4_nominal;
  } else {
    for (const auto &jetView : AllJetViews) {
      TLorentzVector v;
      v.SetPtEtaPhiM(jetView.SmearedPtNominal(), jetView.Eta(), jetView.Phi(),
                     jetView.SmearedMassNominal());
      p4_shifted += v;
    }
  }

  {
    TLorentzVector delta = p4_shifted - p4_nominal;
    MET.SetXYZM(MET.Px() - delta.Px(), MET.Py() - delta.Py(), 0., 0.);
  }
  MyCorrection::variation jesVar = MyCorrection::variation::nom;
  MyCorrection::variation jerVar = MyCorrection::variation::nom;
  if (!IsDATA) {
    if (systHelper->getCurrentIterSysTarget().find("Jet_En") !=
        std::string::npos) {
      jesVar = systHelper->getCurrentIterVariation();
    } else if (systHelper->getCurrentIterSysTarget() == "Jet_Res") {
      jerVar = systHelper->getCurrentIterVariation();
    }
  }
  std::vector<size_t> jet_indices = SelectJetIndices(
      AllJetViews, Jet::JetID::TIGHT, 25., 2.4, jesVar, jerVar);
  jet_indices = JetsVetoLeptonInside(AllJetViews, jet_indices, AllElectronViews,
                                     tight_electron_indices, AllMuonViews,
                                     tight_muon_indices, 0.4);
  if (jet_indices.size() != 1)
    return false;
  Jets = MaterializeJets(AllJetViews, jet_indices, jesVar, jerVar);
  std::sort(Jets.begin(), Jets.end(), PtComparing);
  if (ZCand.Pt() / Jets[0].Pt() < 0.75 || ZCand.Pt() / Jets[0].Pt() > 1.25)
    return false;
  if (ZCand.DeltaPhi(Jets[0]) < 2.f)
    return false;
  return true;
}

bool CalibrationTree::PassWCharmBaselineSelection() {
  if (!(ev.PassTrigger(Mu_Trigger[DataEra.Data()])))
    return false;
  if (!PassJetVetoMap(AllJetViews, AllMuonViews))
    return false;
  if (!PassMetFilter(AllJetViews, ev))
    return false;
  std::vector<size_t> loose_muon_indices = SelectMuonIndices(
      AllMuonViews, Muon_Veto_ID, Muon_Veto_Pt, Muon_Veto_Eta);
  loose_muon_indices =
      SelectMuonIndices(AllMuonViews, loose_muon_indices, Muon_Veto_Iso,
                        Muon_Veto_Pt, Muon_Veto_Eta);
  std::vector<size_t> tight_muon_indices =
      SelectMuonIndices(AllMuonViews, loose_muon_indices, Muon_Tight_ID,
                        Muon_Tight_Pt[DataEra.Data()], Muon_Tight_Eta);
  tight_muon_indices =
      SelectMuonIndices(AllMuonViews, tight_muon_indices, Muon_Tight_Iso,
                        Muon_Tight_Pt[DataEra.Data()], Muon_Tight_Eta);
  std::vector<size_t> loose_electron_indices = SelectElectronIndices(
      AllElectronViews, Electron_Veto_ID, Electron_Veto_Pt, Electron_Veto_Eta);
  std::vector<size_t> tight_electron_indices = SelectElectronIndices(
      AllElectronViews, loose_electron_indices, Electron_Tight_ID,
      Electron_Tight_Pt[DataEra.Data()], Electron_Tight_Eta);
  std::vector<size_t> soft_muon_indices =
      SelectMuonIndices(AllMuonViews, Muon_Tight_ID, 5., Muon_Tight_Eta);
  std::vector<size_t> soft_muon_indices_iso = SelectMuonIndices(
      AllMuonViews, soft_muon_indices, Muon_Tight_Iso, 5., Muon_Tight_Eta);
  std::vector<size_t> soft_muon_indices_noniso;
  for (auto idx : soft_muon_indices) {
    if (std::find(soft_muon_indices_iso.begin(), soft_muon_indices_iso.end(),
                  idx) == soft_muon_indices_iso.end())
      soft_muon_indices_noniso.push_back(idx);
  }

  if (!(tight_muon_indices.size() == 1 && tight_electron_indices.size() == 0 &&
        loose_muon_indices.size() == 1 && loose_electron_indices.size() == 0))
    return false;

  MyCorrection::variation jesVar = MyCorrection::variation::nom;
  MyCorrection::variation jerVar = MyCorrection::variation::nom;
  if (!IsDATA) {
    if (systHelper->getCurrentIterSysTarget().find("Jet_En") !=
        std::string::npos) {
      jesVar = systHelper->getCurrentIterVariation();
    } else if (systHelper->getCurrentIterSysTarget() == "Jet_Res") {
      jerVar = systHelper->getCurrentIterVariation();
    }
  }
  std::vector<size_t> jet_indices = SelectJetIndices(
      AllJetViews, Jet::JetID::TIGHT, 25., 2.4, jesVar, jerVar);
  jet_indices = JetsVetoLeptonInside(AllJetViews, jet_indices, AllElectronViews,
                                     tight_electron_indices, AllMuonViews,
                                     tight_muon_indices, 0.4);
  if (jet_indices.size() != 1)
    return false;
  Jets = MaterializeJets(AllJetViews, jet_indices, jesVar, jerVar);
  std::sort(Jets.begin(), Jets.end(), PtComparing);

  auto deltaR2 = [](float eta1, float phi1, float eta2, float phi2) {
    const float deta = eta1 - eta2;
    float dphi = phi1 - phi2;

    // [-pi, pi] 범위로 접어주기
    while (dphi > M_PI)
      dphi -= 2.f * M_PI;
    while (dphi < -M_PI)
      dphi += 2.f * M_PI;

    return deta * deta + dphi * dphi;
  };

  constexpr float dR2_cut = 0.4f * 0.4f;
  constexpr size_t kAmbiguous = std::numeric_limits<size_t>::max();

  // jet 하나당 "유일한" soft-muon 하나를 임시로 저장하는 map
  //   - 첫 번째로 붙은 뮤온: mu_idx 저장
  //   - 두 번째부터 붙으면: kAmbiguous 로 마킹 (여러 개 붙은 jet)
  std::unordered_map<size_t, size_t> jet_to_mu;
  jet_to_mu.reserve(soft_muon_indices_noniso.size());

  for (auto mu_idx : soft_muon_indices_noniso) {
    const float mu_eta = AllMuonViews[mu_idx].Eta();
    const float mu_phi = AllMuonViews[mu_idx].Phi();

    float best_dR2 = dR2_cut;
    size_t best_jet_idx = std::numeric_limits<size_t>::max();

    for (auto jet_idx : jet_indices) {
      const float jet_eta = AllJetViews[jet_idx].Eta();
      const float jet_phi = AllJetViews[jet_idx].Phi();

      const float dR2 = deltaR2(jet_eta, jet_phi, mu_eta, mu_phi);
      if (dR2 < best_dR2) {
        best_dR2 = dR2;
        best_jet_idx = jet_idx;
      }
    }

    if (best_jet_idx != std::numeric_limits<size_t>::max()) {
      auto it = jet_to_mu.find(best_jet_idx);
      if (it == jet_to_mu.end()) {
        // 이 jet에 붙은 첫 번째 soft-muon
        jet_to_mu.emplace(best_jet_idx, mu_idx);
      } else {
        // 이미 하나 있었는데 또 들어오면 -> 이 jet은 "여러 뮤온"이라 ambiguous
        it->second = kAmbiguous;
      }
    }
  }

  // 이제 "뮤온이 정확히 하나만 붙은 jet" 만 골라서 결과 벡터에 담기
  std::vector<size_t> unique_jet_indices;
  std::vector<size_t> unique_softmuon_indices;
  unique_jet_indices.reserve(jet_to_mu.size());
  unique_softmuon_indices.reserve(jet_to_mu.size());

  for (const auto &[jet_idx, mu_idx] : jet_to_mu) {
    if (mu_idx == kAmbiguous)
      continue; // 여러 개 붙은 jet는 스킵
    unique_jet_indices.push_back(jet_idx);
    unique_softmuon_indices.push_back(mu_idx);
  }

  if (unique_jet_indices.size() != 1 || jet_indices.size() != 1)
    return false;
  Muons = MaterializeMuons(AllMuonViews, tight_muon_indices);

  if (Jets[0].muEF() > 0.5f || Jets[0].muEF() + Jets[0].neHEF() > 0.7f)
    return false;

  Muon soft_muon = MaterializeMuons(AllMuonViews, unique_softmuon_indices)[0];
  Particle DiMuon = Muons[0] + soft_muon;
  if (DiMuon.M() < 12. || (DiMuon.M() > 81. && DiMuon.M() < 101.))
    return false;
  if (Muons[0].Charge() * soft_muon.Charge() > 0)
    return false;

  MET = ev.GetMETVector(Event::MET_Type::PUPPI);

  TLorentzVector p4_nominal(0, 0, 0, 0);
  TLorentzVector p4_shifted(0, 0, 0, 0);
  for (const auto &jetView : AllJetViews) {
    TLorentzVector v;
    v.SetPtEtaPhiM(jetView.Pt(), jetView.Eta(), jetView.Phi(), jetView.Mass());
    p4_nominal += v;
  }

  if (systHelper->getCurrentIterSysTarget().find("Jet_En") !=
      std::string::npos) {
    const bool doBreakdown = HasFlag("doBreakdown");
    const TString srcT = systHelper->getCurrentIterSysSource();
    if (doBreakdown) {
      if (srcT.EqualTo("total", TString::kIgnoreCase))
        return false;
      ApplyJetScaleVariation(AllJetViews, srcT);
    } else {
      if (!srcT.EqualTo("total", TString::kIgnoreCase))
        return false;
      ApplyJetScaleVariation(AllJetViews, "total");
    }

    if (systHelper->getCurrentIterVariation() == MyCorrection::variation::up) {
      for (const auto &jetView : AllJetViews) {
        TLorentzVector v;
        v.SetPtEtaPhiM(jetView.JesPtUp(), jetView.Eta(), jetView.Phi(),
                       jetView.JesMassUp());
        p4_shifted += v;
      }
    } else if (systHelper->getCurrentIterVariation() ==
               MyCorrection::variation::down) {
      for (const auto &jetView : AllJetViews) {
        TLorentzVector v;
        v.SetPtEtaPhiM(jetView.JesPtDown(), jetView.Eta(), jetView.Phi(),
                       jetView.JesMassDown());
        p4_shifted += v;
      }
    }
  } else if (systHelper->getCurrentIterSysTarget() == "Jet_Res") {
    MET = ev.GetMETVector(Event::MET_Type::PUPPI,
                          systHelper->getCurrentIterVariation(),
                          Event::MET_Syst::JER);
    p4_shifted = p4_nominal;
  } else if (systHelper->getCurrentIterSysTarget() == "UE") {
    MET = ev.GetMETVector(Event::MET_Type::PUPPI,
                          systHelper->getCurrentIterVariation(),
                          Event::MET_Syst::UE);
    p4_shifted = p4_nominal;
  } else {
    for (const auto &jetView : AllJetViews) {
      TLorentzVector v;
      v.SetPtEtaPhiM(jetView.SmearedPtNominal(), jetView.Eta(), jetView.Phi(),
                     jetView.SmearedMassNominal());
      p4_shifted += v;
    }
  }

  {
    TLorentzVector delta = p4_shifted - p4_nominal;
    MET.SetXYZM(MET.Px() - delta.Px(), MET.Py() - delta.Py(), 0., 0.);
  }

  if (MET.Pt() < 30.f)
    return false;

  Particle WT = Muons[0] + MET;
  float MT = std::sqrt( 2.f * Muons[0].Pt() * MET.Pt() * (1.f - std::cos(Muons[0].DeltaPhi(MET))) );
  if (MT < 40.f || MT > 120.f)
    return false;

  if (WT.Pt() < 30.f)
    return false;

  if (std::abs(MET.DeltaPhi(Jets[0])) < 1.0)
    return false;
  if (std::abs(Muons[0].DeltaPhi(Jets[0])) > 2.8)
    return false;
  if (std::abs(WT.DeltaPhi(Jets[0])) < 2.0)
    return false;
  const float pt_ratio = Jets[0].Pt() / WT.Pt();
  if (pt_ratio < 0.5f || pt_ratio > 2.0f)
    return false;

  return true;
}

bool CalibrationTree::PassTTSemilepBaselineSelection() {
  if (!(ev.PassTrigger(Mu_Trigger[DataEra.Data()])))
    return false;
  if (!PassJetVetoMap(AllJetViews, AllMuonViews))
    return false;
  if (!PassMetFilter(AllJetViews, ev))
    return false;
  std::vector<size_t> loose_muon_indices = SelectMuonIndices(
      AllMuonViews, Muon_Veto_ID, Muon_Veto_Pt, Muon_Veto_Eta);
  loose_muon_indices =
      SelectMuonIndices(AllMuonViews, loose_muon_indices, Muon_Veto_Iso,
                        Muon_Veto_Pt, Muon_Veto_Eta);
  std::vector<size_t> tight_muon_indices =
      SelectMuonIndices(AllMuonViews, loose_muon_indices, Muon_Tight_ID,
                        Muon_Tight_Pt[DataEra.Data()], Muon_Tight_Eta);
  tight_muon_indices =
      SelectMuonIndices(AllMuonViews, tight_muon_indices, Muon_Tight_Iso,
                        Muon_Tight_Pt[DataEra.Data()], Muon_Tight_Eta);
  std::vector<size_t> loose_electron_indices = SelectElectronIndices(
      AllElectronViews, Electron_Veto_ID, Electron_Veto_Pt, Electron_Veto_Eta);
  std::vector<size_t> tight_electron_indices = SelectElectronIndices(
      AllElectronViews, loose_electron_indices, Electron_Tight_ID,
      Electron_Tight_Pt[DataEra.Data()], Electron_Tight_Eta);
  if (!(tight_muon_indices.size() == 1 && tight_electron_indices.size() == 0 &&
        loose_muon_indices.size() == 1 && loose_electron_indices.size() == 0))
    return false;
  Muons = MaterializeMuons(AllMuonViews, tight_muon_indices);

  MET = ev.GetMETVector(Event::MET_Type::PUPPI);

  TLorentzVector p4_nominal(0, 0, 0, 0);
  TLorentzVector p4_shifted(0, 0, 0, 0);
  for (const auto &jetView : AllJetViews) {
    TLorentzVector v;
    v.SetPtEtaPhiM(jetView.Pt(), jetView.Eta(), jetView.Phi(), jetView.Mass());
    p4_nominal += v;
  }

  if (systHelper->getCurrentIterSysTarget().find("Jet_En") !=
      std::string::npos) {
    const bool doBreakdown = HasFlag("doBreakdown");
    const TString srcT = systHelper->getCurrentIterSysSource();
    if (doBreakdown) {
      if (srcT.EqualTo("total", TString::kIgnoreCase))
        return false;
      ApplyJetScaleVariation(AllJetViews, srcT);
    } else {
      if (!srcT.EqualTo("total", TString::kIgnoreCase))
        return false;
      ApplyJetScaleVariation(AllJetViews, "total");
    }

    if (systHelper->getCurrentIterVariation() == MyCorrection::variation::up) {
      for (const auto &jetView : AllJetViews) {
        TLorentzVector v;
        v.SetPtEtaPhiM(jetView.JesPtUp(), jetView.Eta(), jetView.Phi(),
                       jetView.JesMassUp());
        p4_shifted += v;
      }
    } else if (systHelper->getCurrentIterVariation() ==
               MyCorrection::variation::down) {
      for (const auto &jetView : AllJetViews) {
        TLorentzVector v;
        v.SetPtEtaPhiM(jetView.JesPtDown(), jetView.Eta(), jetView.Phi(),
                       jetView.JesMassDown());
        p4_shifted += v;
      }
    }
  } else if (systHelper->getCurrentIterSysTarget() == "Jet_Res") {
    MET = ev.GetMETVector(Event::MET_Type::PUPPI,
                          systHelper->getCurrentIterVariation(),
                          Event::MET_Syst::JER);
    p4_shifted = p4_nominal;
  } else if (systHelper->getCurrentIterSysTarget() == "UE") {
    MET = ev.GetMETVector(Event::MET_Type::PUPPI,
                          systHelper->getCurrentIterVariation(),
                          Event::MET_Syst::UE);
    p4_shifted = p4_nominal;
  } else {
    for (const auto &jetView : AllJetViews) {
      TLorentzVector v;
      v.SetPtEtaPhiM(jetView.SmearedPtNominal(), jetView.Eta(), jetView.Phi(),
                     jetView.SmearedMassNominal());
      p4_shifted += v;
    }
  }

  {
    TLorentzVector delta = p4_shifted - p4_nominal;
    MET.SetXYZM(MET.Px() - delta.Px(), MET.Py() - delta.Py(), 0., 0.);
  }
  MyCorrection::variation jesVar = MyCorrection::variation::nom;
  MyCorrection::variation jerVar = MyCorrection::variation::nom;
  if (!IsDATA) {
    if (systHelper->getCurrentIterSysTarget().find("Jet_En") !=
        std::string::npos) {
      jesVar = systHelper->getCurrentIterVariation();
    } else if (systHelper->getCurrentIterSysTarget() == "Jet_Res") {
      jerVar = systHelper->getCurrentIterVariation();
    }
  }
  std::vector<size_t> jet_indices = SelectJetIndices(
      AllJetViews, Jet::JetID::TIGHT, 25., 2.4, jesVar, jerVar);
  jet_indices = JetsVetoLeptonInside(AllJetViews, jet_indices, AllElectronViews,
                                     tight_electron_indices, AllMuonViews,
                                     tight_muon_indices, 0.4);
  if (jet_indices.size() != 4)
    return false;
  Jets = MaterializeJets(AllJetViews, jet_indices, jesVar, jerVar);
  std::sort(Jets.begin(), Jets.end(), PtComparing);
  // check all jets seperated by dR>0.8
  for (size_t i = 0; i < Jets.size(); ++i) {
    for (size_t j = i + 1; j < Jets.size(); ++j) {
      if (Jets[i].DeltaR(Jets[j]) < 0.8f)
        return false;
    }
  }
  // check least one b-tagged jet
  bool has_btagged_jet = false;
  for (const auto &jet : Jets) {
    if (jet.GetTaggerResult(JetTagging::JetFlavTagger::ParT,
                            JetTagging::JetFlavTaggerScoreType::B) >
        myCorr->GetBTaggingWP(JetTagging::JetFlavTagger::ParT,
                              JetTagging::JetFlavTaggerWP::Tight)) {
      has_btagged_jet = true;
      break;
    }
  }
  if (!has_btagged_jet)
    return false;
  return true;
}