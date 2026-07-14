#include "HadronAnalyzer.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <string>

namespace {

constexpr std::array<const char *, 3> kJetFlavourNames = {
    "LightJet", "CharmJet", "BottomJet"};
constexpr std::array<const char *, 6> kGenOriginNames = {
    "Prompt", "FromDstar", "FromC", "FromB", "Feeddown", "Other"};
constexpr std::array<const char *, 9> kGenPdgNames = {
    "D0",      "Dplus",  "Dstar",   "Lambda", "LambdaC",
    "LambdaB", "BMeson", "BBaryon", "Other"};

template <typename T, std::size_t N>
void FillArray(std::array<T, N> &values, T value) {
  values.fill(value);
}

template <typename... Vectors>
void ClearVectors(Vectors &...vectors) {
  (vectors.clear(), ...);
}

template <std::size_t N>
void SetBinLabels(const Hist1DHandle &histogram,
                  const std::array<const char *, N> &labels) {
  for (std::size_t index = 0; index < N; ++index)
    histogram.get()->GetXaxis()->SetBinLabel(index + 1, labels[index]);
}

} // namespace

void HadronAnalyzer::OutputBuffers::clear() {
  run = 0;
  luminosityBlock = 0;
  event = 0;
  channel = -1;
  weight = 1.f;
  ht = 0.f;
  metPt = 0.f;
  metPhi = 0.f;
  leptonPt = 0.f;
  leptonEta = 0.f;
  leptonPhi = 0.f;
  nJet = 0;
  nBTaggedJet = 0;
  nCTaggedJet = 0;

  FillArray(jetPt, -999.f);
  FillArray(jetEta, -999.f);
  FillArray(jetPhi, -999.f);
  FillArray(jetMass, -999.f);
  FillArray(jetOriginalIndex, -1);
  FillArray(jetHadronFlavour, -1);
  FillArray(jetPartonFlavour, -999);
  FillArray(jetGenJetIndex, -1);
  FillArray(genJetPt, -999.f);
  FillArray(genJetEta, -999.f);
  FillArray(genJetPhi, -999.f);
  FillArray(genJetMass, -999.f);
  FillArray(recoGenJetDeltaR, -999.f);
  FillArray(genJetHadronFlavour, -1);
  FillArray(genJetPartonFlavour, -999);

  FillArray(genJetNBottomBaryon, -1);
  FillArray(genJetNCharmBaryon, -1);
  FillArray(genJetND0, -1);
  FillArray(genJetNDHadron, -1);
  FillArray(genJetNDplus, -1);
  FillArray(genJetNDstar, -1);
  FillArray(genJetNFeeddownD, -1);
  FillArray(genJetNLambdaB, -1);
  FillArray(genJetNLambdaBaryon, -1);
  FillArray(genJetNLambdaC, -1);
  FillArray(genJetNLightLambda, -1);
  FillArray(genJetNPromptD, -1);

  FillArray(nD0, 0);
  FillArray(nDstar, 0);
  FillArray(nLambda, 0);
  FillArray(nLambdaC, 0);
  FillArray(nGenHadron, 0);

  ClearVectors(
      bphJetRank, bphSpecies, bphSourceIndex, bphPdgId, bphCharge,
      bphDaughterIndex1, bphDaughterIndex2, bphDaughterIndex3);
  ClearVectors(
      bphDeltaR, bphPt, bphEta, bphPhi, bphMass, bphFitPt, bphFitEta,
      bphFitPhi, bphFitMass, bphMassHypothesis1, bphMassHypothesis2,
      bphMassError1, bphMassError2, bphSvProbability, bphSvChi2, bphSvNdof,
      bphLxy, bphLxyError, bphCosTheta2D, bphDeltaMass);
  ClearVectors(
      genHadronJetRank, genHadronGenJetIndex, genHadronSourceIndex,
      genHadronPdgId, genHadronMotherPdgId, genHadronAncestorPdgId,
      genHadronFlavour, genHadronStatus, genHadronIsBottomBaryon,
      genHadronIsCharmBaryon, genHadronIsD0, genHadronIsDHadron,
      genHadronIsDplus, genHadronIsDstar, genHadronIsFeeddown,
      genHadronIsFromB, genHadronIsFromC, genHadronIsFromDstar,
      genHadronIsLambdaB, genHadronIsLambdaBaryon, genHadronIsLambdaC,
      genHadronIsLastCopy, genHadronIsLightLambda, genHadronIsPrompt);
  ClearVectors(genHadronPt, genHadronEta, genHadronPhi, genHadronMass,
               genHadronDeltaR, genHadronRecoJetDeltaR, genHadronVx,
               genHadronVy, genHadronVz);
}

