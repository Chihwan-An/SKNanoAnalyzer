#include "MyCorrection.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "TRandom3.h"

namespace {

// ---------------------------------------------------------------------------
// Generalized Endpoint curvature bias kappa_b, in 1/TeV, with its uncertainty.
//
// Above ~200 GeV the momentum comes largely from the muon system, so the scale
// bias measured at the Z peak from tracker information alone does not describe
// it. The GE method injects additive biases into simulation and picks the one
// that reproduces data in q/pT. Source: MUO POG "High pT: Momentum Scale"
// (methodology in AN-2018/008).
//
// Grid is [phi bin][eta bin]. Phi bins are [-180,-60), [-60,60), [60,180]
// degrees; eta bins are [-2.4,-2.1), [-2.1,-1.2), [-1.2,0), [0,1.2), [1.2,2.1),
// [2.1,2.4]. Eta keeps its sign here -- do not fold to |eta|.
//
// 2024 and 2025 are deliberately absent: those measurements use a different
// binning (docs/highptmuons.pdf) and are not settled. Eras without a map are
// left uncorrected rather than treated as an error.
// ---------------------------------------------------------------------------
struct GECell {
  float kappa;
  float sigma;
};

// Binning is per era rather than fixed: the published 2022-2023 maps are
// 3 phi x 6 eta, while the 2024/2025 measurement uses a finer grid. Edges are
// the inner boundaries; a value below the first edge lands in bin 0 and one
// above the last edge in the final bin. Cells are indexed [phi][eta].
struct GEMap {
  std::vector<float> phiEdgesDeg; // inner boundaries in degrees
  std::vector<float> etaEdges;    // inner boundaries, signed eta
  std::vector<std::vector<GECell>> cells;

