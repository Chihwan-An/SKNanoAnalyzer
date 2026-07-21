#include "TopJetMatcher.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

namespace HadronAnalysis {
namespace {

constexpr std::uint16_t kIsHardProcess = 1U << 7;
constexpr std::uint16_t kFromHardProcess = 1U << 8;
constexpr std::uint16_t kFromHardProcessBeforeFSR = 1U << 11;
constexpr std::uint16_t kIsLastCopy = 1U << 13;
constexpr std::uint16_t kIsLastCopyBeforeFSR = 1U << 14;
constexpr float kCostEpsilon = 1.e-6f;

bool HasFlag(const GenParticleInput &particle, std::uint16_t flag) {
  return (particle.statusFlags & flag) != 0;
}

int Sign(int value) { return (value > 0) - (value < 0); }

bool IsUpTypeQuark(int pdgId) {
  const int absolute = std::abs(pdgId);
  return absolute == 2 || absolute == 4;
}

bool IsDownTypeQuark(int pdgId) {
  const int absolute = std::abs(pdgId);
  return absolute == 1 || absolute == 3 || absolute == 5;
}

bool IsChargedLepton(int pdgId) {
  const int absolute = std::abs(pdgId);
  return absolute == 11 || absolute == 13 || absolute == 15;
}

bool IsNeutrino(int pdgId) {
  const int absolute = std::abs(pdgId);
  return absolute == 12 || absolute == 14 || absolute == 16;
}

float DeltaPhi(float first, float second) {
  constexpr float pi = 3.14159265358979323846f;
  constexpr float twoPi = 2.f * pi;
  float result = std::remainder(first - second, twoPi);
  if (result <= -pi)
    result += twoPi;
  return result;
}

template <typename First, typename Second>
float DeltaR(const First &first, const Second &second) {
  const float deltaEta = first.eta - second.eta;
  const float deltaPhi = DeltaPhi(first.phi, second.phi);
  return std::sqrt(deltaEta * deltaEta + deltaPhi * deltaPhi);
}

float PtBalanceCost(float firstPt, float secondPt, float weight) {
  if (firstPt <= 0.f || secondPt <= 0.f)
    return 0.f;
  return weight * std::abs(std::log(firstPt / secondPt));
}

class TruthGraph {
public:
  explicit TruthGraph(const std::vector<GenParticleInput> &particles)
      : particles_(particles), children_(particles.size()) {
    for (std::size_t index = 0; index < particles_.size(); ++index) {
      const int mother = particles_[index].motherIndex;
      if (IsValid(mother) && mother != static_cast<int>(index))
        children_[mother].push_back(static_cast<int>(index));
    }
  }

  bool IsValid(int index) const {
    return index >= 0 && static_cast<std::size_t>(index) < particles_.size();
  }

  std::vector<int> Family(int seed) const {
    std::vector<int> family;
    if (!IsValid(seed))
      return family;

    const int pdgId = particles_[seed].pdgId;
    std::vector<unsigned char> visited(particles_.size(), 0);
    std::vector<int> pending{seed};
    visited[seed] = 1;
    while (!pending.empty()) {
      const int current = pending.back();
      pending.pop_back();
      family.push_back(current);

      const int mother = particles_[current].motherIndex;
      if (IsValid(mother) && !visited[mother] &&
          particles_[mother].pdgId == pdgId) {
        visited[mother] = 1;
        pending.push_back(mother);
      }
      for (const int child : children_[current]) {
        if (!visited[child] && particles_[child].pdgId == pdgId) {
          visited[child] = 1;
          pending.push_back(child);
        }
      }
    }
    std::sort(family.begin(), family.end());
    return family;
  }

  std::vector<int> DecayChildren(const std::vector<int> &family) const {
    std::vector<int> result;
    if (family.empty())
      return result;
    const int familyPdgId = particles_[family.front()].pdgId;
    for (const int member : family) {
      for (const int child : children_[member]) {
        if (particles_[child].pdgId != familyPdgId)
          result.push_back(child);
      }
    }
    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());
    return result;
  }

