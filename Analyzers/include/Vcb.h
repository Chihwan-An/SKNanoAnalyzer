#ifndef Vcb_h
#define Vcb_h

#include <array>
#include <cstddef>
#include <functional>
#include <future>
#include <memory>
#include <string_view>
#include <set>
#include <tuple>
#include <variant>

#include "AnalyzerCore.h"
#include "Gen.h"
#include "GenView.h"
#include "MLHelper.h"
#include "SystematicHelper.h"
#include "TFitConstraintEp.h"
#include "TFitConstraintM.h"
#include "TFitConstraintMGaus.h"
#include "TFitParticleEtEtaPhi.h"
#include "TFitParticleMCCart.h"
#include "TFitParticlePt.h"
#include "TKinFitter.h"
#include "TMatrixD.h"
#include "VcbParameters.h"

using FloatArray = std::vector<float>;
using FloatArrays = std::vector<FloatArray>;
using IntArray = std::vector<int>;
using BoolArray = std::vector<uint8_t>;
using VariousArray = std::variant<FloatArray, IntArray, BoolArray>;

class Vcb : public AnalyzerCore {
public:
  void load_modelling_json(const TString &filename);
  uint32_t rle_bucket(uint64_t run, uint64_t lumi, uint64_t event,
                      uint32_t nbuckets);
  enum class Cat { N0, L0, C0, C1, C2, C3, C4, B0, B1, B2, B3, B4 };
  inline bool isPIDUpTypeQuark(int pdg) {
    return (abs(pdg) == 2 || abs(pdg) == 4 || abs(pdg) == 6);
  }
  inline bool isPIDDownTypeQuark(int pdg) {
    return (abs(pdg) == 1 || abs(pdg) == 3 || abs(pdg) == 5);
  }
  inline bool isPIDLepton(int pdg) {
    return (abs(pdg) == 11 || abs(pdg) == 13 || abs(pdg) == 15);
  }
  inline bool isPIDNeutrino(int pdg) {
    return (abs(pdg) == 12 || abs(pdg) == 14 || abs(pdg) == 16);
  }
  inline bool isDaughterOf(int idx, int m_idx) {
    while (idx >= 0) {
      int midx = AllGens[idx].MotherIndex();
      if (midx < 0)
        break; // No valid mother
      if (midx == m_idx)
        return true; // Found the W in the chain
      idx = midx;
    }
    return false;
  };
  inline int GetSimplePID(int pid, bool usesign = false) {
    int simple_pid = 0;
    if (abs(pid) == 5)
      simple_pid = 5;
    else if (abs(pid) == 4)
      simple_pid = 4;
    else
      simple_pid = 0;
    if (!usesign)
      return simple_pid;
    if (pid < 0)
      return -simple_pid;
    return simple_pid;
  }
  array<std::size_t, 4> GetTopAndAntiTopIndices(const GenViewCollection &gens);
  float LeptonTriggerWeight(
      bool isEle,
      const MyCorrection::variation syst = MyCorrection::variation::nom,
      const TString &source = "total");
  void Clear();
  int Unroller(RVec<Jet> &jets);
  int Unroller(Jet &jet1, Jet &jet2);
  short GetPassedBTaggingWP(const Jet &jet);
  short GetPassedCTaggingWP(const Jet &jet);
  void SetChannel();
  bool virtual CheckChannel() = 0;
  void initializeAnalyzer() override;
  void executeEvent() override;
  virtual void executeEventFromParameter();
  bool virtual PassBaseLineSelection(bool remove_flavtagging_cut = false,
                                     bool loose_cut = false) = 0;
  void virtual FillHistogramsAtThisPoint(std::string_view histPrefix,
                                         float weight = 1.f);
  void virtual FillKinematicFitterResult(const TString &histPrefix,
                                         float weight);
  void virtual SkimTree();
  void SetTTbarId();
  void SetSystematicLambda(bool remove_flavtagging_sf = false);
  void virtual CreateTrainingTree();
  void virtual CreateTemplateTrainingTree();
  RVec<int> virtual FindTTbarJetIndices();
  void virtual FillTrainingTree();
  void virtual FillTemplateTrainingTree();
  float virtual MCNormalization() = 0;
  void virtual InferONNX();
  void virtual InferTabNet();
  std::string virtual GetRegionString() = 0;
  void WriteHist() override;
  inline size_t FindNthMaxIndex(FloatArray &array, int ranking) {
    if (ranking < 0 || static_cast<size_t>(ranking) >= array.size()) {
      throw std::out_of_range("Ranking is out of bounds");
    }
    std::vector<size_t> indices(array.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::sort(indices.begin(), indices.end(),
              [&array](size_t i1, size_t i2) { return array[i1] > array[i2]; });
    return indices[ranking];
  }
  inline std::vector<int> UnravelIndex(int idx, std::vector<int> shape) {
    std::vector<int> indices(shape.size());
    for (int i = shape.size(); i-- > 0;) {
      indices[i] = idx % shape[i];
      idx /= shape[i];
    }
    return indices;
  }
  inline std::string GetTTHFPostFix() {
    if (ttbj || ttbb)
      return "+B";
    if (ttcc)
      return "+C";
    if (ttLF)
      return "+LF";
    return "";
  }

  bool virtual FillONNXRecoInfo(const TString &histPrefix, float weight);
  bool virtual FillTabNetInfo(const TString &histPrefix, float weight);
  inline float GetJetEnergyFractionWithRadius(Jet &jet, float radius) {
    float sum = 0;
    for (size_t i = 0; i < Jets.size(); i++) {
      if (jet.DeltaR(Jets[i]) < 1e-5)
        continue;
      if (jet.DeltaR(Jets[i]) < radius)
        sum += Jets[i].E();
    }
    return jet.E() / sum;
  }

  RVec<RVec<unsigned int>> virtual GetPermutations(const RVec<Jet> &jets);
  inline JetTagging::JetFlavTagger CurrentFlavTagger() const {
    return FlavTagger[DataEra.Data()];
  }
  inline float JetBScore(const Jet &jet) const {
    return jet.GetTaggerResult(CurrentFlavTagger(),
                               JetTagging::JetFlavTaggerScoreType::B);
  }
  inline float JetCvBScore(const Jet &jet) const {
    return jet.GetTaggerResult(CurrentFlavTagger(),
                               JetTagging::JetFlavTaggerScoreType::CvB);
  }
  inline float JetCvLScore(const Jet &jet) const {
    return jet.GetTaggerResult(CurrentFlavTagger(),
                               JetTagging::JetFlavTaggerScoreType::CvL);
  }
  inline float JetQvGScore(const Jet &jet) const {
    return jet.GetTaggerResult(CurrentFlavTagger(),
                               JetTagging::JetFlavTaggerScoreType::QvG);
  }
  inline std::pair<float, float> JetCScorePair(const Jet &jet) const {
    return {JetCvBScore(jet), JetCvLScore(jet)};
  }
  float JetHFvLFScore(const Jet &jet) const;
  float JetBvCScore(const Jet &jet) const;
  Cat JetCategory(const Jet &jet) const;
  void UpdateAllJetTaggingCaches(const JetViewCollection &jets);
  Vcb();
  virtual ~Vcb() = default;

  // modelling constant
  struct ModellingPatch {
    std::vector<float> patch_ScaleVariation;
    std::vector<float> patch_PSVariation;
    float patch_hdamp_up = 0.f;
    float patch_hdamp_down = 0.f;
    float patch_minnlo = 0.f;
  };

  std::unordered_map<std::string, ModellingPatch> modelling_patches;
  // Objects
  MuonViewCollection AllMuonViews;
  ElectronViewCollection AllElectronViews;
  JetViewCollection AllJetViews;
  GenViewCollection AllGenViews;
  RVec<Gen> AllGens;
  RVec<LHE> AllLHEs;
  RVec<GenJet> AllGenJets;
  // Selected Objects
  RVec<Jet> Jets;
  RVec<Electron> Electrons_Veto;
  RVec<Electron> Electrons;
  RVec<Muon> Muons_Veto;
  RVec<Muon> Muons;
  Lepton lepton;
  RVec<Lepton> leptons;
  Event ev;
  Particle MET;
  RVec<int> ttbar_jet_indices;
  RVec<int> ttbar_AllGenJets_indices;
  // event info
  float HT;
  short n_jets;
  short n_b_tagged_jets;
  short n_c_tagged_jets;
  short n_hf_jets;
  short n_partonFlav_b_jets;
  short n_partonFlav_c_jets;
  bool find_all_jets;
  bool find_all_genjets;
  int tt_decay_code;
  bool ttbj;
  bool ttbb;
  bool ttcc;
  bool ttLF;
  bool skimTreeInitialized = false;
  std::vector<Long64_t> skim_passed_global_entries;
  std::unique_ptr<SystematicHelper> systHelper;
  std::vector<float> jetHFvLFAll;
  std::vector<float> jetBvCAll;
  std::vector<Vcb::Cat> jetCategoryAll;

  enum class Channel { MM, ME, EE, Mu, El, FH };

  inline TString GetChannelString(Channel ch) {
    switch (ch) {
    case Channel::MM:
      return "MM";
    case Channel::ME:
      return "ME";
    case Channel::EE:
      return "EE";
    case Channel::Mu:
      return "Mu";
    case Channel::El:
      return "El";
    case Channel::FH:
      return "FH";
    default:
      return "";
    }
  }

  Channel channel;

  std::unique_ptr<MLHelper> myMLHelper;
  std::vector<std::unique_ptr<MLHelper>> myMLHelper_RECO_folds;
  std::vector<std::unique_ptr<MLHelper>> myMLHelper_CLASSIF_folds;
  std::vector<std::unique_ptr<MLHelper>> myMLHelper_TabNet_folds;
  // json holder for modelling patch
  nlohmann::json modelling_json;
};

#endif
