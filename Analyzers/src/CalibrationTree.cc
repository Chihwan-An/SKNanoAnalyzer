#include "CalibrationTree.h"
#include "BranchManager.h"
#include "GenView.h"
#include "JetTaggingParameter.h"
#include "MyCorrection.h"
#include "Particle.h"
#include "TEntryList.h"
#include "TEntryListArray.h"
#include "TKinFitter.h"
#include "TObjArray.h"
#include "UParTScoreUtils.h"
#include "VcbParameters.h"
#include <RtypesCore.h>
#include <TLorentzVector.h>
#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <limits>
#include <string>
#include <unordered_map>
#include <vector>

CalibrationTree::CalibrationTree() {}

float CalibrationTree::Calc_Each_Chi2(TAbsFitConstraint *constraint, float mass,
                                      float width) {
  const TMatrixD *currPar = constraint->getParCurr();
  float deltaY = 1 - (*currPar)(0, 0);
  float chi2 = (mass * mass) * (deltaY * deltaY) / (width * width);

  return chi2;
}

float CalibrationTree::Calc_Each_Chi2(TAbsFitParticle *ptr) {
  const TMatrixD *iniPar = ptr->getParIni();
  const TMatrixD *currPar = ptr->getParCurr();
  const TMatrixD *covMatrix = ptr->getCovMatrix();

  float chi2 = 0;
  for (Int_t i = 0; i < iniPar->GetNcols(); i++) {
    float deltaY = (*iniPar)(i, i) - (*currPar)(i, i);
    chi2 += deltaY * deltaY / (*covMatrix)(i, i);
  }

  return chi2;
}

CalibrationTree::KinFitterResult CalibrationTree::Chi2Prefit(const SelectedJetView &had_t_b, const SelectedJetView &lep_t_b,
                                        const SelectedJetView &had_w_1, const SelectedJetView &had_w_2,
                                        const Particle &neutrino,
                                        const Lepton &lepton){

  KinFitterResult result;
  result.status = 0;
  double chi2_thad = ((had_t_b.P4() + had_w_1.P4() + had_w_2.P4()).M() - T_MASS)/T_WIDTH;
  double chi2_tlep = ((lep_t_b.P4() + lepton + neutrino).M() - T_MASS)/T_WIDTH;
  double chi2_whad = ((had_w_1.P4() + had_w_2.P4()).M() - W_MASS)/W_WIDTH;
  result.chi2_thad = chi2_thad*chi2_thad;
  result.chi2_tlep = chi2_tlep*chi2_tlep;
  result.chi2_whad = chi2_whad*chi2_whad;
  result.chi2_wlep = 0.;
  result.fit_chi2 =
      result.chi2_thad + result.chi2_tlep + result.chi2_whad +
      result.chi2_wlep;
  return result;
}



CalibrationTree::KinFitterResult CalibrationTree::FitKinFitterTTSemilep(
    const SelectedJetView &had_t_b, const SelectedJetView &lep_t_b, const SelectedJetView &had_w_1,
    const SelectedJetView &had_w_2, Particle &neutrino, Lepton &lepton) {
  KinFitterResult result;
  result.status = -999;
  result.fit_chi2 = 9999.;
  result.chi2_thad = 9999.;
  result.chi2_tlep = 9999.;
  result.chi2_whad = 9999.;
  result.chi2_wlep = 9999.;
  // Fitter 설정
  std::unique_ptr<TKinFitter> fitter =
      std::make_unique<TKinFitter>("ttSemilepFitter", "ttSemilepFitter");
  fitter->reset();
  fitter->setVerbosity(0);
  fitter->setMaxNbIter(HasFlag("TTSemilepFullKinFitIter") ? 5000 : 500);
  fitter->setMaxDeltaS(1e-2);
  fitter->setMaxF(1e-2);

  TLorentzVector had_t_b_p4 = static_cast<TLorentzVector>(had_t_b);
  TMatrixD had_t_b_Cov(1, 1);
  float had_t_b_JER = had_t_b.Pt();
  had_t_b_JER *= myCorr->GetJER(had_t_b.Eta(), had_t_b.Pt(), ev.GetRho());
  had_t_b_JER *= myCorr->GetJERSF(had_t_b.Eta(), had_t_b.Pt());
  had_t_b_Cov(0, 0) = had_t_b_JER * had_t_b_JER;
  auto had_t_b_fit = std::make_unique<TFitParticlePt>(
      "had_t_b", "had_t_b", &had_t_b_p4, &had_t_b_Cov);
  TLorentzVector lep_t_b_p4 = static_cast<TLorentzVector>(lep_t_b);
  TMatrixD lep_t_b_Cov(1, 1);
  float lep_t_b_JER = lep_t_b.Pt();
  lep_t_b_JER *= myCorr->GetJER(lep_t_b.Eta(), lep_t_b.Pt(), ev.GetRho());
  lep_t_b_JER *= myCorr->GetJERSF(lep_t_b.Eta(), lep_t_b.Pt());
  lep_t_b_Cov(0, 0) = lep_t_b_JER * lep_t_b_JER;
  auto lep_t_b_fit = std::make_unique<TFitParticlePt>(
      "lep_t_b", "lep_t_b", &lep_t_b_p4, &lep_t_b_Cov);
  TLorentzVector had_w1_p4 = static_cast<TLorentzVector>(had_w_1);
  TMatrixD had_w1_Cov(1, 1);
  float had_w1_JER = had_w_1.Pt();
  had_w1_JER *= myCorr->GetJER(had_w_1.Eta(), had_w_1.Pt(), ev.GetRho());
  had_w1_JER *= myCorr->GetJERSF(had_w_1.Eta(), had_w_1.Pt());
  had_w1_Cov(0, 0) = had_w1_JER * had_w1_JER;
  auto had_w1_fit = std::make_unique<TFitParticlePt>("had_w1", "had_w1",
                                                     &had_w1_p4, &had_w1_Cov);
  TLorentzVector had_w2_p4 = static_cast<TLorentzVector>(had_w_2);
  TMatrixD had_w2_Cov(1, 1);
  float had_w2_JER = had_w_2.Pt();
  had_w2_JER *= myCorr->GetJER(had_w_2.Eta(), had_w_2.Pt(), ev.GetRho());
  had_w2_JER *= myCorr->GetJERSF(had_w_2.Eta(), had_w_2.Pt());
  had_w2_Cov(0, 0) = had_w2_JER * had_w2_JER;
  auto had_w2_fit = std::make_unique<TFitParticlePt>("had_w2", "had_w2",
                                                     &had_w2_p4, &had_w2_Cov);
  TMatrixD lepton_Cov(1, 1);
  lepton_Cov(0, 0) = TMath::Power(lepton.Pt() * 0.0001, 2);
  auto lep =
      std::make_unique<TFitParticlePt>("lep", "lep", &lepton, &lepton_Cov);
  TVector3 neu_p3 = neutrino.Vect();
  auto neu =
      std::make_unique<TFitParticleMCCart>("neu", "neu", &neu_p3, 0., nullptr);

  // ---------------- Constraints ----------------
  auto mHadW = std::make_unique<TFitConstraintMGaus>(
      "MW_had", "MW_had", nullptr, nullptr, W_MASS, W_WIDTH);
  mHadW->addParticle1(had_w1_fit.get());
  mHadW->addParticle1(had_w2_fit.get());
  auto mLepW = std::make_unique<TFitConstraintMGaus>(
      "MW_lep", "MW_lep", nullptr, nullptr, W_MASS, W_WIDTH);
  mLepW->addParticle1(lep.get());
  mLepW->addParticle1(neu.get());
  auto mHadT = std::make_unique<TFitConstraintMGaus>(
      "MT_had", "MT_had", nullptr, nullptr, T_MASS, T_WIDTH);
  mHadT->addParticle1(had_t_b_fit.get());
  mHadT->addParticle1(had_w1_fit.get());
  mHadT->addParticle1(had_w2_fit.get());
  auto mLepT = std::make_unique<TFitConstraintMGaus>(
      "MT_lep", "MT_lep", nullptr, nullptr, T_MASS, T_WIDTH);
  mLepT->addParticle1(lep_t_b_fit.get());
  mLepT->addParticle1(lep.get());
  mLepT->addParticle1(neu.get());
  auto px_balance = std::make_unique<TFitConstraintEp>(
      "px", "px", TFitConstraintEp::component::pX, 0.);
  auto py_balance = std::make_unique<TFitConstraintEp>(
      "py", "py", TFitConstraintEp::component::pY, 0.);
  px_balance->addParticle(had_t_b_fit.get());
  px_balance->addParticle(lep_t_b_fit.get());
  px_balance->addParticle(had_w1_fit.get());
  px_balance->addParticle(had_w2_fit.get());
  px_balance->addParticle(lep.get());
  px_balance->addParticle(neu.get());
  py_balance->addParticle(had_t_b_fit.get());
  py_balance->addParticle(lep_t_b_fit.get());
  py_balance->addParticle(had_w1_fit.get());
  py_balance->addParticle(had_w2_fit.get());
  py_balance->addParticle(lep.get());
  py_balance->addParticle(neu.get());
  // --------------- fitter에 등록 ---------------
  fitter->addMeasParticle(had_t_b_fit.get());
  fitter->addMeasParticle(lep_t_b_fit.get());
  fitter->addMeasParticle(had_w1_fit.get());
  fitter->addMeasParticle(had_w2_fit.get());
  fitter->addMeasParticle(lep.get());
  fitter->addUnmeasParticle(neu.get());
  fitter->addConstraint(mHadW.get());
  fitter->addConstraint(mLepW.get());
  fitter->addConstraint(mHadT.get());
  fitter->addConstraint(mLepT.get());
  fitter->addConstraint(px_balance.get());
  fitter->addConstraint(py_balance.get());
  // 피팅 수행
  fitter->fit();
  result.status = fitter->getStatus();
  result.fit_chi2 = fitter->getS();
  result.chi2_whad = Calc_Each_Chi2(mHadW.get(), W_MASS, W_WIDTH);
  result.chi2_wlep = Calc_Each_Chi2(mLepW.get(), W_MASS, W_WIDTH);
  result.chi2_thad = Calc_Each_Chi2(mHadT.get(), T_MASS, T_WIDTH);
  result.chi2_tlep = Calc_Each_Chi2(mLepT.get(), T_MASS, T_WIDTH);
  return result;
}

// status, chi2, fitted_b, fitted_lep, fitted_neu
std::tuple<int, double, TLorentzVector, TLorentzVector, TLorentzVector>
CalibrationTree::FitKinFitterLepTop(const SelectedJetView &bjet, Particle &neutrino,
                                    Lepton &lepton) {
  // Fitter 설정
  std::unique_ptr<TKinFitter> fitter =
      std::make_unique<TKinFitter>("lepTopFitter", "lepTopFitter");
  fitter->reset();
  fitter->setVerbosity(0);
  fitter->setMaxNbIter(500);
  fitter->setMaxDeltaS(1e-2);
  fitter->setMaxF(1e-2);

  // --- b-jet 4-vector 및 JER covariance ---
  TLorentzVector jet_p4 = static_cast<TLorentzVector>(bjet);
  TMatrixD jet_Cov(1, 1);
  float jet_JER = bjet.Pt();
  jet_JER *= myCorr->GetJER(bjet.Eta(), bjet.Pt(), ev.GetRho());
  jet_JER *= myCorr->GetJERSF(bjet.Eta(), bjet.Pt());
  jet_Cov(0, 0) = jet_JER * jet_JER;

  auto lep_t_b =
      std::make_unique<TFitParticlePt>("lep_t_b", "lep_t_b", &jet_p4, &jet_Cov);

  // --- lepton covariance (매우 작은 에러) ---
  TMatrixD lepton_Cov(1, 1);
  lepton_Cov(0, 0) = TMath::Power(lepton.Pt() * 0.0001, 2);
  auto lep =
      std::make_unique<TFitParticlePt>("lep", "lep", &lepton, &lepton_Cov);

  // --- neutrino (unmeasured, 3-mom Cartesian) ---
  TVector3 neu_p3 = neutrino.Vect();
  auto neu =
      std::make_unique<TFitParticleMCCart>("neu", "neu", &neu_p3, 0., nullptr);

  // ---------------- Constraints ----------------

  // (1) W mass (lep + neu)
  auto mLepW = std::make_unique<TFitConstraintMGaus>("MW", "MW", nullptr,
                                                     nullptr, W_MASS, W_WIDTH);
  mLepW->addParticle1(lep.get());
  mLepW->addParticle1(neu.get());

  // (2) leptonic top mass (bjet + lep + neu)
  auto mLepT = std::make_unique<TFitConstraintMGaus>("MT", "MT", nullptr,
                                                     nullptr, T_MASS, T_WIDTH);
  mLepT->addParticle1(lep_t_b.get());
  mLepT->addParticle1(lep.get());
  mLepT->addParticle1(neu.get());

  // (3) px / py balance: bjet + lep + neu 의 합이 0
  auto px_balance = std::make_unique<TFitConstraintEp>(
      "px", "px", TFitConstraintEp::component::pX, 0.);
  auto py_balance = std::make_unique<TFitConstraintEp>(
      "py", "py", TFitConstraintEp::component::pY, 0.);

  px_balance->addParticle(lep_t_b.get());
  px_balance->addParticle(lep.get());
  px_balance->addParticle(neu.get());

  py_balance->addParticle(lep_t_b.get());
  py_balance->addParticle(lep.get());
  py_balance->addParticle(neu.get());

  // --------------- fitter에 등록 ---------------

  fitter->addMeasParticle(lep_t_b.get());
  fitter->addMeasParticle(lep.get());
  fitter->addUnmeasParticle(neu.get());

  fitter->addConstraint(mLepW.get());
  fitter->addConstraint(mLepT.get());
  fitter->addConstraint(px_balance.get());
  fitter->addConstraint(py_balance.get());

  // 피팅 수행
  // fitter->fit(); do not fit, just get chi2 and get assignment

  int status = fitter->getStatus();
  double chi2 = fitter->getS();

  TLorentzVector fitted_b(*lep_t_b->getCurr4Vec());
  TLorentzVector fitted_lep(*lep->getCurr4Vec());
  TLorentzVector fitted_neu(*neu->getCurr4Vec());

  return std::make_tuple(status, chi2, fitted_b, fitted_lep, fitted_neu);
}

std::variant<float, std::pair<float, float>>
CalibrationTree::SolveNeutrinoPz(const Lepton &lepton, const Particle &met) {
  float Ptl_dot_Ptnu = lepton.Px() * met.Px() + lepton.Py() * met.Py();
  float lepton_mass = lepton.M();

  // solve a*x^2 + b*x + c = 0, where x = pz of neutrino
  float k = TMath::Power(W_MASS, 2.) / 2.0 - lepton_mass * lepton_mass / 2.0 +
            Ptl_dot_Ptnu;
  float a = TMath::Power(lepton.Pt(), 2.0);
  float b = -2 * k * lepton.Pz();
  float c = TMath::Power(lepton.Pt(), 2.0) * TMath::Power(met.Pt(), 2.0) -
            TMath::Power(k, 2.0);

  float determinant = TMath::Power(b, 2.f) - 4.f * a * c;
  if (determinant < 0) {
    float real_pz = -b / (2.f * a);
    return real_pz;
  } else {
    float pz1 = (-b + TMath::Sqrt(determinant)) / (2.f * a);
    float pz2 = (-b - TMath::Sqrt(determinant)) / (2.f * a);
    return std::make_pair(pz1, pz2);
  }
}

void CalibrationTree::initializeAnalyzer() {
  SetChannel();
  string SKNANO_HOME = std::getenv("SKNANO_HOME");
  if (!IsDATA) {
    TString json_path = SKNANO_HOME + "/ModellingPatch/" + MCSample.Data() +
                        "_" + DataEra.Data() + "_summary.json";
    load_modelling_json(json_path);
  }
  if (IsDATA) {
    systHelper = std::make_unique<SystematicHelper>(
        SKNANO_HOME + "/AnalyzerTools/noSyst.yaml", DataStream, DataEra);
  } else {

    systHelper = std::make_unique<SystematicHelper>(
        SKNANO_HOME + "/AnalyzerTools/"
                      "CalibrationSystematic.yaml",
        MCSample, DataEra);
  }
  myCorr = new MyCorrection(DataEra, DataPeriod, IsDATA ? DataStream : MCSample,
                            IsDATA);
  SetSystematicLambda();

  if (fChain) {
    if (fChain->GetTreeNumber() < 0) {
      fChain->LoadTree(0);
    }
    static constexpr std::array<std::string_view, 10> branchNames = {
        "Jet_btagMyUParTAK4B",       "Jet_btagMyUParTAK4CvB",
        "Jet_btagMyUParTAK4CvL",     "Jet_btagMyUParTAK4CvNotB",
        "Jet_btagMyUParTAK4SvCB",    "Jet_btagMyUParTAK4SvUDG",
        "Jet_btagMyUParTAK4UDG",     "Jet_btagMyUParTAK4HFvLF",
        "Jet_btagMyUParTAK4BvC",     "Jet_btagMyUParTAK4QvG"};
    for (const auto name : branchNames) {
      const std::string key(name);
      myUParTColumns.emplace(
          key, GetColumnHandle<float>(key, ColumnRequirement::Optional));
    }
    useMyUParTBranches =
        myUParTColumns.at("Jet_btagMyUParTAK4B").available() &&
        myUParTColumns.at("Jet_btagMyUParTAK4CvB").available() &&
        myUParTColumns.at("Jet_btagMyUParTAK4CvL").available() &&
        myUParTColumns.at("Jet_btagMyUParTAK4CvNotB").available() &&
        myUParTColumns.at("Jet_btagMyUParTAK4SvCB").available() &&
        myUParTColumns.at("Jet_btagMyUParTAK4SvUDG").available() &&
        myUParTColumns.at("Jet_btagMyUParTAK4UDG").available();
    if (useMyUParTBranches) {
      std::cout << "[CalibrationTree] MyUParT input branches detected. "
                   "Using MyUParT scores for output tree."
                << std::endl;
    }
  }
}

