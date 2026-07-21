#include "MyCorrection.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

// JetID

bool MyCorrection::PassJetID(const JetView &jet, const JetView::JetID &id) const {
  correction::Correction::Ref cset = nullptr;
  float out = 0.f;
  switch (id) {
  case JetView::JetID::TIGHT:
    cset = cset_jetid->at("AK4PUPPI_Tight");
    out = cset->evaluate(
        {fabs(jet.Eta()), jet.ChHEF(), jet.NeHEF(), jet.ChEmEF(), jet.NeEmEF(),
         jet.MuEF(), static_cast<int>(jet.ChMultiplicity()),
         static_cast<int>(jet.NeMultiplicity()),
         static_cast<int>(jet.ChMultiplicity() + jet.NeMultiplicity())});
    return out > 0.5;
  case JetView::JetID::TIGHTLEPVETO:
    cset = cset_jetid->at("AK4PUPPI_TightLeptonVeto");
    out = cset->evaluate(
        {fabs(jet.Eta()), jet.ChHEF(), jet.NeHEF(), jet.ChEmEF(), jet.NeEmEF(),
         jet.MuEF(), static_cast<int>(jet.ChMultiplicity()),
         static_cast<int>(jet.NeMultiplicity()),
         static_cast<int>(jet.ChMultiplicity() + jet.NeMultiplicity())});
    return out > 0.5;
  case JetView::JetID::NOCUT:
    return true;
  default:
    throw runtime_error("[MyCorrection::PassJetID] Invalid JetID type");
  }
}

bool MyCorrection::PassFatJetID(const FatJetView &fatjet,
                                FatJetView::ID id) const {
  correction::Correction::Ref cset = nullptr;
  float out;
  switch (id) {
  case FatJetView::ID::TIGHT:
    cset = cset_jetid->at("AK8PUPPI_Tight");
    out = cset->evaluate({fabs(fatjet.Eta()), fatjet.chHEF(), fatjet.neHEF(),
                          fatjet.chEmEF(), fatjet.neEmEF(), fatjet.muEF(),
                          fatjet.chMultiplicity(), fatjet.neMultiplicity(),
                          fatjet.chMultiplicity() + fatjet.neMultiplicity()});
    return out > 0.5; // return is real
    break;
  case FatJetView::ID::TIGHTLEPVETO:
    cset = cset_jetid->at("AK8PUPPI_TightLeptonVeto");
    out = cset->evaluate({fabs(fatjet.Eta()), fatjet.chHEF(), fatjet.neHEF(),
                          fatjet.chEmEF(), fatjet.neEmEF(), fatjet.muEF(),
                          fatjet.chMultiplicity(), fatjet.neMultiplicity(),
                          fatjet.chMultiplicity() + fatjet.neMultiplicity()});
    return out > 0.5; // return is real
    break;
  case FatJetView::ID::NOCUT:
    // No cut, always return true
    return true;
    break;
  default:
    throw runtime_error("[MyCorrection::PassFatJetID] Invalid JetID type");
  }
}

// JERC
float MyCorrection::safeEvaluate2D(const correction::Correction::Ref &cset,
                                   const string &function_name, float x,
                                   float y) const {
  if (!cset) {
    throw SKNano::ConfigError("[MyCorrection::" + function_name +
                              "] Correction set is null");
  }

  static thread_local vector<correction::Variable::Type> args;
  args.clear();
  args.emplace_back(x);
  args.emplace_back(y);
  try {
    return cset->evaluate(args);
  } catch (const std::exception &e) {
    std::ostringstream oss;
    oss << "[MyCorrection::" << function_name
        << "] Error during evaluation: " << e.what() << "; arguments ("
        << args.size() << "): ";
    for (const auto &arg : args)
      std::visit([&oss](const auto &value) { oss << value << " "; }, arg);
    throw SKNano::CorrectionError(oss.str());
  }
}