void HadronAnalyzer::initializeAnalyzer() {
  if (IsDATA)
    throw SKNano::ConfigError("[HadronAnalyzer] gen-hadron study requires MC");
  if (HasFlag("Mu") == HasFlag("El"))
    throw SKNano::ConfigError(
        "[HadronAnalyzer] select exactly one channel with --userflags Mu or El");
  if (!HasFlag("BaselineOnly"))
    Userflags.push_back("BaselineOnly");
  if (!HasFlag("QuadJet"))
    Userflags.push_back("QuadJet");

  Vcb::initializeAnalyzer();

  if (!HasD0ToKPiViews() || !HasDstarToD0PiViews() ||
      !HasGenJetBPHViews() || !HasGenJetHadronViews() ||
      !HasLambdaViews() || !HasLambdaCToPKPiViews())
    throw SKNano::ConfigError(
        "[HadronAnalyzer] input does not contain the complete CustomBPH schema");

  BookOutputTree();
  BookHistograms();
}

void HadronAnalyzer::BookOutputTree() {
  outputTree_ = BookTree("BPHEvents");
  outputTree_.Branch("run", output_.run);
  outputTree_.Branch("luminosityBlock", output_.luminosityBlock);
  outputTree_.Branch("event", output_.event);
  outputTree_.Branch("channel", output_.channel);
  outputTree_.Branch("weight", output_.weight);
  outputTree_.Branch("HT", output_.ht);
  outputTree_.Branch("MET_pt", output_.metPt);
  outputTree_.Branch("MET_phi", output_.metPhi);
  outputTree_.Branch("Lepton_pt", output_.leptonPt);
  outputTree_.Branch("Lepton_eta", output_.leptonEta);
  outputTree_.Branch("Lepton_phi", output_.leptonPhi);
  outputTree_.Branch("nJet", output_.nJet);
  outputTree_.Branch("nBTaggedJet", output_.nBTaggedJet);
  outputTree_.Branch("nCTaggedJet", output_.nCTaggedJet);

  outputTree_.Branch("Jet_pt", output_.jetPt);
  outputTree_.Branch("Jet_eta", output_.jetEta);
  outputTree_.Branch("Jet_phi", output_.jetPhi);
  outputTree_.Branch("Jet_mass", output_.jetMass);
  outputTree_.Branch("Jet_originalIndex", output_.jetOriginalIndex);
  outputTree_.Branch("Jet_hadronFlavour", output_.jetHadronFlavour);
  outputTree_.Branch("Jet_partonFlavour", output_.jetPartonFlavour);
  outputTree_.Branch("Jet_genJetIdx", output_.jetGenJetIndex);
  outputTree_.Branch("GenJet_pt", output_.genJetPt);
  outputTree_.Branch("GenJet_eta", output_.genJetEta);
  outputTree_.Branch("GenJet_phi", output_.genJetPhi);
  outputTree_.Branch("GenJet_mass", output_.genJetMass);
  outputTree_.Branch("Jet_genJetDeltaR", output_.recoGenJetDeltaR);
  outputTree_.Branch("GenJet_hadronFlavour", output_.genJetHadronFlavour);
  outputTree_.Branch("GenJet_partonFlavour", output_.genJetPartonFlavour);
  outputTree_.Branch("GenJet_nBottomBaryon", output_.genJetNBottomBaryon);
  outputTree_.Branch("GenJet_nCharmBaryon", output_.genJetNCharmBaryon);
  outputTree_.Branch("GenJet_nD0", output_.genJetND0);
  outputTree_.Branch("GenJet_nDHadron", output_.genJetNDHadron);
  outputTree_.Branch("GenJet_nDplus", output_.genJetNDplus);
  outputTree_.Branch("GenJet_nDstar", output_.genJetNDstar);
  outputTree_.Branch("GenJet_nFeeddownD", output_.genJetNFeeddownD);
  outputTree_.Branch("GenJet_nLambdaB", output_.genJetNLambdaB);
  outputTree_.Branch("GenJet_nLambdaBaryon", output_.genJetNLambdaBaryon);
  outputTree_.Branch("GenJet_nLambdaC", output_.genJetNLambdaC);
  outputTree_.Branch("GenJet_nLightLambda", output_.genJetNLightLambda);
  outputTree_.Branch("GenJet_nPromptD", output_.genJetNPromptD);

  outputTree_.Branch("Jet_nD0", output_.nD0);
  outputTree_.Branch("Jet_nDstar", output_.nDstar);
  outputTree_.Branch("Jet_nLambda", output_.nLambda);
  outputTree_.Branch("Jet_nLambdaC", output_.nLambdaC);
  outputTree_.Branch("Jet_nGenHadron", output_.nGenHadron);

  outputTree_.Branch("BPH_jetRank", output_.bphJetRank);
  outputTree_.Branch("BPH_species", output_.bphSpecies);
  outputTree_.Branch("BPH_sourceIdx", output_.bphSourceIndex);
  outputTree_.Branch("BPH_deltaR", output_.bphDeltaR);
  outputTree_.Branch("BPH_pt", output_.bphPt);
  outputTree_.Branch("BPH_eta", output_.bphEta);
  outputTree_.Branch("BPH_phi", output_.bphPhi);
  outputTree_.Branch("BPH_mass", output_.bphMass);
  outputTree_.Branch("BPH_pdgId", output_.bphPdgId);
  outputTree_.Branch("BPH_charge", output_.bphCharge);
  outputTree_.Branch("BPH_fitPt", output_.bphFitPt);
  outputTree_.Branch("BPH_fitEta", output_.bphFitEta);
  outputTree_.Branch("BPH_fitPhi", output_.bphFitPhi);
  outputTree_.Branch("BPH_fitMass", output_.bphFitMass);
  outputTree_.Branch("BPH_massHypothesis1", output_.bphMassHypothesis1);
  outputTree_.Branch("BPH_massHypothesis2", output_.bphMassHypothesis2);
  outputTree_.Branch("BPH_massError1", output_.bphMassError1);
  outputTree_.Branch("BPH_massError2", output_.bphMassError2);
  outputTree_.Branch("BPH_svProbability", output_.bphSvProbability);
  outputTree_.Branch("BPH_svChi2", output_.bphSvChi2);
  outputTree_.Branch("BPH_svNdof", output_.bphSvNdof);
  outputTree_.Branch("BPH_lxy", output_.bphLxy);
  outputTree_.Branch("BPH_lxyError", output_.bphLxyError);
  outputTree_.Branch("BPH_cosTheta2D", output_.bphCosTheta2D);
  outputTree_.Branch("BPH_deltaMass", output_.bphDeltaMass);
  outputTree_.Branch("BPH_daughterIdx1", output_.bphDaughterIndex1);
  outputTree_.Branch("BPH_daughterIdx2", output_.bphDaughterIndex2);
  outputTree_.Branch("BPH_daughterIdx3", output_.bphDaughterIndex3);

  outputTree_.Branch("GenHadron_jetRank", output_.genHadronJetRank);
  outputTree_.Branch("GenHadron_genJetIdx", output_.genHadronGenJetIndex);
  outputTree_.Branch("GenHadron_srcIdx", output_.genHadronSourceIndex);
  outputTree_.Branch("GenHadron_pdgId", output_.genHadronPdgId);
  outputTree_.Branch("GenHadron_motherPdgId", output_.genHadronMotherPdgId);
  outputTree_.Branch("GenHadron_ancestorPdgId", output_.genHadronAncestorPdgId);
  outputTree_.Branch("GenHadron_flavour", output_.genHadronFlavour);
  outputTree_.Branch("GenHadron_status", output_.genHadronStatus);
  outputTree_.Branch("GenHadron_pt", output_.genHadronPt);
  outputTree_.Branch("GenHadron_eta", output_.genHadronEta);
  outputTree_.Branch("GenHadron_phi", output_.genHadronPhi);
  outputTree_.Branch("GenHadron_mass", output_.genHadronMass);
  outputTree_.Branch("GenHadron_deltaR", output_.genHadronDeltaR);
  outputTree_.Branch("GenHadron_recoJetDeltaR", output_.genHadronRecoJetDeltaR);
  outputTree_.Branch("GenHadron_vx", output_.genHadronVx);
  outputTree_.Branch("GenHadron_vy", output_.genHadronVy);
  outputTree_.Branch("GenHadron_vz", output_.genHadronVz);
  outputTree_.Branch("GenHadron_isBottomBaryon", output_.genHadronIsBottomBaryon);
  outputTree_.Branch("GenHadron_isCharmBaryon", output_.genHadronIsCharmBaryon);
  outputTree_.Branch("GenHadron_isD0", output_.genHadronIsD0);
  outputTree_.Branch("GenHadron_isDHadron", output_.genHadronIsDHadron);
  outputTree_.Branch("GenHadron_isDplus", output_.genHadronIsDplus);
  outputTree_.Branch("GenHadron_isDstar", output_.genHadronIsDstar);
  outputTree_.Branch("GenHadron_isFeeddown", output_.genHadronIsFeeddown);
  outputTree_.Branch("GenHadron_isFromB", output_.genHadronIsFromB);
  outputTree_.Branch("GenHadron_isFromC", output_.genHadronIsFromC);
  outputTree_.Branch("GenHadron_isFromDstar", output_.genHadronIsFromDstar);
  outputTree_.Branch("GenHadron_isLambdaB", output_.genHadronIsLambdaB);
  outputTree_.Branch("GenHadron_isLambdaBaryon", output_.genHadronIsLambdaBaryon);
  outputTree_.Branch("GenHadron_isLambdaC", output_.genHadronIsLambdaC);
  outputTree_.Branch("GenHadron_isLastCopy", output_.genHadronIsLastCopy);
  outputTree_.Branch("GenHadron_isLightLambda", output_.genHadronIsLightLambda);
  outputTree_.Branch("GenHadron_isPrompt", output_.genHadronIsPrompt);
}

