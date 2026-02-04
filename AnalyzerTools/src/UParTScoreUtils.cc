#include "UParTScoreUtils.h"

#include "OtJsonLutBank.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <numeric>
#include <stdexcept>

namespace UParTScore {

double clip(double x, double lo, double hi) {
  return std::min(hi, std::max(lo, x));
}

Prob3 sanitize_prob3(double pb, double pc, double pl, double eps) {
  pb = std::max(pb, eps);
  pc = std::max(pc, eps);
  pl = std::max(pl, eps);

  const double s = pb + pc + pl;

  Prob3 out;
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

Prob3 compute_prob3_from_branches(double p_udg, double CvL, double CvB,
                                  double SvUDG, double eps) {
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

std::pair<double, double> hf_bvc_from_prob3(const Prob3 &p, double eps) {
  const double hf = clip(p.pb + p.pc, 0.0, 1.0);
  const double den = std::max(hf, eps);
  const double bvc = clip(p.pb / den, 0.0, 1.0);
  return {hf, bvc};
}

Prob3 Mapper::MappedProb3(double probudg, double SvUDG, double CvL, double CvB,
                          float pt, int hadronFlavor) const {
  constexpr double eps = 1e-12;

  Prob3 p = compute_prob3_from_branches(probudg, CvL, CvB, SvUDG, eps);

  auto helmert_submatrix = [](int K) -> std::vector<double> {
    if (K < 2)
      throw std::invalid_argument("K<2");
    std::vector<double> H(static_cast<std::size_t>(K) * (K - 1), 0.0);
    for (int j = 1; j < K; ++j) {
      double d = std::sqrt(double(j) * (j + 1.0));
      for (int i = 0; i < j; ++i)
        H[static_cast<std::size_t>(i) * (K - 1) + (j - 1)] = 1.0 / d;
      H[static_cast<std::size_t>(j) * (K - 1) + (j - 1)] = -double(j) / d;
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
        v += clr[i] * H[static_cast<std::size_t>(i) * (K - 1) + c];
      Z[c] = v;
    }
    return Z;
  };

  auto iilr = [&](const std::vector<double> &Z,
                  int ref_idx = -1) -> std::vector<double> {
    int Km1 = (int)Z.size(), K = Km1 + 1;
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
    std::vector<int> inv_perm(K, -1);
    for (int j = 0; j < K; ++j)
      inv_perm[perm[j]] = j;

    auto H = helmert_submatrix(K);
    std::vector<double> clr(K, 0.0), Pp(K), P(K);
    for (int i = 0; i < K; ++i) {
      double v = 0.0;
      for (int c = 0; c < K - 1; ++c)
        v += Z[c] * H[static_cast<std::size_t>(i) * (K - 1) + c];
      clr[i] = v;
    }

    double s = 0.0;
    for (int i = 0; i < K; ++i) {
      Pp[i] = std::exp(clr[i]);
      s += Pp[i];
    }
    if (!(s > 0.0))
      throw std::runtime_error("sum");
    for (int i = 0; i < K; ++i)
      Pp[i] /= s;

    for (int orig = 0; orig < K; ++orig)
      P[orig] = Pp[inv_perm[orig]];
    return P;
  };

  auto ilr_vec = ilr({p.pb, p.pc, p.pl});

  if (isData_ || unmapped_ || !lut_) {
    Prob3 unmapped_prob3 = sanitize_prob3(p.pb, p.pc, p.pl, eps);
    unmapped_prob3.ilr_dim1 = ilr_vec[0];
    unmapped_prob3.ilr_dim2 = ilr_vec[1];
    return unmapped_prob3;
  }

  int pf = int(std::abs(hadronFlavor));
  if (pf != 4 && pf != 5)
    pf = 0;

  auto [mapped_ilr1, mapped_ilr2] =
      lut_->map_ilr_z(pt, pf, float(ilr_vec[0]), float(ilr_vec[1]));

  auto mapped_vec = iilr({mapped_ilr1, mapped_ilr2});

  Prob3 mapped_prob3 =
      sanitize_prob3(mapped_vec[0], mapped_vec[1], mapped_vec[2], eps);
  mapped_prob3.ilr_dim1 = mapped_ilr1;
  mapped_prob3.ilr_dim2 = mapped_ilr2;
  return mapped_prob3;
}

std::pair<double, double>
Mapper::HFvLF_BvC(double probudg, double SvUDG, double CvL, double CvB,
                  float pt, int hadronFlavor) const {
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

  if (isData_)
    return {HFvLF_legacy, BvC_legacy};

  if (!lut_)
    return {HFvLF_legacy, BvC_legacy};

  constexpr double eps = 1e-12;
  Prob3 p_src = compute_prob3_from_branches(probudg, CvL, CvB, SvUDG, eps);

  if (!p_src.ok) {
    return {HFvLF_legacy, BvC_legacy};
  }

  int pf = int(std::abs(hadronFlavor));
  if (pf != 4 && pf != 5)
    pf = 0;

  const float pl_in = float(p_src.pl);
  const float pc_in = float(p_src.pc);

  auto [pl_m, pc_m] = lut_->map_probl_probc(pt, pf, pl_in, pc_in);

  double pl = clip(double(pl_m), 0.0, 1.0);
  double pc = clip(double(pc_m), 0.0, 1.0);
  double pb = 1.0 - pl - pc;

  Prob3 p_map = sanitize_prob3(pb, pc, pl, eps);

  auto [HFvLF_m, BvC_m] = hf_bvc_from_prob3(p_map, eps);

  if (HFvLF_legacy < 0.0 || BvC_legacy < 0.0) {
    return {HFvLF_legacy, BvC_legacy};
  }

  return {HFvLF_m, BvC_m};
}

} // namespace UParTScore
