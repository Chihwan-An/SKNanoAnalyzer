#include "Vcb.h"
#include "GenView.h"
#include "Muon.h"
#include "TEntryList.h"
#include "TEntryListArray.h"
#include "BranchManager.h"
#include <TLorentzVector.h>
#include <unordered_set>
#include "TObjArray.h"
#include "TBranch.h"

namespace {

constexpr double HF_T1 = 0.264;
constexpr double HF_T2 = 0.448;
constexpr double HF_T3 = 0.767;
constexpr double BVC_T1 = 0.01;
constexpr double BVC_T2 = 0.028;
constexpr double BVC_T3 = 0.094;
constexpr double BVC_T4 = 0.69;
constexpr double BVC_T5 = 0.918;
constexpr double BVC_T6 = 0.978;
constexpr double BVC_T7 = 0.994;

int bin_hf(double x) {
  return int(x >= HF_T1) + int(x >= HF_T2) + int(x >= HF_T3);
}

int bin_bvc(double y) {
  return int(y >= BVC_T1) + int(y >= BVC_T2) + int(y >= BVC_T3) +
         int(y >= BVC_T4) + int(y >= BVC_T5) + int(y >= BVC_T6) +
         int(y >= BVC_T7);
}

std::pair<double, double> HFvLF_BvC_from_components(double probudg,
                                                    double SvUDG, double CvL,
                                                    double CvB) {
  const double probs = (SvUDG > 0.0 && probudg > 0.0 && SvUDG < 1.0)
                           ? SvUDG * probudg / (1.0 - SvUDG)
                           : -1.0;

  const double probc = (CvL > 0.0 && CvL < 1.0 && probs > 0.0 && probudg > 0.0)
                           ? CvL * (probs + probudg) / (1.0 - CvL)
                           : -1.0;

  const double probbbblepb =
      (CvB > 0.0 && probc > 0.0) ? (1.0 - CvB) * probc / CvB : -1.0;

  const double BvC = (CvB > 0.0) ? (1.0 - CvB) : -1.0;

  const double HFvLF =
      (probbbblepb > 0.0 && probc > 0.0 && probs > 0.0 && probudg > 0.0)
          ? (probbbblepb + probc) / (probbbblepb + probc + probs + probudg)
          : -1.0;

  return {HFvLF, BvC};
}

std::pair<double, double> HFvLF_BvC_from_ParT(const Jet &j) {
  using Tagger = JetTagging::JetFlavTagger;
  using Score = JetTagging::JetFlavTaggerScoreType;

  return HFvLF_BvC_from_components(
      j.GetTaggerResult(Tagger::ParT, Score::probUDG),
      j.GetTaggerResult(Tagger::ParT, Score::SvUDG),
      j.GetTaggerResult(Tagger::ParT, Score::CvL),
      j.GetTaggerResult(Tagger::ParT, Score::CvB));
}

std::pair<double, double> HFvLF_BvC_from_storage(const JetSoA &store,
                                                 std::size_t idx) {
  return HFvLF_BvC_from_components(
      store.uparTAK4UDG[idx], store.uparTAK4SvUDG[idx], store.uparTAK4CvL[idx],
      store.uparTAK4CvB[idx]);
}

Vcb::Cat classify_from_scores(double hf, double bvc) {
  if (!(hf >= 0.0 && hf <= 1.0 && bvc >= 0.0 && bvc <= 1.0))
    return Vcb::Cat::N0;

  const int ih = bin_hf(hf);
  if (ih == 0)
    return Vcb::Cat::L0;
  if (ih == 1)
    return Vcb::Cat::C0;
  if (ih == 2)
    return Vcb::Cat::C1;

  static constexpr Vcb::Cat TOP_MAP[8] = {
      Vcb::Cat::C4, Vcb::Cat::C3, Vcb::Cat::C2, Vcb::Cat::B0,
      Vcb::Cat::B1, Vcb::Cat::B2, Vcb::Cat::B3, Vcb::Cat::B4};
  const int jb = bin_bvc(bvc);
  return TOP_MAP[jb];
}

Vcb::Cat classify_from_storage(const JetSoA &store, std::size_t idx) {
  auto [hf, bvc] = HFvLF_BvC_from_storage(store, idx);
  return classify_from_scores(hf, bvc);
}

void ComputeParTScores(const JetSoA &store, std::vector<float> &hfScores,
                       std::vector<float> &bvcScores,
                       std::vector<Vcb::Cat> &categories) {
  const std::size_t n = store.size();
  hfScores.resize(n);
  bvcScores.resize(n);
  categories.resize(n);
  for (std::size_t i = 0; i < n; ++i) {
    auto [hf, bvc] = HFvLF_BvC_from_storage(store, i);
    hfScores[i] = static_cast<float>(hf);
    bvcScores[i] = static_cast<float>(bvc);
    categories[i] = classify_from_scores(hf, bvc);
  }
}

void ComputeParTScores(const JetViewCollection &jets,
                       std::vector<float> &hfScores,
                       std::vector<float> &bvcScores,
                       std::vector<Vcb::Cat> &categories) {
  const auto &storagePtr = jets.storage();
  if (!storagePtr) {
    hfScores.clear();
    bvcScores.clear();
    categories.clear();
    return;
  }
  ComputeParTScores(*storagePtr, hfScores, bvcScores, categories);
}

} // namespace

Vcb::Vcb() {}

