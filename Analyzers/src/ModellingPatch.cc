#include "ModellingPatch.h"

#include <TLorentzVector.h>
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace {
constexpr unsigned short FIRST_COPY_MASK = 1u << 12;
constexpr unsigned short LAST_COPY_MASK = 1u << 13;
} // namespace

ModellingPatch::ModellingPatch() {}

void ModellingPatch::ensure_sizes(SubprocessSums &sums, std::size_t scale,
                                  std::size_t ps) const {
  if (sums.sumScaleVariation.size() < scale)
    sums.sumScaleVariation.resize(scale, 0.0);
  if (sums.sumPSVariation.size() < ps)
    sums.sumPSVariation.resize(ps, 0.0);
}

std::string ModellingPatch::subprocess_name(int genTtbarId) const {
  if (!isTT_)
    return baseSubprocessName_;

  int mod = genTtbarId % 100;
  if (mod < 0)
    mod += 100;

  if (mod >= 51 && mod <= 55)
    return "BB";
  if (mod >= 41 && mod <= 45)
    return "CC";
  return "JJ";
}

std::array<size_t, 4>
ModellingPatch::GetTopAndAntiTopIndices(const GenViewCollection &gens) const {
  constexpr size_t npos = std::numeric_limits<size_t>::max();

  size_t FirstCopyTopIndex = npos;
  size_t FirstCopyAntiTopIndex = npos;
  size_t LastCopyTopIndex = npos;
  size_t LastCopyAntiTopIndex = npos;

  const size_t n = gens.size();

  for (size_t idx = 0; idx < n; ++idx) {
    const GenView &gen = gens[idx];

    const int pdg = gen.PdgId();
    const auto flags = gen.StatusFlags();

    const bool isFirstCopy = (flags & FIRST_COPY_MASK) != 0;
    const bool isLastCopy = (flags & LAST_COPY_MASK) != 0;

    if (pdg == 6) { // top
      if (isFirstCopy) {
        if (FirstCopyTopIndex != npos)
          throw std::runtime_error("Multiple first-copy tops found in event");
        FirstCopyTopIndex = idx;
      }
      if (isLastCopy) {
        if (LastCopyTopIndex != npos)
          throw std::runtime_error("Multiple last-copy tops found in event");
        LastCopyTopIndex = idx;
      }
    } else if (pdg == -6) { // anti-top
      if (isFirstCopy) {
        if (FirstCopyAntiTopIndex != npos)
          throw std::runtime_error("Multiple first-copy antitops found in event");
        FirstCopyAntiTopIndex = idx;
      }
      if (isLastCopy) {
        if (LastCopyAntiTopIndex != npos)
          throw std::runtime_error("Multiple last-copy antitops found in event");
        LastCopyAntiTopIndex = idx;
      }
    }
  }

  if (FirstCopyTopIndex == npos || FirstCopyAntiTopIndex == npos ||
      LastCopyTopIndex == npos || LastCopyAntiTopIndex == npos) {
    throw std::runtime_error("Missing first/last copy top or antitop");
  }

  return {FirstCopyTopIndex, FirstCopyAntiTopIndex, LastCopyTopIndex,
          LastCopyAntiTopIndex};
}

void ModellingPatch::initializeAnalyzer() {
  baseSubprocessName_ = std::string(MCSample.Data());
  isTT_ = (!IsDATA && baseSubprocessName_.find("TT") != std::string::npos);

  if (!IsDATA) {
    myCorr = new MyCorrection(DataEra, DataPeriod,
                              IsDATA ? DataStream : MCSample, IsDATA);
  }
}