  int BestCopy(int seed) const {
    const auto family = Family(seed);
    if (family.empty())
      return -1;
    return *std::max_element(family.begin(), family.end(),
                             [&](int first, int second) {
                               return CopyScore(first) < CopyScore(second);
                             });
  }

  std::tuple<int, float, int> CopyScore(int index) const {
    const auto &particle = particles_[index];
    int score = 0;
    if (HasFlag(particle, kIsLastCopyBeforeFSR))
      score += 400;
    if (HasFlag(particle, kIsLastCopy))
      score += 200;
    if (HasFlag(particle, kFromHardProcessBeforeFSR))
      score += 80;
    if (HasFlag(particle, kFromHardProcess))
      score += 40;
    if (HasFlag(particle, kIsHardProcess))
      score += 20;
    if (particle.status == 23)
      score += 4;

    const bool hasSelfChild = std::any_of(
        children_[index].begin(), children_[index].end(),
        [&](int child) { return particles_[child].pdgId == particle.pdgId; });
    if (!hasSelfChild)
      score += 10;
    return {score, particle.pt, -index};
  }

  const GenParticleInput &operator[](int index) const {
    return particles_[index];
  }

  std::size_t size() const { return particles_.size(); }

private:
  const std::vector<GenParticleInput> &particles_;
  std::vector<std::vector<int>> children_;
};

struct TopDecay {
  int topPdgId = 0;
  int b = -1;
  int w = -1;
  int up = -1;
  int down = -1;
  bool valid = false;
  bool hadronic = false;
  bool leptonic = false;
  std::tuple<int, float, int> score{0, 0.f, 0};
};

int BestSeed(const TruthGraph &graph, const std::vector<int> &seeds) {
  if (seeds.empty())
    return -1;
  return *std::max_element(seeds.begin(), seeds.end(),
                           [&](int first, int second) {
                             return graph.CopyScore(graph.BestCopy(first)) <
                                    graph.CopyScore(graph.BestCopy(second));
                           });
}

TopDecay ParseTopDecay(const TruthGraph &graph, int topSeed) {
  TopDecay result;
  result.topPdgId = graph[topSeed].pdgId;
  result.score = graph.CopyScore(graph.BestCopy(topSeed));
  const int topSign = Sign(result.topPdgId);
  const auto topChildren = graph.DecayChildren(graph.Family(topSeed));

  std::vector<int> bSeeds;
  std::vector<int> wSeeds;
  for (const int child : topChildren) {
    if (graph[child].pdgId == topSign * 5)
      bSeeds.push_back(child);
    if (graph[child].pdgId == topSign * 24)
      wSeeds.push_back(child);
  }
  const int bSeed = BestSeed(graph, bSeeds);
  const int wSeed = BestSeed(graph, wSeeds);
  if (bSeed < 0 || wSeed < 0)
    return result;

  result.b = graph.BestCopy(bSeed);
  result.w = graph.BestCopy(wSeed);
  const int wSign = Sign(graph[wSeed].pdgId);
  const auto wChildren = graph.DecayChildren(graph.Family(wSeed));
  std::vector<int> upSeeds;
  std::vector<int> downSeeds;
  std::vector<int> chargedLeptons;
  std::vector<int> neutrinos;
  for (const int child : wChildren) {
    const int pdgId = graph[child].pdgId;
    if (IsUpTypeQuark(pdgId) && Sign(pdgId) == wSign)
      upSeeds.push_back(child);
    if (IsDownTypeQuark(pdgId) && Sign(pdgId) == -wSign)
      downSeeds.push_back(child);
    if (IsChargedLepton(pdgId) && Sign(pdgId) == -wSign)
      chargedLeptons.push_back(child);
    if (IsNeutrino(pdgId) && Sign(pdgId) == wSign)
      neutrinos.push_back(child);
  }

  result.hadronic = !upSeeds.empty() && !downSeeds.empty();
  result.leptonic = !chargedLeptons.empty() && !neutrinos.empty();
  result.valid = result.hadronic != result.leptonic;
  if (result.hadronic) {
    result.up = graph.BestCopy(BestSeed(graph, upSeeds));
    result.down = graph.BestCopy(BestSeed(graph, downSeeds));
  }
  return result;
}

std::vector<TopDecay> FindTopDecays(const TruthGraph &graph, int sign) {
  std::vector<TopDecay> result;
  std::vector<int> representatives;
  for (std::size_t index = 0; index < graph.size(); ++index) {
    if (graph[static_cast<int>(index)].pdgId != sign * 6)
      continue;
    const auto family = graph.Family(static_cast<int>(index));
    if (family.empty() ||
        std::find(representatives.begin(), representatives.end(),
                  family.front()) != representatives.end())
      continue;
    representatives.push_back(family.front());
    result.push_back(ParseTopDecay(graph, family.front()));
  }
  return result;
}

struct TruthRoles {
  TopologyStatus status = TopologyStatus::MissingTopPair;
  int hadronicTopPdgId = 0;
  std::array<int, kNumTopJetRoles> partons{{-1, -1, -1, -1}};
};

TruthRoles ResolveTruthRoles(const std::vector<GenParticleInput> &particles) {
  TruthRoles result;
  const TruthGraph graph(particles);
  const auto tops = FindTopDecays(graph, 1);
  const auto antitops = FindTopDecays(graph, -1);
  if (tops.empty() || antitops.empty())
    return result;

  const bool topHasDecay =
      std::any_of(tops.begin(), tops.end(), [](const TopDecay &decay) {
        return decay.b >= 0 && decay.w >= 0;
      });
  const bool antitopHasDecay =
      std::any_of(antitops.begin(), antitops.end(), [](const TopDecay &decay) {
        return decay.b >= 0 && decay.w >= 0;
      });
  if (!topHasDecay || !antitopHasDecay) {
    result.status = TopologyStatus::MissingTopDecay;
    return result;
  }

  const TopDecay *bestTop = nullptr;
  const TopDecay *bestAntitop = nullptr;
  std::tuple<int, float, int, int, float, int> bestScore{
      std::numeric_limits<int>::min(), 0.f, 0,
      std::numeric_limits<int>::min(), 0.f, 0};
  for (const auto &top : tops) {
    for (const auto &antitop : antitops) {
      if (!top.valid || !antitop.valid || top.hadronic == antitop.hadronic)
        continue;
      const auto score = std::tuple_cat(top.score, antitop.score);
      if (!bestTop || score > bestScore) {
        bestTop = &top;
        bestAntitop = &antitop;
        bestScore = score;
      }
    }
  }
  if (!bestTop) {
    result.status = TopologyStatus::NotSemileptonic;
    return result;
  }

  const TopDecay &hadronic = bestTop->hadronic ? *bestTop : *bestAntitop;
  const TopDecay &leptonic = bestTop->leptonic ? *bestTop : *bestAntitop;
  result.status = TopologyStatus::Success;
  result.hadronicTopPdgId = hadronic.topPdgId;
  result.partons = {hadronic.b, leptonic.b, hadronic.up, hadronic.down};
  return result;
}

struct AssignmentOption {
  int genJet = -1;
  int recoJet = -1;
  float partonGenDeltaR = -1.f;
  float genRecoDeltaR = -1.f;
  float cost = 0.f;
  RecoMatchSource source = RecoMatchSource::None;
  bool absoluteFlavourFallback = false;
};

struct Assignment {
  std::array<AssignmentOption, kNumTopJetRoles> options{};
  int nGen = -1;
  int nReco = -1;
  float cost = std::numeric_limits<float>::infinity();
};

std::array<int, 2 * kNumTopJetRoles> Signature(const Assignment &assignment) {
  std::array<int, 2 * kNumTopJetRoles> result{};
  for (std::size_t role = 0; role < kNumTopJetRoles; ++role) {
    result[2 * role] = assignment.options[role].genJet;
    result[2 * role + 1] = assignment.options[role].recoJet;
  }
  return result;
}

bool BetterCardinality(const Assignment &first, const Assignment &second) {
  return first.nReco > second.nReco ||
         (first.nReco == second.nReco && first.nGen > second.nGen);
}

struct AssignmentSearchResult {
  Assignment best;
  float secondBestCost = std::numeric_limits<float>::infinity();
};

AssignmentSearchResult FindBestAssignment(
    const std::array<std::vector<AssignmentOption>, kNumTopJetRoles> &options,
    std::size_t nGenJets, std::size_t nRecoJets) {
  std::array<int, kNumTopJetRoles> order{{0, 1, 2, 3}};
  std::sort(order.begin(), order.end(), [&](int first, int second) {
    return options[first].size() < options[second].size();
  });

  std::array<int, kNumTopJetRoles + 1> suffixCanGen{};
  std::array<int, kNumTopJetRoles + 1> suffixCanReco{};
  for (int depth = static_cast<int>(kNumTopJetRoles) - 1; depth >= 0; --depth) {
    const auto &roleOptions = options[order[depth]];
    suffixCanGen[depth] = suffixCanGen[depth + 1] +
                          std::any_of(roleOptions.begin(), roleOptions.end(),
                                      [](const AssignmentOption &option) {
                                        return option.genJet >= 0;
                                      });
    suffixCanReco[depth] = suffixCanReco[depth + 1] +
                           std::any_of(roleOptions.begin(), roleOptions.end(),
                                       [](const AssignmentOption &option) {
                                         return option.recoJet >= 0;
                                       });
  }

  AssignmentSearchResult result;
  Assignment current;
  current.nGen = 0;
  current.nReco = 0;
  current.cost = 0.f;
  std::vector<unsigned char> usedGen(nGenJets, 0);
  std::vector<unsigned char> usedReco(nRecoJets, 0);

  auto visit = [&](auto &&self, int depth) -> void {
    if (result.best.nReco >= 0) {
      const int recoUpper = current.nReco + suffixCanReco[depth];
      const int genUpper = current.nGen + suffixCanGen[depth];
      if (recoUpper < result.best.nReco ||
          (recoUpper == result.best.nReco && genUpper < result.best.nGen))
        return;
    }
    if (depth == static_cast<int>(kNumTopJetRoles)) {
      if (BetterCardinality(current, result.best)) {
        result.best = current;
        result.secondBestCost = std::numeric_limits<float>::infinity();
        return;
      }
      if (current.nReco != result.best.nReco ||
          current.nGen != result.best.nGen)
        return;

      const bool lowerCost = current.cost < result.best.cost - kCostEpsilon;
      const bool tiedButDeterministic =
          std::abs(current.cost - result.best.cost) <= kCostEpsilon &&
          Signature(current) < Signature(result.best);
      if (lowerCost || tiedButDeterministic) {
        result.secondBestCost =
            std::min(result.secondBestCost, result.best.cost);
        result.best = current;
      } else {
        result.secondBestCost = std::min(result.secondBestCost, current.cost);
      }
      return;
    }

    const int role = order[depth];
    for (const auto &option : options[role]) {
      if (option.genJet >= 0 && usedGen[option.genJet])
        continue;
      if (option.recoJet >= 0 && usedReco[option.recoJet])
        continue;

      current.options[role] = option;
      if (option.genJet >= 0) {
        usedGen[option.genJet] = 1;
        ++current.nGen;
      }
      if (option.recoJet >= 0) {
        usedReco[option.recoJet] = 1;
        ++current.nReco;
      }
      current.cost += option.cost;
      self(self, depth + 1);
      current.cost -= option.cost;
      if (option.recoJet >= 0) {
        --current.nReco;
        usedReco[option.recoJet] = 0;
      }
      if (option.genJet >= 0) {
        --current.nGen;
        usedGen[option.genJet] = 0;
      }
    }
  };
  visit(visit, 0);
  return result;
}

} // namespace

