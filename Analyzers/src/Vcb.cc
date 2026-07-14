#include "Vcb.h"
#include "BranchManager.h"
#include "GenView.h"
#include "OtJsonLutBank.h"
#include "TBranch.h"
#include "TEntryList.h"
#include "TEntryListArray.h"
#include "TObjArray.h"
#include "UParTScoreUtils.h"
#include <TLorentzVector.h>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <sstream>
#include <system_error>
#include <unordered_map>
#include <unordered_set>

namespace {
std::string to_lower_ascii(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
    return static_cast<char>(std::tolower(c));
  });
  return s;
}

bool starts_with(const std::string &value, const std::string &prefix) {
  return value.size() >= prefix.size() &&
         value.compare(0, prefix.size(), prefix) == 0;
}

bool is_digits(const std::string &value) {
  return !value.empty() &&
         std::all_of(value.begin(), value.end(),
                     [](unsigned char c) { return std::isdigit(c) != 0; });
}

bool is_bootstrap_source(const std::string &source) {
  const std::string lowered = to_lower_ascii(source);
  return starts_with(lowered, "bootstrap_");
}

std::string normalize_bootstrap_source_key(std::string source) {
  source = to_lower_ascii(std::move(source));
  constexpr char prefix[] = "bootstrap_";
  if (!starts_with(source, prefix))
    return "";

  std::string index = source.substr(sizeof(prefix) - 1);
  if (!is_digits(index))
    return "";
  if (index.size() == 1)
    index = "0" + index;
  return "syst_target_bootstrap_" + index;
}

std::string ot_source_to_integrated_stem(std::string source) {
  source = to_lower_ascii(std::move(source));
  if (source == "fsr")
    return "syst_weight_FSR";
  if (source == "isr")
    return "syst_weight_ISR";
  if (source == "muf")
    return "syst_weight_MuF";
  if (source == "mur")
    return "syst_weight_MuR";
  if (source == "pileup")
    return "syst_weight_Pileup";
  if (source == "jet_en" || source == "jes")
    return "syst_tree_Jet_En";
  if (source == "jet_res" || source == "jer")
    return "syst_tree_Jet_Res";
  if (starts_with(source, "priorsf_"))
    return "syst_target_priorSF_" + source.substr(8);
  return "";
}

std::vector<float> ot_pt_edges() {
  return {25.f, 35.f, 50.f, 70.f, 90.f, 120.f, 1e+8f};
}

UParTScore::Prob3 remap_prob3_with_second_lut(const UParTScore::Mapper &mapper,
                                              const UParTScore::Prob3 &p_in,
                                              float pt, int hadronFlavor) {
  constexpr double eps = 1e-12;
  const double pb = std::max(p_in.pb, eps);
  const double pc = std::max(p_in.pc, eps);
  const double pl = std::max(p_in.pl, eps);

  // Build a branch-like tuple reproducing current (pb, pc, pl),
  // then run the second OT map.
  const double probudg = pl;
  const double SvUDG = 0.0;
  const double CvL = pc / std::max(pc + pl, eps);
  const double CvB = pc / std::max(pb + pc, eps);

  return mapper.MappedProb3(probudg, SvUDG, CvL, CvB, pt, hadronFlavor);
}

float strict_ot_pt_value(const std::vector<float> &pts, std::size_t idx,
                         const char *label, const std::string &syst_target) {
  if (idx >= pts.size()) {
    throw std::runtime_error("[Vcb::OtLutPtFromStore] Missing " +
                             std::string(label) + " for jet index " +
                             std::to_string(idx) +
                             " (systematic target: " + syst_target + ")");
  }

  const float pt = pts[idx];
  if (!(pt > 0.f) || !std::isfinite(pt)) {
    throw std::runtime_error("[Vcb::OtLutPtFromStore] Invalid " +
                             std::string(label) + " = " + std::to_string(pt) +
                             " at jet index " + std::to_string(idx) +
                             " (systematic target: " + syst_target + ")");
  }
  return pt;
}

float ot_pt_value_or_nominal(const std::vector<float> &pts, std::size_t idx,
                             const std::vector<float> &nominal_pts,
                             const std::string &syst_target) {
  const float nominal_pt =
      strict_ot_pt_value(nominal_pts, idx, "smearedPtNominal", syst_target);

  if (idx >= pts.size())
    return nominal_pt;

  const float pt = pts[idx];
  if (!(pt > 0.f) || !std::isfinite(pt))
    return nominal_pt;

  return pt;
}
} // namespace

// namespace {

// constexpr double HF_T1 = 0.264;
// constexpr double HF_T2 = 0.448;
// constexpr double HF_T3 = 0.767;
// constexpr double BVC_T1 = 0.01;
// constexpr double BVC_T2 = 0.028;
// constexpr double BVC_T3 = 0.094;
// constexpr double BVC_T4 = 0.69;
// constexpr double BVC_T5 = 0.918;
// constexpr double BVC_T6 = 0.978;
// constexpr double BVC_T7 = 0.994;

// int bin_hf(double x) {
//   return int(x >= HF_T1) + int(x >= HF_T2) + int(x >= HF_T3);
// }

// int bin_bvc(double y) {
//   return int(y >= BVC_T1) + int(y >= BVC_T2) + int(y >= BVC_T3) +
//          int(y >= BVC_T4) + int(y >= BVC_T5) + int(y >= BVC_T6) +
//          int(y >= BVC_T7);
// }

// std::pair<double, double> HFvLF_BvC_from_components(double probudg,
//                                                     double SvUDG, double CvL,
//                                                     double CvB) {
//   const double probs = (SvUDG > 0.0 && probudg > 0.0 && SvUDG < 1.0)
//                            ? SvUDG * probudg / (1.0 - SvUDG)
//                            : -1.0;

//   const double probc = (CvL > 0.0 && CvL < 1.0 && probs > 0.0 && probudg >
//   0.0)
//                            ? CvL * (probs + probudg) / (1.0 - CvL)
//                            : -1.0;

//   const double probbbblepb =
//       (CvB > 0.0 && probc > 0.0) ? (1.0 - CvB) * probc / CvB : -1.0;

//   const double BvC = (CvB > 0.0) ? (1.0 - CvB) : -1.0;

//   const double HFvLF =
//       (probbbblepb > 0.0 && probc > 0.0 && probs > 0.0 && probudg > 0.0)
//           ? (probbbblepb + probc) / (probbbblepb + probc + probs + probudg)
//           : -1.0;
//   return {HFvLF, BvC};
// }

// std::pair<double, double> HFvLF_BvC_from_ParT(const Jet &j) {
//   using Tagger = JetTagging::JetFlavTagger;
//   using Score = JetTagging::JetFlavTaggerScoreType;

//   return HFvLF_BvC_from_components(
//       j.GetTaggerResult(Tagger::ParT, Score::probUDG),
//       j.GetTaggerResult(Tagger::ParT, Score::SvUDG),
//       j.GetTaggerResult(Tagger::ParT, Score::CvL),
//       j.GetTaggerResult(Tagger::ParT, Score::CvB));
// }

// std::pair<double, double> HFvLF_BvC_from_storage(const JetSoA &store,
//                                                  std::size_t idx) {
//   return HFvLF_BvC_from_components(
//       store.uparTAK4UDG[idx], store.uparTAK4SvUDG[idx],
//       store.uparTAK4CvL[idx], store.uparTAK4CvB[idx]);
// }

// Vcb::Cat classify_from_scores(double hf, double bvc) {
//   if (!(hf >= 0.0 && hf <= 1.0 && bvc >= 0.0 && bvc <= 1.0))
//     return Vcb::Cat::N0;

//   const int ih = bin_hf(hf);
//   if (ih == 0)
//     return Vcb::Cat::L0;
//   if (ih == 1)
//     return Vcb::Cat::C0;
//   if (ih == 2)
//     return Vcb::Cat::C1;

//   static constexpr Vcb::Cat TOP_MAP[8] = {
//       Vcb::Cat::C4, Vcb::Cat::C3, Vcb::Cat::C2, Vcb::Cat::B0,
//       Vcb::Cat::B1, Vcb::Cat::B2, Vcb::Cat::B3, Vcb::Cat::B4};
//   const int jb = bin_bvc(bvc);
//   return TOP_MAP[jb];
// }

