#include "TopJetMatcher.h"

#include <cassert>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace {

constexpr std::uint16_t kHard = 1U << 7;
constexpr std::uint16_t kFromHard = 1U << 8;
constexpr std::uint16_t kFirst = 1U << 12;
constexpr std::uint16_t kLast = 1U << 13;
constexpr std::uint16_t kLastBeforeFSR = 1U << 14;

HadronAnalysis::GenParticleInput
Particle(int pdgId, int mother, float eta, float phi,
         std::uint16_t flags = kFromHard | kLast | kLastBeforeFSR,
         float pt = 60.f) {
  return {pt, eta, phi, 0.f, pdgId, mother, 23, flags};
}

std::vector<HadronAnalysis::GenParticleInput> SemileptonicTruth() {
  return {
      Particle(6, -1, 0.f, 0.f, kHard | kFromHard | kFirst, 180.f),
      Particle(6, 0, 0.f, 0.f, kHard | kFromHard | kLast, 175.f),
      Particle(-6, -1, 1.f, 1.f, kHard | kFromHard | kFirst, 170.f),
      Particle(-6, 2, 1.f, 1.f, kHard | kFromHard | kLast, 165.f),
      Particle(24, 1, 0.1f, 0.1f, kFromHard | kLast, 100.f),
      Particle(5, 1, -0.8f, -0.7f),
      Particle(-24, 3, 1.1f, 1.1f, kFromHard | kLast, 90.f),
      Particle(-5, 3, 1.8f, 1.7f),
      Particle(2, 4, 0.2f, 0.2f, kFromHard | kFirst, 55.f),
      Particle(2, 8, 0.22f, 0.21f, kFromHard | kLast | kLastBeforeFSR, 52.f),
      Particle(-1, 4, 0.6f, 0.55f),
      Particle(13, 6, 1.15f, 1.12f),
      Particle(-14, 6, 1.05f, 1.08f),
  };
}

std::vector<HadronAnalysis::GenJetInput> StandardGenJets() {
  return {
      {58.f, -0.79f, -0.69f, 8.f, 5},
      {57.f, 1.79f, 1.69f, 8.f, -5},
      {50.f, 0.23f, 0.22f, 7.f, 2},
      {48.f, 0.61f, 0.56f, 7.f, -1},
  };
}

void TestExactNanoAODLinksAndCopyResolution() {
  const auto truth = SemileptonicTruth();
  const auto genJets = StandardGenJets();
  const std::vector<HadronAnalysis::RecoJetInput> recoJets = {
      {49.f, 0.24f, 0.22f, 8.f, 0, 31, 2},
      {56.f, -0.78f, -0.68f, 9.f, 1, 12, 0},
      {47.f, 0.60f, 0.55f, 8.f, 2, 44, 3},
      {55.f, 1.78f, 1.68f, 9.f, 3, 5, 1},
  };

  const auto result =
      HadronAnalysis::TopJetMatcher{}.Match(truth, genJets, recoJets);
  assert(result.topologyStatus == HadronAnalysis::TopologyStatus::Success);
  assert(result.hadronicTopPdgId == 6);
  assert(result.complete);
  assert(result.nGenMatched == 4);
  assert(result.nRecoMatched == 4);
  assert(result.matches[0].genPartIndex == 5);
  assert(result.matches[1].genPartIndex == 7);
  assert(result.matches[2].genPartIndex == 9);
  assert(result.matches[3].genPartIndex == 10);
  assert(result.matches[0].recoJetIndex == 1);
  assert(result.matches[1].recoJetIndex == 3);
  assert(result.matches[2].recoJetIndex == 0);
  assert(result.matches[3].recoJetIndex == 2);
  assert(result.matches[0].recoJetOriginalIndex == 12);
  for (const auto &match : result.matches) {
    assert(match.recoMatchSource ==
           HadronAnalysis::RecoMatchSource::NanoAODGenJetIndex);
    assert(!match.usedAbsoluteFlavourFallback);
  }
}