  const GECell *find(const float eta, const float phi) const {
    if (std::fabs(eta) > 2.4f)
      return nullptr;
    const float phiDeg = phi * 180.f / static_cast<float>(M_PI);
    std::size_t iphi = 0;
    while (iphi < phiEdgesDeg.size() && phiDeg >= phiEdgesDeg[iphi])
      ++iphi;
    std::size_t ieta = 0;
    while (ieta < etaEdges.size() && eta >= etaEdges[ieta])
      ++ieta;
    if (iphi >= cells.size() || ieta >= cells[iphi].size())
      return nullptr;
    return &cells[iphi][ieta];
  }
};

// Published grid for 2022-2023BPix: phi in [-180,-60), [-60,60), [60,180]
// degrees and eta in [-2.4,-2.1), [-2.1,-1.2), [-1.2,0), [0,1.2), [1.2,2.1),
// [2.1,2.4]. Eta keeps its sign -- do not fold to |eta|.
const std::vector<float> kGePhiEdges3 = {-60.f, 60.f};
const std::vector<float> kGeEtaEdges6 = {-2.1f, -1.2f, 0.f, 1.2f, 2.1f};

const std::unordered_map<std::string, GEMap> kGeKappa = {
    {"2022",
     {kGePhiEdges3, kGeEtaEdges6,
      {
          {{-0.16f, 0.1f}, {-0.03f, 0.05f}, {-0.05f, 0.04f}, {0.f, 0.04f}, {0.07f, 0.06f}, {-0.06f, 0.11f}},
          {{0.11f, 0.1f}, {-0.01f, 0.06f}, {0.06f, 0.04f}, {0.02f, 0.03f}, {0.05f, 0.05f}, {-0.06f, 0.1f}},
          {{0.17f, 0.11f}, {0.16f, 0.04f}, {-0.04f, 0.04f}, {-0.01f, 0.03f}, {0.04f, 0.06f}, {-0.f, 0.09f}},
      }}},
    {"2022EE",
     {kGePhiEdges3, kGeEtaEdges6,
      {
          {{-0.12f, 0.05f}, {-0.03f, 0.03f}, {0.013f, 0.022f}, {0.029f, 0.023f}, {-0.04f, 0.03f}, {-0.28f, 0.05f}},
          {{0.24f, 0.05f}, {0.1f, 0.03f}, {-0.006f, 0.022f}, {-0.047f, 0.022f}, {-0.14f, 0.03f}, {-0.48f, 0.05f}},
          {{0.28f, 0.05f}, {0.07f, 0.03f}, {-0.028f, 0.022f}, {0.018f, 0.022f}, {0.07f, 0.03f}, {0.05f, 0.05f}},
      }}},
    {"2023",
     {kGePhiEdges3, kGeEtaEdges6,
      {
          {{-0.21f, 0.06f}, {-0.01f, 0.04f}, {0.01f, 0.027f}, {-0.045f, 0.03f}, {0.01f, 0.04f}, {0.f, 0.07f}},
          {{0.08f, 0.07f}, {-0.04f, 0.04f}, {0.07f, 0.025f}, {0.03f, 0.027f}, {-0.13f, 0.04f}, {-0.36f, 0.06f}},
          {{0.27f, 0.07f}, {0.05f, 0.04f}, {0.054f, 0.026f}, {0.02f, 0.027f}, {0.f, 0.04f}, {-0.04f, 0.06f}},
      }}},
    {"2023BPix",
     {kGePhiEdges3, kGeEtaEdges6,
      {
          {{-0.25f, 0.08f}, {0.07f, 0.05f}, {0.02f, 0.04f}, {-0.02f, 0.04f}, {0.08f, 0.06f}, {0.12f, 0.09f}},
          {{-0.05f, 0.08f}, {0.f, 0.05f}, {0.05f, 0.03f}, {-0.05f, 0.04f}, {-0.17f, 0.05f}, {-0.33f, 0.09f}},
          {{0.24f, 0.09f}, {0.09f, 0.05f}, {-0.01f, 0.03f}, {0.03f, 0.04f}, {0.04f, 0.06f}, {-0.2f, 0.07f}},
      }}},
};

// ---------------------------------------------------------------------------
// High-pT muon momentum resolution.
//
// The width is a cubic in the FULL momentum p, not pT: the measurement fits
// (1/p - 1/p_gen)/(1/p_gen) in bins of p. Source: Schulte & Zhong, "High-pT
// muon resolution measurement for 2022 and 2023" (2024-06-03).
// Index [0] = barrel |eta| < 1.2, [1] = forward 1.2 - 2.4.
//
// 2024 is absent: the POG has that measurement in progress and currently asks
// analyses to evaluate the non-closure in their own phase space. With no entry
// the nominal smearing is a no-op and only the uncertainty remains.
// ---------------------------------------------------------------------------
struct MuonResPoly {
  float a0, a1, a2, a3;
};
using MuonResSet = std::array<MuonResPoly, 2>;

const std::unordered_map<std::string, MuonResSet> kMuonResPoly = {
    {"2022",
     {{{0.01152f, 5.95e-5f, -2.92e-8f, 5.14e-12f},
       {0.01405f, 5.28e-5f, -1.90e-8f, 3.01e-12f}}}},
    {"2022EE",
     {{{0.0126f, 5.89e-5f, -2.85e-8f, 4.92e-12f},
       {0.0150f, 4.81e-5f, -1.42e-8f, 1.95e-12f}}}},
    {"2023",
     {{{0.0172f, 6.15e-5f, -3.14e-8f, 5.82e-12f},
       {0.01424f, 5.31e-5f, -1.92e-8f, 3.21e-12f}}}},
    {"2023BPix",
     {{{0.0118f, 6.14e-5f, -3.12e-8f, 5.74e-12f},
       {0.0141f, 5.38e-5f, -2.00e-8f, 3.42e-12f}}}},
};

// Extra smearing strength f = sqrt(smearfac^2 - 1): 10% -> 0.458, 5% -> 0.320.
// f = 0 means simulation already matches data in that era and eta region.
const std::unordered_map<std::string, std::array<float, 2>> kMuonSmearF = {
    {"2022", {{0.000f, 0.320f}}},
    {"2022EE", {{0.320f, 0.460f}}},
    {"2023", {{0.320f, 0.460f}}},
    {"2023BPix", {{0.000f, 0.568f}}},
};

// The systematic is a flat ten percent regardless of era and eta.
constexpr float kMuonSmearSystF = 0.4583f;

int MuonResEtaBin(const float eta) { return (std::fabs(eta) < 1.2f) ? 0 : 1; }

} // namespace