void Vcb::load_modelling_json(const TString &filename) {
  using json = nlohmann::json;

  std::cout << "Loading modelling json file: " << filename.Data() << std::endl;
  std::ifstream file_stream(filename.Data());
  if (!file_stream.is_open()) {
    throw std::runtime_error("Could not open modelling json file: " +
                             std::string(filename.Data()));
  }

  json j;
  file_stream >> j;
  file_stream.close();

  if (!j.contains("PD")) {
    throw std::runtime_error("modelling_json does not contain 'PD' field");
  }
  std::string pd_key = j["PD"].get<std::string>();

  if (!j.contains("subprocesses")) {
    throw std::runtime_error(
        "modelling_json does not contain 'subprocesses' field");
  }
  const auto &subprocs = j["subprocesses"];

  // 기존 거 다 지움
  modelling_patches.clear();

  // 여기서 실제로 있는 서브프로세스들 전부 밀어넣기
  for (auto it = subprocs.begin(); it != subprocs.end(); ++it) {
    const std::string sub_name = it.key(); // "BB", "CC", "JJ" ...
    const auto &sub = it.value();

    ModellingPatch mp;

    if (sub.contains("patch_ScaleVariation")) {
      for (const auto &v : sub["patch_ScaleVariation"]) {
        mp.patch_ScaleVariation.push_back(static_cast<float>(v.get<double>()));
      }
    }

    if (sub.contains("patch_PSVariation")) {
      for (const auto &v : sub["patch_PSVariation"]) {
        mp.patch_PSVariation.push_back(static_cast<float>(v.get<double>()));
      }
    }

    mp.patch_hdamp_up = sub.value("patch_hdamp_up", 0.0f);
    mp.patch_hdamp_down = sub.value("patch_hdamp_down", 0.0f);
    mp.patch_minnlo = sub.value("patch_minnlo", 0.0f);

    // 맵에 저장
    modelling_patches.emplace(sub_name, std::move(mp));
  }

  // 로그
  std::cout << "[modelling] PD = " << pd_key << std::endl;
  std::cout << "[modelling] loaded subprocesses: ";
  for (const auto &kv : modelling_patches) {
    std::cout << kv.first << " ";
  }
  std::cout << std::endl;
}

void Vcb::initializeAnalyzer() {
  SetChannel();
  string SKNANO_HOME = std::getenv("SKNANO_HOME");
  if (!IsDATA) {
    TString json_path = SKNANO_HOME + "/ModellingPatch/" + MCSample.Data() +
                        "_" + DataEra.Data() + "_summary.json";
    load_modelling_json(json_path);
  }
  const auto activeTagger = FlavTagger[DataEra.Data()];
  std::string btagging_eff_file = "btaggingEff.json";
  std::string ctagging_eff_file = "ctaggingEff.json";
  std::string btagging_R_file = "btaggingR.json";
  std::string ctagging_R_file = "ctaggingR.json";
  if (channel == Channel::FH) {
    std::cout << "Initialize MyCorrection for FH" << std::endl;
    btagging_R_file = "Vcb_FH_btaggingR.json";
    ctagging_R_file = "Vcb_FH_ctaggingR.json";
    myCorr = new MyCorrection(
        DataEra, DataPeriod, IsDATA ? DataStream : MCSample, IsDATA,
        btagging_eff_file, ctagging_eff_file, btagging_R_file, ctagging_R_file);
    myCorr->SetTaggingParam(activeTagger, FH_BTag_WP);
    myMLHelper = std::make_unique<MLHelper>(
        "/data6/Users/yeonjoon/SKNanoAnalyzer/data/Run3_v12_Run2_v9/2022EE/"
        "spanet_FH_2022EE.onnx",
        MLHelper::ModelType::ONNX);
  } else if (channel == Channel::Mu || channel == Channel::El) {
    std::cout << "Initialize MyCorrection for SL" << std::endl;
    btagging_R_file = "Vcb_SL_btaggingR.json";
    ctagging_R_file = "Vcb_SL_ctaggingR.json";
    myCorr = new MyCorrection(
        DataEra, DataPeriod, IsDATA ? DataStream : MCSample, IsDATA,
        btagging_eff_file, ctagging_eff_file, btagging_R_file, ctagging_R_file);
    myCorr->SetTaggingParam(activeTagger, SL_BTag_WP);
    // myMLHelper =
    // std::make_unique<MLHelper>("/data6/Users/yeonjoon/SKNanoAnalyzer/data/spanet_run3.onnx",
    // MLHelper::ModelType::ONNX);
    myMLHelper_CLASSIF_folds.reserve(4);
    myMLHelper_RECO_folds.reserve(4);
    for (int i = 0; i < 4; ++i) {
      myMLHelper_CLASSIF_folds.push_back(std::make_unique<MLHelper>(
          "/data6/Users/yeonjoon/SKNANOAnalyzer_NanoV15/data/spanet_version_" +
              std::to_string(i) + "_CLASSIF.onnx",
          MLHelper::ModelType::ONNX)); // 생성자 인자 있을 경우
    }
    for (int i = 0; i < 4; ++i) {
      myMLHelper_RECO_folds.push_back(std::make_unique<MLHelper>(
          "/data6/Users/yeonjoon/SKNANOAnalyzer_NanoV15/data/spanet_version_" +
              std::to_string(i) + "_RECO.onnx",
          MLHelper::ModelType::ONNX)); // 생성자 인자 있을 경우
    }
    for (int i = 0; i < 4; ++i) {
      myMLHelper_TabNet_folds.push_back(std::make_unique<MLHelper>(
          "/data6/Users/yeonjoon/SKNANOAnalyzer_NanoV15/data/tabnet_fold" +
              std::to_string(i) + ".onnx",
          MLHelper::ModelType::ONNX)); // 생성자 인자 있을 경우
    }
  } else if (channel == Channel::MM || channel == Channel::ME ||
             channel == Channel::EE) {
    std::cout << "Initialize MyCorrection for DL" << std::endl;
    btagging_R_file = "Vcb_DL_btaggingR.json";
    ctagging_R_file = "Vcb_DL_ctaggingR.json";
    myCorr = new MyCorrection(
        DataEra, DataPeriod, IsDATA ? DataStream : MCSample, IsDATA,
        btagging_eff_file, ctagging_eff_file, btagging_R_file, ctagging_R_file);
    myCorr->SetTaggingParam(activeTagger, DL_BTag_WP);
  } else { // because FH doesn't need to specify the channel
    std::cout << "Initialize MyCorrection for FH" << std::endl;
    btagging_R_file = "Vcb_FH_btaggingR.json";
    ctagging_R_file = "Vcb_FH_ctaggingR.json";
    myCorr = new MyCorrection(
        DataEra, DataPeriod, IsDATA ? DataStream : MCSample, IsDATA,
        btagging_eff_file, ctagging_eff_file, btagging_R_file, ctagging_R_file);
    myCorr->SetTaggingParam(activeTagger, FH_BTag_WP);
    myMLHelper = std::make_unique<MLHelper>(
        "/data6/Users/yeonjoon/SKNanoAnalyzer/data/Run3_v12_Run2_v9/2022EE/"
        "spanet_FH_2022EE.onnx",
        MLHelper::ModelType::ONNX);
  }

  if (IsDATA) {
    systHelper = std::make_unique<SystematicHelper>(
        "/data6/Users/yeonjoon/SKNanoAnalyzer/AnalyzerTools/noSyst.yaml",
        DataStream, DataEra);
  } else {
    systHelper = std::make_unique<SystematicHelper>(
        "/data6/Users/yeonjoon/SKNanoAnalyzer/AnalyzerTools/"
        "VcbSystematic_BTag.yaml",
        MCSample, DataEra);
  }

  CreateTrainingTree();
  CreateTemplateTrainingTree();
}