void HadronAnalyzer::BookHistograms() {
  const auto histograms = Hists("HadronAnalyzer");
  selectedEvents_ = histograms.Book1D("SelectedEvents", 1, 0., 1.);
  for (std::size_t rank = 0; rank < kLeadingJets; ++rank) {
    const auto jet = histograms.Group("Jet" + std::to_string(rank));
    d0Multiplicity_[rank] = jet.Book1D("D0Multiplicity", 10, 0., 10.);
    dstarMultiplicity_[rank] =
        jet.Book1D("DstarMultiplicity", 6, 0., 6.);
    lambdaMultiplicity_[rank] =
        jet.Book1D("LambdaMultiplicity", 10, 0., 10.);
    lambdaCMultiplicity_[rank] =
        jet.Book1D("LambdaCMultiplicity", 10, 0., 10.);
    genHadronMultiplicity_[rank] =
        jet.Book1D("GenHadronMultiplicity", 30, 0., 30.);

    const auto comparison = histograms.Group(
        "RecoVsGen/Jet" + std::to_string(rank));
    recoVsGenD0_[rank] = comparison.Book2D("D0Multiplicity", 8, 0., 8., 8,
                                            0., 8.);
    recoVsGenDstar_[rank] = comparison.Book2D(
        "DstarMultiplicity", 6, 0., 6., 6, 0., 6.);
    recoVsGenLambda_[rank] = comparison.Book2D(
        "LambdaMultiplicity", 8, 0., 8., 8, 0., 8.);
    recoVsGenLambdaC_[rank] = comparison.Book2D(
        "LambdaCMultiplicity", 8, 0., 8., 8, 0., 8.);
    recoVsGenTotal_[rank] = comparison.Book2D(
        "TotalMultiplicity", 20, 0., 20., 30, 0., 30.);
    for (auto *handle : {&recoVsGenD0_[rank], &recoVsGenDstar_[rank],
                         &recoVsGenLambda_[rank], &recoVsGenLambdaC_[rank],
                         &recoVsGenTotal_[rank]}) {
      handle->get()->GetXaxis()->SetTitle("N(reco candidates)");
      handle->get()->GetYaxis()->SetTitle("N(gen hadrons)");
    }
  }

  for (std::size_t flavour = 0; flavour < kJetFlavourCategories; ++flavour) {
    const auto jet = histograms.Group(kJetFlavourNames[flavour]);
    d0FitMassKPi_[flavour] = jet.Book1D("D0/FitMassKPi", 70, 1.70, 2.05);
    d0PtFraction_[flavour] = jet.Book1D("D0/PtFraction", 60, 0., 1.5);
    d0DeltaRJet_[flavour] = jet.Book1D("D0/DeltaRJet", 40, 0., 0.4);
    d0LxySignificance_[flavour] =
        jet.Book1D("D0/LxySignificance", 60, 0., 30.);
    d0CosTheta2D_[flavour] = jet.Book1D("D0/CosTheta2D", 50, -1., 1.);
    dstarDeltaMass_[flavour] =
        jet.Book1D("Dstar/DeltaMass", 70, 0.135, 0.170);
    lambdaFitMass_[flavour] =
        jet.Book1D("Lambda/FitMass", 80, 1.08, 1.16);
    lambdaCFitMass_[flavour] =
        jet.Book1D("LambdaC/FitMass", 90, 2.15, 2.42);
    lambdaCPtFraction_[flavour] =
        jet.Book1D("LambdaC/PtFraction", 60, 0., 1.5);
    genHadronPtFraction_[flavour] =
        jet.Book1D("GenHadron/PtFraction", 60, 0., 1.5);
    genHadronDeltaRRecoJet_[flavour] =
        jet.Book1D("GenHadron/DeltaRRecoJet", 60, 0., 0.6);
    genHadronOriginCategory_[flavour] =
        jet.Book1D("GenHadron/OriginCategory", kGenOriginNames.size(), -0.5,
                   kGenOriginNames.size() - 0.5);
    genHadronPdgCategory_[flavour] =
        jet.Book1D("GenHadron/PdgCategory", kGenPdgNames.size(), -0.5,
                   kGenPdgNames.size() - 0.5);
    SetBinLabels(genHadronOriginCategory_[flavour], kGenOriginNames);
    SetBinLabels(genHadronPdgCategory_[flavour], kGenPdgNames);
  }
}

