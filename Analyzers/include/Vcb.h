#ifndef Vcb_h
#define Vcb_h

#include <array>
#include <cstddef>
#include <functional>
#include <future>
#include <memory>
#include <set>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

#include "AnalyzerCore.h"
#include "Gen.h"
#include "GenView.h"
#include "MLHelper.h"
#include "OtJsonLutBank.h"
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

namespace UParTScore {
enum class Cat : int;
struct Prob3;
} // namespace UParTScore

class Vcb : public AnalyzerCore {
public:
  using Prob3 = UParTScore::Prob3;
  uint32_t rle_bucket(uint64_t run, uint64_t lumi, uint64_t event,
                      uint32_t nbuckets);

  void rle_bucket_compute_checksum();

  using Cat = UParTScore::Cat;
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
  void virtual FillTreeAtThisPoint(
      std::string_view treePrefix, float MCNormalizationWeight,
      const std::unordered_map<std::string, float> &weight_map);
  void virtual FillKinematicFitterResult(const TString &histPrefix,
                                         float weight);
  std::pair<double, double> HFvLF_BvC_from_storage(const JetSoA &store,
                                                   std::size_t idx);
  void virtual SkimTree();
  void SetTTbarId();
  void SetSystematicLambda(bool remove_flavtagging_sf = false);
  void virtual CreateTrainingTree();
  void virtual CreateTemplateTrainingTree();
  RVec<int> virtual FindTTbarJetIndices();
  void virtual FillTrainingTree();
  void virtual FillTemplateTrainingTree(const std::unordered_map<std::string, float> &weight_map);
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
  float JetProbBScore(const Jet &jet) const;
  float JetProbCScore(const Jet &jet) const;
  float JetProbLScore(const Jet &jet) const;
  float JetILRdim1Score(const Jet &jet) const;
  float JetILRdim2Score(const Jet &jet) const;
  
  Cat JetCategory(const Jet &jet) const;
  void UpdateAllJetTaggingCaches(const JetViewCollection &jets);
  void UpdateAllJetTaggingCaches(
      const JetViewCollection &jets,
      const std::vector<std::size_t> &selected_indices);
  Vcb();
  virtual ~Vcb() = default;

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
  short n_loose_b_tagged_jets;
  short n_loose_c_tagged_jets;
  short n_hf_jets;
  short n_loose_hf_jets;
  short n_hadronFlav_b_jets;
  short n_hadronFlav_c_jets;
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
  mutable std::vector<float> jetHFvLFAll;
  mutable std::vector<float> jetBvCAll;
  mutable std::vector<Vcb::Cat> jetCategoryAll;
  mutable std::vector<float> jetProbBAll;
  mutable std::vector<float> jetProbCAll;
  mutable std::vector<float> jetProbLAll;
  mutable std::vector<float> jetILRdim1All;
  mutable std::vector<float> jetILRdim2All;
  mutable bool partInputBranchesValidated = false;

  struct TreeKinematicsBuffers {
    std::vector<float> Jet_Pt;
    std::vector<float> Jet_Eta;
    std::vector<float> Jet_Phi;
    std::vector<float> Jet_Mass;
    std::vector<int> Jet_Category;
    std::vector<float> Jet_ILR_Dim_1;
    std::vector<float> Jet_ILR_Dim_2;
    std::vector<float> Lepton_Pt;
    std::vector<float> Lepton_Eta;
    std::vector<float> Lepton_Phi;
    std::vector<float> Lepton_Mass;
    float Met_Pt = 0.f;
    float Met_Eta = 0.f;
    float Met_Phi = 0.f;
    float HT = 0.f;
    int n_jets = 0;
    int n_b_tagged_jets = 0;
    int n_c_tagged_jets = 0;
    int nPVsGood = 0;
    float ZCand_Mass = -999.f;
    float ZCand_Pt = -999.f;
    float ZCand_Eta = -999.f;
    float MCNormalization = -999.f;
    bool booked = false;
    std::vector<std::string> weight_keys;
    std::vector<float> weight_values;
    std::unordered_map<std::string, std::size_t> weight_index;
    int fold_idx_spanet = -999;
    int fold_idx_tabnet = -999;
  };
  std::unordered_map<std::string, TreeKinematicsBuffers> tree_buffers;

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

  std::unique_ptr<OtJsonLutBank> UParT_OT_Central;
  std::unordered_map<std::string, std::unique_ptr<OtJsonLutBank>>
      UParT_OT_ByPath;
  std::unordered_set<std::string> UParT_OT_WarnedOnce;
  const OtJsonLutBank *UParT_OT_SystActive = nullptr;
  std::string UParT_OT_Central_Path =
      "/data6/Users/yeonjoon/OptimalTransport/OTv8/LUTv8_integrated.json.gz";

protected:
  // --- pure helpers: static
  static int bin_hf(double x);
  static int bin_bvc(double y);
  const OtJsonLutBank *CurrentOtLut() const;
  void UpdateActiveOtLutForCurrentSystematic();
  const OtJsonLutBank *GetOrLoadOtLut(const std::string &json_path,
                                      const std::string &bundle_key = "");
  std::string BuildSystOtLutKey(const std::string &source,
                                MyCorrection::variation variation) const;
  float OtLutPtFromStore(const JetSoA &store, std::size_t idx) const;

  std::pair<double, double> HFvLF_BvC_from_components(double probudg,
                                                      double SvUDG, double CvL,
                                                      double CvB, float pt,
                                                      int hadronFlavor) const;

  // --- Vcb 컨텍스트에서 쓰도록 멤버로 (지금은 멤버 접근 안 해도 OK)
  std::pair<double, double> HFvLF_BvC_from_ParT(const Jet &j) const;
  std::pair<double, double> HFvLF_BvC_from_storage(const JetSoA &store,
                                                   std::size_t idx) const;

  Cat classify_from_scores(double hf, double bvc) const;
  Cat classify_from_storage(const JetSoA &store, std::size_t idx) const;



  void ComputeParTScores(const JetViewCollection &jets,
                         std::vector<float> &hfScores,
                         std::vector<float> &bvcScores,
                         std::vector<Vcb::Cat> &categories,
                         const std::vector<std::size_t> *selected_indices =
                             nullptr) const;
  static inline double clip(double x, double lo, double hi);



  static Prob3 sanitize_prob3(double pb, double pc, double pl,
                              double eps = 1e-12);
  static Prob3 compute_prob3_from_branches(double p_udg, double CvL, double CvB,
                                           double SvUDG, double eps = 1e-12);
  static std::pair<double, double> hf_bvc_from_prob3(const Prob3 &p,
                                                     double eps = 1e-12);

  // 네가 실제로 외부에서 쓰는 핵심 API(캐시 채우는 쪽에서 호출)
  Prob3 MappedProb3_from_components(double probudg, double SvUDG, double CvL,
                                    double CvB, float pt,
                                    int hadronFlavor) const;
};

#endif