float MyCorrection::safeEvaluate3D(const correction::Correction::Ref &cset,
                                   const string &function_name, float x,
                                   float y, float z) const {
  if (!cset) {
    throw SKNano::ConfigError("[MyCorrection::" + function_name +
                              "] Correction set is null");
  }

  static thread_local vector<correction::Variable::Type> args;
  args.clear();
  args.emplace_back(x);
  args.emplace_back(y);
  args.emplace_back(z);
  try {
    return cset->evaluate(args);
  } catch (const std::exception &e) {
    std::ostringstream oss;
    oss << "[MyCorrection::" << function_name
        << "] Error during evaluation: " << e.what() << "; arguments ("
        << args.size() << "): ";
    for (const auto &arg : args)
      std::visit([&oss](const auto &value) { oss << value << " "; }, arg);
    throw SKNano::CorrectionError(oss.str());
  }
}

const correction::Correction::Ref &
MyCorrection::getJERPtResolutionCorrection() const {
  if (!cachedJERPtResolution) {
    string cset_string = JME_JER_GT.at(GetEra().Data());
    cset_string.replace(cset_string.find("######"), 6, "PtResolution");
    cachedJERPtResolution = cset_jerc->at(cset_string);
  }
  return cachedJERPtResolution;
}

const correction::Correction::Ref &
MyCorrection::getJERScaleFactorCorrection() const {
  if (!cachedJERScaleFactor) {
    string cset_string = JME_JER_GT.at(GetEra().Data());
    cset_string.replace(cset_string.find("######"), 6, "ScaleFactor");
    cachedJERScaleFactor = cset_jerc->at(cset_string);
  }
  return cachedJERScaleFactor;
}

const correction::Correction::Ref &
MyCorrection::getJERSFUncertaintyCorrection() const {
  if (!cachedJERSFUncertainty) {
    string cset_string = JME_JER_GT.at(GetEra().Data());
    cset_string.replace(cset_string.find("######"), 6, "SFUncertainty");
    cachedJERSFUncertainty = cset_jerc->at(cset_string);
  }
  return cachedJERSFUncertainty;
}

const correction::Correction::Ref &
MyCorrection::getJESUncertaintyCorrection(const string &source) const {
  const string sourceKey = source.empty() ? "total" : source;
  auto it = cachedJESUncertaintyCorrections.find(sourceKey);
  if (it == cachedJESUncertaintyCorrections.end()) {
    string cset_string = JME_JES_GT.at(GetEra().Data());
    cset_string.replace(cset_string.find("######"), 6, sourceKey);
    it = cachedJESUncertaintyCorrections
             .emplace(sourceKey, cset_jerc->at(cset_string))
             .first;
  }
  return it->second;
}

float MyCorrection::GetJER(const float eta, const float pt,
                           const float rho) const {
  return safeEvaluate3D(getJERPtResolutionCorrection(), "GetJER", eta, pt,
                        rho);
}

float MyCorrection::GetJERSF(const float eta, const float pt,
                             const variation syst,
                             const TString &source) const {
  const auto &cset = getJERScaleFactorCorrection();
  if (GetEra() == "2024") {
    const float sf = safeEvaluate2D(cset, "GetJERSF", eta, pt);
    if (syst == variation::nom) {
      return sf;
    }

    const float sf_unc =
        safeEvaluate2D(getJERSFUncertaintyCorrection(), "GetJERSFUncertainty",
                       eta, pt);
    if (syst == variation::up) {
      return sf + sf_unc;
    }
    if (syst == variation::down) {
      const float sf_down = sf - sf_unc;
      return sf_down > 0.f ? sf_down : 0.f;
    }
    return sf;
  }
  if (Run == 3) {
    return safeEvaluate(cset, "GetJERSF", {eta, pt, getSystString_JME(syst)});
  } else if (Run == 2) {
    return safeEvaluate(cset, "GetJERSF", {eta, getSystString_JME(syst)});
  }
  return 1.;
}

MyCorrection::JERSFSet MyCorrection::GetJERSFSet(const float eta,
                                                 const float pt,
                                                 const TString &source) const {
  const float sf = GetJERSF(eta, pt, variation::nom, source);
  const auto variations = GetJERSFVariations(eta, pt, sf, source);
  return {sf, variations.up, variations.down};
}