void TestGlobalDeltaRAssignmentBeatsGreedy() {
  auto truth = SemileptonicTruth();
  auto genJets = StandardGenJets();
  truth[5].eta = 0.f;
  truth[5].phi = 0.f;
  truth[9].eta = 0.18f;
  truth[9].phi = 0.f;
  genJets[0].eta = 0.f;
  genJets[0].phi = 0.f;
  genJets[2].eta = 0.18f;
  genJets[2].phi = 0.f;

  const std::vector<HadronAnalysis::RecoJetInput> recoJets = {
      // Greedy would give this shared jet to role 0, starving role 2.
      {54.f, 0.08f, 0.f, 8.f, 0, 100, -1},
      {53.f, -0.15f, 0.f, 8.f, 1, 101, -1},
      {55.f, 1.79f, 1.69f, 8.f, 2, 102, 1},
      {46.f, 0.61f, 0.56f, 8.f, 3, 103, 3},
  };

  const auto result =
      HadronAnalysis::TopJetMatcher{}.Match(truth, genJets, recoJets);
  assert(result.complete);
  assert(result.matches[0].recoJetIndex == 1);
  assert(result.matches[2].recoJetIndex == 0);
  assert(result.matches[0].recoMatchSource ==
         HadronAnalysis::RecoMatchSource::DeltaRFallback);
  assert(result.matches[2].recoMatchSource ==
         HadronAnalysis::RecoMatchSource::DeltaRFallback);
  assert(result.assignmentMargin == -1.f);
}

void TestNonSemileptonicTopologyIsReported() {
  auto truth = SemileptonicTruth();
  truth[11] = Particle(-2, 6, 1.15f, 1.12f);
  truth[12] = Particle(1, 6, 1.05f, 1.08f);
  const auto result =
      HadronAnalysis::TopJetMatcher{}.Match(truth, StandardGenJets(), {});
  assert(result.topologyStatus ==
         HadronAnalysis::TopologyStatus::NotSemileptonic);
  assert(!result.complete);
  assert(result.nGenMatched == 0);
  assert(result.nRecoMatched == 0);
}

void TestSignedFlavourFallbackIsExplicit() {
  const auto truth = SemileptonicTruth();
  auto genJets = StandardGenJets();
  genJets[1].partonFlavour = 5;
  const std::vector<HadronAnalysis::RecoJetInput> recoJets = {
      {56.f, -0.79f, -0.69f, 8.f, 0, 10, 0},
      {56.f, 1.79f, 1.69f, 8.f, 1, 11, 1},
      {49.f, 0.23f, 0.22f, 8.f, 2, 12, 2},
      {47.f, 0.61f, 0.56f, 8.f, 3, 13, 3},
  };
  const auto result =
      HadronAnalysis::TopJetMatcher{}.Match(truth, genJets, recoJets);
  assert(result.complete);
  assert(result.matches[1].usedAbsoluteFlavourFallback);
  assert(!result.matches[0].usedAbsoluteFlavourFallback);
}

void TestInvalidConfigurationIsRejected() {
  HadronAnalysis::TopJetMatcherConfig config;
  config.recoGenFallbackMaxDeltaR = 0.f;
  bool threw = false;
  try {
    const HadronAnalysis::TopJetMatcher matcher(config);
    static_cast<void>(matcher);
  } catch (const std::invalid_argument &) {
    threw = true;
  }
  assert(threw);
}

} // namespace

int main() {
  TestExactNanoAODLinksAndCopyResolution();
  TestGlobalDeltaRAssignmentBeatsGreedy();
  TestNonSemileptonicTopologyIsReported();
  TestSignedFlavourFallbackIsExplicit();
  TestInvalidConfigurationIsRejected();
  std::cout << "TopJetMatcher tests passed\n";
  return 0;
}