void HadronAnalyzer::executeEvent() {
  ev = GetEvent();
  if (!myCorr->IsGoldenLumi(RunNumber, luminosityBlock))
    return;

  {
    auto phase = MeasurePerformancePhase("object_view");
    AllMuonViews = GetAllMuonViews();
    AllElectronViews = GetAllElectronViews();
    AllJetViews = GetAllJetViews();
    AllGenJets = GetAllGenJetViews();
  }

  for (const auto &systDummy : *systHelper) {
    static_cast<void>(systDummy);
    leptons.clear();
    UpdateActiveOtLutForCurrentSystematic();
    const bool removeFlavourTagCut = HasFlag("RemoveFlavTagCut");
    if (!PassBaseLineSelection(removeFlavourTagCut, false))
      continue;
    if (Jets.size() < kLeadingJets)
      return;
    FillSelectedEvent();
    return;
  }
}

int HadronAnalyzer::FindNearestLeadingJet(float eta, float phi,
                                     float &deltaR) const {
  TLorentzVector candidate;
  candidate.SetPtEtaPhiM(1.f, eta, phi, 0.f);
  int bestRank = -1;
  deltaR = std::numeric_limits<float>::infinity();
  for (std::size_t rank = 0; rank < kLeadingJets; ++rank) {
    const float candidateDeltaR = Jets[rank].P4().DeltaR(candidate);
    if (candidateDeltaR < deltaR) {
      deltaR = candidateDeltaR;
      bestRank = static_cast<int>(rank);
    }
  }
  if (deltaR >= kBPHJetRadius) {
    deltaR = -999.f;
    return -1;
  }
  return bestRank;
}