TopJetMatcher::TopJetMatcher(TopJetMatcherConfig config)
    : config_(std::move(config)) {
  if (!(config_.partonGenMaxDeltaR > 0.f) ||
      !(config_.recoGenIndexMaxDeltaR > 0.f) ||
      !(config_.recoGenFallbackMaxDeltaR > 0.f) ||
      !(config_.ptBalanceWeight >= 0.f) ||
      !(config_.absoluteFlavourPenalty >= 0.f) ||
      !(config_.deltaRFallbackPenalty >= 0.f))
    throw std::invalid_argument("TopJetMatcher requires positive radii and "
                                "non-negative cost weights");
}

TopJetMatchResult
TopJetMatcher::Match(const std::vector<GenParticleInput> &genParticles,
                     const std::vector<GenJetInput> &genJets,
                     const std::vector<RecoJetInput> &recoJets) const {
  TopJetMatchResult result;
  for (std::size_t role = 0; role < kNumTopJetRoles; ++role)
    result.matches[role].role = static_cast<TopJetRole>(role);

  const TruthRoles truth = ResolveTruthRoles(genParticles);
  result.topologyStatus = truth.status;
  result.hadronicTopPdgId = truth.hadronicTopPdgId;
  if (truth.status != TopologyStatus::Success)
    return result;

  std::array<std::vector<AssignmentOption>, kNumTopJetRoles> roleOptions;
  for (std::size_t role = 0; role < kNumTopJetRoles; ++role) {
    const int partonIndex = truth.partons[role];
    result.matches[role].genPartIndex = partonIndex;
    result.matches[role].partonPdgId = genParticles[partonIndex].pdgId;
    const auto &parton = genParticles[partonIndex];

    std::vector<std::pair<int, float>> signedCandidates;
    std::vector<std::pair<int, float>> absoluteCandidates;
    for (std::size_t genJetIndex = 0; genJetIndex < genJets.size();
         ++genJetIndex) {
      const float deltaR = DeltaR(parton, genJets[genJetIndex]);
      if (deltaR >= config_.partonGenMaxDeltaR)
        continue;
      if (genJets[genJetIndex].partonFlavour == parton.pdgId)
        signedCandidates.emplace_back(static_cast<int>(genJetIndex), deltaR);
      else if (config_.allowAbsoluteFlavourFallback &&
               std::abs(genJets[genJetIndex].partonFlavour) ==
                   std::abs(parton.pdgId))
        absoluteCandidates.emplace_back(static_cast<int>(genJetIndex), deltaR);
    }
    const bool useAbsoluteFlavour = signedCandidates.empty();
    const auto &genCandidates =
        useAbsoluteFlavour ? absoluteCandidates : signedCandidates;

    for (const auto &[genJetIndex, partonGenDeltaR] : genCandidates) {
      const auto &genJet = genJets[genJetIndex];
      float genCost =
          partonGenDeltaR / config_.partonGenMaxDeltaR +
          PtBalanceCost(genJet.pt, parton.pt, config_.ptBalanceWeight);
      if (useAbsoluteFlavour)
        genCost += config_.absoluteFlavourPenalty;

      std::vector<std::pair<int, float>> exactRecoCandidates;
      for (std::size_t recoIndex = 0; recoIndex < recoJets.size();
           ++recoIndex) {
        if (recoJets[recoIndex].genJetIndex != genJetIndex)
          continue;
        const float deltaR = DeltaR(genJet, recoJets[recoIndex]);
        if (deltaR < config_.recoGenIndexMaxDeltaR)
          exactRecoCandidates.emplace_back(static_cast<int>(recoIndex), deltaR);
      }

      if (!exactRecoCandidates.empty()) {
        for (const auto &[recoIndex, genRecoDeltaR] : exactRecoCandidates) {
          const float recoCost =
              genRecoDeltaR / config_.recoGenIndexMaxDeltaR +
              PtBalanceCost(recoJets[recoIndex].pt, genJet.pt,
                            config_.ptBalanceWeight);
          roleOptions[role].push_back({genJetIndex, recoIndex, partonGenDeltaR,
                                       genRecoDeltaR, genCost + recoCost,
                                       RecoMatchSource::NanoAODGenJetIndex,
                                       useAbsoluteFlavour});
        }
      } else {
        for (std::size_t recoIndex = 0; recoIndex < recoJets.size();
             ++recoIndex) {
          if (recoJets[recoIndex].genJetIndex >= 0)
            continue;
          const float genRecoDeltaR = DeltaR(genJet, recoJets[recoIndex]);
          if (genRecoDeltaR >= config_.recoGenFallbackMaxDeltaR)
            continue;
          const float recoCost =
              genRecoDeltaR / config_.recoGenFallbackMaxDeltaR +
              PtBalanceCost(recoJets[recoIndex].pt, genJet.pt,
                            config_.ptBalanceWeight) +
              config_.deltaRFallbackPenalty;
          roleOptions[role].push_back(
              {genJetIndex, static_cast<int>(recoIndex), partonGenDeltaR,
               genRecoDeltaR, genCost + recoCost,
               RecoMatchSource::DeltaRFallback, useAbsoluteFlavour});
        }
      }
      roleOptions[role].push_back({genJetIndex, -1, partonGenDeltaR, -1.f,
                                   genCost, RecoMatchSource::None,
                                   useAbsoluteFlavour});
    }
    roleOptions[role].push_back(AssignmentOption{});
    std::stable_sort(
        roleOptions[role].begin(), roleOptions[role].end(),
        [](const AssignmentOption &first, const AssignmentOption &second) {
          const auto firstRank =
              std::make_tuple(first.recoJet < 0, first.genJet < 0, first.cost,
                              first.genJet, first.recoJet);
          const auto secondRank =
              std::make_tuple(second.recoJet < 0, second.genJet < 0,
                              second.cost, second.genJet, second.recoJet);
          return firstRank < secondRank;
        });
  }

  const auto assignment =
      FindBestAssignment(roleOptions, genJets.size(), recoJets.size());
  result.nGenMatched = assignment.best.nGen;
  result.nRecoMatched = assignment.best.nReco;
  result.complete = result.nRecoMatched == static_cast<int>(kNumTopJetRoles);
  result.totalCost = result.nGenMatched > 0 ? assignment.best.cost : -1.f;
  if (std::isfinite(assignment.secondBestCost))
    result.assignmentMargin =
        std::max(0.f, assignment.secondBestCost - assignment.best.cost);

  for (std::size_t role = 0; role < kNumTopJetRoles; ++role) {
    const auto &option = assignment.best.options[role];
    auto &match = result.matches[role];
    match.genJetIndex = option.genJet;
    match.recoJetIndex =
        option.recoJet < 0 ? -1 : recoJets[option.recoJet].selectedIndex;
    match.recoJetOriginalIndex =
        option.recoJet < 0 ? -1 : recoJets[option.recoJet].originalIndex;
    match.partonGenJetDeltaR = option.partonGenDeltaR;
    match.genJetRecoDeltaR = option.genRecoDeltaR;
    match.cost = option.genJet < 0 ? -1.f : option.cost;
    match.recoMatchSource = option.source;
    match.usedAbsoluteFlavourFallback = option.absoluteFlavourFallback;
  }
  return result;
}

} // namespace HadronAnalysis
