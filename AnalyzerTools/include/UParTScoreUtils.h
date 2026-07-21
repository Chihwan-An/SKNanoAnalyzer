#pragma once

#include <utility>
#include <vector>

class OtJsonLutBank;

namespace UParTScore {

enum class Cat { N0, L0, C0, C1, C2, C3, C4, B0, B1, B2, B3, B4 };

inline constexpr double HF_T1 = 0.250;
inline constexpr double HF_T2 = 0.454;
inline constexpr double HF_T3 = 0.810;
inline constexpr double BVC_T1 = 0.006;
inline constexpr double BVC_T2 = 0.016;
inline constexpr double BVC_T3 = 0.056;
inline constexpr double BVC_T4 = 0.760;
inline constexpr double BVC_T5 = 0.944;
inline constexpr double BVC_T6 = 0.984;
inline constexpr double BVC_T7 = 0.994;

struct Prob3 {
  double pb = 0.0;
  double pc = 0.0;
  double pl = 1.0; // light/udsg
  double ilr_dim1 = 0.0;
  double ilr_dim2 = 0.0;
  bool ok = false;
};

int bin_hf(double x);
int bin_bvc(double y);
Cat classify_from_scores(double hf, double bvc);

double clip(double x, double lo, double hi);

Prob3 sanitize_prob3(double pb, double pc, double pl, double eps = 1e-12);

Prob3 compute_prob3_from_branches(double p_udg, double CvL, double CvB,
                                  double SvUDG, double eps = 1e-12);

std::pair<double, double> hf_bvc_from_prob3(const Prob3 &p,
                                            double eps = 1e-12);

class Mapper {
public:
  Mapper(const OtJsonLutBank *lut, bool isData, bool unmapped = false)
      : lut_(lut), isData_(isData), unmapped_(unmapped) {}

  Prob3 MappedProb3(double probudg, double SvUDG, double CvL, double CvB,
                    float pt, int hadronFlavor) const;

  std::pair<double, double> HFvLF_BvC(double probudg, double SvUDG, double CvL,
                                      double CvB, float pt,
                                      int hadronFlavor) const;

private:
  const OtJsonLutBank *lut_ = nullptr;
  bool isData_ = false;
  bool unmapped_ = false;
};

} // namespace UParTScore
