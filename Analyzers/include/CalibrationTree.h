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
#include "TFitConstraintEp.h"
#include "TFitConstraintM.h"
#include "TFitConstraintMGaus.h"
#include "TFitParticleEtEtaPhi.h"
#include "TFitParticleMCCart.h"
#include "TFitParticlePt.h"
#include "TKinFitter.h"
#include "TrigObjView.h"
#include "VcbParameters.h"

using FloatArray = std::vector<float>;
using FloatArrays = std::vector<FloatArray>;
using IntArray = std::vector<int>;
using BoolArray = std::vector<uint8_t>;
using VariousArray = std::variant<FloatArray, IntArray, BoolArray>;

class CalibrationTree : public AnalyzerCore {
public:
  enum class Channel { TTDilep, WCharm_Mu, WCharm_El, DYLight, TTSemilep };

  struct KinFitterResult {
    int status;
    double chi2_whad;
    double chi2_wlep;
    double chi2_thad;
    double chi2_tlep;
    void clear() {
      status = -999; // 또는 -1 등 네가 쓰고 싶은 초기값
      chi2_whad = -999.0;
      chi2_wlep = -999.0;
      chi2_thad = -999.0;
      chi2_tlep = -999.0;
    }
  };

  void Clear();
  void SetChannel();
  void initializeAnalyzer() override;
  void executeEvent() override;
  void SkimTree();
  void WriteHist() override;
  virtual void executeEventFromParameter();
  std::variant<float, std::pair<float, float>>
  SolveNeutrinoPz(const Lepton &lepton, const Particle &met);
  std::tuple<int, double, TLorentzVector, TLorentzVector, TLorentzVector>
  FitKinFitterLepTop(const Jet &bjet, Particle &neutrino, Lepton &lepton);
  KinFitterResult FitKinFitterTTSemilep(const Jet &had_t_b, const Jet &lep_t_b,
                                        const Jet &had_w_1, const Jet &had_w_2,
                                        Particle &neutrino,
                                        Lepton &lepton);
  float Calc_Each_Chi2(TAbsFitParticle *ptr);
  float Calc_Each_Chi2(TAbsFitConstraint *constraint, float mass, float width);
  KinFitterResult Chi2Prefit(const Jet &had_t_b, const Jet &lep_t_b,
                                        const Jet &had_w_1, const Jet &had_w_2,
                                        const Particle &neutrino,
                                        const Lepton &lepton);

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
  SVViewCollection AllSVViews;
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
  // additional info
  float log_chi2;
  float mmuj0;
  float melj0;
  float mmuj1;
  float melj1;
  float wcharm_soft_mu_pt;
  float wcharm_dimuon_mass;
  float wcharm_jet_muEF;
  float wcharm_jet_neEmEF;
  float wcharm_jet_muEF_plus_neEmEF;
  float wcharm_mt;
  float wcharm_w_pt;
  float wcharm_dphi_met_jet;
  float wcharm_dphi_lep_jet;
  float wcharm_dphi_w_jet;
  float wcharm_pt_ratio;
  float wcharm_dphi_trkmet_met;
  int wcharm_jet0_nsv;
  int wcharm_sv0_idx;
  int wcharm_sv1_idx;
  float wcharm_sv0_pt;
  float wcharm_sv0_eta;
  float wcharm_sv0_phi;
  float wcharm_sv0_mass;
  float wcharm_sv0_charge;
  float wcharm_sv0_chi2;
  float wcharm_sv0_ndof;
  float wcharm_sv0_ntracks;
  float wcharm_sv0_dlen;
  float wcharm_sv0_dlenSig;
  float wcharm_sv0_dxy;
  float wcharm_sv0_dxySig;
  float wcharm_sv0_pAngle;
  float wcharm_sv0_x;
  float wcharm_sv0_y;
  float wcharm_sv0_z;
  float wcharm_sv1_pt;
  float wcharm_sv1_eta;
  float wcharm_sv1_phi;
  float wcharm_sv1_mass;
  float wcharm_sv1_charge;
  float wcharm_sv1_chi2;
  float wcharm_sv1_ndof;
  float wcharm_sv1_ntracks;
  float wcharm_sv1_dlen;
  float wcharm_sv1_dlenSig;
  float wcharm_sv1_dxy;
  float wcharm_sv1_dxySig;
  float wcharm_sv1_pAngle;
  float wcharm_sv1_x;
  float wcharm_sv1_y;
  float wcharm_sv1_z;
  // kinematic fitter result
  KinFitterResult best_KF_result;

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
