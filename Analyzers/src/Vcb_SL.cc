#include "Vcb_SL.h"
#include "VcbParameters.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <limits>
#include <numeric>
#include <stdexcept>

namespace {
constexpr int kTabNetEventContextFeatDim = 5;
constexpr int kWcbNNNumFolds = 5;
constexpr int kWcbNNMaxJets = 8;
constexpr int kWcbNNJetFeatureDim = 13;
constexpr int kWcbNNJetVectorDim = 4;
constexpr int kWcbNNLepFeatureDim = 5;
constexpr int kWcbNNLepVectorDim = 4;
constexpr int kWcbNNLepSequenceLen = 2;
constexpr const char *kWcbNNModelDir =
    "/data6/Users/yeonjoon/CMSSW_15_0_10/src/PhysicsTools/NanoTTH/data/nn/"
    "1L_Wcb/v4";
constexpr std::array<const char *, 8> kWcbNNScoreBranchNames = {
    "score_tt_Wcb", "score_ttLF", "score_ttcj", "score_tt2c",
    "score_ttcc",   "score_ttbj", "score_tt2b", "score_ttbb",
};
constexpr std::array<const char *, 8> kWcbNNPreSoftmaxBranchNames = {
    "pre_softmax_tt_Wcb", "pre_softmax_ttLF", "pre_softmax_ttcj",
    "pre_softmax_tt2c",   "pre_softmax_ttcc", "pre_softmax_ttbj",
    "pre_softmax_tt2b",   "pre_softmax_ttbb",
};

struct WcbNNResult {
  std::array<float, kWcbNNScoreBranchNames.size()> scores = {};
  std::array<float, kWcbNNScoreBranchNames.size()> pre_softmax = {};
  int category = -1;
};

float safe_log_for_wcb_nn(float value) {
  return (value > 0.f && std::isfinite(value)) ? std::log(value) : 0.f;
}

void append_preprocessed_feature(FloatArray &dest,
                                 const std::vector<float> &src,
                                 int target_length, float median,
                                 float norm_factor, float lower_bound,
                                 float upper_bound) {
  for (int i = 0; i < target_length; ++i) {
    if (i < static_cast<int>(src.size())) {
      float value = (src[i] - median) * norm_factor;
      if (!std::isfinite(value))
        value = 0.f;
      dest.push_back(std::clamp(value, lower_bound, upper_bound));
    } else {
      dest.push_back(0.f);
    }
  }
}

std::array<float, kWcbNNScoreBranchNames.size()>
derive_centered_logits_from_scores(
    const std::array<float, kWcbNNScoreBranchNames.size()> &scores) {
  constexpr float kProbFloor = 1.e-12f;
  std::array<float, kWcbNNScoreBranchNames.size()> logits = {};

  float prob_sum = 0.f;
  for (std::size_t i = 0; i < scores.size(); ++i) {
    const float clipped_score = std::max(scores[i], kProbFloor);
    logits[i] = clipped_score;
    prob_sum += clipped_score;
  }
  if (!(prob_sum > 0.f) || !std::isfinite(prob_sum)) {
    logits.fill(0.f);
    return logits;
  }

  float mean_logit = 0.f;
  for (std::size_t i = 0; i < logits.size(); ++i) {
    logits[i] = std::log(logits[i] / prob_sum);
    mean_logit += logits[i];
  }
  mean_logit /= static_cast<float>(logits.size());
  for (float &value : logits) {
    value -= mean_logit;
  }
  return logits;
}

const FloatArray &
extract_single_output_scores(const std::unordered_map<std::string, FloatArray>
                                 &output_data,
                             const MLHelper &helper) {
  const auto output_names = helper.GetOutputNames();
  if (!output_names.empty()) {
    const auto it = output_data.find(output_names.front());
    if (it != output_data.end())
      return it->second;
  }
  if (output_data.size() == 1)
    return output_data.begin()->second;
  throw std::runtime_error(
      "[Vcb_SL] Failed to resolve the Wcb NN output tensor.");
}

const std::vector<std::unique_ptr<MLHelper>> &get_wcb_nn_fold_helpers() {
  static const auto helpers = [] {
    std::vector<std::unique_ptr<MLHelper>> loaded_helpers;
    loaded_helpers.reserve(kWcbNNNumFolds);
    for (int i = 0; i < kWcbNNNumFolds; ++i) {
      loaded_helpers.emplace_back(std::make_unique<MLHelper>(
          std::string(kWcbNNModelDir) + "/net." + std::to_string(i) + ".onnx",
          MLHelper::ModelType::ONNX));
    }
    return loaded_helpers;
  }();
  return helpers;
}

bool use_single_wcb_nn_fold(const TString &data_era) {
  return data_era.Contains("2024");
}

WcbNNResult evaluate_wcb_nn(const Vcb_SL &analyzer,
                           unsigned long long event_number) {
  std::vector<float> ak4_pt_log;
  std::vector<float> ak4_energy_log;
  std::vector<float> ak4_eta;
  std::vector<float> ak4_tag_B4;
  std::vector<float> ak4_tag_B3;
  std::vector<float> ak4_tag_B2;
  std::vector<float> ak4_tag_B1;
  std::vector<float> ak4_tag_B0;
  std::vector<float> ak4_tag_C4;
  std::vector<float> ak4_tag_C3;
  std::vector<float> ak4_tag_C2;
  std::vector<float> ak4_tag_C1;
  std::vector<float> ak4_tag_C0;
  std::vector<float> ak4_px;
  std::vector<float> ak4_py;
  std::vector<float> ak4_pz;
  std::vector<float> ak4_energy;

  const auto reserve_jets = analyzer.Jets.size();
  ak4_pt_log.reserve(reserve_jets);
  ak4_energy_log.reserve(reserve_jets);
  ak4_eta.reserve(reserve_jets);
  ak4_tag_B4.reserve(reserve_jets);
  ak4_tag_B3.reserve(reserve_jets);
  ak4_tag_B2.reserve(reserve_jets);
  ak4_tag_B1.reserve(reserve_jets);
  ak4_tag_B0.reserve(reserve_jets);
  ak4_tag_C4.reserve(reserve_jets);
  ak4_tag_C3.reserve(reserve_jets);
  ak4_tag_C2.reserve(reserve_jets);
  ak4_tag_C1.reserve(reserve_jets);
  ak4_tag_C0.reserve(reserve_jets);
  ak4_px.reserve(reserve_jets);
  ak4_py.reserve(reserve_jets);
  ak4_pz.reserve(reserve_jets);
  ak4_energy.reserve(reserve_jets);

  for (const auto &jet : analyzer.Jets) {
    const int jet_cat = static_cast<int>(analyzer.JetCategory(jet));
    ak4_pt_log.push_back(safe_log_for_wcb_nn(jet.Pt()));
    ak4_energy_log.push_back(safe_log_for_wcb_nn(jet.E()));
    ak4_eta.push_back(jet.Eta());
    ak4_tag_B4.push_back(jet_cat == 11 ? 1.f : 0.f);
    ak4_tag_B3.push_back(jet_cat == 10 ? 1.f : 0.f);
    ak4_tag_B2.push_back(jet_cat == 9 ? 1.f : 0.f);
    ak4_tag_B1.push_back(jet_cat == 8 ? 1.f : 0.f);
    ak4_tag_B0.push_back(jet_cat == 7 ? 1.f : 0.f);
    ak4_tag_C4.push_back(jet_cat == 6 ? 1.f : 0.f);
    ak4_tag_C3.push_back(jet_cat == 5 ? 1.f : 0.f);
    ak4_tag_C2.push_back(jet_cat == 4 ? 1.f : 0.f);
    ak4_tag_C1.push_back(jet_cat == 3 ? 1.f : 0.f);
    ak4_tag_C0.push_back(jet_cat == 2 ? 1.f : 0.f);
    ak4_px.push_back(jet.Px());
    ak4_py.push_back(jet.Py());
    ak4_pz.push_back(jet.Pz());
    ak4_energy.push_back(jet.E());
  }
  std::vector<float> ak4_mask(analyzer.Jets.size(), 1.f);

  std::vector<float> lep_pt_log;
  std::vector<float> lep_energy_log;
  std::vector<float> lep_eta;
  std::vector<float> lep_isMu;
  std::vector<float> lep_isEl;
  std::vector<float> lep_px;
  std::vector<float> lep_py;
  std::vector<float> lep_pz;
  std::vector<float> lep_energy;
  lep_pt_log.reserve(kWcbNNLepSequenceLen);
  lep_energy_log.reserve(kWcbNNLepSequenceLen);
  lep_eta.reserve(kWcbNNLepSequenceLen);
  lep_isMu.reserve(kWcbNNLepSequenceLen);
  lep_isEl.reserve(kWcbNNLepSequenceLen);
  lep_px.reserve(kWcbNNLepSequenceLen);
  lep_py.reserve(kWcbNNLepSequenceLen);
  lep_pz.reserve(kWcbNNLepSequenceLen);
  lep_energy.reserve(kWcbNNLepSequenceLen);

  lep_pt_log.push_back(safe_log_for_wcb_nn(analyzer.lepton.Pt()));
  lep_energy_log.push_back(safe_log_for_wcb_nn(analyzer.lepton.E()));
  lep_eta.push_back(analyzer.lepton.Eta());
  lep_isMu.push_back(analyzer.channel == Vcb::Channel::Mu ? 1.f : 0.f);
  lep_isEl.push_back(analyzer.channel == Vcb::Channel::El ? 1.f : 0.f);
  lep_px.push_back(analyzer.lepton.Px());
  lep_py.push_back(analyzer.lepton.Py());
  lep_pz.push_back(analyzer.lepton.Pz());
  lep_energy.push_back(analyzer.lepton.E());

  lep_pt_log.push_back(safe_log_for_wcb_nn(analyzer.MET.Pt()));
  lep_energy_log.push_back(safe_log_for_wcb_nn(analyzer.MET.E()));
  lep_eta.push_back(0.f);
  lep_isMu.push_back(0.f);
  lep_isEl.push_back(0.f);
  lep_px.push_back(analyzer.MET.Px());
  lep_py.push_back(analyzer.MET.Py());
  lep_pz.push_back(analyzer.MET.Pz());
  lep_energy.push_back(analyzer.MET.E());
  std::vector<float> lep_mask(kWcbNNLepSequenceLen, 1.f);

  std::unordered_map<std::string, VariousArray> input_data = {
      {"jet_features", FloatArray{}}, {"jet_vectors", FloatArray{}},
      {"jet_mask", FloatArray{}},     {"lep_features", FloatArray{}},
      {"lep_vectors", FloatArray{}},  {"lep_mask", FloatArray{}},
  };
  std::unordered_map<std::string, IntArray> input_shape = {
      {"jet_features", {1, kWcbNNJetFeatureDim, kWcbNNMaxJets}},
      {"jet_vectors", {1, kWcbNNJetVectorDim, kWcbNNMaxJets}},
      {"jet_mask", {1, 1, kWcbNNMaxJets}},
      {"lep_features", {1, kWcbNNLepFeatureDim, kWcbNNLepSequenceLen}},
      {"lep_vectors", {1, kWcbNNLepVectorDim, kWcbNNLepSequenceLen}},
      {"lep_mask", {1, 1, kWcbNNLepSequenceLen}},
  };

  auto &jet_features = std::get<FloatArray>(input_data["jet_features"]);
  auto &jet_vectors = std::get<FloatArray>(input_data["jet_vectors"]);
  auto &jet_mask = std::get<FloatArray>(input_data["jet_mask"]);
  auto &lep_features = std::get<FloatArray>(input_data["lep_features"]);
  auto &lep_vectors = std::get<FloatArray>(input_data["lep_vectors"]);
  auto &lep_mask_group = std::get<FloatArray>(input_data["lep_mask"]);

  jet_features.reserve(kWcbNNJetFeatureDim * kWcbNNMaxJets);
  jet_vectors.reserve(kWcbNNJetVectorDim * kWcbNNMaxJets);
  jet_mask.reserve(kWcbNNMaxJets);
  lep_features.reserve(kWcbNNLepFeatureDim * kWcbNNLepSequenceLen);
  lep_vectors.reserve(kWcbNNLepVectorDim * kWcbNNLepSequenceLen);
  lep_mask_group.reserve(kWcbNNLepSequenceLen);

  append_preprocessed_feature(jet_features, ak4_pt_log, kWcbNNMaxJets, 4.f,
                              1.f, -5.f, 5.f);
  append_preprocessed_feature(jet_features, ak4_energy_log, kWcbNNMaxJets, 4.f,
                              1.f, -5.f, 5.f);
  append_preprocessed_feature(jet_features, ak4_eta, kWcbNNMaxJets, 0.f, 1.f,
                              -1.e32f, 1.e32f);
  append_preprocessed_feature(jet_features, ak4_tag_B4, kWcbNNMaxJets, 0.f,
                              1.f, -1.e32f, 1.e32f);
  append_preprocessed_feature(jet_features, ak4_tag_B3, kWcbNNMaxJets, 0.f,
                              1.f, -1.e32f, 1.e32f);
  append_preprocessed_feature(jet_features, ak4_tag_B2, kWcbNNMaxJets, 0.f,
                              1.f, -1.e32f, 1.e32f);
  append_preprocessed_feature(jet_features, ak4_tag_B1, kWcbNNMaxJets, 0.f,
                              1.f, -1.e32f, 1.e32f);
  append_preprocessed_feature(jet_features, ak4_tag_B0, kWcbNNMaxJets, 0.f,
                              1.f, -1.e32f, 1.e32f);
  append_preprocessed_feature(jet_features, ak4_tag_C4, kWcbNNMaxJets, 0.f,
                              1.f, -1.e32f, 1.e32f);
  append_preprocessed_feature(jet_features, ak4_tag_C3, kWcbNNMaxJets, 0.f,
                              1.f, -1.e32f, 1.e32f);
  append_preprocessed_feature(jet_features, ak4_tag_C2, kWcbNNMaxJets, 0.f,
                              1.f, -1.e32f, 1.e32f);
  append_preprocessed_feature(jet_features, ak4_tag_C1, kWcbNNMaxJets, 0.f,
                              1.f, -1.e32f, 1.e32f);
  append_preprocessed_feature(jet_features, ak4_tag_C0, kWcbNNMaxJets, 0.f,
                              1.f, -1.e32f, 1.e32f);

  append_preprocessed_feature(jet_vectors, ak4_px, kWcbNNMaxJets, 0.f, 1.f,
                              -1.e32f, 1.e32f);
  append_preprocessed_feature(jet_vectors, ak4_py, kWcbNNMaxJets, 0.f, 1.f,
                              -1.e32f, 1.e32f);
  append_preprocessed_feature(jet_vectors, ak4_pz, kWcbNNMaxJets, 0.f, 1.f,
                              -1.e32f, 1.e32f);
  append_preprocessed_feature(jet_vectors, ak4_energy, kWcbNNMaxJets, 0.f, 1.f,
                              -1.e32f, 1.e32f);
  append_preprocessed_feature(jet_mask, ak4_mask, kWcbNNMaxJets, 0.f, 1.f,
                              -1.e32f, 1.e32f);

  append_preprocessed_feature(lep_features, lep_pt_log, kWcbNNLepSequenceLen,
                              4.f, 1.f, -5.f, 5.f);
  append_preprocessed_feature(lep_features, lep_energy_log,
                              kWcbNNLepSequenceLen, 4.f, 1.f, -5.f, 5.f);
  append_preprocessed_feature(lep_features, lep_eta, kWcbNNLepSequenceLen, 0.f,
                              1.f, -1.e32f, 1.e32f);
  append_preprocessed_feature(lep_features, lep_isMu, kWcbNNLepSequenceLen,
                              0.f, 1.f, -1.e32f, 1.e32f);
  append_preprocessed_feature(lep_features, lep_isEl, kWcbNNLepSequenceLen,
                              0.f, 1.f, -1.e32f, 1.e32f);

  append_preprocessed_feature(lep_vectors, lep_px, kWcbNNLepSequenceLen, 0.f,
                              1.f, -1.e32f, 1.e32f);
  append_preprocessed_feature(lep_vectors, lep_py, kWcbNNLepSequenceLen, 0.f,
                              1.f, -1.e32f, 1.e32f);
  append_preprocessed_feature(lep_vectors, lep_pz, kWcbNNLepSequenceLen, 0.f,
                              1.f, -1.e32f, 1.e32f);
  append_preprocessed_feature(lep_vectors, lep_energy,
                              kWcbNNLepSequenceLen, 0.f, 1.f, -1.e32f, 1.e32f);
  append_preprocessed_feature(lep_mask_group, lep_mask, kWcbNNLepSequenceLen,
                              0.f, 1.f, -1.e32f, 1.e32f);

  const auto &helpers = get_wcb_nn_fold_helpers();
  if (helpers.empty()) {
    throw std::runtime_error("[Vcb_SL] No Wcb NN fold helper is available.");
  }

  WcbNNResult result;
  auto fill_scores_from_output =
      [&](const std::unordered_map<std::string, FloatArray> &output_data,
          const MLHelper &helper, std::array<float, 8> &dest) {
        const auto &scores = extract_single_output_scores(output_data, helper);
        if (scores.size() != dest.size()) {
          throw std::runtime_error(
              "[Vcb_SL] Unexpected Wcb NN output size: expected 8 scores.");
        }
        std::copy(scores.begin(), scores.end(), dest.begin());
      };

  if (use_single_wcb_nn_fold(analyzer.DataEra)) {
    const std::size_t fold_idx =
        static_cast<std::size_t>(event_number % helpers.size());
    const auto output_data =
        helpers[fold_idx]->Run_ONNX_Model(input_data, input_shape);
    fill_scores_from_output(output_data, *helpers[fold_idx], result.scores);
  } else {
    std::array<float, 8> score_sum = {};
    for (const auto &helper : helpers) {
      const auto output_data = helper->Run_ONNX_Model(input_data, input_shape);
      std::array<float, 8> fold_scores = {};
      fill_scores_from_output(output_data, *helper, fold_scores);
      for (std::size_t i = 0; i < fold_scores.size(); ++i) {
        score_sum[i] += fold_scores[i];
      }
    }
    const float inv_nfolds = 1.f / static_cast<float>(helpers.size());
    for (std::size_t i = 0; i < score_sum.size(); ++i) {
      result.scores[i] = score_sum[i] * inv_nfolds;
    }
  }

  result.pre_softmax = derive_centered_logits_from_scores(result.scores);
  result.category = static_cast<int>(std::distance(
      result.scores.begin(),
      std::max_element(result.scores.begin(), result.scores.end())));
  return result;
}
}