std::size_t HadronAnalyzer::JetFlavourIndex(int jetRank) const {
  const int flavour = std::abs(Jets.at(jetRank).HadronFlavour());
  if (flavour == 5)
    return static_cast<std::size_t>(JetFlavourCategory::Bottom);
  if (flavour == 4)
    return static_cast<std::size_t>(JetFlavourCategory::Charm);
  return static_cast<std::size_t>(JetFlavourCategory::Light);
}

int HadronAnalyzer::GenOriginCategory(bool isPrompt, bool isFromB,
                                      bool isFromC, bool isFromDstar,
                                      bool isFeeddown) {
  if (isFromDstar)
    return 1;
  if (isFromB)
    return 3;
  if (isFromC)
    return 2;
  if (isFeeddown)
    return 4;
  if (isPrompt)
    return 0;
  return 5;
}

int HadronAnalyzer::GenPdgCategory(int pdgId, bool isBottomBaryon) {
  const int absolutePdgId = std::abs(pdgId);
  if (absolutePdgId == 421)
    return 0;
  if (absolutePdgId == 411)
    return 1;
  if (absolutePdgId == 413 || absolutePdgId == 423 ||
      absolutePdgId == 433)
    return 2;
  if (absolutePdgId == 3122)
    return 3;
  if (absolutePdgId == 4122)
    return 4;
  if (absolutePdgId == 5122)
    return 5;
  if (absolutePdgId == 511 || absolutePdgId == 521 ||
      absolutePdgId == 531 || absolutePdgId == 541)
    return 6;
  if (isBottomBaryon)
    return 7;
  return 8;
}

void HadronAnalyzer::FillBPHHistograms(const BPHRecord &record, int jetRank,
                                       float deltaR) {
  const std::size_t flavour = JetFlavourIndex(jetRank);
  const float candidatePt = record.fitPt > 0.f ? record.fitPt : record.pt;
  const float jetPt = Jets[jetRank].Pt();

  switch (record.species) {
  case BPHSpecies::D0ToKPi:
    if (record.massHypothesis1 > 0.f)
      d0FitMassKPi_[flavour].Fill(record.massHypothesis1, output_.weight);
    if (candidatePt >= 0.f && jetPt > 0.f)
      d0PtFraction_[flavour].Fill(candidatePt / jetPt, output_.weight);
    d0DeltaRJet_[flavour].Fill(deltaR, output_.weight);
    if (record.lxyError > 0.f)
      d0LxySignificance_[flavour].Fill(record.lxy / record.lxyError,
                                      output_.weight);
    if (record.cosTheta2D >= -1.f && record.cosTheta2D <= 1.f)
      d0CosTheta2D_[flavour].Fill(record.cosTheta2D, output_.weight);
    break;
  case BPHSpecies::DstarToD0Pi:
    if (record.deltaMass > 0.f)
      dstarDeltaMass_[flavour].Fill(record.deltaMass, output_.weight);
    break;
  case BPHSpecies::Lambda:
    if (record.fitMass > 0.f)
      lambdaFitMass_[flavour].Fill(record.fitMass, output_.weight);
    break;
  case BPHSpecies::LambdaCToPKPi:
    if (record.fitMass > 0.f)
      lambdaCFitMass_[flavour].Fill(record.fitMass, output_.weight);
    if (candidatePt >= 0.f && jetPt > 0.f)
      lambdaCPtFraction_[flavour].Fill(candidatePt / jetPt, output_.weight);
    break;
  }
}