void Vcb::SetChannel() {
  if (HasFlag("Skim")){
    std::cout << "Skimming mode detected, will iterate over channel"
              << std::endl;
  }
  else if (HasFlag("MM"))
    channel = Channel::MM;
  else if (HasFlag("ME"))
    channel = Channel::ME;
  else if (HasFlag("EE"))
    channel = Channel::EE;
  else if (HasFlag("Mu"))
    channel = Channel::Mu;
  else if (HasFlag("El"))
    channel = Channel::El;
  else if (HasFlag("FH"))
    channel = Channel::FH;
  else
    channel = Channel::FH;
}

// fold helper
uint32_t Vcb::rle_bucket(uint64_t run, uint64_t lumi, uint64_t event,
                         uint32_t nbuckets) {
  auto splitmix64 = [](uint64_t x) {
    uint64_t z = (x += 0x9e3779b97f4a7c15ULL);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
  };

  uint64_t seed = run * 0x9E3779B97F4A7C15ULL ^ lumi * 0xBF58476D1CE4E5B9ULL ^
                  event * 0x94D049BB133111EBULL;

  uint64_t h = splitmix64(seed);

  __uint128_t prod = (__uint128_t)h * (__uint128_t)nbuckets;
  return (uint32_t)(prod >> 64);
};

int Vcb::Unroller(RVec<Jet> &jets) {
  if (jets.size() < 4)
    return -1;

  JetTagging::JetFlavTagger tagger = FlavTagger[DataEra.Data()];
  const std::size_t invalidIndex = jets.size();
  std::size_t leadingIndices[4] = {invalidIndex, invalidIndex, invalidIndex,
                                   invalidIndex};
  float leadingScores[4] = {-1.f, -1.f, -1.f, -1.f};

  for (std::size_t idx = 0; idx < jets.size(); ++idx) {
    const float score = jets[idx].GetTaggerResult(
        tagger, JetTagging::JetFlavTaggerScoreType::B);
    for (int position = 0; position < 4; ++position) {
      if (score > leadingScores[position]) {
        for (int shift = 3; shift > position; --shift) {
          leadingScores[shift] = leadingScores[shift - 1];
          leadingIndices[shift] = leadingIndices[shift - 1];
        }
        leadingScores[position] = score;
        leadingIndices[position] = idx;
        break;
      }
    }
  }

  if (leadingIndices[2] == invalidIndex || leadingIndices[3] == invalidIndex)
    return -1;

  const Jet &third_leading_BvsC_jet = jets[leadingIndices[2]];
  const Jet &fourth_leading_BvsC_jet = jets[leadingIndices[3]];

  int third_leading_BvsC_jet_PassedBTaggingWP =
      GetPassedBTaggingWP(third_leading_BvsC_jet);
  int fourth_leading_BvsC_jet_PassedBTaggingWP =
      GetPassedBTaggingWP(fourth_leading_BvsC_jet);
  third_leading_BvsC_jet_PassedBTaggingWP =
      third_leading_BvsC_jet_PassedBTaggingWP == 4
          ? 3
          : third_leading_BvsC_jet_PassedBTaggingWP;
  fourth_leading_BvsC_jet_PassedBTaggingWP =
      fourth_leading_BvsC_jet_PassedBTaggingWP == 4
          ? 3
          : fourth_leading_BvsC_jet_PassedBTaggingWP;
  int unrolled = third_leading_BvsC_jet_PassedBTaggingWP +
                 fourth_leading_BvsC_jet_PassedBTaggingWP * 4;
  return unrolled;
}

int Vcb::Unroller(Jet &jet1, Jet &jet2) {
  int jet1_Cat = static_cast<int>(JetCategory(jet1));
  int jet2_Cat = static_cast<int>(JetCategory(jet2));
  int unrolled = jet1_Cat + jet2_Cat * 12;
  return unrolled;
}

