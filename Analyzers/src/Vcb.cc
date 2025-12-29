#include "Vcb.h"
#include "BranchManager.h"
#include "GenView.h"
#include "Muon.h"
#include "OtJsonLutBank.h"
#include "TBranch.h"
#include "TEntryList.h"
#include "TEntryListArray.h"
#include "TObjArray.h"
#include <TLorentzVector.h>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <unordered_map>
#include <unordered_set>

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
  return std::min(hi, std::max(lo, x));
}

Vcb::Prob3 Vcb::sanitize_prob3(double pb, double pc, double pl, double eps) {
  pb = std::max(pb, eps);
  pc = std::max(pc, eps);
  pl = std::max(pl, eps);

  const double s = pb + pc + pl;

  Vcb::Prob3 out;
  if (!(std::isfinite(s) && s > 0.0)) {
    out.pb = out.pc = out.pl = 1.0 / 3.0;
    out.ok = false;
    return out;
  }

  out.pb = pb / s;
  out.pc = pc / s;
  out.pl = pl / s;
  out.ok =
      std::isfinite(out.pb) && std::isfinite(out.pc) && std::isfinite(out.pl);
  return out;
}

Vcb::Prob3 Vcb::compute_prob3_from_branches(double p_udg, double CvL,
                                            double CvB, double SvUDG,
                                            double eps) {
  CvL = clip(CvL, eps, 1.0 - eps);
  CvB = clip(CvB, eps, 1.0 - eps);
  SvUDG = clip(SvUDG, eps, 1.0 - eps);
  p_udg = clip(p_udg, eps, 1.0 - eps);

  const double p_udsg = p_udg / (1.0 - SvUDG + eps);
  const double p_c = CvL * p_udsg / (1.0 - CvL + eps);
  const double p_b = (1.0 - CvB) * p_c / (CvB + eps);

  return sanitize_prob3(std::isfinite(p_b) ? p_b : eps,
                        std::isfinite(p_c) ? p_c : eps,
                        std::isfinite(p_udsg) ? p_udsg : eps, eps);
}

std::pair<double, double> Vcb::hf_bvc_from_prob3(const Prob3 &p, double eps) {
  const double hf = clip(p.pb + p.pc, 0.0, 1.0);
  const double den = std::max(hf, eps);
  const double bvc = clip(p.pb / den, 0.0, 1.0);
  return {hf, bvc};
}