void CalibrationTree::SetChannel() {
  if (HasFlag("Skim")) {
    std::cout << "Skimming mode detected, will iterate over channel"
              << std::endl;
  } else if (HasFlag("TTDilep")) {
    channel = Channel::TTDilep;
  } else if (HasFlag("WCharm_Mu")) {
    channel = Channel::WCharm_Mu;
  } else if (HasFlag("WCharm_El")) {
    channel = Channel::WCharm_El;
  } else if (HasFlag("DYLight")) {
    channel = Channel::DYLight;
  } else if (HasFlag("DYCharm")) {
    channel = Channel::DYCharm;
  } else if (HasFlag("QCDCharmDijet")) {
    channel = Channel::QCDCharmDijet;
  } else if (HasFlag("TTSemilep")) {
    channel = Channel::TTSemilep;
  } else {
    throw std::runtime_error(
        "Channel not specified! Please set one of the channel flags.");
  }
}

void CalibrationTree::ResetEventCaches() {
  leptonSelectionCache.reset(currentEntry);
  svViewsLoaded = false;
  AllMuonViews = MuonViewCollection();
  AllElectronViews = ElectronViewCollection();
  AllJetViews = JetViewCollection();
  AllSVViews = SVViewCollection();
  AllTrigObjViews = TrigObjViewCollection();
}

void CalibrationTree::EnsureSVViews() {
  if (!svViewsLoaded) {
    AllSVViews = GetAllSVViews();
    svViewsLoaded = true;
  }
}

const std::string &CalibrationTree::CachedTreeName(std::string_view treePrefix) {
  const std::string raw(treePrefix);
  const auto cached = sanitizedTreeNameCache.find(raw);
  if (cached != sanitizedTreeNameCache.end()) {
    return cached->second;
  }

  std::string sanitized;
  sanitized.reserve(raw.size());
  for (char c : raw) {
    const unsigned char uc = static_cast<unsigned char>(c);
    if (std::isalnum(uc) || c == '_') {
      sanitized.push_back(c);
    } else {
      sanitized.push_back('_');
    }
  }
  while (!sanitized.empty() && sanitized.front() == '_') {
    sanitized.erase(sanitized.begin());
  }
  while (!sanitized.empty() && sanitized.back() == '_') {
    sanitized.pop_back();
  }
  if (sanitized.empty()) {
    sanitized = "Tree";
  }

  auto inserted =
      sanitizedTreeNameCache.emplace(std::move(raw), std::move(sanitized));
  return inserted.first->second;
}

const std::string &
CalibrationTree::CachedJetBranchName(std::size_t jetIndex,
                                     std::string_view suffix) {
  std::string key;
  key.reserve(suffix.size() + 16);
  key.append(std::to_string(jetIndex));
  key.push_back('/');
  key.append(suffix);

  const auto cached = jetBranchNameCache.find(key);
  if (cached != jetBranchNameCache.end()) {
    return cached->second;
  }

  std::string branchName;
  branchName.reserve(suffix.size() + 16);
  branchName.append("Jet_");
  branchName.append(std::to_string(jetIndex));
  branchName.push_back('_');
  branchName.append(suffix);

  auto inserted =
      jetBranchNameCache.emplace(std::move(key), std::move(branchName));
  return inserted.first->second;
}

void CalibrationTree::BuildLeptonSelectionCache() {
  if (leptonSelectionCache.valid && leptonSelectionCache.entry == currentEntry) {
    return;
  }

  leptonSelectionCache.reset(currentEntry);
  auto &cache = leptonSelectionCache;

  cache.looseMuonIndices =
      SelectMuonIndices(AllMuonViews, Muon_Veto_ID, Muon_Veto_Pt, Muon_Veto_Eta);
  cache.looseMuonIndices =
      SelectMuonIndices(AllMuonViews, cache.looseMuonIndices, Muon_Veto_Iso,
                        Muon_Veto_Pt, Muon_Veto_Eta);
  cache.tightMuonIndices =
      SelectMuonIndices(AllMuonViews, cache.looseMuonIndices, Muon_Tight_ID,
                        Muon_Tight_Pt[DataEra.Data()], Muon_Tight_Eta);
  cache.tightMuonIndices =
      SelectMuonIndices(AllMuonViews, cache.tightMuonIndices, Muon_Tight_Iso,
                        Muon_Tight_Pt[DataEra.Data()], Muon_Tight_Eta);

  cache.looseElectronIndices = SelectElectronIndices(
      AllElectronViews, Electron_Veto_ID, Electron_Veto_Pt, Electron_Veto_Eta);
  cache.tightElectronIndices =
      SelectElectronIndices(AllElectronViews, cache.looseElectronIndices,
                            Electron_Tight_ID, Electron_Tight_Pt[DataEra.Data()],
                            Electron_Tight_Eta);
  cache.valid = true;
}

bool CalibrationTree::PassChannelTriggerPreselection() const {
  switch (channel) {
  case Channel::TTDilep:
    if (IsDATA && DataStream.Contains("EGamma")) {
      return ev.PassTrigger(El_Trigger[DataEra.Data()]) &&
             !ev.PassTrigger(Mu_Trigger[DataEra.Data()]);
    }
    return ev.PassTrigger(Mu_Trigger[DataEra.Data()]) ||
           ev.PassTrigger(El_Trigger[DataEra.Data()]);
  case Channel::WCharm_Mu:
  case Channel::DYLight:
  case Channel::DYCharm:
  case Channel::TTSemilep:
    return ev.PassTrigger(Mu_Trigger[DataEra.Data()]);
  case Channel::WCharm_El:
    return ev.PassTrigger(El_Trigger[DataEra.Data()]);
  case Channel::QCDCharmDijet:
    return true;
  default:
    return false;
  }
}

bool CalibrationTree::PassLeptonCachePreselection() {
  BuildLeptonSelectionCache();
  const auto &cache = leptonSelectionCache;

  auto dimuon_mass = [&](std::size_t idx1, std::size_t idx2) {
    TLorentzVector mu1;
    mu1.SetPtEtaPhiM(AllMuonViews[idx1].Pt(), AllMuonViews[idx1].Eta(),
                     AllMuonViews[idx1].Phi(), AllMuonViews[idx1].M());
    TLorentzVector mu2;
    mu2.SetPtEtaPhiM(AllMuonViews[idx2].Pt(), AllMuonViews[idx2].Eta(),
                     AllMuonViews[idx2].Phi(), AllMuonViews[idx2].M());
    return static_cast<float>((mu1 + mu2).M());
  };

  switch (channel) {
  case Channel::TTDilep:
    if (!(cache.tightMuonIndices.size() == 1 &&
          cache.tightElectronIndices.size() == 1 &&
          cache.looseMuonIndices.size() == 1 &&
          cache.looseElectronIndices.size() == 1)) {
      return false;
    }
    return AllMuonViews[cache.tightMuonIndices[0]].Charge() *
               AllElectronViews[cache.tightElectronIndices[0]].Charge() <=
           0;
  case Channel::DYLight:
  case Channel::DYCharm:
    if (!(cache.tightMuonIndices.size() == 2 &&
          cache.looseMuonIndices.size() == 2 &&
          cache.tightElectronIndices.empty() &&
          cache.looseElectronIndices.empty())) {
      return false;
    }
    if (AllMuonViews[cache.tightMuonIndices[0]].Charge() *
            AllMuonViews[cache.tightMuonIndices[1]].Charge() >
        0) {
      return false;
    }
    {
      const float mass =
          dimuon_mass(cache.tightMuonIndices[0], cache.tightMuonIndices[1]);
      return mass >= 81.f && mass <= 101.f;
    }
  case Channel::WCharm_Mu:
  case Channel::TTSemilep:
    return cache.tightMuonIndices.size() == 1 &&
           cache.tightElectronIndices.empty() &&
           cache.looseMuonIndices.size() == 1 &&
           cache.looseElectronIndices.empty();
  case Channel::WCharm_El:
    return cache.tightMuonIndices.empty() &&
           cache.tightElectronIndices.size() == 1 &&
           cache.looseMuonIndices.empty() &&
           cache.looseElectronIndices.size() == 1;
  case Channel::QCDCharmDijet:
    return cache.looseMuonIndices.empty() && cache.looseElectronIndices.empty();
  default:
    return false;
  }
}

bool CalibrationTree::PassEventPreselectionBeforeSystematics() {
  if (!myCorr->IsGoldenLumi(RunNumber, luminosityBlock)) {
    return false;
  }
  if (!PassChannelTriggerPreselection()) {
    return false;
  }

  AllMuonViews = GetAllMuonViews();
  AllElectronViews = GetAllElectronViews();
  return PassLeptonCachePreselection();
}

float CalibrationTree::ReadMyUParTValue(std::string_view branchName,
                                        std::size_t jetIndex) {
  constexpr float kInvalidScore = -1.f;
  if (currentLocalEntry < 0) {
    return kInvalidScore;
  }

  const std::string key(branchName);
  const auto it = myUParTColumns.find(key);
  if (it == myUParTColumns.end() || !it->second.available()) {
    return kInvalidScore;
  }
  const auto &column = it->second;
  if (jetIndex >= column.size()) {
    return kInvalidScore;
  }
  const float value = column[jetIndex];
  if (!std::isfinite(value)) {
    return kInvalidScore;
  }
  return value;
}

void CalibrationTree::FillHistogramsAtThisPoint(std::string_view histPrefix,
                                                float weight) {
  const std::string base(histPrefix);
  std::string name;
  name.reserve(base.size() + 64);

  auto fill1d = [&](std::string_view suffix, float value, float w, int nbin,
                    float xmin, float xmax) {
    name.assign(base);
    name.push_back('/');
    name.append(suffix);
    Hists().Fill(name, value, w, nbin, xmin, xmax);
  };

  auto fill2d = [&](std::string_view suffix, float x, float y, float w,
                    int nbinx, float xmin, float xmax, int nbiny, float ymin,
                    float ymax) {
    name.assign(base);
    name.push_back('/');
    name.append(suffix);
    Hists().Fill(name, x, y, w, nbinx, xmin, xmax, nbiny, ymin, ymax);
  };

  fill1d("MET_Pt", MET.Pt(), weight, 100, 0, 200);
  fill1d("MET_Phi", MET.Phi(), weight, 64, -3.2, 3.2);

  for (size_t idx = 0; idx < Jets.size(); ++idx) {
    const SelectedJetView jet = Jets[idx];
    const short hardflav = IsDATA ? -1 : abs(jet.hadronFlavour());
    fill1d("Jet_" + to_string(idx) + "_Pt", jet.Pt(), weight, 100, 0, 200);
    fill1d("Jet_" + to_string(idx) + "_Eta", jet.Eta(), weight, 50, -2.5, 2.5);
    fill1d("Jet_" + to_string(idx) + "_Phi", jet.Phi(), weight, 64, -3.2, 3.2);
    fill1d("Jet_" + to_string(idx) + "_UParT_B",
           jet.GetTaggerResult(JetTagging::JetFlavTagger::ParT,
                               JetTagging::JetFlavTaggerScoreType::B),
           weight, 50, 0.f, 1.f);
    fill1d("Jet_" + to_string(idx) + "_UParT_CvB",
           jet.GetTaggerResult(JetTagging::JetFlavTagger::ParT,
                               JetTagging::JetFlavTaggerScoreType::CvB),
           weight, 50, 0.f, 1.f);
    fill1d("Jet_" + to_string(idx) + "_UParT_CvL",
           jet.GetTaggerResult(JetTagging::JetFlavTagger::ParT,
                               JetTagging::JetFlavTaggerScoreType::CvL),
           weight, 50, 0.f, 1.f);
    fill1d("Jet_" + to_string(idx) + "_UParT_CvNotB",
           jet.GetTaggerResult(JetTagging::JetFlavTagger::ParT,
                               JetTagging::JetFlavTaggerScoreType::CvNotB),
           weight, 50, 0.f, 1.f);
    fill1d("Jet_" + to_string(idx) + "_UParT_SvCB",
           jet.GetTaggerResult(JetTagging::JetFlavTagger::ParT,
                               JetTagging::JetFlavTaggerScoreType::SvCB),
           weight, 50, 0.f, 1.f);
    fill1d("Jet_" + to_string(idx) + "_UParT_SvUDG",
           jet.GetTaggerResult(JetTagging::JetFlavTagger::ParT,
                               JetTagging::JetFlavTaggerScoreType::SvUDG),
           weight, 50, 0.f, 1.f);
    fill1d("Jet_" + to_string(idx) + "_UParT_probUDG",
           jet.GetTaggerResult(JetTagging::JetFlavTagger::ParT,
                               JetTagging::JetFlavTaggerScoreType::probUDG),
           weight, 50, 0.f, 1.f);
    fill1d("Jet_" + to_string(idx) + "_UParT_probB",
           jet.GetTaggerResult(JetTagging::JetFlavTagger::ParT,
                               JetTagging::JetFlavTaggerScoreType::probB),
           weight, 50, 0.f, 1.f);
    fill1d("Jet_" + to_string(idx) + "_UParT_probBB",
           jet.GetTaggerResult(JetTagging::JetFlavTagger::ParT,
                               JetTagging::JetFlavTaggerScoreType::probBB),
           weight, 50, 0.f, 1.f);

    const float probudg =
        jet.GetTaggerResult(JetTagging::JetFlavTagger::ParT,
                            JetTagging::JetFlavTaggerScoreType::probUDG);
    const float svudg =
        jet.GetTaggerResult(JetTagging::JetFlavTagger::ParT,
                            JetTagging::JetFlavTaggerScoreType::SvUDG);
    const float cvl =
        jet.GetTaggerResult(JetTagging::JetFlavTagger::ParT,
                            JetTagging::JetFlavTaggerScoreType::CvL);
    const float cvb =
        jet.GetTaggerResult(JetTagging::JetFlavTagger::ParT,
                            JetTagging::JetFlavTaggerScoreType::CvB);
    auto valid_score = [](float v) { return v >= 0.f && v <= 1.f; };
    if (valid_score(probudg) && valid_score(svudg) && valid_score(cvl) &&
        valid_score(cvb)) {
      const auto p =
          UParTScore::compute_prob3_from_branches(probudg, cvl, cvb, svudg);
      const auto [hf, bvc] = UParTScore::hf_bvc_from_prob3(p);
      const int cat = static_cast<int>(UParTScore::classify_from_scores(hf, bvc));

      fill1d("Jet_" + to_string(idx) + "_prob3_B", static_cast<float>(p.pb),
             weight, 50, 0.f, 1.f);
      fill1d("Jet_" + to_string(idx) + "_prob3_C", static_cast<float>(p.pc),
             weight, 50, 0.f, 1.f);
      fill1d("Jet_" + to_string(idx) + "_prob3_L", static_cast<float>(p.pl),
             weight, 50, 0.f, 1.f);
      fill1d("Jet_" + to_string(idx) + "_HFvLF", static_cast<float>(hf), weight,
             50, 0.f, 1.f);
      fill1d("Jet_" + to_string(idx) + "_BvC", static_cast<float>(bvc), weight,
             50, 0.f, 1.f);
      fill1d("Jet_" + to_string(idx) + "_Cat", static_cast<float>(cat), weight,
             12, -0.5f, 11.5f);
    }
    fill1d("Jet_" + to_string(idx) + "_hadronFlavour",
           static_cast<float>(hardflav), weight, 7, -1.f, 6.f);
  }

  if (channel == Channel::TTSemilep) {
    fill1d("TTSL_FitStatus", static_cast<float>(ttsl_fit_status), weight, 21,
           -10.5f, 10.5f);
    fill1d("TTSL_LogFitChi2", ttsl_log_fit_chi2, weight, 100, -10.f, 10.f);
    fill1d("TTSL_FitChi2", ttsl_fit_chi2, weight, 100, 0.f, 500.f);
    fill1d("TTSL_PrefitChi2", ttsl_prefit_chi2, weight, 100, 0.f, 500.f);
    fill1d("TTSL_PrefitMW", ttsl_prefit_mw, weight, 100, 0.f, 200.f);
    fill1d("TTSL_DeltaChi2", ttsl_delta_chi2, weight, 100, 0.f, 500.f);
    fill1d("TTSL_BestCombIdx", static_cast<float>(ttsl_best_comb_idx), weight,
           12, -0.5f, 11.5f);
    fill1d("TTSL_NValidCombs", static_cast<float>(ttsl_n_valid_combs), weight,
           13, -0.5f, 12.5f);
    fill1d("TTSL_NPrefitCandidates",
           static_cast<float>(ttsl_n_prefit_candidates), weight, 25, -0.5f,
           24.5f);
    fill1d("TTSL_NFitCandidates",
           static_cast<float>(ttsl_n_fit_candidates), weight, 25, -0.5f,
           24.5f);
    fill1d("TTSL_METPtBeforeJetSyst", ttsl_met_pt_before_jet_syst, weight, 100,
           0.f, 200.f);
    fill1d("TTSL_METPtAfterJetSyst", ttsl_met_pt_after_jet_syst, weight, 100,
           0.f, 200.f);
  }

  for (size_t idx = 0; idx < Electrons.size(); ++idx) {
    const ElectronView ele = Electrons[idx];
    fill1d("Electron_" + to_string(idx) + "_Pt", ele.Pt(), weight, 100, 0, 500);
    fill1d("Electron_" + to_string(idx) + "_Eta", ele.Eta(), weight, 50, -2.5,
           2.5);
    fill1d("Electron_" + to_string(idx) + "_Phi", ele.Phi(), weight, 64, -3.2,
           3.2);
  }

  for (size_t idx = 0; idx < Muons.size(); ++idx) {
    const MuonView mu = Muons[idx];
    fill1d("Muon_" + to_string(idx) + "_Pt", mu.Pt(), weight, 100, 0, 500);
    fill1d("Muon_" + to_string(idx) + "_Eta", mu.Eta(), weight, 50, -2.5, 2.5);
    fill1d("Muon_" + to_string(idx) + "_Phi", mu.Phi(), weight, 64, -3.2, 3.2);
  }
  if (Muons.size() >= 2) {
    TLorentzVector ZCand = Muons[0].P4() + Muons[1].P4();
    fill1d("Dimuon_Mass", ZCand.M(), weight, 100, 70, 110);
  }

  if (channel == Channel::WCharm_Mu || channel == Channel::WCharm_El) {
    fill1d("WCharm_Jet0_muEF", wcharm_jet_muEF, weight, 50, 0.f, 1.f);
    fill1d("WCharm_Jet0_neEmEF", wcharm_jet_neEmEF, weight, 50, 0.f, 1.f);
    fill1d("WCharm_Jet0_muEF_plus_neEmEF", wcharm_jet_muEF_plus_neEmEF,
           weight, 60, 0.f, 1.5f);
    fill1d("WCharm_SoftMuon_Pt", wcharm_soft_mu_pt, weight, 50, 0.f, 50.f);
    fill1d("WCharm_MT", wcharm_mt, weight, 100, 0.f, 200.f);
    fill1d("WCharm_W_Pt", wcharm_w_pt, weight, 100, 0.f, 300.f);
    fill1d("WCharm_dphi_MET_Jet0", wcharm_dphi_met_jet, weight, 64, 0.f,
           3.2f);
    fill1d("WCharm_dphi_Lep_Jet0", wcharm_dphi_lep_jet, weight, 64, 0.f,
           3.2f);
    fill1d("WCharm_dphi_W_Jet0", wcharm_dphi_w_jet, weight, 64, 0.f, 3.2f);
    fill1d("WCharm_PtRatio_Jet0_W", wcharm_pt_ratio, weight, 60, 0.f, 3.f);
    fill1d("WCharm_dphi_TrkMET_MET", wcharm_dphi_trkmet_met, weight, 64, 0.f,
           3.2f);
    if (channel == Channel::WCharm_Mu)
      fill1d("WCharm_DiMuon_Mass", wcharm_dimuon_mass, weight, 100, 0.f,
             200.f);

    fill1d("WCharm_Jet0_nSVs", static_cast<float>(wcharm_jet0_nsv), weight, 8,
           -0.5f, 7.5f);

    auto fill_sv = [&](const std::string &tag, int idx, float pt, float eta,
                       float phi, float mass, float charge, float chi2,
                       float ndof, float ntracks, float dlen, float dlenSig,
                       float dxy, float dxySig, float pAngle, float x, float y,
                       float z) {
      if (idx < 0)
        return;
      fill1d(tag + "_Pt", pt, weight, 100, 0.f, 200.f);
      fill1d(tag + "_Eta", eta, weight, 50, -2.5f, 2.5f);
      fill1d(tag + "_Phi", phi, weight, 64, -3.2f, 3.2f);
      fill1d(tag + "_Mass", mass, weight, 60, 0.f, 10.f);
      fill1d(tag + "_Charge", charge, weight, 11, -5.5f, 5.5f);
      fill1d(tag + "_Chi2", chi2, weight, 50, 0.f, 10.f);
      fill1d(tag + "_Ndof", ndof, weight, 30, 0.f, 30.f);
      fill1d(tag + "_NTracks", ntracks, weight, 20, -0.5f, 19.5f);
      fill1d(tag + "_Dlen", dlen, weight, 100, 0.f, 10.f);
      fill1d(tag + "_DlenSig", dlenSig, weight, 100, 0.f, 50.f);
      fill1d(tag + "_Dxy", dxy, weight, 100, 0.f, 10.f);
      fill1d(tag + "_DxySig", dxySig, weight, 100, 0.f, 50.f);
      fill1d(tag + "_PAngle", pAngle, weight, 64, 0.f, 3.2f);
      fill1d(tag + "_X", x, weight, 100, -1.f, 1.f);
      fill1d(tag + "_Y", y, weight, 100, -1.f, 1.f);
      fill1d(tag + "_Z", z, weight, 100, -20.f, 20.f);
    };

    fill_sv("WCharm_Jet0_SV0", wcharm_sv0_idx, wcharm_sv0_pt, wcharm_sv0_eta,
            wcharm_sv0_phi, wcharm_sv0_mass, wcharm_sv0_charge,
            wcharm_sv0_chi2, wcharm_sv0_ndof, wcharm_sv0_ntracks,
            wcharm_sv0_dlen, wcharm_sv0_dlenSig, wcharm_sv0_dxy,
            wcharm_sv0_dxySig, wcharm_sv0_pAngle, wcharm_sv0_x, wcharm_sv0_y,
            wcharm_sv0_z);
    fill_sv("WCharm_Jet0_SV1", wcharm_sv1_idx, wcharm_sv1_pt, wcharm_sv1_eta,
            wcharm_sv1_phi, wcharm_sv1_mass, wcharm_sv1_charge,
            wcharm_sv1_chi2, wcharm_sv1_ndof, wcharm_sv1_ntracks,
            wcharm_sv1_dlen, wcharm_sv1_dlenSig, wcharm_sv1_dxy,
            wcharm_sv1_dxySig, wcharm_sv1_pAngle, wcharm_sv1_x, wcharm_sv1_y,
            wcharm_sv1_z);
  }

  if (channel == Channel::DYCharm && dycharm_has_tagprobe) {
    fill1d("DYCharm_TagJet_Pt", dycharm_tag_pt, weight, 100, 0.f, 200.f);
    fill1d("DYCharm_TagJet_Eta", dycharm_tag_eta, weight, 50, -2.5f, 2.5f);
    fill1d("DYCharm_TagJet_Phi", dycharm_tag_phi, weight, 64, -3.2f, 3.2f);
    fill1d("DYCharm_TagJet_hadronFlavour", dycharm_tag_hadronFlavour, weight, 7, -1.f,
           6.f);

    fill1d("DYCharm_ProbeJet_Pt", dycharm_probe_pt, weight, 100, 0.f, 200.f);
    fill1d("DYCharm_ProbeJet_Eta", dycharm_probe_eta, weight, 50, -2.5f, 2.5f);
    fill1d("DYCharm_ProbeJet_Phi", dycharm_probe_phi, weight, 64, -3.2f, 3.2f);
    fill1d("DYCharm_ProbeJet_hadronFlavour", dycharm_probe_hadronFlavour, weight, 7, -1.f,
           6.f);
    fill1d("DYCharm_Z_Pt", dycharm_z_pt, weight, 100, 0.f, 300.f);
    fill1d("DYCharm_Z_Mass", dycharm_z_mass, weight, 100, 70.f, 110.f);

    fill1d("DYCharm_JJ_Pt", dycharm_jj_pt, weight, 100, 0.f, 300.f);
    fill1d("DYCharm_JJ_Mass", dycharm_jj_mass, weight, 100, 0.f, 200.f);
    fill1d("DYCharm_dR_JJ", dycharm_dR_jj, weight, 60, 0.f, 3.f);
    fill1d("DYCharm_dphi_Z_JJ", dycharm_dphi_z_jj, weight, 64, 0.f, 3.2f);
    fill1d("DYCharm_PtBal_Z_JJ", dycharm_ptbal_z_jj, weight, 60, 0.f, 2.f);
    fill1d("DYCharm_PtAsym_JJ", dycharm_ptasym_jj, weight, 50, 0.f, 1.f);
  }

  if (channel == Channel::QCDCharmDijet && qcdcharm_has_tagprobe) {
    fill1d("QCDCharm_TagJet_Pt", qcdcharm_tag_pt, weight, 100, 0.f, 300.f);
    fill1d("QCDCharm_TagJet_Eta", qcdcharm_tag_eta, weight, 50, -2.5f, 2.5f);
    fill1d("QCDCharm_TagJet_Phi", qcdcharm_tag_phi, weight, 64, -3.2f, 3.2f);
    fill1d("QCDCharm_TagJet_hadronFlavour", qcdcharm_tag_hadronFlavour, weight, 7, -1.f,
           6.f);
    fill1d("QCDCharm_ProbeJet_Pt", qcdcharm_probe_pt, weight, 100, 0.f, 300.f);
    fill1d("QCDCharm_ProbeJet_Eta", qcdcharm_probe_eta, weight, 50, -2.5f, 2.5f);
    fill1d("QCDCharm_ProbeJet_Phi", qcdcharm_probe_phi, weight, 64, -3.2f, 3.2f);
    fill1d("QCDCharm_ProbeJet_hadronFlavour", qcdcharm_probe_hadronFlavour, weight, 7, -1.f,
           6.f);
    fill1d("QCDCharm_JJ_Pt", qcdcharm_jj_pt, weight, 100, 0.f, 500.f);
    fill1d("QCDCharm_JJ_Mass", qcdcharm_jj_mass, weight, 100, 0.f, 500.f);
    fill1d("QCDCharm_dR_JJ", qcdcharm_dR_jj, weight, 60, 0.f, 3.f);
    fill1d("QCDCharm_dphi_JJ", qcdcharm_dphi_jj, weight, 64, 0.f, 3.2f);
    fill1d("QCDCharm_PtAsym_JJ", qcdcharm_ptasym_jj, weight, 50, 0.f, 1.f);
  }
}

