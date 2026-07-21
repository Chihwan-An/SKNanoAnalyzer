#include "LambdaCTruthMatcher.h"

#include <cassert>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace {

using Gen = HadronAnalysis::LambdaCGenParticleInput;
using Track = HadronAnalysis::LambdaCRecoTrackInput;

Track Reco(const Gen &particle) {
  return {particle.pt, particle.eta, particle.phi,
          (particle.pdgId > 0) - (particle.pdgId < 0)};
}

void TestDirectDecayAssignment() {
  const std::vector<Gen> particles = {
      {20.f, 0.f, 0.f, 4122, -1, 2},
      {5.f, 0.10f, 0.10f, 2212, 0, 1},
      {4.f, 0.20f, 0.20f, -321, 0, 1},
      {3.f, 0.30f, 0.30f, 211, 0, 1},
  };
  // Reco order K, p, pi corresponds to hypothesis index 2 (Kppi).
  const std::array<Track, 3> tracks = {
      Reco(particles[2]), Reco(particles[1]), Reco(particles[3])};
  const auto result =
      HadronAnalysis::LambdaCTruthMatcher{}.Match(0, tracks, particles);
  assert(result.mask == (std::uint8_t{1} << 2));
  assert(result.bestIndex == 2);
}

void TestResonantAntiLambdaCDecay() {
  const std::vector<Gen> particles = {
      {20.f, 0.f, 0.f, -4122, -1, 2},
      {10.f, 0.1f, 0.1f, -313, 0, 2},
      {5.f, -0.1f, -0.1f, -2212, 0, 1},
      {4.f, 0.2f, 0.2f, 321, 1, 1},
      {3.f, 0.3f, 0.3f, -211, 1, 1},
  };
  // Reco order pi, K, p corresponds to hypothesis index 5 (piKp).
  const std::array<Track, 3> tracks = {
      Reco(particles[4]), Reco(particles[3]), Reco(particles[2])};
  const auto result =
      HadronAnalysis::LambdaCTruthMatcher{}.Match(0, tracks, particles);
  assert(result.mask == (std::uint8_t{1} << 5));
  assert(result.bestIndex == 5);
}

void TestAmbiguousSameChargeTracksKeepMask() {
  const std::vector<Gen> particles = {
      {20.f, 0.f, 0.f, 4122, -1, 2},
      {5.f, 0.10f, 0.10f, 2212, 0, 1},
      {4.f, 0.20f, 0.20f, -321, 0, 1},
      {5.f, 0.10f, 0.10f, 211, 0, 1},
  };
  const std::array<Track, 3> tracks = {
      Reco(particles[1]), Reco(particles[2]), Reco(particles[3])};
  const auto result =
      HadronAnalysis::LambdaCTruthMatcher{}.Match(0, tracks, particles);
  const std::uint8_t expected =
      (std::uint8_t{1} << 0) | (std::uint8_t{1} << 5);
  assert(result.mask == expected);
  assert(result.bestIndex == 0);
}

void TestInvalidOrIncompatibleParentIsUnmatched() {
  const std::vector<Gen> particles = {
      {20.f, 0.f, 0.f, 421, -1, 2},
      {5.f, 0.10f, 0.10f, 2212, 0, 1},
      {4.f, 0.20f, 0.20f, -321, 0, 1},
      {3.f, 0.30f, 0.30f, 211, 0, 1},
  };
  const std::array<Track, 3> tracks = {
      Reco(particles[1]), Reco(particles[2]), Reco(particles[3])};
  const auto result =
      HadronAnalysis::LambdaCTruthMatcher{}.Match(0, tracks, particles);
  assert(result.mask == 0);
  assert(result.bestIndex == -1);
}

void TestInvalidConfigurationIsRejected() {
  HadronAnalysis::LambdaCTruthMatchConfig config;
  config.daughterMaxDeltaR = 0.f;
  bool threw = false;
  try {
    const HadronAnalysis::LambdaCTruthMatcher matcher(config);
    static_cast<void>(matcher);
  } catch (const std::invalid_argument &) {
    threw = true;
  }
  assert(threw);
}

} // namespace

int main() {
  TestDirectDecayAssignment();
  TestResonantAntiLambdaCDecay();
  TestAmbiguousSameChargeTracksKeepMask();
  TestInvalidOrIncompatibleParentIsUnmatched();
  TestInvalidConfigurationIsRejected();
  std::cout << "LambdaCTruthMatcher tests passed\n";
  return 0;
}