Vcb::Prob3 Vcb::MappedProb3_from_components(double probudg, double SvUDG,
                                            double CvL, double CvB, float pt,
                                            int hadronFalvor) const {
  constexpr double eps = 1e-12;

  Prob3 p = compute_prob3_from_branches(probudg, CvL, CvB, SvUDG, eps);
    

  int pf = int(std::abs(hadronFalvor));
  if (pf != 4 && pf != 5)
    pf = 0;

  auto helmert_submatrix = [](int K) -> std::vector<double> {
    if (K < 2)
      throw std::invalid_argument("K<2");
    std::vector<double> H((size_t)K * (K - 1), 0.0);
    for (int j = 1; j < K; ++j) {
      double d = std::sqrt(double(j) * (j + 1.0));
      for (int i = 0; i < j; ++i)
        H[(size_t)i * (K - 1) + (j - 1)] = 1.0 / d;
      H[(size_t)j * (K - 1) + (j - 1)] = -double(j) / d;
    }
    return H; // row-major: H[i*(K-1)+c]
  };

  auto ilr = [&](const std::vector<double> &P, int ref_idx = -1,
                 double eps = 1e-12) -> std::vector<double> {
    int K = (int)P.size();
    if (K < 2)
      throw std::invalid_argument("K<2");
    if (ref_idx < 0)
      ref_idx += K;
    if (ref_idx < 0 || ref_idx >= K)
      throw std::out_of_range("ref_idx");

    std::vector<int> perm;
    perm.reserve(K);
    for (int j = 0; j < K; ++j)
      if (j != ref_idx)
        perm.push_back(j);
    perm.push_back(ref_idx);

    std::vector<double> Pp(K), logP(K), clr(K);
    for (int j = 0; j < K; ++j) {
      double v = P[perm[j]];
      Pp[j] = std::max(v, eps);
    }
    double s = std::accumulate(Pp.begin(), Pp.end(), 0.0);
    if (!(s > 0.0))
      throw std::runtime_error("sum");
    for (int j = 0; j < K; ++j)
      Pp[j] /= s;

    double m = 0.0;
    for (int j = 0; j < K; ++j) {
      logP[j] = std::log(Pp[j]);
      m += logP[j];
    }
    m /= double(K);
    for (int j = 0; j < K; ++j)
      clr[j] = logP[j] - m;

    auto H = helmert_submatrix(K);
    std::vector<double> Z(K - 1, 0.0);
    for (int c = 0; c < K - 1; ++c) {
      double v = 0.0;
      for (int i = 0; i < K; ++i)
        v += clr[i] * H[(size_t)i * (K - 1) + c];
      Z[c] = v;
    }
    return Z;
  };


  auto iilr = [&](const std::vector<double>& Z, int ref_idx = -1) -> std::vector<double> {
  int Km1 = (int)Z.size(), K = Km1 + 1; if (K < 2) throw std::invalid_argument("K<2");
  if (ref_idx < 0) ref_idx += K; if (ref_idx < 0 || ref_idx >= K) throw std::out_of_range("ref_idx");

  std::vector<int> perm; perm.reserve(K);
  for (int j = 0; j < K; ++j) if (j != ref_idx) perm.push_back(j); perm.push_back(ref_idx);
  std::vector<int> inv_perm(K, -1); for (int j = 0; j < K; ++j) inv_perm[perm[j]] = j;

  auto H = helmert_submatrix(K);
  std::vector<double> clr(K, 0.0), Pp(K), P(K);
  for (int i = 0; i < K; ++i) {
    double v = 0.0;
    for (int c = 0; c < K - 1; ++c) v += Z[c] * H[(size_t)i * (K - 1) + c];
    clr[i] = v;
  }

  double s = 0.0;
  for (int i = 0; i < K; ++i) { Pp[i] = std::exp(clr[i]); s += Pp[i]; }
  if (!(s > 0.0)) throw std::runtime_error("sum");
  for (int i = 0; i < K; ++i) Pp[i] /= s;

  for (int orig = 0; orig < K; ++orig) P[orig] = Pp[inv_perm[orig]];
  return P;
};

  auto ilr_vec = ilr({p.pb, p.pc, p.pl});
  // LUT: (ilr1, ilr2) -> (mapped_ilr1, mapped_ilr2)
  auto [mapped_ilr1, mapped_ilr2] =
      UParT_OT_Central->map_ilr_z(pt, pf, float(ilr_vec[0]), float(ilr_vec[1]));

  auto mapped_vec = iilr({mapped_ilr1, mapped_ilr2});

  
  if(IsDATA){
    Prob3 unmapped_prob3 = sanitize_prob3(p.pb, p.pc, p.pl, eps);
    unmapped_prob3.ilr_dim1 = ilr_vec[0];
    unmapped_prob3.ilr_dim2 = ilr_vec[1];
    return unmapped_prob3;
  }
  else{
  Prob3 mapped_prob3 = sanitize_prob3(mapped_vec[0], mapped_vec[1], mapped_vec[2], eps);
  mapped_prob3.ilr_dim1 = mapped_ilr1;
  mapped_prob3.ilr_dim2 = mapped_ilr2;
  return mapped_prob3;
  }

}

Vcb::Vcb() {}

int Vcb::bin_hf(double x) {
  return int(x >= HF_T1) + int(x >= HF_T2) + int(x >= HF_T3);
}