// Vcb::Cat classify_from_storage(const JetSoA &store, std::size_t idx) {
//   auto [hf, bvc] = HFvLF_BvC_from_storage(store, idx);
//   return classify_from_scores(hf, bvc);
// }

// void ComputeParTScores(const JetSoA &store, std::vector<float> &hfScores,
//                        std::vector<float> &bvcScores,
//                        std::vector<Vcb::Cat> &categories) {
//   const std::size_t n = store.size();
//   hfScores.resize(n);
//   bvcScores.resize(n);
//   categories.resize(n);
//   for (std::size_t i = 0; i < n; ++i) {
//     auto [hf, bvc] = HFvLF_BvC_from_storage(store, i);
//     hfScores[i] = static_cast<float>(hf);
//     bvcScores[i] = static_cast<float>(btivc);
//     categories[i] = classify_from_scores(hf, bvc);
//   }
// }

// void ComputeParTScores(const JetViewCollection &jets,
//                        std::vector<float> &hfScores,
//                        std::vector<float> &bvcScores,
//                        std::vector<Vcb::Cat> &categories) {
//   const auto &storagePtr = jets.storage();
//   if (!storagePtr) {
//     hfScores.clear();
//     bvcScores.clear();
//     categories.clear();
//     return;
//   }
//   ComputeParTScores(*storagePtr, hfScores, bvcScores, categories);
// }

// } // namespace
double Vcb::clip(double x, double lo, double hi) {
  return UParTScore::clip(x, lo, hi);
}

Vcb::Prob3 Vcb::sanitize_prob3(double pb, double pc, double pl, double eps) {
  return UParTScore::sanitize_prob3(pb, pc, pl, eps);
}

Vcb::Prob3 Vcb::compute_prob3_from_branches(double p_udg, double CvL,
                                            double CvB, double SvUDG,
                                            double eps) {
  return UParTScore::compute_prob3_from_branches(p_udg, CvL, CvB, SvUDG, eps);
}

std::pair<double, double> Vcb::hf_bvc_from_prob3(const Prob3 &p, double eps) {
  return UParTScore::hf_bvc_from_prob3(p, eps);
}

Vcb::Prob3 Vcb::MappedProb3_from_components(double probudg, double SvUDG,
                                            double CvL, double CvB, float pt,
                                            int hadronFalvor) const {
  UParTScore::Mapper mapper_central(CurrentOtLut(), IsDATA,
                                    HasFlag("Unmapped"));
  Prob3 p =
      mapper_central.MappedProb3(probudg, SvUDG, CvL, CvB, pt, hadronFalvor);

  if (!IsDATA && !HasFlag("Unmapped") && UParT_OT_SystActive) {
    UParTScore::Mapper mapper_syst(UParT_OT_SystActive, false, false);
    p = remap_prob3_with_second_lut(mapper_syst, p, pt, hadronFalvor);
  }
  return p;
}

Vcb::Vcb() {}

const OtJsonLutBank *Vcb::CurrentOtLut() const {
  return UParT_OT_Central.get();
}

std::string Vcb::BuildSystOtLutKey(const std::string &source,
                                   MyCorrection::variation variation) const {
  const std::string lowered = to_lower_ascii(source);

  if (starts_with(lowered, "bootstrap_")) {
    // Requested behavior: bootstrap down variation uses central LUT.
    if (variation == MyCorrection::variation::down)
      return "";
    if (variation != MyCorrection::variation::up)
      return "";
    return normalize_bootstrap_source_key(lowered);
  }

  const std::string stem = ot_source_to_integrated_stem(lowered);
  if (stem.empty())
    return "";

  switch (variation) {
  case MyCorrection::variation::up:
    return stem + "_Up";
  case MyCorrection::variation::down:
    return stem + "_Down";
  default:
    return "";
  }
}

const OtJsonLutBank *Vcb::GetOrLoadOtLut(const std::string &json_path,
                                         const std::string &bundle_key) {
  std::string cache_key = json_path;
  cache_key += "#";
  cache_key += bundle_key.empty() ? "central" : bundle_key;

  auto it = UParT_OT_ByPath.find(cache_key);
  if (it != UParT_OT_ByPath.end())
    return it->second.get();

  auto lut = std::make_unique<OtJsonLutBank>(ot_pt_edges());
  lut->load_json(json_path, bundle_key);
  const OtJsonLutBank *ptr = lut.get();
  UParT_OT_ByPath.emplace(cache_key, std::move(lut));
  return ptr;
}

void Vcb::UpdateActiveOtLutForCurrentSystematic() {
  UParT_OT_SystActive = nullptr;

  if (!UParT_OT_Central || IsDATA || HasFlag("Unmapped") || !systHelper)
    return;

  const std::string current_iter = systHelper->getCurrentSysName();
  const auto targets = systHelper->get_targets_from_name(current_iter);
  const bool uses_flavtag =
      std::find(targets.begin(), targets.end(), "flavtag") != targets.end();
  if (!uses_flavtag)
    return;

  const auto variation = systHelper->getCurrentIterVariation();
  if (variation == MyCorrection::variation::nom)
    return;

  const std::string source = systHelper->getCurrentIterSysSource();
  const bool bootstrap_down =
      variation == MyCorrection::variation::down && is_bootstrap_source(source);
  const std::string bundle_key = BuildSystOtLutKey(source, variation);
  if (bundle_key.empty()) {
    if (bootstrap_down)
      return;

    const std::string warn_key =
        "unknown_source:" + source + ":" + std::to_string(int(variation));
    if (UParT_OT_WarnedOnce.insert(warn_key).second) {
      std::cerr << "[Vcb::UpdateActiveOtLutForCurrentSystematic] Unknown OT "
                   "systematic source '"
                << source << "' for " << current_iter
                << ", fallback to central LUT" << std::endl;
    }
    return;
  }

  std::error_code ec;
  if (!std::filesystem::exists(UParT_OT_Central_Path, ec)) {
    const std::string warn_key = "missing_path:" + UParT_OT_Central_Path;
    if (UParT_OT_WarnedOnce.insert(warn_key).second) {
      std::cerr << "[Vcb::UpdateActiveOtLutForCurrentSystematic] Missing OT "
                   "LUT: "
                << UParT_OT_Central_Path << ", fallback to central LUT"
                << std::endl;
    }
    return;
  }

  try {
    UParT_OT_SystActive = GetOrLoadOtLut(UParT_OT_Central_Path, bundle_key);
  } catch (const std::exception &e) {
    const std::string warn_key = "load_fail:" + bundle_key;
    if (UParT_OT_WarnedOnce.insert(warn_key).second) {
      std::cerr << "[Vcb::UpdateActiveOtLutForCurrentSystematic] Failed to "
                   "load OT LUT "
                << bundle_key << " from " << UParT_OT_Central_Path << ": "
                << e.what() << ", fallback to central LUT" << std::endl;
    }
    UParT_OT_SystActive = nullptr;
  }
}

float Vcb::OtLutPtFromStore(const JetSoA &store, std::size_t idx) const {
  const std::string syst_target =
      systHelper ? systHelper->getCurrentIterSysTarget() : "Central";
  const auto variation = systHelper ? systHelper->getCurrentIterVariation()
                                    : MyCorrection::variation::nom;

  if (IsDATA) {
    return strict_ot_pt_value(store.correctedPt, idx, "correctedPt",
                              syst_target);
  }
  const float nominal_pt = strict_ot_pt_value(store.smearedPtNominal, idx,
                                              "smearedPtNominal", syst_target);

  if (syst_target.find("Jet_En") != std::string::npos) {
    switch (variation) {
    case MyCorrection::variation::up:
      return ot_pt_value_or_nominal(store.jesPtUp, idx, store.smearedPtNominal,
                                    syst_target);
    case MyCorrection::variation::down:
      return ot_pt_value_or_nominal(store.jesPtDown, idx,
                                    store.smearedPtNominal, syst_target);
    case MyCorrection::variation::nom:
      return nominal_pt;
    default:
      throw std::runtime_error(
          "[Vcb::OtLutPtFromStore] Unsupported variation for Jet_En target");
    }
  }

  if (syst_target == "Jet_Res") {
    switch (variation) {
    case MyCorrection::variation::up:
      return ot_pt_value_or_nominal(store.smearedPtUp, idx,
                                    store.smearedPtNominal, syst_target);
    case MyCorrection::variation::down:
      return ot_pt_value_or_nominal(store.smearedPtDown, idx,
                                    store.smearedPtNominal, syst_target);
    case MyCorrection::variation::nom:
      return nominal_pt;
    default:
      throw std::runtime_error(
          "[Vcb::OtLutPtFromStore] Unsupported variation for Jet_Res target");
    }
  }

  return nominal_pt;
}