Vcb_SL::Vcb_SL() {
  // Persist shapes (batch dimension fixed to 1 for now)
  onnx_input_shape["Momenta_data"] = {1, kMaxJetsForONNX, kMomFeatDimONNX};
  onnx_input_shape["Momenta_mask"] = {1, kMaxJetsForONNX};
  onnx_input_shape["Met_data"] = {1, 1, 3};
  onnx_input_shape["Met_mask"] = {1, 1};
  onnx_input_shape["Lepton_data"] = {1, 1, 7};
  onnx_input_shape["Lepton_mask"] = {1, 1};

  // Allocate the map entries once so later we only mutate the vectors.
  onnx_input_data["Momenta_data"] = FloatArray{};
  onnx_input_data["Momenta_mask"] = BoolArray{};
  onnx_input_data["Met_data"] = FloatArray{};
  onnx_input_data["Met_mask"] = BoolArray{};
  onnx_input_data["Lepton_data"] = FloatArray{};
  onnx_input_data["Lepton_mask"] = BoolArray{};

  std::get<FloatArray>(onnx_input_data["Momenta_data"])
      .reserve(kMaxJetsForONNX * kMomFeatDimONNX);
  std::get<BoolArray>(onnx_input_data["Momenta_mask"]).reserve(kMaxJetsForONNX);
  std::get<FloatArray>(onnx_input_data["Met_data"]).reserve(3);
  std::get<BoolArray>(onnx_input_data["Met_mask"]).reserve(1);
  std::get<FloatArray>(onnx_input_data["Lepton_data"]).reserve(7);
  std::get<BoolArray>(onnx_input_data["Lepton_mask"]).reserve(1);

  // TabNet buffers
  const int tabnet_input_dim =
      1 + 2 + 2 + (kUseILRJetFeatures ? 4 : 2) +
      static_cast<int>(kWcbNNPreSoftmaxBranchNames.size()) + 2 +
      kTabNetEventContextFeatDim;
  tabnet_input_shape["input"] = {1, tabnet_input_dim};
  tabnet_input_data["input"] = FloatArray{};
  std::get<FloatArray>(tabnet_input_data["input"]).reserve(tabnet_input_dim);
  tabnet_class_logits.reserve(7);
  tabnet_class_scores.reserve(7);
  tabnet_weighted_scores.reserve(7);
}

void Vcb_SL::CreateTrainingTree() {
  if (HasFlag("Skim"))
    return;
  RVec<TString> keeps = {};
  RVec<TString> drops = {"*"};
  BookTree("Training_Tree", keeps, drops);
  OutputTree("Training_Tree").Branch("Jet_Px", Jet_Px);
  OutputTree("Training_Tree").Branch("Jet_Py", Jet_Py);
  OutputTree("Training_Tree").Branch("Jet_Pz", Jet_Pz);
  OutputTree("Training_Tree").Branch("Jet_E", Jet_E);
  OutputTree("Training_Tree").Branch("Jet_Mass", Jet_M);
  OutputTree("Training_Tree").Branch("Jet_BvsC", Jet_BvsC);
  OutputTree("Training_Tree").Branch("Jet_CvsB", Jet_CvsB);
  OutputTree("Training_Tree").Branch("Jet_CvsL", Jet_CvsL);
  OutputTree("Training_Tree").Branch("Jet_QvsG", Jet_QvsG);
  OutputTree("Training_Tree").Branch("Jet_HFvLF", Jet_HFvLF);
  OutputTree("Training_Tree").Branch("Jet_BvC", Jet_BvC);
  OutputTree("Training_Tree").Branch("Jet_Category", Jet_Category);
  OutputTree("Training_Tree").Branch("Jet_B_WP", Jet_B_WP);
  OutputTree("Training_Tree").Branch("Jet_C_WP", Jet_C_WP);
  OutputTree("Training_Tree").Branch("Jet_ILR_Dim_1", Jet_ILR_Dim_1);
  OutputTree("Training_Tree").Branch("Jet_ILR_Dim_2", Jet_ILR_Dim_2);
  OutputTree("Training_Tree").Branch("Jet_isTTbarJet", Jet_isTTbarJet);
  OutputTree("Training_Tree").Branch("Jet_ttbarJet_idx", Jet_ttbarJet_idx);
  OutputTree("Training_Tree").Branch("Jet_Pt", Jet_Pt);
  OutputTree("Training_Tree").Branch("Jet_Eta", Jet_Eta);
  OutputTree("Training_Tree").Branch("Jet_Phi", Jet_Phi);
  OutputTree("Training_Tree").Branch("edge_index_jet_jet0", edge_index_jet_jet0);
  OutputTree("Training_Tree").Branch("edge_index_jet_jet1", edge_index_jet_jet1);
  OutputTree("Training_Tree").Branch("deltaR_jet_jet", deltaR_jet_jet);
  OutputTree("Training_Tree").Branch("invM_jet_jet", invM_jet_jet);
  OutputTree("Training_Tree").Branch("cosTheta_jet_jet", cosTheta_jet_jet);
  OutputTree("Training_Tree").Branch("edge_index_jet_lepton0", edge_index_jet_lepton0);
  OutputTree("Training_Tree").Branch("edge_index_jet_lepton1", edge_index_jet_lepton1);
  OutputTree("Training_Tree").Branch("deltaR_jet_lepton", deltaR_jet_lepton);
  OutputTree("Training_Tree").Branch("invM_jet_lepton", invM_jet_lepton);
  OutputTree("Training_Tree").Branch("cosTheta_jet_lepton", cosTheta_jet_lepton);
  OutputTree("Training_Tree").Branch("edge_index_jet_neutrino0", edge_index_jet_neutrino0);
  OutputTree("Training_Tree").Branch("edge_index_jet_neutrino1", edge_index_jet_neutrino1);
  OutputTree("Training_Tree").Branch("deltaR_jet_neutrino", deltaR_jet_neutrino);
  OutputTree("Training_Tree").Branch("invM_jet_neutrino", invM_jet_neutrino);
  OutputTree("Training_Tree").Branch("cosTheta_jet_neutrino", cosTheta_jet_neutrino);
  OutputTree("Training_Tree").Branch("edge_index_lepton_neutrino0", edge_index_lepton_neutrino0);
  OutputTree("Training_Tree").Branch("edge_index_lepton_neutrino1", edge_index_lepton_neutrino1);
  OutputTree("Training_Tree").Branch("deltaR_lepton_neutrino", deltaR_lepton_neutrino);
  OutputTree("Training_Tree").Branch("invM_lepton_neutrino", invM_lepton_neutrino);
  OutputTree("Training_Tree").Branch("cosTheta_lepton_neutrino", cosTheta_lepton_neutrino);
  OutputTree("Training_Tree").Branch("Index_Hard_Process", parton_jet_assignment);
}

void Vcb_SL::CreateTemplateTrainingTree() {
  if (HasFlag("Skim"))
    return;
  RVec<TString> keeps = {};
  RVec<TString> drops = {"*"};
  BookTree("Template_Training_Tree", keeps, drops);
}

RVec<RVec<unsigned int>>
Vcb_SL::GetPermutations(const SelectedJetViewCollection &jets) {
  RVec<RVec<unsigned int>> permutations;
  std::vector<unsigned int> b_tagged_idx;
  int max_jet = 8;
  if (jets.size() < 8)
    max_jet = jets.size();
  for (unsigned int i = 0; i < max_jet; i++) {
    if (GetPassedBTaggingWP(jets[i]) >= 1) {
      b_tagged_idx.push_back(i);
    }
  }
  // Permutations rules:
  // idx 0 and 1 are b jets from top decay. b-tagged jets only.
  // idx 2 and 3 are w jets from hadronic top decay. they are interchangeable.

  std::vector<std::pair<unsigned int, unsigned int>> b_tagged_pairs;

  for (unsigned int i = 0; i < b_tagged_idx.size(); i++) {
    for (unsigned int j = 0; j < b_tagged_idx.size();
         j++) // there is ordering of b-jet in Semileptonic channel
    {
      if (i == j)
        continue;
      b_tagged_pairs.push_back(
          std::make_pair(b_tagged_idx[i], b_tagged_idx[j]));
    }
  }

  for (auto &pair : b_tagged_pairs) {

    std::vector<unsigned int> remaining_idx;
    std::vector<std::vector<unsigned int>> w_pairs;

    for (unsigned int i = 0; i < max_jet; i++) {
      if (i != pair.first && i != pair.second) {
        remaining_idx.push_back(i);
      }
    }
    // make all permutations of remaining_idx using next_permutation
    do {
      // push back first 4 elements
      w_pairs.push_back({remaining_idx[0], remaining_idx[1]});
    } while (std::next_permutation(remaining_idx.begin(), remaining_idx.end()));
    for (auto &w_pair : w_pairs) {
      std::sort(w_pair.begin(), w_pair.end());
    }
    // remove duplicates
    std::sort(w_pairs.begin(), w_pairs.end());
    w_pairs.erase(std::unique(w_pairs.begin(), w_pairs.end()), w_pairs.end());
    for (auto &w_pair : w_pairs) {
      // push back the b jet pair and w jet pairs
      std::vector<unsigned int> permutation;
      permutation.push_back(pair.first);
      permutation.push_back(pair.second);
      permutation.insert(permutation.end(), w_pair.begin(), w_pair.end());
      permutations.push_back(permutation);
    }
  }
  return permutations;
}

std::variant<float, std::pair<float, float>>
Vcb_SL::SolveNeutrinoPz(const Lepton &lepton, const Particle &met) {
  float Ptl_dot_Ptnu = lepton.Px() * met.Px() + lepton.Py() * met.Py();
  float lepton_mass = lepton.M();

  // solve a*x^2 + b*x + c = 0, where x = pz of neutrino
  float k = TMath::Power(W_MASS, 2.) / 2.0 - lepton_mass * lepton_mass / 2.0 +
            Ptl_dot_Ptnu;
  float a = TMath::Power(lepton.Pt(), 2.0);
  float b = -2 * k * lepton.Pz();
  float c = TMath::Power(lepton.Pt(), 2.0) * TMath::Power(met.Pt(), 2.0) -
            TMath::Power(k, 2.0);

  float determinant = TMath::Power(b, 2.f) - 4.f * a * c;
  if (determinant < 0) {
    float real_pz = -b / (2.f * a);
    return real_pz;
  } else {
    float pz1 = (-b + TMath::Sqrt(determinant)) / (2.f * a);
    float pz2 = (-b - TMath::Sqrt(determinant)) / (2.f * a);
    return std::make_pair(pz1, pz2);
  }
}

bool Vcb_SL::PassBaseLineSelection(bool remove_flavtagging_cut,
                                   bool loose_cut) {
  Clear();
  wcb_nn_scores.fill(0.f);
  wcb_nn_pre_softmax.fill(0.f);
  wcb_nn_category = -1;
  wcb_nn_cache_valid = false;
  FillCutFlow(0); // start

  // 1) trigger
  if (channel == Channel::Mu)
    if (!ev.PassTrigger(Mu_Trigger[DataEra.Data()]))
      return false;
  if (channel == Channel::El)
    if (!ev.PassTrigger(El_Trigger[DataEra.Data()]))
      return false;
  FillCutFlow(1); // trigger passed

  // 2) jet veto map (common)
  if (!PassJetVetoMap(AllJetViews))
    return false;
  FillCutFlow(2); // veto maps passed

  // 3) MET filter
  if (!PassMetFilter(AllJetViews, ev))
    return false;
  FillCutFlow(3); // met filter passed

  // 4) lepton selection
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
    auto select_tight_muons = [&](MuonView::MuonID id, bool require_iso) {
      std::vector<std::size_t> indices = SelectMuonIndices(
          AllMuonViews, id, Muon_Tight_Pt[DataEra.Data()], Muon_Tight_Eta);
      if (require_iso) {
        indices =
            SelectMuonIndices(AllMuonViews, indices, Muon_Tight_Iso,
                              Muon_Tight_Pt[DataEra.Data()], Muon_Tight_Eta);
      }
      return indices;
    };

    auto select_tight_electrons = [&](ElectronView::ElectronID id) {
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
        select_tight_muons(MuonView::MuonID::POG_TIGHT, true);
    const std::vector<std::size_t> mu_tight_prompt =
        select_tight_muons(MuonView::MuonID::POG_PROMPTMVA_WP0p64, false);
    const std::vector<std::size_t> el_tight_wp80 =
        select_tight_electrons(ElectronView::ElectronID::POG_MVAISO_WP80);
    const std::vector<std::size_t> el_tight_prompt =
        select_tight_electrons(ElectronView::ElectronID::POG_PROMPTMVA_MEDIUM);

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
    Muons = MuonViewCollection(AllMuonViews.storage(), Muons_indices);
    Electrons =
        ElectronViewCollection(AllElectronViews.storage(), Electrons_indices);
    lepton = MakeLeptonSnapshot(Electrons[0]);
    leptons.push_back(lepton);
  }
  if (channel == Channel::Mu) {
    if (!HasFlag("Skim")) {
      if (!(Muons_indices.size() == 1 && Electron_Veto_indices.size() == 0 &&
            Electrons_indices.size() == 0 && Muons_Veto_indices.size() == 1))
        return false;
    }
    Muons = MuonViewCollection(AllMuonViews.storage(), Muons_indices);
    Electrons =
        ElectronViewCollection(AllElectronViews.storage(), Electrons_indices);
    lepton = MakeLeptonSnapshot(Muons[0]);
    leptons.push_back(lepton);
  }
  FillCutFlow(4); // lepton passed

  // 5) MET + jet p4 shift
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
    MET = ev.GetMETVector(Event::MET_Type::PUPPI, systVar, Event::MET_Syst::UE);
    doJetPropagation = false;
  }

  if (doJetPropagation)
    PropagateJetSystToMET(AllJetViews, MET, jesVar, jerVar);
  if (MET.Pt() < (loose_cut ? SL_MET_cut - 5.f : SL_MET_cut)) return false;
  FillCutFlow(5); // MET/JES/JER propagation done

  // 6) jets
  float Jet_Pt_Cut = loose_cut ? SL_Jet_Pt_cut - 5.f : SL_Jet_Pt_cut;
  std::vector<std::size_t> jetIndices = SelectJetIndices(
      AllJetViews, Jet_ID, Jet_Pt_Cut, Jet_Eta_cut, jesVar, jerVar);

  jetIndices = JetsVetoLeptonInside(AllJetViews, jetIndices, AllElectronViews,
                                    Electrons_indices, AllMuonViews,
                                    Muons_indices, Jet_Veto_DR);

  if (HasFlag("QuadJet")) {
    if (jetIndices.size() < 4)
      return false;
  } else {
    if (jetIndices.size() < 3)
      return false;
  }

  Jets = SelectJetViews(AllJetViews, jetIndices, jesVar, jerVar);

  HT = GetHT(Jets);
  n_jets = Jets.size();
  FillCutFlow(6); // jet selection passed

  // 7) flavour tagging
  UpdateAllJetTaggingCaches(AllJetViews, jetIndices);
  // short bWP_work = loose_cut ? 0 : 1;
  // short cWP_work = loose_cut ? 0 : 1;
  short bWP_work = 1;
  short cWP_work = 1;
  short bWP_loose_work = 0;
  short cWP_loose_work = 0;

  for (const auto &jet : Jets) {
    const short bWP = GetPassedBTaggingWP(jet);
    const short cWP = GetPassedCTaggingWP(jet);

    if (bWP >= bWP_work)
      n_b_tagged_jets++;
    if (bWP >= bWP_loose_work)
      n_loose_b_tagged_jets++;
    if (!IsDATA) {
      if (std::abs(jet.hadronFlavour()) == 5)
        n_hadronFlav_b_jets++;
      if (std::abs(jet.hadronFlavour()) == 4)
        n_hadronFlav_c_jets++;
    }
  }
  // if (n_b_tagged_jets == 1) {
  //   // in this case cWp should be tightened by one level
  //   cWP_work = loose_cut ? 0 : 3;
  // }
  for (const auto &jet : Jets) {
    const short cWP = GetPassedCTaggingWP(jet);
    if (cWP >= cWP_work)
      n_c_tagged_jets++;
    if (cWP >= cWP_loose_work)
      n_loose_c_tagged_jets++;
  }
  n_hf_jets = n_b_tagged_jets + n_c_tagged_jets;
  n_loose_hf_jets = n_loose_b_tagged_jets + n_loose_c_tagged_jets;
  FillCutFlow(7); // b/c tag c ounting done
  if(!loose_cut){
    if ((n_b_tagged_jets < 1 || n_hf_jets < 3) && !remove_flavtagging_cut) return false;
  }
  else{
    if ((n_loose_b_tagged_jets < 1 || n_loose_hf_jets < 3) && !remove_flavtagging_cut) return false;
  }

  // if ((n_b_tagged_jets < 2) && !remove_flavtagging_cut)
  //   return false;
  FillCutFlow(8); // flavour tag cut passed

  SetTTbarId();
  SetSystematicLambda();
  return true;
}