int Vcb::bin_bvc(double y) {
  return int(y >= BVC_T1) + int(y >= BVC_T2) + int(y >= BVC_T3) +
         int(y >= BVC_T4) + int(y >= BVC_T5) + int(y >= BVC_T6) +
         int(y >= BVC_T7);
}

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
  // -----------------------------
  // (Legacy) HFvLF/BvC 계산 (그대로 유지)
  // -----------------------------
  const double probs = (SvUDG > 0.0 && probudg > 0.0 && SvUDG < 1.0)
                           ? SvUDG * probudg / (1.0 - SvUDG)
                           : -1.0;

  const double probc_legacy =
      (CvL > 0.0 && CvL < 1.0 && probs > 0.0 && probudg > 0.0)
          ? CvL * (probs + probudg) / (1.0 - CvL)
          : -1.0;

  const double probbbblepb = (CvB > 0.0 && probc_legacy > 0.0)
                                 ? (1.0 - CvB) * probc_legacy / CvB
                                 : -1.0;

  const double BvC_legacy = (CvB > 0.0) ? (1.0 - CvB) : -1.0;

  const double HFvLF_legacy =
      (probbbblepb > 0.0 && probc_legacy > 0.0 && probs > 0.0 && probudg > 0.0)
          ? (probbbblepb + probc_legacy) /
                (probbbblepb + probc_legacy + probs + probudg)
          : -1.0;

  // DATA는 레거시 그대로 반환
  if (IsDATA)
    return {HFvLF_legacy, BvC_legacy};

  // MC인데 LUT 없으면 레거시 반환(기존과 동일)
  if (!UParT_OT_Central)
    return {HFvLF_legacy, BvC_legacy};

  // -----------------------------
  // (New) probb/probc/problight 계산 (closure=1)
  // -----------------------------
  constexpr double eps = 1e-12;
  Prob3 p_src = compute_prob3_from_branches(probudg, CvL, CvB, SvUDG, eps);

  if (!p_src.ok) {
    // 입력이 이상하면 레거시 fallback
    return {HFvLF_legacy, BvC_legacy};
  }

  // -----------------------------
  // (MC) LUT 매핑: (problight, probc) -> mapped (problight, probc)
  //  hadronFalvor: 5->b, 4->c, else->light(0)
  // -----------------------------
  int pf = int(std::abs(hadronFalvor));
  if (pf != 4 && pf != 5)
    pf = 0;

  // IMPORTANT: LUT input domain = (problight, probc)
  const float pl_in = float(p_src.pl);
  const float pc_in = float(p_src.pc);

  auto [pl_m, pc_m] = UParT_OT_Central->map_probl_probc(pt, pf, pl_in, pc_in);

  // reconstruct pb = 1 - pl - pc, then closure
  double pl = clip(double(pl_m), 0.0, 1.0);
  double pc = clip(double(pc_m), 0.0, 1.0);
  double pb = 1.0 - pl - pc;

  Prob3 p_map = sanitize_prob3(pb, pc, pl, eps);

  // -----------------------------
  // mapped HFvLF/BvC 재계산
  //   HFvLF = pb+pc
  //   BvC   = pb/(pb+pc)
  // -----------------------------
  auto [HFvLF_m, BvC_m] = hf_bvc_from_prob3(p_map, eps);

  // 레거시 값이 음수/비정상인 경우엔 기존처럼 identity 처리(원하는 정책에 맞춰
  // 유지)
  if (HFvLF_legacy < 0.0 || BvC_legacy < 0.0) {
    // 여기선 “매핑하지 않고” 레거시를 그대로 쓰는 정책 유지
    return {HFvLF_legacy, BvC_legacy};
  }

  return {HFvLF_m, BvC_m};
}

std::pair<double, double> Vcb::HFvLF_BvC_from_ParT(const Jet &j) const {
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
      store.uparTAK4CvB[idx], float(store.pt[idx]),
      int(store.hadronFlavour[idx]));
}