void HadronAnalyzer::AppendBPHCandidate(const BPHRecord &record) {
  float deltaR = -999.f;
  const int jetRank =
      FindNearestLeadingJet(record.matchingEta, record.matchingPhi, deltaR);
  if (jetRank < 0)
    return;

  output_.bphJetRank.push_back(jetRank);
  output_.bphSpecies.push_back(static_cast<int>(record.species));
  output_.bphSourceIndex.push_back(record.sourceIndex);
  output_.bphDeltaR.push_back(deltaR);
  output_.bphPt.push_back(record.pt);
  output_.bphEta.push_back(record.eta);
  output_.bphPhi.push_back(record.phi);
  output_.bphMass.push_back(record.mass);
  output_.bphPdgId.push_back(record.pdgId);
  output_.bphCharge.push_back(record.charge);
  output_.bphFitPt.push_back(record.fitPt);
  output_.bphFitEta.push_back(record.fitEta);
  output_.bphFitPhi.push_back(record.fitPhi);
  output_.bphFitMass.push_back(record.fitMass);
  output_.bphMassHypothesis1.push_back(record.massHypothesis1);
  output_.bphMassHypothesis2.push_back(record.massHypothesis2);
  output_.bphMassError1.push_back(record.massError1);
  output_.bphMassError2.push_back(record.massError2);
  output_.bphSvProbability.push_back(record.svProbability);
  output_.bphSvChi2.push_back(record.svChi2);
  output_.bphSvNdof.push_back(record.svNdof);
  output_.bphLxy.push_back(record.lxy);
  output_.bphLxyError.push_back(record.lxyError);
  output_.bphCosTheta2D.push_back(record.cosTheta2D);
  output_.bphDeltaMass.push_back(record.deltaMass);
  output_.bphDaughterIndex1.push_back(record.daughterIndex1);
  output_.bphDaughterIndex2.push_back(record.daughterIndex2);
  output_.bphDaughterIndex3.push_back(record.daughterIndex3);

  switch (record.species) {
  case BPHSpecies::D0ToKPi:
    ++output_.nD0[jetRank];
    break;
  case BPHSpecies::DstarToD0Pi:
    ++output_.nDstar[jetRank];
    break;
  case BPHSpecies::Lambda:
    ++output_.nLambda[jetRank];
    break;
  case BPHSpecies::LambdaCToPKPi:
    ++output_.nLambdaC[jetRank];
    break;
  }
  FillBPHHistograms(record, jetRank, deltaR);
}

