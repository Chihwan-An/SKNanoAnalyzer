#ifndef CalibrationTree_h 
#define CalibrationTree_h

#include <array>
#include <cstddef>
#include <functional>
#include <future>
#include <memory>
#include <set>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <variant>
#include <vector>

#include "AnalyzerCore.h"
#include "Gen.h"
#include "GenView.h"
#include "MLHelper.h"
#include "SystematicHelper.h"
#include "TrigObjView.h"
#include "VcbParameters.h"

using FloatArray = std::vector<float>;
using FloatArrays = std::vector<FloatArray>;
using IntArray = std::vector<int>;
using BoolArray = std::vector<uint8_t>;
using VariousArray = std::variant<FloatArray, IntArray, BoolArray>;

class CalibrationTree : public AnalyzerCore {
public:
  enum class Channel { TTDilep, WCharm_Mu, WCharm_El, DYLight, TTSemilep};
 
  void Clear();
  void SetChannel();
  void initializeAnalyzer() override;
  void executeEvent() override;
  virtual void executeEventFromParameter();
  bool PassBaseLineSelection();
  bool PassTTDilepBaselineSelection();
  bool PassWCharmBaselineSelection();
  bool PassDYLightBaselineSelection();
  bool PassTTSemilepBaselineSelection();
  void virtual FillHistogramsAtThisPoint(std::string_view histPrefix,
                                         float weight = 1.f);
  void virtual FillTreeAtThisPoint(
      std::string_view treePrefix, float MCNormalizationWeight,
      const std::unordered_map<std::string, float> &weight_map);
  void SetSystematicLambda(bool remove_flavtagging_sf = false);
  inline float virtual MCNormalization() {
    return MCweight() * ev.GetTriggerLumi(Mu_Trigger[DataEra.Data()]);
  }

  CalibrationTree();
  virtual ~CalibrationTree() = default;

  // Objects
  MuonViewCollection AllMuonViews;
  ElectronViewCollection AllElectronViews;
  JetViewCollection AllJetViews;
  GenViewCollection AllGenViews;
  TrigObjViewCollection AllTrigObjViews;
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
  // event info
  float HT;
  short n_jets;
  short n_b_tagged_jets;
  short n_c_tagged_jets;
  short n_hf_jets;
  short n_hadronFlav_b_jets;
  short n_hadronFlav_c_jets;

  bool skimTreeInitialized = false;
  std::vector<Long64_t> skim_passed_global_entries;
  std::unique_ptr<SystematicHelper> systHelper;
  std::vector<float> jetHFvLFAll;
  std::vector<float> jetBvCAll;
  float LeptonTriggerWeight(bool isEle, const MyCorrection::variation syst,
                            const TString &source);


  inline TString GetChannelString(Channel ch) {
    switch (ch) {
    case Channel::TTDilep:
      return "TTDilep";
    case Channel::WCharm_Mu:
      return "WCharm_Mu";
    case Channel::WCharm_El:
      return "WCharm_El";
    case Channel::DYLight:
      return "DYLight";
    case Channel::TTSemilep:
      return "TTSemilep";
    default:
      return "Unknown";
    }
  }

  Channel channel;
};

#endif
