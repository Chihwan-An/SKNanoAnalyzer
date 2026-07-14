#include "MyCorrection.h"
#include "MLHelper.h"

#include <TLorentzVector.h>
#include <algorithm>
#include <execution>
#include <iostream>
#include <numeric>
#include <vector>

MyCorrection::MyCorrection() {}
MyCorrection::MyCorrection(const TString &era, const TString &period,
                           const TString &sample, const bool IsData,
                           const string &btagging_eff_file,
                           const string &ctagging_eff_file,
                           const string &btagging_R_file,
                           const string &ctagging_R_file) {
  cout << "[MyCorrection::MyCorrection] MyCorrection created for " << era
       << endl;
  SetEra(era);
  SetPeriod(period);
  SetSample(sample);
  setIsData(IsData);

  EraConfig config = GetEraConfig(era, btagging_eff_file, ctagging_eff_file,
                                  btagging_R_file, ctagging_R_file);
  struct CorrectionInfo {
    string name;
    string path;
    unique_ptr<CorrectionSet> &cset;
    bool isOptional;
  };

  // load correction sets
  // only jetid and jet, jerc are available for 2024 for now, I change optional
  // flag to true for other sets
  loadCorrectionSet("muon SF", config.json_muon, cset_muon, true);
  loadCorrectionSet("puWeights", config.json_puWeights, cset_puWeights, true);
  loadCorrectionSet("btagging", config.json_btagging, cset_btagging, true);
  loadCorrectionSet("ctagging", config.json_ctagging, cset_ctagging, true);
  loadCorrectionSet("btagging eff", config.json_btagging_eff, cset_btagging_eff,
                    true);
  loadCorrectionSet("ctagging eff", config.json_ctagging_eff, cset_ctagging_eff,
                    true);
  loadCorrectionSet("electron", config.json_electron, cset_electron, true);
  loadCorrectionSet("electron variation", config.json_electron_variation,
                    cset_electron_variation, true);
  loadCorrectionSet("photon", config.json_photon, cset_photon, true);
  loadCorrectionSet("jetid", config.json_jetid, cset_jetid, false);
  loadCorrectionSet("jerc", config.json_jerc, cset_jerc, false);
  loadCorrectionSet("jerc_fatjet", config.json_jerc_fatjet, cset_jerc_fatjet,
                    true);
  loadCorrectionSet("jetvetomap", config.json_jetvetomap, cset_jetvetomap,
                    false);
  // Optional files
  loadRoccoR(config.txt_roccor, true);
  loadGoldenJson(config.golden_json, true);
  loadCorrectionSet("jmar", config.json_jmar, cset_jmar, true);
  loadCorrectionSet("muon trig eff", config.json_muon_trig_eff,
                    cset_muon_trig_eff, true);
  loadCorrectionSet("muon trig sf", config.json_muon_trig_sf, cset_muon_trig_sf,
                    true); // temporary due to no mu trig sf for 2024
  loadCorrectionSet("electron hlt", config.json_electron_hlt, cset_electron_hlt,
                    true);
  loadCorrectionSet("met", config.json_met, cset_met, true);
  loadCorrectionSet("btagging R", config.json_btagging_R, cset_btagging_R,
                    true);
  loadCorrectionSet("ctagging R", config.json_ctagging_R, cset_ctagging_R,
                    true);
  loadCorrectionSet("muon TopHNT idsf", config.json_muon_TopHNT_idsf,
                    cset_muon_TopHNT_idsf, true);
  loadCorrectionSet("muon TopHNT dblmu leg1 eff",
                    config.json_muon_TopHNT_dblmu_leg1_eff,
                    cset_muon_TopHNT_dblmu_leg1_eff, true);
  loadCorrectionSet("muon TopHNT dblmu leg2 eff",
                    config.json_muon_TopHNT_dblmu_leg2_eff,
                    cset_muon_TopHNT_dblmu_leg2_eff, true);
  loadCorrectionSet("muon TopHNT emu leg1 eff",
                    config.json_muon_TopHNT_emu_leg1_eff,
                    cset_muon_TopHNT_emu_leg1_eff, true);
  loadCorrectionSet("muon TopHNT emu leg2 eff",
                    config.json_muon_TopHNT_emu_leg2_eff,
                    cset_muon_TopHNT_emu_leg2_eff, true);
  loadCorrectionSet("electron TopHNT idsf", config.json_electron_TopHNT_idsf,
                    cset_electron_TopHNT_idsf, true);
  loadCorrectionSet("electron TopHNT emu leg1 eff",
                    config.json_electron_TopHNT_emu_leg1_eff,
                    cset_electron_TopHNT_emu_leg1_eff, true);
  loadCorrectionSet("electron TopHNT emu leg2 eff",
                    config.json_electron_TopHNT_emu_leg2_eff,
                    cset_electron_TopHNT_emu_leg2_eff, true);

  MLHelper_hDampUp =
      make_unique<MLHelper>(config.onnx_hDampUp, MLHelper::ModelType::ONNX);
  MLHelper_hDampDown =
      make_unique<MLHelper>(config.onnx_hDampDown, MLHelper::ModelType::ONNX);
  MLHelper_TopPtReweight = make_unique<MLHelper>(config.onnx_toppt_reweight,
                                                 MLHelper::ModelType::ONNX);
  MLHelper_rBnom =
      make_unique<MLHelper>(config.onnx_rBnom, MLHelper::ModelType::ONNX);
  MLHelper_rBUp =
      make_unique<MLHelper>(config.onnx_rBUp, MLHelper::ModelType::ONNX);

  LUM_keys["2024"] = "Collisions24_BCDEFGHI_goldenJSON";
  LUM_keys["2023BPix"] = "Collisions2023_369803_370790_eraD_GoldenJson";
  LUM_keys["2023"] = "Collisions2023_366403_369802_eraBC_GoldenJson";
  LUM_keys["2022EE"] = "Collisions2022_359022_362760_eraEFG_GoldenJson";
  LUM_keys["2022"] = "Collisions2022_355100_357900_eraBCD_GoldenJson";
  LUM_keys["2018"] = "Collisions18_UltraLegacy_goldenJSON";
  LUM_keys["2017"] = "Collisions17_UltraLegacy_goldenJSON";
  LUM_keys["2016postVFP"] = "Collisions16_UltraLegacy_goldenJSON";
  LUM_keys["2016preVFP"] = "Collisions16_UltraLegacy_goldenJSON";

  EGM_keys["2024"] = "2024Prompt";
  EGM_keys["2023BPix"] = "2023PromptD";
  EGM_keys["2023"] = "2023PromptC";
  EGM_keys["2022EE"] = "2022Re-recoE+PromptFG";
  EGM_keys["2022"] = "2022Re-recoBCD";
  EGM_keys["2016preVFP"] = "2016preVFP";
  EGM_keys["2016postVFP"] = "2016postVFP";
  EGM_keys["2017"] = "2017";
  EGM_keys["2018"] = "2018";

  // Please use ####### as placeholder
  if (!IsData) {
    JME_JER_GT["2024"] = "Summer24Prompt24_JRV1_MC_######_AK4PFPuppi";
    JME_JES_GT["2024"] = "Summer24Prompt24_V3_MC_######_AK4PFPuppi";
  } else {
    JME_JER_GT["2024"] = "Summer24Prompt24_JRV1_MC_######_AK4PFPuppi";
    JME_JES_GT["2024"] = "Summer24Prompt24_V3_DATA_######_AK4PFPuppi";
  }

  JME_vetomap_keys["2024"] = "Summer24Prompt24_RunBCDEFGHI_V1";

  JME_PILEUP_keys["2016preVFP"] = "PUJetID_eff";
  JME_PILEUP_keys["2016postVFP"] = "PUJetID_eff";
  JME_PILEUP_keys["2017"] = "PUJetID_eff";
  JME_PILEUP_keys["2018"] = "PUJetID_eff";
}