void Vcb::FillHistogramsAtThisPoint(std::string_view histPrefix,
                                    float weight) {
  const std::string base(histPrefix);
  std::string name;
  name.reserve(base.size() + 64);

  auto fill1d = [&](std::string_view suffix, float value, float w, int nbin,
                    float xmin, float xmax) {
    name.assign(base);
    name.push_back('/');
    name.append(suffix);
    FillHist(name, value, w, nbin, xmin, xmax);
  };

  auto fill2d = [&](std::string_view suffix, float x, float y, float w,
                    int nbinx, float xmin, float xmax, int nbiny, float ymin,
                    float ymax) {
    name.assign(base);
    name.push_back('/');
    name.append(suffix);
    FillHist(name, x, y, w, nbinx, xmin, xmax, nbiny, ymin, ymax);
  };

  fill1d("MET", MET.Pt(), weight, 50, 0.f, 200.f);
  fill1d("HT", HT, weight, 100, 200.f, 2000.f);
  fill1d("n_jets", n_jets, weight, 12, 0.f, 12.f);
  fill1d("n_b_tagged_jets", n_b_tagged_jets, weight, 8, 0.f, 8.f);
  fill1d("n_c_tagged_jets", n_c_tagged_jets, weight, 8, 0.f, 8.f);
  fill1d("n_partonFlav_b_jets", n_partonFlav_b_jets, weight, 6, 2.f, 8.f);
  fill1d("n_partonFlav_c_jets", n_partonFlav_c_jets, weight, 8, 0.f, 8.f);
  fill2d("real_b_vs_tagged_b", n_partonFlav_b_jets, n_b_tagged_jets, weight, 8,
         0.f, 8.f, 6, 2.f, 8.f);
  fill2d("real_c_vs_tagged_c", n_partonFlav_c_jets, n_c_tagged_jets, weight, 8,
         0.f, 8.f, 6, 2.f, 8.f);
  fill1d("nPVsGood", ev.nPVsGood(), weight, 70, 0.f, 70.f);

  for (size_t i = 0; i < std::min(static_cast<size_t>(5), Jets.size()); ++i) {
    const auto jetCategory = static_cast<int>(JetCategory(Jets[i]));
    const std::string idx = std::to_string(i);

    name.assign(base);
    name.append("/Jet_Pt_").append(idx);
    FillHist(name, Jets[i].Pt(), weight, 50, 0.f, 500.f);

    name.assign(base);
    name.append("/Jet_Eta_").append(idx);
    FillHist(name, Jets[i].Eta(), weight, 50, -2.5, 2.5);

    name.assign(base);
    name.append("/Jet_Phi_").append(idx);
    FillHist(name, Jets[i].Phi(), weight, 50, -3.14, 3.14);

    name.assign(base);
    name.append("/Jet_BvsC_").append(idx);
    FillHist(name, JetBScore(Jets[i]), weight, 50, 0.f, 1.f);

    name.assign(base);
    name.append("/Jet_CvsB_").append(idx);
    FillHist(name, JetCvBScore(Jets[i]), weight, 50, 0.f, 1.f);

    name.assign(base);
    name.append("/Jet_CvsL_").append(idx);
    FillHist(name, JetCvLScore(Jets[i]), weight, 50, 0.f, 1.f);

    name.assign(base);
    name.append("/Jet_QvsG_").append(idx);
    FillHist(name, JetQvGScore(Jets[i]), weight, 50, 0.f, 1.f);

    name.assign(base);
    name.append("/Jet_Category_").append(idx);
    FillHist(name, jetCategory, weight, 12, 0.f, 12.f);

    name.assign(base);
    name.append("/Jet_HFvLF_").append(idx);
    FillHist(name, JetHFvLFScore(Jets[i]), weight, 50, 0.f, 1.f);

    name.assign(base);
    name.append("/Jet_BvC_").append(idx);
    FillHist(name, JetBvCScore(Jets[i]), weight, 50, 0.f, 1.f);
  }

  for (size_t i = 0; i < leptons.size(); i++) {
    const std::string idx = std::to_string(i);

    name.assign(base);
    name.append("/Lepton_Pt_").append(idx);
    FillHist(name, leptons[i].Pt(), weight, 50, 0.f, 500.f);

    name.assign(base);
    name.append("/Lepton_Eta_").append(idx);
    FillHist(name, leptons[i].Eta(), weight, 50, -2.5, 2.5);

    name.assign(base);
    name.append("/Lepton_Phi_").append(idx);
    FillHist(name, leptons[i].Phi(), weight, 50, -3.14, 3.14);
  }

  if (leptons.size() == 2) {
    Particle ZCand = leptons[0] + leptons[1];
    fill1d("ZCand_Mass", ZCand.M(), weight, 50, 0.f, 200.f);
    fill1d("ZCand_Pt", ZCand.Pt(), weight, 50, 0.f, 200.f);
    fill1d("ZCand_Eta", ZCand.Eta(), weight, 50, -2.5, 2.5);
  }
}

void Vcb::UpdateAllJetTaggingCaches(const JetViewCollection &jets) {
  ComputeParTScores(jets, jetHFvLFAll, jetBvCAll, jetCategoryAll);
}

Vcb::Cat Vcb::JetCategory(const Jet &jet) const {
  const int originalIdx = jet.OriginalIndex();
  if (originalIdx < 0 ||
      static_cast<std::size_t>(originalIdx) >= jetCategoryAll.size())
    return Vcb::Cat::N0;
  return jetCategoryAll[static_cast<std::size_t>(originalIdx)];
}

float Vcb::JetHFvLFScore(const Jet &jet) const {
  const int originalIdx = jet.OriginalIndex();
  if (originalIdx < 0 ||
      static_cast<std::size_t>(originalIdx) >= jetHFvLFAll.size())
    return -1.f;
  return jetHFvLFAll[static_cast<std::size_t>(originalIdx)];
}

float Vcb::JetBvCScore(const Jet &jet) const {
  const int originalIdx = jet.OriginalIndex();
  if (originalIdx < 0 ||
      static_cast<std::size_t>(originalIdx) >= jetBvCAll.size())
    return -1.f;
  return jetBvCAll[static_cast<std::size_t>(originalIdx)];
}

