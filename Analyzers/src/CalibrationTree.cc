#include "CalibrationTree.h"
#include "BranchManager.h"
#include "GenView.h"
#include "Jet.h"
#include "JetTaggingParameter.h"
#include "Muon.h"
#include "MyCorrection.h"
#include "Particle.h"
#include "TBranch.h"
#include "TEntryList.h"
#include "TEntryListArray.h"
#include "TKinFitter.h"
#include "TObjArray.h"
#include "VcbParameters.h"
#include <RtypesCore.h>
#include <TLorentzVector.h>
#include <algorithm>
#include <cctype>
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

CalibrationTree::KinFitterResult CalibrationTree::Chi2Prefit(const Jet &had_t_b, const Jet &lep_t_b,
                                        const Jet &had_w_1, const Jet &had_w_2,
                                        const Particle &neutrino,
                                        const Lepton &lepton){

  KinFitterResult result;
  result.status = 0;
  double chi2_thad = ((had_t_b + had_w_1 + had_w_2).M() - T_MASS)/T_WIDTH;
  double chi2_tlep = ((lep_t_b + lepton + neutrino).M() - T_MASS)/T_WIDTH;
  double chi2_whad = ((had_w_1 + had_w_2).M() - W_MASS)/W_WIDTH;
  result.chi2_thad = chi2_thad*chi2_thad;
  result.chi2_tlep = chi2_tlep*chi2_tlep;
  result.chi2_whad = chi2_whad*chi2_whad;
  result.chi2_wlep = 0.;
  return result;
}



CalibrationTree::KinFitterResult CalibrationTree::FitKinFitterTTSemilep(
    const Jet &had_t_b, const Jet &lep_t_b, const Jet &had_w_1,
    const Jet &had_w_2, Particle &neutrino, Lepton &lepton) {
  KinFitterResult result;
  result.status = -999;
  result.chi2_thad = 9999.;
  result.chi2_tlep = 9999.;
  result.chi2_whad = 9999.;
  result.chi2_wlep = 9999.;
  // Fitter 설정
  std::unique_ptr<TKinFitter> fitter =
      std::make_unique<TKinFitter>("ttSemilepFitter", "ttSemilepFitter");
  fitter->reset();
  fitter->setVerbosity(0);
  fitter->setMaxNbIter(5000);
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
  px_balance->addParticle(lep.get());
  px_balance->addParticle(neu.get());
  py_balance->addParticle(had_t_b_fit.get());
  py_balance->addParticle(lep_t_b_fit.get());
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
  result.chi2_whad = Calc_Each_Chi2(mHadW.get(), W_MASS, W_WIDTH);
  result.chi2_wlep = Calc_Each_Chi2(mLepW.get(), W_MASS, W_WIDTH);
  result.chi2_thad = Calc_Each_Chi2(mHadT.get(), T_MASS, T_WIDTH);
  result.chi2_tlep = Calc_Each_Chi2(mLepT.get(), T_MASS, T_WIDTH);
  return result;
}

// status, chi2, fitted_b, fitted_lep, fitted_neu
std::tuple<int, double, TLorentzVector, TLorentzVector, TLorentzVector>
CalibrationTree::FitKinFitterLepTop(const Jet &bjet, Particle &neutrino,
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
  } else if (HasFlag("TTSemilep")) {
    channel = Channel::TTSemilep;
  } else {
    throw std::runtime_error(
        "Channel not specified! Please set one of the channel flags.");
  }
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

  fill1d("MET_Pt", MET.Pt(), weight, 100, 0, 200);
  fill1d("MET_Phi", MET.Phi(), weight, 64, -3.2, 3.2);

  for (size_t idx = 0; idx < Jets.size(); ++idx) {
    const Jet &jet = Jets[idx];
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
    fill1d("Jet_" + to_string(idx) + "_hadronFlavour",
           static_cast<float>(hardflav), weight, 7, -1.f, 6.f);
  }
  for (size_t idx = 0; idx < Electrons.size(); ++idx) {
    const Electron &ele = Electrons[idx];
    fill1d("Electron_" + to_string(idx) + "_Pt", ele.Pt(), weight, 100, 0, 500);
    fill1d("Electron_" + to_string(idx) + "_Eta", ele.Eta(), weight, 50, -2.5,
           2.5);
    fill1d("Electron_" + to_string(idx) + "_Phi", ele.Phi(), weight, 64, -3.2,
           3.2);
  }

  for (size_t idx = 0; idx < Muons.size(); ++idx) {
    const Muon &mu = Muons[idx];
    fill1d("Muon_" + to_string(idx) + "_Pt", mu.Pt(), weight, 100, 0, 500);
    fill1d("Muon_" + to_string(idx) + "_Eta", mu.Eta(), weight, 50, -2.5, 2.5);
    fill1d("Muon_" + to_string(idx) + "_Phi", mu.Phi(), weight, 64, -3.2, 3.2);
  }
  if (Muons.size() >= 2) {
    TLorentzVector ZCand = Muons[0] + Muons[1];
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
}