MyCorrection::JERSFVariations
MyCorrection::GetJERSFVariations(const float eta, const float pt,
                                 const float nominal,
                                 const TString &source) const {
  static_cast<void>(source);
  if (GetEra() == "2024") {
    const float sf_unc =
        safeEvaluate2D(getJERSFUncertaintyCorrection(), "GetJERSFUncertainty",
                       eta, pt);
    return {nominal + sf_unc, std::max(nominal - sf_unc, 0.f)};
  }
  const auto &cset = getJERScaleFactorCorrection();
  if (Run == 3) {
    return {safeEvaluate(cset, "GetJERSF",
                         {eta, pt, getSystString_JME(variation::up)}),
            safeEvaluate(cset, "GetJERSF",
                         {eta, pt, getSystString_JME(variation::down)})};
  }
  if (Run == 2) {
    return {safeEvaluate(cset, "GetJERSF",
                         {eta, getSystString_JME(variation::up)}),
            safeEvaluate(cset, "GetJERSF",
                         {eta, getSystString_JME(variation::down)})};
  }
  return {nominal, nominal};
}

// JESC
float MyCorrection::GetJESSF(const float area, const float eta, const float pt,
                             const float phi, const float rho,
                             const unsigned int runNumber) const {
  const string era = GetEra().Data();
  if (!preparedJESValid || preparedJESEra != era ||
      preparedJESIsData != IsDATA) {
    const string keyTemplate = JME_JES_GT.at(era);
    const auto makeKey = [&keyTemplate](const char *level) {
      string key = keyTemplate;
      const auto marker = key.find("######");
      if (marker == string::npos)
        throw SKNano::ConfigError("[MyCorrection::GetJESSF] Invalid JES key template");
      key.replace(marker, 6, level);
      return key;
    };

    preparedJESCompound = nullptr;
    preparedJESL1 = nullptr;
    preparedJESL2 = nullptr;
    preparedJESL3 = nullptr;
    preparedJESResidual = nullptr;
    if (era != "2024") {
      preparedJESCompound = cset_jerc->compound().at(makeKey("L1L2L3Res"));
    } else {
      preparedJESL1 = cset_jerc->at(makeKey("L1FastJet"));
      preparedJESL2 = cset_jerc->at(makeKey("L2Relative"));
      preparedJESL3 = cset_jerc->at(makeKey("L3Absolute"));
      if (IsDATA)
        preparedJESResidual = cset_jerc->at(makeKey("L2L3Residual"));
    }
    preparedJESEra = era;
    preparedJESIsData = IsDATA;
    preparedJESValid = true;
  }

  if (era != "2024") {
    if (GetEra() == "2023BPix") {
      if (IsDATA)
        return safeEvaluateFloats(
            preparedJESCompound, "GetJESSF",
            std::array<float, 6>{area, eta, pt, rho, phi,
                                 static_cast<float>(runNumber)});
      return safeEvaluateFloats(
          preparedJESCompound, "GetJESSF",
          std::array<float, 5>{area, eta, pt, rho, phi});
    } else if (GetEra() == "2023") {
      if (IsDATA)
        return safeEvaluateFloats(
            preparedJESCompound, "GetJESSF",
            std::array<float, 5>{area, eta, pt, rho,
                                 static_cast<float>(runNumber)});
      return safeEvaluateFloats(
          preparedJESCompound, "GetJESSF",
          std::array<float, 4>{area, eta, pt, rho});
    }
    return safeEvaluateFloats(
        preparedJESCompound, "GetJESSF",
        std::array<float, 4>{area, eta, pt, rho});
  }
  else{
    float current_pt = pt;
    float sf_L1 = safeEvaluateFloats(
        preparedJESL1, "GetJESCorrection",
        std::array<float, 4>{area, eta, current_pt, rho});
    current_pt = current_pt * sf_L1;
    float sf_L2 = safeEvaluateFloats(
        preparedJESL2, "GetJESCorrection",
        std::array<float, 3>{eta, phi, current_pt});
    current_pt = current_pt * sf_L2;
    float sf_L3 = safeEvaluateFloats(
        preparedJESL3, "GetJESCorrection",
        std::array<float, 2>{eta, current_pt});
    current_pt = current_pt * sf_L3;
    float sf_res = 1.;
    if (IsDATA) {
      if(abs(eta) >=2.0 && abs(eta) < 2.5) current_pt = std::max(30.001f, current_pt);
      sf_res = safeEvaluateFloats(
          preparedJESResidual, "GetJESCorrection",
          std::array<float, 3>{static_cast<float>(runNumber), eta,
                               current_pt});
    }
    return sf_L1 * sf_L2 * sf_L3 * sf_res;
  }
}