int Vcb::bin_hf(double x) { return UParTScore::bin_hf(x); }

int Vcb::bin_bvc(double y) { return UParTScore::bin_bvc(y); }

// std::pair<double,double>
// Vcb::HFvLF_BvC_from_components(double probudg, double SvUDG, double CvL,
// double CvB,
//                                float pt, int hadronFalvor) const {
//   const double probs = (SvUDG > 0.0 && probudg > 0.0 && SvUDG < 1.0)
//                            ? SvUDG * probudg / (1.0 - SvUDG)
//                            : -1.0;

//   const double probc = (CvL > 0.0 && CvL < 1.0 && probs > 0.0 && probudg >
//   0.0)
//                            ? CvL * (probs + probudg) / (1.0 - CvL)
//                            : -1.0;

//   const double probbbblepb =
//       (CvB > 0.0 && probc > 0.0) ? (1.0 - CvB) * probc / CvB : -1.0;

//   const double BvC = (CvB > 0.0) ? (1.0 - CvB) : -1.0;

//   const double HFvLF =
//       (probbbblepb > 0.0 && probc > 0.0 && probs > 0.0 && probudg > 0.0)
//           ? (probbbblepb + probc) / (probbbblepb + probc + probs + probudg)
//           : -1.0;

//   if (IsDATA) return {HFvLF, BvC};
//   if (!UParT_OT_Central) return {HFvLF, BvC};

//   int pf = int(std::abs(hadronFalvor));
//   if (pf != 4 && pf != 5) pf = 0; // light for non c/b jets

//   auto [mapped_hfvlf, mapped_bvc] =
//       UParT_OT_Central->map(pt, pf, float(HFvLF), float(BvC));
//   if( HFvLF < 0.0 || BvC < 0.0 ) {
//     mapped_hfvlf = HFvLF;
//     mapped_bvc = BvC;
//   }

//   // 디버그 원하면 여기서 출력 (원래 storage에 있던 print 위치를 여기로 이동)
//   // std::cout << "Jet pt: " << pt << ", hadronFalvor: " << pf
//   //            << ", original HFvLF: " << HFvLF << ", original BvC: " << BvC
//   //            << " => mapped HFvLF: " << mapped_hfvlf << ", mapped BvC: "
//   << mapped_bvc << std::endl;

//   return {mapped_hfvlf, mapped_bvc};
// }

// Vcb.cc (클래스 멤버 함수로 추가 권장)
std::pair<double, double>
Vcb::HFvLF_BvC_from_components(double probudg, double SvUDG, double CvL,
                               double CvB, float pt, int hadronFalvor) const {
  const Prob3 p =
      MappedProb3_from_components(probudg, SvUDG, CvL, CvB, pt, hadronFalvor);
  return UParTScore::hf_bvc_from_prob3(p);
}

std::pair<double, double>
Vcb::HFvLF_BvC_from_ParT(const SelectedJetView &j) const {
  using Tagger = JetTagging::JetFlavTagger;
  using Score = JetTagging::JetFlavTaggerScoreType;

  return HFvLF_BvC_from_components(
      j.GetTaggerResult(Tagger::ParT, Score::probUDG),
      j.GetTaggerResult(Tagger::ParT, Score::SvUDG),
      j.GetTaggerResult(Tagger::ParT, Score::CvL),
      j.GetTaggerResult(Tagger::ParT, Score::CvB), j.Pt(), j.hadronFlavour());
}

std::pair<double, double> Vcb::HFvLF_BvC_from_storage(const JetSoA &store,
                                                      std::size_t idx) const {
  return HFvLF_BvC_from_components(
      store.uparTAK4UDG[idx], store.uparTAK4SvUDG[idx], store.uparTAK4CvL[idx],
      store.uparTAK4CvB[idx], OtLutPtFromStore(store, idx),
      int(store.hadronFlavour[idx]));
}

Vcb::Cat Vcb::classify_from_scores(double hf, double bvc) const {
  return UParTScore::classify_from_scores(hf, bvc);
}

Vcb::Cat Vcb::classify_from_storage(const JetSoA &store,
                                    std::size_t idx) const {
  auto [hf, bvc] = HFvLF_BvC_from_storage(store, idx);
  return classify_from_scores(hf, bvc);
}

void Vcb::initializeAnalyzer() {
  const bool baseline_only = HasFlag("BaselineOnly");
  if (!baseline_only)
    rle_bucket_compute_checksum();
  SetChannel();
  string SKNANO_HOME = std::getenv("SKNANO_HOME");
  string TABNET_TRAINING_DIR = "/data6/Users/yeonjoon/CMSSW_15_0_10/src/PhysicsTools/NanoTTH/data/tabnet/input_cat";
  if (!IsDATA && !baseline_only) {
    TString json_path = SKNANO_HOME + "/ModellingPatch/" + MCSample.Data() +
                        "_" + DataEra.Data() + "_summary.json";
    load_modelling_json(json_path);
  }
  const auto activeTagger = FlavTagger[DataEra.Data()];
  std::string btagging_eff_file = "btaggingEff.json";
  std::string ctagging_eff_file = "ctaggingEff.json";
  std::string btagging_R_file = "btaggingR.json";
  std::string ctagging_R_file = "ctaggingR.json";
  UParT_OT_Central = std::make_unique<OtJsonLutBank>(ot_pt_edges());
  UParT_OT_Central->load_json(UParT_OT_Central_Path);
  UParT_OT_SystActive = nullptr;
  if (channel == Channel::FH) {
    std::cout << "Initialize MyCorrection for FH" << std::endl;
    btagging_R_file = "Vcb_FH_btaggingR.json";
    ctagging_R_file = "Vcb_FH_ctaggingR.json";
    myCorr = new MyCorrection(
        DataEra, DataPeriod, IsDATA ? DataStream : MCSample, IsDATA,
        btagging_eff_file, ctagging_eff_file, btagging_R_file, ctagging_R_file);
    myCorr->SetTaggingParam(activeTagger, FH_BTag_WP);
    myMLHelper = std::make_unique<MLHelper>(
        "/data6/Users/yeonjoon/SKNanoAnalyzer/data/Run3_v12_Run2_v9/2022EE/"
        "spanet_FH_2022EE.onnx",
        MLHelper::ModelType::ONNX);
  } else if (channel == Channel::Mu || channel == Channel::El) {
    std::cout << "Initialize MyCorrection for SL" << std::endl;
    btagging_R_file = "Vcb_SL_btaggingR.json";
    ctagging_R_file = "Vcb_SL_ctaggingR.json";
    myCorr = new MyCorrection(
        DataEra, DataPeriod, IsDATA ? DataStream : MCSample, IsDATA,
        btagging_eff_file, ctagging_eff_file, btagging_R_file, ctagging_R_file);
    myCorr->SetTaggingParam(activeTagger, SL_BTag_WP);
    // myMLHelper =
    // std::make_unique<MLHelper>("/data6/Users/yeonjoon/SKNanoAnalyzer/data/spanet_run3.onnx",
    // MLHelper::ModelType::ONNX);
    if (!baseline_only) {
      myMLHelper_CLASSIF_folds.reserve(4);
      myMLHelper_RECO_folds.reserve(4);
      for (int i = 0; i < 4; ++i) {
        myMLHelper_CLASSIF_folds.push_back(std::make_unique<MLHelper>(
            "/data6/Users/yeonjoon/CMSSW_15_0_10/src/PhysicsTools/NanoTTH/data/"
            "spanet/input_cat/classif/spanet_fold" +
                std::to_string(i) + ".onnx",
            MLHelper::ModelType::ONNX)); // 생성자 인자 있을 경우
      }
      for (int i = 0; i < 4; ++i) {
        myMLHelper_RECO_folds.push_back(std::make_unique<MLHelper>(
            "/data6/Users/yeonjoon/CMSSW_15_0_10/src/PhysicsTools/NanoTTH/data/"
            "spanet/input_cat/reco/spanet_fold" +
                std::to_string(i) + ".onnx",
            MLHelper::ModelType::ONNX)); // 생성자 인자 있을 경우
      }
      for (int i = 0; i < 4; ++i) {
        myMLHelper_TabNet_folds.push_back(std::make_unique<MLHelper>(
            TABNET_TRAINING_DIR + "/tabnet_fold" + std::to_string(i) +
                ".onnx",
            MLHelper::ModelType::ONNX)); // 생성자 인자 있을 경우
      }
    }
  } else if (channel == Channel::MM || channel == Channel::ME ||
             channel == Channel::EE) {
    std::cout << "Initialize MyCorrection for DL" << std::endl;
    btagging_R_file = "Vcb_DL_btaggingR.json";
    ctagging_R_file = "Vcb_DL_ctaggingR.json";
    myCorr = new MyCorrection(
        DataEra, DataPeriod, IsDATA ? DataStream : MCSample, IsDATA,
        btagging_eff_file, ctagging_eff_file, btagging_R_file, ctagging_R_file);
    myCorr->SetTaggingParam(activeTagger, DL_BTag_WP);
  } else { // because FH doesn't need to specify the channel
    std::cout << "Initialize MyCorrection for FH" << std::endl;
    btagging_R_file = "Vcb_FH_btaggingR.json";
    ctagging_R_file = "Vcb_FH_ctaggingR.json";
    myCorr = new MyCorrection(
        DataEra, DataPeriod, IsDATA ? DataStream : MCSample, IsDATA,
        btagging_eff_file, ctagging_eff_file, btagging_R_file, ctagging_R_file);
    myCorr->SetTaggingParam(activeTagger, FH_BTag_WP);
    myMLHelper = std::make_unique<MLHelper>(
        "/data6/Users/yeonjoon/SKNanoAnalyzer/data/Run3_v12_Run2_v9/2022EE/"
        "spanet_FH_2022EE.onnx",
        MLHelper::ModelType::ONNX);
  }
  if (IsDATA || baseline_only) {
    systHelper = std::make_unique<SystematicHelper>(
        SKNANO_HOME + "/AnalyzerTools/noSyst.yaml",
        IsDATA ? DataStream : MCSample, DataEra);
  } else {

    systHelper = std::make_unique<SystematicHelper>(SKNANO_HOME +
                                                        "/AnalyzerTools/"
                                                        "VcbSystematic_OT.yaml",
                                                    MCSample, DataEra);
  }

  if (!baseline_only) {
    CreateTrainingTree();
    CreateTemplateTrainingTree();
  }
}