MyCorrection::MuonScaleAndError MyCorrection::GetMuonScaleAndError(
    int charge, float pt, float eta, float phi, int trackerLayers,
    const float matched_pt) const {
  float roccor = 1.;
  float roccor_err = 0.;

  // few GeVs of muon shuold not use this correction, because the authors did
  // not consider the radiations of low pt muons inside detectors still true for
  // Run3?
  if (pt < 10.)
    return {1.f, 0.f};

  if (IsDATA) {
    roccor =
        rc.kScaleDT(charge, pt, eta, phi, 0, 0);
    roccor_err =
        rc.kScaleDTerror(charge, pt, eta, phi);
  } else {
    // Random seed is initialized in SKNanoLoader::Init()
    gRandom->SetSeed(int(pt / eta));
    float u = gRandom->Rndm();
    if (matched_pt > 0) { // matched
      roccor = rc.kSpreadMC(charge, pt, eta, phi, matched_pt, 0, 0);
      roccor_err = rc.kSpreadMCerror(charge, pt, eta, phi, matched_pt);
    } else {
      // roccor = rc.kScaleMC(muon.Charge(), muon.Pt(), muon.Eta(), muon.Phi(),
      // 0, 0); roccor_err = 0.; roccor_err = rc.kScaleMCerror(muon.Charge(),
      // muon.Pt(), muon.Eta(), muon.Phi());
      roccor = rc.kSmearMC(charge, pt, eta, phi, trackerLayers, u, 0, 0);
      roccor_err = rc.kSmearMCerror(charge, pt, eta, phi, trackerLayers, u);
    }
  }

  return {roccor, roccor_err};
}

float MyCorrection::GetMuonRECOSF(const MuonView &muon,
                                  const variation syst) const {
  if (Run == 3)
    return 1.f;
  auto cset = cset_muon->at("NUM_TrackerMuons_DEN_genTracks");
  return safeEvaluate(cset, "GetMuonRECOSF",
                      {muon.Eta(), std::max(40.f, muon.MiniAODPt()),
                       getSystString_MUO(syst)});
}

float MyCorrection::GetMuonHighPtSF(const TString &key, const float eta,
                                    const float pt,
                                    const variation syst) const {
  if (!cset_muon_highpt)
    return 1.f;
  // The maps stop at |eta| = 2.4 and start at 50 GeV; correctionlib throws
  // outside them. The JSON applies abs() to eta itself.
  const float clampedEta =
      std::min(std::fabs(eta), HIGHPT_SF_MAX_ABSETA - 1e-3f);
  const float clampedPt = std::max(pt, HIGHPT_SF_MIN_MOMENTUM);
  auto cset = cset_muon_highpt->at(string(key.Data()));
  return safeEvaluate(cset, "GetMuonHighPtSF",
                      {clampedEta, clampedPt, getSystString_MUO(syst)});
}

float MyCorrection::GetMuonHighPtRECOSF(const float eta, const float p,
                                        const variation syst) const {
  if (!cset_muon_highpt)
    return 1.f;
  // Binned in the full momentum p, not pt -- see the POG high-pT twiki.
  const float clampedEta =
      std::min(std::fabs(eta), HIGHPT_SF_MAX_ABSETA - 1e-3f);
  const float clampedP = std::max(p, HIGHPT_SF_MIN_MOMENTUM);
  auto cset = cset_muon_highpt->at("NUM_GlobalMuons_DEN_TrackerMuonProbes");
  return safeEvaluate(cset, "GetMuonHighPtRECOSF",
                      {clampedEta, clampedP, getSystString_MUO(syst)});
}

float MyCorrection::GetMuonGEScaledPt(const float pt, const float eta,
                                      const float phi, const int charge,
                                      const variation syst) const {
  const auto it = kGeKappa.find(GetEra().Data());
  if (it == kGeKappa.end())
    return pt; // No map for this era (Run 2, 2024+); leave the momentum alone.

  const GECell *cell = it->second.find(eta, phi);
  if (!cell)
    return pt;

  float kappa = cell->kappa;
  if (syst == variation::up)
    kappa += cell->sigma;
  else if (syst == variation::down)
    kappa -= cell->sigma;

  // k_meas = k_true + kappa  =>  pT_corr = pT / (1 - q * kappa * pT[TeV])
  const float ptTeV = pt / 1000.f;
  const float denom = 1.f - static_cast<float>(charge) * kappa * ptTeV;

  // Guard the pole: kappa*pT approaching 1 blows the correction up, which a
  // multi-TeV muon can reach (kappa = -0.48/TeV at pT = 2 TeV gives 0.04).
  // Emitting the uncorrected momentum beats emitting a runaway one.
  if (std::fabs(denom) < 0.1f) {
    std::cerr << "[MyCorrection::GetMuonGEScaledPt] Near-singular GE correction"
              << " (denom = " << denom << ", kappa = " << kappa
              << " TeV^-1, pT = " << pt << " GeV, q = " << charge
              << "); keeping the uncorrected pT." << std::endl;
    return pt;
  }
  return pt / denom;
}