void CalibrationTree::FillTreeAtThisPoint(
    std::string_view treePrefix, float MCNormalizationWeight,
    const std::unordered_map<std::string, float> &weight_map) {

  const std::string &tree_name = CachedTreeName(treePrefix);
  BookTree(tree_name, {}, {"*"});
  auto safe_log = [](double value) {
    return value > 0. ? static_cast<float>(std::log(value)) : -9999.f;
  };

  OutputTree(tree_name).Set( "log_chi2", log_chi2);
  OutputTree(tree_name).Set( "ttsl_fit_status", ttsl_fit_status);
  OutputTree(tree_name).Set( "ttsl_fit_chi2", ttsl_fit_chi2);
  OutputTree(tree_name).Set( "ttsl_log_fit_chi2", ttsl_log_fit_chi2);
  OutputTree(tree_name).Set( "ttsl_second_fit_chi2", ttsl_second_fit_chi2);
  OutputTree(tree_name).Set( "ttsl_delta_chi2", ttsl_delta_chi2);
  OutputTree(tree_name).Set( "ttsl_prefit_chi2", ttsl_prefit_chi2);
  OutputTree(tree_name).Set( "ttsl_prefit_mw", ttsl_prefit_mw);
  OutputTree(tree_name).Set( "ttsl_best_comb_idx", ttsl_best_comb_idx);
  OutputTree(tree_name).Set( "ttsl_second_comb_idx", ttsl_second_comb_idx);
  OutputTree(tree_name).Set( "ttsl_best_nu_idx", ttsl_best_nu_idx);
  OutputTree(tree_name).Set( "ttsl_n_valid_combs", ttsl_n_valid_combs);
  OutputTree(tree_name).Set( "ttsl_n_prefit_candidates",
            ttsl_n_prefit_candidates);
  OutputTree(tree_name).Set( "ttsl_n_fit_candidates", ttsl_n_fit_candidates);
  OutputTree(tree_name).Set( "ttsl_met_propagated", ttsl_met_propagated);
  OutputTree(tree_name).Set( "ttsl_met_pt_before_jet_syst",
            ttsl_met_pt_before_jet_syst);
  OutputTree(tree_name).Set( "ttsl_met_pt_after_jet_syst",
            ttsl_met_pt_after_jet_syst);
  OutputTree(tree_name).Set( "mmuj0", mmuj0);
  OutputTree(tree_name).Set( "mmuj1", mmuj1);
  OutputTree(tree_name).Set( "melj0", melj0);
  OutputTree(tree_name).Set( "melj1", melj1);
  OutputTree(tree_name).Set( "log_chi2_had_t",
            safe_log(best_KF_result.chi2_thad));
  OutputTree(tree_name).Set( "log_chi2_lep_t",
            safe_log(best_KF_result.chi2_tlep));
  OutputTree(tree_name).Set( "log_chi2_had_w",
            safe_log(best_KF_result.chi2_whad));
  OutputTree(tree_name).Set( "log_chi2_lep_w",
            safe_log(best_KF_result.chi2_wlep));
  OutputTree(tree_name).Set( "log_chi2", log_chi2);
  const auto valid_score = [](float v) { return v >= 0.f && v <= 1.f; };
  using JetTagger = JetTagging::JetFlavTagger;
  using JetScore = JetTagging::JetFlavTaggerScoreType;

  auto set_jet_tagger_score = [&](size_t jet_idx, std::string_view tagger_name,
                                  JetTagger tagger, JetScore score,
                                  std::string_view score_name) {
    std::string suffix;
    suffix.reserve(tagger_name.size() + score_name.size() + 1);
    suffix.append(tagger_name);
    suffix.push_back('_');
    suffix.append(score_name);
    OutputTree(tree_name).Set( CachedJetBranchName(jet_idx, suffix),
              Jets[jet_idx].GetTaggerResult(tagger, score));
  };

  auto set_v15_jet_tagger_scores = [&](size_t jet_idx) {
    set_jet_tagger_score(jet_idx, "DeepJet", JetTagger::DeepJet, JetScore::B,
                         "B");
    set_jet_tagger_score(jet_idx, "DeepJet", JetTagger::DeepJet,
                         JetScore::CvB, "CvB");
    set_jet_tagger_score(jet_idx, "DeepJet", JetTagger::DeepJet,
                         JetScore::CvL, "CvL");

    set_jet_tagger_score(jet_idx, "PNet", JetTagger::ParticleNet, JetScore::B,
                         "B");
    set_jet_tagger_score(jet_idx, "PNet", JetTagger::ParticleNet,
                         JetScore::CvB, "CvB");
    set_jet_tagger_score(jet_idx, "PNet", JetTagger::ParticleNet,
                         JetScore::CvL, "CvL");
    set_jet_tagger_score(jet_idx, "PNet", JetTagger::ParticleNet,
                         JetScore::CvNotB, "CvNotB");
  };

  for (size_t i = 0; i < Jets.size(); ++i) {
    OutputTree(tree_name).Set( CachedJetBranchName(i, "Pt"), Jets[i].Pt());
    set_v15_jet_tagger_scores(i);

    if (useMyUParTBranches) {
      const int original_idx = Jets[i].OriginalIndex();
      const std::size_t source_idx =
          original_idx >= 0 ? static_cast<std::size_t>(original_idx) : i;

      const float my_b = ReadMyUParTValue("Jet_btagMyUParTAK4B", source_idx);
      const float my_cvb = ReadMyUParTValue("Jet_btagMyUParTAK4CvB", source_idx);
      const float my_cvl = ReadMyUParTValue("Jet_btagMyUParTAK4CvL", source_idx);
      const float my_cvnotb =
          ReadMyUParTValue("Jet_btagMyUParTAK4CvNotB", source_idx);
      const float my_svcb =
          ReadMyUParTValue("Jet_btagMyUParTAK4SvCB", source_idx);
      const float my_svudg =
          ReadMyUParTValue("Jet_btagMyUParTAK4SvUDG", source_idx);
      const float my_udg = ReadMyUParTValue("Jet_btagMyUParTAK4UDG", source_idx);
      const float my_hfvlf =
          ReadMyUParTValue("Jet_btagMyUParTAK4HFvLF", source_idx);
      const float my_bvc = ReadMyUParTValue("Jet_btagMyUParTAK4BvC", source_idx);
      const float my_qvg = ReadMyUParTValue("Jet_btagMyUParTAK4QvG", source_idx);

      OutputTree(tree_name).Set( CachedJetBranchName(i, "UParT_B"), my_b);
      OutputTree(tree_name).Set( CachedJetBranchName(i, "UParT_CvB"), my_cvb);
      OutputTree(tree_name).Set( CachedJetBranchName(i, "UParT_CvL"), my_cvl);
      OutputTree(tree_name).Set( CachedJetBranchName(i, "UParT_CvNotB"), my_cvnotb);
      OutputTree(tree_name).Set( CachedJetBranchName(i, "UParT_SvCB"), my_svcb);
      OutputTree(tree_name).Set( CachedJetBranchName(i, "UParT_SvUDG"), my_svudg);
      OutputTree(tree_name).Set( CachedJetBranchName(i, "UParT_probUDG"), my_udg);
      OutputTree(tree_name).Set( CachedJetBranchName(i, "UParT_probB"), -1.f);
      OutputTree(tree_name).Set( CachedJetBranchName(i, "UParT_probBB"), -1.f);
      OutputTree(tree_name).Set( CachedJetBranchName(i, "MyUParT_B"), my_b);
      OutputTree(tree_name).Set( CachedJetBranchName(i, "MyUParT_CvB"), my_cvb);
      OutputTree(tree_name).Set( CachedJetBranchName(i, "MyUParT_CvL"), my_cvl);
      OutputTree(tree_name).Set( CachedJetBranchName(i, "MyUParT_CvNotB"), my_cvnotb);
      OutputTree(tree_name).Set( CachedJetBranchName(i, "MyUParT_SvCB"), my_svcb);
      OutputTree(tree_name).Set( CachedJetBranchName(i, "MyUParT_SvUDG"), my_svudg);
      OutputTree(tree_name).Set( CachedJetBranchName(i, "MyUParT_QvG"), my_qvg);
      OutputTree(tree_name).Set( CachedJetBranchName(i, "MyUParT_probUDG"), my_udg);
      OutputTree(tree_name).Set( CachedJetBranchName(i, "MyUParT_HFvLF"), my_hfvlf);
      OutputTree(tree_name).Set( CachedJetBranchName(i, "MyUParT_BvC"), my_bvc);

      float hf = my_hfvlf;
      float bvc = my_bvc;
      float pb = -1.f;
      float pc = -1.f;
      float pl = -1.f;
      int cat = -1;
      if (valid_score(my_udg) && valid_score(my_svudg) && valid_score(my_cvl) &&
          valid_score(my_cvb)) {
        const auto p =
            UParTScore::compute_prob3_from_branches(my_udg, my_cvl, my_cvb,
                                                    my_svudg);
        pb = static_cast<float>(p.pb);
        pc = static_cast<float>(p.pc);
        pl = static_cast<float>(p.pl);
        if (!valid_score(hf) || !valid_score(bvc)) {
          const auto hf_bvc = UParTScore::hf_bvc_from_prob3(p);
          hf = static_cast<float>(hf_bvc.first);
          bvc = static_cast<float>(hf_bvc.second);
        }
      }
      if (valid_score(hf) && valid_score(bvc)) {
        cat = static_cast<int>(UParTScore::classify_from_scores(hf, bvc));
      }

      OutputTree(tree_name).Set( CachedJetBranchName(i, "prob3_B"), pb);
      OutputTree(tree_name).Set( CachedJetBranchName(i, "prob3_C"), pc);
      OutputTree(tree_name).Set( CachedJetBranchName(i, "prob3_L"), pl);
      OutputTree(tree_name).Set( CachedJetBranchName(i, "HFvLF"), hf);
      OutputTree(tree_name).Set( CachedJetBranchName(i, "BvC"), bvc);
      OutputTree(tree_name).Set( CachedJetBranchName(i, "Cat"), cat);
      OutputTree(tree_name).Set( CachedJetBranchName(i, "hadronFlavour"),
                IsDATA ? -1 : abs(Jets[i].hadronFlavour()));
      continue;
    }

    OutputTree(tree_name).Set( CachedJetBranchName(i, "UParT_B"),
              Jets[i].GetTaggerResult(JetTagging::JetFlavTagger::ParT,
                                      JetTagging::JetFlavTaggerScoreType::B));
    OutputTree(tree_name).Set( CachedJetBranchName(i, "UParT_CvB"),
              Jets[i].GetTaggerResult(JetTagging::JetFlavTagger::ParT,
                                      JetTagging::JetFlavTaggerScoreType::CvB));
    OutputTree(tree_name).Set( CachedJetBranchName(i, "UParT_CvL"),
              Jets[i].GetTaggerResult(JetTagging::JetFlavTagger::ParT,
                                      JetTagging::JetFlavTaggerScoreType::CvL));
    OutputTree(tree_name).Set(CachedJetBranchName(i, "UParT_CvNotB"),
        Jets[i].GetTaggerResult(JetTagging::JetFlavTagger::ParT,
                                JetTagging::JetFlavTaggerScoreType::CvNotB));
    OutputTree(tree_name).Set(CachedJetBranchName(i, "UParT_SvCB"),
        Jets[i].GetTaggerResult(JetTagging::JetFlavTagger::ParT,
                                JetTagging::JetFlavTaggerScoreType::SvCB));
    OutputTree(tree_name).Set(CachedJetBranchName(i, "UParT_SvUDG"),
        Jets[i].GetTaggerResult(JetTagging::JetFlavTagger::ParT,
                                JetTagging::JetFlavTaggerScoreType::SvUDG));
    OutputTree(tree_name).Set(CachedJetBranchName(i, "UParT_probUDG"),
        Jets[i].GetTaggerResult(JetTagging::JetFlavTagger::ParT,
                                JetTagging::JetFlavTaggerScoreType::probUDG));
    OutputTree(tree_name).Set(CachedJetBranchName(i, "UParT_probB"),
        Jets[i].GetTaggerResult(JetTagging::JetFlavTagger::ParT,
                                JetTagging::JetFlavTaggerScoreType::probB));
    OutputTree(tree_name).Set(CachedJetBranchName(i, "UParT_probBB"),
        Jets[i].GetTaggerResult(JetTagging::JetFlavTagger::ParT,
                                JetTagging::JetFlavTaggerScoreType::probBB));
    OutputTree(tree_name).Set( CachedJetBranchName(i, "MyUParT_B"), -1.f);
    OutputTree(tree_name).Set( CachedJetBranchName(i, "MyUParT_CvB"), -1.f);
    OutputTree(tree_name).Set( CachedJetBranchName(i, "MyUParT_CvL"), -1.f);
    OutputTree(tree_name).Set( CachedJetBranchName(i, "MyUParT_CvNotB"), -1.f);
    OutputTree(tree_name).Set( CachedJetBranchName(i, "MyUParT_SvCB"), -1.f);
    OutputTree(tree_name).Set( CachedJetBranchName(i, "MyUParT_SvUDG"), -1.f);
    OutputTree(tree_name).Set( CachedJetBranchName(i, "MyUParT_QvG"), -1.f);
    OutputTree(tree_name).Set( CachedJetBranchName(i, "MyUParT_probUDG"), -1.f);
    OutputTree(tree_name).Set( CachedJetBranchName(i, "MyUParT_HFvLF"), -1.f);
    OutputTree(tree_name).Set( CachedJetBranchName(i, "MyUParT_BvC"), -1.f);

    const float probudg =
        Jets[i].GetTaggerResult(JetTagging::JetFlavTagger::ParT,
                                JetTagging::JetFlavTaggerScoreType::probUDG);
    const float svudg =
        Jets[i].GetTaggerResult(JetTagging::JetFlavTagger::ParT,
                                JetTagging::JetFlavTaggerScoreType::SvUDG);
    const float cvl =
        Jets[i].GetTaggerResult(JetTagging::JetFlavTagger::ParT,
                                JetTagging::JetFlavTaggerScoreType::CvL);
    const float cvb =
        Jets[i].GetTaggerResult(JetTagging::JetFlavTagger::ParT,
                                JetTagging::JetFlavTaggerScoreType::CvB);
    float hf = -1.f;
    float bvc = -1.f;
    float pb = -1.f;
    float pc = -1.f;
    float pl = -1.f;
    int cat = -1;
    if (valid_score(probudg) && valid_score(svudg) && valid_score(cvl) &&
        valid_score(cvb)) {
      const auto p =
          UParTScore::compute_prob3_from_branches(probudg, cvl, cvb, svudg);
      const auto hf_bvc = UParTScore::hf_bvc_from_prob3(p);
      hf = static_cast<float>(hf_bvc.first);
      bvc = static_cast<float>(hf_bvc.second);
      pb = static_cast<float>(p.pb);
      pc = static_cast<float>(p.pc);
      pl = static_cast<float>(p.pl);
      cat = static_cast<int>(UParTScore::classify_from_scores(hf, bvc));
    }
    OutputTree(tree_name).Set( CachedJetBranchName(i, "prob3_B"), pb);
    OutputTree(tree_name).Set( CachedJetBranchName(i, "prob3_C"), pc);
    OutputTree(tree_name).Set( CachedJetBranchName(i, "prob3_L"), pl);
    OutputTree(tree_name).Set( CachedJetBranchName(i, "HFvLF"), hf);
    OutputTree(tree_name).Set( CachedJetBranchName(i, "BvC"), bvc);
    OutputTree(tree_name).Set( CachedJetBranchName(i, "Cat"), cat);
    OutputTree(tree_name).Set( CachedJetBranchName(i, "hadronFlavour"),
              IsDATA ? -1 : abs(Jets[i].hadronFlavour()));
  }

  if (channel == Channel::WCharm_Mu || channel == Channel::WCharm_El) {
    OutputTree(tree_name).Set( "WCharm_Jet0_nElectrons", wcharm_jet0_nelectrons);
    OutputTree(tree_name).Set( "WCharm_Jet0_nMuons", wcharm_jet0_nmuons);
    OutputTree(tree_name).Set( "WCharm_Jet0_nSoftMuons", wcharm_jet0_nsoftmuons);
    OutputTree(tree_name).Set( "WCharm_Jet0_nSVs", wcharm_jet0_nsv);
    OutputTree(tree_name).Set( "WCharm_Jet0_SV0_idx", wcharm_sv0_idx);
    OutputTree(tree_name).Set( "WCharm_Jet0_SV1_idx", wcharm_sv1_idx);

    OutputTree(tree_name).Set( "WCharm_Jet0_SV0_Pt", wcharm_sv0_pt);
    OutputTree(tree_name).Set( "WCharm_Jet0_SV0_Eta", wcharm_sv0_eta);
    OutputTree(tree_name).Set( "WCharm_Jet0_SV0_Phi", wcharm_sv0_phi);
    OutputTree(tree_name).Set( "WCharm_Jet0_SV0_Mass", wcharm_sv0_mass);
    OutputTree(tree_name).Set( "WCharm_Jet0_SV0_Charge", wcharm_sv0_charge);
    OutputTree(tree_name).Set( "WCharm_Jet0_SV0_Chi2", wcharm_sv0_chi2);
    OutputTree(tree_name).Set( "WCharm_Jet0_SV0_Ndof", wcharm_sv0_ndof);
    OutputTree(tree_name).Set( "WCharm_Jet0_SV0_NTracks", wcharm_sv0_ntracks);
    OutputTree(tree_name).Set( "WCharm_Jet0_SV0_Dlen", wcharm_sv0_dlen);
    OutputTree(tree_name).Set( "WCharm_Jet0_SV0_DlenSig", wcharm_sv0_dlenSig);
    OutputTree(tree_name).Set( "WCharm_Jet0_SV0_Dxy", wcharm_sv0_dxy);
    OutputTree(tree_name).Set( "WCharm_Jet0_SV0_DxySig", wcharm_sv0_dxySig);
    OutputTree(tree_name).Set( "WCharm_Jet0_SV0_PAngle", wcharm_sv0_pAngle);
    OutputTree(tree_name).Set( "WCharm_Jet0_SV0_X", wcharm_sv0_x);
    OutputTree(tree_name).Set( "WCharm_Jet0_SV0_Y", wcharm_sv0_y);
    OutputTree(tree_name).Set( "WCharm_Jet0_SV0_Z", wcharm_sv0_z);

    OutputTree(tree_name).Set( "WCharm_Jet0_SV1_Pt", wcharm_sv1_pt);
    OutputTree(tree_name).Set( "WCharm_Jet0_SV1_Eta", wcharm_sv1_eta);
    OutputTree(tree_name).Set( "WCharm_Jet0_SV1_Phi", wcharm_sv1_phi);
    OutputTree(tree_name).Set( "WCharm_Jet0_SV1_Mass", wcharm_sv1_mass);
    OutputTree(tree_name).Set( "WCharm_Jet0_SV1_Charge", wcharm_sv1_charge);
    OutputTree(tree_name).Set( "WCharm_Jet0_SV1_Chi2", wcharm_sv1_chi2);
    OutputTree(tree_name).Set( "WCharm_Jet0_SV1_Ndof", wcharm_sv1_ndof);
    OutputTree(tree_name).Set( "WCharm_Jet0_SV1_NTracks", wcharm_sv1_ntracks);
    OutputTree(tree_name).Set( "WCharm_Jet0_SV1_Dlen", wcharm_sv1_dlen);
    OutputTree(tree_name).Set( "WCharm_Jet0_SV1_DlenSig", wcharm_sv1_dlenSig);
    OutputTree(tree_name).Set( "WCharm_Jet0_SV1_Dxy", wcharm_sv1_dxy);
    OutputTree(tree_name).Set( "WCharm_Jet0_SV1_DxySig", wcharm_sv1_dxySig);
    OutputTree(tree_name).Set( "WCharm_Jet0_SV1_PAngle", wcharm_sv1_pAngle);
    OutputTree(tree_name).Set( "WCharm_Jet0_SV1_X", wcharm_sv1_x);
    OutputTree(tree_name).Set( "WCharm_Jet0_SV1_Y", wcharm_sv1_y);
    OutputTree(tree_name).Set( "WCharm_Jet0_SV1_Z", wcharm_sv1_z);
  }

  for (const auto &kv : weight_map) {
    OutputTree(tree_name).Set( "weight_" + kv.first, kv.second);
  }
  OutputTree(tree_name).Set( "MCNormalization", MCNormalizationWeight);
  OutputTree(tree_name).Fill();
}