void Vcb_SL::FillKinematicFitterResult(const TString &histPrefix,
                                       float weight) {
  Particle fitted_lep_w = best_KF_result.fitted_lep + best_KF_result.fitted_neu;
  Particle fitted_had_w =
      best_KF_result.fitted_had_w1 + best_KF_result.fitted_had_w2;
  Particle fitted_lep_top = fitted_lep_w + best_KF_result.fitted_lep_t_b;
  Particle fitted_had_top = fitted_had_w + best_KF_result.fitted_had_t_b;
  Particle had_w(Jets[best_KF_result.best_had_w1_idx].P4() +
                 Jets[best_KF_result.best_had_w2_idx].P4());
  TLorentzVector neutrino_p4;
  neutrino_p4.SetXYZM(MET.Px(), MET.Py(), best_KF_result.best_neu_pz, 0.);
  Particle neutrino(neutrino_p4);
  Particle lep_w = lepton + neutrino;
  Particle lep_top(lep_w + Jets[best_KF_result.best_lep_t_b_idx].P4());
  Particle had_top(had_w + Jets[best_KF_result.best_had_t_b_idx].P4());

  Hists().Fill(histPrefix + "/" + "had_top_mass", had_top.M(), weight, 100, 100.,
           300.);
  Hists().Fill(histPrefix + "/" + "lep_top_mass", lep_top.M(), weight, 100, 100.,
           300.);
  Hists().Fill(histPrefix + "/" + "had_w_mass", had_w.M(), weight, 200, 50., 200.);
  Hists().Fill(histPrefix + "/" + "lep_w_mass", lep_w.M(), weight, 100, 50., 100.);
  Hists().Fill(histPrefix + "/" + "Fitted_had_top_mass", fitted_had_top.M(), weight,
           100, 100., 300.);
  Hists().Fill(histPrefix + "/" + "Fitted_lep_top_mass", fitted_lep_top.M(), weight,
           100, 100., 300.);
  Hists().Fill(histPrefix + "/" + "Fitted_had_w_mass", fitted_had_w.M(), weight,
           100, 50., 100.);
  Hists().Fill(histPrefix + "/" + "Fitted_lep_w_mass", fitted_lep_w.M(), weight,
           100, 50., 100.);
  Hists().Fill(histPrefix + "/" + "Chi2", best_KF_result.chi2, weight, 600, 0.,
           300.);
}

void Vcb_SL::FillTrainingTree() {
  ttbar_jet_indices = FindTTbarJetIndices();

  float weight = MCNormalization();
  // weight *= systHelper->calculateWeight()["Central"];
  OutputTree("Training_Tree").Set( "index_fold",
            static_cast<int>(rle_bucket(RunNumber, luminosityBlock, event, 4)));
  OutputTree("Training_Tree").Set( "weight", weight);
  OutputTree("Training_Tree").Set( "genTtbarId", genTtbarId);
  OutputTree("Training_Tree").Set( "Met_Pt", MET.Pt());
  OutputTree("Training_Tree").Set( "Met_Phi", MET.Phi());
  OutputTree("Training_Tree").Set( "HT", HT);
  OutputTree("Training_Tree").Set( "n_jets", n_jets);
  OutputTree("Training_Tree").Set( "n_b_tagged_jets", n_b_tagged_jets);
  OutputTree("Training_Tree").Set( "n_c_tagged_jets", n_c_tagged_jets);
  OutputTree("Training_Tree").Set( "find_all_jets", find_all_jets);
  OutputTree("Training_Tree").Set( "decay_mode", (tt_decay_code / 100));
  OutputTree("Training_Tree").Set( "run", static_cast<int>(RunNumber));
  OutputTree("Training_Tree").Set( "event", static_cast<int>(event));
  OutputTree("Training_Tree").Set( "lumi", static_cast<int>(luminosityBlock));

  // for (size_t i = 0; i <= 8; i++)
  // {
  //     if (i < n_jets)
  //     {
  //         OutputTree("Training_Tree").Set( "Jet_Pt_" + std::to_string(i),
  //         Jets[i].Pt()); OutputTree("Training_Tree").Set( "Jet_Eta_" +
  //         std::to_string(i), Jets[i].Eta()); OutputTree("Training_Tree").Set(
  //         "Jet_Phi_" + std::to_string(i), Jets[i].Phi());
  //         OutputTree("Training_Tree").Set( "Jet_M_" + std::to_string(i),
  //         Jets[i].M()); OutputTree("Training_Tree").Set( "Jet_BvsC_" +
  //         std::to_string(i), JetBScore(Jets[i]));
  //         OutputTree("Training_Tree").Set( "Jet_CvsB_" + std::to_string(i),
  //         JetCvBScore(Jets[i])); OutputTree("Training_Tree").Set( "Jet_CvsL_" +
  //         std::to_string(i), JetCvLScore(Jets[i]));
  //         // Tagging WP
  //         OutputTree("Training_Tree").Set( "Jet_B_WP_" + std::to_string(i),
  //         GetPassedBTaggingWP(Jets[i])); OutputTree("Training_Tree").Set(
  //         "Jet_C_WP_" + std::to_string(i), GetPassedCTaggingWP(Jets[i]));

  //         auto it = find(ttbar_jet_indices.begin(),
  //         ttbar_jet_indices.end(), i); if (it != ttbar_jet_indices.end())
  //         {
  //             OutputTree("Training_Tree").Set( "Jet_isTTbarJet_" +
  //             std::to_string(i), int(1)); OutputTree("Training_Tree").Set(
  //             "Jet_ttbarJet_idx_" + std::to_string(i), int(it -
  //             ttbar_jet_indices.begin()));
  //         }
  //         else
  //         {
  //             OutputTree("Training_Tree").Set( "Jet_isTTbarJet_" +
  //             std::to_string(i), int(0)); OutputTree("Training_Tree").Set(
  //             "Jet_ttbarJet_idx_" + std::to_string(i), int(-999));
  //         }
  //     }
  //     else
  //     {
  //         OutputTree("Training_Tree").Set( "Jet_Pt_" + std::to_string(i), -999.);
  //         OutputTree("Training_Tree").Set( "Jet_Eta_" + std::to_string(i),
  //         -999.); OutputTree("Training_Tree").Set( "Jet_Phi_" +
  //         std::to_string(i), -999.); OutputTree("Training_Tree").Set( "Jet_M_" +
  //         std::to_string(i), -999.); OutputTree("Training_Tree").Set( "Jet_BvsC_"
  //         + std::to_string(i), -999.); OutputTree("Training_Tree").Set(
  //         "Jet_CvsB_" + std::to_string(i), -999.);
  //         OutputTree("Training_Tree").Set( "Jet_CvsL_" + std::to_string(i),
  //         -999.); OutputTree("Training_Tree").Set( "Jet_isTTbarJet_" +
  //         std::to_string(i), int(-999)); OutputTree("Training_Tree").Set(
  //         "Jet_ttbarJet_idx_" + std::to_string(i), int(-999));
  //         OutputTree("Training_Tree").Set( "Jet_B_WP_" + std::to_string(i),
  //         -999); OutputTree("Training_Tree").Set( "Jet_C_WP_" +
  //         std::to_string(i), -999);
  //     }
  // }
  // clear vectors
  Jet_Px.clear();
  Jet_Py.clear();
  Jet_Pz.clear();
  Jet_E.clear();
  Jet_M.clear();
  Jet_BvsC.clear();
  Jet_CvsB.clear();
  Jet_CvsL.clear();
  Jet_QvsG.clear();
  Jet_HFvLF.clear();
  Jet_BvC.clear();
  Jet_Category.clear();
  Jet_B_WP.clear();
  Jet_C_WP.clear();
  Jet_ILR_Dim_1.clear();
  Jet_ILR_Dim_2.clear();
  Jet_isTTbarJet.clear();
  Jet_ttbarJet_idx.clear();
  Jet_Pt.clear();
  Jet_Eta.clear();
  Jet_Phi.clear();
  edge_index_jet_jet0.clear();
  edge_index_jet_jet1.clear();
  deltaR_jet_jet.clear();
  invM_jet_jet.clear();
  cosTheta_jet_jet.clear();
  edge_index_jet_lepton0.clear();
  edge_index_jet_lepton1.clear();
  deltaR_jet_lepton.clear();
  invM_jet_lepton.clear();
  cosTheta_jet_lepton.clear();
  edge_index_jet_neutrino0.clear();
  edge_index_jet_neutrino1.clear();
  deltaR_jet_neutrino.clear();
  invM_jet_neutrino.clear();
  cosTheta_jet_neutrino.clear();
  edge_index_lepton_neutrino0.clear();
  edge_index_lepton_neutrino1.clear();
  deltaR_lepton_neutrino.clear();
  invM_lepton_neutrino.clear();
  cosTheta_lepton_neutrino.clear();

  int max_jet = 8;
  if (Jets.size() < 8)
    max_jet = Jets.size();

  TLorentzVector neutrino_p4;
  neutrino_p4.SetXYZM(MET.Px(), MET.Py(), best_KF_result.best_neu_pz, 0.);
  Particle neutrino(neutrino_p4);
  OutputTree("Training_Tree").Set( "neutrino_Pt", neutrino.Pt());
  OutputTree("Training_Tree").Set( "neutrino_Eta", neutrino.Eta());
  OutputTree("Training_Tree").Set( "neutrino_Phi", neutrino.Phi());
  OutputTree("Training_Tree").Set( "neutrino_M", neutrino.M());
  OutputTree("Training_Tree").Set( "neutrino_Px", neutrino.Px());
  OutputTree("Training_Tree").Set( "neutrino_Py", neutrino.Py());
  OutputTree("Training_Tree").Set( "neutrino_Pz", neutrino.Pz());
  OutputTree("Training_Tree").Set( "neutrino_E", neutrino.E());
  OutputTree("Training_Tree").Set( "gen_neutrino_Pt", gen_neutrino.Pt());
  OutputTree("Training_Tree").Set( "gen_neutrino_Eta", gen_neutrino.Eta());
  OutputTree("Training_Tree").Set( "gen_neutrino_Phi", gen_neutrino.Phi());
  OutputTree("Training_Tree").Set( "gen_neutrino_Px", gen_neutrino.Px());
  OutputTree("Training_Tree").Set( "gen_neutrino_Py", gen_neutrino.Py());
  OutputTree("Training_Tree").Set( "gen_neutrino_Pz", gen_neutrino.Pz());
  OutputTree("Training_Tree").Set( "gen_neutrino_E", gen_neutrino.E());

  parton_jet_assignment.clear();
  parton_jet_assignment = {static_cast<int>(ttbar_jet_indices[0]),  // hb
                           static_cast<int>(ttbar_jet_indices[2]),  // w1
                           static_cast<int>(ttbar_jet_indices[3]),  // w2
                           static_cast<int>(ttbar_jet_indices[1])}; // lb
  for (size_t i = 0; i < max_jet; i++) {

    Jet_Px.push_back(Jets[i].Px());
    Jet_Py.push_back(Jets[i].Py());
    Jet_Pz.push_back(Jets[i].Pz());
    Jet_E.push_back(Jets[i].E());
    Jet_M.push_back(Jets[i].M());
    Jet_BvsC.push_back(JetBScore(Jets[i]));
    Jet_CvsB.push_back(JetCvBScore(Jets[i]));
    Jet_CvsL.push_back(JetCvLScore(Jets[i]));
    Jet_QvsG.push_back(JetQvGScore(Jets[i]));
    Jet_HFvLF.push_back(JetHFvLFScore(Jets[i]));
    Jet_BvC.push_back(JetBvCScore(Jets[i]));
    Jet_Category.push_back(static_cast<int>(JetCategory(Jets[i])));
    Jet_B_WP.push_back(GetPassedBTaggingWP(Jets[i]));
    Jet_C_WP.push_back(GetPassedCTaggingWP(Jets[i]));
    Jet_ILR_Dim_1.push_back(JetILRdim1Score(Jets[i]));
    Jet_ILR_Dim_2.push_back(JetILRdim2Score(Jets[i]));
    Jet_Pt.push_back(Jets[i].Pt());
    Jet_Eta.push_back(Jets[i].Eta());
    Jet_Phi.push_back(Jets[i].Phi());
    auto it = find(ttbar_jet_indices.begin(), ttbar_jet_indices.end(), i);
    if (it != ttbar_jet_indices.end()) {
      Jet_isTTbarJet.push_back(1);
      Jet_ttbarJet_idx.push_back(it - ttbar_jet_indices.begin());
    } else {
      Jet_isTTbarJet.push_back(0);
      Jet_ttbarJet_idx.push_back(-999);
    }
  }

  for (int i = 0; i < max_jet; i++) {
    for (int j = 0; j < max_jet; j++) {
      edge_index_jet_jet0.push_back(i);
      edge_index_jet_jet1.push_back(j);
      float this_deltaR = Jets[i].DeltaR(Jets[j]);
      float this_invM = (Jets[i].P4() + Jets[j].P4()).M();
      TVector3 v1 = Jets[i].Vect();
      TVector3 v2 = Jets[j].Vect();
      float this_cosTheta = TMath::Cos(v1.Angle(v2));

      deltaR_jet_jet.push_back(this_deltaR);
      invM_jet_jet.push_back(this_invM);
      cosTheta_jet_jet.push_back(this_cosTheta);
    }
  }

  for (int i = 0; i < max_jet; i++) {
    edge_index_jet_lepton0.push_back(i);
    edge_index_jet_lepton1.push_back(0);
    edge_index_jet_neutrino0.push_back(i);
    edge_index_jet_neutrino1.push_back(0);
    deltaR_jet_lepton.push_back(Jets[i].DeltaR(lepton));
    invM_jet_lepton.push_back((Jets[i].P4() + lepton).M());
    deltaR_jet_neutrino.push_back(Jets[i].DeltaR(neutrino));
    invM_jet_neutrino.push_back((Jets[i].P4() + neutrino).M());

    TVector3 vjet = Jets[i].Vect();
    TVector3 vlepton = lepton.Vect();
    TVector3 vneutrino = neutrino.Vect();
    cosTheta_jet_lepton.push_back(TMath::Cos(vjet.Angle(vlepton)));
    cosTheta_jet_neutrino.push_back(TMath::Cos(vjet.Angle(vneutrino)));
  }

  edge_index_lepton_neutrino0.push_back(0);
  edge_index_lepton_neutrino1.push_back(0);
  TVector3 vlepton = lepton.Vect();
  TVector3 vneutrino = neutrino.Vect();
  deltaR_lepton_neutrino.push_back(lepton.DeltaR(neutrino));
  invM_lepton_neutrino.push_back((lepton + neutrino).M());
  cosTheta_lepton_neutrino.push_back(TMath::Cos(vlepton.Angle(vneutrino)));

  OutputTree("Training_Tree").Set( "Lepton_Pt", lepton.Pt());
  OutputTree("Training_Tree").Set( "Lepton_Eta", lepton.Eta());
  OutputTree("Training_Tree").Set( "Lepton_Phi", lepton.Phi());
  OutputTree("Training_Tree").Set( "Lepton_Mass", lepton.M());
  OutputTree("Training_Tree").Set( "Lepton_Px", lepton.Px());
  OutputTree("Training_Tree").Set( "Lepton_Py", lepton.Py());
  OutputTree("Training_Tree").Set( "Lepton_Pz", lepton.Pz());
  OutputTree("Training_Tree").Set( "Lepton_E", lepton.E());
  OutputTree("Training_Tree").Set( "Lepton_Charge", lepton.Charge());
  OutputTree("Training_Tree").Set( "Lepton_isMuon", int(channel == Channel::Mu));
  OutputTree("Training_Tree").Set( "Lepton_isElectron", int(channel == Channel::El));

  OutputTree("Training_Tree").Set( "KF_had_t_b_idx", best_KF_result.best_had_t_b_idx);
  OutputTree("Training_Tree").Set( "KF_lep_t_b_idx", best_KF_result.best_lep_t_b_idx);
  OutputTree("Training_Tree").Set( "KF_had_w1_idx", best_KF_result.best_had_w1_idx);
  OutputTree("Training_Tree").Set( "KF_had_w2_idx", best_KF_result.best_had_w2_idx);
  OutputTree("Training_Tree").Set( "KF_chi2", best_KF_result.chi2);

  int answer;
  if (IsDATA)
    answer = -999;
  // check TT is in MCSample string
  if (MCSample.Contains("TT")) {
    if (MCSample.Contains("Vcb"))
      answer = category_for_training_SL.at("Vcb");
    else if (ttcc)
      answer = category_for_training_SL.at("TT+C");
    else if (ttbb || ttbj)
      answer = category_for_training_SL.at("TT+B");
    else
      answer = category_for_training_SL.at("TT+LF");
  } else
    answer = category_for_training_SL["Others"];
  OutputTree("Training_Tree").Set( "y", answer);
  OutputTree("Training_Tree").Set( "KF_chi2", best_KF_result.chi2);

  OutputTree("Training_Tree").Fill();
}

