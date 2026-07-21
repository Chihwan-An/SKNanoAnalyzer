#ifndef LAMBDAC_TRUTH_MATCHER_H
#define LAMBDAC_TRUTH_MATCHER_H

#include <array>
#include <cstdint>
#include <vector>

namespace HadronAnalysis {

struct LambdaCRecoTrackInput {
  float pt = 0.f;
  float eta = 0.f;
  float phi = 0.f;
  int charge = 0;
};

struct LambdaCGenParticleInput {
  float pt = 0.f;
  float eta = 0.f;
  float phi = 0.f;
  int pdgId = 0;
  int motherIndex = -1;
  int status = 0;
};

struct LambdaCTruthMatchConfig {
  float daughterMaxDeltaR = 0.03f;
  float daughterMaxRelativePt = 0.50f;
};

struct LambdaCTruthHypothesisResult {
  // Bit order matches LambdaCToPKPi_hypothesisMask:
  // pKpi, ppiK, Kppi, pipK, Kpip, piKp.
  std::uint8_t mask = 0;
  int bestIndex = -1;
};

class LambdaCTruthMatcher {
public:
  explicit LambdaCTruthMatcher(LambdaCTruthMatchConfig config = {});

  LambdaCTruthHypothesisResult Match(
      int parentIndex, const std::array<LambdaCRecoTrackInput, 3> &tracks,
      const std::vector<LambdaCGenParticleInput> &particles) const;

private:
  LambdaCTruthMatchConfig config_;
};

} // namespace HadronAnalysis

#endif