void CalibrationTree::FillTreeAtThisPoint(
    std::string_view treePrefix, float MCNormalizationWeight,
    const std::unordered_map<std::string, float> &weight_map) {

  auto sanitize_branch_name = [](std::string_view raw) -> std::string {
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
    if (sanitized.empty())
      sanitized = "Tree";
    return sanitized;
  };

  const std::string tree_name = sanitize_branch_name(treePrefix);
  NewTree(tree_name, {}, {"*"});
  SetBranch(tree_name, "log_chi2", log_chi2);
  SetBranch(tree_name, "mmuj0", mmuj0);
  SetBranch(tree_name, "mmuj1", mmuj1);
  SetBranch(tree_name, "melj0", melj0);
  SetBranch(tree_name, "melj1", melj1);
  SetBranch(tree_name, "log_chi2_had_t", std::log(best_KF_result.chi2_thad));
  SetBranch(tree_name, "log_chi2_lep_t", std::log(best_KF_result.chi2_tlep));
  SetBranch(tree_name, "log_chi2_had_w", std::log(best_KF_result.chi2_whad));
  SetBranch(tree_name, "log_chi2_lep_w", std::log(best_KF_result.chi2_wlep));
  SetBranch(tree_name, "log_chi2", std::log(best_KF_result.chi2_tlep +
                                         best_KF_result.chi2_thad +
                                         best_KF_result.chi2_whad +
                                         best_KF_result.chi2_wlep));
  for (size_t i = 0; i < Jets.size(); ++i) {
    SetBranch(tree_name, "Jet_" + std::to_string(i) + "_Pt", Jets[i].Pt());
    SetBranch(tree_name, "Jet_" + std::to_string(i) + "_UParT_B",
              Jets[i].GetTaggerResult(JetTagging::JetFlavTagger::ParT,
                                      JetTagging::JetFlavTaggerScoreType::B));
    SetBranch(tree_name, "Jet_" + std::to_string(i) + "_UParT_CvB",
              Jets[i].GetTaggerResult(JetTagging::JetFlavTagger::ParT,
                                      JetTagging::JetFlavTaggerScoreType::CvB));
    SetBranch(tree_name, "Jet_" + std::to_string(i) + "_UParT_CvL",
              Jets[i].GetTaggerResult(JetTagging::JetFlavTagger::ParT,
                                      JetTagging::JetFlavTaggerScoreType::CvL));
    SetBranch(
        tree_name, "Jet_" + std::to_string(i) + "_UParT_CvNotB",
        Jets[i].GetTaggerResult(JetTagging::JetFlavTagger::ParT,
                                JetTagging::JetFlavTaggerScoreType::CvNotB));
    SetBranch(
        tree_name, "Jet_" + std::to_string(i) + "_UParT_SvCB",
        Jets[i].GetTaggerResult(JetTagging::JetFlavTagger::ParT,
                                JetTagging::JetFlavTaggerScoreType::SvCB));
    SetBranch(
        tree_name, "Jet_" + std::to_string(i) + "_UParT_SvUDG",
        Jets[i].GetTaggerResult(JetTagging::JetFlavTagger::ParT,
                                JetTagging::JetFlavTaggerScoreType::SvUDG));
    SetBranch(
        tree_name, "Jet_" + std::to_string(i) + "_UParT_probUDG",
        Jets[i].GetTaggerResult(JetTagging::JetFlavTagger::ParT,
                                JetTagging::JetFlavTaggerScoreType::probUDG));
    SetBranch(
        tree_name, "Jet_" + std::to_string(i) + "_UParT_probB",
        Jets[i].GetTaggerResult(JetTagging::JetFlavTagger::ParT,
                                JetTagging::JetFlavTaggerScoreType::probB));
    SetBranch(
        tree_name, "Jet_" + std::to_string(i) + "_UParT_probBB",
        Jets[i].GetTaggerResult(JetTagging::JetFlavTagger::ParT,
                                JetTagging::JetFlavTaggerScoreType::probBB));
    SetBranch(tree_name, "Jet_" + std::to_string(i) + "_hadronFlavour",
              IsDATA ? -1 : abs(Jets[i].hadronFlavour()));
  }

  if (channel == Channel::WCharm_Mu || channel == Channel::WCharm_El) {
    SetBranch(tree_name, "WCharm_Jet0_nSVs", wcharm_jet0_nsv);
    SetBranch(tree_name, "WCharm_Jet0_SV0_idx", wcharm_sv0_idx);
    SetBranch(tree_name, "WCharm_Jet0_SV1_idx", wcharm_sv1_idx);

    SetBranch(tree_name, "WCharm_Jet0_SV0_Pt", wcharm_sv0_pt);
    SetBranch(tree_name, "WCharm_Jet0_SV0_Eta", wcharm_sv0_eta);
    SetBranch(tree_name, "WCharm_Jet0_SV0_Phi", wcharm_sv0_phi);
    SetBranch(tree_name, "WCharm_Jet0_SV0_Mass", wcharm_sv0_mass);
    SetBranch(tree_name, "WCharm_Jet0_SV0_Charge", wcharm_sv0_charge);
    SetBranch(tree_name, "WCharm_Jet0_SV0_Chi2", wcharm_sv0_chi2);
    SetBranch(tree_name, "WCharm_Jet0_SV0_Ndof", wcharm_sv0_ndof);
    SetBranch(tree_name, "WCharm_Jet0_SV0_NTracks", wcharm_sv0_ntracks);
    SetBranch(tree_name, "WCharm_Jet0_SV0_Dlen", wcharm_sv0_dlen);
    SetBranch(tree_name, "WCharm_Jet0_SV0_DlenSig", wcharm_sv0_dlenSig);
    SetBranch(tree_name, "WCharm_Jet0_SV0_Dxy", wcharm_sv0_dxy);
    SetBranch(tree_name, "WCharm_Jet0_SV0_DxySig", wcharm_sv0_dxySig);
    SetBranch(tree_name, "WCharm_Jet0_SV0_PAngle", wcharm_sv0_pAngle);
    SetBranch(tree_name, "WCharm_Jet0_SV0_X", wcharm_sv0_x);
    SetBranch(tree_name, "WCharm_Jet0_SV0_Y", wcharm_sv0_y);
    SetBranch(tree_name, "WCharm_Jet0_SV0_Z", wcharm_sv0_z);

    SetBranch(tree_name, "WCharm_Jet0_SV1_Pt", wcharm_sv1_pt);
    SetBranch(tree_name, "WCharm_Jet0_SV1_Eta", wcharm_sv1_eta);
    SetBranch(tree_name, "WCharm_Jet0_SV1_Phi", wcharm_sv1_phi);
    SetBranch(tree_name, "WCharm_Jet0_SV1_Mass", wcharm_sv1_mass);
    SetBranch(tree_name, "WCharm_Jet0_SV1_Charge", wcharm_sv1_charge);
    SetBranch(tree_name, "WCharm_Jet0_SV1_Chi2", wcharm_sv1_chi2);
    SetBranch(tree_name, "WCharm_Jet0_SV1_Ndof", wcharm_sv1_ndof);
    SetBranch(tree_name, "WCharm_Jet0_SV1_NTracks", wcharm_sv1_ntracks);
    SetBranch(tree_name, "WCharm_Jet0_SV1_Dlen", wcharm_sv1_dlen);
    SetBranch(tree_name, "WCharm_Jet0_SV1_DlenSig", wcharm_sv1_dlenSig);
    SetBranch(tree_name, "WCharm_Jet0_SV1_Dxy", wcharm_sv1_dxy);
    SetBranch(tree_name, "WCharm_Jet0_SV1_DxySig", wcharm_sv1_dxySig);
    SetBranch(tree_name, "WCharm_Jet0_SV1_PAngle", wcharm_sv1_pAngle);
    SetBranch(tree_name, "WCharm_Jet0_SV1_X", wcharm_sv1_x);
    SetBranch(tree_name, "WCharm_Jet0_SV1_Y", wcharm_sv1_y);
    SetBranch(tree_name, "WCharm_Jet0_SV1_Z", wcharm_sv1_z);
  }

  for (const auto &kv : weight_map) {
    SetBranch(tree_name, "weight_" + kv.first, kv.second);
  }
  SetBranch(tree_name, "MCNormalization", MCNormalizationWeight);
  FillTrees(tree_name);
}