void Vcb_SL::EnsureWcbNNEvaluated() {
  if (wcb_nn_cache_valid)
    return;

  const WcbNNResult nn_result =
      evaluate_wcb_nn(*this, static_cast<unsigned long long>(event));
  wcb_nn_scores = nn_result.scores;
  wcb_nn_pre_softmax = nn_result.pre_softmax;
  wcb_nn_category = nn_result.category;
  wcb_nn_cache_valid = true;
}

void Vcb_SL::FillHistogramsAtThisPoint(std::string_view histPrefix,
                                       float weight) {
  Vcb::FillHistogramsAtThisPoint(histPrefix, weight);

  const std::string base(histPrefix);
  std::string name;
  name.reserve(base.size() + 64);

  bool use_placeholder_scores = !wcb_nn_cache_valid && weight == 0.f;
  if (!use_placeholder_scores) {
    try {
      EnsureWcbNNEvaluated();
    } catch (const std::exception &e) {
      std::cerr << "[Vcb_SL::FillHistogramsAtThisPoint] Wcb NN inference failed: "
                << e.what() << "\n";
      wcb_nn_scores.fill(0.f);
      wcb_nn_pre_softmax.fill(0.f);
      wcb_nn_category = -1;
      wcb_nn_cache_valid = true;
    }
  }

  for (std::size_t i = 0; i < kWcbNNScoreBranchNames.size(); ++i) {
    name.assign(base);
    name.push_back('/');
    name.append(kWcbNNScoreBranchNames[i]);
    const float value = use_placeholder_scores ? 0.f : wcb_nn_scores[i];
    Hists().Fill(name, value, weight, 50, 0.f, 1.f);
  }

  name.assign(base);
  name.append("/nn_category");
  const float category_value =
      use_placeholder_scores ? -1.f : static_cast<float>(wcb_nn_category);
  Hists().Fill(name, category_value, weight, 9, -1.5f, 7.5f);
}

std::string sanitize_branch_name(std::string_view raw) {
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
}

void Vcb_SL::FillTreeAtThisPoint(
    std::string_view treePrefix, float MCNormalizationWeight,
    const std::unordered_map<std::string, float> &weight_map) {
  const std::string tree_name = sanitize_branch_name(treePrefix);
  auto [buffers_it, _] = tree_buffers.try_emplace(tree_name);
  auto &buffers = buffers_it->second;

  if (!buffers.booked) {
    BookTree(tree_name, {}, {});
    auto tree = OutputTree(tree_name);
    tree.Branch("Jet_Pt", buffers.Jet_Pt);
    tree.Branch("Jet_Eta", buffers.Jet_Eta);
    tree.Branch("Jet_Phi", buffers.Jet_Phi);
    tree.Branch("Jet_Mass", buffers.Jet_Mass);
    tree.Branch("Jet_Category", buffers.Jet_Category);
    tree.Branch("Jet_ILR_Dim_1", buffers.Jet_ILR_Dim_1);
    tree.Branch("Jet_ILR_Dim_2", buffers.Jet_ILR_Dim_2);
    tree.Branch("Lepton_Pt", buffers.Lepton_Pt);
    tree.Branch("Lepton_Eta", buffers.Lepton_Eta);
    tree.Branch("Lepton_Phi", buffers.Lepton_Phi);
    tree.Branch("Lepton_Mass", buffers.Lepton_Mass);
    tree.Branch("Met_Pt", buffers.Met_Pt);
    tree.Branch("Met_Eta", buffers.Met_Eta);
    tree.Branch("Met_Phi", buffers.Met_Phi);
    tree.Branch("HT", buffers.HT);
    tree.Branch("n_jets", buffers.n_jets);
    tree.Branch("n_b_tagged_jets", buffers.n_b_tagged_jets);
    tree.Branch("n_c_tagged_jets", buffers.n_c_tagged_jets);
    tree.Branch("nPVsGood", buffers.nPVsGood);
    tree.Branch("ZCand_Mass", buffers.ZCand_Mass);
    tree.Branch("ZCand_Pt", buffers.ZCand_Pt);
    tree.Branch("ZCand_Eta", buffers.ZCand_Eta);
    tree.Branch("weight_mc", buffers.MCNormalization);
    tree.Branch("index_fold_spanet", buffers.fold_idx_spanet);
    tree.Branch("index_fold", buffers.fold_idx_tabnet);

    std::unordered_map<std::string, float> skeleton_weights(weight_map.begin(),
                                                            weight_map.end());
    if (systHelper) {
      const auto prebook_weights = systHelper->calculateWeight(true);
      skeleton_weights.insert(prebook_weights.begin(), prebook_weights.end());
    }

    buffers.weight_keys.reserve(skeleton_weights.size());
    buffers.weight_values.assign(skeleton_weights.size(), 0.f);

    std::size_t idx = 0;
    for (const auto &kv : skeleton_weights) {
      const std::string branch_name =
          "weight_" + sanitize_branch_name(kv.first);
      buffers.weight_keys.push_back(kv.first);
      buffers.weight_index.emplace(kv.first, idx);
      tree.Branch(branch_name, buffers.weight_values[idx]);
      ++idx;
    }
    buffers.booked = true;
  }

  buffers.Jet_Pt.clear();
  buffers.Jet_Eta.clear();
  buffers.Jet_Phi.clear();
  buffers.Jet_Mass.clear();
  buffers.Jet_Category.clear();
  buffers.Jet_ILR_Dim_1.clear();
  buffers.Jet_ILR_Dim_2.clear();
  for (const auto &jet : Jets) {
    buffers.Jet_Pt.push_back(jet.Pt());
    buffers.Jet_Eta.push_back(jet.Eta());
    buffers.Jet_Phi.push_back(jet.Phi());
    buffers.Jet_Mass.push_back(jet.M());
    buffers.Jet_Category.push_back(static_cast<int>(JetCategory(jet)));
    buffers.Jet_ILR_Dim_1.push_back(JetILRdim1Score(jet));
    buffers.Jet_ILR_Dim_2.push_back(JetILRdim2Score(jet));
  }

  buffers.Lepton_Pt.clear();
  buffers.Lepton_Eta.clear();
  buffers.Lepton_Phi.clear();
  buffers.Lepton_Mass.clear();
  for (const auto &lep : leptons) {
    buffers.Lepton_Pt.push_back(lep.Pt());
    buffers.Lepton_Eta.push_back(lep.Eta());
    buffers.Lepton_Phi.push_back(lep.Phi());
    buffers.Lepton_Mass.push_back(lep.M());
  }

  buffers.Met_Pt = MET.Pt();
  buffers.Met_Eta = MET.Eta();
  buffers.Met_Phi = MET.Phi();
  buffers.HT = HT;
  buffers.n_jets = n_jets;
  buffers.n_b_tagged_jets = n_b_tagged_jets;
  buffers.n_c_tagged_jets = n_c_tagged_jets;
  buffers.nPVsGood = ev.nPVsGood();
  buffers.MCNormalization = MCNormalizationWeight;

  if (leptons.size() == 2) {
    Particle ZCand = leptons[0] + leptons[1];
    buffers.ZCand_Mass = ZCand.M();
    buffers.ZCand_Pt = ZCand.Pt();
    buffers.ZCand_Eta = ZCand.Eta();
  } else {
    buffers.ZCand_Mass = -999.f;
    buffers.ZCand_Pt = -999.f;
    buffers.ZCand_Eta = -999.f;
  }

  std::fill(buffers.weight_values.begin(), buffers.weight_values.end(), 0.f);
  for (const auto &kv : weight_map) {
    auto idx_it = buffers.weight_index.find(kv.first);
    if (idx_it != buffers.weight_index.end() &&
        idx_it->second < buffers.weight_values.size()) {
      buffers.weight_values[idx_it->second] = kv.second;
    }
  }

  buffers.fold_idx_spanet = rle_bucket(RunNumber, luminosityBlock, event, 4);
  buffers.fold_idx_tabnet = rle_bucket(luminosityBlock, RunNumber, event, 4);

  if (MCSample.Contains("TT")) {
    auto [firstTopIdx, firstAntiTopIdx, lastTopIdx, lastAntiTopIdx] =
        GetTopAndAntiTopIndices(AllGenViews);

    const TLorentzVector top = AllGenViews[firstTopIdx].P4();
    const TLorentzVector antiTop = AllGenViews[firstAntiTopIdx].P4();

    OutputTree(tree_name).Set( "Gen_Top_Pt", top.Pt());
    OutputTree(tree_name).Set( "Gen_AntiTop_Pt", antiTop.Pt());
    OutputTree(tree_name).Set( "Gen_Top_Eta", top.Eta());
    OutputTree(tree_name).Set( "Gen_AntiTop_Eta", antiTop.Eta());
    OutputTree(tree_name).Set( "Gen_Top_Phi", top.Phi());
    OutputTree(tree_name).Set( "Gen_AntiTop_Phi", antiTop.Phi());
    OutputTree(tree_name).Set( "Gen_Top_Mass", top.M());
    OutputTree(tree_name).Set( "Gen_AntiTop_Mass", antiTop.M());
    OutputTree(tree_name).Set( "Gen_TTbar_Pt", (top + antiTop).Pt());
    OutputTree(tree_name).Set( "Gen_TTbar_Mass", (top + antiTop).M());
    OutputTree(tree_name).Set( "Gen_TTbar_Eta", (top + antiTop).Eta());
    OutputTree(tree_name).Set( "Gen_TTbar_Phi", (top + antiTop).Phi());

    float w_toppt = myCorr->GetTopPtReweight(top, antiTop);
    auto [topIdx, WTopIdx, BHadTopIdx, antiTopIdx, WAntiTopIdx,
          BHadAntiTopIdx] = myCorr->GetGenIdxofTopDecayProducts(AllGenViews);
    float weight_bfrag = 1.f;
    float weight_bfrag_up = 1.f;
    float xb = -1.f;
    float xb_anti = -1.f;
    if ((BHadTopIdx == std::numeric_limits<std::size_t>::max()) ||
        (BHadAntiTopIdx == std::numeric_limits<std::size_t>::max())) {
      weight_bfrag = -1.f;
      weight_bfrag_up = -1.f;
    } else {
      auto LastCopyTop = AllGenViews[topIdx].P4();
      auto LastCopyAntiTop = AllGenViews[antiTopIdx].P4();
      auto LastCopyWPlus = AllGenViews[WTopIdx].P4();
      auto LastCopyWMinus = AllGenViews[WAntiTopIdx].P4();
      auto FirstCopyAntiTopBHad = AllGenViews[BHadAntiTopIdx].P4();
      auto FirstCopyTopBHad = AllGenViews[BHadTopIdx].P4();

      const float x_e_top =
          2 * FirstCopyTopBHad * LastCopyTop / LastCopyTop.M2();
      const float x_e_antitop =
          2 * FirstCopyAntiTopBHad * LastCopyAntiTop / LastCopyAntiTop.M2();
      const float w_top = LastCopyWPlus.M2() / LastCopyTop.M2();
      const float w_antitop = LastCopyWMinus.M2() / LastCopyAntiTop.M2();
      const float clip_value = 1.2f;
      const float x_b_top = std::min(x_e_top / (1 - w_top), clip_value);
      const float x_b_antitop =
          std::min(x_e_antitop / (1 - w_antitop), clip_value);
      xb = x_b_top;
      xb_anti = x_b_antitop;

      weight_bfrag = myCorr->GetBFragReweight(
          LastCopyTop, LastCopyAntiTop, LastCopyWPlus, LastCopyWMinus,
          FirstCopyTopBHad, FirstCopyAntiTopBHad, MyCorrection::variation::nom);
      weight_bfrag_up = myCorr->GetBFragReweight(
          LastCopyTop, LastCopyAntiTop, LastCopyWPlus, LastCopyWMinus,
          FirstCopyTopBHad, FirstCopyAntiTopBHad, MyCorrection::variation::up);
    }

    OutputTree(tree_name).Set( "weight_top_pt", w_toppt);
    OutputTree(tree_name).Set( "weight_bfrag", weight_bfrag);
    OutputTree(tree_name).Set( "weight_bfrag_up", weight_bfrag_up);
    OutputTree(tree_name).Set( "xb", xb);
    OutputTree(tree_name).Set( "xb_anti", xb_anti);
  } else {
    OutputTree(tree_name).Set( "weight_top_pt", 1.f);
    OutputTree(tree_name).Set( "weight_bfrag", 1.f);
    OutputTree(tree_name).Set( "weight_bfrag_up", 1.f);
    OutputTree(tree_name).Set( "xb", -1.f);
    OutputTree(tree_name).Set( "xb_anti", -1.f);
  }

  OutputTree(tree_name).Set( "genTtbarId", genTtbarId);
  OutputTree(tree_name).Set( "decay_mode", static_cast<int>(tt_decay_code / 100));
  OutputTree(tree_name).Set( "run", static_cast<int>(RunNumber));
  OutputTree(tree_name).Set( "luminosityBlock", static_cast<int>(luminosityBlock));
  OutputTree(tree_name).Set( "event", static_cast<int>(event));

  try {
    InferONNX();
  } catch (const std::exception &e) {
    std::cerr << "[Vcb_SL::FillTreeAtThisPoint] SPANet inference failed: "
              << e.what() << "\n";
    onnx_inference_valid = false;
    assignment.fill(-1);
    std::fill(class_score_logp.begin(), class_score_logp.end(), 0.f);
    detection_score_logp = -999.f;
    assignment_logp = -999.f;
  }

  bool onnx_assignment_valid = onnx_inference_valid;
  for (std::size_t i = 2; i < assignment.size(); ++i) {
    const int idx = assignment[i];
    if (idx < 0 || static_cast<std::size_t>(idx) >= Jets.size()) {
      onnx_assignment_valid = false;
      break;
    }
  }

  for (std::size_t i = 0; i < class_score_logp.size(); ++i) {
    OutputTree(tree_name).Set( "logp_class_" + std::to_string(i), class_score_logp[i]);
  }
  OutputTree(tree_name).Set( "detection_score_logp", detection_score_logp);
  OutputTree(tree_name).Set( "assignment_logp", assignment_logp);
  OutputTree(tree_name).Set( "assignment_hb_idx", assignment[0]);
  OutputTree(tree_name).Set( "assignment_lb_idx", assignment[1]);
  OutputTree(tree_name).Set( "assignment_w1_idx", assignment[2]);
  OutputTree(tree_name).Set( "assignment_w2_idx", assignment[3]);
  OutputTree(tree_name).Set( "onnx_assignment_valid", onnx_assignment_valid);

  if (onnx_assignment_valid) {
    try {
      InferTabNet();
    } catch (const std::exception &e) {
      std::cerr << "[Vcb_SL::FillTreeAtThisPoint] TabNet inference failed: "
                << e.what() << "\n";
      final_template_score = -1.f;
      tabnet_class_logits.clear();
      tabnet_class_scores.clear();
      tabnet_weighted_scores.clear();
    }
  } else {
    final_template_score = -1.f;
    tabnet_class_logits.clear();
    tabnet_class_scores.clear();
    tabnet_weighted_scores.clear();
  }
  OutputTree(tree_name).Set( "Template_MVA_Score", final_template_score);
  for (std::size_t i = 0; i < 7; ++i) {
    const float logit_value =
        i < tabnet_class_logits.size() ? tabnet_class_logits[i] : -999.f;
    const float score_value =
        i < tabnet_class_scores.size() ? tabnet_class_scores[i] : -999.f;
    const float weighted_score_value =
        i < tabnet_weighted_scores.size() ? tabnet_weighted_scores[i] : -999.f;
    OutputTree(tree_name).Set( "tabnet_logit_" + std::to_string(i), logit_value);
    OutputTree(tree_name).Set( "tabnet_score_" + std::to_string(i), score_value);
    OutputTree(tree_name).Set( "tabnet_weighted_score_" + std::to_string(i),
              weighted_score_value);
  }

  try {
    EnsureWcbNNEvaluated();
    for (std::size_t i = 0; i < kWcbNNScoreBranchNames.size(); ++i) {
      OutputTree(tree_name).Set( kWcbNNScoreBranchNames[i], wcb_nn_scores[i]);
    }
    for (std::size_t i = 0; i < kWcbNNPreSoftmaxBranchNames.size(); ++i) {
      OutputTree(tree_name).Set( kWcbNNPreSoftmaxBranchNames[i],
                wcb_nn_pre_softmax[i]);
    }
    OutputTree(tree_name).Set( "nn_category", wcb_nn_category);
  } catch (const std::exception &e) {
    std::cerr << "[Vcb_SL::FillTreeAtThisPoint] Wcb NN inference failed: "
              << e.what() << "\n";
    wcb_nn_scores.fill(0.f);
    wcb_nn_pre_softmax.fill(0.f);
    wcb_nn_category = -1;
    wcb_nn_cache_valid = true;
    for (const char *branch_name : kWcbNNScoreBranchNames) {
      OutputTree(tree_name).Set( branch_name, 0.f);
    }
    for (const char *branch_name : kWcbNNPreSoftmaxBranchNames) {
      OutputTree(tree_name).Set( branch_name, 0.f);
    }
    OutputTree(tree_name).Set( "nn_category", -1);
  }

  OutputTree(tree_name).Fill();
}