short Vcb::GetPassedBTaggingWP(const Jet &jet) {
  const Cat category = JetCategory(jet);
  switch (category) {
  case Vcb::Cat::B4:
    return 4;
  case Vcb::Cat::B3:
    return 3;
  case Vcb::Cat::B2:
    return 2;
  case Vcb::Cat::B1:
    return 1;
  case Vcb::Cat::B0:
    return 0;
  default:
    return -1;
  }
}

short Vcb::GetPassedCTaggingWP(const Jet &jet) {
  const Cat category = JetCategory(jet);
  switch (category) {
  case Vcb::Cat::C4:
    return 4;
  case Vcb::Cat::C3:
    return 3;
  case Vcb::Cat::C2:
    return 2;
  case Vcb::Cat::C1:
    return 1;
  case Vcb::Cat::C0:
    return 0;
  default:
    return -1;
  }
}

void Vcb::executeEvent() {
  AllMuonViews = GetAllMuonViews();
  AllElectronViews = GetAllElectronViews();
  AllJetViews = GetAllJetViews();
  AllGenViews = GetAllGenViews();
  UpdateAllJetTaggingCaches(AllJetViews);
  AllGens = GetAllGens();
  AllGenJets = GetAllGenJets();
  ev = GetEvent();

  if (HasFlag("Skim")) {
    Clear();
    SkimTree();
    return;
  }
  if (HasFlag("TemplateTraining")) {
    Clear();
    for (const auto &syst_dummy : *systHelper) {
      if (!PassBaseLineSelection(false, false))
        continue;
      InferONNX();
      if (systHelper->getCurrentIterSysTarget().find("Central") !=
          std::string::npos) {
        FillTemplateTrainingTree();
        return;
      }
    }
    return;
  }
  
  if (HasFlag("Training")) {
    Clear();
    for (const auto &syst_dummy : *systHelper) {
      if (!PassBaseLineSelection(false, true))
        continue;
      if (systHelper->getCurrentIterSysTarget().find("Central") !=
          std::string::npos) {
        FillTrainingTree();
        return;
      }
    }
    return;
  }
  if (!CheckChannel()) {
    throw std::runtime_error("Invalid channel flag for this analyzer");
  }
  for (const auto &syst_dummy : *systHelper) {
    leptons.clear();
    executeEventFromParameter();
    if (HasFlag("spurious"))
      break;
  }
}