void CalibrationTree::executeEvent() {
  AllMuonViews = GetAllMuonViews();
  AllElectronViews = GetAllElectronViews();
  AllJetViews = GetAllJetViews();
  AllSVViews = GetAllSVViews();
  AllTrigObjViews = GetAllTrigObjViews();

  ev = GetEvent();
  if (HasFlag("Skim")) {
    Clear();
    SkimTree();
    return;
  }
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

  if (!PassJetVetoMap(AllJetViews, AllMuonViews))
    return;
  if (DataEra == "2022EE") {
    const float max_eta = std::numeric_limits<float>::infinity();
    auto eep_veto_indices =
        SelectJetIndices(AllJetViews, Jet::JetID::NOCUT, 30.f, max_eta);
    RVec<Jet> eep_veto_jets;
    eep_veto_jets.reserve(eep_veto_indices.size());
    for (auto idx : eep_veto_indices)
      eep_veto_jets.emplace_back(MaterializeJet(AllJetViews, idx));
    if (!PassJetVetoMap(eep_veto_jets, AllMuonViews, "jetvetomap_eep"))
      return;
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

  auto select_tight_muons = [&](Muon::MuonID id, bool require_iso) {
    std::vector<std::size_t> indices = SelectMuonIndices(
        AllMuonViews, id, Muon_Tight_Pt[DataEra.Data()], Muon_Tight_Eta);
    if (require_iso) {
      indices = SelectMuonIndices(AllMuonViews, indices, Muon_Tight_Iso,
                                  Muon_Tight_Pt[DataEra.Data()],
                                  Muon_Tight_Eta);
    }
    return indices;
  };

  auto select_tight_electrons = [&](Electron::ElectronID id) {
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
      select_tight_muons(Muon::MuonID::POG_TIGHT, true);
  const std::vector<std::size_t> mu_tight_prompt =
      select_tight_muons(Muon::MuonID::POG_PROMPTMVA_WP0p64, false);
  const std::vector<std::size_t> el_tight_wp80 =
      select_tight_electrons(Electron::ElectronID::POG_MVAISO_WP80);
  const std::vector<std::size_t> el_tight_prompt =
      select_tight_electrons(Electron::ElectronID::POG_PROMPTMVA_MEDIUM);

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

  std::function<float(MyCorrection::variation, TString)> BTag_lambda =
      [&](MyCorrection::variation syst, TString source) {
        float weight = 1.f;
        weight *= myCorr->GetBTaggingSF(
            Jets, JetTagging::JetTaggingSFMethod::shape, syst, source);
        weight *= myCorr->GetBTaggingR(Jets, Sample_Shorthand[MCSample.Data()],
                                       syst, source);
        return weight;
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
  Jets.clear();
  MET = Particle();
  log_chi2 = -9999.f;
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
  best_KF_result.clear();
}

void CalibrationTree::executeEventFromParameter() {
  Clear();

  if (!PassBaseLineSelection())
    return;

  SetSystematicLambda();

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
  if (!PassJetVetoMap(AllJetViews, AllMuonViews))
    return false;
  if (!PassMetFilter(AllJetViews, ev))
    return false;
  std::vector<size_t> loose_muon_indices = SelectMuonIndices(
      AllMuonViews, Muon_Veto_ID, Muon_Veto_Pt, Muon_Veto_Eta);
  loose_muon_indices =
      SelectMuonIndices(AllMuonViews, loose_muon_indices, Muon_Veto_Iso,
                        Muon_Veto_Pt, Muon_Veto_Eta);
  std::vector<size_t> tight_muon_indices =
      SelectMuonIndices(AllMuonViews, loose_muon_indices, Muon_Tight_ID,
                        Muon_Tight_Pt[DataEra.Data()], Muon_Tight_Eta);
  tight_muon_indices =
      SelectMuonIndices(AllMuonViews, tight_muon_indices, Muon_Tight_Iso,
                        Muon_Tight_Pt[DataEra.Data()], Muon_Tight_Eta);
  std::vector<size_t> loose_electron_indices = SelectElectronIndices(
      AllElectronViews, Electron_Veto_ID, Electron_Veto_Pt, Electron_Veto_Eta);
  std::vector<size_t> tight_electron_indices = SelectElectronIndices(
      AllElectronViews, loose_electron_indices, Electron_Tight_ID,
      Electron_Tight_Pt[DataEra.Data()], Electron_Tight_Eta);
  if (!(tight_muon_indices.size() == 1 && tight_electron_indices.size() == 1 &&
        loose_muon_indices.size() == 1 && loose_electron_indices.size() == 1))
    return false;
  if ((AllMuonViews[tight_muon_indices[0]].Charge() *
       AllElectronViews[tight_electron_indices[0]].Charge()) > 0)
    return false;
  Muons = MaterializeMuons(AllMuonViews, tight_muon_indices);
  Electrons = MaterializeElectrons(AllElectronViews, tight_electron_indices);

  MET = ev.GetMETVector(Event::MET_Type::PUPPI);
  std::sort(Jets.begin(), Jets.end(), PtComparing);

  TLorentzVector p4_nominal(0, 0, 0, 0);
  TLorentzVector p4_shifted(0, 0, 0, 0);
  for (const auto &jetView : AllJetViews) {
    TLorentzVector v;
    v.SetPtEtaPhiM(jetView.Pt(), jetView.Eta(), jetView.Phi(), jetView.Mass());
    p4_nominal += v;
  }

  if (systHelper->getCurrentIterSysTarget().find("Jet_En") !=
      std::string::npos) {
    const bool doBreakdown = HasFlag("doBreakdown");
    const TString srcT = systHelper->getCurrentIterSysSource();
    if (doBreakdown) {
      if (srcT.EqualTo("total", TString::kIgnoreCase))
        return false;
      ApplyJetScaleVariation(AllJetViews, srcT);
    } else {
      if (!srcT.EqualTo("total", TString::kIgnoreCase))
        return false;
      ApplyJetScaleVariation(AllJetViews, "total");
    }

    if (systHelper->getCurrentIterVariation() == MyCorrection::variation::up) {
      for (const auto &jetView : AllJetViews) {
        TLorentzVector v;
        v.SetPtEtaPhiM(jetView.JesPtUp(), jetView.Eta(), jetView.Phi(),
                       jetView.JesMassUp());
        p4_shifted += v;
      }
    } else if (systHelper->getCurrentIterVariation() ==
               MyCorrection::variation::down) {
      for (const auto &jetView : AllJetViews) {
        TLorentzVector v;
        v.SetPtEtaPhiM(jetView.JesPtDown(), jetView.Eta(), jetView.Phi(),
                       jetView.JesMassDown());
        p4_shifted += v;
      }
    }
  } else if (systHelper->getCurrentIterSysTarget() == "Jet_Res") {
    MET = ev.GetMETVector(Event::MET_Type::PUPPI,
                          systHelper->getCurrentIterVariation(),
                          Event::MET_Syst::JER);
    p4_shifted = p4_nominal;
  } else if (systHelper->getCurrentIterSysTarget() == "UE") {
    MET = ev.GetMETVector(Event::MET_Type::PUPPI,
                          systHelper->getCurrentIterVariation(),
                          Event::MET_Syst::UE);
    p4_shifted = p4_nominal;
  } else {
    for (const auto &jetView : AllJetViews) {
      TLorentzVector v;
      v.SetPtEtaPhiM(jetView.SmearedPtNominal(), jetView.Eta(), jetView.Phi(),
                     jetView.SmearedMassNominal());
      p4_shifted += v;
    }
  }

  {
    TLorentzVector delta = p4_shifted - p4_nominal;
    MET.SetXYZM(MET.Px() - delta.Px(), MET.Py() - delta.Py(), 0., 0.);
  }
  if (MET.Pt() < 40.)
    return false;
  MyCorrection::variation jesVar = MyCorrection::variation::nom;
  MyCorrection::variation jerVar = MyCorrection::variation::nom;
  if (!IsDATA) {
    if (systHelper->getCurrentIterSysTarget().find("Jet_En") !=
        std::string::npos) {
      jesVar = systHelper->getCurrentIterVariation();
    } else if (systHelper->getCurrentIterSysTarget() == "Jet_Res") {
      jerVar = systHelper->getCurrentIterVariation();
    }
  }
  std::vector<size_t> jet_indices = SelectJetIndices(
      AllJetViews, Jet_ID, 25., 2.5, jesVar, jerVar);
  jet_indices = JetsVetoLeptonInside(AllJetViews, jet_indices, AllElectronViews,
                                     tight_electron_indices, AllMuonViews,
                                     tight_muon_indices, 0.4);
  if (jet_indices.size() != 2)
    return false;
  Particle emu = Muons[0] + Electrons[0];
  float mt_emumet = std::sqrt(2.f * emu.Pt() * MET.Pt() *
                              (1.f - std::cos(emu.DeltaPhi(MET))));
  if (mt_emumet > 70.f)
    return false;
  Jets = MaterializeJets(AllJetViews, jet_indices, jesVar, jerVar);
  std::sort(Jets.begin(), Jets.end(), PtComparing);
  size_t jet_0_hadflav = AllJetViews[jet_indices[0]].HadronFlavour();
  size_t jet_1_hadflav = AllJetViews[jet_indices[1]].HadronFlavour();
  std::string jet_0_hadflav_str = (jet_0_hadflav == 5)   ? "b"
                                  : (jet_0_hadflav == 4) ? "c"
                                                         : "light";
  std::string jet_1_hadflav_str = (jet_1_hadflav == 5)   ? "b"
                                  : (jet_1_hadflav == 4) ? "c"
                                                         : "light";

  mmuj0 = (Muons[0] + Jets[0]).M();
  mmuj1 = (Muons[0] + Jets[1]).M();
  melj0 = (Electrons[0] + Jets[0]).M();
  melj1 = (Electrons[0] + Jets[1]).M();

  return true;
}

// bool CalibrationTree::PassTTDilepBaselineSelection() {
//   if(IsDATA && DataStream.Contains("EGamma")) {
//     if (!ev.PassTrigger(El_Trigger[DataEra.Data()]))
//       return false;
//     if (ev.PassTrigger(Mu_Trigger[DataEra.Data()]))
//       return false;
//   }
//   else{
//     if (!(ev.PassTrigger(Mu_Trigger[DataEra.Data()]) ||
//           ev.PassTrigger(El_Trigger[DataEra.Data()])))
//       return false;
//   }
//   if (!PassJetVetoMap(AllJetViews, AllMuonViews))
//     return false;
//   if (!PassMetFilter(AllJetViews, ev))
//     return false;
//   std::vector<size_t> loose_muon_indices = SelectMuonIndices(
//       AllMuonViews, Muon_Veto_ID, Muon_Veto_Pt, Muon_Veto_Eta);
//   loose_muon_indices =
//       SelectMuonIndices(AllMuonViews, loose_muon_indices, Muon_Veto_Iso,
//                         Muon_Veto_Pt, Muon_Veto_Eta);
//   std::vector<size_t> tight_muon_indices =
//       SelectMuonIndices(AllMuonViews, loose_muon_indices, Muon_Tight_ID,
//                         Muon_Tight_Pt[DataEra.Data()], Muon_Tight_Eta);
//   tight_muon_indices =
//       SelectMuonIndices(AllMuonViews, tight_muon_indices, Muon_Tight_Iso,
//                         Muon_Tight_Pt[DataEra.Data()], Muon_Tight_Eta);
//   std::vector<size_t> loose_electron_indices = SelectElectronIndices(
//       AllElectronViews, Electron_Veto_ID, Electron_Veto_Pt,
//       Electron_Veto_Eta);
//   std::vector<size_t> tight_electron_indices = SelectElectronIndices(
//       AllElectronViews, loose_electron_indices, Electron_Tight_ID,
//       Electron_Tight_Pt[DataEra.Data()], Electron_Tight_Eta);
//   if (!(tight_muon_indices.size() == 1 && tight_electron_indices.size() == 1
//   &&
//         loose_muon_indices.size() == 1 && loose_electron_indices.size() ==
//         1))
//     return false;
//   if ((AllMuonViews[tight_muon_indices[0]].Charge() *
//        AllElectronViews[tight_electron_indices[0]].Charge()) > 0)
//     return false;
//   Muons = MaterializeMuons(AllMuonViews, tight_muon_indices);
//   Electrons = MaterializeElectrons(AllElectronViews, tight_electron_indices);

//   MET = ev.GetMETVector(Event::MET_Type::PUPPI);
//   std::sort(Jets.begin(), Jets.end(), PtComparing);

//   TLorentzVector p4_nominal(0, 0, 0, 0);
//   TLorentzVector p4_shifted(0, 0, 0, 0);
//   for (const auto &jetView : AllJetViews) {
//     TLorentzVector v;
//     v.SetPtEtaPhiM(jetView.Pt(), jetView.Eta(), jetView.Phi(),
//     jetView.Mass()); p4_nominal += v;
//   }

//   if (systHelper->getCurrentIterSysTarget().find("Jet_En") !=
//       std::string::npos) {
//     const bool doBreakdown = HasFlag("doBreakdown");
//     const TString srcT = systHelper->getCurrentIterSysSource();
//     if (doBreakdown) {
//       if (srcT.EqualTo("total", TString::kIgnoreCase))
//         return false;
//       ApplyJetScaleVariation(AllJetViews, srcT);
//     } else {
//       if (!srcT.EqualTo("total", TString::kIgnoreCase))
//         return false;
//       ApplyJetScaleVariation(AllJetViews, "total");
//     }

//     if (systHelper->getCurrentIterVariation() == MyCorrection::variation::up)
//     {
//       for (const auto &jetView : AllJetViews) {
//         TLorentzVector v;
//         v.SetPtEtaPhiM(jetView.JesPtUp(), jetView.Eta(), jetView.Phi(),
//                        jetView.JesMassUp());
//         p4_shifted += v;
//       }
//     } else if (systHelper->getCurrentIterVariation() ==
//                MyCorrection::variation::down) {
//       for (const auto &jetView : AllJetViews) {
//         TLorentzVector v;
//         v.SetPtEtaPhiM(jetView.JesPtDown(), jetView.Eta(), jetView.Phi(),
//                        jetView.JesMassDown());
//         p4_shifted += v;
//       }
//     }
//   } else if (systHelper->getCurrentIterSysTarget() == "Jet_Res") {
//     MET = ev.GetMETVector(Event::MET_Type::PUPPI,
//                           systHelper->getCurrentIterVariation(),
//                           Event::MET_Syst::JER);
//     p4_shifted = p4_nominal;
//   } else if (systHelper->getCurrentIterSysTarget() == "UE") {
//     MET = ev.GetMETVector(Event::MET_Type::PUPPI,
//                           systHelper->getCurrentIterVariation(),
//                           Event::MET_Syst::UE);
//     p4_shifted = p4_nominal;
//   } else {
//     for (const auto &jetView : AllJetViews) {
//       TLorentzVector v;
//       v.SetPtEtaPhiM(jetView.SmearedPtNominal(), jetView.Eta(),
//       jetView.Phi(),
//                      jetView.SmearedMassNominal());
//       p4_shifted += v;
//     }
//   }

//   {
//     TLorentzVector delta = p4_shifted - p4_nominal;
//     MET.SetXYZM(MET.Px() - delta.Px(), MET.Py() - delta.Py(), 0., 0.);
//   }
//   if (MET.Pt() < 40.)
//     return false;
//   MyCorrection::variation jesVar = MyCorrection::variation::nom;
//   MyCorrection::variation jerVar = MyCorrection::variation::nom;
//   if (!IsDATA) {
//     if (systHelper->getCurrentIterSysTarget().find("Jet_En") !=
//         std::string::npos) {
//       jesVar = systHelper->getCurrentIterVariation();
//     } else if (systHelper->getCurrentIterSysTarget() == "Jet_Res") {
//       jerVar = systHelper->getCurrentIterVariation();
//     }
//   }
//   std::vector<size_t> jet_indices = SelectJetIndices(
//       AllJetViews, Jet_ID, 25., 2.5, jesVar, jerVar);
//   jet_indices = JetsVetoLeptonInside(AllJetViews, jet_indices,
//   AllElectronViews,
//                                      tight_electron_indices, AllMuonViews,
//                                      tight_muon_indices, 0.4);
//   if (jet_indices.size() != 2)
//     return false;
//   Jets = MaterializeJets(AllJetViews, jet_indices, jesVar, jerVar);
//   std::sort(Jets.begin(), Jets.end(), PtComparing);
//   return true;
// }

bool CalibrationTree::PassDYLightBaselineSelection() {
  if (!(ev.PassTrigger(Mu_Trigger[DataEra.Data()])))
    return false;
  if (!PassJetVetoMap(AllJetViews, AllMuonViews))
    return false;
  if (!PassMetFilter(AllJetViews, ev))
    return false;
  std::vector<size_t> loose_muon_indices = SelectMuonIndices(
      AllMuonViews, Muon_Veto_ID, Muon_Veto_Pt, Muon_Veto_Eta);
  loose_muon_indices =
      SelectMuonIndices(AllMuonViews, loose_muon_indices, Muon_Veto_Iso,
                        Muon_Veto_Pt, Muon_Veto_Eta);
  std::vector<size_t> tight_muon_indices =
      SelectMuonIndices(AllMuonViews, loose_muon_indices, Muon_Tight_ID,
                        Muon_Tight_Pt[DataEra.Data()], Muon_Tight_Eta);
  tight_muon_indices =
      SelectMuonIndices(AllMuonViews, tight_muon_indices, Muon_Tight_Iso,
                        Muon_Tight_Pt[DataEra.Data()], Muon_Tight_Eta);
  std::vector<size_t> loose_electron_indices = SelectElectronIndices(
      AllElectronViews, Electron_Veto_ID, Electron_Veto_Pt, Electron_Veto_Eta);
  std::vector<size_t> tight_electron_indices = SelectElectronIndices(
      AllElectronViews, loose_electron_indices, Electron_Tight_ID,
      Electron_Tight_Pt[DataEra.Data()], Electron_Tight_Eta);
  if (!(tight_muon_indices.size() == 2 && tight_electron_indices.size() == 0 &&
        loose_muon_indices.size() == 2 && loose_electron_indices.size() == 0))
    return false;
  if ((AllMuonViews[tight_muon_indices[0]].Charge() *
       AllMuonViews[tight_muon_indices[1]].Charge()) > 0)
    return false;
  Muons = MaterializeMuons(AllMuonViews, tight_muon_indices);
  std::sort(Muons.begin(), Muons.end(), PtComparing);
  Particle ZCand = Muons[0] + Muons[1];
  if (ZCand.M() < 81. || ZCand.M() > 101.)
    return false;

  MET = ev.GetMETVector(Event::MET_Type::PUPPI);

  TLorentzVector p4_nominal(0, 0, 0, 0);
  TLorentzVector p4_shifted(0, 0, 0, 0);
  for (const auto &jetView : AllJetViews) {
    TLorentzVector v;
    v.SetPtEtaPhiM(jetView.Pt(), jetView.Eta(), jetView.Phi(), jetView.Mass());
    p4_nominal += v;
  }

  if (systHelper->getCurrentIterSysTarget().find("Jet_En") !=
      std::string::npos) {
    const bool doBreakdown = HasFlag("doBreakdown");
    const TString srcT = systHelper->getCurrentIterSysSource();
    if (doBreakdown) {
      if (srcT.EqualTo("total", TString::kIgnoreCase))
        return false;
      ApplyJetScaleVariation(AllJetViews, srcT);
    } else {
      if (!srcT.EqualTo("total", TString::kIgnoreCase))
        return false;
      ApplyJetScaleVariation(AllJetViews, "total");
    }

    if (systHelper->getCurrentIterVariation() == MyCorrection::variation::up) {
      for (const auto &jetView : AllJetViews) {
        TLorentzVector v;
        v.SetPtEtaPhiM(jetView.JesPtUp(), jetView.Eta(), jetView.Phi(),
                       jetView.JesMassUp());
        p4_shifted += v;
      }
    } else if (systHelper->getCurrentIterVariation() ==
               MyCorrection::variation::down) {
      for (const auto &jetView : AllJetViews) {
        TLorentzVector v;
        v.SetPtEtaPhiM(jetView.JesPtDown(), jetView.Eta(), jetView.Phi(),
                       jetView.JesMassDown());
        p4_shifted += v;
      }
    }
  } else if (systHelper->getCurrentIterSysTarget() == "Jet_Res") {
    MET = ev.GetMETVector(Event::MET_Type::PUPPI,
                          systHelper->getCurrentIterVariation(),
                          Event::MET_Syst::JER);
    p4_shifted = p4_nominal;
  } else if (systHelper->getCurrentIterSysTarget() == "UE") {
    MET = ev.GetMETVector(Event::MET_Type::PUPPI,
                          systHelper->getCurrentIterVariation(),
                          Event::MET_Syst::UE);
    p4_shifted = p4_nominal;
  } else {
    for (const auto &jetView : AllJetViews) {
      TLorentzVector v;
      v.SetPtEtaPhiM(jetView.SmearedPtNominal(), jetView.Eta(), jetView.Phi(),
                     jetView.SmearedMassNominal());
      p4_shifted += v;
    }
  }

  {
    TLorentzVector delta = p4_shifted - p4_nominal;
    MET.SetXYZM(MET.Px() - delta.Px(), MET.Py() - delta.Py(), 0., 0.);
  }
  MyCorrection::variation jesVar = MyCorrection::variation::nom;
  MyCorrection::variation jerVar = MyCorrection::variation::nom;
  if (!IsDATA) {
    if (systHelper->getCurrentIterSysTarget().find("Jet_En") !=
        std::string::npos) {
      jesVar = systHelper->getCurrentIterVariation();
    } else if (systHelper->getCurrentIterSysTarget() == "Jet_Res") {
      jerVar = systHelper->getCurrentIterVariation();
    }
  }
  std::vector<size_t> jet_indices = SelectJetIndices(
      AllJetViews, Jet_ID, 25., 2.5, jesVar, jerVar);
  jet_indices = JetsVetoLeptonInside(AllJetViews, jet_indices, AllElectronViews,
                                     tight_electron_indices, AllMuonViews,
                                     tight_muon_indices, 0.4);
  if (jet_indices.size() != 1)
    return false;
  Jets = MaterializeJets(AllJetViews, jet_indices, jesVar, jerVar);
  std::sort(Jets.begin(), Jets.end(), PtComparing);
  if (ZCand.Pt() / Jets[0].Pt() < 0.75 || ZCand.Pt() / Jets[0].Pt() > 1.25)
    return false;
  if (ZCand.DeltaPhi(Jets[0]) < 2.f)
    return false;
  return true;
}

// bool CalibrationTree::PassDYLightBaselineSelection() {
//   if (!(ev.PassTrigger(Mu_Trigger[DataEra.Data()])))
//     return false;
//   if (!PassJetVetoMap(AllJetViews, AllMuonViews))
//     return false;
//   if (!PassMetFilter(AllJetViews, ev))
//     return false;
//   std::vector<size_t> loose_muon_indices = SelectMuonIndices(
//       AllMuonViews, Muon_Veto_ID, Muon_Veto_Pt, Muon_Veto_Eta);
//   loose_muon_indices =
//       SelectMuonIndices(AllMuonViews, loose_muon_indices, Muon_Veto_Iso,
//                         Muon_Veto_Pt, Muon_Veto_Eta);
//   std::vector<size_t> tight_muon_indices =
//       SelectMuonIndices(AllMuonViews, loose_muon_indices, Muon_Tight_ID,
//                         Muon_Tight_Pt[DataEra.Data()], Muon_Tight_Eta);
//   tight_muon_indices =
//       SelectMuonIndices(AllMuonViews, tight_muon_indices, Muon_Tight_Iso,
//                         Muon_Tight_Pt[DataEra.Data()], Muon_Tight_Eta);
//   std::vector<size_t> loose_electron_indices = SelectElectronIndices(
//       AllElectronViews, Electron_Veto_ID, Electron_Veto_Pt,
//       Electron_Veto_Eta);
//   std::vector<size_t> tight_electron_indices = SelectElectronIndices(
//       AllElectronViews, loose_electron_indices, Electron_Tight_ID,
//       Electron_Tight_Pt[DataEra.Data()], Electron_Tight_Eta);
//   if (!(tight_muon_indices.size() == 2 && tight_electron_indices.size() == 0
//   &&
//         loose_muon_indices.size() == 2 && loose_electron_indices.size() ==
//         0))
//     return false;
//   if ((AllMuonViews[tight_muon_indices[0]].Charge() *
//        AllMuonViews[tight_muon_indices[1]].Charge()) > 0)
//     return false;
//   Muons = MaterializeMuons(AllMuonViews, tight_muon_indices);
//   std::sort(Muons.begin(), Muons.end(), PtComparing);
//   Particle ZCand = Muons[0] + Muons[1];
//   if (ZCand.M() < 81. || ZCand.M() > 101.)
//     return false;

//   MET = ev.GetMETVector(Event::MET_Type::PUPPI);

//   TLorentzVector p4_nominal(0, 0, 0, 0);
//   TLorentzVector p4_shifted(0, 0, 0, 0);
//   for (const auto &jetView : AllJetViews) {
//     TLorentzVector v;
//     v.SetPtEtaPhiM(jetView.Pt(), jetView.Eta(), jetView.Phi(),
//     jetView.Mass()); p4_nominal += v;
//   }

//   if (systHelper->getCurrentIterSysTarget().find("Jet_En") !=
//       std::string::npos) {
//     const bool doBreakdown = HasFlag("doBreakdown");
//     const TString srcT = systHelper->getCurrentIterSysSource();
//     if (doBreakdown) {
//       if (srcT.EqualTo("total", TString::kIgnoreCase))
//         return false;
//       ApplyJetScaleVariation(AllJetViews, srcT);
//     } else {
//       if (!srcT.EqualTo("total", TString::kIgnoreCase))
//         return false;
//       ApplyJetScaleVariation(AllJetViews, "total");
//     }

//     if (systHelper->getCurrentIterVariation() == MyCorrection::variation::up)
//     {
//       for (const auto &jetView : AllJetViews) {
//         TLorentzVector v;
//         v.SetPtEtaPhiM(jetView.JesPtUp(), jetView.Eta(), jetView.Phi(),
//                        jetView.JesMassUp());
//         p4_shifted += v;
//       }
//     } else if (systHelper->getCurrentIterVariation() ==
//                MyCorrection::variation::down) {
//       for (const auto &jetView : AllJetViews) {
//         TLorentzVector v;
//         v.SetPtEtaPhiM(jetView.JesPtDown(), jetView.Eta(), jetView.Phi(),
//                        jetView.JesMassDown());
//         p4_shifted += v;
//       }
//     }
//   } else if (systHelper->getCurrentIterSysTarget() == "Jet_Res") {
//     MET = ev.GetMETVector(Event::MET_Type::PUPPI,
//                           systHelper->getCurrentIterVariation(),
//                           Event::MET_Syst::JER);
//     p4_shifted = p4_nominal;
//   } else if (systHelper->getCurrentIterSysTarget() == "UE") {
//     MET = ev.GetMETVector(Event::MET_Type::PUPPI,
//                           systHelper->getCurrentIterVariation(),
//                           Event::MET_Syst::UE);
//     p4_shifted = p4_nominal;
//   } else {
//     for (const auto &jetView : AllJetViews) {
//       TLorentzVector v;
//       v.SetPtEtaPhiM(jetView.SmearedPtNominal(), jetView.Eta(),
//       jetView.Phi(),
//                      jetView.SmearedMassNominal());
//       p4_shifted += v;
//     }
//   }

//   {
//     TLorentzVector delta = p4_shifted - p4_nominal;
//     MET.SetXYZM(MET.Px() - delta.Px(), MET.Py() - delta.Py(), 0., 0.);
//   }
//   MyCorrection::variation jesVar = MyCorrection::variation::nom;
//   MyCorrection::variation jerVar = MyCorrection::variation::nom;
//   if (!IsDATA) {
//     if (systHelper->getCurrentIterSysTarget().find("Jet_En") !=
//         std::string::npos) {
//       jesVar = systHelper->getCurrentIterVariation();
//     } else if (systHelper->getCurrentIterSysTarget() == "Jet_Res") {
//       jerVar = systHelper->getCurrentIterVariation();
//     }
//   }
//   std::vector<size_t> jet_indices = SelectJetIndices(
//       AllJetViews, Jet_ID, 25., 2.5, jesVar, jerVar);
//   jet_indices = JetsVetoLeptonInside(AllJetViews, jet_indices,
//   AllElectronViews,
//                                      tight_electron_indices, AllMuonViews,
//                                      tight_muon_indices, 0.4);
//   if (jet_indices.size() != 2)
//     return false;
//   RVec<Jet> Jets_tnp = MaterializeJets(AllJetViews, jet_indices, jesVar,
//   jerVar); Particle gluon = Jets_tnp[0] + Jets_tnp[1];
//   std::sort(Jets_tnp.begin(), Jets_tnp.end(), PtComparing);
//   if (ZCand.Pt() / gluon.Pt() < 0.75 || ZCand.Pt() / gluon.Pt() > 1.25)
//     return false;
//   if (ZCand.DeltaPhi(gluon) < 2.f)
//     return false;
//   std::vector<float> jet_cvnotb
//   {Jets_tnp[0].GetTaggerResult(JetTagging::JetFlavTagger::ParT,
//   JetTagging::JetFlavTaggerScoreType::CvNotB),
//   Jets_tnp[1].GetTaggerResult(JetTagging::JetFlavTagger::ParT,
//   JetTagging::JetFlavTaggerScoreType::CvNotB)}; const float wp = 0.1;
//   std::vector<size_t> pass_cvnotb_indices;
//   for (size_t i=0; i<jet_cvnotb.size(); ++i) {
//     if (jet_cvnotb[i] < wp)
//       pass_cvnotb_indices.push_back(i);
//   }
//   if (pass_cvnotb_indices.size() == 0)
//     return false;
//   else if (pass_cvnotb_indices.size() == 1){
//     //push back the jet which not pass cvnotb
//     size_t idx = (pass_cvnotb_indices[0] == 0) ? 1 : 0;
//     Jets.push_back(Jets_tnp[idx]);
//   }
//   else if (pass_cvnotb_indices.size() == 2){
//     //randomly pick one jet
//     size_t idx = rand() % 2;;
//     Jets.push_back(Jets_tnp[idx]);
//   }

//   size_t probjet_hadflav = Jets[0].hadronFlavour();
//   std::string probjet_hadflav_str =
//       (probjet_hadflav == 5)   ? "b"
//       : (probjet_hadflav == 4) ? "c"
//                              : "light";
//   FillHist("DYLight_ProbJet_CvNotB_HadFlav_" + probjet_hadflav_str,
//            jet_cvnotb[Jets_tnp[0] == Jets[0] ? 0 : 1], 1.f, 50, 0, 1);
//   return true;
// }

bool CalibrationTree::PassWCharmBaselineSelection() {
  const bool kRequireJetSoftMuon = false; // toggle soft-muon-in-jet requirement

  // --- common preselection ---
  if (!PassJetVetoMap(AllJetViews, AllMuonViews))
    return false;
  if (!PassMetFilter(AllJetViews, ev))
    return false;

  // primary lepton indices (no shadowing!)
  std::vector<size_t> tight_muon_indices;
  std::vector<size_t> tight_electron_indices;

  // keep primary lepton as base
  Lepton primary_lepton; // assumes Muon/Electron derive from Lepton (slicing OK
                         // for base-only use)
  bool primary_is_mu = false;

  // also keep real primary muon for dimuon veto
  Muon primary_muon;
  bool has_primary_muon = false;

  // --- channel-dependent trigger + exactly-one tight lepton ---
  if (channel == Channel::WCharm_Mu) {
    if (!ev.PassTrigger(Mu_Trigger[DataEra.Data()]))
      return false;

    auto loose_muon_indices = SelectMuonIndices(AllMuonViews, Muon_Veto_ID,
                                                Muon_Veto_Pt, Muon_Veto_Eta);
    loose_muon_indices =
        SelectMuonIndices(AllMuonViews, loose_muon_indices, Muon_Veto_Iso,
                          Muon_Veto_Pt, Muon_Veto_Eta);

    tight_muon_indices =
        SelectMuonIndices(AllMuonViews, loose_muon_indices, Muon_Tight_ID,
                          Muon_Tight_Pt[DataEra.Data()], Muon_Tight_Eta);
    tight_muon_indices =
        SelectMuonIndices(AllMuonViews, tight_muon_indices, Muon_Tight_Iso,
                          Muon_Tight_Pt[DataEra.Data()], Muon_Tight_Eta);

    auto loose_electron_indices =
        SelectElectronIndices(AllElectronViews, Electron_Veto_ID,
                              Electron_Veto_Pt, Electron_Veto_Eta);
    tight_electron_indices = SelectElectronIndices(
        AllElectronViews, loose_electron_indices, Electron_Tight_ID,
        Electron_Tight_Pt[DataEra.Data()], Electron_Tight_Eta);

    if (!(tight_muon_indices.size() == 1 &&
          tight_electron_indices.size() == 0 &&
          loose_muon_indices.size() == 1 && loose_electron_indices.size() == 0))
      return false;

    Muons = MaterializeMuons(AllMuonViews, tight_muon_indices);
    primary_muon = Muons[0];
    has_primary_muon = true;

    primary_lepton = static_cast<Lepton &>(Muons[0]);
    primary_is_mu = true;

  } else if (channel == Channel::WCharm_El) {
    if (!ev.PassTrigger(El_Trigger[DataEra.Data()]))
      return false;

    auto loose_muon_indices = SelectMuonIndices(AllMuonViews, Muon_Veto_ID,
                                                Muon_Veto_Pt, Muon_Veto_Eta);
    loose_muon_indices =
        SelectMuonIndices(AllMuonViews, loose_muon_indices, Muon_Veto_Iso,
                          Muon_Veto_Pt, Muon_Veto_Eta);

    tight_muon_indices =
        SelectMuonIndices(AllMuonViews, loose_muon_indices, Muon_Tight_ID,
                          Muon_Tight_Pt[DataEra.Data()], Muon_Tight_Eta);
    tight_muon_indices =
        SelectMuonIndices(AllMuonViews, tight_muon_indices, Muon_Tight_Iso,
                          Muon_Tight_Pt[DataEra.Data()], Muon_Tight_Eta);

    auto loose_electron_indices =
        SelectElectronIndices(AllElectronViews, Electron_Veto_ID,
                              Electron_Veto_Pt, Electron_Veto_Eta);
    tight_electron_indices = SelectElectronIndices(
        AllElectronViews, loose_electron_indices, Electron_Tight_ID,
        Electron_Tight_Pt[DataEra.Data()], Electron_Tight_Eta);

    if (!(tight_muon_indices.size() == 0 &&
          tight_electron_indices.size() == 1 &&
          loose_muon_indices.size() == 0 && loose_electron_indices.size() == 1))
      return false;

    Electrons = MaterializeElectrons(AllElectronViews, tight_electron_indices);
    primary_lepton = static_cast<Lepton &>(Electrons[0]);
    primary_is_mu = false;

  } else {
    return false;
  }

  lepton = primary_lepton;

  // --- JES/JER variation used for jet selection/materialization ---
  MyCorrection::variation jesVar = MyCorrection::variation::nom;
  MyCorrection::variation jerVar = MyCorrection::variation::nom;
  if (!IsDATA) {
    if (systHelper->getCurrentIterSysTarget().find("Jet_En") !=
        std::string::npos)
      jesVar = systHelper->getCurrentIterVariation();
    else if (systHelper->getCurrentIterSysTarget() == "Jet_Res")
      jerVar = systHelper->getCurrentIterVariation();
  }

  // --- select jets and veto overlap with tight leptons ---
  std::vector<size_t> jet_indices = SelectJetIndices(
      AllJetViews, Jet_ID, 25.f, 2.5, jesVar, jerVar);

  jet_indices = JetsVetoLeptonInside(AllJetViews, jet_indices, AllElectronViews,
                                     tight_electron_indices, AllMuonViews,
                                     tight_muon_indices, 0.4);

  if (jet_indices.size() != 1)
    return false;

  Jets = MaterializeJets(AllJetViews, jet_indices, jesVar, jerVar);
  std::sort(Jets.begin(), Jets.end(), PtComparing);

  // --- SV-in-jet info (use jet view indices) ---
  {
    const auto &jetView = AllJetViews[jet_indices[0]];
    wcharm_jet0_nsv = static_cast<int>(jetView.NSVs());

    auto valid_sv_index = [&](short idx) -> int {
      if (idx < 0)
        return -1;
      const std::size_t uidx = static_cast<std::size_t>(idx);
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
    // --- soft muon (non-iso) list ---
    std::vector<size_t> soft_muon_indices = SelectMuonIndices(
        AllMuonViews, Muon::MuonID::POG_TIGHT, 5.f, Muon_Tight_Eta);

    std::vector<size_t> soft_muon_indices_iso =
        SelectMuonIndices(AllMuonViews, soft_muon_indices,
                          Muon::MuonID::POG_PFISO_LOOSE, 5.f, Muon_Tight_Eta);

    std::vector<size_t> soft_muon_indices_noniso;
    soft_muon_indices_noniso.reserve(soft_muon_indices.size());
    for (auto idx : soft_muon_indices) {
      if (std::find(soft_muon_indices_iso.begin(), soft_muon_indices_iso.end(),
                    idx) == soft_muon_indices_iso.end())
        soft_muon_indices_noniso.push_back(idx);
    }

    // --- match non-iso soft muon to the (only) jet, require exactly one unique
    // muon ---
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
          it->second = kAmbiguous; // multiple muons on this jet
      }
    }

    size_t unique_softmu_idx = std::numeric_limits<size_t>::max();
    size_t unique_count = 0;
    for (const auto &[jet_idx, mu_idx] : jet_to_mu) {
      if (mu_idx == kAmbiguous)
        continue;
      unique_softmu_idx = mu_idx;
      ++unique_count;
    }
    if (unique_count != 1)
      return false;

    // --- jet composition veto ---
    wcharm_jet_muEF = Jets[0].muEF();
    wcharm_jet_neEmEF = Jets[0].neEmEF();
    wcharm_jet_muEF_plus_neEmEF = wcharm_jet_muEF + wcharm_jet_neEmEF;
    if (wcharm_jet_muEF > 0.5f)
      return false;
    float cut_muEF_plus_neEMF = channel == Channel::WCharm_Mu ? 0.7f : 1.0f;
    if (wcharm_jet_muEF_plus_neEmEF > cut_muEF_plus_neEMF)
      return false;

    // --- materialize soft muon ---
    Muon soft_muon = MaterializeMuons(AllMuonViews, {unique_softmu_idx})[0];

    //-- soft muon kinematics ---
    wcharm_soft_mu_pt = soft_muon.Pt();


    // --- dimuon mass veto only for mu-channel ---
    if (primary_is_mu && has_primary_muon) {
      Particle DiMuon = primary_muon + soft_muon;
      wcharm_dimuon_mass = DiMuon.M();
      if (wcharm_dimuon_mass < 12.f ||
          (wcharm_dimuon_mass > 80.f && wcharm_dimuon_mass < 100.f))
        return false;
    }

    // --- opposite sign between primary lepton and soft muon ---
    if (primary_lepton.Charge() * soft_muon.Charge() > 0)
      return false;
  }

  // --- MET and systematic shift logic (kept same structure) ---
  MET = ev.GetMETVector(Event::MET_Type::PUPPI);

  TLorentzVector p4_nominal(0, 0, 0, 0);
  TLorentzVector p4_shifted(0, 0, 0, 0);
  for (const auto &jetView : AllJetViews) {
    TLorentzVector v;
    v.SetPtEtaPhiM(jetView.Pt(), jetView.Eta(), jetView.Phi(), jetView.Mass());
    p4_nominal += v;
  }

  if (systHelper->getCurrentIterSysTarget().find("Jet_En") !=
      std::string::npos) {
    const bool doBreakdown = HasFlag("doBreakdown");
    const TString srcT = systHelper->getCurrentIterSysSource();
    if (doBreakdown) {
      if (srcT.EqualTo("total", TString::kIgnoreCase))
        return false;
      ApplyJetScaleVariation(AllJetViews, srcT);
    } else {
      if (!srcT.EqualTo("total", TString::kIgnoreCase))
        return false;
      ApplyJetScaleVariation(AllJetViews, "total");
    }

    if (systHelper->getCurrentIterVariation() == MyCorrection::variation::up) {
      for (const auto &jetView : AllJetViews) {
        TLorentzVector v;
        v.SetPtEtaPhiM(jetView.JesPtUp(), jetView.Eta(), jetView.Phi(),
                       jetView.JesMassUp());
        p4_shifted += v;
      }
    } else if (systHelper->getCurrentIterVariation() ==
               MyCorrection::variation::down) {
      for (const auto &jetView : AllJetViews) {
        TLorentzVector v;
        v.SetPtEtaPhiM(jetView.JesPtDown(), jetView.Eta(), jetView.Phi(),
                       jetView.JesMassDown());
        p4_shifted += v;
      }
    }
  } else if (systHelper->getCurrentIterSysTarget() == "Jet_Res") {
    MET = ev.GetMETVector(Event::MET_Type::PUPPI,
                          systHelper->getCurrentIterVariation(),
                          Event::MET_Syst::JER);
    p4_shifted = p4_nominal;
  } else if (systHelper->getCurrentIterSysTarget() == "UE") {
    MET = ev.GetMETVector(Event::MET_Type::PUPPI,
                          systHelper->getCurrentIterVariation(),
                          Event::MET_Syst::UE);
    p4_shifted = p4_nominal;
  } else {
    for (const auto &jetView : AllJetViews) {
      TLorentzVector v;
      v.SetPtEtaPhiM(jetView.SmearedPtNominal(), jetView.Eta(), jetView.Phi(),
                     jetView.SmearedMassNominal());
      p4_shifted += v;
    }
  }

  {
    TLorentzVector delta = p4_shifted - p4_nominal;
    MET.SetXYZM(MET.Px() - delta.Px(), MET.Py() - delta.Py(), 0., 0.);
  }

  if (MET.Pt() < 30.f)
    return false;

  // --- W kinematics + topology (now using base Lepton) ---
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
  if (wcharm_dphi_met_jet < 1.0)
    return false;
  wcharm_dphi_lep_jet = std::abs(primary_lepton.DeltaPhi(Jets[0]));
  if (wcharm_dphi_lep_jet > 2.0)
    return false;
  wcharm_dphi_w_jet = std::abs(WT.DeltaPhi(Jets[0]));
  if (wcharm_dphi_w_jet < 2.0)
    return false;

  wcharm_pt_ratio = Jets[0].Pt() / WT.Pt();
  if (wcharm_pt_ratio < 0.5f || wcharm_pt_ratio > 2.0f)
    return false;

  const float trkmet_pt = TrkMET_pt;
  const float trkmet_phi = TrkMET_phi;
  const float dphi_trkmet_met = std::abs(
      std::atan2(std::sin(trkmet_phi - MET.Phi()), std::cos(trkmet_phi - MET.Phi())));
  wcharm_dphi_trkmet_met = dphi_trkmet_met;
  if (wcharm_dphi_trkmet_met > 1.0f)
    return false;

  
  return true;
}

bool CalibrationTree::PassTTSemilepBaselineSelection() {
  if (!(ev.PassTrigger(Mu_Trigger[DataEra.Data()])))
    return false;
  if (!PassJetVetoMap(AllJetViews, AllMuonViews))
    return false;
  if (!PassMetFilter(AllJetViews, ev))
    return false;

  std::vector<size_t> loose_muon_indices = SelectMuonIndices(
      AllMuonViews, Muon_Veto_ID, Muon_Veto_Pt, Muon_Veto_Eta);
  loose_muon_indices =
      SelectMuonIndices(AllMuonViews, loose_muon_indices, Muon_Veto_Iso,
                        Muon_Veto_Pt, Muon_Veto_Eta);
  std::vector<size_t> tight_muon_indices =
      SelectMuonIndices(AllMuonViews, loose_muon_indices, Muon_Tight_ID,
                        Muon_Tight_Pt[DataEra.Data()], Muon_Tight_Eta);
  tight_muon_indices =
      SelectMuonIndices(AllMuonViews, tight_muon_indices, Muon_Tight_Iso,
                        Muon_Tight_Pt[DataEra.Data()], Muon_Tight_Eta);
  std::vector<size_t> loose_electron_indices = SelectElectronIndices(
      AllElectronViews, Electron_Veto_ID, Electron_Veto_Pt, Electron_Veto_Eta);
  std::vector<size_t> tight_electron_indices = SelectElectronIndices(
      AllElectronViews, loose_electron_indices, Electron_Tight_ID,
      Electron_Tight_Pt[DataEra.Data()], Electron_Tight_Eta);

  if (!(tight_muon_indices.size() == 1 && tight_electron_indices.size() == 0 &&
        loose_muon_indices.size() == 1 && loose_electron_indices.size() == 0))
    return false;

  Muons = MaterializeMuons(AllMuonViews, tight_muon_indices);

  MET = ev.GetMETVector(Event::MET_Type::PUPPI);

  TLorentzVector p4_nominal(0, 0, 0, 0);
  TLorentzVector p4_shifted(0, 0, 0, 0);
  for (const auto &jetView : AllJetViews) {
    TLorentzVector v;
    v.SetPtEtaPhiM(jetView.Pt(), jetView.Eta(), jetView.Phi(), jetView.Mass());
    p4_nominal += v;
  }

  if (systHelper->getCurrentIterSysTarget().find("Jet_En") !=
      std::string::npos) {
    const bool doBreakdown = HasFlag("doBreakdown");
    const TString srcT = systHelper->getCurrentIterSysSource();
    if (doBreakdown) {
      if (srcT.EqualTo("total", TString::kIgnoreCase))
        return false;
      ApplyJetScaleVariation(AllJetViews, srcT);
    } else {
      if (!srcT.EqualTo("total", TString::kIgnoreCase))
        return false;
      ApplyJetScaleVariation(AllJetViews, "total");
    }

    if (systHelper->getCurrentIterVariation() == MyCorrection::variation::up) {
      for (const auto &jetView : AllJetViews) {
        TLorentzVector v;
        v.SetPtEtaPhiM(jetView.JesPtUp(), jetView.Eta(), jetView.Phi(),
                       jetView.JesMassUp());
        p4_shifted += v;
      }
    } else if (systHelper->getCurrentIterVariation() ==
               MyCorrection::variation::down) {
      for (const auto &jetView : AllJetViews) {
        TLorentzVector v;
        v.SetPtEtaPhiM(jetView.JesPtDown(), jetView.Eta(), jetView.Phi(),
                       jetView.JesMassDown());
        p4_shifted += v;
      }
    }
  } else if (systHelper->getCurrentIterSysTarget() == "Jet_Res") {
    MET = ev.GetMETVector(Event::MET_Type::PUPPI,
                          systHelper->getCurrentIterVariation(),
                          Event::MET_Syst::JER);
    p4_shifted = p4_nominal;
  } else if (systHelper->getCurrentIterSysTarget() == "UE") {
    MET = ev.GetMETVector(Event::MET_Type::PUPPI,
                          systHelper->getCurrentIterVariation(),
                          Event::MET_Syst::UE);
    p4_shifted = p4_nominal;
  } else {
    for (const auto &jetView : AllJetViews) {
      TLorentzVector v;
      v.SetPtEtaPhiM(jetView.SmearedPtNominal(), jetView.Eta(), jetView.Phi(),
                     jetView.SmearedMassNominal());
      p4_shifted += v;
    }
  }

  {
    TLorentzVector delta = p4_shifted - p4_nominal;
    MET.SetXYZM(MET.Px() - delta.Px(), MET.Py() - delta.Py(), 0., 0.);
  }

  MyCorrection::variation jesVar = MyCorrection::variation::nom;
  MyCorrection::variation jerVar = MyCorrection::variation::nom;
  if (!IsDATA) {
    if (systHelper->getCurrentIterSysTarget().find("Jet_En") !=
        std::string::npos) {
      jesVar = systHelper->getCurrentIterVariation();
    } else if (systHelper->getCurrentIterSysTarget() == "Jet_Res") {
      jerVar = systHelper->getCurrentIterVariation();
    }
  }

  std::vector<size_t> jet_indices = SelectJetIndices(
      AllJetViews, Jet_ID, 25., 2.5, jesVar, jerVar);
  jet_indices = JetsVetoLeptonInside(AllJetViews, jet_indices, AllElectronViews,
                                     tight_electron_indices, AllMuonViews,
                                     tight_muon_indices, 0.4);
  if (jet_indices.size() != 4)
    return false;

  Jets = MaterializeJets(AllJetViews, jet_indices, jesVar, jerVar);
  std::sort(Jets.begin(), Jets.end(), PtComparing);

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

  // ---- neutrino pz solutions ----
  std::variant<float, std::pair<float, float>> neutrinoPz =
      SolveNeutrinoPz(Muons[0], MET);

  std::vector<Particle> neutrino_solutions;
  neutrino_solutions.reserve(2);

  if (std::holds_alternative<float>(neutrinoPz)) {
    float pz = std::get<float>(neutrinoPz);
    Particle neutrino_p4;
    neutrino_p4.SetPxPyPzE(MET.Px(), MET.Py(), pz,
                           std::sqrt(MET.Pt() * MET.Pt() + pz * pz));
    neutrino_solutions.push_back(neutrino_p4);
  } else {
    auto pz_pair = std::get<std::pair<float, float>>(neutrinoPz);
    Particle neutrino_p4_1;
    neutrino_p4_1.SetPxPyPzE(
        MET.Px(), MET.Py(), pz_pair.first,
        std::sqrt(MET.Pt() * MET.Pt() + pz_pair.first * pz_pair.first));
    neutrino_solutions.push_back(neutrino_p4_1);
    Particle neutrino_p4_2;
    neutrino_p4_2.SetPxPyPzE(
        MET.Px(), MET.Py(), pz_pair.second,
        std::sqrt(MET.Pt() * MET.Pt() + pz_pair.second * pz_pair.second));
    neutrino_solutions.push_back(neutrino_p4_2);
  }

  // ---- scan all combinations & neutrino solutions, pick best fit ----
  best_KF_result.clear();
  float min_total_chi2 = std::numeric_limits<float>::max();
  bool found_good_fit = false;
  size_t best_comb_idx = 0;

  for (auto &neutrino_sol : neutrino_solutions) {
    for (size_t comb_idx = 0; comb_idx < comb_lut.size(); ++comb_idx) {
      const auto &comb = comb_lut[comb_idx];

      KinFitterResult tt_fit_res =
          Chi2Prefit(Jets[comb[0]], Jets[comb[1]], Jets[comb[2]], Jets[comb[3]], neutrino_sol, Muons[0]);

      if (tt_fit_res.status != 0)
        continue;  // only accept converged fits

      float total_chi2 = tt_fit_res.chi2_thad + tt_fit_res.chi2_tlep +
                         tt_fit_res.chi2_whad + tt_fit_res.chi2_wlep;

      if (total_chi2 < min_total_chi2) {
        min_total_chi2 = total_chi2;
        best_KF_result = tt_fit_res;
        found_good_fit = true;
        best_comb_idx = comb_idx;
      }
    }
  }

  if (!found_good_fit)
    return false;  // no valid kinematic fit -> reject event

  if (std::log(min_total_chi2 + 1e-6) > 5.5f)
    return false;

  Jet W1 = Jets[comb_lut[best_comb_idx][2]];
  Jet W2 = Jets[comb_lut[best_comb_idx][3]];

  Jets.clear();
  Jets.push_back(W1);
  Jets.push_back(W2);

  return true;
}