void Vcb_SL::FillTemplateTrainingTree(
    const std::unordered_map<std::string, float> &weight_map) {
  for (const auto &kv : weight_map) {
    OutputTree("Template_Training_Tree").Set( "weight_" + kv.first, kv.second);
  }
  ttbar_jet_indices = FindTTbarJetIndices();
  Particle hw(Jets[assignment[2]].P4() + Jets[assignment[3]].P4());
  OutputTree("Template_Training_Tree").Set( "m_had_w", hw.M());
  auto fill_category_bits = [](int cat, float &N0, float &L0, float (&C)[5],
                               float (&B)[5]) {
    N0 = L0 = 0.f;
    for (int i = 0; i < 5; ++i) {
      C[i] = 0.f;
      B[i] = 0.f;
    }

    if (cat == 0) {
      N0 = 1.f;
    } else if (cat == 1) {
      L0 = 1.f;
    } else if (2 <= cat && cat <= 6) {
      // C0..C4
      C[cat - 2] = 1.f;
    } else if (7 <= cat && cat <= 11) {
      // B0..B4
      B[cat - 7] = 1.f;
    }
  };
  float N0_W1, L0_W1, C_W1[5], B_W1[5];
  fill_category_bits(static_cast<int>(JetCategory(Jets[assignment[2]])), N0_W1,
                     L0_W1, C_W1, B_W1);
  float N0_W2, L0_W2, C_W2[5], B_W2[5];
  fill_category_bits(static_cast<int>(JetCategory(Jets[assignment[3]])), N0_W2,
                     L0_W2, C_W2, B_W2);
  OutputTree("Template_Training_Tree").Set( "Cat_w_u",
            static_cast<int>(JetCategory(Jets[assignment[2]])));
  OutputTree("Template_Training_Tree").Set( "Cat_w_d",
            static_cast<int>(JetCategory(Jets[assignment[3]])));
  OutputTree("Template_Training_Tree").Set( "N0_w_u", N0_W1);
  OutputTree("Template_Training_Tree").Set( "L0_w_u", L0_W1);
  OutputTree("Template_Training_Tree").Set( "N0_w_d", N0_W2);
  OutputTree("Template_Training_Tree").Set( "L0_w_d", L0_W2);
  for (int i = 0; i < 5; ++i) {
    OutputTree("Template_Training_Tree").Set( "C" + std::to_string(i) + "_w_u",
              C_W1[i]);
    OutputTree("Template_Training_Tree").Set( "B" + std::to_string(i) + "_w_u",
              B_W1[i]);
    OutputTree("Template_Training_Tree").Set( "C" + std::to_string(i) + "_w_d",
              C_W2[i]);
    OutputTree("Template_Training_Tree").Set( "B" + std::to_string(i) + "_w_d",
              B_W2[i]);
  }
  OutputTree("Template_Training_Tree").Set( "ilr_dim1_w_u",
            JetILRdim1Score(Jets[assignment[2]]));
  OutputTree("Template_Training_Tree").Set( "ilr_dim2_w_u",
            JetILRdim2Score(Jets[assignment[2]]));
  OutputTree("Template_Training_Tree").Set( "ilr_dim1_w_d",
            JetILRdim1Score(Jets[assignment[3]]));
  OutputTree("Template_Training_Tree").Set( "ilr_dim2_w_d",
            JetILRdim2Score(Jets[assignment[3]]));
  OutputTree("Template_Training_Tree").Set( "pt_w_u", Jets[assignment[2]].Pt());
  OutputTree("Template_Training_Tree").Set( "pt_w_d", Jets[assignment[3]].Pt());
  OutputTree("Template_Training_Tree").Set( "eta_w_u", Jets[assignment[2]].Eta());
  OutputTree("Template_Training_Tree").Set( "eta_w_d", Jets[assignment[3]].Eta());

  OutputTree("Template_Training_Tree").Set( "logp_class_0", class_score_logp[0]);
  OutputTree("Template_Training_Tree").Set( "logp_class_1", class_score_logp[1]);
  OutputTree("Template_Training_Tree").Set( "logp_class_2", class_score_logp[2]);
  OutputTree("Template_Training_Tree").Set( "logp_class_3", class_score_logp[3]);
  OutputTree("Template_Training_Tree").Set( "logp_class_4", class_score_logp[4]);
  OutputTree("Template_Training_Tree").Set( "logp_class_5", class_score_logp[5]);

  OutputTree("Template_Training_Tree").Set( "index_fold",
            static_cast<int>(
                rle_bucket(luminosityBlock, RunNumber, event,
                           4))); // order of input is intentionally changed
  OutputTree("Template_Training_Tree").Set( "genTtbarId", genTtbarId);
  OutputTree("Template_Training_Tree").Set( "decay_mode", (tt_decay_code / 100));
  OutputTree("Template_Training_Tree").Set( "detection_score_logp",
            detection_score_logp);
  OutputTree("Template_Training_Tree").Set( "assignment_logp", assignment_logp);
  try {
    EnsureWcbNNEvaluated();
    for (std::size_t i = 0; i < kWcbNNScoreBranchNames.size(); ++i) {
      OutputTree("Template_Training_Tree").Set( kWcbNNScoreBranchNames[i],
                wcb_nn_scores[i]);
    }
    for (std::size_t i = 0; i < kWcbNNPreSoftmaxBranchNames.size(); ++i) {
      OutputTree("Template_Training_Tree").Set( kWcbNNPreSoftmaxBranchNames[i],
                wcb_nn_pre_softmax[i]);
    }
    OutputTree("Template_Training_Tree").Set( "nn_category", wcb_nn_category);
  } catch (const std::exception &e) {
    std::cerr
        << "[Vcb_SL::FillTemplateTrainingTree] Wcb NN inference failed: "
        << e.what() << "\n";
    wcb_nn_scores.fill(0.f);
    wcb_nn_pre_softmax.fill(0.f);
    wcb_nn_category = -1;
    wcb_nn_cache_valid = true;
    for (const char *branch_name : kWcbNNScoreBranchNames) {
      OutputTree("Template_Training_Tree").Set( branch_name, 0.f);
    }
    for (const char *branch_name : kWcbNNPreSoftmaxBranchNames) {
      OutputTree("Template_Training_Tree").Set( branch_name, 0.f);
    }
    OutputTree("Template_Training_Tree").Set( "nn_category", -1);
  }

  int chk_reco_correct = 0;
  if (ttbar_jet_indices[2] == assignment[2] &&
      ttbar_jet_indices[3] == assignment[3]) {
    chk_reco_correct = 1;
  }
  OutputTree("Template_Training_Tree").Set( "weight_mc", MCNormalization());
  OutputTree("Template_Training_Tree").Set( "chk_reco_correct", chk_reco_correct);
  OutputTree("Template_Training_Tree").Set( "n_bjets", n_loose_b_tagged_jets);
  OutputTree("Template_Training_Tree").Set( "n_jets", n_jets);
  OutputTree("Template_Training_Tree").Set( "n_cjets", n_loose_c_tagged_jets);
  OutputTree("Template_Training_Tree").Set( "ht", HT);
  OutputTree("Template_Training_Tree").Set( "Met_Pt", MET.Pt());




  OutputTree("Template_Training_Tree").Fill();
}