void Vcb::SetChannel() {
  if (HasFlag("Skim")) {
    std::cout << "Skimming mode detected, will iterate over channel"
              << std::endl;
  } else if (HasFlag("MM"))
    channel = Channel::MM;
  else if (HasFlag("ME"))
    channel = Channel::ME;
  else if (HasFlag("EE"))
    channel = Channel::EE;
  else if (HasFlag("Mu"))
    channel = Channel::Mu;
  else if (HasFlag("El"))
    channel = Channel::El;
  else if (HasFlag("FH"))
    channel = Channel::FH;
  else
    channel = Channel::FH;
}

// fold helper
uint32_t Vcb::rle_bucket(uint64_t run, uint64_t lumi, uint64_t event,
                         uint32_t nbuckets) {
  auto splitmix64 = [](uint64_t x) {
    uint64_t z = (x += 0x9e3779b97f4a7c15ULL);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
  };

  uint64_t seed = run * 0x9E3779B97F4A7C15ULL ^ lumi * 0xBF58476D1CE4E5B9ULL ^
                  event * 0x94D049BB133111EBULL;

  uint64_t h = splitmix64(seed);

  __uint128_t prod = (__uint128_t)h * (__uint128_t)nbuckets;
  return (uint32_t)(prod >> 64);
};

void Vcb::rle_bucket_compute_checksum() {
  constexpr uint64_t GOLDEN_CHECKSUM =
      0x4068d908153fc923; // precomputed golden checksum value
  constexpr uint64_t INIT_ACC = 0x1234567890abcdefULL;
  const std::size_t N_CHECKSUM_SAMPLE = 10'000'000;

  constexpr auto mix_checksum = [](uint64_t acc, uint32_t bucket) -> uint64_t {
    return acc * 0x9e3779b97f4a7c15ULL + bucket;
  };

  uint64_t acc = INIT_ACC;

  for (uint64_t i = 0; i < N_CHECKSUM_SAMPLE; ++i) {
    uint64_t run = i;
    uint64_t lumi = i * 0x9e3779b97f4a7c15ULL;
    uint64_t event = i ^ 0xDEADBEEFCAFEBABEULL;

    uint32_t nbuckets = static_cast<uint32_t>((i % 1'000'000ULL) + 1ULL);

    uint32_t b = rle_bucket(run, lumi, event, nbuckets);
    if (b >= nbuckets) {
      throw SKNano::LogicError("Out-of-range rle_bucket result");
    }

    acc = mix_checksum(acc, b);
  }
  if (acc != GOLDEN_CHECKSUM)
    throw std::runtime_error("rle_bucket checksum mismatch! Expected: " +
                             std::to_string(GOLDEN_CHECKSUM) +
                             ", Computed: " + std::to_string(acc));
  else
    std::cout << "rle_bucket checksum match! Checksum: " << std::hex << acc
              << std::dec << std::endl;
}

int Vcb::Unroller(const SelectedJetViewCollection &jets) {
  if (jets.size() < 4)
    return -1;

  JetTagging::JetFlavTagger tagger = FlavTagger[DataEra.Data()];
  const std::size_t invalidIndex = jets.size();
  std::size_t leadingIndices[4] = {invalidIndex, invalidIndex, invalidIndex,
                                   invalidIndex};
  float leadingScores[4] = {-1.f, -1.f, -1.f, -1.f};

  for (std::size_t idx = 0; idx < jets.size(); ++idx) {
    const float score = jets[idx].GetTaggerResult(
        tagger, JetTagging::JetFlavTaggerScoreType::B);
    for (int position = 0; position < 4; ++position) {
      if (score > leadingScores[position]) {
        for (int shift = 3; shift > position; --shift) {
          leadingScores[shift] = leadingScores[shift - 1];
          leadingIndices[shift] = leadingIndices[shift - 1];
        }
        leadingScores[position] = score;
        leadingIndices[position] = idx;
        break;
      }
    }
  }

  if (leadingIndices[2] == invalidIndex || leadingIndices[3] == invalidIndex)
    return -1;

  const auto third_leading_BvsC_jet = jets[leadingIndices[2]];
  const auto fourth_leading_BvsC_jet = jets[leadingIndices[3]];

  int third_leading_BvsC_jet_PassedBTaggingWP =
      GetPassedBTaggingWP(third_leading_BvsC_jet);
  int fourth_leading_BvsC_jet_PassedBTaggingWP =
      GetPassedBTaggingWP(fourth_leading_BvsC_jet);
  third_leading_BvsC_jet_PassedBTaggingWP =
      third_leading_BvsC_jet_PassedBTaggingWP == 4
          ? 3
          : third_leading_BvsC_jet_PassedBTaggingWP;
  fourth_leading_BvsC_jet_PassedBTaggingWP =
      fourth_leading_BvsC_jet_PassedBTaggingWP == 4
          ? 3
          : fourth_leading_BvsC_jet_PassedBTaggingWP;
  int unrolled = third_leading_BvsC_jet_PassedBTaggingWP +
                 fourth_leading_BvsC_jet_PassedBTaggingWP * 4;
  return unrolled;
}

int Vcb::Unroller(const SelectedJetView &jet1,
                  const SelectedJetView &jet2) {
  int jet1_Cat = static_cast<int>(JetCategory(jet1));
  int jet2_Cat = static_cast<int>(JetCategory(jet2));
  int unrolled = jet1_Cat + jet2_Cat * 12;
  return unrolled;
}