void CalibrationTree::executeEvent() {
  ev = GetEvent();
  ResetEventCaches();
  if (HasFlag("Skim")) {
    AllMuonViews = GetAllMuonViews();
    AllElectronViews = GetAllElectronViews();
    AllJetViews = GetAllJetViews();
    Clear();
    SkimTree();
    return;
  }

  if (!PassEventPreselectionBeforeSystematics()) {
    return;
  }

  AllJetViews = GetAllJetViews();
  for (const auto &syst_dummy : *systHelper) {
    leptons.clear();
    executeEventFromParameter();
  }
}

void CalibrationTree::SkimTree() {
  Clear();
  if (!skimTreeInitialized) {
    RVec<TString> keeps = {"*"};
    RVec<TString> drops = {};
    skimTreeInitialized = true;
  }

  if (!myCorr->IsGoldenLumi(RunNumber, luminosityBlock))
    return;

  const bool pass_mu_trig = ev.PassTrigger(Mu_Trigger[DataEra.Data()]);
  const bool pass_el_trig = ev.PassTrigger(El_Trigger[DataEra.Data()]);
  if (!pass_mu_trig && !pass_el_trig)
    return;

  if (!PassJetVetoMap(AllJetViews))
    return;
  if (DataEra == "2022EE") {
    const float max_eta = std::numeric_limits<float>::infinity();
    auto eep_veto_indices =
        SelectJetIndices(AllJetViews, JetView::JetID::NOCUT, 30.f, max_eta);
    static_cast<void>(eep_veto_indices);
  }
  if (!PassMetFilter(AllJetViews, ev))
    return;

  std::vector<std::size_t> muon_veto_indices = SelectMuonIndices(
      AllMuonViews, Muon_Veto_ID, Muon_Veto_Pt, Muon_Veto_Eta);
  muon_veto_indices =
      SelectMuonIndices(AllMuonViews, muon_veto_indices, Muon_Veto_Iso,
                        Muon_Veto_Pt, Muon_Veto_Eta);

  std::vector<std::size_t> electron_veto_indices = SelectElectronIndices(
      AllElectronViews, Electron_Veto_ID, Electron_Veto_Pt, Electron_Veto_Eta);

  auto select_tight_muons = [&](MuonView::MuonID id, bool require_iso) {
    std::vector<std::size_t> indices = SelectMuonIndices(
        AllMuonViews, id, Muon_Tight_Pt[DataEra.Data()], Muon_Tight_Eta);
    if (require_iso) {
      indices = SelectMuonIndices(AllMuonViews, indices, Muon_Tight_Iso,
                                  Muon_Tight_Pt[DataEra.Data()],
                                  Muon_Tight_Eta);
    }
    return indices;
  };

  auto select_tight_electrons = [&](ElectronView::ElectronID id) {
    return SelectElectronIndices(AllElectronViews, id,
                                 Electron_Tight_Pt[DataEra.Data()],
                                 Electron_Tight_Eta);
  };

  auto count_extra_loose = [](const std::vector<std::size_t> &loose,
                              const std::vector<std::size_t> &tight) {
    std::size_t count = 0;
    for (const auto idx : loose) {
      if (std::find(tight.begin(), tight.end(), idx) == tight.end())
        ++count;
    }
    return count;
  };

  const std::vector<std::size_t> mu_tight_pog =
      select_tight_muons(MuonView::MuonID::POG_TIGHT, true);
  const std::vector<std::size_t> mu_tight_prompt =
      select_tight_muons(MuonView::MuonID::POG_PROMPTMVA_WP0p64, false);
  const std::vector<std::size_t> el_tight_wp80 =
      select_tight_electrons(ElectronView::ElectronID::POG_MVAISO_WP80);
  const std::vector<std::size_t> el_tight_prompt =
      select_tight_electrons(ElectronView::ElectronID::POG_PROMPTMVA_MEDIUM);

  auto pass_mu_case = [&](const std::vector<std::size_t> &mu_tight,
                          const std::vector<std::size_t> &el_tight) {
    if (mu_tight.size() != 1)
      return false;
    if (!el_tight.empty())
      return false;
    if (!electron_veto_indices.empty())
      return false;
    if (count_extra_loose(muon_veto_indices, mu_tight) != 0)
      return false;
    return true;
  };

  auto pass_el_case = [&](const std::vector<std::size_t> &mu_tight,
                          const std::vector<std::size_t> &el_tight) {
    if (el_tight.size() != 1)
      return false;
    if (!mu_tight.empty())
      return false;
    if (!muon_veto_indices.empty())
      return false;
    if (count_extra_loose(electron_veto_indices, el_tight) != 0)
      return false;
    return true;
  };

  const bool pass_mu =
      pass_mu_trig &&
      (pass_mu_case(mu_tight_pog, el_tight_wp80) ||
       pass_mu_case(mu_tight_prompt, el_tight_prompt));
  const bool pass_el =
      pass_el_trig &&
      (pass_el_case(mu_tight_pog, el_tight_wp80) ||
       pass_el_case(mu_tight_prompt, el_tight_prompt));
  if (!pass_mu && !pass_el)
    return;

  skim_passed_global_entries.push_back(currentEntry);
}

void CalibrationTree::WriteHist() {
  if (HasFlag("Skim")) {
    if (!skim_passed_global_entries.empty() && fChain) {
      std::sort(skim_passed_global_entries.begin(),
                skim_passed_global_entries.end());
      skim_passed_global_entries.erase(
          std::unique(skim_passed_global_entries.begin(),
                      skim_passed_global_entries.end()),
          skim_passed_global_entries.end());

      fChain->SetBranchStatus("*", 1);
      fChain->ResetBranchAddresses();

      TEntryList elist("skim_list", "Selected entries");
      for (Long64_t entry : skim_passed_global_entries) {
        elist.Enter(entry, fChain);
      }
      fChain->SetEntryList(&elist);

      if (TTree *curTree = fChain->GetTree()) {
        configureTreeCache(curTree);
      }

      TTree *skimTree = fChain->CopyTree("");
      if (skimTree) {
        skimTree->SetName("Events");
        treemap["Events"] = skimTree;
      }

      fChain->SetEntryList(0);
      skim_passed_global_entries.clear();
    }
  }

  AnalyzerCore::WriteHist();
}