RVec<int> Vcb_SL::FindTTbarJetIndices() {
  // Return structure:
  //   { idx_bHad, idx_bLep, idx_Wq1, idx_Wq2 },
  // where each entry is the matched Reco Jet index (or -999 if not found).
  RVec<int> ttbar_jet_indices = {-1, -1, -1, -1};
  // Meaning:
  // ttbar_jet_indices[0] = b from hadronic top
  // ttbar_jet_indices[1] = b from leptonic top
  // ttbar_jet_indices[2] = first quark from hadronic W
  // ttbar_jet_indices[3] = second quark from hadronic W

  // For debugging or classification
  tt_decay_code = -9999;
  find_all_jets = false;

  // ------------------------------------------------------------------
  // 1) Find last copies of t, tbar, W+, W- in the Gen collection
  // ------------------------------------------------------------------
  int idx_t_plus = -1;
  int idx_t_minus = -1;
  int idx_w_plus = -1;
  int idx_w_minus = -1;
  bool found_t_plus = false;
  bool found_t_minus = false;
  bool found_w_plus = false;
  bool found_w_minus = false;

  // Traverse from end so the first time we see them is the "last copy".
  for (int i = (int)AllGens.size() - 1; i >= 0; i--) {
    const int pid = AllGens[i].PID();
    if (!found_w_plus && pid == 24) {
      idx_w_plus = i;
      found_w_plus = true;
    }
    if (!found_w_minus && pid == -24) {
      idx_w_minus = i;
      found_w_minus = true;
    }
    if (!found_t_plus && pid == 6) {
      idx_t_plus = i;
      found_t_plus = true;
    }
    if (!found_t_minus && pid == -6) {
      idx_t_minus = i;
      found_t_minus = true;
    }

    if (found_w_plus && found_w_minus && found_t_plus && found_t_minus)
      break;
  }

  // If for some reason we fail to find them, exit early
  if (!found_w_plus || !found_w_minus || !found_t_plus || !found_t_minus) {
    std::cout << "[ERROR] Did not find last copies of t/tbar/W+/W- in "
                 "semileptonic routine.\n";
    return ttbar_jet_indices; // all -1
  }

  // ------------------------------------------------------------------
  // 2) Determine which W is hadronic and which W is leptonic
  //    by looking at their final daughters.
  // ------------------------------------------------------------------
  // We'll store the indices of the final W daughters (2 for hadronic, or
  // lepton+nu for leptonic) Keep track of b from t, b from tbar, etc.
  bool w_plus_had = false;
  bool w_minus_had = false;

  // We also want to keep track of the final b from each top
  int idx_b_from_t_plus = -1;
  int idx_b_from_t_minus = -1;
  bool found_b_from_t_plus = false;
  bool found_b_from_t_minus = false;

  // W+ daughters:
  int idx_w_plus_dau1 = -1;
  int idx_w_plus_dau2 = -1;
  // W- daughters:
  int idx_w_minus_dau1 = -1;
  int idx_w_minus_dau2 = -1;

  // Loop over Gens to find final copies of b from top, and final copies of W
  // daughters
  for (int i = (int)AllGens.size() - 1; i >= 0; i--) {
    const auto &g = AllGens[i];
    int pid = g.PID();
    if (!g.isFirstCopy())
      continue;
    if (!(isPIDLepton(pid) || isPIDNeutrino(pid) || isPIDUpTypeQuark(pid) ||
          isPIDDownTypeQuark(pid)))
      continue;
    if (!g.isPrompt() ||
        !g.fromHardProcess()) // filter bremsstrahlung, semi-leptonic decay of
                              // b,c hadrons
      continue;
    // b from t
    if (!found_b_from_t_plus && pid == 5 && isDaughterOf(i, idx_t_plus) &&
        !isDaughterOf(i, idx_w_plus)) {
      idx_b_from_t_plus = i;
      found_b_from_t_plus = true;
    }
    // b-bar from tbar
    if (!found_b_from_t_minus && pid == -5 && isDaughterOf(i, idx_t_minus) &&
        !isDaughterOf(i, idx_w_minus)) {
      idx_b_from_t_minus = i;
      found_b_from_t_minus = true;
    }

    // W+ daughters
    if (isDaughterOf(i, idx_w_plus)) {
      if (idx_w_plus_dau1 < 0)
        idx_w_plus_dau1 = i;
      else if (idx_w_plus_dau2 < 0)
        idx_w_plus_dau2 = i;
    }
    // W- daughters
    if (isDaughterOf(i, idx_w_minus)) {
      if (idx_w_minus_dau1 < 0)
        idx_w_minus_dau1 = i;
      else if (idx_w_minus_dau2 < 0)
        idx_w_minus_dau2 = i;
    }

    // If we've found everything, we can break early
    if (found_b_from_t_plus && found_b_from_t_minus && idx_w_plus_dau1 >= 0 &&
        idx_w_plus_dau2 >= 0 && idx_w_minus_dau1 >= 0 &&
        idx_w_minus_dau2 >= 0) {
      break;
    }
  }

  // Check W+:
  int pid_Wp1 = (idx_w_plus_dau1 >= 0 ? AllGens[idx_w_plus_dau1].PID() : 0);
  int pid_Wp2 = (idx_w_plus_dau2 >= 0 ? AllGens[idx_w_plus_dau2].PID() : 0);
  bool wplus_has_lept = (isPIDLepton(pid_Wp1) || isPIDLepton(pid_Wp2));
  bool wplus_has_nu = (isPIDNeutrino(pid_Wp1) || isPIDNeutrino(pid_Wp2));
  // => If W+ has a lepton & neutrino => leptonic
  //    otherwise hadronic
  w_plus_had = !(wplus_has_lept && wplus_has_nu);

  // Similarly for W-:
  int pid_Wm1 = (idx_w_minus_dau1 >= 0 ? AllGens[idx_w_minus_dau1].PID() : 0);
  int pid_Wm2 = (idx_w_minus_dau2 >= 0 ? AllGens[idx_w_minus_dau2].PID() : 0);
  bool wminus_has_lept = (isPIDLepton(pid_Wm1) || isPIDLepton(pid_Wm2));
  bool wminus_has_nu = (isPIDNeutrino(pid_Wm1) || isPIDNeutrino(pid_Wm2));
  w_minus_had = !(wminus_has_lept && wminus_has_nu);

  // sorting the found result to be
  // dau1 = up quark or lepton
  // dau2 = down quark or neutrino
  if (w_plus_had) {
    if (isPIDLepton(pid_Wm2)) {
      std::swap(idx_w_minus_dau1, idx_w_minus_dau2);
      std::swap(pid_Wm1, pid_Wm2);
    }
    if (isPIDUpTypeQuark(pid_Wp2)) {
      std::swap(idx_w_plus_dau1, idx_w_plus_dau2);
      std::swap(pid_Wp1, pid_Wp2);
    }
    tt_decay_code = abs(pid_Wp1) * 1000 + abs(pid_Wp2) * 100 + abs(pid_Wm1);
    const auto neutrino = AllGens[idx_w_minus_dau2];
    gen_neutrino.SetPtEtaPhiM(neutrino.Pt(), neutrino.Eta(), neutrino.Phi(),
                              neutrino.Mass());
  } else if (w_minus_had) {
    if (isPIDLepton(pid_Wp2)) {
      std::swap(idx_w_plus_dau1, idx_w_plus_dau2);
      std::swap(pid_Wp1, pid_Wp2);
    }
    if (isPIDUpTypeQuark(pid_Wm2)) {
      std::swap(idx_w_minus_dau1, idx_w_minus_dau2);
      std::swap(pid_Wm1, pid_Wm2);
    }
    tt_decay_code = abs(pid_Wm1) * 1000 + abs(pid_Wm2) * 100 + abs(pid_Wp1);
    const auto neutrino = AllGens[idx_w_plus_dau2];
    gen_neutrino.SetPtEtaPhiM(neutrino.Pt(), neutrino.Eta(), neutrino.Phi(),
                              neutrino.Mass());
  }

  // In a semi-leptonic event, exactly one W should be hadronic and the other
  // leptonic. If both are hadronic or both are leptonic => code won't handle
  // that properly.
  bool found_all_gen =
      (found_b_from_t_plus && found_b_from_t_minus &&
       ((w_plus_had && !w_minus_had) || (!w_plus_had && w_minus_had)));
  if (!found_all_gen) {
    // std::cout << "[ERROR] This event does not appear to be semi-leptonic as
    // expected.\n";
    // //debugging what is wrong
    // std::cout << "w_plus_had: " << w_plus_had << " w_minus_had: " <<
    // w_minus_had << std::endl; std::cout << "found_b_from_t_plus: " <<
    // found_b_from_t_plus << " found_b_from_t_minus: " <<
    // found_b_from_t_minus
    // << std::endl; std::cout << "idx_w_plus_dau1: " << idx_w_plus_dau1 << "
    // idx_w_plus_dau2: " << idx_w_plus_dau2 << std::endl; std::cout <<
    // "idx_w_minus_dau1: " << idx_w_minus_dau1 << " idx_w_minus_dau2: " <<
    // idx_w_minus_dau2 << std::endl;
    // //pid
    // std::cout << "pid_Wp1: " << pid_Wp1 << " pid_Wp2: " << pid_Wp2 <<
    // std::endl; std::cout << "pid_Wm1: " << pid_Wm1 << " pid_Wm2: " <<
    // pid_Wm2
    // << std::endl;
    return ttbar_jet_indices; // all -1
  }

  Hists().Fill("FindTT_SemiLep_CutFlow", 0, 1.f, 10, 0., 10.);

  // ------------------------------------------------------------------
  // 3) Identify the hadronic side quarks & the leptonic side b
  // ------------------------------------------------------------------
  // We'll store hadronic side b and Wq1, Wq2, plus the other b is from
  // leptonic side.
  int idx_bHad = -1;
  int idx_bLep = -1;
  int idx_wq1 = -1;
  int idx_wq2 = -1;

  // If W+ is hadronic:
  if (w_plus_had) {
    idx_bHad = idx_b_from_t_plus;
    idx_bLep = idx_b_from_t_minus; // b from the other top
    // The W+ must have 2 quarks
    idx_wq1 = idx_w_plus_dau1;
    idx_wq2 = idx_w_plus_dau2;
  } else {
    // Then W- is hadronic
    idx_bHad = idx_b_from_t_minus;
    idx_bLep = idx_b_from_t_plus;
    // The W- must have 2 quarks
    idx_wq1 = idx_w_minus_dau1;
    idx_wq2 = idx_w_minus_dau2;
  }

  // ------------------------------------------------------------------
  // 4) Build a small vector of these 3 hadronic quarks (bHad + Wq1 + Wq2)
  //    plus the *leptonic b* if we also want to match it to a jet.
  // ------------------------------------------------------------------
  // Let's say we want to match 4 total jets: bHad, bLep, Wq1, Wq2.
  RVec<GenView> relevant_gens;
  relevant_gens.push_back(AllGens[idx_bHad]);
  relevant_gens.push_back(AllGens[idx_bLep]);
  relevant_gens.push_back(AllGens[idx_wq1]);
  relevant_gens.push_back(AllGens[idx_wq2]);

  // We'll keep an index-based ordering:
  //   0 => bHad, 1 => bLep, 2 => Wq1, 3 => Wq2
  // so we fill ttbar_jet_indices in that order eventually.

  // ------------------------------------------------------------------
  // 5) Match these 4 Gen objects to GenJets by PID + deltaR, then
  //    match those GenJets to Reco Jets.
  // ------------------------------------------------------------------
  // Group them by PID
  std::unordered_map<int, RVec<std::pair<size_t, GenView>>>
      map_pid_to_genIdxObj;
  for (size_t iG = 0; iG < relevant_gens.size(); iG++) {
    int pid = relevant_gens[iG].PID();
    map_pid_to_genIdxObj[pid].push_back({iG, relevant_gens[iG]});
  }

  // partonFlavour array for all GenJets
  RVec<int> genjet_flavours(AllGenJets.size());
  for (size_t i = 0; i < AllGenJets.size(); i++) {
    int genJet_flavour = AllGenJets[i].partonFlavour();
    genjet_flavours[i] = genJet_flavour;
  }

  // For each final-state gen object, we store "which GenJet it matched"
  // Start all unmatched -> -999
  std::unordered_map<size_t, int> map_genIndex_to_genJetIdx;
  for (size_t iG = 0; iG < relevant_gens.size(); iG++)
    map_genIndex_to_genJetIdx[iG] = -999;

  // Do matching by PID group
  for (auto &kv : map_pid_to_genIdxObj) {
    int target_pid = kv.first;
    auto &idxGenPairs = kv.second; // each element is { iG, GenObject }

    // Gather candidate GenJets that have the same partonFlavour == target_pid
    RVec<GenJetView> candidateGenJets;
    RVec<int> candidateGJIndices;
    for (size_t j = 0; j < genjet_flavours.size(); j++) {
      if (genjet_flavours[j] == target_pid) {
        candidateGenJets.push_back(AllGenJets[j]);
        candidateGJIndices.push_back(j);
      }
    }

    if (candidateGenJets.empty()) {
      // No GenJet for this PID
      for (auto &p : idxGenPairs)
        map_genIndex_to_genJetIdx.at(p.first) = -1;
      continue;
    }

    // Prepare only the Gen objects that share that PID
    RVec<GenView> these_gens;
    RVec<size_t> these_genIndices;
    these_gens.reserve(idxGenPairs.size());
    for (auto &p : idxGenPairs)
      these_gens.push_back(p.second);

    // Perform deltaR matching
    auto result_map =
        deltaRMatchingViews(these_gens, candidateGenJets, 0.4);

    // Store
    // result_map[iGenInGroup] -> iJetInCandidate (or -1)
    for (auto &matchPair : result_map) {
      size_t iGenInGroup = matchPair.first;
      int iGJetInGroup = matchPair.second;
      size_t releventGenIndex = idxGenPairs[iGenInGroup].first;
      if (iGJetInGroup >= 0)
        map_genIndex_to_genJetIdx.at(releventGenIndex) =
            candidateGJIndices[iGJetInGroup];
      else
        map_genIndex_to_genJetIdx.at(releventGenIndex) = -1;
    }
  }

  // Check how many are matched
  bool matched_all_genJets = true;
  for (size_t iG = 0; iG < relevant_gens.size(); iG++) {
    if (map_genIndex_to_genJetIdx[iG] < 0) {
      matched_all_genJets = false;
      break;
    }
  }

  if (matched_all_genJets)
    Hists().Fill("FindTT_SemiLep_CutFlow", 1, 1.f, 10, 0., 10.);

  find_all_genjets = matched_all_genJets;
  ttbar_AllGenJets_indices.clear();
  for (size_t iG = 0; iG < relevant_gens.size(); iG++) {
    if (map_genIndex_to_genJetIdx[iG] >= 0) {
      ttbar_AllGenJets_indices.push_back(map_genIndex_to_genJetIdx[iG]);
    } else {
      ttbar_AllGenJets_indices.push_back(-1);
    }
  }
  // Now match GenJets -> Reco Jets
  // Build the subset of GenJets that *were* matched
  RVec<GenJetView> matchedGenJets;
  RVec<size_t> matchedGenIndices; // which of the 4 did it come from

  for (size_t iG = 0; iG < relevant_gens.size(); iG++) {
    if (map_genIndex_to_genJetIdx[iG] >= 0) {
      matchedGenJets.push_back(AllGenJets[map_genIndex_to_genJetIdx[iG]]);
      matchedGenIndices.push_back(iG);
    }
  }

  auto recoMatchMap =
      GenJetMatchingViews(Jets, matchedGenJets, ev.GetRho(), 0.4, INFINITY);

  // Invert that map so we can see for i-th GenJet in matchedGenJets which
  // RecoJet was matched
  std::unordered_map<int, int> map_genJetIdx_inSubset_to_recoJetIdx;
  for (auto &kv : recoMatchMap) {
    int iRecoJet = kv.first;
    int iGenJetSub = kv.second; // index in matchedGenJets
    if (iGenJetSub >= 0)
      map_genJetIdx_inSubset_to_recoJetIdx[iGenJetSub] = iRecoJet;
  }

  // Fill the final 4-element array in order: { bHad, bLep, Wq1, Wq2 }
  // i.e. [0,1,2,3] from relevant_gens
  for (const auto &kv : map_genJetIdx_inSubset_to_recoJetIdx) {
    int iGenJetSub = kv.first;
    int iRecoJet = kv.second;
    int iG = matchedGenIndices[iGenJetSub];
    ttbar_jet_indices[iG] = iRecoJet;
  }

  // Check if all 4 are matched
  find_all_jets = true;
  for (auto idx : ttbar_jet_indices) {
    if (idx < 0) {
      find_all_jets = false;
      break;
    }
  }
  if (find_all_jets)
    Hists().Fill("FindTT_SemiLep_CutFlow", 2, 1.f, 10, 0., 10.);

  // ------------------------------------------------------------------
  // 6) Optionally fill some Gen-level or GenJet-level histograms
  // ------------------------------------------------------------------
  if (find_all_jets) {
    // Reconstruct hadronic top from (bHad + wq1 + wq2)
    TLorentzVector W_had =
        AllGens[idx_wq1].P4() + AllGens[idx_wq2].P4();
    TLorentzVector Top_had = W_had + AllGens[idx_bHad].P4();
    TLorentzVector Top_lep;
    if (w_plus_had)
      Top_lep = AllGens[idx_bLep].P4() + AllGens[idx_w_minus_dau1].P4() +
                AllGens[idx_w_minus_dau2].P4();
    else
      Top_lep = AllGens[idx_bLep].P4() + AllGens[idx_w_plus_dau1].P4() +
                AllGens[idx_w_plus_dau2].P4();
    Particle W_had_Reco(
        Jets[ttbar_jet_indices[2]].P4() + Jets[ttbar_jet_indices[3]].P4());
    Particle Top_had_Reco(W_had_Reco + Jets[ttbar_jet_indices[0]].P4());
    TLorentzVector Top_had_GenJet = matchedGenJets[0].P4() +
                                    matchedGenJets[2].P4() +
                                    matchedGenJets[3].P4();
    TLorentzVector W_had_GenJet =
        matchedGenJets[2].P4() + matchedGenJets[3].P4();
    // Reconstruct leptonic top from (bLep + lepton + neutrino) if you want
    // In the code above, we can figure out the indices of the lepton,
    // neutrino from the other W For demonstration, we just fill the hadronic
    // top mass:
    Hists().Fill("genLevel_HadTopMass", Top_had.M(), 1.f, 100, 100., 300.);
    Hists().Fill("genLevel_HadWMass", W_had.M(), 1.f, 100, 50., 110.);
    Hists().Fill("genLevel_LepTopMass", Top_lep.M(), 1.f, 100, 100., 300.);
    Hists().Fill("genLevel_LepWMass", W_had.M(), 1.f, 100, 50., 110.);
    Hists().Fill("GenJetLevel_HadTopMass", Top_had_GenJet.M(), 1.f, 100, 100.,
             300.);
    Hists().Fill("GenJetLevel_HadWMass", W_had_GenJet.M(), 1.f, 100, 50., 110.);
    Hists().Fill("recoLevel_HadTopMass", Top_had_Reco.M(), 1.f, 100, 100., 300.);
    Hists().Fill("recoLevel_HadWMass", W_had_Reco.M(), 1.f, 100, 50., 110.);
  }

  return ttbar_jet_indices;
}

void Vcb_SL::GetKineMaticFitterResult(const SelectedJetViewCollection &jets, Particle &MET,
                                      Lepton &lepton) {
  RVec<RVec<unsigned int>> possible_permutations = GetPermutations(jets);

  best_KF_result.chi2 = 9999.;
  std::variant<float, std::pair<float, float>> neutrino_pz =
      SolveNeutrinoPz(lepton, MET);
  for (const auto &permutation : possible_permutations) {
    int status = -999;
    float chi2 = 9999.;
    RVec<unsigned int> this_permutation;
    RVec<TLorentzVector> fitted_result;
    // check second element of neutrino_pz is nullptr or not
    float this_neutrino_pz;
    if (std::holds_alternative<float>(neutrino_pz)) {
      Particle neutrino = Particle();
      neutrino.SetXYZM(MET.Px(), MET.Py(), std::get<float>(neutrino_pz), 0.);
      auto result = FitKinFitter(jets, permutation, neutrino, lepton);
      status = std::get<0>(result);
      chi2 = std::get<1>(result);
      this_permutation = std::get<2>(result);
      fitted_result = std::get<3>(result);
      this_neutrino_pz = std::get<float>(neutrino_pz);
    } else {
      float pz1 = std::get<std::pair<float, float>>(neutrino_pz).first;
      float pz2 = std::get<std::pair<float, float>>(neutrino_pz).second;
      Particle neutrino1 = Particle();
      neutrino1.SetXYZM(MET.Px(), MET.Py(), pz1, 0.);
      Particle neutrino2 = Particle();
      neutrino2.SetXYZM(MET.Px(), MET.Py(), pz2, 0.);
      auto result1 = FitKinFitter(jets, permutation, neutrino1, lepton);
      auto result2 = FitKinFitter(jets, permutation, neutrino2, lepton);
      auto result =
          (std::get<1>(result1) < std::get<1>(result2)) ? result1 : result2;
      status = std::get<0>(result);
      chi2 = std::get<1>(result);
      this_permutation = std::get<2>(result);
      fitted_result = std::get<3>(result);
      this_neutrino_pz =
          (std::get<1>(result1) < std::get<1>(result2)) ? pz1 : pz2;
    }

    if (chi2 < best_KF_result.chi2 && status == 0) {
      best_KF_result.status = status;
      best_KF_result.chi2 = chi2;
      best_KF_result.best_had_t_b_idx = this_permutation[0];
      best_KF_result.best_lep_t_b_idx = this_permutation[1];
      best_KF_result.best_had_w1_idx = this_permutation[2];
      best_KF_result.best_had_w2_idx = this_permutation[3];
      best_KF_result.fitted_had_t_b = fitted_result[0];
      best_KF_result.fitted_lep_t_b = fitted_result[1];
      best_KF_result.fitted_had_w1 = fitted_result[2];
      best_KF_result.fitted_had_w2 = fitted_result[3];
      best_KF_result.fitted_lep = fitted_result[4];
      best_KF_result.fitted_neu = fitted_result[5];
      best_KF_result.best_neu_pz = this_neutrino_pz;
    }
  }
}