void HadronAnalyzer::FillSelectedEvent() {
  output_.clear();
  output_.run = RunNumber.get();
  output_.luminosityBlock = luminosityBlock.get();
  output_.event = event.get();
  output_.channel = static_cast<int>(channel);
  output_.weight = MCNormalization();
  output_.ht = HT;
  output_.metPt = MET.Pt();
  output_.metPhi = MET.Phi();
  output_.leptonPt = lepton.Pt();
  output_.leptonEta = lepton.Eta();
  output_.leptonPhi = lepton.Phi();
  output_.nJet = n_jets;
  output_.nBTaggedJet = n_b_tagged_jets;
  output_.nCTaggedJet = n_c_tagged_jets;

  auto genJetBPH = GetAllGenJetBPHViews();
  auto genHadrons = GetAllGenJetHadronViews();
  auto d0s = GetAllD0ToKPiViews();
  auto dstars = GetAllDstarToD0PiViews();
  auto lambdas = GetAllLambdaViews();
  auto lambdaCs = GetAllLambdaCToPKPiViews();

  std::array<int, kLeadingJets> leadingGenJetIndices;
  leadingGenJetIndices.fill(-1);
  for (std::size_t rank = 0; rank < kLeadingJets; ++rank) {
    const auto jet = Jets[rank];
    output_.jetPt[rank] = jet.Pt();
    output_.jetEta[rank] = jet.Eta();
    output_.jetPhi[rank] = jet.Phi();
    output_.jetMass[rank] = jet.Mass();
    output_.jetOriginalIndex[rank] = jet.OriginalIndex();
    output_.jetHadronFlavour[rank] = jet.HadronFlavour();
    output_.jetPartonFlavour[rank] = jet.PartonFlavour();

    const int genJetIndex = jet.GenJetIdx();
    output_.jetGenJetIndex[rank] = genJetIndex;
    leadingGenJetIndices[rank] = genJetIndex;
    if (genJetIndex < 0 ||
        static_cast<std::size_t>(genJetIndex) >= AllGenJets.size() ||
        static_cast<std::size_t>(genJetIndex) >= genJetBPH.size())
      continue;

    const auto genJet = AllGenJets[genJetIndex];
    const auto bph = genJetBPH[genJetIndex];
    output_.genJetPt[rank] = genJet.Pt();
    output_.genJetEta[rank] = genJet.Eta();
    output_.genJetPhi[rank] = genJet.Phi();
    output_.genJetMass[rank] = genJet.Mass();
    output_.recoGenJetDeltaR[rank] = jet.DeltaR(genJet);
    output_.genJetHadronFlavour[rank] = genJet.HadronFlavour();
    output_.genJetPartonFlavour[rank] = genJet.PartonFlavour();
    output_.genJetNBottomBaryon[rank] = bph.nBottomBaryon();
    output_.genJetNCharmBaryon[rank] = bph.nCharmBaryon();
    output_.genJetND0[rank] = bph.nD0();
    output_.genJetNDHadron[rank] = bph.nDHadron();
    output_.genJetNDplus[rank] = bph.nDplus();
    output_.genJetNDstar[rank] = bph.nDstar();
    output_.genJetNFeeddownD[rank] = bph.nFeeddownD();
    output_.genJetNLambdaB[rank] = bph.nLambdaB();
    output_.genJetNLambdaBaryon[rank] = bph.nLambdaBaryon();
    output_.genJetNLambdaC[rank] = bph.nLambdaC();
    output_.genJetNLightLambda[rank] = bph.nLightLambda();
    output_.genJetNPromptD[rank] = bph.nPromptD();
  }

  for (const auto d0 : d0s) {
    BPHRecord record;
    record.species = BPHSpecies::D0ToKPi;
    record.sourceIndex = static_cast<int>(d0.index());
    record.pt = d0.pt();
    record.eta = d0.eta();
    record.phi = d0.phi();
    record.mass = d0.mass();
    record.pdgId = d0.pdgId();
    record.charge = d0.charge();
    record.fitPt = d0.fit_pt();
    record.fitEta = d0.fit_eta();
    record.fitPhi = d0.fit_phi();
    record.massHypothesis1 = d0.fit_mass_Kpi();
    record.massHypothesis2 = d0.fit_mass_piK();
    record.massError1 = d0.fit_massErr_Kpi();
    record.massError2 = d0.fit_massErr_piK();
    record.svProbability = d0.svprob();
    record.svChi2 = d0.sv_chi2();
    record.svNdof = d0.sv_ndof();
    record.lxy = d0.l_xy();
    record.lxyError = d0.l_xy_unc();
    record.cosTheta2D = d0.cos_theta_2D();
    record.daughterIndex1 = d0.trk1_idx();
    record.daughterIndex2 = d0.trk2_idx();
    record.matchingEta = record.fitEta;
    record.matchingPhi = record.fitPhi;
    AppendBPHCandidate(record);
  }

  for (const auto dstar : dstars) {
    BPHRecord record;
    record.species = BPHSpecies::DstarToD0Pi;
    record.sourceIndex = static_cast<int>(dstar.index());
    record.pt = dstar.pt();
    record.eta = dstar.eta();
    record.phi = dstar.phi();
    record.mass = dstar.mass();
    record.pdgId = dstar.pdgId();
    record.charge = dstar.charge();
    record.massHypothesis1 = dstar.mass_Kpi();
    record.massHypothesis2 = dstar.mass_piK();
    record.deltaMass = dstar.delta_mass();
    record.daughterIndex1 = dstar.d0_idx();
    record.daughterIndex2 = dstar.soft_pi_idx();
    record.matchingEta = record.eta;
    record.matchingPhi = record.phi;
    AppendBPHCandidate(record);
  }

  for (const auto lambda : lambdas) {
    BPHRecord record;
    record.species = BPHSpecies::Lambda;
    record.sourceIndex = static_cast<int>(lambda.index());
    record.pt = lambda.pt();
    record.eta = lambda.eta();
    record.phi = lambda.phi();
    record.mass = lambda.mass();
    record.pdgId = lambda.pdgId();
    record.charge = lambda.charge();
    record.fitPt = lambda.fit_pt();
    record.fitEta = lambda.fit_eta();
    record.fitPhi = lambda.fit_phi();
    record.fitMass = lambda.fit_mass();
    record.massHypothesis1 = lambda.fit_mass();
    record.massError1 = lambda.fit_massErr();
    record.svProbability = lambda.svprob();
    record.svChi2 = lambda.chi2();
    record.lxy = lambda.l_xy();
    record.lxyError = lambda.l_xy_unc();
    record.cosTheta2D = lambda.fit_cos_theta_2D();
    record.daughterIndex1 = lambda.trk1_idx();
    record.daughterIndex2 = lambda.trk2_idx();
    record.matchingEta = record.fitEta;
    record.matchingPhi = record.fitPhi;
    AppendBPHCandidate(record);
  }

  for (const auto lambdaC : lambdaCs) {
    BPHRecord record;
    record.species = BPHSpecies::LambdaCToPKPi;
    record.sourceIndex = static_cast<int>(lambdaC.index());
    record.pt = lambdaC.pt();
    record.eta = lambdaC.eta();
    record.phi = lambdaC.phi();
    record.mass = lambdaC.mass();
    record.pdgId = lambdaC.pdgId();
    record.charge = lambdaC.charge();
    record.fitPt = lambdaC.fit_pt();
    record.fitEta = lambdaC.fit_eta();
    record.fitPhi = lambdaC.fit_phi();
    record.fitMass = lambdaC.fit_mass();
    record.massHypothesis1 = lambdaC.fit_mass();
    record.massError1 = lambdaC.fit_massErr();
    record.svProbability = lambdaC.svprob();
    record.svChi2 = lambdaC.sv_chi2();
    record.svNdof = lambdaC.sv_ndof();
    record.lxy = lambdaC.l_xy();
    record.lxyError = lambdaC.l_xy_unc();
    record.cosTheta2D = lambdaC.cos_theta_2D();
    record.daughterIndex1 = lambdaC.kaon_idx();
    record.daughterIndex2 = lambdaC.pion_idx();
    record.daughterIndex3 = lambdaC.proton_idx();
    record.matchingEta = record.fitEta;
    record.matchingPhi = record.fitPhi;
    AppendBPHCandidate(record);
  }

  for (const auto hadron : genHadrons) {
    int jetRank = -1;
    for (std::size_t rank = 0; rank < kLeadingJets; ++rank) {
      if (leadingGenJetIndices[rank] >= 0 &&
          hadron.genJetIdx() == leadingGenJetIndices[rank]) {
        jetRank = static_cast<int>(rank);
        break;
      }
    }
    if (jetRank < 0)
      continue;

    TLorentzVector hadronP4;
    hadronP4.SetPtEtaPhiM(hadron.pt(), hadron.eta(), hadron.phi(),
                          hadron.mass());
    const float recoJetDeltaR = Jets[jetRank].P4().DeltaR(hadronP4);
    output_.genHadronJetRank.push_back(jetRank);
    output_.genHadronGenJetIndex.push_back(hadron.genJetIdx());
    output_.genHadronSourceIndex.push_back(hadron.srcIdx());
    output_.genHadronPdgId.push_back(hadron.pdgId());
    output_.genHadronMotherPdgId.push_back(hadron.motherPdgId());
    output_.genHadronAncestorPdgId.push_back(hadron.ancestorPdgId());
    output_.genHadronFlavour.push_back(hadron.flavour());
    output_.genHadronStatus.push_back(hadron.status());
    output_.genHadronPt.push_back(hadron.pt());
    output_.genHadronEta.push_back(hadron.eta());
    output_.genHadronPhi.push_back(hadron.phi());
    output_.genHadronMass.push_back(hadron.mass());
    output_.genHadronDeltaR.push_back(hadron.deltaR());
    output_.genHadronRecoJetDeltaR.push_back(recoJetDeltaR);
    output_.genHadronVx.push_back(hadron.vx());
    output_.genHadronVy.push_back(hadron.vy());
    output_.genHadronVz.push_back(hadron.vz());
    output_.genHadronIsBottomBaryon.push_back(hadron.isBottomBaryon());
    output_.genHadronIsCharmBaryon.push_back(hadron.isCharmBaryon());
    output_.genHadronIsD0.push_back(hadron.isD0());
    output_.genHadronIsDHadron.push_back(hadron.isDHadron());
    output_.genHadronIsDplus.push_back(hadron.isDplus());
    output_.genHadronIsDstar.push_back(hadron.isDstar());
    output_.genHadronIsFeeddown.push_back(hadron.isFeeddown());
    output_.genHadronIsFromB.push_back(hadron.isFromB());
    output_.genHadronIsFromC.push_back(hadron.isFromC());
    output_.genHadronIsFromDstar.push_back(hadron.isFromDstar());
    output_.genHadronIsLambdaB.push_back(hadron.isLambdaB());
    output_.genHadronIsLambdaBaryon.push_back(hadron.isLambdaBaryon());
    output_.genHadronIsLambdaC.push_back(hadron.isLambdaC());
    output_.genHadronIsLastCopy.push_back(hadron.isLastCopy());
    output_.genHadronIsLightLambda.push_back(hadron.isLightLambda());
    output_.genHadronIsPrompt.push_back(hadron.isPrompt());
    ++output_.nGenHadron[jetRank];

    const std::size_t flavour = JetFlavourIndex(jetRank);
    if (output_.genJetPt[jetRank] > 0.f)
      genHadronPtFraction_[flavour].Fill(
          hadron.pt() / output_.genJetPt[jetRank], output_.weight);
    genHadronDeltaRRecoJet_[flavour].Fill(recoJetDeltaR, output_.weight);
    genHadronOriginCategory_[flavour].Fill(
        GenOriginCategory(hadron.isPrompt(), hadron.isFromB(),
                          hadron.isFromC(), hadron.isFromDstar(),
                          hadron.isFeeddown()),
        output_.weight);
    genHadronPdgCategory_[flavour].Fill(
        GenPdgCategory(hadron.pdgId(), hadron.isBottomBaryon()),
        output_.weight);
  }

  selectedEvents_.Fill(0.5f, output_.weight);
  for (std::size_t rank = 0; rank < kLeadingJets; ++rank) {
    d0Multiplicity_[rank].Fill(output_.nD0[rank], output_.weight);
    dstarMultiplicity_[rank].Fill(output_.nDstar[rank], output_.weight);
    lambdaMultiplicity_[rank].Fill(output_.nLambda[rank], output_.weight);
    lambdaCMultiplicity_[rank].Fill(output_.nLambdaC[rank], output_.weight);
    genHadronMultiplicity_[rank].Fill(output_.nGenHadron[rank], output_.weight);
    if (output_.jetGenJetIndex[rank] >= 0) {
      recoVsGenD0_[rank].Fill(output_.nD0[rank], output_.genJetND0[rank],
                              output_.weight);
      recoVsGenDstar_[rank].Fill(output_.nDstar[rank],
                                 output_.genJetNDstar[rank], output_.weight);
      recoVsGenLambda_[rank].Fill(output_.nLambda[rank],
                                  output_.genJetNLightLambda[rank],
                                  output_.weight);
      recoVsGenLambdaC_[rank].Fill(output_.nLambdaC[rank],
                                   output_.genJetNLambdaC[rank],
                                   output_.weight);
      const int nReco = output_.nD0[rank] + output_.nDstar[rank] +
                        output_.nLambda[rank] + output_.nLambdaC[rank];
      recoVsGenTotal_[rank].Fill(nReco, output_.nGenHadron[rank],
                                 output_.weight);
    }
  }
  outputTree_.Fill();
}
