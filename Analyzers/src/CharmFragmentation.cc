#include "CharmFragmentation.h"

#include <algorithm>
#include <cmath>

namespace HadronAnalysis {
namespace {

struct CartesianFourVector {
  float px = 0.f;
  float py = 0.f;
  float pz = 0.f;
  float energy = 0.f;
};

CartesianFourVector Cartesian(const FragmentationKinematics &input) {
  CartesianFourVector result;
  result.px = input.pt * std::cos(input.phi);
  result.py = input.pt * std::sin(input.phi);
  result.pz = input.pt * std::sinh(input.eta);
  const float momentumSquared =
      result.px * result.px + result.py * result.py + result.pz * result.pz;
  result.energy =
      std::sqrt(std::max(0.f, momentumSquared + input.mass * input.mass));
  return result;
}

} // namespace

float LongitudinalMomentumFraction(const FragmentationKinematics &hadron,
                                   const FragmentationKinematics &jet) {
  const auto hadronP4 = Cartesian(hadron);
  const auto jetP4 = Cartesian(jet);
  const float jetMomentumSquared =
      jetP4.px * jetP4.px + jetP4.py * jetP4.py + jetP4.pz * jetP4.pz;
  if (!(jetMomentumSquared > 0.f))
    return -1.f;
  return (hadronP4.px * jetP4.px + hadronP4.py * jetP4.py +
          hadronP4.pz * jetP4.pz) /
         jetMomentumSquared;
}

float InvariantMass(const FragmentationKinematics &first,
                    const FragmentationKinematics &second) {
  const auto firstP4 = Cartesian(first);
  const auto secondP4 = Cartesian(second);
  const float energy = firstP4.energy + secondP4.energy;
  const float px = firstP4.px + secondP4.px;
  const float py = firstP4.py + secondP4.py;
  const float pz = firstP4.pz + secondP4.pz;
  return std::sqrt(
      std::max(0.f, energy * energy - px * px - py * py - pz * pz));
}

int RecoWMassBin(float mass) {
  if (!(mass >= 0.f))
    return -1;
  return static_cast<int>(std::upper_bound(kRecoWMassSplitPoints.begin(),
                                           kRecoWMassSplitPoints.end(), mass) -
                          kRecoWMassSplitPoints.begin());
}

} // namespace HadronAnalysis