tuple<int, float, RVec<unsigned int>, RVec<TLorentzVector>>
Vcb_SL::FitKinFitter(const SelectedJetViewCollection &jets,
                     const RVec<unsigned int> &permutation, Particle &neutrino,
                     Lepton &lepton) {
  // Initialize the fitter with smart pointers
  std::unique_ptr<TKinFitter> fitter =
      std::make_unique<TKinFitter>("fitter", "fitter");
  fitter->reset();
  fitter->setVerbosity(0);
  fitter->setMaxNbIter(500);
  fitter->setMaxDeltaS(1e-2);
  fitter->setMaxF(1e-2);

  // Prepare particle and covariance matrices
  RVec<TLorentzVector> this_pts;
  RVec<TMatrixD> this_JERs;
  for (const auto &jet : jets) {
    TLorentzVector this_part = static_cast<TLorentzVector>(jet);
    this_pts.push_back(this_part);
    TMatrixD this_Cov(1, 1);
    float this_JER = jet.Pt();
    this_JER *= myCorr->GetJER(jet.Eta(), jet.Pt(), ev.GetRho());
    this_JER *= myCorr->GetJERSF(jet.Eta(), jet.Pt());
    this_Cov(0, 0) = this_JER * this_JER;
    this_JERs.push_back(this_Cov);
  }

  // Initialize the TFitParticlePt objects with smart pointers
  auto had_t_b = std::make_unique<TFitParticlePt>("had_t_b", "had_t_b",
                                                  &(this_pts[permutation[0]]),
                                                  &(this_JERs[permutation[0]]));
  auto lep_t_b = std::make_unique<TFitParticlePt>("lep_t_b", "lep_t_b",
                                                  &(this_pts[permutation[1]]),
                                                  &(this_JERs[permutation[1]]));
  auto had_w1 = std::make_unique<TFitParticlePt>(
      "w11", "w11", &(this_pts[permutation[2]]), &(this_JERs[permutation[2]]));
  auto had_w2 = std::make_unique<TFitParticlePt>(
      "w12", "w12", &(this_pts[permutation[3]]), &(this_JERs[permutation[3]]));

  TMatrixD lepton_Cov(1, 1);
  lepton_Cov(0, 0) = TMath::Power((lepton.Pt() * 0.0001), 2);
  auto lep =
      std::make_unique<TFitParticlePt>("lep", "lep", &(lepton), &(lepton_Cov));

  TVector3 neutrino_p3 = neutrino.Vect();
  auto neu = std::make_unique<TFitParticleMCCart>("neu", "neu", &(neutrino_p3),
                                                  0., nullptr);

  RVec<std::unique_ptr<TFitParticlePt>> extra_jets;
  for (size_t i = 0; i < jets.size(); i++) {
    if (std::find(permutation.begin(), permutation.end(), i) ==
        permutation.end()) {
      auto extra_jet = std::make_unique<TFitParticlePt>(
          "extra_jet_" + std::to_string(i), "extra_jet_" + std::to_string(i),
          &(this_pts[i]), &(this_JERs[i]));
      extra_jets.push_back(std::move(extra_jet));
    }
  }

  // Mass and energy-momentum constraints
  // auto mW1 = std::make_unique<TFitConstraintMGaus>("MW1", "MW1", nullptr,
  // nullptr, W_MASS, W_WIDTH);
  auto mHadW = std::make_unique<TFitConstraintMGaus>("MW1", "MW1", nullptr,
                                                     nullptr, W_MASS, W_WIDTH);
  mHadW->addParticle1(had_w1.get());
  mHadW->addParticle1(had_w2.get());

  // auto mW2 = std::make_unique<TFitConstraintMGaus>("MW2", "MW2", nullptr,
  // nullptr, W_MASS, W_WIDTH);
  auto mLepW = std::make_unique<TFitConstraintMGaus>("MW2", "MW2", nullptr,
                                                     nullptr, W_MASS, W_WIDTH);
  mLepW->addParticle1(lep.get());
  mLepW->addParticle1(neu.get());

  auto mHadT = std::make_unique<TFitConstraintMGaus>("MT1", "MT1", nullptr,
                                                     nullptr, T_MASS, T_WIDTH);
  mHadT->addParticle1(had_t_b.get());
  mHadT->addParticle1(had_w1.get());
  mHadT->addParticle1(had_w2.get());

  auto mLepT = std::make_unique<TFitConstraintMGaus>("MT2", "MT2", nullptr,
                                                     nullptr, T_MASS, T_WIDTH);
  mLepT->addParticle1(lep_t_b.get());
  mLepT->addParticle1(lep.get());
  mLepT->addParticle1(neu.get());

  // pt balance constraint
  auto px_balance = std::make_unique<TFitConstraintEp>(
      "px", "px", TFitConstraintEp::component::pX, 0.);
  auto py_balance = std::make_unique<TFitConstraintEp>(
      "py", "py", TFitConstraintEp::component::pY, 0.);
  px_balance->addParticle(had_t_b.get());
  px_balance->addParticle(lep_t_b.get());
  px_balance->addParticle(had_w1.get());
  px_balance->addParticle(had_w2.get());
  px_balance->addParticle(lep.get());
  px_balance->addParticle(neu.get());
  py_balance->addParticle(had_t_b.get());
  py_balance->addParticle(lep_t_b.get());
  py_balance->addParticle(had_w1.get());
  py_balance->addParticle(had_w2.get());
  py_balance->addParticle(lep.get());
  py_balance->addParticle(neu.get());
  for (size_t i = 0; i < extra_jets.size(); i++) {
    px_balance->addParticle(extra_jets[i].get());
    py_balance->addParticle(extra_jets[i].get());
  }

  // Add particles and constraints to the fitter
  fitter->addMeasParticle(had_t_b.get());
  fitter->addMeasParticle(lep_t_b.get());
  fitter->addMeasParticle(had_w1.get());
  fitter->addMeasParticle(had_w2.get());
  fitter->addMeasParticle(lep.get());
  fitter->addUnmeasParticle(neu.get());

  // add extra jets
  for (size_t i = 0; i < extra_jets.size(); i++) {
    fitter->addMeasParticle(extra_jets[i].get());
  }

  fitter->addConstraint(mHadW.get());
  fitter->addConstraint(mLepW.get());
  fitter->addConstraint(mHadT.get());
  fitter->addConstraint(mLepT.get());
  fitter->addConstraint(px_balance.get());
  fitter->addConstraint(py_balance.get());

  // Perform the fit
  fitter->fit();

  int status = fitter->getStatus();
  float chi2 = fitter->getS();
  // Collect fitted particles
  RVec<TLorentzVector> fitted_result = {TLorentzVector(*had_t_b->getCurr4Vec()),
                                        TLorentzVector(*lep_t_b->getCurr4Vec()),
                                        TLorentzVector(*had_w1->getCurr4Vec()),
                                        TLorentzVector(*had_w2->getCurr4Vec()),
                                        TLorentzVector(*lep->getCurr4Vec()),
                                        TLorentzVector(*neu->getCurr4Vec())};

  // Return the result as a tuple
  return std::make_tuple(status, chi2, permutation, fitted_result);
}

void Vcb_SL::InferONNX() {
  ttbar_indices_computed = false;
  onnx_inference_valid = false;
  assignment.fill(-1);
  std::fill(class_score_logp.begin(), class_score_logp.end(), 0.f);
  detection_score_logp = -999.f;
  assignment_logp = -999.f;
  class_label = classCategory::tt;
  size_t current_fold = rle_bucket(RunNumber, luminosityBlock, event, 4);

  // ------------------------
  // 1. 준비: 시퀀셜(Jet) 파트
  // ------------------------
  constexpr int max_jet_onnx = kMaxJetsForONNX;

  auto &Momenta_data = std::get<FloatArray>(onnx_input_data["Momenta_data"]);
  auto &Momenta_mask = std::get<BoolArray>(onnx_input_data["Momenta_mask"]);
  Momenta_data.clear();
  Momenta_mask.clear();

  for (int i = 0; i < max_jet_onnx; ++i) {
    if (i < static_cast<int>(Jets.size())) {
      const auto &j = Jets[i];

      const float pt = j.Pt();
      const float eta = j.Eta();
      const float phi = j.Phi();
      const float mass = j.M();

      // shared kinematics
      Momenta_data.push_back(pt);
      Momenta_data.push_back(eta);
      Momenta_data.push_back(static_cast<float>(TMath::Sin(phi)));
      Momenta_data.push_back(static_cast<float>(TMath::Cos(phi)));
      Momenta_data.push_back(mass);

      if constexpr (kUseILRJetFeatures) {
        const float ilrdim1 = JetILRdim1Score(j);
        const float ilrdim2 = JetILRdim2Score(j);
        Momenta_data.push_back(ilrdim1);
        Momenta_data.push_back(ilrdim2);
      } else {
        float N0 = 0.f;
        float L0 = 0.f;
        std::array<float, 5> C = {};
        std::array<float, 5> B = {};
        const int jetCat = static_cast<int>(JetCategory(j));
        if (jetCat == 0) {
          N0 = 1.f;
        } else if (jetCat == 1) {
          L0 = 1.f;
        } else if (2 <= jetCat && jetCat <= 6) {
          C[jetCat - 2] = 1.f;
        } else if (7 <= jetCat && jetCat <= 11) {
          B[jetCat - 7] = 1.f;
        }
        Momenta_data.push_back(N0);
        Momenta_data.push_back(L0);
        for (float value : C)
          Momenta_data.push_back(value);
        for (float value : B)
          Momenta_data.push_back(value);
      }

      Momenta_mask.push_back(1);
    } else {
      // padding jet
      for (int k = 0; k < kMomFeatDimONNX; ++k)
        Momenta_data.push_back(0.f);
      Momenta_mask.push_back(0);
    }
  }
  const size_t expected_reco_mom_size =
      static_cast<size_t>(max_jet_onnx) * kMomFeatDimONNX;
  if (Momenta_data.size() != expected_reco_mom_size ||
      Momenta_mask.size() != static_cast<size_t>(max_jet_onnx)) {
    std::cerr << "[InferONNX] Invalid RECO Momenta shape in "
              << (kUseILRJetFeatures ? "ILR" : "JetCategory") << " mode: got ("
              << Momenta_mask.size() << ", "
              << (Momenta_mask.empty()
                      ? 0
                      : Momenta_data.size() / Momenta_mask.size())
              << "), expected (" << max_jet_onnx << ", " << kMomFeatDimONNX
              << ")\n";
    return;
  }

  // ------------------------
  // 2. 글로벌 파트: MET
  // ------------------------
  auto &Met_data = std::get<FloatArray>(onnx_input_data["Met_data"]);
  auto &Met_mask = std::get<BoolArray>(onnx_input_data["Met_mask"]);
  Met_data.clear();
  Met_mask.clear();
  Met_data.push_back(static_cast<float>(MET.Pt()));
  Met_data.push_back(static_cast<float>(TMath::Sin(MET.Phi())));
  Met_data.push_back(static_cast<float>(TMath::Cos(MET.Phi())));
  Met_mask.push_back(1);

  // ------------------------
  // 3. 글로벌 파트: Lepton
  // ------------------------
  auto &Lepton_data = std::get<FloatArray>(onnx_input_data["Lepton_data"]);
  auto &Lepton_mask = std::get<BoolArray>(onnx_input_data["Lepton_mask"]);
  Lepton_data.clear();
  Lepton_mask.clear();
  Lepton_data.push_back(lepton.Pt());
  Lepton_data.push_back(lepton.Eta());
  Lepton_data.push_back(static_cast<float>(TMath::Sin(lepton.Phi())));
  Lepton_data.push_back(static_cast<float>(TMath::Cos(lepton.Phi())));
  Lepton_data.push_back(lepton.M());
  if (channel == Channel::Mu) {
    Lepton_data.push_back(1.f); // utag
    Lepton_data.push_back(0.f); // etag
  } else {
    Lepton_data.push_back(0.f);
    Lepton_data.push_back(1.f);
  }
  Lepton_mask.push_back(1);

  // ------------------------
  // 5. 추론 호출
  // ------------------------
  const auto &input_shape_reco = onnx_input_shape;
  auto input_shape_classif = onnx_input_shape;
  auto input_data_classif = onnx_input_data;
  auto &classif_mom = std::get<FloatArray>(input_data_classif["Momenta_data"]);
  auto &classif_mask = std::get<BoolArray>(input_data_classif["Momenta_mask"]);
  input_shape_classif["Momenta_data"] = {1, max_jet_onnx, kMomFeatDimONNX};
  input_shape_classif["Momenta_mask"] = {1, max_jet_onnx};
  const size_t expected_classif_mom_size =
      static_cast<size_t>(max_jet_onnx) * kMomFeatDimONNX;
  if (classif_mom.size() != expected_classif_mom_size ||
      classif_mask.size() != static_cast<size_t>(max_jet_onnx)) {
    std::cerr << "[InferONNX] Invalid CLASSIF Momenta shape in "
              << (kUseILRJetFeatures ? "ILR" : "JetCategory") << " mode: got ("
              << classif_mask.size() << ", "
              << (classif_mask.empty()
                      ? 0
                      : classif_mom.size() / classif_mask.size())
              << "), expected (" << max_jet_onnx << ", " << kMomFeatDimONNX
              << ")\n";
    return;
  }

  std::unordered_map<std::string, FloatArray> output_data_classif =
      myMLHelper_CLASSIF_folds[current_fold]->Run_ONNX_Model(
          input_data_classif, input_shape_classif);

  std::unordered_map<std::string, FloatArray> output_data_reco =
      myMLHelper_RECO_folds[current_fold]->Run_ONNX_Model(onnx_input_data,
                                                          input_shape_reco);
  // ------------------------
  // 6. (옵션) 분류 점수 있으면 사용
  // ------------------------
  {
    const auto &signalScores = output_data_classif.at("EVENT/signal");
    for (size_t i = 0; i < signalScores.size(); ++i)
      class_score_logp[i] = signalScores[i];
    // if (signalScores.size() >= class_score.size()) {
    //   for (size_t i = 0; i < class_score.size(); ++i)
    //     class_score[i] = std::exp(signalScores.at(i));

    // std::array<float, 4> class_score_temp = {class_score[0], class_score[1],
    //                                          class_score[2], class_score[3],
    //                                         class_score[4], class_score[5]};
    // class_score_temp[0] *= 0.00543f;
    // class_score_temp[1] *= 0.86851f;
    // class_score_temp[2] *= 0.12068f;
    // class_score_temp[3] *= 0.00538f;

    // int max_class = std::distance(
    //     class_score_temp.begin(),
    //     std::max_element(class_score_temp.begin(), class_score_temp.end()));
    // switch (max_class) {
    // case 0:
    //   class_label = classCategory::Signal;
    //   break;
    // case 1:
    //   class_label = classCategory::tt;
    //   break;
    // case 2:
    //   class_label = classCategory::ttC;
    //   break;
    // case 3:
    //   class_label = classCategory::ttB;
    //   break;
    // default:
    //   break;
    // }
    //}
  }

  // ------------------------
  // 7. 후처리: logits에서 최적 해석 조합 찾기

  // logits
  // auto &ltlog_logits =
  //     output_data_reco.at("lt_assignment_log_probability"); // [1, max_jet]
  // auto &htlog_logits = output_data_reco.at(
  //     "ht_assignment_log_probability"); // [1, max_jet, max_jet, max_jet] ->
  //                                       // (hb, w1, w2)
  auto &hw45log_logits = output_data_reco.at(
      "hw_45_assignment_log_probability"); // [1, max_jet, max_jet] -> (w1, w2)

  auto &hw45log_detection_logits =
      output_data_reco.at("hw_45_detection_log_probability");

  detection_score_logp = hw45log_detection_logits[0];

  const int J = (int)std::min<size_t>(max_jet_onnx, Jets.size());

  // shapes (배치차원 포함)
  std::vector<int> hw45_shape = {1, max_jet_onnx, max_jet_onnx};
  std::vector<int> ht_shape = {1, max_jet_onnx, max_jet_onnx, max_jet_onnx};
  std::vector<int> lt_shape = {1, max_jet_onnx};

  int w1_assignment = -1, w2_assignment = -1, hb_assignment = -1,
      lb_assignment = -1;

  // ---------------------------
  // 1) hw_45: (w1, w2) 먼저 선택
  // ---------------------------
  for (size_t r = 0; r < hw45log_logits.size(); ++r) {
    size_t idx = FindNthMaxIndex(hw45log_logits, (int)r);
    auto unr = UnravelIndex((int)idx, hw45_shape); // [0, w1, w2]
    int c1 = unr[1], c2 = unr[2];
    if (c1 >= J || c2 >= J)
      continue; // 패딩 무시
    if (c1 == c2)
      continue; // 서로 달라야 함
    w1_assignment = c1;
    w2_assignment = c2;
    assignment_logp = hw45log_logits.at(idx);
    break;
  }
  if (w1_assignment < 0 || w2_assignment < 0) {
    std::cerr << "[InferONNX] No valid (w1,w2) from hw_45 logits.\n";
    return;
  }

  // ---------------------------
  // 2) ht: 고른 (w1,w2)에 맞춰 hb 선택
  // ---------------------------
  // for (size_t r = 0; r < htlog_logits.size(); ++r) {
  //   size_t idx = FindNthMaxIndex(htlog_logits, (int)r);
  //   auto unr = UnravelIndex((int)idx, ht_shape); // [0, hb, w1', w2']
  //   int hb_c = unr[1], w1_c = unr[2], w2_c = unr[3];
  //   if (hb_c >= J || w1_c >= J || w2_c >= J)
  //     continue;
  //   if (w1_c != w1 || w2_c != w2)
  //     continue; // 우리가 고른 (w1,w2)만 허용
  //   if (hb_c == w1 || hb_c == w2)
  //     continue; // 유니크
  //   hb = hb_c;
  //   break;
  // }
  // // fallback: 위 루프에서 못 찾으면 (w1,w2) slice에서 hb argmax 직접 찾기
  // if (hb < 0) {
  //   float best = -std::numeric_limits<float>::infinity();
  //   for (int a = 0; a < J; ++a) {
  //     if (a == w1 || a == w2)
  //       continue;
  //     size_t flat = (size_t)(0 * max_jet_onnx * max_jet_onnx * max_jet_onnx +
  //                            a * max_jet_onnx * max_jet_onnx +
  //                            w1 * max_jet_onnx + w2);
  //     float s = htlog_logits.at(flat);
  //     if (s > best) {
  //       best = s;
  //       hb = a;
  //     }
  //   }
  //   if (hb < 0) {
  //     std::cerr << "[InferONNX] No valid hb for fixed (w1,w2).\n";
  //     return;
  //   }
  // }

  // // ---------------------------
  // // 3) lt: (hb,w1,w2)와 모두 다른 lb 선택
  // // ---------------------------
  // for (size_t r = 0; r < ltlog_logits.size(); ++r) {
  //   size_t idx = FindNthMaxIndex(ltlog_logits, (int)r);
  //   auto unr = UnravelIndex((int)idx, lt_shape); // [0, lb]
  //   int lb_c = unr[1];
  //   if (lb_c >= J)
  //     continue;
  //   if (lb_c == hb || lb_c == w1 || lb_c == w2)
  //     continue; // 유니크
  //   lb = lb_c;
  //   break;
  // }
  // if (lb < 0) {
  //   std::cerr << "[InferONNX] No valid lb distinct from (hb,w1,w2).\n";
  //   return;
  // }

  // ---------------------------
  // 4) 최종 검증 & 기록
  // ---------------------------
  const int nJ = (int)Jets.size();
  for (int idx : {w1_assignment, w2_assignment}) {
    if (idx < 0 || idx >= nJ) {
      std::cerr << "[InferONNX] bad index from logits (nJets=" << nJ << ")\n";
      return;
    }
  }
  assignment[0] = hb_assignment;
  assignment[1] = lb_assignment;
  assignment[2] = w1_assignment;
  assignment[3] = w2_assignment;

  onnx_inference_valid = true;
}