MyCorrection::~MyCorrection() {}

MyCorrection::EraConfig
MyCorrection::GetEraConfig(TString era, const string &btagging_eff_file,
                           const string &ctagging_eff_file,
                           const string &btagging_R_file,
                           const string &ctagging_R_file) const {
  EraConfig config;

  const char *json_pog_path = getenv("JSONPOG_REPO_PATH");
  const char *sknano_data = getenv("SKNANO_DATA");
  const char *external_roccor = getenv("ROCCOR_PATH");
  cout << "[MyCorrection::GetEraConfig] json_pog_path: "
       << (json_pog_path ? json_pog_path : "NULL") << endl;
  cout << "[MyCorrection::GetEraConfig] sknano_data: "
       << (sknano_data ? sknano_data : "NULL") << endl;
  cout << "[MyCorrection::GetEraConfig] external_roccor: "
       << (external_roccor ? external_roccor : "NULL") << endl;
  if (!json_pog_path || !sknano_data || !external_roccor) {
    throw runtime_error(
        "JSONPOG_REPO_PATH or SKNANO_DATA or ROCCOR_PATH is not set");
  }

  const string json_pog_path_str(json_pog_path);
  const string sknano_data_str(sknano_data);
  const string external_roccor_str(external_roccor);

  config.json_muon = json_pog_path_str + "/MUO";
  config.json_muon_trig_eff = sknano_data_str;
  config.json_muon_trig_sf =
      json_pog_path_str + "/MUO"; // temporary due to no mu trig sf for 2024
  config.json_puWeights = json_pog_path_str + "/LUM";
  config.json_btagging = json_pog_path_str + "/BTV";
  config.json_ctagging = json_pog_path_str + "/BTV";
  config.json_btagging_eff = sknano_data_str;
  config.json_ctagging_eff = sknano_data_str;
  config.json_btagging_R = sknano_data_str;
  config.json_ctagging_R = sknano_data_str;
  config.json_electron = json_pog_path_str + "/EGM";
  config.json_electron_hlt = config.json_electron;
  config.json_electron_variation = json_pog_path_str + "/EGM";
  config.json_photon = json_pog_path_str + "/EGM";
  config.json_jetid = json_pog_path_str + "/JME";
  config.json_jerc = json_pog_path_str + "/JME";
  config.json_jerc_fatjet = json_pog_path_str + "/JME";
  config.json_jetvetomap = json_pog_path_str + "/JME";
  config.json_jmar = json_pog_path_str + "/JME";
  config.json_met = json_pog_path_str + "/JME";
  config.txt_roccor = external_roccor_str;
  config.golden_json = sknano_data_str;

  config.onnx_hDampDown = sknano_data_str;
  config.onnx_hDampUp = sknano_data_str;
  config.onnx_toppt_reweight = sknano_data_str;
  config.onnx_rBnom = sknano_data_str;
  config.onnx_rBUp = sknano_data_str;

  // config.json_muon_custom_TopHNT_idsf = sknano_data_str;
  // config.json_muon_custom_dblmu_leg1_eff = sknano_data_str;
  // config.json_muon_custom_dblmu_leg2_eff = sknano_data_str;
  // config.json_muon_custom_emu_leg1_eff = sknano_data_str;
  // config.json_muon_custom_emu_leg2_eff = sknano_data_str;
  // config.json_electron_custom_TopHNT_idsf = sknano_data_str;
  // config.json_electron_custom_emu_leg1_eff = sknano_data_str;
  // config.json_electron_custom_emu_leg2_eff = sknano_data_str;

  if (era == "2024") {
    const string tag =
        "/Run3-24CDEReprocessingFGHIPrompt-Summer24-NanoAODv15/latest/";
    const string tag_temp = "/Run3-23DSep23-Summer23BPix-NanoAODv12/latest/";
    config.json_muon += tag + "muon_Z.json.gz";
    config.json_muon_trig_eff += "/2024/MUO/muon_trig.json";
    config.json_muon_trig_sf += tag + "muon_Z.json.gz";
    config.json_puWeights += tag + "puWeights_BCDEFGHI.json.gz";
    config.json_btagging += tag + "btagging.json.gz";
    // config.json_ctagging += "/2023_Summer23BPix/ctagging.json.gz";
    // config.json_btagging_eff += "/2023BPix/BTV/" + btagging_eff_file;
    // config.json_ctagging_eff += "/2023BPix/BTV/" + ctagging_eff_file;
    // config.json_btagging_R += "/2023BPix/BTV/" + btagging_R_file;
    // config.json_ctagging_R += "/2023BPix/BTV/" + ctagging_R_file;
    config.json_electron += tag + "electron.json.gz";
    config.json_electron_variation = tag + "electronSS_EtDependent.json.gz";
    config.json_electron_hlt += tag + "electronHlt.json.gz";
    // config.json_photon += "/2023_Summer23BPix/photon.json.gz";
    config.json_jetid += tag + "jetid.json.gz";
    config.json_jerc += tag + "jet_jerc.json.gz";
    // config.json_jerc_fatjet += "/2023_Summer23BPix/fatJet_jerc.json.gz";
    config.json_jetvetomap += tag + "jetvetomaps.json.gz";
    // config.json_met += "/2023_Summer23BPix/met.json.gz";
    config.txt_roccor += "/RoccoR2023BPix.txt";
    config.golden_json +=
        "/2024/LUM/Cert_Collisions2024_378981_386951_Golden.json";

    config.onnx_hDampDown += "/2024/ONNX/mymodel12_hdamp_down_13.6TeV.onnx";
    config.onnx_hDampUp += "/2024/ONNX/mymodel12_hdamp_up_13.6TeV.onnx";
    config.onnx_toppt_reweight +=
        "/2024/ONNX/mymodel12_13TeV_MiNNLO_afterShower.onnx";
    config.onnx_rBnom += "/2024/ONNX/mymodel12_rB_nom_CP5_2M.onnx";
    config.onnx_rBUp += "/2024/ONNX/mymodel12_rB_up_CP5_2M.onnx";

    // print in red
    cout << "\033[1;31m[MyCorrection::GetEraConfig] Warning: ONNX models for "
            "TopPt reweight is for 13TeV! Please update the models for "
            "13.6TeV!\033[0m"
         << endl;

  } else {
    throw invalid_argument("[MyCorrection::GetEraConfig] Invalid era: " + era);
  }

  return config;
}

// GoldenLumi
bool MyCorrection::IsGoldenLumi(const unsigned int runNumber,
                                const unsigned int lumiSection) const {
  if (!IsDATA)
    return true;
  return golden_json_parser->isGood(runNumber, lumiSection);
}