float MyCorrection::GetMuonGESigmaShiftedPt(const float pt, const float eta,
                                            const float phi, const int charge,
                                            const variation syst) const {
  if (syst == variation::nom)
    return pt;

  const auto it = kGeKappa.find(GetEra().Data());
  if (it == kGeKappa.end())
    return pt;
  const GECell *cell = it->second.find(eta, phi);
  if (!cell)
    return pt;

  // Centred on zero: the shift is the uncertainty on kappa, not kappa itself.
  const float sigma = cell->sigma;
  const float kappa = (syst == variation::up) ? sigma : -sigma;

  const float ptTeV = pt / 1000.f;
  const float denom = 1.f - static_cast<float>(charge) * kappa * ptTeV;
  if (std::fabs(denom) < 0.1f) {
    std::cerr << "[MyCorrection::GetMuonGESigmaShiftedPt] Near-singular shift"
              << " (denom = " << denom << ", sigma = " << sigma
              << " TeV^-1, pT = " << pt << " GeV, q = " << charge
              << "); keeping the unshifted pT." << std::endl;
    return pt;
  }
  return pt / denom;
}

float MyCorrection::GetMuonHighPtResolution(const float p,
                                            const float eta) const {
  const auto it = kMuonResPoly.find(GetEra().Data());
  if (it == kMuonResPoly.end())
    return 0.f;
  const MuonResPoly &c = it->second[MuonResEtaBin(eta)];
  return c.a0 + c.a1 * p + c.a2 * p * p + c.a3 * p * p * p;
}

float MyCorrection::GetMuonHighPtSmearFactor(const float p, const float eta,
                                             const unsigned int seed,
                                             const variation syst) const {
  if (IsDATA)
    return 1.f;
  const auto itF = kMuonSmearF.find(GetEra().Data());
  if (itF == kMuonSmearF.end())
    return 1.f; // No map for this era (Run 2, 2024+); no extra smearing.

  const float fNom = itF->second[MuonResEtaBin(eta)];
  float f = fNom;
  if (syst == variation::up) {
    // Independent Gaussians, so the widths add in quadrature.
    f = std::sqrt(fNom * fNom + kMuonSmearSystF * kMuonSmearSystF);
  } else if (syst == variation::down) {
    // Convolution can only widen a resolution, so this floors at zero, which
    // makes the nuisance one-sided wherever the nominal smearing is already 0.
    f = std::sqrt(
        std::max(fNom * fNom - kMuonSmearSystF * kMuonSmearSystF, 0.f));
  }
  if (f <= 0.f)
    return 1.f;

  const float sigma = GetMuonHighPtResolution(p, eta);
  if (sigma <= 0.f)
    return 1.f;

  // Nominal and variations share the seed so they share the Gaussian pull:
  // the variation is then a coherent shift rather than an independent
  // re-smearing, which would pile resolution on top of the nominal.
  TRandom3 rng(seed);
  return 1.f + rng.Gaus(0.f, sigma * f);
}

float MyCorrection::GetMuonRECOSF(const MuonViewCollection &muons,
                                  const variation syst) const {
  float weight = 1.f;
  for (const auto muon : muons)
    weight *= GetMuonRECOSF(muon, syst);
  return weight;
}

float MyCorrection::GetMuonIDSF(const TString &key, const MuonView &muon,
                                const variation syst) const {
  if (key == "TopHNT") {
    auto cset = cset_muon_TopHNT_idsf->at("sf");
    if (syst == variation::nom)
      return safeEvaluate(cset, "GetMuonIDSF",
                          {fabs(muon.Eta()), muon.MiniAODPt(), "nom"});
    if (syst == variation::up)
      return safeEvaluate(cset, "GetMuonIDSF",
                          {fabs(muon.Eta()), muon.MiniAODPt(), "up"});
    if (syst == variation::down)
      return safeEvaluate(cset, "GetMuonIDSF",
                          {fabs(muon.Eta()), muon.MiniAODPt(), "down"});
    throw runtime_error("[MyCorrection::GetMuonIDSF] Invalid syst value");
  }
  auto cset = cset_muon->at(string(key));
  return safeEvaluate(
      cset, "GetMuonIDSF",
      {fabs(muon.Eta()), muon.MiniAODPt(), getSystString_MUO(syst)});
}

