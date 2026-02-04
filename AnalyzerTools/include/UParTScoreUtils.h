#pragma once

#include <utility>
#include <vector>

class OtJsonLutBank;

namespace UParTScore {

struct Prob3 {
  double pb = 0.0;
  double pc = 0.0;
  double pl = 1.0; // light/udsg
  double ilr_dim1 = 0.0;
  double ilr_dim2 = 0.0;
  bool ok = false;
};

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