void Vcb_SL::InferTabNet() {
  final_template_score = -1.f;
  tabnet_class_logits.clear();
  tabnet_class_scores.clear();
  tabnet_weighted_scores.clear();

  static const std::array<float, 7> class_weight_vec = {                1.0,
                0.8231302499771118,
                0.6102186441421509,
                0.5526220202445984,
                0.14842554926872253,
                6.188438892364502,
                4.698705196380615,};

  //                                                "varlist": [
  //   "m_had_w",
  //   "pt_w_u",
  //   "pt_w_d",
  //   "eta_w_u",
  //   "eta_w_d",
  //   "Cat_w_u",
  //   "Cat_w_d",
  //   "pre_softmax_tt_Wcb",
  //   "pre_softmax_ttLF",
  //   "pre_softmax_ttcj",
  //   "pre_softmax_tt2c",
  //   "pre_softmax_ttcc",
  //   "pre_softmax_ttbj",
  //   "pre_softmax_tt2b",
  //   "pre_softmax_ttbb",
  //   "detection_score_logp",
  //   "assignment_logp",
  //   "n_bjets",
  //   "n_jets",
  //   "n_cjets",
  //   "ht",
  //   "Met_Pt",
  // ]

  auto &input_vector = std::get<FloatArray>(tabnet_input_data["input"]);
  const int expected_tabnet_input_dim =
      1 + 2 + 2 + (kUseILRJetFeatures ? 4 : 2) +
      static_cast<int>(kWcbNNPreSoftmaxBranchNames.size()) + 2 +
      kTabNetEventContextFeatDim;
  input_vector.clear();
  input_vector.reserve(expected_tabnet_input_dim);
  EnsureWcbNNEvaluated();

  // m_had_w
  {
    Particle hw(Jets[assignment[2]].P4() + Jets[assignment[3]].P4());
    input_vector.push_back(hw.M());
  }
  // pt_w_u
  input_vector.push_back(Jets[assignment[2]].Pt());
  // pt_w_d
  input_vector.push_back(Jets[assignment[3]].Pt());
  // eta_w_u
  input_vector.push_back(Jets[assignment[2]].Eta());
  // eta_w_d
  input_vector.push_back(Jets[assignment[3]].Eta());
  if constexpr (kUseILRJetFeatures) {
    // ilrdim1_w_u / ilrdim1_w_d / ilrdim2_w_u / ilrdim2_w_d
    input_vector.push_back(JetILRdim1Score(Jets[assignment[2]]));
    input_vector.push_back(JetILRdim1Score(Jets[assignment[3]]));
    input_vector.push_back(JetILRdim2Score(Jets[assignment[2]]));
    input_vector.push_back(JetILRdim2Score(Jets[assignment[3]]));
  } else {
    // Cat_w_u / Cat_w_d
    input_vector.push_back(
        static_cast<float>(JetCategory(Jets[assignment[2]])));
    input_vector.push_back(
        static_cast<float>(JetCategory(Jets[assignment[3]])));
  }
  // pre_softmax_tt_Wcb ~ pre_softmax_ttbb
  for (float pre_softmax : wcb_nn_pre_softmax)
    input_vector.push_back(pre_softmax);
  // detection_score_logp
  input_vector.push_back(detection_score_logp);
  // assignment_logp
  input_vector.push_back(assignment_logp);
  input_vector.push_back(n_loose_b_tagged_jets);
  input_vector.push_back(n_jets);
  input_vector.push_back(n_loose_c_tagged_jets);
  input_vector.push_back(HT);
  input_vector.push_back(MET.Pt());
  if (static_cast<int>(input_vector.size()) != expected_tabnet_input_dim) {
    std::cerr << "[InferTabNet] Invalid TabNet input feature size in "
              << (kUseILRJetFeatures ? "ILR" : "JetCategory") << " mode: got "
              << input_vector.size() << ", expected "
              << expected_tabnet_input_dim << "\n";
    final_template_score = -1.f;
    return;
  }
  int this_fold = rle_bucket(luminosityBlock, RunNumber, event, 4);
  std::unordered_map<std::string, FloatArray> output_data =
      myMLHelper_TabNet_folds[this_fold]->Run_ONNX_Model(tabnet_input_data,
                                                         tabnet_input_shape);
  const auto &tabnet_logits_output = output_data.at("logits"); // [1, 7]
  tabnet_class_logits.assign(tabnet_logits_output.begin(),
                             tabnet_logits_output.end());

  // element-wise exp (vectorized) with reusable buffers
  tabnet_class_scores.reserve(tabnet_class_logits.size());
  tabnet_weighted_scores.resize(tabnet_class_logits.size());

  if (!tabnet_class_logits.empty()) {
    const float max_val = *std::max_element(tabnet_class_logits.begin(),
                                            tabnet_class_logits.end());
    float sum = 0.f;
    for (float x : tabnet_class_logits) {
      float e = std::exp(x - max_val);
      tabnet_class_scores.push_back(e);
      sum += e;
    }
    if (sum > 0.f) {
      const float inv_sum = 1.0f / sum;
      for (auto &p : tabnet_class_scores) {
        p *= inv_sum;
      }
    }
  }

  std::transform(tabnet_class_scores.begin(), tabnet_class_scores.end(),
                 class_weight_vec.begin(), tabnet_weighted_scores.begin(),
                 [](float score, float w) { return score * w; });

  if (tabnet_weighted_scores.size() < 2) {
    final_template_score = -1.f;
    return;
  }

  final_template_score = tabnet_weighted_scores[0] + tabnet_weighted_scores[1];
  final_template_score /= std::accumulate(tabnet_weighted_scores.begin(),
                                          tabnet_weighted_scores.end(), 0.f);
}

bool Vcb_SL::FillTabNetInfo(const TString &histPrefix, float weight) {
  Hists().Fill(histPrefix + "/" + "Template_MVA_Score", final_template_score,
           weight, 200, 0., 1.);
  return true;
}

bool Vcb_SL::FillONNXRecoInfo(const TString &histPrefix, float weight) {
  if (!onnx_inference_valid) {
    InferONNX();
    if (!onnx_inference_valid) {
      std::cerr << "[Vcb_SL::FillONNXRecoInfo] ONNX inference failed; skipping "
                   "event.\n";
      return false;
    }
  }

  for (size_t i = 2; i < assignment.size(); ++i) {
    int idx = assignment[i];
    if (idx < 0 || static_cast<std::size_t>(idx) >= Jets.size()) {
      std::cerr
          << "[Vcb_SL::FillONNXRecoInfo] Received invalid assignment index "
          << idx << " (nJets=" << Jets.size() << "). Skipping event.\n";
      return false;
    }
  }

  if (!ttbar_indices_computed) {
    ttbar_jet_indices = MCSample.Contains("TT")
                            ? FindTTbarJetIndices()
                            : std::vector<int>{-1, -1, -1, -1};
    ttbar_indices_computed = true;
  }
  if (find_all_jets) {
    Hists().Fill(histPrefix + "/" + "CorrectAssignment_Tot", n_jets,
             n_b_tagged_jets, 1., 6, 4., 10., 4, 2, 6);
    bool isCorrect = true;
    // check if the assignment is correct. w1 and w2 can be swapped
    // if (assignment[0] != ttbar_jet_indices[0]) isCorrect = false;
    // if (assignment[1] != ttbar_jet_indices[1]) isCorrect = false;
    if ((assignment[2] != ttbar_jet_indices[2] ||
         assignment[3] != ttbar_jet_indices[3]) &&
        (assignment[2] != ttbar_jet_indices[3] ||
         assignment[3] != ttbar_jet_indices[2]))
      isCorrect = false;
    if (isCorrect)
      Hists().Fill(histPrefix + "/" + "CorrectAssignment", n_jets, n_b_tagged_jets,
               1., 6, 4., 10., 4, 2, 6);
    else
      Hists().Fill(histPrefix + "/" + "WrongAssignment", n_jets, n_b_tagged_jets,
               1., 6, 4., 10., 4, 2, 6);
  }

  Particle hw(Jets[assignment[2]].P4() + Jets[assignment[3]].P4());

  Hists().Fill(histPrefix + "/" + "Reco_HadWMass", hw.M(), weight, 50, 30., 130.);
  Hists().Fill(histPrefix + "/" + "Reco_W1JetPt", Jets[assignment[2]].Pt(), weight,
           50, 0., 200.);
  Hists().Fill(histPrefix + "/" + "Reco_W2JetPt", Jets[assignment[3]].Pt(), weight,
           50, 0., 200.);
  Hists().Fill(histPrefix + "/" + "Reco_W1_Cat",
           static_cast<int>(JetCategory(Jets[assignment[2]])), weight, 12, 0.,
           12.);
  Hists().Fill(histPrefix + "/" + "Reco_W2_Cat",
           static_cast<int>(JetCategory(Jets[assignment[3]])), weight, 12, 0.,
           12.);
  Hists().Fill(histPrefix + "/" + "Reco_W1_BvC", JetBvCScore(Jets[assignment[2]]),
           weight, 50, 0., 1.);
  Hists().Fill(histPrefix + "/" + "Reco_W2_BvC", JetBvCScore(Jets[assignment[3]]),
           weight, 50, 0., 1.);
  Hists().Fill(histPrefix + "/" + "Reco_W1_HFvLF",
           JetHFvLFScore(Jets[assignment[2]]), weight, 50, 0., 1.);
  Hists().Fill(histPrefix + "/" + "Reco_W2_HFvLF",
           JetHFvLFScore(Jets[assignment[3]]), weight, 50, 0., 1.);

  int unrolledIdx = Unroller(Jets[assignment[2]], Jets[assignment[3]]);
  Hists().Fill(histPrefix + "/" + "Reco_W1Cat_W2Cat_Unrolled", unrolledIdx, weight,
           144, 0., 144.);
  Hists().Fill(histPrefix + "/" + "Reco_W1Cat_W2Cat_Unrolled_2D",
           static_cast<int>(JetCategory(Jets[assignment[2]])),
           static_cast<int>(JetCategory(Jets[assignment[3]])), weight, 12, 0.,
           12., 12, 0., 12.);

  Hists().Fill(histPrefix + "/" + "Class_Score0",
           static_cast<float>(exp(class_score_logp[0])), weight, 50, 0., 1.);
  Hists().Fill(histPrefix + "/" + "Class_Score1",
           static_cast<float>(exp(class_score_logp[1])), weight, 50, 0., 1.);
  Hists().Fill(histPrefix + "/" + "Class_Score2",
           static_cast<float>(exp(class_score_logp[2])), weight, 50, 0., 1.);
  Hists().Fill(histPrefix + "/" + "Class_Score3",
           static_cast<float>(exp(class_score_logp[3])), weight, 50, 0., 1.);
  Hists().Fill(histPrefix + "/" + "Class_Score4",
           static_cast<float>(exp(class_score_logp[4])), weight, 50, 0., 1.);
  Hists().Fill(histPrefix + "/" + "Class_Score5",
           static_cast<float>(exp(class_score_logp[5])), weight, 50, 0., 1.);

  Hists().Fill(histPrefix + "/" + "detection_score",
           static_cast<float>(exp(detection_score_logp)), weight, 50, 0., 1.);
  Hists().Fill(histPrefix + "/" + "assignment_score",
           static_cast<float>(exp(assignment_logp)), weight, 50, 0., 1.);
  return true;
}