float MyCorrection::GetMuonIDSF(
    const TString &key, const MuonViewCollection &muons,
    const std::vector<std::size_t> &indices, const variation syst) const {
  float weight = 1.f;
  for (const std::size_t index : indices) {
    if (index >= muons.size())
      throw SKNano::LogicError("[MyCorrection::GetMuonIDSF] index out of range");
    weight *= GetMuonIDSF(key, muons[index], syst);
  }
  return weight;
}

float MyCorrection::GetMuonIDSF(const TString &key,
                                const MuonViewCollection &muons,
                                const variation syst) const {
  float weight = 1.f;
  for (const auto muon : muons)
    weight *= GetMuonIDSF(key, muon, syst);
  return weight;
}

// Electron
// For Run2, scale uncertainty is not stored in the NanoAODv9.
// Should patch from https://github.com/cms-egamma/ScaleFactorsJSON
// https://twiki.cern.ch/twiki/bin/view/CMS/EgammSFandSSRun3
float MyCorrection::GetElectronScaleUnc(const float scEta,
                                        const unsigned char seedGain,
                                        const unsigned int runNumber,
                                        const float r9, const float pt,
                                        const variation syst) const {
  if (IsDATA)
    return 1.0;
  // Eras whose EGM scale-and-smearing file is absent get no variation rather
  // than a null dereference.
  if (!cset_electron_variation)
    return 1.0;

  switch (Run) {
  case 2: {
    if (syst == variation::nom)
      return 1.;
    auto cset = cset_electron_variation->at("UL-EGM_ScaleUnc");
    vector<correction::Variable::Type> args = {
        GetEra().Data(), getSystString_EGMScale(syst), scEta,
        static_cast<int>(seedGain)};
    return safeEvaluate(cset, "GetElectronScaleSF", args);
  }
  case 3: {
    if (syst == variation::nom)
      return 1.;
    // The EtDependent JSON keeps the scale uncertainties in SmearAndSyst, and
    // they are already multiplicative factors (scale_up = 1 + escale). The
    // older schema exposed them through the scale correction itself under
    // "total_uncertainty"; that key does not exist here.
    static_cast<void>(seedGain);
    static_cast<void>(runNumber);
    auto cset = cset_electron_variation->at("SmearAndSyst");
    const string systKey =
        (syst == variation::up) ? "scale_up" : "scale_down";
    return safeEvaluate(cset, "GetElectronScaleUnc",
                        {systKey, pt, r9, scEta});
  }
  default:
    throw runtime_error(
        "[MyCorrection::GetElectronScaleUnc] Invalid run number");
  }

  // This should never be reached, but added to avoid compiler warning
  return 1.0;
}

float MyCorrection::GetElectronRECOSF(const float eta, const float pt,
                                      const float phi,
                                      const variation syst) const {
  auto cset = Run == 3 ? cset_electron->at("Electron-ID-SF")
                       : cset_electron->at("UL-Electron-ID-SF");
  switch (Run) {
  case 2:
    if (pt < 20.)
      return GetElectronIDSF("RecoBelow20", eta, pt, phi, syst);
    else
      return GetElectronIDSF("RecoAbove20", eta, pt, phi, syst);
    break;
  case 3:
    if (pt < 20.)
      return safeEvaluate(cset, "GetElectronRECOSF",
                          {GetEra().Data() + std::string("Prompt"),
                           getSystString_EGM(syst), "RecoBelow20", eta, pt});
    else if (pt < 75.)
      return safeEvaluate(cset, "GetElectronRECOSF",
                          {GetEra().Data() + std::string("Prompt"),
                           getSystString_EGM(syst), "Reco20to75", eta, pt});
    else
      return safeEvaluate(cset, "GetElectronRECOSF",
                          {GetEra().Data() + std::string("Prompt"),
                           getSystString_EGM(syst), "RecoAbove75", eta, pt});
    break;
  default:
    throw runtime_error("[MyCorrection::GetElectronRECOSF] Invalid run number");
  }
}

