#include "LambdaCTruthMatcher.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <vector>

namespace HadronAnalysis {
namespace {

// Values select the species ordered as proton, kaon, pion for each reco track.
constexpr std::array<std::array<std::size_t, 3>, 6> kAssignments = {{
    {0, 1, 2},
    {0, 2, 1},
    {1, 0, 2},
    {2, 0, 1},
    {1, 2, 0},
    {2, 1, 0},
}};

constexpr std::array<int, 3> kSpeciesAbsPdgIds = {2212, 321, 211};
constexpr float kPi = 3.14159265358979323846f;

bool IsValidIndex(int index, std::size_t size) {
  return index >= 0 && static_cast<std::size_t>(index) < size;
}

float DeltaPhi(float first, float second) {
  float result = std::remainder(first - second, 2.f * kPi);
  if (result <= -kPi)
    result += 2.f * kPi;
  return result;
}

int UnitCharge(int pdgId) { return (pdgId > 0) - (pdgId < 0); }

bool IsAcceptedDaughter(int pdgId) {
  const int absolute = std::abs(pdgId);
  return std::find(kSpeciesAbsPdgIds.begin(), kSpeciesAbsPdgIds.end(),
                   absolute) != kSpeciesAbsPdgIds.end();
}

void CollectTerminalDaughters(
    int index, const std::vector<LambdaCGenParticleInput> &particles,
    const std::vector<std::vector<int>> &children,
    std::vector<unsigned char> &seen, std::vector<int> &daughters,
    unsigned int depth) {
  if (!IsValidIndex(index, particles.size()) || depth > 64 || seen[index])
    return;
  seen[index] = 1;

  const auto &particle = particles[index];
  if (particle.status == 1 || children[index].empty()) {
    if (IsAcceptedDaughter(particle.pdgId))
      daughters.push_back(index);
    return;
  }
  for (const int child : children[index])
    CollectTerminalDaughters(child, particles, children, seen, daughters,
                             depth + 1);
}

bool MatchCost(const LambdaCRecoTrackInput &track,
               const LambdaCGenParticleInput &particle,
               const LambdaCTruthMatchConfig &config, float &cost) {
  if (track.charge != UnitCharge(particle.pdgId) || !(particle.pt > 0.f))
    return false;
  const float deltaEta = track.eta - particle.eta;
  const float deltaPhi = DeltaPhi(track.phi, particle.phi);
  const float deltaR = std::sqrt(deltaEta * deltaEta + deltaPhi * deltaPhi);
  const float relativePt = std::abs(track.pt - particle.pt) / particle.pt;
  if (!(deltaR < config.daughterMaxDeltaR) ||
      !(relativePt < config.daughterMaxRelativePt))
    return false;
  cost = deltaR * deltaR + relativePt * relativePt;
  return true;
}

} // namespace

LambdaCTruthMatcher::LambdaCTruthMatcher(LambdaCTruthMatchConfig config)
    : config_(config) {
  if (!(config_.daughterMaxDeltaR > 0.f) ||
      !(config_.daughterMaxRelativePt > 0.f))
    throw std::invalid_argument(
        "LambdaCTruthMatcher matching thresholds must be positive");
}

LambdaCTruthHypothesisResult LambdaCTruthMatcher::Match(
    int parentIndex, const std::array<LambdaCRecoTrackInput, 3> &tracks,
    const std::vector<LambdaCGenParticleInput> &particles) const {
  LambdaCTruthHypothesisResult result;
  if (!IsValidIndex(parentIndex, particles.size()) ||
      std::abs(particles[parentIndex].pdgId) != 4122)
    return result;

  std::vector<std::vector<int>> children(particles.size());
  for (std::size_t index = 0; index < particles.size(); ++index) {
    const int mother = particles[index].motherIndex;
    if (IsValidIndex(mother, particles.size()) &&
        mother != static_cast<int>(index))
      children[mother].push_back(static_cast<int>(index));
  }

  std::vector<unsigned char> seen(particles.size(), 0);
  seen[parentIndex] = 1;
  std::vector<int> daughters;
  for (const int child : children[parentIndex])
    CollectTerminalDaughters(child, particles, children, seen, daughters, 0);
  if (daughters.size() != 3)
    return result;

  std::array<int, 3> daughterBySpecies = {-1, -1, -1};
  for (const int daughter : daughters) {
    const int absolute = std::abs(particles[daughter].pdgId);
    const auto species = std::find(kSpeciesAbsPdgIds.begin(),
                                   kSpeciesAbsPdgIds.end(), absolute);
    if (species == kSpeciesAbsPdgIds.end())
      return result;
    const std::size_t speciesIndex =
        static_cast<std::size_t>(species - kSpeciesAbsPdgIds.begin());
    if (daughterBySpecies[speciesIndex] >= 0)
      return result;
    daughterBySpecies[speciesIndex] = daughter;
  }
  if (std::any_of(daughterBySpecies.begin(), daughterBySpecies.end(),
                  [](int index) { return index < 0; }))
    return result;

  float bestCost = std::numeric_limits<float>::infinity();
  for (std::size_t hypothesis = 0; hypothesis < kAssignments.size();
       ++hypothesis) {
    float totalCost = 0.f;
    bool matches = true;
    for (std::size_t track = 0; track < tracks.size(); ++track) {
      float trackCost = 0.f;
      const int daughter = daughterBySpecies[kAssignments[hypothesis][track]];
      if (!MatchCost(tracks[track], particles[daughter], config_, trackCost)) {
        matches = false;
        break;
      }
      totalCost += trackCost;
    }
    if (!matches)
      continue;

    result.mask |= std::uint8_t{1} << hypothesis;
    if (totalCost < bestCost) {
      bestCost = totalCost;
      result.bestIndex = static_cast<int>(hypothesis);
    }
  }
  return result;
}

} // namespace HadronAnalysis