void Vcb::FillHistogramsAtThisPoint(std::string_view histPrefix, float weight) {
  const std::string base(histPrefix);
  std::string name;
  name.reserve(base.size() + 64);

  auto fill1d = [&](std::string_view suffix, float value, float w, int nbin,
                    float xmin, float xmax) {
    name.assign(base);
    name.push_back('/');
    name.append(suffix);
    Hists().Fill(name, value, w, nbin, xmin, xmax);
  };

  auto fill2d = [&](std::string_view suffix, float x, float y, float w,
                    int nbinx, float xmin, float xmax, int nbiny, float ymin,
                    float ymax) {
    name.assign(base);
    name.push_back('/');
    name.append(suffix);
    Hists().Fill(name, x, y, w, nbinx, xmin, xmax, nbiny, ymin, ymax);
  };

  // temp

  fill1d("MET", MET.Pt(), weight, 50, 0.f, 200.f);
  fill1d("HT", HT, weight, 100, 200.f, 2000.f);
  fill1d("n_jets", n_jets, weight, 12, 0.f, 12.f);
  fill1d("n_b_tagged_jets", n_b_tagged_jets, weight, 8, 0.f, 8.f);
  fill1d("n_c_tagged_jets", n_c_tagged_jets, weight, 8, 0.f, 8.f);
  fill1d("n_hadronFlav_b_jets", n_hadronFlav_b_jets, weight, 6, 2.f, 8.f);
  fill1d("n_hadronFlav_c_jets", n_hadronFlav_c_jets, weight, 8, 0.f, 8.f);
  fill2d("real_b_vs_tagged_b", n_hadronFlav_b_jets, n_b_tagged_jets, weight, 8,
         0.f, 8.f, 6, 2.f, 8.f);
  fill2d("real_c_vs_tagged_c", n_hadronFlav_c_jets, n_c_tagged_jets, weight, 8,
         0.f, 8.f, 6, 2.f, 8.f);
  fill1d("nPVsGood", ev.nPVsGood(), weight, 70, 0.f, 70.f);

  for (size_t i = 0; i < std::min(static_cast<size_t>(5), Jets.size()); ++i) {
    const auto jetCategory = static_cast<int>(JetCategory(Jets[i]));
    const std::string idx = std::to_string(i);

    name.assign(base);
    name.append("/Jet_Pt_").append(idx);
    Hists().Fill(name, Jets[i].Pt(), weight, 50, 0.f, 500.f);

    name.assign(base);
    name.append("/Jet_Eta_").append(idx);
    Hists().Fill(name, Jets[i].Eta(), weight, 50, -2.5, 2.5);

    name.assign(base);
    name.append("/Jet_Phi_").append(idx);
    Hists().Fill(name, Jets[i].Phi(), weight, 50, -3.14, 3.14);

    name.assign(base);
    name.append("/Jet_Category_").append(idx);
    Hists().Fill(name, jetCategory, weight, 12, 0.f, 12.f);

    name.assign(base);
    name.append("/Jet_HFvLF_").append(idx);
    Hists().Fill(name, JetHFvLFScore(Jets[i]), weight, 50, 0.f, 1.f);

    name.assign(base);
    name.append("/Jet_BvC_").append(idx);
    Hists().Fill(name, JetBvCScore(Jets[i]), weight, 50, 0.f, 1.f);

    name.assign(base);
    name.append("/Jet_newprobb_").append(idx);
    Hists().Fill(name, JetProbBScore(Jets[i]), weight, 50, 0.f, 1.f);

    name.assign(base);
    name.append("/Jet_newprobc_").append(idx);
    Hists().Fill(name, JetProbCScore(Jets[i]), weight, 50, 0.f, 1.f);

    name.assign(base);
    name.append("/Jet_newproblight_").append(idx);
    Hists().Fill(name, JetProbLScore(Jets[i]), weight, 50, 0.f, 1.f);

    name.assign(base);
    name.append("/Jet_ILRdim1_").append(idx);
    Hists().Fill(name, JetILRdim1Score(Jets[i]), weight, 50, -6.f, 6.f);

    name.assign(base);
    name.append("/Jet_ILRdim2_").append(idx);
    Hists().Fill(name, JetILRdim2Score(Jets[i]), weight, 50, -6.f, 6.f);
  }

  for (size_t i = 0; i < leptons.size(); i++) {
    const std::string idx = std::to_string(i);

    name.assign(base);
    name.append("/Lepton_Pt_").append(idx);
    Hists().Fill(name, leptons[i].Pt(), weight, 50, 0.f, 500.f);

    name.assign(base);
    name.append("/Lepton_Eta_").append(idx);
    Hists().Fill(name, leptons[i].Eta(), weight, 50, -2.5, 2.5);

    name.assign(base);
    name.append("/Lepton_Phi_").append(idx);
    Hists().Fill(name, leptons[i].Phi(), weight, 50, -3.14, 3.14);
  }

  if (leptons.size() == 2) {
    Particle ZCand = leptons[0] + leptons[1];
    fill1d("ZCand_Mass", ZCand.M(), weight, 50, 0.f, 200.f);
    fill1d("ZCand_Pt", ZCand.Pt(), weight, 50, 0.f, 200.f);
    fill1d("ZCand_Eta", ZCand.Eta(), weight, 50, -2.5, 2.5);
  }
}

void Vcb::FillTreeAtThisPoint(
    std::string_view treePrefix, float MCNormalizationWeight,
    const std::unordered_map<std::string, float> &weight_map) {}

void Vcb::UpdateAllJetTaggingCaches(const JetViewCollection &jets) {
  ComputeParTScores(jets, jetHFvLFAll, jetBvCAll, jetCategoryAll, nullptr);
}

void Vcb::UpdateAllJetTaggingCaches(
    const JetViewCollection &jets,
    const std::vector<std::size_t> &selected_indices) {
  ComputeParTScores(jets, jetHFvLFAll, jetBvCAll, jetCategoryAll,
                    &selected_indices);
}

void Vcb::ComputeParTScores(
    const JetViewCollection &jets, std::vector<float> &hfScores,
    std::vector<float> &bvcScores, std::vector<Vcb::Cat> &categories,
    const std::vector<std::size_t> *selected_indices) const {
  const auto &storagePtr = jets.storage();
  if (!storagePtr) {
    hfScores.clear();
    bvcScores.clear();
    categories.clear();

    // 캐시도 비우기
    jetProbBAll.clear();
    jetProbCAll.clear();
    jetProbLAll.clear();
    jetHFvLFAll.clear();
    jetBvCAll.clear();
    jetILRdim1All.clear();
    jetILRdim2All.clear();
    return;
  }

  const JetSoA &store = *storagePtr;
  const std::size_t n = store.size();

  if (!partInputBranchesValidated) {
    partInputBranchesValidated = true;
    std::vector<std::string> missing;
    auto check_branch = [&](const auto &column, const char *name) {
      const auto *branch = column.branch();
      if (!branch || !branch->valid())
        missing.emplace_back(name);
    };
    check_branch(store.uparTAK4UDG, "Jet_btagUParTAK4UDG");
    check_branch(store.uparTAK4SvUDG, "Jet_btagUParTAK4SvUDG");
    check_branch(store.uparTAK4CvL, "Jet_btagUParTAK4CvL");
    check_branch(store.uparTAK4CvB, "Jet_btagUParTAK4CvB");
    if (!IsDATA)
      check_branch(store.hadronFlavour, "Jet_hadronFlavour");

    if (!missing.empty()) {
      std::ostringstream oss;
      oss << "[Vcb::ComputeParTScores] Missing required branches for ParT "
             "category tagging: ";
      for (std::size_t i = 0; i < missing.size(); ++i) {
        if (i)
          oss << ", ";
        oss << missing[i];
      }
      throw std::runtime_error(oss.str());
    }
  }

  hfScores.assign(n, -1.f);
  bvcScores.assign(n, -1.f);
  categories.assign(n, Vcb::Cat::N0);

  // ✅ mapped prob3 캐시 (핵심)
  jetProbBAll.assign(n, -1.f);
  jetProbCAll.assign(n, -1.f);
  jetProbLAll.assign(n, -1.f);
  jetILRdim1All.assign(n, -999.f);
  jetILRdim2All.assign(n, -999.f);

  // (선택) 기존 캐시 유지한다면 같이 채움
  jetHFvLFAll.assign(n, -1.f);
  jetBvCAll.assign(n, -1.f);

  UParTScore::Mapper mapper_central(CurrentOtLut(), IsDATA,
                                    HasFlag("Unmapped"));
  std::unique_ptr<UParTScore::Mapper> mapper_syst = nullptr;
  if (!IsDATA && !HasFlag("Unmapped") && UParT_OT_SystActive) {
    mapper_syst =
        std::make_unique<UParTScore::Mapper>(UParT_OT_SystActive, false, false);
  }

  auto fill_one = [&](std::size_t i) {
    if (i >= n)
      return;
    const float lut_pt = OtLutPtFromStore(store, i);

    // 1) mapped (pb,pc,pl)
    Prob3 p = mapper_central.MappedProb3(
        double(store.uparTAK4UDG[i]), double(store.uparTAK4SvUDG[i]),
        double(store.uparTAK4CvL[i]), double(store.uparTAK4CvB[i]), lut_pt,
        int(store.hadronFlavour[i]));
    if (mapper_syst) {
      p = remap_prob3_with_second_lut(*mapper_syst, p, lut_pt,
                                      int(store.hadronFlavour[i]));
    }

    jetProbBAll[i] = float(p.pb);
    jetProbCAll[i] = float(p.pc);
    jetProbLAll[i] = float(p.pl);
    jetILRdim1All[i] = float(p.ilr_dim1);
    jetILRdim2All[i] = float(p.ilr_dim2);

    // 2) mapped hf/bvc from mapped prob3
    auto [hf, bvc] = UParTScore::hf_bvc_from_prob3(p);

    hfScores[i] = float(hf);
    bvcScores[i] = float(bvc);
    categories[i] = classify_from_scores(hf, bvc);

    jetHFvLFAll[i] = hfScores[i];
    jetBvCAll[i] = bvcScores[i];
  };

  if (selected_indices) {
    for (const std::size_t i : *selected_indices) {
      fill_one(i);
    }
    return;
  }

  for (std::size_t i = 0; i < n; ++i) {
    fill_one(i);
  }
}