float MyCorrection::GetElectronRECOSF(const ElectronView &electron,
                                      const variation syst) const {
  return GetElectronRECOSF(std::fabs(electron.Eta()), electron.Pt(),
                           electron.Phi(), syst);
}

float MyCorrection::GetElectronRECOSF(
    const ElectronViewCollection &electrons, const variation syst) const {
  float weight = 1.f;
  for (const auto electron : electrons)
    weight *= GetElectronRECOSF(electron, syst);
  return weight;
}

float MyCorrection::GetElectronIDSF(const TString &Electron_ID_SF_Key,
                                    const float eta, const float pt,
                                    const float phi,
                                    const variation syst) const {
  if (Electron_ID_SF_Key == "TopHNT") {
    auto cset = cset_electron_TopHNT_idsf->at("sf");
    if (syst == variation::nom) {
      return safeEvaluate(cset, "GetElectronRECOSF", {eta, pt, "nom"});
    } else if (syst == variation::up) {
      return safeEvaluate(cset, "GetElectronRECOSF", {eta, pt, "up"});
    } else if (syst == variation::down) {
      return safeEvaluate(cset, "GetElectronRECOSF", {eta, pt, "down"});
    } else {
      throw runtime_error("[MyCorrection::GetElectronIDSF] Invalid syst value");
    }
  } else {
    // POG IDs
    string key;
    if (Run == 2)
      key = "UL-Electron-ID-SF";
    else if (Run == 3)
      key = "Electron-ID-SF";
    else
      throw runtime_error("[MyCorrection::GetElectronIDSF] Invalid run number");

    auto cset = cset_electron->at(key);
    return safeEvaluate(cset, "GetElectronIDSF",
                        {DataEra.Data() + std::string("Prompt"),
                         getSystString_EGM(syst), string(Electron_ID_SF_Key),
                         eta, pt < 999.9f ? pt : 999.9f});
  }
}

float MyCorrection::GetElectronIDSF(const TString &key,
                                    const ElectronView &electron,
                                    const variation syst) const {
  const float eta = key == "TopHNT" ? electron.ScEta()
                                    : std::fabs(electron.Eta());
  return GetElectronIDSF(key, eta, electron.Pt(), electron.Phi(), syst);
}

float MyCorrection::GetElectronIDSF(
    const TString &key, const ElectronViewCollection &electrons,
    const variation syst) const {
  float weight = 1.f;
  for (const auto electron : electrons)
    weight *= GetElectronIDSF(key, electron, syst);
  return weight;
}

// Trigger
float MyCorrection::GetMuonTriggerEff(const TString &Muon_Trigger_Eff_Key,
                                      const float eta, const float pt,
                                      const bool isData,
                                      const variation syst) const {
  static bool warned_missing_trig_eff = false;
  if (!cset_muon_trig_eff) {
    if (!warned_missing_trig_eff) {
      cerr << "[MyCorrection::GetMuonTriggerEff] Warning: trigger efficiency "
              "correction set is not loaded, returning 1."
           << endl;
      warned_missing_trig_eff = true;
    }
    return 1.;
  }

  correction::Correction::Ref cset;
  try {
    cset = cset_muon_trig_eff->at(string(Muon_Trigger_Eff_Key));
  } catch (const std::out_of_range &e) {
    if (!warned_missing_trig_eff) {
      cerr << "[MyCorrection::GetMuonTriggerEff] Warning: key "
           << Muon_Trigger_Eff_Key
           << " not found in trigger efficiency set, returning 1. (" << e.what()
           << ")" << endl;
      warned_missing_trig_eff = true;
    }
    return 1.;
  }
  if (isData)
    return safeEvaluate(cset, "GetTriggerEff",
                        {"data", getSystString_MUO(syst), eta, pt});
  else
    return safeEvaluate(cset, "GetTriggerEff",
                        {"mc", getSystString_MUO(syst), eta, pt});
}