void CalibrationTree::SetSystematicLambda(bool remove_flavtagging_sf) {
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
    else if (MCSample.Contains("TT")) {
      const int gent = genTtbarId % 100;
      if (gent >= 51 && gent <= 56)
        return "BB";
      else if (gent >= 41 && gent <= 46)
        return "CC";
      else
        return "JJ";
    } else
      return MCSample.Data();
  };
  std::function<float(MyCorrection::variation, TString)> MuF_lambda =
      [this, get_subproc_name](MyCorrection::variation syst,
                               TString /*source*/) {
        const std::string subproc_name = get_subproc_name();
        switch (syst) {
        case MyCorrection::variation::up:
          return GetScaleVariation(MyCorrection::variation::up,
                                   MyCorrection::variation::nom) *
                 modelling_patches[subproc_name].patch_ScaleVariation[4];
        case MyCorrection::variation::down:
          return GetScaleVariation(MyCorrection::variation::down,
                                   MyCorrection::variation::nom) *
                 modelling_patches[subproc_name].patch_ScaleVariation[3];
        default:
          return 1.f;
        }
      };

  std::function<float(MyCorrection::variation, TString)> MuR_lambda =
      [this, get_subproc_name](MyCorrection::variation syst,
                               TString /*source*/) {
        const std::string subproc_name = get_subproc_name();
        switch (syst) {
        case MyCorrection::variation::up:
          return GetScaleVariation(MyCorrection::variation::nom,
                                   MyCorrection::variation::up) *
                 modelling_patches[subproc_name].patch_ScaleVariation[6];
        case MyCorrection::variation::down:
          return GetScaleVariation(MyCorrection::variation::nom,
                                   MyCorrection::variation::down) *
                 modelling_patches[subproc_name].patch_ScaleVariation[1];
        default:
          return 1.f;
        }
      };

  std::function<float(MyCorrection::variation, TString)> ISR_lambda =
      [this, get_subproc_name](MyCorrection::variation syst,
                               TString /*source*/) {
        const std::string subproc_name = get_subproc_name();
        switch (syst) {
        case MyCorrection::variation::up:
          return GetPSWeight(MyCorrection::variation::up,
                             MyCorrection::variation::nom) *
                 modelling_patches[subproc_name].patch_PSVariation[0];
        case MyCorrection::variation::down:
          return GetPSWeight(MyCorrection::variation::down,
                             MyCorrection::variation::nom) *
                 modelling_patches[subproc_name].patch_PSVariation[2];
        default:
          return 1.f;
        }
      };

  std::function<float(MyCorrection::variation, TString)> FSR_lambda =
      [this, get_subproc_name](MyCorrection::variation syst,
                               TString /*source*/) {
        switch (syst) {
        case MyCorrection::variation::up:
          return GetPSWeight(MyCorrection::variation::nom,
                             MyCorrection::variation::up) *
                 modelling_patches[get_subproc_name()].patch_PSVariation[1];
        case MyCorrection::variation::down:
          return GetPSWeight(MyCorrection::variation::nom,
                             MyCorrection::variation::down) *
                 modelling_patches[get_subproc_name()].patch_PSVariation[3];
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
  // weight_function_map["BFrag"] = dummy_lambda;

  if (remove_flavtagging_sf)
    weight_function_map["btag"] = dummy_lambda;
  else
    weight_function_map["btag"] = dummy_lambda;
  systHelper->assignWeightFunctionMap(weight_function_map);
}

void CalibrationTree::Clear() {
  HT = 0;
  n_jets = 0;
  n_b_tagged_jets = 0;
  n_c_tagged_jets = 0;
  n_hf_jets = 0;
  n_hadronFlav_b_jets = 0;
  n_hadronFlav_c_jets = 0;
  leptons.clear();
  Muons = MuonViewCollection();
  Muons_Veto = MuonViewCollection();
  Electrons = ElectronViewCollection();
  Electrons_Veto = ElectronViewCollection();
  lepton = Lepton();
  Jets = SelectedJetViewCollection();
  MET = Particle();
  log_chi2 = -9999.f;
  ttsl_fit_status = -9999;
  ttsl_best_comb_idx = -1;
  ttsl_second_comb_idx = -1;
  ttsl_best_nu_idx = -1;
  ttsl_n_valid_combs = 0;
  ttsl_n_prefit_candidates = 0;
  ttsl_n_fit_candidates = 0;
  ttsl_met_propagated = 0;
  ttsl_fit_chi2 = -9999.f;
  ttsl_log_fit_chi2 = -9999.f;
  ttsl_second_fit_chi2 = -9999.f;
  ttsl_delta_chi2 = -9999.f;
  ttsl_prefit_chi2 = -9999.f;
  ttsl_prefit_mw = -9999.f;
  ttsl_met_pt_before_jet_syst = -9999.f;
  ttsl_met_pt_after_jet_syst = -9999.f;
  mmuj0 = -9999.f;
  mmuj1 = -9999.f;
  melj0 = -9999.f;
  melj1 = -9999.f;
  wcharm_soft_mu_pt = -9999.f;
  wcharm_dimuon_mass = -9999.f;
  wcharm_jet_muEF = -9999.f;
  wcharm_jet_neEmEF = -9999.f;
  wcharm_jet_muEF_plus_neEmEF = -9999.f;
  wcharm_mt = -9999.f;
  wcharm_w_pt = -9999.f;
  wcharm_dphi_met_jet = -9999.f;
  wcharm_dphi_lep_jet = -9999.f;
  wcharm_dphi_w_jet = -9999.f;
  wcharm_pt_ratio = -9999.f;
  wcharm_dphi_trkmet_met = -9999.f;
  wcharm_jet0_nelectrons = -9999;
  wcharm_jet0_nmuons = -9999;
  wcharm_jet0_nsoftmuons = -9999;
  wcharm_jet0_nsv = -9999;
  wcharm_sv0_idx = -1;
  wcharm_sv1_idx = -1;
  wcharm_sv0_pt = -9999.f;
  wcharm_sv0_eta = -9999.f;
  wcharm_sv0_phi = -9999.f;
  wcharm_sv0_mass = -9999.f;
  wcharm_sv0_charge = -9999.f;
  wcharm_sv0_chi2 = -9999.f;
  wcharm_sv0_ndof = -9999.f;
  wcharm_sv0_ntracks = -9999.f;
  wcharm_sv0_dlen = -9999.f;
  wcharm_sv0_dlenSig = -9999.f;
  wcharm_sv0_dxy = -9999.f;
  wcharm_sv0_dxySig = -9999.f;
  wcharm_sv0_pAngle = -9999.f;
  wcharm_sv0_x = -9999.f;
  wcharm_sv0_y = -9999.f;
  wcharm_sv0_z = -9999.f;
  wcharm_sv1_pt = -9999.f;
  wcharm_sv1_eta = -9999.f;
  wcharm_sv1_phi = -9999.f;
  wcharm_sv1_mass = -9999.f;
  wcharm_sv1_charge = -9999.f;
  wcharm_sv1_chi2 = -9999.f;
  wcharm_sv1_ndof = -9999.f;
  wcharm_sv1_ntracks = -9999.f;
  wcharm_sv1_dlen = -9999.f;
  wcharm_sv1_dlenSig = -9999.f;
  wcharm_sv1_dxy = -9999.f;
  wcharm_sv1_dxySig = -9999.f;
  wcharm_sv1_pAngle = -9999.f;
  wcharm_sv1_x = -9999.f;
  wcharm_sv1_y = -9999.f;
  wcharm_sv1_z = -9999.f;
  dycharm_has_tagprobe = false;
  dycharm_tag_pt = -9999.f;
  dycharm_tag_eta = -9999.f;
  dycharm_tag_phi = -9999.f;
  dycharm_probe_pt = -9999.f;
  dycharm_probe_eta = -9999.f;
  dycharm_probe_phi = -9999.f;
  dycharm_z_pt = -9999.f;
  dycharm_z_mass = -9999.f;
  dycharm_jj_pt = -9999.f;
  dycharm_jj_mass = -9999.f;
  dycharm_dR_jj = -9999.f;
  dycharm_dphi_z_jj = -9999.f;
  dycharm_ptbal_z_jj = -9999.f;
  dycharm_ptasym_jj = -9999.f;

  qcdcharm_has_tagprobe = false;
  qcdcharm_tag_pt = -9999.f;
  qcdcharm_tag_eta = -9999.f;
  qcdcharm_tag_phi = -9999.f;
  qcdcharm_probe_pt = -9999.f;
  qcdcharm_probe_eta = -9999.f;
  qcdcharm_probe_phi = -9999.f;
  qcdcharm_jj_pt = -9999.f;
  qcdcharm_jj_mass = -9999.f;
  qcdcharm_dR_jj = -9999.f;
  qcdcharm_dphi_jj = -9999.f;
  qcdcharm_ptasym_jj = -9999.f;
  best_KF_result.clear();
}

void CalibrationTree::executeEventFromParameter() {
  Clear();

  if (!PassBaseLineSelection())
    return;

  const std::string channel_str = GetChannelString(channel).Data();
  const std::string base_path = channel_str + "/Inclusive/";
  const std::string current_iter_name =
      systHelper ? systHelper->getCurrentSysName() : "Central";
  const std::string current_iter_prefix = base_path + current_iter_name + "/";

  std::string sample_postfix = Sample_Shorthand[MCSample.Data()];
  // -------------------------
  // DATA: Inclusive/Central/data_obs 만 채움
  // -------------------------
  if (IsDATA) {
    const std::unordered_map<std::string, float> data_weights = {
        {"Central", 1.f}};
    FillHistogramsAtThisPoint(base_path + "Central/data_obs", 1.f);
    FillTreeAtThisPoint(current_iter_prefix + "data_obs", 1.f, data_weights);

    return;
  }

  const float normalization = MCNormalization();
  const auto weight_map = systHelper->calculateWeight(false);
  FillTreeAtThisPoint(current_iter_prefix + Sample_Shorthand[MCSample.Data()],
                      normalization, weight_map);
  for (const auto &kv : weight_map) {
    const std::string &syst = kv.first;
    const float w = kv.second;
    FillHistogramsAtThisPoint(base_path + syst + "/" + sample_postfix,
                              w * normalization);
  }
}

float CalibrationTree::LeptonTriggerWeight(bool isEle,
                                           const MyCorrection::variation syst,
                                           const TString & /*source*/) {

  if (Muons.size() == 1 && Electrons.size() == 0)
    return myCorr->GetMuonTriggerSF(Mu_Trigger_SF_Key[DataEra.Data()], Muons[0],
                                    syst);

  else if (Muons.size() == 0 && Electrons.size() == 1)
    return myCorr->GetElectronTriggerSF(El_Trigger_SF_Key[DataEra.Data()],
                                        lepton.Eta(), lepton.Pt(), lepton.Phi(),
                                        syst);
  else if (Muons.size() == 1 && Electrons.size() == 1) {
    float mu_eff_data = myCorr->GetMuonTriggerEff(
        Mu_Trigger_SF_Key[DataEra.Data()], fabs(Muons[0].Eta()), Muons[0].Pt(),
        true, syst);
    float mu_eff_mc = myCorr->GetMuonTriggerEff(
        Mu_Trigger_SF_Key[DataEra.Data()], fabs(Muons[0].Eta()), Muons[0].Pt(),
        false, syst);
    float el_eff_data = myCorr->GetElectronTriggerEff(
        El_Trigger_SF_Key[DataEra.Data()], fabs(Electrons[0].Eta()),
        Electrons[0].Pt(), Electrons[0].Phi(), true, syst);
    float el_eff_mc = myCorr->GetElectronTriggerEff(
        El_Trigger_SF_Key[DataEra.Data()], fabs(Electrons[0].Eta()),
        Electrons[0].Pt(), Electrons[0].Phi(), false, syst);
    float combined_eff_data = 1 - (1 - mu_eff_data) * (1 - el_eff_data);
    float combined_eff_mc = 1 - (1 - mu_eff_mc) * (1 - el_eff_mc);
    if (combined_eff_mc == 0)
      return 1.f;
    return combined_eff_data / combined_eff_mc;
  }

  else if (Muons.size() == 2 && Electrons.size() == 0) {
    return myCorr->GetMuonTriggerSF(Mu_Trigger_SF_Key[DataEra.Data()], Muons,
                                    syst);
  }

  else if (Muons.size() == 0 && Electrons.size() == 0) {
    // QCD dijet (no leptons): no trigger SF applied
    return 1.f;
  }

  else
    throw std::runtime_error("Invalid lepton configuration for trigger "
                             "weight calculation. nMuons: " +
                             std::to_string(Muons.size()) + ", nElectrons: " +
                             std::to_string(Electrons.size()));
}

bool CalibrationTree::PassBaseLineSelection() {
  if (!myCorr->IsGoldenLumi(RunNumber, luminosityBlock)) {
    return false;
  }
  switch (channel) {
  case Channel::TTDilep:
    return PassTTDilepBaselineSelection();
  case Channel::WCharm_Mu:
    return PassWCharmBaselineSelection();
  case Channel::WCharm_El:
    return PassWCharmBaselineSelection();
  case Channel::DYLight:
    return PassDYLightBaselineSelection();
  case Channel::DYCharm:
    return PassDYCharmBaselineSelection();
  case Channel::QCDCharmDijet:
    return PassQCDCharmDijetSelection();
  case Channel::TTSemilep:
    return PassTTSemilepBaselineSelection();
  default:
    throw std::runtime_error("Unknown channel for baseline selection.");
  }
}

bool CalibrationTree::PassTTDilepBaselineSelection() {
  if (IsDATA && DataStream.Contains("EGamma")) {
    if (!ev.PassTrigger(El_Trigger[DataEra.Data()]))
      return false;
    if (ev.PassTrigger(Mu_Trigger[DataEra.Data()]))
      return false;
  } else {
    if (!(ev.PassTrigger(Mu_Trigger[DataEra.Data()]) ||
          ev.PassTrigger(El_Trigger[DataEra.Data()])))
      return false;
  }
  if (!PassJetVetoMap(AllJetViews))
    return false;
  if (!PassMetFilter(AllJetViews, ev))
    return false;
  BuildLeptonSelectionCache();
  const auto &loose_muon_indices = leptonSelectionCache.looseMuonIndices;
  const auto &tight_muon_indices = leptonSelectionCache.tightMuonIndices;
  const auto &loose_electron_indices =
      leptonSelectionCache.looseElectronIndices;
  const auto &tight_electron_indices =
      leptonSelectionCache.tightElectronIndices;
  if (!(tight_muon_indices.size() == 1 && tight_electron_indices.size() == 1 &&
        loose_muon_indices.size() == 1 && loose_electron_indices.size() == 1))
    return false;
  if ((AllMuonViews[tight_muon_indices[0]].Charge() *
       AllElectronViews[tight_electron_indices[0]].Charge()) > 0)
    return false;
  Muons = SelectMuonViews(AllMuonViews, tight_muon_indices);
  Electrons = SelectElectronViews(AllElectronViews, tight_electron_indices);

  MyCorrection::variation jesVar = MyCorrection::variation::nom;
  MyCorrection::variation jerVar = MyCorrection::variation::nom;
  const std::string systTarget = systHelper->getCurrentIterSysTarget();
  const TString systSource = systHelper->getCurrentIterSysSource();
  const MyCorrection::variation systVar = systHelper->getCurrentIterVariation();

  MET = ev.GetMETVector(Event::MET_Type::PUPPI);
  bool doJetPropagation = true;
  if (systTarget.find("Jet_En") != std::string::npos) {
    const bool doBreakdown = HasFlag("doBreakdown");
    if (!PrepareJetJESVariations(AllJetViews, systSource, doBreakdown))
      return false;
    if (!IsDATA)
      jesVar = systVar;
  } else if (systTarget == "Jet_Res") {
    if (!IsDATA)
      jerVar = systVar;
  } else if (systTarget == "UE") {
    MET = ev.GetMETVector(Event::MET_Type::PUPPI, systVar,
                          Event::MET_Syst::UE);
    doJetPropagation = false;
  }

  if (doJetPropagation)
    PropagateJetSystToMET(AllJetViews, MET, jesVar, jerVar);
  if (MET.Pt() < 20.)
    return false;
  std::vector<size_t> jet_indices = SelectJetIndices(
      AllJetViews, Jet_ID, 25., 2.5, jesVar, jerVar);
  jet_indices = JetsVetoLeptonInside(AllJetViews, jet_indices, AllElectronViews,
                                     tight_electron_indices, AllMuonViews,
                                     tight_muon_indices, 0.4);
  if (jet_indices.size() != 2)
    return false;
  Particle emu(Muons[0].P4() + Electrons[0].P4());
  float mt_emumet = std::sqrt(2.f * emu.Pt() * MET.Pt() *
                              (1.f - std::cos(emu.DeltaPhi(MET))));
  if (mt_emumet < 100.f)
    return false;
  Jets = SelectJetViews(AllJetViews, jet_indices, jesVar, jerVar);


  mmuj0 = (Muons[0].P4() + Jets[0].P4()).M();
  mmuj1 = (Muons[0].P4() + Jets[1].P4()).M();
  melj0 = (Electrons[0].P4() + Jets[0].P4()).M();
  melj1 = (Electrons[0].P4() + Jets[1].P4()).M();

  // constexpr float kMinJetCut   = 128.0f;  // t_minjet
  // constexpr float kMiniMaxCut  = 127.4f;  // thr_minimax

  // const float mlj0_min = std::min(mmuj0, melj0);
  // const float mlj1_min = std::min(mmuj1, melj1);
  // if (!(mlj0_min < kMinJetCut && mlj1_min < kMinJetCut))
  //   return false;

  // const float maxA = std::max(mmuj0, melj1);  // (mu-j0, e-j1)
  // const float maxB = std::max(mmuj1, melj0);  // (mu-j1, e-j0)
  // const float mbl_minimax = std::min(maxA, maxB);
  // if (!(mbl_minimax < kMiniMaxCut))
  //   return false;

  return true;
}

bool CalibrationTree::PassWCharmBaselineSelection() {
  const bool kRequireJetSoftMuon = false;

  if (!PassJetVetoMap(AllJetViews))
    return false;
  if (!PassMetFilter(AllJetViews, ev))
    return false;

  BuildLeptonSelectionCache();
  const auto &loose_muon_indices = leptonSelectionCache.looseMuonIndices;
  const auto &tight_muon_indices = leptonSelectionCache.tightMuonIndices;
  const auto &loose_electron_indices =
      leptonSelectionCache.looseElectronIndices;
  const auto &tight_electron_indices =
      leptonSelectionCache.tightElectronIndices;

  Lepton primary_lepton;
  bool primary_is_mu = false;
  MuonView primary_muon;
  bool has_primary_muon = false;

  if (channel == Channel::WCharm_Mu) {
    if (!ev.PassTrigger(Mu_Trigger[DataEra.Data()]))
      return false;

    if (!(tight_muon_indices.size() == 1 &&
          tight_electron_indices.empty() &&
          loose_muon_indices.size() == 1 &&
          loose_electron_indices.empty()))
      return false;

    Muons = SelectMuonViews(AllMuonViews, tight_muon_indices);
    primary_muon = Muons[0];
    has_primary_muon = true;
    primary_lepton = MakeLeptonSnapshot(Muons[0]);
    primary_is_mu = true;
  } else if (channel == Channel::WCharm_El) {
    if (!ev.PassTrigger(El_Trigger[DataEra.Data()]))
      return false;

    if (!(tight_muon_indices.empty() &&
          tight_electron_indices.size() == 1 &&
          loose_muon_indices.empty() &&
          loose_electron_indices.size() == 1))
      return false;

    Electrons = SelectElectronViews(AllElectronViews, tight_electron_indices);
    primary_lepton = MakeLeptonSnapshot(Electrons[0]);
  } else {
    return false;
  }

  lepton = primary_lepton;

  MyCorrection::variation jesVar = MyCorrection::variation::nom;
  MyCorrection::variation jerVar = MyCorrection::variation::nom;
  const std::string systTarget = systHelper->getCurrentIterSysTarget();
  const TString systSource = systHelper->getCurrentIterSysSource();
  const MyCorrection::variation systVar =
      systHelper->getCurrentIterVariation();

  MET = ev.GetMETVector(Event::MET_Type::PUPPI);

  bool doJetPropagation = true;
  if (systTarget.find("Jet_En") != std::string::npos) {
    const bool doBreakdown = HasFlag("doBreakdown");
    if (!PrepareJetJESVariations(AllJetViews, systSource, doBreakdown))
      return false;
    if (!IsDATA)
      jesVar = systVar;
  } else if (systTarget == "Jet_Res") {
    if (!IsDATA)
      jerVar = systVar;
  } else if (systTarget == "UE") {
    MET = ev.GetMETVector(Event::MET_Type::PUPPI, systVar,
                          Event::MET_Syst::UE);
    doJetPropagation = false;
  }

  if (doJetPropagation)
    PropagateJetSystToMET(AllJetViews, MET, jesVar, jerVar);

  std::vector<size_t> jet_indices =
      SelectJetIndices(AllJetViews, Jet_ID, 25.f, 2.5, jesVar, jerVar);
  jet_indices = JetsVetoLeptonInside(AllJetViews, jet_indices, AllElectronViews,
                                     tight_electron_indices, AllMuonViews,
                                     tight_muon_indices, 0.4);

  if (jet_indices.size() != 1)
    return false;

  Jets = SelectJetViews(AllJetViews, jet_indices, jesVar, jerVar);

  EnsureSVViews();
  {
    const auto &jetView = AllJetViews[jet_indices[0]];
    wcharm_jet0_nsv = static_cast<int>(jetView.NSVs());

    auto valid_sv_index = [&](short idx) -> int {
      if (idx < 0)
        return -1;
      const auto uidx = static_cast<std::size_t>(idx);
      return uidx < AllSVViews.size() ? static_cast<int>(idx) : -1;
    };

    wcharm_sv0_idx = valid_sv_index(jetView.SvIdx1());
    wcharm_sv1_idx = valid_sv_index(jetView.SvIdx2());

    auto fill_sv = [&](int sv_idx, float &pt, float &eta, float &phi,
                       float &mass, float &charge, float &chi2, float &ndof,
                       float &ntracks, float &dlen, float &dlenSig,
                       float &dxy, float &dxySig, float &pAngle, float &x,
                       float &y, float &z) {
      if (sv_idx < 0)
        return;
      const auto &sv = AllSVViews[static_cast<std::size_t>(sv_idx)];
      if (!sv.valid())
        return;
      pt = sv.Pt();
      eta = sv.Eta();
      phi = sv.Phi();
      mass = sv.Mass();
      charge = static_cast<float>(sv.Charge());
      chi2 = sv.Chi2();
      ndof = sv.Ndof();
      ntracks = static_cast<float>(sv.NTracks());
      dlen = sv.Dlen();
      dlenSig = sv.DlenSig();
      dxy = sv.Dxy();
      dxySig = sv.DxySig();
      pAngle = sv.PAngle();
      x = sv.X();
      y = sv.Y();
      z = sv.Z();
    };

    fill_sv(wcharm_sv0_idx, wcharm_sv0_pt, wcharm_sv0_eta, wcharm_sv0_phi,
            wcharm_sv0_mass, wcharm_sv0_charge, wcharm_sv0_chi2,
            wcharm_sv0_ndof, wcharm_sv0_ntracks, wcharm_sv0_dlen,
            wcharm_sv0_dlenSig, wcharm_sv0_dxy, wcharm_sv0_dxySig,
            wcharm_sv0_pAngle, wcharm_sv0_x, wcharm_sv0_y, wcharm_sv0_z);

    fill_sv(wcharm_sv1_idx, wcharm_sv1_pt, wcharm_sv1_eta, wcharm_sv1_phi,
            wcharm_sv1_mass, wcharm_sv1_charge, wcharm_sv1_chi2,
            wcharm_sv1_ndof, wcharm_sv1_ntracks, wcharm_sv1_dlen,
            wcharm_sv1_dlenSig, wcharm_sv1_dxy, wcharm_sv1_dxySig,
            wcharm_sv1_pAngle, wcharm_sv1_x, wcharm_sv1_y, wcharm_sv1_z);
  }

  if (kRequireJetSoftMuon) {
    std::vector<size_t> soft_muon_indices = SelectMuonIndices(
        AllMuonViews, MuonView::MuonID::POG_TIGHT, 5.f, Muon_Tight_Eta);

    std::vector<size_t> soft_muon_indices_iso =
        SelectMuonIndices(AllMuonViews, soft_muon_indices,
                          MuonView::MuonID::POG_PFISO_LOOSE, 5.f, Muon_Tight_Eta);

    std::vector<size_t> soft_muon_indices_noniso;
    soft_muon_indices_noniso.reserve(soft_muon_indices.size());
    for (auto idx : soft_muon_indices) {
      if (std::find(soft_muon_indices_iso.begin(), soft_muon_indices_iso.end(),
                    idx) == soft_muon_indices_iso.end())
        soft_muon_indices_noniso.push_back(idx);
    }

    auto deltaR2 = [](float eta1, float phi1, float eta2, float phi2) {
      const float deta = eta1 - eta2;
      float dphi = phi1 - phi2;
      while (dphi > M_PI)
        dphi -= 2.f * M_PI;
      while (dphi < -M_PI)
        dphi += 2.f * M_PI;
      return deta * deta + dphi * dphi;
    };

    constexpr float dR2_cut = 0.4f * 0.4f;
    constexpr size_t kAmbiguous = std::numeric_limits<size_t>::max();

    std::unordered_map<size_t, size_t> jet_to_mu;
    jet_to_mu.reserve(soft_muon_indices_noniso.size());

    for (auto mu_idx : soft_muon_indices_noniso) {
      const float mu_eta = AllMuonViews[mu_idx].Eta();
      const float mu_phi = AllMuonViews[mu_idx].Phi();

      float best_dR2 = dR2_cut;
      size_t best_jet_idx = std::numeric_limits<size_t>::max();

      for (auto jet_idx : jet_indices) {
        const float jet_eta = AllJetViews[jet_idx].Eta();
        const float jet_phi = AllJetViews[jet_idx].Phi();
        const float dR2 = deltaR2(jet_eta, jet_phi, mu_eta, mu_phi);
        if (dR2 < best_dR2) {
          best_dR2 = dR2;
          best_jet_idx = jet_idx;
        }
      }

      if (best_jet_idx != std::numeric_limits<size_t>::max()) {
        auto it = jet_to_mu.find(best_jet_idx);
        if (it == jet_to_mu.end())
          jet_to_mu.emplace(best_jet_idx, mu_idx);
        else
          it->second = kAmbiguous;
      }
    }

    size_t unique_softmu_idx = std::numeric_limits<size_t>::max();
    size_t unique_count = 0;
    for (const auto &entry : jet_to_mu) {
      const size_t mu_idx = entry.second;
      if (mu_idx == kAmbiguous)
        continue;
      unique_softmu_idx = mu_idx;
      ++unique_count;
    }
    if (unique_count != 1)
      return false;

    wcharm_jet_muEF = Jets[0].muEF();
    wcharm_jet_neEmEF = Jets[0].neEmEF();
    wcharm_jet_muEF_plus_neEmEF = wcharm_jet_muEF + wcharm_jet_neEmEF;
    if (wcharm_jet_muEF > 0.5f)
      return false;
    const float max_muEF_plus_neEmEF =
        channel == Channel::WCharm_Mu ? 0.7f : 1.0f;
    if (wcharm_jet_muEF_plus_neEmEF > max_muEF_plus_neEmEF)
      return false;

    const MuonView soft_muon = AllMuonViews[unique_softmu_idx];
    wcharm_soft_mu_pt = soft_muon.Pt();

    if (primary_is_mu && has_primary_muon) {
      Particle dimuon(primary_muon.P4() + soft_muon.P4());
      wcharm_dimuon_mass = dimuon.M();
      if (wcharm_dimuon_mass < 12.f ||
          (wcharm_dimuon_mass > 80.f && wcharm_dimuon_mass < 100.f))
        return false;
    }

    if (primary_lepton.Charge() * soft_muon.Charge() > 0)
      return false;
  }

  if (MET.Pt() < 30.f)
    return false;

  Particle WT = primary_lepton + MET;
  const float MT = std::sqrt(2.f * primary_lepton.Pt() * MET.Pt() *
                             (1.f - std::cos(primary_lepton.DeltaPhi(MET))));
  wcharm_mt = MT;
  wcharm_w_pt = WT.Pt();
  if (wcharm_mt < 40.f || wcharm_mt > 120.f)
    return false;
  if (wcharm_w_pt < 30.f)
    return false;

  wcharm_dphi_met_jet = std::abs(MET.DeltaPhi(Jets[0]));
  if (wcharm_dphi_met_jet < 1.0f)
    return false;
  wcharm_dphi_lep_jet = std::abs(primary_lepton.DeltaPhi(Jets[0]));
  if (wcharm_dphi_lep_jet > 2.0f)
    return false;
  wcharm_dphi_w_jet = std::abs(WT.DeltaPhi(Jets[0]));
  if (wcharm_dphi_w_jet < 2.0f)
    return false;

  wcharm_pt_ratio = Jets[0].Pt() / WT.Pt();
  if (wcharm_pt_ratio < 0.5f || wcharm_pt_ratio > 2.0f)
    return false;

  const float dphi_trkmet_met =
      std::abs(std::atan2(std::sin(TrkMET_phi - MET.Phi()),
                          std::cos(TrkMET_phi - MET.Phi())));
  wcharm_dphi_trkmet_met = dphi_trkmet_met;
  if (wcharm_dphi_trkmet_met > 1.0f)
    return false;

  return true;
}

bool CalibrationTree::PassDYLightBaselineSelection() {
  if (!(ev.PassTrigger(Mu_Trigger[DataEra.Data()])))
    return false;
  if (!PassJetVetoMap(AllJetViews))
    return false;
  if (!PassMetFilter(AllJetViews, ev))
    return false;
  BuildLeptonSelectionCache();
  const auto &loose_muon_indices = leptonSelectionCache.looseMuonIndices;
  const auto &tight_muon_indices = leptonSelectionCache.tightMuonIndices;
  const auto &loose_electron_indices =
      leptonSelectionCache.looseElectronIndices;
  const auto &tight_electron_indices =
      leptonSelectionCache.tightElectronIndices;
  if (!(tight_muon_indices.size() == 2 && tight_electron_indices.size() == 0 &&
        loose_muon_indices.size() == 2 && loose_electron_indices.size() == 0))
    return false;
  if ((AllMuonViews[tight_muon_indices[0]].Charge() *
       AllMuonViews[tight_muon_indices[1]].Charge()) > 0)
    return false;
  Muons = SelectMuonViews(AllMuonViews, tight_muon_indices);
  Particle ZCand(Muons[0].P4() + Muons[1].P4());
  if (ZCand.M() < 81. || ZCand.M() > 101.)
    return false;

  MyCorrection::variation jesVar = MyCorrection::variation::nom;
  MyCorrection::variation jerVar = MyCorrection::variation::nom;
  const std::string systTarget = systHelper->getCurrentIterSysTarget();
  const TString systSource = systHelper->getCurrentIterSysSource();
  const MyCorrection::variation systVar = systHelper->getCurrentIterVariation();

  MET = ev.GetMETVector(Event::MET_Type::PUPPI);
  bool doJetPropagation = true;
  if (systTarget.find("Jet_En") != std::string::npos) {
    const bool doBreakdown = HasFlag("doBreakdown");
    if (!PrepareJetJESVariations(AllJetViews, systSource, doBreakdown))
      return false;
    if (!IsDATA)
      jesVar = systVar;
  } else if (systTarget == "Jet_Res") {
    if (!IsDATA)
      jerVar = systVar;
  } else if (systTarget == "UE") {
    MET = ev.GetMETVector(Event::MET_Type::PUPPI, systVar,
                          Event::MET_Syst::UE);
    doJetPropagation = false;
  }

  if (doJetPropagation)
    PropagateJetSystToMET(AllJetViews, MET, jesVar, jerVar);
  std::vector<size_t> jet_indices = SelectJetIndices(
      AllJetViews, Jet_ID, 25., 2.5, jesVar, jerVar);
  jet_indices = JetsVetoLeptonInside(AllJetViews, jet_indices, AllElectronViews,
                                     tight_electron_indices, AllMuonViews,
                                     tight_muon_indices, 0.4);
  if (jet_indices.size() != 1)
    return false;
  Jets = SelectJetViews(AllJetViews, jet_indices, jesVar, jerVar);
  if (ZCand.Pt() / Jets[0].Pt() < 0.75 || ZCand.Pt() / Jets[0].Pt() > 1.25)
    return false;
  if (ZCand.DeltaPhi(Jets[0]) < 2.f)
    return false;
  return true;
}

bool CalibrationTree::PassDYCharmBaselineSelection() {
  // 0) Event cleaning
  if (!ev.PassTrigger(Mu_Trigger[DataEra.Data()])) return false;
  if (!PassJetVetoMap(AllJetViews)) return false;
  if (!PassMetFilter(AllJetViews, ev)) return false;

  // 1) Z->mumu baseline
  BuildLeptonSelectionCache();
  const auto &loose_muon_indices = leptonSelectionCache.looseMuonIndices;
  const auto &tight_muon_indices = leptonSelectionCache.tightMuonIndices;
  const auto &loose_electron_indices =
      leptonSelectionCache.looseElectronIndices;
  const auto &tight_electron_indices =
      leptonSelectionCache.tightElectronIndices;

  if (!(tight_muon_indices.size() == 2 &&
        loose_muon_indices.size() == 2 &&
        tight_electron_indices.empty() &&
        loose_electron_indices.empty()))
    return false;

  if (AllMuonViews[tight_muon_indices[0]].Charge() *
          AllMuonViews[tight_muon_indices[1]].Charge() > 0)
    return false;

  Muons = SelectMuonViews(AllMuonViews, tight_muon_indices);

  Particle ZCand(Muons[0].P4() + Muons[1].P4());
  if (ZCand.M() < 81. || ZCand.M() > 101.) return false;

  // (optional but usually helpful for Z+jets)
  // if (ZCand.Pt() < 30.) return false;

  // 2) Jet collection (JES/JER propagate)
  MyCorrection::variation jesVar = MyCorrection::variation::nom;
  MyCorrection::variation jerVar = MyCorrection::variation::nom;
  const std::string systTarget = systHelper->getCurrentIterSysTarget();
  const TString systSource = systHelper->getCurrentIterSysSource();
  const MyCorrection::variation systVar = systHelper->getCurrentIterVariation();

  MET = ev.GetMETVector(Event::MET_Type::PUPPI);
  bool doJetPropagation = true;
  if (systTarget.find("Jet_En") != std::string::npos) {
    const bool doBreakdown = HasFlag("doBreakdown");
    if (!PrepareJetJESVariations(AllJetViews, systSource, doBreakdown))
      return false;
    if (!IsDATA)
      jesVar = systVar;
  } else if (systTarget == "Jet_Res") {
    if (!IsDATA)
      jerVar = systVar;
  } else if (systTarget == "UE") {
    MET = ev.GetMETVector(Event::MET_Type::PUPPI, systVar,
                          Event::MET_Syst::UE);
    doJetPropagation = false;
  }

  if (doJetPropagation)
    PropagateJetSystToMET(AllJetViews, MET, jesVar, jerVar);

  std::vector<size_t> jet_indices =
      SelectJetIndices(AllJetViews, Jet_ID, /*pt=*/25., /*eta=*/2.5, jesVar, jerVar);

  jet_indices = JetsVetoLeptonInside(AllJetViews, jet_indices,
                                     AllElectronViews, tight_electron_indices,
                                     AllMuonViews, tight_muon_indices,
                                     /*dR=*/0.4);

  // g->cc: extra jets가 있으면 topology가 흐려지므로 "정확히 2 jets" 유지
  if (jet_indices.size() != 2) return false;

  SelectedJetViewCollection candidate_jets =
      SelectJetViews(AllJetViews, jet_indices, jesVar, jerVar);

  // 3) Tag definition: soft muon in jet (WCharm-style, non-iso)
  std::vector<size_t> soft_muon_indices = SelectMuonIndices(
      AllMuonViews, MuonView::MuonID::POG_TIGHT, 5.f, Muon_Tight_Eta);
  std::vector<size_t> soft_muon_indices_iso =
      SelectMuonIndices(AllMuonViews, soft_muon_indices,
                        MuonView::MuonID::POG_PFISO_LOOSE, 5.f, Muon_Tight_Eta);

  std::vector<size_t> soft_muon_indices_noniso;
  soft_muon_indices_noniso.reserve(soft_muon_indices.size());
  for (auto idx : soft_muon_indices) {
    if (std::find(soft_muon_indices_iso.begin(), soft_muon_indices_iso.end(),
                  idx) == soft_muon_indices_iso.end())
      soft_muon_indices_noniso.push_back(idx);
  }

  auto deltaR2 = [](float eta1, float phi1, float eta2, float phi2) {
    const float deta = eta1 - eta2;
    float dphi = phi1 - phi2;
    while (dphi > M_PI)
      dphi -= 2.f * M_PI;
    while (dphi < -M_PI)
      dphi += 2.f * M_PI;
    return deta * deta + dphi * dphi;
  };

  constexpr float dR2_cut = 0.4f * 0.4f;
  constexpr size_t kAmbiguous = std::numeric_limits<size_t>::max();

  std::unordered_map<size_t, size_t> jet_to_mu;
  jet_to_mu.reserve(soft_muon_indices_noniso.size());

  for (auto mu_idx : soft_muon_indices_noniso) {
    const float mu_eta = AllMuonViews[mu_idx].Eta();
    const float mu_phi = AllMuonViews[mu_idx].Phi();

    float best_dR2 = dR2_cut;
    size_t best_jet_idx = std::numeric_limits<size_t>::max();

    for (size_t jet_idx = 0; jet_idx < candidate_jets.size(); ++jet_idx) {
      const float jet_eta = candidate_jets[jet_idx].Eta();
      const float jet_phi = candidate_jets[jet_idx].Phi();
      const float dR2 = deltaR2(jet_eta, jet_phi, mu_eta, mu_phi);
      if (dR2 < best_dR2) {
        best_dR2 = dR2;
        best_jet_idx = jet_idx;
      }
    }

    if (best_jet_idx != std::numeric_limits<size_t>::max()) {
      auto it = jet_to_mu.find(best_jet_idx);
      if (it == jet_to_mu.end())
        jet_to_mu.emplace(best_jet_idx, mu_idx);
      else
        it->second = kAmbiguous; // multiple muons on this jet
    }
  }

  auto pass_wcharm_softmu_jet_quality = [&](const SelectedJetView &jet) -> bool {
    const float muEF = jet.muEF();
    const float neEmEF = jet.neEmEF();
    const float muEF_plus_neEmEF = muEF + neEmEF;
    if (muEF > 0.5f)
      return false;
    if (muEF_plus_neEmEF > 0.7f)
      return false;
    return true;
  };

  std::vector<size_t> tag_candidates;
  tag_candidates.reserve(candidate_jets.size());
  for (const auto &entry : jet_to_mu) {
    if (entry.second == kAmbiguous)
      continue;
    if (!pass_wcharm_softmu_jet_quality(candidate_jets[entry.first]))
      continue;
    tag_candidates.push_back(entry.first);
  }
  if (tag_candidates.empty())
    return false;

  // 4) Choose tag/probe deterministically (random if both pass)
  auto splitmix64 = [](uint64_t x) {
    uint64_t z = (x += 0x9e3779b97f4a7c15ULL);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
  };

  size_t tag_idx  = SIZE_MAX;
  size_t probe_idx = SIZE_MAX;

  if (tag_candidates.size() > 1) {
    uint64_t seed =
        (static_cast<uint64_t>(RunNumber)        * 0x9E3779B97F4A7C15ULL) ^
        (static_cast<uint64_t>(luminosityBlock)  * 0xBF58476D1CE4E5B9ULL) ^
        (static_cast<uint64_t>(event)            * 0x94D049BB133111EBULL);

    tag_idx = (splitmix64(seed) & 1ULL) ? 1 : 0;
    probe_idx = 1 - tag_idx;
  } else {
    tag_idx = tag_candidates[0];
    probe_idx = 1 - tag_idx;
  }

  const SelectedJetView TagJet = candidate_jets[tag_idx];
  const SelectedJetView ProbeJet = candidate_jets[probe_idx];

  // 5) SOTA-ish g->cc topology (DY + g(->cc))
  // Idea:
  //   - two jets close: g splitting -> small ΔR(j,j)
  //   - Z recoils against dijet system: Δφ(Z, jj) ~ pi and pT balance
  //   - splitting tends to give low m_jj, and not-too-crazy pT asymmetry
  Particle JJ(TagJet.P4() + ProbeJet.P4());

  const float dR_jj = TagJet.DeltaR(ProbeJet);
  const float dphi_Z_JJ = ZCand.DeltaPhi(JJ);
  const float ptbal = ZCand.Pt() / JJ.Pt();
  const float asym  = std::fabs(TagJet.Pt() - ProbeJet.Pt()) / (TagJet.Pt() + ProbeJet.Pt());
  const float mjj   = JJ.M();

  // ---- tuneable working points (start point) ----
  // ΔR: splitting enrichment (too tight이면 stat 급감, too loose면 QCD-like)
  if (dR_jj > 1.2f) return false;

  // Z-JJ recoil: system-level back-to-back
  if (dphi_Z_JJ < 2.7f) return false;

  // pT balance: Z and dijet system are balanced
  if (ptbal < 0.8f || ptbal > 1.2f) return false;

  // splitting-like: low dijet mass + moderate asymmetry
  if (mjj > 70.f) return false;
  if (asym > 0.35f) return false;

  // (optional) avoid ultra-soft probe in splitting tail
  // if (ProbeJet.Pt() < 30.f) return false;

  // 6) Cache kinematics for histograms (keep tag/probe)
  dycharm_has_tagprobe = true;
  dycharm_tag_pt = TagJet.Pt();
  dycharm_tag_eta = TagJet.Eta();
  dycharm_tag_phi = TagJet.Phi();
  dycharm_tag_hadronFlavour = TagJet.hadronFlavour();
  dycharm_probe_pt = ProbeJet.Pt();
  dycharm_probe_eta = ProbeJet.Eta();
  dycharm_probe_phi = ProbeJet.Phi();
  dycharm_probe_hadronFlavour = ProbeJet.hadronFlavour();
  dycharm_z_pt = ZCand.Pt();
  dycharm_z_mass = ZCand.M();
  dycharm_jj_pt = JJ.Pt();
  dycharm_jj_mass = JJ.M();
  dycharm_dR_jj = dR_jj;
  dycharm_dphi_z_jj = dphi_Z_JJ;
  dycharm_ptbal_z_jj = ptbal;
  dycharm_ptasym_jj = asym;

  // 7) Store: PROBE jet only for tree
  Jets = candidate_jets.selectPositions({probe_idx});

  // (optional) Muons도 이미 저장하고 있으니 그대로
  return true;
}

bool CalibrationTree::PassQCDCharmDijetSelection() {
  // ============================================================
  // 0) Event cleaning + trigger
  // ============================================================
  //if (!ev.PassTrigger(Jet_Trigger[DataEra.Data()]))
  //  return false;

  if (!PassJetVetoMap(AllJetViews))
    return false;
  if (!PassMetFilter(AllJetViews, ev))
    return false;

  // (optional) leptonic contamination veto: W/Z+jets 줄이기
  {
    BuildLeptonSelectionCache();
    if (!leptonSelectionCache.looseMuonIndices.empty() ||
        !leptonSelectionCache.looseElectronIndices.empty())
      return false;
  }

  // ============================================================
  // 1) Jets with JES/JER (if you keep this infra)
  // ============================================================
  MyCorrection::variation jesVar = MyCorrection::variation::nom;
  MyCorrection::variation jerVar = MyCorrection::variation::nom;

  // MET propagation is not strictly needed for dijet, but keep consistent infra
  const std::string systTarget = systHelper->getCurrentIterSysTarget();
  const TString systSource = systHelper->getCurrentIterSysSource();
  const MyCorrection::variation systVar = systHelper->getCurrentIterVariation();

  MET = ev.GetMETVector(Event::MET_Type::PUPPI);
  bool doJetPropagation = true;
  if (systTarget.find("Jet_En") != std::string::npos) {
    const bool doBreakdown = HasFlag("doBreakdown");
    if (!PrepareJetJESVariations(AllJetViews, systSource, doBreakdown))
      return false;
    if (!IsDATA)
      jesVar = systVar;
  } else if (systTarget == "Jet_Res") {
    if (!IsDATA)
      jerVar = systVar;
  } else if (systTarget == "UE") {
    MET = ev.GetMETVector(Event::MET_Type::PUPPI, systVar,
                          Event::MET_Syst::UE);
    doJetPropagation = false;
  }

  if (doJetPropagation)
    PropagateJetSystToMET(AllJetViews, MET, jesVar, jerVar);

  // --- tuneable working points ---
  constexpr float jet_pt_min = 50.f;
  constexpr float jet_eta_max = 2.4f;    // tracker coverage (flavor tagging)
  constexpr float jet_veto_dr_lep = 0.4f;

  // For 3rd-jet veto / alpha cut: include softer jets
  constexpr float j3_pt_min = 15.f;
  constexpr float j3_eta_max = 2.5f;

  // Dijet topology
  constexpr float dphi_min = 2.8f;       // back-to-back
  constexpr float alpha_max = 0.15f;     // pT3 / pTavg
  constexpr float ystar_max = 1.0f;      // |y1-y2|/2
  constexpr float asym_max = 0.15f;      // |pT1-pT2|/(pT1+pT2)

  // Build jet indices for leading pair
  std::vector<size_t> jet_indices =
      SelectJetIndices(AllJetViews, Jet_ID, jet_pt_min, jet_eta_max, jesVar, jerVar);

  std::vector<size_t> dummy_tight_mu, dummy_tight_el;
  jet_indices = JetsVetoLeptonInside(AllJetViews, jet_indices,
                                     AllElectronViews, dummy_tight_el,
                                     AllMuonViews, dummy_tight_mu,
                                     jet_veto_dr_lep);

  if (jet_indices.size() < 2)
    return false;

  SelectedJetViewCollection jets =
      SelectJetViews(AllJetViews, jet_indices, jesVar, jerVar);

  const SelectedJetView j1 = jets[0];
  const SelectedJetView j2 = jets[1];

  // ============================================================
  // 2) 2->2 enforcing topology cuts (SOTA-ish)
  // ============================================================
  // back-to-back
  if (j1.DeltaPhi(j2) < dphi_min)
    return false;

  // y* cut (rapidity boost)
  // NOTE: use Rapidity() (preferred) if available; else Eta() fallback.
  auto jet_y = [](const SelectedJetView &j) -> float {
    return j.Eta();         // fallback
  };
  const float y1 = jet_y(j1);
  const float y2 = jet_y(j2);
  const float ystar = 0.5f * std::fabs(y1 - y2);
  if (ystar > ystar_max)
    return false;

  // pT asymmetry
  const float asym = std::fabs(j1.Pt() - j2.Pt()) / (j1.Pt() + j2.Pt());
  if (asym > asym_max)
    return false;

  // 3rd-jet veto via alpha = pT3 / pTavg
  // Build a looser jet list including softer jets to find j3
  {
    std::vector<size_t> jets_for_j3 =
        SelectJetIndices(AllJetViews, Jet_ID, j3_pt_min, j3_eta_max, jesVar, jerVar);

    // remove the leading two jets from consideration by ΔR matching (robust to index mismatch)
    SelectedJetViewCollection alljets =
        SelectJetViews(AllJetViews, jets_for_j3, jesVar, jerVar);

    float pt3 = 0.f;
    for (const auto &j : alljets) {
      const bool is_j1 = (j.DeltaR(j1) < 1e-3);
      const bool is_j2 = (j.DeltaR(j2) < 1e-3);
      if (is_j1 || is_j2) continue;
      if (j.Pt() > pt3) pt3 = j.Pt();
    }

    const float ptavg = 0.5f * (j1.Pt() + j2.Pt());
    const float alpha = (ptavg > 0.f) ? (pt3 / ptavg) : 999.f;
    if (alpha > alpha_max)
      return false;
  }

  // ============================================================
  // 3) Tag definition: soft muon in jet (WCharm-style, non-iso)
  // ============================================================
  std::vector<size_t> soft_muon_indices = SelectMuonIndices(
      AllMuonViews, MuonView::MuonID::POG_TIGHT, 5.f, Muon_Tight_Eta);
  std::vector<size_t> soft_muon_indices_iso =
      SelectMuonIndices(AllMuonViews, soft_muon_indices,
                        MuonView::MuonID::POG_PFISO_LOOSE, 5.f, Muon_Tight_Eta);

  std::vector<size_t> soft_muon_indices_noniso;
  soft_muon_indices_noniso.reserve(soft_muon_indices.size());
  for (auto idx : soft_muon_indices) {
    if (std::find(soft_muon_indices_iso.begin(), soft_muon_indices_iso.end(),
                  idx) == soft_muon_indices_iso.end())
      soft_muon_indices_noniso.push_back(idx);
  }

  auto deltaR2 = [](float eta1, float phi1, float eta2, float phi2) {
    const float deta = eta1 - eta2;
    float dphi = phi1 - phi2;
    while (dphi > M_PI)
      dphi -= 2.f * M_PI;
    while (dphi < -M_PI)
      dphi += 2.f * M_PI;
    return deta * deta + dphi * dphi;
  };

  constexpr float dR2_cut = 0.4f * 0.4f;
  constexpr size_t kAmbiguous = std::numeric_limits<size_t>::max();

  std::unordered_map<size_t, size_t> jet_to_mu;
  jet_to_mu.reserve(soft_muon_indices_noniso.size());

  for (auto mu_idx : soft_muon_indices_noniso) {
    const float mu_eta = AllMuonViews[mu_idx].Eta();
    const float mu_phi = AllMuonViews[mu_idx].Phi();

    float best_dR2 = dR2_cut;
    size_t best_jet_idx = std::numeric_limits<size_t>::max();

    for (size_t jet_idx = 0; jet_idx < 2; ++jet_idx) {
      const float jet_eta = jets[jet_idx].Eta();
      const float jet_phi = jets[jet_idx].Phi();
      const float dR2 = deltaR2(jet_eta, jet_phi, mu_eta, mu_phi);
      if (dR2 < best_dR2) {
        best_dR2 = dR2;
        best_jet_idx = jet_idx;
      }
    }

    if (best_jet_idx != std::numeric_limits<size_t>::max()) {
      auto it = jet_to_mu.find(best_jet_idx);
      if (it == jet_to_mu.end())
        jet_to_mu.emplace(best_jet_idx, mu_idx);
      else
        it->second = kAmbiguous; // multiple muons on this jet
    }
  }

  auto pass_wcharm_softmu_jet_quality = [&](const SelectedJetView &jet) -> bool {
    const float muEF = jet.muEF();
    const float neEmEF = jet.neEmEF();
    const float muEF_plus_neEmEF = muEF + neEmEF;
    if (muEF > 0.5f)
      return false;
    if (muEF_plus_neEmEF > 0.7f)
      return false;
    return true;
  };

  std::vector<size_t> tag_candidates;
  tag_candidates.reserve(jets.size());
  for (const auto &entry : jet_to_mu) {
    if (entry.second == kAmbiguous)
      continue;
    if (!pass_wcharm_softmu_jet_quality(jets[entry.first]))
      continue;
    tag_candidates.push_back(entry.first);
  }
  if (tag_candidates.empty())
    return false;

  // Choose tag/probe: if both pass, randomize deterministically to avoid bias
  auto splitmix64 = [](uint64_t x) {
    uint64_t z = (x += 0x9e3779b97f4a7c15ULL);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
  };

  size_t tag_idx = SIZE_MAX;
  size_t probe_idx = SIZE_MAX;

  if (tag_candidates.size() > 1) {
    uint64_t seed =
        (static_cast<uint64_t>(RunNumber)       * 0x9E3779B97F4A7C15ULL) ^
        (static_cast<uint64_t>(luminosityBlock) * 0xBF58476D1CE4E5B9ULL) ^
        (static_cast<uint64_t>(event)           * 0x94D049BB133111EBULL);
    tag_idx = (splitmix64(seed) & 1ULL) ? 1 : 0;
    probe_idx = 1 - tag_idx;
  } else {
    tag_idx = tag_candidates[0];
    probe_idx = 1 - tag_idx;
  }

  const SelectedJetView TagJet = jets[tag_idx];
  const SelectedJetView ProbeJet = jets[probe_idx];

  // ============================================================
  // 4) Cache kinematics for histograms (keep tag/probe)
  // ============================================================
  qcdcharm_has_tagprobe = true;
  qcdcharm_tag_pt = TagJet.Pt();
  qcdcharm_tag_eta = TagJet.Eta();
  qcdcharm_tag_phi = TagJet.Phi();
  qcdcharm_tag_hadronFlavour = TagJet.hadronFlavour();
  qcdcharm_probe_pt = ProbeJet.Pt();
  qcdcharm_probe_eta = ProbeJet.Eta();
  qcdcharm_probe_phi = ProbeJet.Phi();
  qcdcharm_probe_hadronFlavour = ProbeJet.hadronFlavour();
  const Particle JJ(TagJet.P4() + ProbeJet.P4());
  qcdcharm_jj_pt = JJ.Pt();
  qcdcharm_jj_mass = JJ.M();
  qcdcharm_dR_jj = TagJet.DeltaR(ProbeJet);
  qcdcharm_dphi_jj = TagJet.DeltaPhi(ProbeJet);
  qcdcharm_ptasym_jj =
      std::fabs(TagJet.Pt() - ProbeJet.Pt()) / (TagJet.Pt() + ProbeJet.Pt());

  // ============================================================
  // 5) Store PROBE jet only (tree requirement)
  // ============================================================
  Jets = jets.selectPositions({probe_idx});

  // (optional) keep tag jet for closure/purity studies
  // TagJets.clear();
  // TagJets.push_back(TagJet);

  return true;
}


bool CalibrationTree::PassTTSemilepBaselineSelection() {
  // --------------------------------------------------------------------------
  // Nominal configuration for the ttbar semileptonic hadronic-W jet anchor.
  //
  // Purpose:
  //   Select two jets assigned to the hadronic W in semileptonic ttbar events.
  //   These jets are used as a mixed-flavour W-jet anchor for b/c/light
  //   mixture deconvolution, not as a pure charm control region.
  //
  // Important:
  //   Do not apply tagger-score cuts to the selected W jets here, otherwise
  //   the CvsB / BvsL score distributions will be sculpted.
  //
  // This version:
  //   - does NOT run the constrained kinematic fitter;
  //   - uses only cheap prefit topology variables;
  //   - scans all 4-jet ttbar semileptonic assignments;
  //   - chooses the W-pair without using tagger scores;
  //   - requires b-tag only on the two topology-assigned b-side jets;
  //   - keeps only the selected two W jets in Jets.
  // --------------------------------------------------------------------------

  constexpr float kJetPtMin = 25.0f;
  constexpr float kJetEtaMax = 2.5f;
  constexpr float kLeptonJetDR = 0.4f;

  constexpr bool kRequireExactly4Jets = true;

  // Loose prefit W mass window on the two jets assigned to hadronic W.
  constexpr bool  kApplyPrefitWMassWindow = true;
  constexpr float kMinPrefitWMass = 50.0f;
  constexpr float kMaxPrefitWMass = 110.0f;

  // Cheap topology-assignment score cut.
  // Keep disabled for nominal unless data/MC closure is validated.
  constexpr bool  kApplyTopologyScoreCut = false;
  constexpr float kMaxTopologyScore = 30.0f;

  // Ambiguity cut on distinct W-pair hypotheses.
  // Keep disabled for nominal.
  constexpr bool  kApplyDeltaChi2Cut = false;
  constexpr float kMinDeltaChi2 = 5.0f;

  // Tag-side b-tag requirement.
  //
  // Important:
  //   This is applied only after the topology assignment.
  //   It is applied only to best_comb[0], best_comb[1].
  //   It is never applied to best_comb[2], best_comb[3].
  //
  // Recommended nominal:
  //   one b-side jet Medium, the other at least Loose.
  constexpr bool kRequireTagSideBTag = true;
  constexpr bool kRequireOneMediumOneLoose = true;

  // Reconstruction targets and effective resolutions.
  // These are ranking-scale parameters, not natural widths.
  constexpr float kRecoWMass = 80.379f;
  constexpr float kRecoTopMass = 172.5f;

  constexpr float kSigmaRecoW = 12.0f;
  constexpr float kSigmaRecoHadTop = 23.0f;
  constexpr float kSigmaRecoLepTop = 30.0f;
  constexpr float kSigmaRecoTopBalance = 35.0f;

  // --------------------------------------------------------------------------
  // Trigger / event cleaning
  // --------------------------------------------------------------------------

  if (!(ev.PassTrigger(Mu_Trigger[DataEra.Data()])))
    return false;

  if (!PassJetVetoMap(AllJetViews))
    return false;

  if (!PassMetFilter(AllJetViews, ev))
    return false;

  // --------------------------------------------------------------------------
  // Lepton selection: exactly one tight muon, no extra loose lepton
  // --------------------------------------------------------------------------

  BuildLeptonSelectionCache();

  const auto &loose_muon_indices =
      leptonSelectionCache.looseMuonIndices;
  const auto &tight_muon_indices =
      leptonSelectionCache.tightMuonIndices;
  const auto &loose_electron_indices =
      leptonSelectionCache.looseElectronIndices;
  const auto &tight_electron_indices =
      leptonSelectionCache.tightElectronIndices;

  if (!(tight_muon_indices.size() == 1 &&
        tight_electron_indices.size() == 0 &&
        loose_muon_indices.size() == 1 &&
        loose_electron_indices.size() == 0))
    return false;

  Muons = SelectMuonViews(AllMuonViews, tight_muon_indices);

  if (Muons.empty())
    return false;

  // --------------------------------------------------------------------------
  // JES/JER/MET systematic handling
  // --------------------------------------------------------------------------

  MyCorrection::variation jesVar = MyCorrection::variation::nom;
  MyCorrection::variation jerVar = MyCorrection::variation::nom;

  const std::string systTarget = systHelper->getCurrentIterSysTarget();
  const TString systSource = systHelper->getCurrentIterSysSource();
  const MyCorrection::variation systVar =
      systHelper->getCurrentIterVariation();

  MET = ev.GetMETVector(Event::MET_Type::PUPPI);
  ttsl_met_pt_before_jet_syst = MET.Pt();

  bool doJetPropagation = true;
  ttsl_met_propagated = 0;

  const bool skipJetMETPropagation =
      HasFlag("TTSemilepNoJetMETPropagation") ||
      HasFlag("NoJetMETPropagation");

  if (systTarget.find("Jet_En") != std::string::npos) {
    const bool doBreakdown = HasFlag("doBreakdown");

    if (!PrepareJetJESVariations(AllJetViews, systSource, doBreakdown))
      return false;

    if (!IsDATA)
      jesVar = systVar;

  } else if (systTarget == "Jet_Res") {
    if (!IsDATA)
      jerVar = systVar;

  } else if (systTarget == "UE") {
    MET = ev.GetMETVector(Event::MET_Type::PUPPI,
                          systVar,
                          Event::MET_Syst::UE);
    doJetPropagation = false;
  }

  if (doJetPropagation && !skipJetMETPropagation) {
    PropagateJetSystToMET(AllJetViews, MET, jesVar, jerVar);
    ttsl_met_propagated = 1;
  }

  ttsl_met_pt_after_jet_syst = MET.Pt();

  // --------------------------------------------------------------------------
  // Jet selection
  // --------------------------------------------------------------------------

  std::vector<size_t> jet_indices =
      SelectJetIndices(AllJetViews, Jet_ID, kJetPtMin, kJetEtaMax,
                       jesVar, jerVar);

  jet_indices =
      JetsVetoLeptonInside(AllJetViews,
                           jet_indices,
                           AllElectronViews,
                           tight_electron_indices,
                           AllMuonViews,
                           tight_muon_indices,
                           kLeptonJetDR);

  if (kRequireExactly4Jets) {
    if (jet_indices.size() != 4)
      return false;
  } else {
    if (jet_indices.size() < 4)
      return false;
  }

  Jets = SelectJetViews(AllJetViews, jet_indices, jesVar, jerVar);

  if (Jets.size() < 4)
    return false;

  // This function is designed for exactly-4-jet topology.
  // If kRequireExactly4Jets is false, only the leading four selected jets
  // are used by the assignment table below.
  if (kRequireExactly4Jets && Jets.size() != 4)
    return false;

  // --------------------------------------------------------------------------
  // Assignment table.
  //
  // comb[0] : hadronic-b candidate
  // comb[1] : leptonic-b candidate
  // comb[2], comb[3] : hadronic-W candidates
  //
  // No tagger score is used in this assignment.
  // --------------------------------------------------------------------------

  constexpr std::array<std::array<std::size_t, 4>, 12> comb_lut = {
      {{0, 1, 2, 3},
       {0, 2, 1, 3},
       {0, 3, 1, 2},
       {1, 0, 2, 3},
       {1, 2, 0, 3},
       {1, 3, 0, 2},
       {2, 0, 1, 3},
       {2, 1, 0, 3},
       {2, 3, 0, 1},
       {3, 0, 1, 2},
       {3, 1, 0, 2},
       {3, 2, 0, 1}}};

  // --------------------------------------------------------------------------
  // Neutrino pz solutions.
  //
  // Cheap analytic reconstruction only. No constrained fitter.
  // --------------------------------------------------------------------------

  lepton = MakeLeptonSnapshot(Muons[0]);
  std::variant<float, std::pair<float, float>> neutrinoPz =
      SolveNeutrinoPz(lepton, MET);

  std::vector<Particle> neutrino_solutions;
  neutrino_solutions.reserve(2);

  if (std::holds_alternative<float>(neutrinoPz)) {
    const float pz = std::get<float>(neutrinoPz);

    Particle neutrino_p4;
    neutrino_p4.SetPxPyPzE(MET.Px(),
                           MET.Py(),
                           pz,
                           std::sqrt(MET.Pt() * MET.Pt() + pz * pz));

    neutrino_solutions.push_back(neutrino_p4);

  } else {
    const auto pz_pair = std::get<std::pair<float, float>>(neutrinoPz);

    Particle neutrino_p4_1;
    neutrino_p4_1.SetPxPyPzE(
        MET.Px(),
        MET.Py(),
        pz_pair.first,
        std::sqrt(MET.Pt() * MET.Pt() + pz_pair.first * pz_pair.first));

    Particle neutrino_p4_2;
    neutrino_p4_2.SetPxPyPzE(
        MET.Px(),
        MET.Py(),
        pz_pair.second,
        std::sqrt(MET.Pt() * MET.Pt() + pz_pair.second * pz_pair.second));

    neutrino_solutions.push_back(neutrino_p4_1);
    neutrino_solutions.push_back(neutrino_p4_2);
  }

  if (neutrino_solutions.empty())
    return false;

  // --------------------------------------------------------------------------
  // Cheap topology-based assignment.
  //
  // Score:
  //
  //   score =
  //     ((m_jj        - mW) / sigmaW)^2
  //   + ((m_bjj       - mt) / sigmaHadTop)^2
  //   + ((m_blnu      - mt) / sigmaLepTop)^2
  //   + 0.5 * ((m_bjj - m_blnu) / sigmaTopBalance)^2
  //
  // This score is used to rank W-pair hypotheses.
  // It is not a constrained-fitter chi2.
  // --------------------------------------------------------------------------

  best_KF_result.clear();

  ttsl_n_prefit_candidates = 0;
  ttsl_n_fit_candidates = 0;  // no fitter is run
  ttsl_n_valid_combs = 0;

  constexpr std::size_t nComb = comb_lut.size();
  constexpr std::size_t kMaxNeutrinoSolutions = 2;

  const auto Pull2 = [](const float value,
                        const float center,
                        const float sigma) -> float {
    const float pull = (value - center) / sigma;
    return pull * pull;
  };

  struct TopologyCandidate {
    std::size_t comb_idx = 0;
    std::size_t nu_idx = 0;

    float score = std::numeric_limits<float>::max();

    float mW = -1.0f;
    float mTopHad = -1.0f;
    float mTopLep = -1.0f;

    float abs_mW_residual = std::numeric_limits<float>::max();
    float abs_top_balance = std::numeric_limits<float>::max();
  };

  const auto CandidateLess =
      [](const TopologyCandidate &a,
         const TopologyCandidate &b) -> bool {
        if (a.score != b.score)
          return a.score < b.score;

        if (a.abs_mW_residual != b.abs_mW_residual)
          return a.abs_mW_residual < b.abs_mW_residual;

        if (a.abs_top_balance != b.abs_top_balance)
          return a.abs_top_balance < b.abs_top_balance;

        if (a.comb_idx != b.comb_idx)
          return a.comb_idx < b.comb_idx;

        return a.nu_idx < b.nu_idx;
      };

  std::vector<TopologyCandidate> best_candidates_by_assignment;
  best_candidates_by_assignment.reserve(nComb);

  for (std::size_t comb_idx = 0; comb_idx < nComb; ++comb_idx) {
    const auto &comb = comb_lut[comb_idx];

    const SelectedJetView bHadCand = Jets[comb[0]];
    const SelectedJetView bLepCand = Jets[comb[1]];
    const SelectedJetView W1cand = Jets[comb[2]];
    const SelectedJetView W2cand = Jets[comb[3]];

    const float mW_prefit = (W1cand.P4() + W2cand.P4()).M();

    if (!std::isfinite(mW_prefit))
      continue;

    if (kApplyPrefitWMassWindow) {
      if (mW_prefit < kMinPrefitWMass ||
          mW_prefit > kMaxPrefitWMass)
        continue;
    }

    bool has_valid_candidate = false;
    TopologyCandidate best_for_assignment;

    const std::size_t nNuSolutions =
        std::min(neutrino_solutions.size(), kMaxNeutrinoSolutions);

    for (std::size_t nu_idx = 0; nu_idx < nNuSolutions; ++nu_idx) {
      const Particle &neutrino_sol = neutrino_solutions[nu_idx];

      const float mTopHad =
          (bHadCand.P4() + W1cand.P4() + W2cand.P4()).M();

      const float mTopLep =
          (bLepCand.P4() + Muons[0].P4() + neutrino_sol).M();

      if (!std::isfinite(mTopHad) || !std::isfinite(mTopLep))
        continue;

      if (mTopHad <= 0.0f || mTopLep <= 0.0f)
        continue;

      const float chi2W =
          Pull2(mW_prefit, kRecoWMass, kSigmaRecoW);

      const float chi2HadTop =
          Pull2(mTopHad, kRecoTopMass, kSigmaRecoHadTop);

      const float chi2LepTop =
          Pull2(mTopLep, kRecoTopMass, kSigmaRecoLepTop);

      const float chi2TopBalance =
          Pull2(mTopHad - mTopLep, 0.0f, kSigmaRecoTopBalance);

      const float topology_score =
          chi2W + chi2HadTop + chi2LepTop + 0.5f * chi2TopBalance;

      if (!std::isfinite(topology_score) || topology_score < 0.0f)
        continue;

      ++ttsl_n_prefit_candidates;

      TopologyCandidate cand;
      cand.comb_idx = comb_idx;
      cand.nu_idx = nu_idx;
      cand.score = topology_score;
      cand.mW = mW_prefit;
      cand.mTopHad = mTopHad;
      cand.mTopLep = mTopLep;
      cand.abs_mW_residual = std::fabs(mW_prefit - kRecoWMass);
      cand.abs_top_balance = std::fabs(mTopHad - mTopLep);

      if (!has_valid_candidate || CandidateLess(cand, best_for_assignment)) {
        best_for_assignment = cand;
        has_valid_candidate = true;
      }
    }

    if (!has_valid_candidate)
      continue;

    best_candidates_by_assignment.push_back(best_for_assignment);
    ++ttsl_n_valid_combs;
  }

  if (best_candidates_by_assignment.empty())
    return false;

  std::sort(best_candidates_by_assignment.begin(),
            best_candidates_by_assignment.end(),
            CandidateLess);

  const TopologyCandidate best_candidate =
      best_candidates_by_assignment.front();

  // --------------------------------------------------------------------------
  // Find second-best distinct W-pair hypothesis.
  //
  // A bHad/bLep swap with the same W pair is not counted as W-pair ambiguity.
  // --------------------------------------------------------------------------

  const auto SameWPair =
      [&comb_lut](const TopologyCandidate &a,
                  const TopologyCandidate &b) -> bool {
        const auto &comb_a = comb_lut[a.comb_idx];
        const auto &comb_b = comb_lut[b.comb_idx];

        const bool same_order =
            comb_a[2] == comb_b[2] && comb_a[3] == comb_b[3];

        const bool swapped_order =
            comb_a[2] == comb_b[3] && comb_a[3] == comb_b[2];

        return same_order || swapped_order;
      };

  bool has_second_best = false;
  TopologyCandidate second_best_candidate;

  for (std::size_t idx = 1; idx < best_candidates_by_assignment.size(); ++idx) {
    const TopologyCandidate &cand = best_candidates_by_assignment[idx];

    if (SameWPair(best_candidate, cand))
      continue;

    second_best_candidate = cand;
    has_second_best = true;
    break;
  }

  if (kApplyTopologyScoreCut) {
    if (best_candidate.score > kMaxTopologyScore)
      return false;
  }

  const float logTopologyScore =
      std::log(best_candidate.score + 1e-6f);

  const float deltaChi2 =
      has_second_best
          ? (second_best_candidate.score - best_candidate.score)
          : std::numeric_limits<float>::max();

  if (kApplyDeltaChi2Cut) {
    if (!has_second_best)
      return false;

    if (deltaChi2 < kMinDeltaChi2)
      return false;
  }

  // --------------------------------------------------------------------------
  // Best topology assignment.
  //
  // Do not use b-tagging to choose this assignment.
  // B-tagging is applied only after this point, only to the b-side candidates.
  // --------------------------------------------------------------------------

  const auto &best_comb = comb_lut[best_candidate.comb_idx];

  const SelectedJetView bHad = Jets[best_comb[0]];
  const SelectedJetView bLep = Jets[best_comb[1]];

  const SelectedJetView W1 = Jets[best_comb[2]];
  const SelectedJetView W2 = Jets[best_comb[3]];

  // --------------------------------------------------------------------------
  // Tag-side b-tag requirement.
  //
  // This reduces W+jets / QCD background while avoiding a direct tagger-score
  // cut on the stored W jets.
  //
  // Important:
  //   If the b-side fails, reject the event.
  //   Do NOT fall back to another assignment that passes b-tagging, because that
  //   would make the W-pair choice tagger-dependent.
  // --------------------------------------------------------------------------

  if (kRequireTagSideBTag) {
    const auto btagger = FlavTagger.at(DataEra.Data());
    const float loose_wp =
        GetBTaggingWP(btagger, JetTagging::JetFlavTaggerWP::Loose);
    const float medium_wp =
        GetBTaggingWP(btagger, JetTagging::JetFlavTaggerWP::Medium);

    const float bHad_bscore =
        bHad.GetTaggerResult(btagger, JetTagging::JetFlavTaggerScoreType::B);
    const bool bHad_loose = bHad_bscore > loose_wp;
    const bool bHad_medium = bHad_bscore > medium_wp;

    const float bLep_bscore =
        bLep.GetTaggerResult(btagger, JetTagging::JetFlavTaggerScoreType::B);
    const bool bLep_loose = bLep_bscore > loose_wp;
    const bool bLep_medium = bLep_bscore > medium_wp;

    const int n_bside_loose =
        static_cast<int>(bHad_loose) + static_cast<int>(bLep_loose);

    const int n_bside_medium =
        static_cast<int>(bHad_medium) + static_cast<int>(bLep_medium);

    // Optional diagnostics if corresponding branches exist:
    //
    // ttsl_bside_n_loose = n_bside_loose;
    // ttsl_bside_n_medium = n_bside_medium;
    //
    // For monitoring only. Do not cut on W-side b-tag multiplicity:
    //
    // ttsl_wside_n_loose =
    //     static_cast<int>(W1.GetTaggerResult(
    //         btagger, JetTagging::JetFlavTaggerScoreType::B) > loose_wp) +
    //     static_cast<int>(W2.GetTaggerResult(
    //         btagger, JetTagging::JetFlavTaggerScoreType::B) > loose_wp);
    //
    // ttsl_wside_n_medium =
    //     static_cast<int>(W1.GetTaggerResult(
    //         btagger, JetTagging::JetFlavTaggerScoreType::B) > medium_wp) +
    //     static_cast<int>(W2.GetTaggerResult(
    //         btagger, JetTagging::JetFlavTaggerScoreType::B) > medium_wp);

    if (kRequireOneMediumOneLoose) {
      const bool passOneMediumOneLoose =
          (bHad_medium && bLep_loose) ||
          (bLep_medium && bHad_loose);

      if (!passOneMediumOneLoose)
        return false;

    } else {
      if (n_bside_medium < 1)
        return false;
    }
  }

  // --------------------------------------------------------------------------
  // Fill diagnostic branches.
  //
  // Legacy names are preserved for compatibility, but note:
  //   ttsl_fit_chi2 now stores topology_score, not fitter chi2.
  // --------------------------------------------------------------------------

  log_chi2 = logTopologyScore;

  ttsl_fit_status = 0;
  ttsl_fit_chi2 = best_candidate.score;
  ttsl_log_fit_chi2 = logTopologyScore;

  ttsl_prefit_chi2 = best_candidate.score;
  ttsl_prefit_mw = best_candidate.mW;

  ttsl_best_comb_idx = static_cast<int>(best_candidate.comb_idx);
  ttsl_second_comb_idx =
      has_second_best ? static_cast<int>(second_best_candidate.comb_idx) : -1;

  ttsl_best_nu_idx = static_cast<int>(best_candidate.nu_idx);

  ttsl_second_fit_chi2 =
      has_second_best ? second_best_candidate.score : -9999.0f;

  ttsl_delta_chi2 =
      has_second_best ? deltaChi2 : -9999.0f;

  // --------------------------------------------------------------------------
  // Keep only the two hadronic-W assigned jets in the calibration tree.
  // No tagger requirement has been applied to these two jets.
  // --------------------------------------------------------------------------

  Jets = Jets.selectPositions({best_comb[2], best_comb[3]});

  return true;
}
