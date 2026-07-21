#ifndef CHARMFRAGMENTATION_H
#define CHARMFRAGMENTATION_H

#include <array>
#include <cstddef>

namespace HadronAnalysis {

struct FragmentationKinematics {
  float pt = 0.f;
  float eta = 0.f;
  float phi = 0.f;
  float mass = 0.f;
};

constexpr std::array<float, 5> kRecoWMassSplitPoints{
    {60.f, 70.f, 80.f, 90.f, 100.f}};
constexpr std::size_t kNumRecoWMassBins = kRecoWMassSplitPoints.size() + 1;

float LongitudinalMomentumFraction(const FragmentationKinematics &hadron,
                                   const FragmentationKinematics &jet);

float InvariantMass(const FragmentationKinematics &first,
                    const FragmentationKinematics &second);

int RecoWMassBin(float mass);

} // namespace HadronAnalysis

#endif