Vcb::Cat Vcb::JetCategory(const SelectedJetView &jet) const {
  const int originalIdx = jet.OriginalIndex();
  if (originalIdx < 0 ||
      static_cast<std::size_t>(originalIdx) >= jetCategoryAll.size())
    return Vcb::Cat::N0;
  return jetCategoryAll[static_cast<std::size_t>(originalIdx)];
}

float Vcb::JetHFvLFScore(const SelectedJetView &jet) const {
  const int originalIdx = jet.OriginalIndex();
  if (originalIdx < 0 ||
      static_cast<std::size_t>(originalIdx) >= jetHFvLFAll.size())
    return -1.f;
  return jetHFvLFAll[static_cast<std::size_t>(originalIdx)];
}

float Vcb::JetBvCScore(const SelectedJetView &jet) const {
  const int originalIdx = jet.OriginalIndex();
  if (originalIdx < 0 ||
      static_cast<std::size_t>(originalIdx) >= jetBvCAll.size())
    return -1.f;
  return jetBvCAll[static_cast<std::size_t>(originalIdx)];
}

float Vcb::JetProbBScore(const SelectedJetView &jet) const {
  const int originalIdx = jet.OriginalIndex();
  if (originalIdx < 0 ||
      static_cast<std::size_t>(originalIdx) >= jetProbBAll.size())
    return -1.f;
  return jetProbBAll[static_cast<std::size_t>(originalIdx)];
}

float Vcb::JetProbCScore(const SelectedJetView &jet) const {
  const int originalIdx = jet.OriginalIndex();
  if (originalIdx < 0 ||
      static_cast<std::size_t>(originalIdx) >= jetProbCAll.size())
    return -1.f;
  return jetProbCAll[static_cast<std::size_t>(originalIdx)];
}

float Vcb::JetProbLScore(const SelectedJetView &jet) const {
  const int originalIdx = jet.OriginalIndex();
  if (originalIdx < 0 ||
      static_cast<std::size_t>(originalIdx) >= jetProbLAll.size())
    return -1.f;
  return jetProbLAll[static_cast<std::size_t>(originalIdx)];
}

float Vcb::JetILRdim1Score(const SelectedJetView &jet) const {
  const int originalIdx = jet.OriginalIndex();
  if (originalIdx < 0 ||
      static_cast<std::size_t>(originalIdx) >= jetILRdim1All.size())
    return -999.f;
  return jetILRdim1All[static_cast<std::size_t>(originalIdx)];
}

float Vcb::JetILRdim2Score(const SelectedJetView &jet) const {
  const int originalIdx = jet.OriginalIndex();
  if (originalIdx < 0 ||
      static_cast<std::size_t>(originalIdx) >= jetILRdim2All.size())
    return -999.f;
  return jetILRdim2All[static_cast<std::size_t>(originalIdx)];
}

short Vcb::GetPassedBTaggingWP(const SelectedJetView &jet) {
  const Cat category = JetCategory(jet);
  switch (category) {
  case Vcb::Cat::B4:
    return 4;
  case Vcb::Cat::B3:
    return 3;
  case Vcb::Cat::B2:
    return 2;
  case Vcb::Cat::B1:
    return 1;
  case Vcb::Cat::B0:
    return 0;
  default:
    return -1;
  }
}

short Vcb::GetPassedCTaggingWP(const SelectedJetView &jet) {
  const Cat category = JetCategory(jet);
  switch (category) {
  case Vcb::Cat::C4:
    return 4;
  case Vcb::Cat::C3:
    return 3;
  case Vcb::Cat::C2:
    return 2;
  case Vcb::Cat::C1:
    return 1;
  case Vcb::Cat::C0:
    return 0;
  default:
    return -1;
  }
}

void Vcb::executeEvent() {
  // Header-only rejection must precede object/Gen/JEC/JER materialisation.
  ev = GetEvent();
  if (!myCorr->IsGoldenLumi(RunNumber, luminosityBlock)) {
    return;
  }

  {
    auto phase = MeasurePerformancePhase("object_view");
    AllMuonViews = GetAllMuonViews();
    AllElectronViews = GetAllElectronViews();
    AllJetViews = GetAllJetViews();
    AllGenViews = GetAllGenViews();
    AllGens = AllGenViews;
    AllGenJets = GetAllGenJetViews();
  }

  if (HasFlag("Skim")) {
    Clear();
    SkimTree();
    return;
  }
  if (HasFlag("TemplateTraining")) {
    Clear();
    for (const auto &syst_dummy : *systHelper) {
      UpdateActiveOtLutForCurrentSystematic();
      bool passed = false;
      {
        auto phase = MeasurePerformancePhase("selection");
        passed = PassBaseLineSelection(false, false);
      }
      if (!passed)
        continue;
      {
        auto phase = MeasurePerformancePhase("onnx");
        InferONNX();
      }
      if (systHelper->getCurrentIterSysTarget().find("Central") !=
          std::string::npos) {
        const auto weight_map = systHelper->calculateWeight(false);
        FillTemplateTrainingTree(weight_map);
        return;
      }
    }
    return;
  }

  if (HasFlag("Training")) {
    Clear();
    for (const auto &syst_dummy : *systHelper) {
      UpdateActiveOtLutForCurrentSystematic();
      bool passed = false;
      {
        auto phase = MeasurePerformancePhase("selection");
        passed = PassBaseLineSelection(false, true);
      }
      if (!passed)
        continue;
      if (systHelper->getCurrentIterSysTarget().find("Central") !=
          std::string::npos) {
        FillTrainingTree();
        return;
      }
    }
    return;
  }
  if (!CheckChannel()) {
    throw std::runtime_error("Invalid channel flag for this analyzer");
  }
  {
    auto phase = MeasurePerformancePhase("systematic");
    for (const auto &syst_dummy : *systHelper) {
      leptons.clear();
      executeEventFromParameter();
      if (HasFlag("spurious"))
        break;
    }
  }
}