Vcb::Cat Vcb::classify_from_scores(double hf, double bvc) const {
  if (!(hf >= 0.0 && hf <= 1.0 && bvc >= 0.0 && bvc <= 1.0))
    return Vcb::Cat::N0;

  const int ih = bin_hf(hf);
  if (ih == 0)
    return Vcb::Cat::L0;
  if (ih == 1)
    return Vcb::Cat::C0;
  if (ih == 2)
    return Vcb::Cat::C1;

  static constexpr Vcb::Cat TOP_MAP[8] = {
      Vcb::Cat::C4, Vcb::Cat::C3, Vcb::Cat::C2, Vcb::Cat::B0,
      Vcb::Cat::B1, Vcb::Cat::B2, Vcb::Cat::B3, Vcb::Cat::B4};
  const int jb = bin_bvc(bvc);
  return TOP_MAP[jb];
}

Vcb::Cat Vcb::classify_from_storage(const JetSoA &store,
                                    std::size_t idx) const {
  auto [hf, bvc] = HFvLF_BvC_from_storage(store, idx);
  return classify_from_scores(hf, bvc);
}

void Vcb::initializeAnalyzer() {
  rle_bucket_compute_checksum();
  SetChannel();
  string SKNANO_HOME = std::getenv("SKNANO_HOME");
  if (!IsDATA) {
    TString json_path = SKNANO_HOME + "/ModellingPatch/" + MCSample.Data() +
                        "_" + DataEra.Data() + "_summary.json";
    load_modelling_json(json_path);
  }
  const auto activeTagger = FlavTagger[DataEra.Data()];
  std::string btagging_eff_file = "btaggingEff.json";
  std::string ctagging_eff_file = "ctaggingEff.json";
  std::string btagging_R_file = "btaggingR.json";
  std::string ctagging_R_file = "ctaggingR.json";
  UParT_OT_Central = std::make_unique<OtJsonLutBank>(
      std::vector<float>{25, 35, 50, 70, 90, 120, 1e+8f});
  UParT_OT_Central->load_json(
      "/data6/Users/yeonjoon/SKNANOAnalyzer_NanoV15/data/Run3_v15_Run2_v15/2024/BTV/LUT_200_v2.json");
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
    myMLHelper_CLASSIF_folds.reserve(4);
    myMLHelper_RECO_folds.reserve(4);
    for (int i = 0; i < 4; ++i) {
      myMLHelper_CLASSIF_folds.push_back(std::make_unique<MLHelper>(
          SKNANO_HOME + "/data/spanet_version_" + std::to_string(i) +
              "_CLASSIF.onnx",
          MLHelper::ModelType::ONNX)); // 생성자 인자 있을 경우
    }
    for (int i = 0; i < 4; ++i) {
      myMLHelper_RECO_folds.push_back(std::make_unique<MLHelper>(
          SKNANO_HOME + "/data/spanet_version_" + std::to_string(i) +
              "_RECO.onnx",
          MLHelper::ModelType::ONNX)); // 생성자 인자 있을 경우
    }
    for (int i = 0; i < 4; ++i) {
      myMLHelper_TabNet_folds.push_back(std::make_unique<MLHelper>(
          SKNANO_HOME + "/data/tabnet_fold" + std::to_string(i) + ".onnx",
          MLHelper::ModelType::ONNX)); // 생성자 인자 있을 경우
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
  if (IsDATA) {
    systHelper = std::make_unique<SystematicHelper>(
        SKNANO_HOME + "/AnalyzerTools/noSyst.yaml", DataStream, DataEra);
  } else {

    systHelper = std::make_unique<SystematicHelper>(
        SKNANO_HOME + "/AnalyzerTools/"
                      "VcbSystematic_BTag.yaml",
        MCSample, DataEra);
  }

  CreateTrainingTree();
  CreateTemplateTrainingTree();
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
      std::cerr << "Out-of-range bucket: b=" << b << " nbuckets=" << nbuckets
                << '\n';
      std::abort();
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

int Vcb::Unroller(RVec<Jet> &jets) {
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

  const Jet &third_leading_BvsC_jet = jets[leadingIndices[2]];
  const Jet &fourth_leading_BvsC_jet = jets[leadingIndices[3]];

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

int Vcb::Unroller(Jet &jet1, Jet &jet2) {
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
    FillHist(name, Jets[i].Pt(), weight, 50, 0.f, 500.f);

    name.assign(base);
    name.append("/Jet_Eta_").append(idx);
    FillHist(name, Jets[i].Eta(), weight, 50, -2.5, 2.5);

    name.assign(base);
    name.append("/Jet_Phi_").append(idx);
    FillHist(name, Jets[i].Phi(), weight, 50, -3.14, 3.14);

    name.assign(base);
    name.append("/Jet_Category_").append(idx);
    FillHist(name, jetCategory, weight, 12, 0.f, 12.f);

    name.assign(base);
    name.append("/Jet_HFvLF_").append(idx);
    FillHist(name, JetHFvLFScore(Jets[i]), weight, 50, 0.f, 1.f);

    name.assign(base);
    name.append("/Jet_BvC_").append(idx);
    FillHist(name, JetBvCScore(Jets[i]), weight, 50, 0.f, 1.f);

    name.assign(base);
    name.append("/Jet_newprobb_").append(idx);
    FillHist(name, JetProbBScore(Jets[i]), weight, 50, 0.f, 1.f);

    name.assign(base);
    name.append("/Jet_newprobc_").append(idx);
    FillHist(name, JetProbCScore(Jets[i]), weight, 50, 0.f, 1.f);

    name.assign(base);
    name.append("/Jet_newproblight_").append(idx);
    FillHist(name, JetProbLScore(Jets[i]), weight, 50, 0.f, 1.f);

    name.assign(base);
    name.append("/Jet_ILRdim1_").append(idx);
    FillHist(name, JetILRdim1Score(Jets[i]), weight, 50, -6.f, 6.f);

    name.assign(base);
    name.append("/Jet_ILRdim2_").append(idx);
    FillHist(name, JetILRdim2Score(Jets[i]), weight, 50, -6.f, 6.f);
  }

  for (size_t i = 0; i < leptons.size(); i++) {
    const std::string idx = std::to_string(i);

    name.assign(base);
    name.append("/Lepton_Pt_").append(idx);
    FillHist(name, leptons[i].Pt(), weight, 50, 0.f, 500.f);

    name.assign(base);
    name.append("/Lepton_Eta_").append(idx);
    FillHist(name, leptons[i].Eta(), weight, 50, -2.5, 2.5);

    name.assign(base);
    name.append("/Lepton_Phi_").append(idx);
    FillHist(name, leptons[i].Phi(), weight, 50, -3.14, 3.14);
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
  ComputeParTScores(jets, jetHFvLFAll, jetBvCAll, jetCategoryAll);
}

void Vcb::ComputeParTScores(const JetViewCollection &jets,
                            std::vector<float> &hfScores,
                            std::vector<float> &bvcScores,
                            std::vector<Vcb::Cat> &categories) const {
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

  hfScores.resize(n);
  bvcScores.resize(n);
  categories.resize(n);

  // ✅ mapped prob3 캐시 (핵심)
  jetProbBAll.resize(n);
  jetProbCAll.resize(n);
  jetProbLAll.resize(n);
  jetILRdim1All.resize(n);
  jetILRdim2All.resize(n);

  // (선택) 기존 캐시 유지한다면 같이 채움
  jetHFvLFAll.resize(n);
  jetBvCAll.resize(n);

  for (std::size_t i = 0; i < n; ++i) {
    // 1) mapped (pb,pc,pl)
    Prob3 p = MappedProb3_from_components(
        double(store.uparTAK4UDG[i]), double(store.uparTAK4SvUDG[i]),
        double(store.uparTAK4CvL[i]), double(store.uparTAK4CvB[i]),
        float(store.pt[i]), int(store.hadronFlavour[i]));

    jetProbBAll[i] = float(p.pb);
    jetProbCAll[i] = float(p.pc);
    jetProbLAll[i] = float(p.pl);
    jetILRdim1All[i] = float(p.ilr_dim1);
    jetILRdim2All[i] = float(p.ilr_dim2);

    // 2) mapped hf/bvc from mapped prob3
    auto [hf, bvc] = hf_bvc_from_prob3(p);

    hfScores[i] = float(hf);
    bvcScores[i] = float(bvc);
    categories[i] = classify_from_scores(hf, bvc);

    jetHFvLFAll[i] = hfScores[i];
    jetBvCAll[i] = bvcScores[i];
  }
}

Vcb::Cat Vcb::JetCategory(const Jet &jet) const {
  const int originalIdx = jet.OriginalIndex();
  if (originalIdx < 0 ||
      static_cast<std::size_t>(originalIdx) >= jetCategoryAll.size())
    return Vcb::Cat::N0;
  return jetCategoryAll[static_cast<std::size_t>(originalIdx)];
}

float Vcb::JetHFvLFScore(const Jet &jet) const {
  const int originalIdx = jet.OriginalIndex();
  if (originalIdx < 0 ||
      static_cast<std::size_t>(originalIdx) >= jetHFvLFAll.size())
    return -1.f;
  return jetHFvLFAll[static_cast<std::size_t>(originalIdx)];
}

float Vcb::JetBvCScore(const Jet &jet) const {
  const int originalIdx = jet.OriginalIndex();
  if (originalIdx < 0 ||
      static_cast<std::size_t>(originalIdx) >= jetBvCAll.size())
    return -1.f;
  return jetBvCAll[static_cast<std::size_t>(originalIdx)];
}

float Vcb::JetProbBScore(const Jet &jet) const {
  const int originalIdx = jet.OriginalIndex();
  if (originalIdx < 0 ||
      static_cast<std::size_t>(originalIdx) >= jetProbBAll.size())
    return -1.f;
  return jetProbBAll[static_cast<std::size_t>(originalIdx)];
}

float Vcb::JetProbCScore(const Jet &jet) const {
  const int originalIdx = jet.OriginalIndex();
  if (originalIdx < 0 ||
      static_cast<std::size_t>(originalIdx) >= jetProbCAll.size())
    return -1.f;
  return jetProbCAll[static_cast<std::size_t>(originalIdx)];
}

float Vcb::JetProbLScore(const Jet &jet) const {
  const int originalIdx = jet.OriginalIndex();
  if (originalIdx < 0 ||
      static_cast<std::size_t>(originalIdx) >= jetProbLAll.size())
    return -1.f;
  return jetProbLAll[static_cast<std::size_t>(originalIdx)];
}

float Vcb::JetILRdim1Score(const Jet &jet) const {
  const int originalIdx = jet.OriginalIndex();
  if (originalIdx < 0 ||
      static_cast<std::size_t>(originalIdx) >= jetILRdim1All.size())
    return -999.f;
  return jetILRdim1All[static_cast<std::size_t>(originalIdx)];
}

float Vcb::JetILRdim2Score(const Jet &jet) const {
  const int originalIdx = jet.OriginalIndex();
  if (originalIdx < 0 ||
      static_cast<std::size_t>(originalIdx) >= jetILRdim2All.size())
    return -999.f;
  return jetILRdim2All[static_cast<std::size_t>(originalIdx)];
}

short Vcb::GetPassedBTaggingWP(const Jet &jet) {
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

short Vcb::GetPassedCTaggingWP(const Jet &jet) {
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
  AllMuonViews = GetAllMuonViews();
  AllElectronViews = GetAllElectronViews();
  AllJetViews = GetAllJetViews();
  AllGenViews = GetAllGenViews();
  UpdateAllJetTaggingCaches(AllJetViews);
  AllGens = GetAllGens();
  AllGenJets = GetAllGenJets();
  ev = GetEvent();
  if(!myCorr->IsGoldenLumi(RunNumber, luminosityBlock)){
    return;
  }

  if (HasFlag("Skim")) {
    Clear();
    SkimTree();
    return;
  }
  if (HasFlag("TemplateTraining")) {
    Clear();
    for (const auto &syst_dummy : *systHelper) {
      if (!PassBaseLineSelection(false, false))
        continue;
      InferONNX();
      if (systHelper->getCurrentIterSysTarget().find("Central") !=
          std::string::npos) {
        FillTemplateTrainingTree();
        return;
      }
    }
    return;
  }

  if (HasFlag("Training")) {
    Clear();
    for (const auto &syst_dummy : *systHelper) {
      if (!PassBaseLineSelection(false, true))
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
  for (const auto &syst_dummy : *systHelper) {
    leptons.clear();
    executeEventFromParameter();
    if (HasFlag("spurious"))
      break;
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
                                   MyCorrection::variation::nom) /
                 modelling_patches[subproc_name].patch_ScaleVariation[4];
        case MyCorrection::variation::down:
          return GetScaleVariation(MyCorrection::variation::down,
                                   MyCorrection::variation::nom) /
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
                                   MyCorrection::variation::up) /
                 modelling_patches[subproc_name].patch_ScaleVariation[6];
        case MyCorrection::variation::down:
          return GetScaleVariation(MyCorrection::variation::nom,
                                   MyCorrection::variation::down) /
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
    weight_function_map["btag"] = dummy_lambda;
  else
    weight_function_map["btag"] = dummy_lambda;
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
        assert(FirstCopyTopIndex == npos &&
               "Multiple first-copy tops found in event");
        FirstCopyTopIndex = idx;
      }
      if (isLastCopy) {
        assert(LastCopyTopIndex == npos &&
               "Multiple last-copy tops found in event");
        LastCopyTopIndex = idx;
      }
    } else if (pdg == -6) { // anti-top
      if (isFirstCopy) {
        assert(FirstCopyAntiTopIndex == npos &&
               "Multiple first-copy antitops found in event");
        FirstCopyAntiTopIndex = idx;
      }
      if (isLastCopy) {
        assert(LastCopyAntiTopIndex == npos &&
               "Multiple last-copy antitops found in event");
        LastCopyAntiTopIndex = idx;
      }
    }
  }

  assert(FirstCopyTopIndex != npos && "No first-copy top found in event");
  assert(FirstCopyAntiTopIndex != npos &&
         "No first-copy antitop found in event");
  assert(LastCopyTopIndex != npos && "No last-copy top found in event");
  assert(LastCopyAntiTopIndex != npos && "No last-copy antitop found in event");

  return {FirstCopyTopIndex, FirstCopyAntiTopIndex, LastCopyTopIndex,
          LastCopyAntiTopIndex};
}

void Vcb::Clear() {
  HT = 0;
  n_jets = 0;
  n_b_tagged_jets = 0;
  n_c_tagged_jets = 0;
  n_hf_jets = 0;
  n_hadronFlav_b_jets = 0;
  n_hadronFlav_c_jets = 0;
  find_all_jets = false;
  leptons.clear();
  Jets.clear();
  MET = Particle();
  ttbj = false;
  ttbb = false;
  ttcc = false;
  ttLF = false;
}

void Vcb::executeEventFromParameter() {
  Clear();
  enum class OUTPUT_TYPE { HISTOGRAMS, TREE };
  OUTPUT_TYPE output_type = OUTPUT_TYPE::HISTOGRAMS;
  if (HasFlag("OutputTrees"))
    output_type = OUTPUT_TYPE::TREE;

  if (!PassBaseLineSelection())
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
  if (output_type == OUTPUT_TYPE::HISTOGRAMS) {
    // InferONNX();
    // InferTabNet();
  }

  // -------------------------
  // DATA: Inclusive/Central/data_obs 만 채움
  // -------------------------
  if (IsDATA) {
    if (output_type == OUTPUT_TYPE::HISTOGRAMS) {
      FillHistogramsAtThisPoint(base_path + "Central/data_obs", 1.f);
      // FillONNXRecoInfo(base_path + "Central/data_obs", 1.f);
      // FillTabNetInfo(base_path + "Central/data_obs", 1.f);
    } else {
      const std::unordered_map<std::string, float> data_weights = {
          {"Central", 1.f}};
      FillTreeAtThisPoint(current_iter_prefix + "data_obs", 1.f, data_weights);
    }
    return;
  }

  // -------------------------
  // MC: 프리북(0 weight) → 가중치 채움
  // -------------------------
  // 1) 프리북: 시스템틱 키 전부에 대해 0으로 한 번씩 채워서 히스토그램 생성
  if (output_type == OUTPUT_TYPE::HISTOGRAMS) {
    const auto prebook_weights =
        systHelper->calculateWeight(true); // skeleton only
    // Central이 calculateWeight(true)에 없을 수 있으니 명시적으로 한 번 프리북
    FillHistogramsAtThisPoint(base_path + "Central/" + sample_postfix, 0.f);
    for (const auto &kv : prebook_weights) {
      const std::string &syst = kv.first;
      FillHistogramsAtThisPoint(base_path + syst + "/" + sample_postfix, 0.f);
    }
  }

  // 2) 실제 채움: normalization × 각 시스템틱 가중치
  const float normalization = MCNormalization();
  const auto weight_map = systHelper->calculateWeight(false);
  if (output_type == OUTPUT_TYPE::TREE) {
    FillTreeAtThisPoint(current_iter_prefix + Sample_Shorthand[MCSample.Data()],
                        normalization, weight_map);
  } else {
    for (const auto &kv : weight_map) {
      const std::string &syst = kv.first;
      const float w = kv.second;
      FillHistogramsAtThisPoint(base_path + syst + "/" + sample_postfix,
                                w * normalization);
      // FillONNXRecoInfo(base_path + syst + "/" + sample_postfix,
      // w * normalization);
      // FillTabNetInfo(base_path + syst + "/" + sample_postfix,
      // w * normalization);
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
void Vcb::FillTemplateTrainingTree() {}

RVec<int> Vcb::FindTTbarJetIndices() {
  RVec<int> iamnothing;
  return iamnothing;
}

void Vcb::FillKinematicFitterResult(const TString &histPrefix, float weight) {}

RVec<RVec<unsigned int>> Vcb::GetPermutations(const RVec<Jet> &jets) {
  RVec<RVec<unsigned int>> iamnothing;
  return iamnothing;
}

void Vcb::SkimTree() {
  Clear();
  if (!skimTreeInitialized) {
    RVec<TString> keeps = {"*"};
    RVec<TString> drops = {};
    // NewTree("Events", keeps, drops); // Placeholder; filled in WriteHist
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

      // 2) BranchManager가 쥐고 있던 주소 정리 (너 프레임워크에 맞게)
      //    예: BranchBase::ResetAllBranchAddresses(); 같은 함수 있으면 호출
      // BranchBase::ResetAllBranchAddresses(); // TODO: 네 쪽 이름 맞춰서

      // 3) 모든 브랜치 활성화 + ROOT가 버퍼 관리
      fChain->SetBranchStatus("*", 1);
      fChain->ResetBranchAddresses();

      // 4) 첫 트리에 대해 캐시/프리페치 설정
      if (TTree *firstTree = fChain->GetTree()) {
        configureTreeCache(firstTree); // SKNanoLoader 멤버
      }

      // 5) 구조만 복사한 빈 트리 생성 (fast 모드)
      std::unique_ptr<TTree> skimTree(fChain->CloneTree(0, "fast"));
      if (skimTree) {
        skimTree->SetName("Events");

        Long64_t prevTreeNumber = -1;

        // 6) 선택된 글로벌 엔트리들만 순회
        for (Long64_t g : skim_passed_global_entries) {
          // 체인의 해당 글로벌 엔트리로 이동 (트리/파일 자동 전환)
          Long64_t local = fChain->LoadTree(g);
          if (local < 0)
            continue;

          TTree *tree = fChain->GetTree();
          if (!tree)
            continue;

          // 트리가 바뀔 때마다 캐시 재설정
          if (tree->GetTreeNumber() != prevTreeNumber) {
            configureTreeCache(tree);
            prevTreeNumber = tree->GetTreeNumber();
          }

          // 실제 데이터 로드
          if (tree->GetEntry(local) <= 0)
            continue;

          // CopyTree가 하던 일을 우리가 직접 Fill
          skimTree->Fill();
        }

        treemap["Events"] = skimTree.release();
      }

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
