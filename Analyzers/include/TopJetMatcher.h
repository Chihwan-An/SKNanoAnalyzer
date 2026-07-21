#ifndef TOPJETMATCHER_H
#define TOPJETMATCHER_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace HadronAnalysis {

constexpr std::size_t kNumTopJetRoles = 4;

enum class TopJetRole : int {
  HadronicB = 0,
  LeptonicB = 1,
  HadronicWUpType = 2,
  HadronicWDownType = 3,
};

enum class TopologyStatus : int {
  Success = 0,
  MissingTopPair = 1,
  MissingTopDecay = 2,
  NotSemileptonic = 3,
};

enum class RecoMatchSource : int {
  None = 0,
  NanoAODGenJetIndex = 1,
  DeltaRFallback = 2,
};

struct GenParticleInput {
  float pt = 0.f;
  float eta = 0.f;
  float phi = 0.f;
  float mass = 0.f;
  int pdgId = 0;
  int motherIndex = -1;
  int status = 0;
  std::uint16_t statusFlags = 0;
};

struct GenJetInput {
  float pt = 0.f;
  float eta = 0.f;
  float phi = 0.f;
  float mass = 0.f;
  int partonFlavour = 0;
};

struct RecoJetInput {
  float pt = 0.f;
  float eta = 0.f;
  float phi = 0.f;
  float mass = 0.f;
  int selectedIndex = -1;
  int originalIndex = -1;
  int genJetIndex = -1;
};

struct TopJetMatcherConfig {
  float partonGenMaxDeltaR = 0.4f;
  float recoGenIndexMaxDeltaR = 0.4f;
  float recoGenFallbackMaxDeltaR = 0.2f;
  float ptBalanceWeight = 0.10f;
  float absoluteFlavourPenalty = 2.0f;
  float deltaRFallbackPenalty = 0.5f;
  bool allowAbsoluteFlavourFallback = true;
};

struct TopJetMatch {
  TopJetRole role = TopJetRole::HadronicB;
  int genPartIndex = -1;
  int partonPdgId = 0;
  int genJetIndex = -1;
  int recoJetIndex = -1;
  int recoJetOriginalIndex = -1;
  float partonGenJetDeltaR = -1.f;
  float genJetRecoDeltaR = -1.f;
  float cost = -1.f;
  RecoMatchSource recoMatchSource = RecoMatchSource::None;
  bool usedAbsoluteFlavourFallback = false;
};

struct TopJetMatchResult {
  TopologyStatus topologyStatus = TopologyStatus::MissingTopPair;
  int hadronicTopPdgId = 0;
  std::array<TopJetMatch, kNumTopJetRoles> matches{};
  int nGenMatched = 0;
  int nRecoMatched = 0;
  bool complete = false;
  float totalCost = -1.f;
  float assignmentMargin = -1.f;
};

class TopJetMatcher {
public:
  explicit TopJetMatcher(TopJetMatcherConfig config = {});

  TopJetMatchResult Match(const std::vector<GenParticleInput> &genParticles,
                          const std::vector<GenJetInput> &genJets,
                          const std::vector<RecoJetInput> &recoJets) const;

private:
  TopJetMatcherConfig config_;
};

} // namespace HadronAnalysis

#endif
