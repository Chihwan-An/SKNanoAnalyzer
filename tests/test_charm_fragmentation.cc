#include "CharmFragmentation.h"

#include <cassert>
#include <cmath>
#include <iostream>

namespace {

bool Close(float first, float second, float tolerance = 1.e-5f) {
  return std::abs(first - second) < tolerance;
}

void TestLongitudinalMomentumFraction() {
  const HadronAnalysis::FragmentationKinematics jet{50.f, 0.7f, 1.2f, 8.f};
  const HadronAnalysis::FragmentationKinematics collinearHadron{10.f, 0.7f,
                                                                1.2f, 2.f};
  assert(
      Close(HadronAnalysis::LongitudinalMomentumFraction(collinearHadron, jet),
            0.2f));

  const HadronAnalysis::FragmentationKinematics invalidJet{};
  assert(HadronAnalysis::LongitudinalMomentumFraction(collinearHadron,
                                                      invalidJet) < 0.f);
}

void TestInvariantMass() {
  constexpr float pi = 3.14159265358979323846f;
  const HadronAnalysis::FragmentationKinematics first{40.f, 0.f, 0.f, 0.f};
  const HadronAnalysis::FragmentationKinematics second{40.f, 0.f, pi, 0.f};
  assert(Close(HadronAnalysis::InvariantMass(first, second), 80.f, 1.e-4f));
}

void TestRecoWMassBins() {
  assert(HadronAnalysis::RecoWMassBin(-1.f) == -1);
  assert(HadronAnalysis::RecoWMassBin(59.9f) == 0);
  assert(HadronAnalysis::RecoWMassBin(60.f) == 1);
  assert(HadronAnalysis::RecoWMassBin(79.9f) == 2);
  assert(HadronAnalysis::RecoWMassBin(80.f) == 3);
  assert(HadronAnalysis::RecoWMassBin(100.f) == 5);
  assert(HadronAnalysis::RecoWMassBin(250.f) == 5);
}

} // namespace

int main() {
  TestLongitudinalMomentumFraction();
  TestInvariantMass();
  TestRecoWMassBins();
  std::cout << "CharmFragmentation tests passed\n";
  return 0;
}