void Vcb::SetSystematicLambda(bool remove_flavtagging_sf) {
  std::unordered_map<
      std::string,
      std::variant<std::function<float(MyCorrection::variation, TString)>,
                   std::function<float()>>>
      weight_function_map;
  std::function<float(MyCorrection::variation, TString)> mu_id_lambda =
      [&](MyCorrection::variation syst, TString /*source*/) {
        return myCorr->GetMuonIDSF(Mu_ID_SF_Key[DataEra.Data()], Muons, syst);
      };
  std::function<float(MyCorrection::variation, TString)> mu_iso_lambda =
      [&](MyCorrection::variation syst, TString source) {
        return myCorr->GetMuonISOSF(Mu_Iso_SF_Key[DataEra.Data()], Muons, syst,
                                    source);
      };
  std::function<float(MyCorrection::variation, TString)> mu_trigger_lambda =
      [&](MyCorrection::variation syst, TString source) {
        return LeptonTriggerWeight(false, syst, source);
      };
  std::function<float(MyCorrection::variation, TString)> el_id_lambda =
      [&](MyCorrection::variation syst, TString /*source*/) {
        return myCorr->GetElectronIDSF(El_ID_SF_Key[DataEra.Data()], Electrons,
                                       syst);
      };
  std::function<float(MyCorrection::variation, TString)> el_recosf_lambda =
      [&](MyCorrection::variation syst, TString /*source*/) {
        return myCorr->GetElectronRECOSF(Electrons, syst);
      };
  std::function<float(MyCorrection::variation, TString)> el_trigger_lambda =
      [&](MyCorrection::variation syst, TString source) {
        return LeptonTriggerWeight(true, syst, source);
      };

  std::function<float(MyCorrection::variation, TString)> pileup_lambda =
      [&](MyCorrection::variation syst, TString source) {
        return myCorr->GetPUWeight(ev.nTrueInt(), syst, source);
      };
  auto get_subproc_name = [this]() {
    if (IsDATA)
      return "data";
    else if(MCSample.Contains("TT")){
      const int gent = genTtbarId % 100;
      if(gent >= 51 && gent <= 56)
        return "BB";
      else if(gent >= 41 && gent <= 46)
        return "CC";
      else
        return "JJ";
    }
    else
      return MCSample.Data();
  };
  std::function<float(MyCorrection::variation, TString)> MuF_lambda =
      [this, get_subproc_name](MyCorrection::variation syst, TString /*source*/) {
      const std::string subproc_name = get_subproc_name();
        switch (syst) {
        case MyCorrection::variation::up:
          return GetScaleVariation(MyCorrection::variation::up,
                                   MyCorrection::variation::nom)
                                   / modelling_patches[subproc_name].patch_ScaleVariation[4];
        case MyCorrection::variation::down:
          return GetScaleVariation(MyCorrection::variation::down,
                                   MyCorrection::variation::nom)
                                   / modelling_patches[subproc_name].patch_ScaleVariation[3];
        default:
          return 1.f;
        }
      };

  std::function<float(MyCorrection::variation, TString)> MuR_lambda =
      [this, get_subproc_name](MyCorrection::variation syst, TString /*source*/) {
      const std::string subproc_name = get_subproc_name();
        switch (syst) {
        case MyCorrection::variation::up:
          return GetScaleVariation(MyCorrection::variation::nom,
                                   MyCorrection::variation::up) / modelling_patches[subproc_name].patch_ScaleVariation[6];
        case MyCorrection::variation::down:
          return GetScaleVariation(MyCorrection::variation::nom,
                                   MyCorrection::variation::down) / modelling_patches[subproc_name].patch_ScaleVariation[1];
        default:
          return 1.f;
        }
      };

  std::function<float(MyCorrection::variation, TString)> ISR_lambda =
      [this, get_subproc_name](MyCorrection::variation syst, TString /*source*/) {
        const std::string subproc_name = get_subproc_name();
        switch (syst) {
        case MyCorrection::variation::up:
          return GetPSWeight(MyCorrection::variation::up,
                             MyCorrection::variation::nom) * modelling_patches[subproc_name].patch_PSVariation[0];
        case MyCorrection::variation::down:
          return GetPSWeight(MyCorrection::variation::down,
                             MyCorrection::variation::nom) * modelling_patches[subproc_name].patch_PSVariation[2];
        default:
          return 1.f;
        }
      };

  std::function<float(MyCorrection::variation, TString)> FSR_lambda =
      [this, get_subproc_name](MyCorrection::variation syst, TString /*source*/) {
        switch (syst) {
        case MyCorrection::variation::up:
          return GetPSWeight(MyCorrection::variation::nom,
                             MyCorrection::variation::up) * modelling_patches[get_subproc_name()].patch_PSVariation[1];
        case MyCorrection::variation::down:
          return GetPSWeight(MyCorrection::variation::nom,
                             MyCorrection::variation::down) * modelling_patches[get_subproc_name()].patch_PSVariation[3];
        default:
          return 1.f;
        }
      };

  std::function<float(MyCorrection::variation, TString)> BTag_lambda =
      [&](MyCorrection::variation syst, TString source) {
        float weight = 1.f;
        weight *= myCorr->GetBTaggingSF(
            Jets, JetTagging::JetTaggingSFMethod::shape, syst, source);
        weight *= myCorr->GetBTaggingR(Jets, Sample_Shorthand[MCSample.Data()],
                                       syst, source);
        return weight;
      };

  std::function<float()> top_pt_reweight_lambda =
    [this, get_subproc_name]() {
        if (!MCSample.Contains("TT"))
            return 1.f;

        auto [firstTopIdx, firstAntiTopIdx, lastTopIdx, lastAntiTopIdx] =
            GetTopAndAntiTopIndices(AllGenViews);

        const TLorentzVector top     = AllGenViews[firstTopIdx].P4();
        const TLorentzVector antiTop = AllGenViews[firstAntiTopIdx].P4();

        const float w_toppt = myCorr->GetTopPtReweight(top, antiTop);
        const float patch   = modelling_patches[get_subproc_name()].patch_minnlo;

        return w_toppt / patch;
    };

  std::function<float(MyCorrection::variation, TString)> hDamp_lambda = 
    [this, get_subproc_name](MyCorrection::variation syst, TString /*source*/) {
      if(!MCSample.Contains("TT")) return 1.f;
      const std::string subproc_name = get_subproc_name();
      auto [firstTopIdx, firstAntiTopIdx, lastTopIdx, lastAntiTopIdx] =
          GetTopAndAntiTopIndices(AllGenViews);
      const TLorentzVector FirstCopyTop = AllGenViews[firstTopIdx].P4();
      const TLorentzVector FirstCopyAntiTop = AllGenViews[firstAntiTopIdx].P4();
      switch (syst) {
      case MyCorrection::variation::up:
        return myCorr->GethDampReweight(FirstCopyTop, FirstCopyAntiTop, syst)
               / modelling_patches[subproc_name].patch_hdamp_up;
      case MyCorrection::variation::down:
        return myCorr->GethDampReweight(FirstCopyTop, FirstCopyAntiTop, syst)
               / modelling_patches[subproc_name].patch_hdamp_down;
      default:
        return 1.f;
      }
    };

  std::function<float(MyCorrection::variation, TString)> dummy_lambda =
      [&](MyCorrection::variation, TString /*source*/) { return 1.f; };

  std::function<float()> dummy_oneside_lambda = [&]() { return 1.f; };

  weight_function_map["Mu_ID"] = mu_id_lambda;
  weight_function_map["Mu_Iso"] = mu_iso_lambda;
  weight_function_map["Mu_Trig"] = mu_trigger_lambda;
  weight_function_map["El_ID"] = el_id_lambda;
  weight_function_map["El_Reco"] = el_recosf_lambda;
  weight_function_map["El_Trig"] = el_trigger_lambda;
  weight_function_map["Pileup"] = pileup_lambda;
  weight_function_map["MuF"] = MuF_lambda;
  weight_function_map["MuR"] = MuR_lambda;
  weight_function_map["ISR"] = ISR_lambda;
  weight_function_map["FSR"] = FSR_lambda;
  weight_function_map["Top_Pt_Reweight"] = dummy_oneside_lambda;
  if (remove_flavtagging_sf)
    weight_function_map["btag"] = dummy_lambda;
  else
    weight_function_map["btag"] = dummy_lambda;
  systHelper->assignWeightFunctionMap(weight_function_map);
}