float MyCorrection::GetJESUncertainty(const float eta, const float pt,
                                      const TString &source) const {
  return safeEvaluate2D(getJESUncertaintyCorrection(source.Data()),
                        "GetJESUncertainty", eta, pt);
}

float MyCorrection::GetJESUncertaintySF(const float eta, const float pt,
                                        const variation syst,
                                        const TString &source) const {
  int int_syst = 0;
  if (syst == variation::up)
    int_syst = 1;
  else if (syst == variation::down)
    int_syst = -1;
  else
    int_syst = 0;

  float this_factor = 1.;
  this_factor += int_syst * GetJESUncertainty(eta, pt, source);
  return this_factor;
}

void MyCorrection::EvaluateJetCorrectionBatch(
    const SKNano::JetCorrectionBatchInput &input,
    SKNano::JetCorrectionBatchOutput &output,
    SKNano::CorrectionLaneMask jerLanes,
    SKNano::CorrectionLaneMask jesLanes,
    const TString &jesSource) const {
  struct PreparedEvaluator {
    const MyCorrection &correction;
    const TString &source;
    float jec(float area, float eta, float rawPt, float phi, float rho,
              std::uint32_t run) const {
      return correction.GetJESSF(area, eta, rawPt, phi, rho, run);
    }
    std::array<float, 3> jerScaleFactors(float eta, float pt) const {
      const auto values = correction.GetJERSFSet(eta, pt);
      return {values.nom, values.up, values.down};
    }
    float jerResolution(float eta, float pt, float rho) const {
      return correction.GetJER(eta, pt, rho);
    }
    float jesUncertainty(float eta, float pt) const {
      return correction.GetJESUncertainty(eta, pt, source);
    }
    float sqrt(float value) const { return std::sqrt(value); }
  } evaluator{*this, jesSource};

  SKNano::JetCorrectionBatch batch(jerLanes, jesLanes);
  batch.evaluate(input, output, evaluator);
}

bool MyCorrection::IsJetVetoZone(const float eta, const float phi,
                                 TString mapCategory) const {
  correction::Correction::Ref cset = nullptr;
  string cset_string = JME_vetomap_keys.at(GetEra().Data());
  cset = cset_jetvetomap->at(cset_string);
  if (safeEvaluate(cset, "IsJetVetoZone", {mapCategory.Data(), eta, phi}) > 0)
    return true;
  return false;
}

void MyCorrection::METXYCorrection(Particle &Met, const int RunNumber,
                                   const int npvs,
                                   const XYCorrection_MetType MetType) {
  if (Run == 3) {
    // No supprot in Run3
    return;
  }
  correction::Correction::Ref cset_pt = nullptr;
  correction::Correction::Ref cset_phi = nullptr;
  switch (MetType) {
  case XYCorrection_MetType::Type1PFMET:
    if (IsDATA) {
      cset_pt = cset_met->at("pt_metphicorr_pfmet_data");
      cset_phi = cset_met->at("phi_metphicorr_pfmet_data");
    } else {
      cset_pt = cset_met->at("pt_metphicorr_pfmet_mc");
      cset_phi = cset_met->at("phi_metphicorr_pfmet_mc");
    }
    break;
  case XYCorrection_MetType::Type1PuppiMET:
    if (IsDATA) {
      cset_pt = cset_met->at("pt_metphicorr_puppimet_data");
      cset_phi = cset_met->at("phi_metphicorr_puppimet_data");
    } else {
      cset_pt = cset_met->at("pt_metphicorr_puppimet_mc");
      cset_phi = cset_met->at("phi_metphicorr_puppimet_mc");
    }
    break;
  }
  float this_pt =
      cset_pt->evaluate({Met.Pt(), Met.Phi(), static_cast<float>(npvs),
                         static_cast<float>(RunNumber)});
  float this_phi =
      cset_phi->evaluate({Met.Pt(), Met.Phi(), static_cast<float>(npvs),
                          static_cast<float>(RunNumber)});
  float this_eta = Met.Eta();
  float this_m = Met.M();
  Met.SetPtEtaPhiM(this_pt, this_eta, this_phi, this_m);
}