float MyCorrection::GetMuonTriggerSF(const TString &key,
                                     const MuonView &muon,
                                     const variation syst) const {
  if (IsDATA)
    return 1.f;
  float weight = 1.f;
  // The pt clamp differs per set: the Z-based maps reach down to ~26 GeV while
  // the high-pT maps start at 50 and throw below that.
  auto tryEval = [&](const unique_ptr<CorrectionSet> &set, const float minPt) {
    if (!set)
      return false;
    try {
      const auto cset = set->at(key.Data());
      weight = safeEvaluate(cset, "GetMuonTriggerSF",
                            {muon.Eta(), std::max(minPt, muon.MiniAODPt()),
                             getSystString_MUO(syst)});
      return true;
    } catch (const std::out_of_range &) {
      return false;
    }
  };
  if (tryEval(cset_muon_trig_sf, 26.f) ||
      tryEval(cset_muon_highpt, HIGHPT_SF_MIN_MOMENTUM) ||
      tryEval(cset_muon, 26.f))
    return weight;
  return 1.f;
}

float MyCorrection::GetMuonTriggerSF(const TString &key,
                                     const MuonViewCollection &muons,
                                     const variation syst) const {
  if (!cset_muon_trig_eff) {
    float weight = 1.f;
    for (const auto muon : muons)
      weight *= GetMuonTriggerSF(key, muon, syst);
    return weight;
  }

  float failData = 1.f;
  float failMc = 1.f;
  for (const auto muon : muons) {
    failData *= 1.f - GetMuonTriggerEff(
        key, std::fabs(muon.Eta()), muon.Pt(), true, syst);
    failMc *= 1.f - GetMuonTriggerEff(
        key, std::fabs(muon.Eta()), muon.Pt(), false, syst);
  }
  const float passMc = 1.f - failMc;
  return passMc == 0.f ? 1.f : (1.f - failData) / passMc;
}

float MyCorrection::GetElectronTriggerEff(
    const TString &Electron_Trigger_SF_Key, const float eta, const float pt,
    const float phi, const bool isDATA, const variation syst) const {
  string key = isDATA ? "Electron-HLT-DataEff" : "Electron-HLT-McEff";
  auto cset = cset_electron_hlt->at(key);
  // hardcoded replacemet
  string ValType = getSystString_EGM(syst);
  if (ValType == "sf")
    ValType = "nom";
  else if (ValType == "sfup")
    ValType = "up";
  else if (ValType == "sfdown")
    ValType = "down";
  else
    throw runtime_error(
        "[MyCorrection::GetElectronTriggerEff] Invalid syst value");
  try {
    if (!isInputInCorrection("phi", cset)) {
      return safeEvaluate(cset, "GetTriggerEff",
                          {EGM_keys.at(GetEra().Data()), ValType,
                           string(Electron_Trigger_SF_Key), eta, pt});
    } else {
      return safeEvaluate(cset, "GetTriggerEff",
                          {EGM_keys.at(GetEra().Data()), ValType,
                           string(Electron_Trigger_SF_Key), eta, pt, phi});
    }
  } catch (exception &e) {
    cerr << "[MyCorrection::GetElectronTriggerEff] " << e.what() << endl;
    throw e;
  }
}

float MyCorrection::GetElectronTriggerSF(const TString &Electron_Trigger_SF_Key,
                                         const float eta, const float pt,
                                         const float phi,
                                         const variation syst) const {
  auto cset = cset_electron_hlt->at("Electron-HLT-SF");
  try {
    if (!isInputInCorrection("phi", cset)) {
      return safeEvaluate(cset, "GetTriggerEff",
                          {EGM_keys.at(GetEra().Data()),
                           getSystString_EGM(syst),
                           string(Electron_Trigger_SF_Key), eta, pt});
    } else {
      return safeEvaluate(cset, "GetTriggerEff",
                          {EGM_keys.at(GetEra().Data()),
                           getSystString_EGM(syst),
                           string(Electron_Trigger_SF_Key), eta, pt, phi});
    }
  } catch (exception &e) {
    cerr << "[MyCorrection::GetElectronTriggerSF] " << e.what() << endl;
    throw e;
  }
}

// Pileup
float MyCorrection::GetPUWeight(const float nTrueInt, const variation syst,
                                const TString &source) const {
  // nota bene: Input should be nTrueInt, not nPileUp
  correction::Correction::Ref cset = nullptr;
  cset = cset_puWeights->at(LUM_keys.at(GetEra().Data()));
  try {
    return safeEvaluate(cset, "GetPUWeight",
                        {nTrueInt, getSystString_LUM(syst)});
  } catch (exception &e) {
    cerr << "[MyCorrection::GetPUWeight] " << e.what() << endl;
    return 1.;
  }
}