array<size_t, 4> Vcb::GetTopAndAntiTopIndices(const GenViewCollection &gens) {
    constexpr size_t npos = std::numeric_limits<size_t>::max();

    size_t FirstCopyTopIndex     = npos;
    size_t FirstCopyAntiTopIndex = npos;
    size_t LastCopyTopIndex      = npos;
    size_t LastCopyAntiTopIndex  = npos;

    const size_t n = gens.size();

    constexpr unsigned long FIRST_COPY_BIT = 1UL << 12;
    constexpr unsigned long LAST_COPY_BIT  = 1UL << 13;

    for (size_t idx = 0; idx < n; ++idx) {
        const GenView &gen = gens[idx];

        const int pdg = gen.PdgId();
        const auto flags = gen.StatusFlags();

        const bool isFirstCopy = (flags & FIRST_COPY_BIT) != 0;
        const bool isLastCopy  = (flags & LAST_COPY_BIT)  != 0;

        if (pdg == 6) {  // top
            if (isFirstCopy) {
                assert(FirstCopyTopIndex == npos && "Multiple first-copy tops found in event");
                FirstCopyTopIndex = idx;
            }
            if (isLastCopy) {
                assert(LastCopyTopIndex == npos && "Multiple last-copy tops found in event");
                LastCopyTopIndex = idx;
            }
        } else if (pdg == -6) {  // anti-top
            if (isFirstCopy) {
                assert(FirstCopyAntiTopIndex == npos && "Multiple first-copy antitops found in event");
                FirstCopyAntiTopIndex = idx;
            }
            if (isLastCopy) {
                assert(LastCopyAntiTopIndex == npos && "Multiple last-copy antitops found in event");
                LastCopyAntiTopIndex = idx;
            }
        }
    }

    assert(FirstCopyTopIndex     != npos && "No first-copy top found in event");
    assert(FirstCopyAntiTopIndex != npos && "No first-copy antitop found in event");
    assert(LastCopyTopIndex      != npos && "No last-copy top found in event");
    assert(LastCopyAntiTopIndex  != npos && "No last-copy antitop found in event");

    return {FirstCopyTopIndex,
            FirstCopyAntiTopIndex,
            LastCopyTopIndex,
            LastCopyAntiTopIndex};
}

void Vcb::Clear() {
  HT = 0;
  n_jets = 0;
  n_b_tagged_jets = 0;
  n_c_tagged_jets = 0;
  n_hf_jets = 0;
  n_partonFlav_b_jets = 0;
  n_partonFlav_c_jets = 0;
  find_all_jets = false;
}

void Vcb::executeEventFromParameter() {
  Clear();
  if (!PassBaseLineSelection())
    return;

  const std::string channel_str = GetChannelString(channel).Data();
  // Inclusive만 사용
  const std::string base_path = channel_str + "/Inclusive/";

  // 샘플 후미(postfix) 결정
  std::string sample_postfix = Sample_Shorthand[MCSample.Data()];
  if (MCSample.Contains("TT") && !MCSample.Contains("Vcb")) {
    sample_postfix += GetTTHFPostFix(); // TTbb/TTcc/… 후미
  }
  //InferONNX();
  //InferTabNet();

  // -------------------------
  // DATA: Inclusive/Central/data_obs 만 채움
  // -------------------------
  if (IsDATA) {
    FillHistogramsAtThisPoint(base_path + "Central/data_obs", 1.f);
    //FillONNXRecoInfo(base_path + "Central/data_obs", 1.f);
    //FillTabNetInfo(base_path + "Central/data_obs", 1.f);
    return;
  }

  // -------------------------
  // MC: 프리북(0 weight) → 가중치 채움
  // -------------------------
  // 1) 프리북: 시스템틱 키 전부에 대해 0으로 한 번씩 채워서 히스토그램 생성
  {
    const auto prebook_weights =
        systHelper->calculateWeight(true); // skeleton only
    // Central이 calculateWeight(true)에 없을 수 있으니 명시적으로 한 번 프리북
    FillHistogramsAtThisPoint(base_path + "Central/" + sample_postfix, 0.f);
    for (const auto &kv : prebook_weights) {
      const std::string &syst = kv.first;
      FillHistogramsAtThisPoint(base_path + syst + "/" + sample_postfix, 0.f);
    }
  }

  // 2) 실제 채움: normalization × 각 시스템틱 가중치
  const float normalization = MCNormalization();
  const auto weight_map = systHelper->calculateWeight(false);
  for (const auto &kv : weight_map) {
    const std::string &syst = kv.first;
    const float w = kv.second;
    FillHistogramsAtThisPoint(base_path + syst + "/" + sample_postfix,
                              w * normalization);
   // FillONNXRecoInfo(base_path + syst + "/" + sample_postfix, w * normalization);
    //FillTabNetInfo(base_path + syst + "/" + sample_postfix,              w * normalization);
  }
}

void Vcb::SetTTbarId() {
  if (!IsDATA) {
    ttbj = (genTtbarId % 100 == 51 || genTtbarId % 100 == 52);
    ttbb = (genTtbarId % 100 >= 53 && genTtbarId % 100 <= 55);
    ttcc = (genTtbarId % 100 >= 41 && genTtbarId % 100 <= 45);
    ttLF = !(ttbj || ttbb || ttcc);
  }
}

void Vcb::CreateTrainingTree() {}
void Vcb::CreateTemplateTrainingTree() {}

void Vcb::FillTrainingTree() {}
void Vcb::FillTemplateTrainingTree() {}

RVec<int> Vcb::FindTTbarJetIndices() {
  RVec<int> iamnothing;
  return iamnothing;
}

void Vcb::FillKinematicFitterResult(const TString &histPrefix, float weight) {}

RVec<RVec<unsigned int>> Vcb::GetPermutations(const RVec<Jet> &jets) {
  RVec<RVec<unsigned int>> iamnothing;
  return iamnothing;
}

void Vcb::SkimTree() {
  Clear();
  if (!skimTreeInitialized) {
    RVec<TString> keeps = {"*"};
    RVec<TString> drops = {};
    //NewTree("Events", keeps, drops); // Placeholder; filled in WriteHist
    skimTreeInitialized = true;
  }

  RVec<Channel> all_channels = {Channel::MM, Channel::ME, Channel::EE,
                                Channel::Mu, Channel::El, Channel::FH};
  for (const auto &ch : all_channels) {
    this->channel = ch;
    if (!CheckChannel())
      continue;
    for (const auto &syst_dummy : *systHelper) {
      Clear();
      leptons.clear();
      if (PassBaseLineSelection(false, false)) {
        // Record global entry; actual copy is deferred to WriteHist.
        skim_passed_global_entries.push_back(currentEntry);
        return; // Exit immediately upon success
      }
    }
  }

  // If no channel passed for all systematic variations
  return;
}

