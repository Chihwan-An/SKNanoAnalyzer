#include "MyCorrection.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

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
    const string key = (GetEra().Contains("2022"))
                           ? "Scale"
                           : EGM_keys.at(GetEra().Data()) + "_ScaleJSON";
    auto cset = cset_electron_variation->at(key);
    vector<correction::Variable::Type> args = {"total_uncertainty",
                                               static_cast<int>(seedGain),
                                               static_cast<float>(runNumber),
                                               scEta,
                                               r9,
                                               pt};
    const float unc = safeEvaluate(cset, "GetElectronScaleSF", args);
    if (syst == variation::up)
      return 1. + unc;
    else if (syst == variation::down)
      return 1. - unc;
    else
      throw runtime_error(
          "[MyCorrection::GetElectronScaleUnc] Invalid syst value");
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
  auto tryEval = [&](const unique_ptr<CorrectionSet> &set) {
    if (!set)
      return false;
    try {
      const auto cset = set->at(key.Data());
      weight = safeEvaluate(cset, "GetMuonTriggerSF",
                            {muon.Eta(), std::max(26.f, muon.MiniAODPt()),
                             getSystString_MUO(syst)});
      return true;
    } catch (const std::out_of_range &) {
      return false;
    }
  };
  if (tryEval(cset_muon_trig_sf) || tryEval(cset_muon))
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