void Vcb::SetSystematicLambda(bool remove_flavtagging_sf) {
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

  std::function<float()> top_pt_reweight_lambda = [this, get_subproc_name]() {
    if (!MCSample.Contains("TT"))
      return 1.f;

    auto [firstTopIdx, firstAntiTopIdx, lastTopIdx, lastAntiTopIdx] =
        GetTopAndAntiTopIndices(AllGenViews);

    const TLorentzVector top = AllGenViews[firstTopIdx].P4();
    const TLorentzVector antiTop = AllGenViews[firstAntiTopIdx].P4();

    const float w_toppt = myCorr->GetTopPtReweight(top, antiTop);
    const float patch = modelling_patches[get_subproc_name()].patch_minnlo;

    return w_toppt / patch;
  };

  std::function<float(MyCorrection::variation, TString)> hDamp_lambda =
      [this, get_subproc_name](MyCorrection::variation syst,
                               TString /*source*/) {
        if (!MCSample.Contains("TT"))
          return 1.f;
        const std::string subproc_name = get_subproc_name();
        auto [firstTopIdx, firstAntiTopIdx, lastTopIdx, lastAntiTopIdx] =
            GetTopAndAntiTopIndices(AllGenViews);
        const TLorentzVector FirstCopyTop = AllGenViews[firstTopIdx].P4();
        const TLorentzVector FirstCopyAntiTop =
            AllGenViews[firstAntiTopIdx].P4();
        switch (syst) {
        case MyCorrection::variation::up:
          return myCorr->GethDampReweight(FirstCopyTop, FirstCopyAntiTop,
                                          syst) /
                 modelling_patches[subproc_name].patch_hdamp_up;
        case MyCorrection::variation::down:
          return myCorr->GethDampReweight(FirstCopyTop, FirstCopyAntiTop,
                                          syst) /
                 modelling_patches[subproc_name].patch_hdamp_down;
        default:
          return 1.f;
        }
      };

  std::function<float(MyCorrection::variation, TString)> bFrag_lambda =
      [&](MyCorrection::variation syst, TString source) {
        if (!MCSample.Contains("TT"))
          return 1.f;
        auto [topIdx, WTopIdx, BHadTopIdx, antiTopIdx, WAntiTopIdx,
              BHadAntiTopIdx] =
            myCorr->GetGenIdxofTopDecayProducts(AllGenViews);
        auto LastCopyTop = AllGenViews[topIdx].P4();
        auto LastCopyAntiTop = AllGenViews[antiTopIdx].P4();
        auto LastCopyWPlus = AllGenViews[WTopIdx].P4();
        auto LastCopyWMinus = AllGenViews[WAntiTopIdx].P4();
        auto FirstCopyAntiTopBHad = AllGenViews[BHadTopIdx].P4();
        auto FirstCopyTopBHad = AllGenViews[BHadAntiTopIdx].P4();
        return myCorr->GetBFragReweight(
            LastCopyTop, LastCopyAntiTop, LastCopyWPlus, LastCopyWMinus,
            FirstCopyTopBHad, FirstCopyAntiTopBHad, syst);
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
  weight_function_map["Top_Pt_Reweight"] = dummy_oneside_lambda;
  weight_function_map["hdamp"] = hDamp_lambda;
  weight_function_map["BFrag"] = dummy_lambda;

  if (remove_flavtagging_sf)
    weight_function_map["flavtag"] = dummy_lambda;
  else
    weight_function_map["flavtag"] = dummy_lambda;
  systHelper->assignWeightFunctionMap(weight_function_map);
}

array<size_t, 4> Vcb::GetTopAndAntiTopIndices(const GenViewCollection &gens) {
  constexpr size_t npos = std::numeric_limits<size_t>::max();

  size_t FirstCopyTopIndex = npos;
  size_t FirstCopyAntiTopIndex = npos;
  size_t LastCopyTopIndex = npos;
  size_t LastCopyAntiTopIndex = npos;

  const size_t n = gens.size();

  constexpr unsigned long FIRST_COPY_BIT = 1UL << 12;
  constexpr unsigned long LAST_COPY_BIT = 1UL << 13;

  for (size_t idx = 0; idx < n; ++idx) {
    const GenView &gen = gens[idx];

    const int pdg = gen.PdgId();
    const auto flags = gen.StatusFlags();

    const bool isFirstCopy = (flags & FIRST_COPY_BIT) != 0;
    const bool isLastCopy = (flags & LAST_COPY_BIT) != 0;

    if (pdg == 6) { // top
      if (isFirstCopy) {
        if (FirstCopyTopIndex != npos)
          throw SKNano::EventDataError("Multiple first-copy tops found in event");
        FirstCopyTopIndex = idx;
      }
      if (isLastCopy) {
        if (LastCopyTopIndex != npos)
          throw SKNano::EventDataError("Multiple last-copy tops found in event");
        LastCopyTopIndex = idx;
      }
    } else if (pdg == -6) { // anti-top
      if (isFirstCopy) {
        if (FirstCopyAntiTopIndex != npos)
          throw SKNano::EventDataError("Multiple first-copy antitops found in event");
        FirstCopyAntiTopIndex = idx;
      }
      if (isLastCopy) {
        if (LastCopyAntiTopIndex != npos)
          throw SKNano::EventDataError("Multiple last-copy antitops found in event");
        LastCopyAntiTopIndex = idx;
      }
    }
  }

  if (FirstCopyTopIndex == npos)
    throw SKNano::EventDataError("No first-copy top found in event");
  if (FirstCopyAntiTopIndex == npos)
    throw SKNano::EventDataError("No first-copy antitop found in event");
  if (LastCopyTopIndex == npos)
    throw SKNano::EventDataError("No last-copy top found in event");
  if (LastCopyAntiTopIndex == npos)
    throw SKNano::EventDataError("No last-copy antitop found in event");

  return {FirstCopyTopIndex, FirstCopyAntiTopIndex, LastCopyTopIndex,
          LastCopyAntiTopIndex};
}

void Vcb::Clear() {
  HT = 0;
  n_jets = 0;
  n_b_tagged_jets = 0;
  n_c_tagged_jets = 0;
  n_loose_b_tagged_jets = 0;
  n_loose_c_tagged_jets = 0;
  n_hf_jets = 0;
  n_loose_hf_jets = 0;
  n_hadronFlav_b_jets = 0;
  n_hadronFlav_c_jets = 0;
  find_all_jets = false;
  leptons.clear();
  Jets = SelectedJetViewCollection();
  MET = Particle();
  ttbj = false;
  ttbb = false;
  ttcc = false;
  ttLF = false;
}

void Vcb::executeEventFromParameter() {
  Clear();
  UpdateActiveOtLutForCurrentSystematic();
  enum class OUTPUT_TYPE { HISTOGRAMS, TREE };
  OUTPUT_TYPE output_type = OUTPUT_TYPE::HISTOGRAMS;
  if (HasFlag("OutputTrees"))
    output_type = OUTPUT_TYPE::TREE;

  bool passed = false;
  {
    auto phase = MeasurePerformancePhase("selection");
    passed = PassBaseLineSelection();
  }
  if (!passed)
    return;

  const std::string channel_str = GetChannelString(channel).Data();
  // Inclusive만 사용
  const std::string base_path = channel_str + "/Inclusive/";
  const std::string current_iter_name =
      systHelper ? systHelper->getCurrentSysName() : "Central";
  const std::string current_iter_prefix = base_path + current_iter_name + "/";

  // 샘플 후미(postfix) 결정
  std::string sample_postfix = Sample_Shorthand[MCSample.Data()];
  if (MCSample.Contains("TT") && !MCSample.Contains("Vcb")) {
    sample_postfix += GetTTHFPostFix(); // TTbb/TTcc/… 후미
  }

  // Fast path used by the Python front-end: selection, corrections and the
  // event loop stay in C++, while ML inference and systematic lanes are
  // intentionally omitted.  Qualifying the base implementation also avoids
  // the Vcb_SL Wcb-NN histogram extension.
  if (HasFlag("BaselineOnly")) {
    auto phase = MeasurePerformancePhase("histogram_tree");
    const std::string output_path =
        base_path + "Central/" + (IsDATA ? "data_obs" : sample_postfix);
    Vcb::FillHistogramsAtThisPoint(output_path,
                                   IsDATA ? 1.f : MCNormalization());
    return;
  }

  if (output_type == OUTPUT_TYPE::HISTOGRAMS) {
    auto phase = MeasurePerformancePhase("onnx");
    InferONNX();
    InferTabNet();
  }

  // -------------------------
  // DATA: Inclusive/Central/data_obs 만 채움
  // -------------------------
  if (IsDATA) {
    auto phase = MeasurePerformancePhase("histogram_tree");
    if (output_type == OUTPUT_TYPE::HISTOGRAMS) {
      FillHistogramsAtThisPoint(base_path + "Central/data_obs", 1.f);
      FillONNXRecoInfo(base_path + "Central/data_obs", 1.f);
      FillTabNetInfo(base_path + "Central/data_obs", 1.f);
    } else {
      const std::unordered_map<std::string, float> data_weights = {
          {"Central", 1.f}};
      FillTreeAtThisPoint(current_iter_prefix + "data_obs", 1.f, data_weights);
    }
    return;
  }

  // Histogram creation is lazy and idempotent in FillHist.  Do not use a
  // zero-weight Fill as a booking mechanism: ROOT still increments fEntries.
  // Actual fills below are therefore the only event-level histogram updates.
  const float normalization = MCNormalization();
  const auto weight_map = systHelper->calculateWeight(false);
  auto phase = MeasurePerformancePhase("histogram_tree");
  if (output_type == OUTPUT_TYPE::TREE) {
    FillTreeAtThisPoint(current_iter_prefix + Sample_Shorthand[MCSample.Data()],
                        normalization, weight_map);
  } else {
    for (const auto &kv : weight_map) {
      const std::string &syst = kv.first;
      const float w = kv.second;
      FillHistogramsAtThisPoint(base_path + syst + "/" + sample_postfix,
                                w * normalization);
       FillONNXRecoInfo(base_path + syst + "/" + sample_postfix,
       w * normalization);
       FillTabNetInfo(base_path + syst + "/" + sample_postfix,
       w * normalization);
    }
  }
}

void Vcb::SetTTbarId() {
  if (!IsDATA) {
    ttbj = (genTtbarId % 100 == 51 || genTtbarId % 100 == 52);
    ttbb = (genTtbarId % 100 >= 53 && genTtbarId % 100 <= 55);
    ttcc = (genTtbarId % 100 >= 41 && genTtbarId % 100 <= 45);
    ttLF = !(ttbj || ttbb || ttcc);
  }
}

void Vcb::CreateTrainingTree() {}
void Vcb::CreateTemplateTrainingTree() {}

void Vcb::FillTrainingTree() {}
void Vcb::FillTemplateTrainingTree(
    const std::unordered_map<std::string, float> &weight_map) {}

RVec<int> Vcb::FindTTbarJetIndices() {
  RVec<int> iamnothing;
  return iamnothing;
}

void Vcb::FillKinematicFitterResult(const TString &histPrefix, float weight) {}

RVec<RVec<unsigned int>>
Vcb::GetPermutations(const SelectedJetViewCollection &jets) {
  RVec<RVec<unsigned int>> iamnothing;
  return iamnothing;
}

void Vcb::SkimTree() {
  Clear();
  if (!skimTreeInitialized) {
    skimTreeInitialized = true;
  }

  RVec<Channel> all_channels = {Channel::MM, Channel::ME, Channel::EE,
                                Channel::Mu, Channel::El, Channel::FH};
  for (const auto &ch : all_channels) {
    this->channel = ch;
    if (!CheckChannel())
      continue;
    for (const auto &syst_dummy : *systHelper) {
      Clear();
      leptons.clear();
      UpdateActiveOtLutForCurrentSystematic();
      if (PassBaseLineSelection(true, true)) {
        // Record global entry; actual copy is deferred to WriteHist.
        skim_passed_global_entries.push_back(currentEntry);
        return; // Exit immediately upon success
      }
    }
  }

  // If no channel passed for all systematic variations
  return;
}

void Vcb::WriteHist() {
  if (HasFlag("Skim")) {
    if (!skim_passed_global_entries.empty() && fChain) {

      // 1) 글로벌 엔트리 정렬 + 중복 제거 (순차 I/O 위해)
      std::sort(skim_passed_global_entries.begin(),
                skim_passed_global_entries.end());
      skim_passed_global_entries.erase(
          std::unique(skim_passed_global_entries.begin(),
                      skim_passed_global_entries.end()),
          skim_passed_global_entries.end());

      // 2) 모든 브랜치 활성화 + ROOT가 버퍼 관리
      fChain->SetBranchStatus("*", 1);
      fChain->ResetBranchAddresses();

      // 3) TEntryList를 이용한 CopyTree (ROOT 내부 최적화 활용)
      TEntryList elist("skim_list", "Selected entries");
      for (Long64_t entry : skim_passed_global_entries) {
        elist.Enter(entry, fChain);
      }
      fChain->SetEntryList(&elist);

      if (TTree *curTree = fChain->GetTree()) {
        configureTreeCache(curTree);
      }

      TTree *skimTree = fChain->CopyTree("");
      if (skimTree) {
        skimTree->SetName("Events");
        treemap["Events"] = skimTree;
      }

      fChain->SetEntryList(0);
      skim_passed_global_entries.clear();
    }
  }

  AnalyzerCore::WriteHist();
}

float Vcb::LeptonTriggerWeight(bool isEle, const MyCorrection::variation syst,
                               const TString & /*source*/) {
  MyCorrection::variation electronVariation = MyCorrection::variation::nom;
  MyCorrection::variation muonVariation = MyCorrection::variation::nom;

  if (syst != MyCorrection::variation::nom) {
    if (isEle)
      electronVariation = syst;
    else
      muonVariation = syst;
  }

  switch (channel) {
  case Channel::FH:
    return 1.f;
  case Channel::Mu:
    if (isEle)
      return 1.f;
    return myCorr->GetMuonTriggerSF(Mu_Trigger_SF_Key[DataEra.Data()], Muons[0],
                                    muonVariation);
  case Channel::El:
    if (!isEle)
      return 1.f;
    return myCorr->GetElectronTriggerSF(El_Trigger_SF_Key[DataEra.Data()],
                                        lepton.Eta(), lepton.Pt(), lepton.Phi(),
                                        electronVariation);
  case Channel::MM:
    if (isEle)
      return 1.f;
    {
      double num = 1.f;
      double den = 1.f;
      for (const auto &mu : Muons) {
        num *= (1.f - 0.93 * myCorr->GetMuonTriggerSF(
                                 Mu_Trigger_SF_Key[DataEra.Data()], mu,
                                 muonVariation));
        den *= (1.f - 0.93); // hardcoded: let MCEff = 1, DataEff = SF, MUON POG
                             // PLEASE GIVE ME EFFICIENCY
      }
      return (1. - num) / (1. - den);
    }
  case Channel::ME: {
    double num = 1.f;
    double den = 1.f;
    num *= (1.f -
            0.93 * myCorr->GetMuonTriggerSF(Mu_Trigger_SF_Key[DataEra.Data()],
                                            Muons[0], muonVariation));
    den *= (1.f - 0.93); // hardcoded: let MCEff = 1, DataEff = SF, MUON POG
                         // PLEASE GIVE ME EFFICIENCY
    num *=
        (1.f - myCorr->GetElectronTriggerDataEff(
                   El_Trigger_SF_Key[DataEra.Data()], Electrons[0].Eta(),
                   Electrons[0].Pt(), Electrons[0].Phi(), electronVariation));
    den *=
        (1.f - myCorr->GetElectronTriggerMCEff(
                   El_Trigger_SF_Key[DataEra.Data()], Electrons[0].Eta(),
                   Electrons[0].Pt(), Electrons[0].Phi(), electronVariation));
    return (1. - num) / (1. - den);
  }
  case Channel::EE:
    if (!isEle)
      return 1.f;
    {
      double num = 1.f;
      double den = 1.f;
      for (const auto &el : Electrons) {
        num *= (1.f - myCorr->GetElectronTriggerDataEff(
                          El_Trigger_SF_Key[DataEra.Data()], el.Eta(), el.Pt(),
                          el.Phi(), electronVariation));
        den *= (1.f - myCorr->GetElectronTriggerMCEff(
                          El_Trigger_SF_Key[DataEra.Data()], el.Eta(), el.Pt(),
                          el.Phi(), electronVariation));
      }
      return (1. - num) / (1. - den);
    }
  default:
    throw std::runtime_error("[Vcb::LeptonTriggerWeight] Invalid channel");
  }
}

void Vcb::InferONNX() {}
void Vcb::InferTabNet() {}

bool Vcb::FillONNXRecoInfo(const TString &histPrefix, float weight) {
  return true;
}

bool Vcb::FillTabNetInfo(const TString &histPrefix, float weight) {
  return true;
}