void Vcb::WriteHist() {
  if (HasFlag("Skim")) {
    if (!skim_passed_global_entries.empty() && fChain) {

      // 1) 글로벌 엔트리 정렬 + 중복 제거 (순차 I/O 위해)
      std::sort(skim_passed_global_entries.begin(),
                skim_passed_global_entries.end());
      skim_passed_global_entries.erase(
          std::unique(skim_passed_global_entries.begin(),
                      skim_passed_global_entries.end()),
          skim_passed_global_entries.end());

      // 2) BranchManager가 쥐고 있던 주소 정리 (너 프레임워크에 맞게)
      //    예: BranchBase::ResetAllBranchAddresses(); 같은 함수 있으면 호출
      // BranchBase::ResetAllBranchAddresses(); // TODO: 네 쪽 이름 맞춰서

      // 3) 모든 브랜치 활성화 + ROOT가 버퍼 관리
      fChain->SetBranchStatus("*", 1);
      fChain->ResetBranchAddresses();

      // 4) 첫 트리에 대해 캐시/프리페치 설정
      if (TTree *firstTree = fChain->GetTree()) {
        configureTreeCache(firstTree);  // SKNanoLoader 멤버
      }

      // 5) 구조만 복사한 빈 트리 생성 (fast 모드)
      std::unique_ptr<TTree> skimTree(fChain->CloneTree(0, "fast"));
      if (skimTree) {
        skimTree->SetName("Events");

        Long64_t prevTreeNumber = -1;

        // 6) 선택된 글로벌 엔트리들만 순회
        for (Long64_t g : skim_passed_global_entries) {
          // 체인의 해당 글로벌 엔트리로 이동 (트리/파일 자동 전환)
          Long64_t local = fChain->LoadTree(g);
          if (local < 0) continue;

          TTree *tree = fChain->GetTree();
          if (!tree) continue;

          // 트리가 바뀔 때마다 캐시 재설정
          if (tree->GetTreeNumber() != prevTreeNumber) {
            configureTreeCache(tree);
            prevTreeNumber = tree->GetTreeNumber();
          }

          // 실제 데이터 로드
          if (tree->GetEntry(local) <= 0) continue;

          // CopyTree가 하던 일을 우리가 직접 Fill
          skimTree->Fill();
        }

        treemap["Events"] = skimTree.release();
      }

      skim_passed_global_entries.clear();
    }
  }

  AnalyzerCore::WriteHist();
}

float Vcb::LeptonTriggerWeight(bool isEle, const MyCorrection::variation syst,
                               const TString & /*source*/) {
  MyCorrection::variation electronVariation = MyCorrection::variation::nom;
  MyCorrection::variation muonVariation = MyCorrection::variation::nom;

  if (syst != MyCorrection::variation::nom) {
    if (isEle)
      electronVariation = syst;
    else
      muonVariation = syst;
  }

  switch (channel) {
  case Channel::FH:
    return 1.f;
  case Channel::Mu:
    if (isEle)
      return 1.f;
    return myCorr->GetMuonTriggerSF(Mu_Trigger_SF_Key[DataEra.Data()], Muons[0],
                                    muonVariation);
  case Channel::El:
    if (!isEle)
      return 1.f;
    return myCorr->GetElectronTriggerSF(El_Trigger_SF_Key[DataEra.Data()],
                                        lepton.Eta(), lepton.Pt(), lepton.Phi(),
                                        electronVariation);
  case Channel::MM:
    if (isEle)
      return 1.f;
    {
      double num = 1.f;
      double den = 1.f;
      for (const auto &mu : Muons) {
        num *= (1.f - 0.93 * myCorr->GetMuonTriggerSF(
                                 Mu_Trigger_SF_Key[DataEra.Data()], mu,
                                 muonVariation));
        den *= (1.f - 0.93); // hardcoded: let MCEff = 1, DataEff = SF, MUON POG
                             // PLEASE GIVE ME EFFICIENCY
      }
      return (1. - num) / (1. - den);
    }
  case Channel::ME: {
    double num = 1.f;
    double den = 1.f;
    num *= (1.f -
            0.93 * myCorr->GetMuonTriggerSF(Mu_Trigger_SF_Key[DataEra.Data()],
                                            Muons[0], muonVariation));
    den *= (1.f - 0.93); // hardcoded: let MCEff = 1, DataEff = SF, MUON POG
                         // PLEASE GIVE ME EFFICIENCY
    num *=
        (1.f - myCorr->GetElectronTriggerDataEff(
                   El_Trigger_SF_Key[DataEra.Data()], Electrons[0].Eta(),
                   Electrons[0].Pt(), Electrons[0].Phi(), electronVariation));
    den *=
        (1.f - myCorr->GetElectronTriggerMCEff(
                   El_Trigger_SF_Key[DataEra.Data()], Electrons[0].Eta(),
                   Electrons[0].Pt(), Electrons[0].Phi(), electronVariation));
    return (1. - num) / (1. - den);
  }
  case Channel::EE:
    if (!isEle)
      return 1.f;
    {
      double num = 1.f;
      double den = 1.f;
      for (const auto &el : Electrons) {
        num *= (1.f - myCorr->GetElectronTriggerDataEff(
                          El_Trigger_SF_Key[DataEra.Data()], el.Eta(), el.Pt(),
                          el.Phi(), electronVariation));
        den *= (1.f - myCorr->GetElectronTriggerMCEff(
                          El_Trigger_SF_Key[DataEra.Data()], el.Eta(), el.Pt(),
                          el.Phi(), electronVariation));
      }
      return (1. - num) / (1. - den);
    }
  default:
    throw std::runtime_error("[Vcb::LeptonTriggerWeight] Invalid channel");
  }
}

void Vcb::InferONNX() {}
void Vcb::InferTabNet() {}

bool Vcb::FillONNXRecoInfo(const TString &histPrefix, float weight) {
  return true;
}

bool Vcb::FillTabNetInfo(const TString &histPrefix, float weight) {
  return true;
}