void ModellingPatch::executeEvent() {
  if (IsDATA)
    return;

  const double sign = genWeight > 0.f ? 1.0 : -1.0;
  const int nScale = nLHEScaleWeight;
  const int nPS = nPSWeight;

  const std::string sub = subprocess_name(genTtbarId);
  auto &acc = accumulators_[sub];
  ensure_sizes(acc, nScale, nPS);

  ++acc.nMC;
  acc.sumSign += sign;

  FillHist(sub + "/NEvents", 0.f, 1.f, 1, 0., 1.);
  FillHist(sub + "/sumSign", 0.f, sign, 1, 0., 1.);

  if (nScale > 0 && LHEScaleWeight.valid()) {
    const auto &scales = LHEScaleWeight.values();
    const int limit = std::min<int>(nScale, static_cast<int>(scales.size()));
    for (int i = 0; i < limit; ++i) {
      const double val = sign * static_cast<double>(scales[i]);
      acc.sumScaleVariation[i] += val;
      FillHist(sub + "/sumScaleVariation", static_cast<float>(i), val, nScale,
               0.f, nScale);
    }
  }

  if (nPS > 0 && PSWeight.valid()) {
    const auto &ps = PSWeight.values();
    const int limit = std::min<int>(nPS, static_cast<int>(ps.size()));
    for (int i = 0; i < limit; ++i) {
      const double val = sign * static_cast<double>(ps[i]);
      acc.sumPSVariation[i] += val;
      FillHist(sub + "/sumPSVariation", static_cast<float>(i), val, nPS, 0,
               nPS);
    }
  }

  if (isTT_) {
    GenViewCollection gens = GetAllGenViews();
    auto [firstTopIdx, firstAntiTopIdx, lastTopIdx, lastAntiTopIdx] =
        GetTopAndAntiTopIndices(gens);

    const TLorentzVector FirstCopyTop = gens[firstTopIdx].P4();
    const TLorentzVector FirstCopyAntiTop = gens[firstAntiTopIdx].P4();

    const TLorentzVector LastCopyTop = gens[lastTopIdx].P4();
    const TLorentzVector LastCopyAntiTop = gens[lastAntiTopIdx].P4();

    const double hdamp_up = myCorr->GethDampReweight(FirstCopyTop, FirstCopyAntiTop,
                                                     MyCorrection::variation::up);
    const double hdamp_down =
        myCorr->GethDampReweight(FirstCopyTop, FirstCopyAntiTop,
                                 MyCorrection::variation::down);
    const double minnlo = myCorr->GetTopPtReweight(LastCopyTop, LastCopyAntiTop);

    acc.sum_hdamp_up += sign * hdamp_up;
    acc.sum_hdamp_down += sign * hdamp_down;
    acc.sum_minnlo += sign * minnlo;

    FillHist(sub + "/sum_hdamp_up", 0.f, sign * hdamp_up, 1, 0., 1.);
    FillHist(sub + "/sum_hdamp_down", 0.f, sign * hdamp_down, 1, 0., 1.);
    FillHist(sub + "/sum_minnlo", 0.f, sign * minnlo, 1, 0., 1.);

    // b-fragmentation
    double bfrag_nom = 1.0;
    double bfrag_up = 1.0;
    try {
      auto idx = myCorr->GetGenIdxofTopDecayProducts(gens);
      constexpr std::size_t npos = std::numeric_limits<std::size_t>::max();
      bool missing_hadron = false;
      for (auto v : idx) {
        if (v == npos) {
          missing_hadron = true;
          break;
        }
      }
      if (!missing_hadron) {
        const TLorentzVector LastCopyTopB = gens[idx[0]].P4();
        const TLorentzVector LastCopyAntiTopB = gens[idx[3]].P4();
        const TLorentzVector LastCopyWPlus = gens[idx[1]].P4();
        const TLorentzVector LastCopyWMinus = gens[idx[4]].P4();
        const TLorentzVector FirstCopyBHadTop = gens[idx[2]].P4();
        const TLorentzVector FirstCopyBHadAntiTop = gens[idx[5]].P4();

        bfrag_nom = myCorr->GetBFragReweight(
            LastCopyTopB, LastCopyAntiTopB, LastCopyWPlus, LastCopyWMinus,
            FirstCopyBHadTop, FirstCopyBHadAntiTop,
            MyCorrection::variation::nom);
        bfrag_up = myCorr->GetBFragReweight(
            LastCopyTopB, LastCopyAntiTopB, LastCopyWPlus, LastCopyWMinus,
            FirstCopyBHadTop, FirstCopyBHadAntiTop,
            MyCorrection::variation::up);
      }
    } catch (const std::exception &) {
      bfrag_nom = 1.0;
      bfrag_up = 1.0;
    }

    acc.sum_bfrag_nom += sign * bfrag_nom;
    acc.sum_bfrag_up += sign * bfrag_up;
    FillHist(sub + "/sum_bfrag_nom", 0.f, sign * bfrag_nom, 1, 0., 1.);
    FillHist(sub + "/sum_bfrag_up", 0.f, sign * bfrag_up, 1, 0., 1.);
  }
}
