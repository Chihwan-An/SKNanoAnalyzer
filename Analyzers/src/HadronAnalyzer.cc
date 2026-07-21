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
constexpr std::array<const char *, 3> kWCharmSpeciesNames = {
    "D0ToKPi", "DstarToD0Pi", "LambdaCToPKPi"};
constexpr std::array<const char *, 2> kWCharmMatchCategoryNames = {
    "Inclusive", "GenMatchedNoBottom"};
constexpr std::array<const char *, HadronAnalysis::kNumRecoWMassBins>
    kRecoWMassBinNames = {"MassLt60",    "Mass60to70", "Mass70to80",
                         "Mass80to90",  "Mass90to100", "MassGe100"};
constexpr std::array<std::array<double, 3>, 6> kLambdaCHypothesisMasses = {{
    {0.9382720813, 0.493677, 0.13957039},
    {0.9382720813, 0.13957039, 0.493677},
    {0.493677, 0.9382720813, 0.13957039},
    {0.13957039, 0.9382720813, 0.493677},
    {0.493677, 0.13957039, 0.9382720813},
    {0.13957039, 0.493677, 0.9382720813},
}};

template <typename LambdaCView, typename Fill>
void ForEachLambdaCHypothesisMass(const LambdaCView &candidate, Fill fill) {
  const std::array<float, 3> pt = {candidate.fit_trk1_pt(),
                                   candidate.fit_trk2_pt(),
                                   candidate.fit_trk3_pt()};
  const std::array<float, 3> eta = {candidate.fit_trk1_eta(),
                                    candidate.fit_trk2_eta(),
                                    candidate.fit_trk3_eta()};
  const std::array<float, 3> phi = {candidate.fit_trk1_phi(),
                                    candidate.fit_trk2_phi(),
                                    candidate.fit_trk3_phi()};
  for (std::size_t hypothesis = 0; hypothesis < kLambdaCHypothesisMasses.size();
       ++hypothesis) {
    if (!(candidate.hypothesisMask() & (1u << hypothesis)))
      continue;
    TLorentzVector daughters[3];
    for (std::size_t daughter = 0; daughter < 3; ++daughter)
      daughters[daughter].SetPtEtaPhiM(
          pt[daughter], eta[daughter], phi[daughter],
          kLambdaCHypothesisMasses[hypothesis][daughter]);
    fill((daughters[0] + daughters[1] + daughters[2]).M());
  }
}

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

void HadronAnalyzer::RegisterTasks() {
  RegisterTask(
      "BPHStudy",
      TaskOptions{true, TaskSystematicPolicy::CentralOnly},
      [this] { ValidateBPHStudy(); }, [this] { BookBPHStudy(); },
      [this] { RunBPHStudy(); });
}

void HadronAnalyzer::ValidateBPHStudy() {
  if (!HasD0ToKPiViews() || !HasDstarToD0PiViews() ||
      !HasGenJetBPHViews() || !HasGenJetHadronViews() ||
      !HasHadronTrackViews() || !HasLambdaViews() ||
      !HasLambdaCToPKPiViews())
    throw SKNano::ConfigError(
        "[HadronAnalyzer] BPHStudy requires the complete CustomBPH schema");
}

void HadronAnalyzer::BookBPHStudy() {
  lambdaCBDTMode_ = HasFlag("LambdaCBDT");
  if (!HasFlag("HadronHistOnly"))
    BookOutputRNTuple();
  BookHistograms();
}

void HadronAnalyzer::RunBPHStudy() { FillSelectedEvent(); }

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

  topJetTopologyStatus = -1;
  topJetHadronicTopPdgId = 0;
  topJetNGenMatched = 0;
  topJetNRecoMatched = 0;
  topJetComplete = 0;
  topJetTotalCost = -1.f;
  topJetAssignmentMargin = -1.f;
  topJetHadronicWRecoMass = -1.f;
  topJetWCharmRole = -1;
  topJetWCharmJetIndex = -1;
  topJetWCharmJetOriginalIndex = -1;
  FillArray(topJetGenPartIndex, -1);
  FillArray(topJetPartonPdgId, 0);
  FillArray(topJetGenJetIndex, -1);
  FillArray(topJetRecoJetIndex, -1);
  FillArray(topJetRecoJetOriginalIndex, -1);
  FillArray(topJetPartonGenJetDeltaR, -1.f);
  FillArray(topJetGenJetRecoDeltaR, -1.f);
  FillArray(topJetCost, -1.f);
  FillArray(topJetMatchSource, 0);
  FillArray(topJetFlavourFallback, 0);

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
      bphInputJetIndex, bphHypothesisMask, bphPrescaleWeight,
      bphGenPartIndex, bphGenMatchCategory, bphGenHypothesisMask,
      bphGenHypothesisIndex, bphDaughterIndex1, bphDaughterIndex2,
      bphDaughterIndex3);
  ClearVectors(
      bphDeltaR, bphPt, bphEta, bphPhi, bphRapidity, bphMass,
      bphInputJetDeltaR, bphWCharmZ, bphWCharmDeltaR, bphFitPt, bphFitEta,
      bphFitPhi, bphFitMass, bphMassHypothesis1, bphMassHypothesis2,
      bphMassError1, bphMassError2, bphSvProbability, bphSvChi2, bphSvNdof,
      bphSvChi2Ndof, bphLxy, bphLxyError, bphLxySignificance,
      bphCosTheta2D, bphPointingAngle2D, bphCandidateDca,
      bphCandidateDcaError, bphCandidateDcaSignificance, bphDaughterDeltaR,
      bphDaughterAbsDz, bphDaughterDca3D, bphDecayLength3D,
      bphDecayLength3DSignificance, bphSoftPionDxyD0Vertex,
      bphSoftPionDzD0Vertex, bphPKMass, bphKPiMass, bphPPiMass,
      bphPKDeltaR, bphKPiDeltaR, bphPPiDeltaR, bphMinPairDeltaR,
      bphMaxPairDeltaR, bphDeltaMass);
  for (std::size_t role = 0; role < 3; ++role) {
    ClearVectors(bphTrackPt[role], bphTrackEta[role], bphTrackPhi[role],
                 bphTrackCharge[role], bphTrackPtFraction[role],
                 bphTrackDxy[role], bphTrackDz[role],
                 bphTrackDxySignificance[role],
                 bphTrackDzSignificance[role],
                 bphTrackDcaSignificance[role], bphTrackPtError[role],
                 bphTrackPtErrorRelative[role],
                 bphTrackNormalizedChi2[role], bphTrackFitPt[role],
                 bphTrackFitPtFraction[role], bphTrackNValidHits[role],
                 bphTrackNValidPixelHits[role]);
  }
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
  RegisterTasks();
  Vcb::initializeAnalyzer();

  InitializeTasks("HadronAll");
}

void HadronAnalyzer::BookOutputRNTuple() {
  outputTree_ = Output().Book("BPHEvents");
  outputTree_.Field("run", output_.run);
  outputTree_.Field("luminosityBlock", output_.luminosityBlock);
  outputTree_.Field("event", output_.event);
  outputTree_.Field("channel", output_.channel);
  outputTree_.Field("weight", output_.weight);
  outputTree_.Field("HT", output_.ht);
  outputTree_.Field("MET_pt", output_.metPt);
  outputTree_.Field("MET_phi", output_.metPhi);
  outputTree_.Field("Lepton_pt", output_.leptonPt);
  outputTree_.Field("Lepton_eta", output_.leptonEta);
  outputTree_.Field("Lepton_phi", output_.leptonPhi);
  outputTree_.Field("nJet", output_.nJet);
  outputTree_.Field("nBTaggedJet", output_.nBTaggedJet);
  outputTree_.Field("nCTaggedJet", output_.nCTaggedJet);

  outputTree_.Field("TopJet_topologyStatus", output_.topJetTopologyStatus);
  outputTree_.Field("TopJet_hadronicTopPdgId",
                     output_.topJetHadronicTopPdgId);
  outputTree_.Field("TopJet_nGenMatched", output_.topJetNGenMatched);
  outputTree_.Field("TopJet_nRecoMatched", output_.topJetNRecoMatched);
  outputTree_.Field("TopJet_complete", output_.topJetComplete);
  outputTree_.Field("TopJet_totalCost", output_.topJetTotalCost);
  outputTree_.Field("TopJet_assignmentMargin",
                     output_.topJetAssignmentMargin);
  outputTree_.Field("TopJet_hadronicWRecoMass",
                     output_.topJetHadronicWRecoMass);
  outputTree_.Field("TopJet_wCharmRole", output_.topJetWCharmRole);
  outputTree_.Field("TopJet_wCharmJetIdx", output_.topJetWCharmJetIndex);
  outputTree_.Field("TopJet_wCharmJetOriginalIdx",
                     output_.topJetWCharmJetOriginalIndex);
  outputTree_.Field("TopJet_genPartIdx", output_.topJetGenPartIndex);
  outputTree_.Field("TopJet_partonPdgId", output_.topJetPartonPdgId);
  outputTree_.Field("TopJet_genJetIdx", output_.topJetGenJetIndex);
  outputTree_.Field("TopJet_recoJetIdx", output_.topJetRecoJetIndex);
  outputTree_.Field("TopJet_recoJetOriginalIdx",
                     output_.topJetRecoJetOriginalIndex);
  outputTree_.Field("TopJet_partonGenJetDeltaR",
                     output_.topJetPartonGenJetDeltaR);
  outputTree_.Field("TopJet_genJetRecoDeltaR",
                     output_.topJetGenJetRecoDeltaR);
  outputTree_.Field("TopJet_cost", output_.topJetCost);
  outputTree_.Field("TopJet_matchSource", output_.topJetMatchSource);
  outputTree_.Field("TopJet_flavourFallback",
                     output_.topJetFlavourFallback);

  outputTree_.Field("Jet_pt", output_.jetPt);
  outputTree_.Field("Jet_eta", output_.jetEta);
  outputTree_.Field("Jet_phi", output_.jetPhi);
  outputTree_.Field("Jet_mass", output_.jetMass);
  outputTree_.Field("Jet_originalIndex", output_.jetOriginalIndex);
  outputTree_.Field("Jet_hadronFlavour", output_.jetHadronFlavour);
  outputTree_.Field("Jet_partonFlavour", output_.jetPartonFlavour);
  outputTree_.Field("Jet_genJetIdx", output_.jetGenJetIndex);
  if (!lambdaCBDTMode_) {
    outputTree_.Field("GenJet_pt", output_.genJetPt);
    outputTree_.Field("GenJet_eta", output_.genJetEta);
    outputTree_.Field("GenJet_phi", output_.genJetPhi);
    outputTree_.Field("GenJet_mass", output_.genJetMass);
    outputTree_.Field("Jet_genJetDeltaR", output_.recoGenJetDeltaR);
    outputTree_.Field("GenJet_hadronFlavour", output_.genJetHadronFlavour);
    outputTree_.Field("GenJet_partonFlavour", output_.genJetPartonFlavour);
    outputTree_.Field("GenJet_nBottomBaryon", output_.genJetNBottomBaryon);
    outputTree_.Field("GenJet_nCharmBaryon", output_.genJetNCharmBaryon);
    outputTree_.Field("GenJet_nD0", output_.genJetND0);
    outputTree_.Field("GenJet_nDHadron", output_.genJetNDHadron);
    outputTree_.Field("GenJet_nDplus", output_.genJetNDplus);
    outputTree_.Field("GenJet_nDstar", output_.genJetNDstar);
    outputTree_.Field("GenJet_nFeeddownD", output_.genJetNFeeddownD);
    outputTree_.Field("GenJet_nLambdaB", output_.genJetNLambdaB);
    outputTree_.Field("GenJet_nLambdaBaryon", output_.genJetNLambdaBaryon);
    outputTree_.Field("GenJet_nLambdaC", output_.genJetNLambdaC);
    outputTree_.Field("GenJet_nLightLambda", output_.genJetNLightLambda);
    outputTree_.Field("GenJet_nPromptD", output_.genJetNPromptD);
  }

  outputTree_.Field("Jet_nD0", output_.nD0);
  outputTree_.Field("Jet_nDstar", output_.nDstar);
  outputTree_.Field("Jet_nLambda", output_.nLambda);
  outputTree_.Field("Jet_nLambdaC", output_.nLambdaC);
  outputTree_.Field("Jet_nGenHadron", output_.nGenHadron);

  outputTree_.Field("BPH_jetRank", output_.bphJetRank);
  outputTree_.Field("BPH_species", output_.bphSpecies);
  outputTree_.Field("BPH_sourceIdx", output_.bphSourceIndex);
  outputTree_.Field("BPH_deltaR", output_.bphDeltaR);
  outputTree_.Field("BPH_pt", output_.bphPt);
  outputTree_.Field("BPH_eta", output_.bphEta);
  outputTree_.Field("BPH_phi", output_.bphPhi);
  outputTree_.Field("BPH_rapidity", output_.bphRapidity);
  outputTree_.Field("BPH_mass", output_.bphMass);
  outputTree_.Field("BPH_pdgId", output_.bphPdgId);
  outputTree_.Field("BPH_charge", output_.bphCharge);
  outputTree_.Field("BPH_inputJetIdx", output_.bphInputJetIndex);
  outputTree_.Field("BPH_inputJetDeltaR", output_.bphInputJetDeltaR);
  outputTree_.Field("BPH_hypothesisMask", output_.bphHypothesisMask);
  outputTree_.Field("BPH_prescaleWeight", output_.bphPrescaleWeight);
  outputTree_.Field("BPH_genPartIdx", output_.bphGenPartIndex);
  outputTree_.Field("BPH_genMatchCategory", output_.bphGenMatchCategory);
  outputTree_.Field("BPH_genHypothesisMask", output_.bphGenHypothesisMask);
  outputTree_.Field("BPH_genHypothesisIdx", output_.bphGenHypothesisIndex);
  outputTree_.Field("BPH_wCharmZ", output_.bphWCharmZ);
  outputTree_.Field("BPH_wCharmDeltaR", output_.bphWCharmDeltaR);
  outputTree_.Field("BPH_fitPt", output_.bphFitPt);
  outputTree_.Field("BPH_fitEta", output_.bphFitEta);
  outputTree_.Field("BPH_fitPhi", output_.bphFitPhi);
  outputTree_.Field("BPH_fitMass", output_.bphFitMass);
  outputTree_.Field("BPH_massHypothesis1", output_.bphMassHypothesis1);
  outputTree_.Field("BPH_massHypothesis2", output_.bphMassHypothesis2);
  outputTree_.Field("BPH_massError1", output_.bphMassError1);
  outputTree_.Field("BPH_massError2", output_.bphMassError2);
  outputTree_.Field("BPH_svProbability", output_.bphSvProbability);
  outputTree_.Field("BPH_svChi2", output_.bphSvChi2);
  outputTree_.Field("BPH_svNdof", output_.bphSvNdof);
  outputTree_.Field("BPH_svChi2Ndof", output_.bphSvChi2Ndof);
  outputTree_.Field("BPH_lxy", output_.bphLxy);
  outputTree_.Field("BPH_lxyError", output_.bphLxyError);
  outputTree_.Field("BPH_lxySignificance", output_.bphLxySignificance);
  outputTree_.Field("BPH_cosTheta2D", output_.bphCosTheta2D);
  outputTree_.Field("BPH_pointingAngle2D", output_.bphPointingAngle2D);
  outputTree_.Field("BPH_candidateDca", output_.bphCandidateDca);
  outputTree_.Field("BPH_candidateDcaError", output_.bphCandidateDcaError);
  outputTree_.Field("BPH_candidateDcaSignificance",
                     output_.bphCandidateDcaSignificance);
  outputTree_.Field("BPH_daughterDeltaR", output_.bphDaughterDeltaR);
  outputTree_.Field("BPH_daughterAbsDz", output_.bphDaughterAbsDz);
  outputTree_.Field("BPH_daughterDca3D", output_.bphDaughterDca3D);
  outputTree_.Field("BPH_decayLength3D", output_.bphDecayLength3D);
  outputTree_.Field("BPH_decayLength3DSignificance",
                     output_.bphDecayLength3DSignificance);
  outputTree_.Field("BPH_softPionDxyD0Vertex",
                     output_.bphSoftPionDxyD0Vertex);
  outputTree_.Field("BPH_softPionDzD0Vertex",
                     output_.bphSoftPionDzD0Vertex);
  outputTree_.Field("BPH_pKMass", output_.bphPKMass);
  outputTree_.Field("BPH_KpiMass", output_.bphKPiMass);
  outputTree_.Field("BPH_ppiMass", output_.bphPPiMass);
  outputTree_.Field("BPH_pKDeltaR", output_.bphPKDeltaR);
  outputTree_.Field("BPH_KpiDeltaR", output_.bphKPiDeltaR);
  outputTree_.Field("BPH_ppiDeltaR", output_.bphPPiDeltaR);
  outputTree_.Field("BPH_minPairDeltaR", output_.bphMinPairDeltaR);
  outputTree_.Field("BPH_maxPairDeltaR", output_.bphMaxPairDeltaR);
  constexpr std::array<const char *, 3> trackRoles = {"daughter1", "daughter2",
                                                      "daughter3"};
  for (std::size_t role = 0; role < trackRoles.size(); ++role) {
    const std::string prefix = "BPH_" + std::string(trackRoles[role]);
    outputTree_.Field(prefix + "Pt", output_.bphTrackPt[role]);
    outputTree_.Field(prefix + "Eta", output_.bphTrackEta[role]);
    outputTree_.Field(prefix + "Phi", output_.bphTrackPhi[role]);
    outputTree_.Field(prefix + "Charge", output_.bphTrackCharge[role]);
    outputTree_.Field(prefix + "PtFraction",
                       output_.bphTrackPtFraction[role]);
    outputTree_.Field(prefix + "Dxy", output_.bphTrackDxy[role]);
    outputTree_.Field(prefix + "Dz", output_.bphTrackDz[role]);
    outputTree_.Field(prefix + "DxySignificance",
                       output_.bphTrackDxySignificance[role]);
    outputTree_.Field(prefix + "DzSignificance",
                       output_.bphTrackDzSignificance[role]);
    outputTree_.Field(prefix + "DcaSignificance",
                       output_.bphTrackDcaSignificance[role]);
    outputTree_.Field(prefix + "PtError", output_.bphTrackPtError[role]);
    outputTree_.Field(prefix + "PtErrorRelative",
                       output_.bphTrackPtErrorRelative[role]);
    outputTree_.Field(prefix + "NormalizedChi2",
                       output_.bphTrackNormalizedChi2[role]);
    outputTree_.Field(prefix + "FitPt", output_.bphTrackFitPt[role]);
    outputTree_.Field(prefix + "FitPtFraction",
                       output_.bphTrackFitPtFraction[role]);
    outputTree_.Field(prefix + "NValidHits",
                       output_.bphTrackNValidHits[role]);
    outputTree_.Field(prefix + "NValidPixelHits",
                       output_.bphTrackNValidPixelHits[role]);
  }
  outputTree_.Field("BPH_deltaMass", output_.bphDeltaMass);
  outputTree_.Field("BPH_daughterIdx1", output_.bphDaughterIndex1);
  outputTree_.Field("BPH_daughterIdx2", output_.bphDaughterIndex2);
  outputTree_.Field("BPH_daughterIdx3", output_.bphDaughterIndex3);

  if (!lambdaCBDTMode_) {
    outputTree_.Field("GenHadron_jetRank", output_.genHadronJetRank);
    outputTree_.Field("GenHadron_genJetIdx", output_.genHadronGenJetIndex);
    outputTree_.Field("GenHadron_srcIdx", output_.genHadronSourceIndex);
    outputTree_.Field("GenHadron_pdgId", output_.genHadronPdgId);
    outputTree_.Field("GenHadron_motherPdgId", output_.genHadronMotherPdgId);
    outputTree_.Field("GenHadron_ancestorPdgId", output_.genHadronAncestorPdgId);
    outputTree_.Field("GenHadron_flavour", output_.genHadronFlavour);
    outputTree_.Field("GenHadron_status", output_.genHadronStatus);
    outputTree_.Field("GenHadron_pt", output_.genHadronPt);
    outputTree_.Field("GenHadron_eta", output_.genHadronEta);
    outputTree_.Field("GenHadron_phi", output_.genHadronPhi);
    outputTree_.Field("GenHadron_mass", output_.genHadronMass);
    outputTree_.Field("GenHadron_deltaR", output_.genHadronDeltaR);
    outputTree_.Field("GenHadron_recoJetDeltaR", output_.genHadronRecoJetDeltaR);
    outputTree_.Field("GenHadron_vx", output_.genHadronVx);
    outputTree_.Field("GenHadron_vy", output_.genHadronVy);
    outputTree_.Field("GenHadron_vz", output_.genHadronVz);
    outputTree_.Field("GenHadron_isBottomBaryon", output_.genHadronIsBottomBaryon);
    outputTree_.Field("GenHadron_isCharmBaryon", output_.genHadronIsCharmBaryon);
    outputTree_.Field("GenHadron_isD0", output_.genHadronIsD0);
    outputTree_.Field("GenHadron_isDHadron", output_.genHadronIsDHadron);
    outputTree_.Field("GenHadron_isDplus", output_.genHadronIsDplus);
    outputTree_.Field("GenHadron_isDstar", output_.genHadronIsDstar);
    outputTree_.Field("GenHadron_isFeeddown", output_.genHadronIsFeeddown);
    outputTree_.Field("GenHadron_isFromB", output_.genHadronIsFromB);
    outputTree_.Field("GenHadron_isFromC", output_.genHadronIsFromC);
    outputTree_.Field("GenHadron_isFromDstar", output_.genHadronIsFromDstar);
    outputTree_.Field("GenHadron_isLambdaB", output_.genHadronIsLambdaB);
    outputTree_.Field("GenHadron_isLambdaBaryon", output_.genHadronIsLambdaBaryon);
    outputTree_.Field("GenHadron_isLambdaC", output_.genHadronIsLambdaC);
    outputTree_.Field("GenHadron_isLastCopy", output_.genHadronIsLastCopy);
    outputTree_.Field("GenHadron_isLightLambda", output_.genHadronIsLightLambda);
    outputTree_.Field("GenHadron_isPrompt", output_.genHadronIsPrompt);
  }
}

void HadronAnalyzer::BookHistograms() {
  const auto histograms = Hists("HadronAnalyzer");
  selectedEvents_ = histograms.Book1D("SelectedEvents", 1, 0., 1.);
  const auto validation = histograms.Group("Validation");
  hadronTrackMultiplicity_ = validation.Book1D("NHadronTrack", 80, 0., 80.);
  d0InclusiveMultiplicity_ = validation.Book1D("ND0", 30, 0., 30.);
  dstarInclusiveMultiplicity_ = validation.Book1D("NDstar", 15, 0., 15.);
  lambdaInclusiveMultiplicity_ = validation.Book1D("NLambda", 30, 0., 30.);
  lambdaCInclusiveMultiplicity_ = validation.Book1D("NLambdaC", 30, 0., 30.);
  hadronTrackPt_ = validation.Book1D("HadronTrackPt", 80, 0., 40.);
  hadronTrackPtErrorRelative_ =
      validation.Book1D("HadronTrackPtErrorRelative", 80, 0., 0.2);
  validationD0Mass_ = validation.Book1D("D0Mass", 80, 1.70, 2.02);
  validationDstarDeltaMass_ =
      validation.Book1D("DstarDeltaMass", 70, 0.135, 0.170);
  validationLambdaMass_ =
      validation.Book1D("LambdaMass", 80, 1.08, 1.17);
  validationLambdaCMass_ =
      validation.Book1D("LambdaCMass", 80, 2.13, 2.45);
  validationLambdaCAcceptedHypothesisMass_ =
      validation.Book1D("LambdaCAcceptedHypothesisMass", 64, 2.13, 2.45);
  validationLambdaCGenMatchedHypothesisMass_ = validation.Book1D(
      "LambdaCGenMatchedHypothesisMass", 64, 2.13, 2.45);

  const auto fragmentation = histograms.Group("WCharmFragmentation");
  wCharmRecoMass_ = fragmentation.Book1D("RecoWMass", 80, 0., 200.);
  wCharmJetPt_ = fragmentation.Book1D("CharmJetPt", 80, 0., 400.);
  wCharmRecoMass_.get()->GetXaxis()->SetTitle("m(j_{W,1},j_{W,2}) [GeV]");
  wCharmJetPt_.get()->GetXaxis()->SetTitle("p_{T}(W charm jet) [GeV]");
  for (std::size_t species = 0; species < kWCharmSpecies; ++species) {
    const auto speciesGroup =
        fragmentation.Group(kWCharmSpeciesNames[species]);
    for (std::size_t category = 0; category < kWCharmMatchCategories;
         ++category) {
      const auto categoryGroup =
          speciesGroup.Group(kWCharmMatchCategoryNames[category]);
      wCharmZVsRecoMass_[species][category] = categoryGroup.Book2D(
          "ZVsRecoWMass", 80, 0., 200., 75, 0., 1.5);
      wCharmZVsRecoMass_[species][category]
          .get()
          ->GetXaxis()
          ->SetTitle("m(j_{W,1},j_{W,2}) [GeV]");
      wCharmZVsRecoMass_[species][category]
          .get()
          ->GetYaxis()
          ->SetTitle("z_{||}(H,j_{c})");
      const auto zGroup = categoryGroup.Group("ZByRecoWMass");
      for (std::size_t massBin = 0;
           massBin < HadronAnalysis::kNumRecoWMassBins; ++massBin) {
        wCharmZByRecoMass_[species][category][massBin] =
            zGroup.Book1D(kRecoWMassBinNames[massBin], 75, 0., 1.5);
        wCharmZByRecoMass_[species][category][massBin]
            .get()
            ->GetXaxis()
            ->SetTitle("z_{||}(H,j_{c})");
      }
    }
  }

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
    const bool isCentralSystematic =
        systHelper->getCurrentIterSysTarget().find("Central") !=
        std::string::npos;
    if (!HasTasksForSystematic(isCentralSystematic))
      continue;

    leptons.clear();
    UpdateActiveOtLutForCurrentSystematic();
    const bool removeFlavourTagCut = HasFlag("RemoveFlavTagCut");
    if (!PassBaseLineSelection(removeFlavourTagCut, false))
      continue;
    if (Jets.size() < kLeadingJets)
      continue;

    RunTasks(isCentralSystematic);
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

void HadronAnalyzer::FillWCharmFragmentation(const BPHRecord &record,
                                             int jetRank, float &z,
                                             float &deltaR) {
  z = -999.f;
  deltaR = -999.f;
  int species = -1;
  switch (record.species) {
  case BPHSpecies::D0ToKPi:
    species = 0;
    break;
  case BPHSpecies::DstarToD0Pi:
    species = 1;
    break;
  case BPHSpecies::LambdaCToPKPi:
    species = 2;
    break;
  case BPHSpecies::Lambda:
    return;
  }

  const int charmJetIndex = output_.topJetWCharmJetIndex;
  if (species < 0 || charmJetIndex < 0 ||
      static_cast<std::size_t>(charmJetIndex) >= Jets.size() ||
      jetRank != charmJetIndex || output_.topJetHadronicWRecoMass < 0.f)
    return;

  const auto charmJet = Jets[charmJetIndex];
  if (record.inputJetIndex >= 0 &&
      record.inputJetIndex != charmJet.OriginalIndex())
    return;

  const bool hasFitKinematics = record.fitPt > 0.f;
  const HadronAnalysis::FragmentationKinematics hadron{
      hasFitKinematics ? record.fitPt : record.pt,
      hasFitKinematics ? record.fitEta : record.eta,
      hasFitKinematics ? record.fitPhi : record.phi,
      record.fitMass > 0.f ? record.fitMass : record.mass};
  const HadronAnalysis::FragmentationKinematics jet{
      charmJet.Pt(), charmJet.Eta(), charmJet.Phi(), charmJet.Mass()};
  TLorentzVector hadronP4;
  hadronP4.SetPtEtaPhiM(hadron.pt, hadron.eta, hadron.phi, hadron.mass);
  deltaR = charmJet.P4().DeltaR(hadronP4);
  if (!(deltaR < kBPHJetRadius)) {
    deltaR = -999.f;
    return;
  }

  z = HadronAnalysis::LongitudinalMomentumFraction(hadron, jet);
  if (!(z >= 0.f)) {
    z = -999.f;
    return;
  }
  const int massBin =
      HadronAnalysis::RecoWMassBin(output_.topJetHadronicWRecoMass);
  if (massBin < 0)
    return;

  const float candidateWeight =
      output_.weight * static_cast<float>(std::max(1, record.prescaleWeight));
  auto fillCategory = [&](std::size_t category) {
    wCharmZVsRecoMass_[species][category].Fill(
        output_.topJetHadronicWRecoMass, z, candidateWeight);
    wCharmZByRecoMass_[species][category][massBin].Fill(z, candidateWeight);
  };
  fillCategory(0);
  if (record.genMatchCategory == 1)
    fillCategory(1);
}

void HadronAnalyzer::AppendBPHCandidate(const BPHRecord &record) {
  float deltaR = -999.f;
  const int jetRank =
      FindNearestLeadingJet(record.matchingEta, record.matchingPhi, deltaR);
  if (jetRank < 0)
    return;

  float wCharmZ = -999.f;
  float wCharmDeltaR = -999.f;
  FillWCharmFragmentation(record, jetRank, wCharmZ, wCharmDeltaR);

  if (StoreBPHCandidate(record)) {
    output_.bphJetRank.push_back(jetRank);
    output_.bphSpecies.push_back(static_cast<int>(record.species));
    output_.bphSourceIndex.push_back(record.sourceIndex);
    output_.bphDeltaR.push_back(deltaR);
    output_.bphPt.push_back(record.pt);
    output_.bphEta.push_back(record.eta);
    output_.bphPhi.push_back(record.phi);
    output_.bphRapidity.push_back(record.rapidity);
    output_.bphMass.push_back(record.mass);
    output_.bphPdgId.push_back(record.pdgId);
    output_.bphCharge.push_back(record.charge);
    output_.bphInputJetIndex.push_back(record.inputJetIndex);
    output_.bphInputJetDeltaR.push_back(record.inputJetDeltaR);
    output_.bphHypothesisMask.push_back(record.hypothesisMask);
    const int outputPrescale =
        lambdaCBDTMode_ && record.genMatchCategory == 0
            ? record.prescaleWeight * kLambdaCBDTBackgroundPrescale
            : record.prescaleWeight;
    output_.bphPrescaleWeight.push_back(outputPrescale);
    output_.bphGenPartIndex.push_back(record.genPartIndex);
    output_.bphGenMatchCategory.push_back(record.genMatchCategory);
    output_.bphGenHypothesisMask.push_back(record.genHypothesisMask);
    output_.bphGenHypothesisIndex.push_back(record.genHypothesisIndex);
    output_.bphWCharmZ.push_back(wCharmZ);
    output_.bphWCharmDeltaR.push_back(wCharmDeltaR);
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
    output_.bphSvChi2Ndof.push_back(record.svChi2Ndof);
    output_.bphLxy.push_back(record.lxy);
    output_.bphLxyError.push_back(record.lxyError);
    output_.bphLxySignificance.push_back(record.lxySignificance);
    output_.bphCosTheta2D.push_back(record.cosTheta2D);
    output_.bphPointingAngle2D.push_back(record.pointingAngle2D);
    output_.bphCandidateDca.push_back(record.candidateDca);
    output_.bphCandidateDcaError.push_back(record.candidateDcaError);
    output_.bphCandidateDcaSignificance.push_back(
        record.candidateDcaSignificance);
    output_.bphDaughterDeltaR.push_back(record.daughterDeltaR);
    output_.bphDaughterAbsDz.push_back(record.daughterAbsDz);
    output_.bphDaughterDca3D.push_back(record.daughterDca3D);
    output_.bphDecayLength3D.push_back(record.decayLength3D);
    output_.bphDecayLength3DSignificance.push_back(
        record.decayLength3DSignificance);
    output_.bphSoftPionDxyD0Vertex.push_back(record.softPionDxyD0Vertex);
    output_.bphSoftPionDzD0Vertex.push_back(record.softPionDzD0Vertex);
    output_.bphPKMass.push_back(record.pKMass);
    output_.bphKPiMass.push_back(record.kPiMass);
    output_.bphPPiMass.push_back(record.pPiMass);
    output_.bphPKDeltaR.push_back(record.pKDeltaR);
    output_.bphKPiDeltaR.push_back(record.kPiDeltaR);
    output_.bphPPiDeltaR.push_back(record.pPiDeltaR);
    output_.bphMinPairDeltaR.push_back(record.minPairDeltaR);
    output_.bphMaxPairDeltaR.push_back(record.maxPairDeltaR);
    for (std::size_t role = 0; role < record.trackFeatures.size(); ++role) {
      const auto &track = record.trackFeatures[role];
      output_.bphTrackPt[role].push_back(track.pt);
      output_.bphTrackEta[role].push_back(track.eta);
      output_.bphTrackPhi[role].push_back(track.phi);
      output_.bphTrackCharge[role].push_back(track.charge);
      output_.bphTrackPtFraction[role].push_back(track.ptFraction);
      output_.bphTrackDxy[role].push_back(track.dxy);
      output_.bphTrackDz[role].push_back(track.dz);
      output_.bphTrackDxySignificance[role].push_back(track.dxySignificance);
      output_.bphTrackDzSignificance[role].push_back(track.dzSignificance);
      output_.bphTrackDcaSignificance[role].push_back(track.dcaSignificance);
      output_.bphTrackPtError[role].push_back(track.ptError);
      output_.bphTrackPtErrorRelative[role].push_back(track.ptErrorRelative);
      output_.bphTrackNormalizedChi2[role].push_back(track.normalizedChi2);
      output_.bphTrackFitPt[role].push_back(track.fitPt);
      output_.bphTrackFitPtFraction[role].push_back(track.fitPtFraction);
      output_.bphTrackNValidHits[role].push_back(track.nValidHits);
      output_.bphTrackNValidPixelHits[role].push_back(track.nValidPixelHits);
    }
    output_.bphDeltaMass.push_back(record.deltaMass);
    output_.bphDaughterIndex1.push_back(record.daughterIndex1);
    output_.bphDaughterIndex2.push_back(record.daughterIndex2);
    output_.bphDaughterIndex3.push_back(record.daughterIndex3);
  }

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

bool HadronAnalyzer::StoreBPHCandidate(const BPHRecord &record) const {
  if (!lambdaCBDTMode_)
    return true;
  if (record.species != BPHSpecies::LambdaCToPKPi)
    return false;
  if (record.genMatchCategory > 0)
    return true;

  // SplitMix64 gives a stable event/candidate hash, so repeated jobs and
  // different shard layouts retain the same unmatched training candidates.
  std::uint64_t hash = output_.event;
  hash ^= static_cast<std::uint64_t>(output_.run) << 32;
  hash ^= static_cast<std::uint64_t>(output_.luminosityBlock) << 16;
  hash ^= static_cast<std::uint64_t>(
      static_cast<std::uint32_t>(record.sourceIndex));
  hash += 0x9e3779b97f4a7c15ULL;
  hash = (hash ^ (hash >> 30)) * 0xbf58476d1ce4e5b9ULL;
  hash = (hash ^ (hash >> 27)) * 0x94d049bb133111ebULL;
  hash ^= hash >> 31;
  return hash % kLambdaCBDTBackgroundPrescale == 0;
}

void HadronAnalyzer::FillTopJetMatches() {
  const auto genViews = GetAllGenViews();
  std::vector<HadronAnalysis::GenParticleInput> genParticles;
  genParticles.reserve(genViews.size());
  for (const auto particle : genViews) {
    genParticles.push_back({particle.Pt(), particle.Eta(), particle.Phi(),
                            particle.Mass(), particle.PdgId(),
                            particle.MotherIndex(), particle.Status(),
                            particle.StatusFlags()});
  }

  std::vector<HadronAnalysis::GenJetInput> genJets;
  genJets.reserve(AllGenJets.size());
  for (const auto jet : AllGenJets) {
    genJets.push_back({jet.Pt(), jet.Eta(), jet.Phi(), jet.Mass(),
                       jet.PartonFlavour()});
  }

  std::vector<HadronAnalysis::RecoJetInput> recoJets;
  recoJets.reserve(Jets.size());
  for (std::size_t index = 0; index < Jets.size(); ++index) {
    const auto jet = Jets[index];
    recoJets.push_back(
        {jet.Pt(), jet.Eta(), jet.Phi(), jet.Mass(),
         static_cast<int>(index), jet.OriginalIndex(), jet.GenJetIdx()});
  }

  const auto result = topJetMatcher_.Match(genParticles, genJets, recoJets);
  output_.topJetTopologyStatus = static_cast<int>(result.topologyStatus);
  output_.topJetHadronicTopPdgId = result.hadronicTopPdgId;
  output_.topJetNGenMatched = result.nGenMatched;
  output_.topJetNRecoMatched = result.nRecoMatched;
  output_.topJetComplete = result.complete;
  output_.topJetTotalCost = result.totalCost;
  output_.topJetAssignmentMargin = result.assignmentMargin;
  for (std::size_t role = 0; role < result.matches.size(); ++role) {
    const auto &match = result.matches[role];
    output_.topJetGenPartIndex[role] = match.genPartIndex;
    output_.topJetPartonPdgId[role] = match.partonPdgId;
    output_.topJetGenJetIndex[role] = match.genJetIndex;
    output_.topJetRecoJetIndex[role] = match.recoJetIndex;
    output_.topJetRecoJetOriginalIndex[role] = match.recoJetOriginalIndex;
    output_.topJetPartonGenJetDeltaR[role] = match.partonGenJetDeltaR;
    output_.topJetGenJetRecoDeltaR[role] = match.genJetRecoDeltaR;
    output_.topJetCost[role] = match.cost;
    output_.topJetMatchSource[role] =
        static_cast<int>(match.recoMatchSource);
    output_.topJetFlavourFallback[role] =
        match.usedAbsoluteFlavourFallback;
  }

  constexpr std::size_t upTypeRole = static_cast<std::size_t>(
      HadronAnalysis::TopJetRole::HadronicWUpType);
  constexpr std::size_t downTypeRole = static_cast<std::size_t>(
      HadronAnalysis::TopJetRole::HadronicWDownType);
  const int upTypeJetIndex = output_.topJetRecoJetIndex[upTypeRole];
  const int downTypeJetIndex = output_.topJetRecoJetIndex[downTypeRole];
  if (upTypeJetIndex < 0 || downTypeJetIndex < 0 ||
      upTypeJetIndex == downTypeJetIndex ||
      static_cast<std::size_t>(upTypeJetIndex) >= Jets.size() ||
      static_cast<std::size_t>(downTypeJetIndex) >= Jets.size())
    return;

  const auto upTypeJet = Jets[upTypeJetIndex];
  const auto downTypeJet = Jets[downTypeJetIndex];
  output_.topJetHadronicWRecoMass = HadronAnalysis::InvariantMass(
      {upTypeJet.Pt(), upTypeJet.Eta(), upTypeJet.Phi(), upTypeJet.Mass()},
      {downTypeJet.Pt(), downTypeJet.Eta(), downTypeJet.Phi(),
       downTypeJet.Mass()});

  int charmRole = -1;
  if (std::abs(output_.topJetPartonPdgId[upTypeRole]) == 4)
    charmRole = static_cast<int>(upTypeRole);
  else if (std::abs(output_.topJetPartonPdgId[downTypeRole]) == 4)
    charmRole = static_cast<int>(downTypeRole);
  if (charmRole < 0)
    return;

  output_.topJetWCharmRole = charmRole;
  output_.topJetWCharmJetIndex = output_.topJetRecoJetIndex[charmRole];
  output_.topJetWCharmJetOriginalIndex =
      output_.topJetRecoJetOriginalIndex[charmRole];
  const auto charmJet = Jets[output_.topJetWCharmJetIndex];
  wCharmRecoMass_.Fill(output_.topJetHadronicWRecoMass, output_.weight);
  wCharmJetPt_.Fill(charmJet.Pt(), output_.weight);
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
  FillTopJetMatches();

  auto genJetBPH = GetAllGenJetBPHViews();
  auto genHadrons = GetAllGenJetHadronViews();
  auto hadronTracks = GetAllHadronTrackViews();
  auto d0s = GetAllD0ToKPiViews();
  auto dstars = GetAllDstarToD0PiViews();
  auto lambdas = GetAllLambdaViews();
  auto lambdaCs = GetAllLambdaCToPKPiViews();

  std::vector<HadronAnalysis::LambdaCGenParticleInput> lambdaCGenParticles;
  bool needsLambdaCTruth = false;
  for (const auto lambdaC : lambdaCs) {
    if (lambdaC.genMatchCategory() > 0) {
      needsLambdaCTruth = true;
      break;
    }
  }
  if (needsLambdaCTruth) {
    const auto genViews = GetAllGenViews();
    lambdaCGenParticles.reserve(genViews.size());
    for (const auto particle : genViews) {
      lambdaCGenParticles.push_back(
          {particle.Pt(), particle.Eta(), particle.Phi(), particle.PdgId(),
           particle.MotherIndex(), particle.Status()});
    }
  }

  auto fillTrackFeatures = [&](BPHTrackFeatures &track, int index,
                               float candidatePt, float fitPt) {
    if (index < 0 || static_cast<std::size_t>(index) >= hadronTracks.size())
      throw SKNano::ConfigError(
          "[HadronAnalyzer] candidate has an out-of-range HadronTrack index");
    const auto source = hadronTracks[index];
    track.pt = source.pt();
    track.eta = source.eta();
    track.phi = source.phi();
    track.charge = source.charge();
    track.ptFraction = candidatePt > 0.f ? source.pt() / candidatePt : -999.f;
    track.dxy = source.dxy();
    track.dz = source.dz();
    track.dxySignificance = source.dxySig();
    track.dzSignificance = source.dzSig();
    track.dcaSignificance = source.dcaSig();
    track.ptError = source.ptErr();
    track.ptErrorRelative =
        source.pt() > 0.f ? source.ptErr() / source.pt() : -999.f;
    track.normalizedChi2 = source.normChi2();
    track.fitPt = fitPt;
    track.fitPtFraction = candidatePt > 0.f ? fitPt / candidatePt : -999.f;
    track.nValidHits = source.nValidHits();
    track.nValidPixelHits = source.nValidPixelHits();
    track.sourceType = source.sourceType();
    track.sourceIndex = source.sourceIdx();
    track.hasTrackDetails = source.hasTrackDetails();
    track.highPurity = source.highPurity();
    track.passVertexSelection = source.passVertexSelection();
    track.passSoftPionSelection = source.passSoftPionSelection();
  };

  auto rapidity = [](float pt, float eta, float phi, float mass) {
    TLorentzVector p4;
    p4.SetPtEtaPhiM(pt, eta, phi, mass);
    return static_cast<float>(p4.Rapidity());
  };
  auto pointingAngle = [](float cosine) {
    return std::acos(std::clamp(cosine, -1.f, 1.f));
  };
  auto daughterDeltaR = [](const BPHTrackFeatures &first,
                           const BPHTrackFeatures &second) {
    TLorentzVector firstP4;
    TLorentzVector secondP4;
    firstP4.SetPtEtaPhiM(first.pt, first.eta, first.phi, 0.f);
    secondP4.SetPtEtaPhiM(second.pt, second.eta, second.phi, 0.f);
    return static_cast<float>(firstP4.DeltaR(secondP4));
  };

  hadronTrackMultiplicity_.Fill(hadronTracks.size(), output_.weight);
  d0InclusiveMultiplicity_.Fill(d0s.size(), output_.weight);
  dstarInclusiveMultiplicity_.Fill(dstars.size(), output_.weight);
  lambdaInclusiveMultiplicity_.Fill(lambdas.size(), output_.weight);
  lambdaCInclusiveMultiplicity_.Fill(lambdaCs.size(), output_.weight);
  for (const auto track : hadronTracks) {
    hadronTrackPt_.Fill(track.pt(), output_.weight);
    if (track.pt() > 0.f)
      hadronTrackPtErrorRelative_.Fill(track.ptErr() / track.pt(),
                                       output_.weight);
  }

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
    record.rapidity = rapidity(d0.pt(), d0.eta(), d0.phi(), d0.mass());
    record.mass = d0.mass();
    record.pdgId = 421;
    record.charge = d0.charge();
    record.inputJetIndex = d0.jetIdx();
    record.inputJetDeltaR = d0.jetDeltaR();
    record.hypothesisMask = d0.hypothesisMask();
    record.prescaleWeight = d0.prescaleWeight();
    record.genPartIndex = d0.genPartIdx();
    record.genMatchCategory = d0.genMatchCategory();
    record.fitPt = d0.pt();
    record.fitEta = d0.eta();
    record.fitPhi = d0.phi();
    record.fitMass = d0.mass();
    record.massHypothesis1 = d0.mass();
    record.svProbability = d0.svprob();
    record.svChi2 = d0.sv_chi2();
    record.svNdof = d0.sv_ndof();
    record.svChi2Ndof =
        d0.sv_ndof() > 0.f ? d0.sv_chi2() / d0.sv_ndof() : -999.f;
    record.lxy = d0.l_xy();
    record.lxySignificance = d0.l_xy_sig();
    record.lxyError = d0.l_xy_sig() > 0.f ? d0.l_xy() / d0.l_xy_sig()
                                               : -999.f;
    record.cosTheta2D = d0.cos_theta_2D();
    record.pointingAngle2D = pointingAngle(d0.cos_theta_2D());
    record.daughterDca3D = d0.daughter_dca_3D();
    record.decayLength3D = d0.decay_length_3D();
    record.decayLength3DSignificance = d0.decay_length_3D_sig();
    record.daughterIndex1 = d0.trk1Idx();
    record.daughterIndex2 = d0.trk2Idx();
    fillTrackFeatures(record.trackFeatures[0], record.daughterIndex1,
                      record.pt, d0.fit_trk1_pt());
    fillTrackFeatures(record.trackFeatures[1], record.daughterIndex2,
                      record.pt, d0.fit_trk2_pt());
    record.daughterDeltaR =
        daughterDeltaR(record.trackFeatures[0], record.trackFeatures[1]);
    record.daughterAbsDz = std::abs(record.trackFeatures[0].dz -
                                    record.trackFeatures[1].dz);
    record.matchingEta = record.fitEta;
    record.matchingPhi = record.fitPhi;
    validationD0Mass_.Fill(record.mass, output_.weight);
    AppendBPHCandidate(record);
  }

  for (const auto dstar : dstars) {
    BPHRecord record;
    record.species = BPHSpecies::DstarToD0Pi;
    record.sourceIndex = static_cast<int>(dstar.index());
    record.pt = dstar.pt();
    record.eta = dstar.eta();
    record.phi = dstar.phi();
    record.rapidity =
        rapidity(dstar.pt(), dstar.eta(), dstar.phi(), dstar.mass());
    record.mass = dstar.mass();
    record.pdgId = dstar.charge() < 0 ? -413 : 413;
    record.charge = dstar.charge();
    record.inputJetIndex = dstar.jetIdx();
    record.inputJetDeltaR = dstar.jetDeltaR();
    record.hypothesisMask = dstar.hypothesisMask();
    record.genPartIndex = dstar.genPartIdx();
    record.genMatchCategory = dstar.genMatchCategory();
    record.massHypothesis1 = dstar.mass();
    record.deltaMass = dstar.deltaMass();
    record.softPionDxyD0Vertex = dstar.soft_pi_dxy_d0_vtx();
    record.softPionDzD0Vertex = dstar.soft_pi_dz_d0_vtx();
    record.daughterIndex1 = dstar.d0Idx();
    record.daughterIndex2 = dstar.softPiIdx();
    if (record.daughterIndex1 < 0 ||
        static_cast<std::size_t>(record.daughterIndex1) >= d0s.size())
      throw SKNano::ConfigError(
          "[HadronAnalyzer] Dstar has an out-of-range D0 index");
    const auto d0 = d0s[record.daughterIndex1];
    record.svProbability = d0.svprob();
    record.svChi2 = d0.sv_chi2();
    record.svNdof = d0.sv_ndof();
    record.svChi2Ndof =
        d0.sv_ndof() > 0.f ? d0.sv_chi2() / d0.sv_ndof() : -999.f;
    record.lxy = d0.l_xy();
    record.lxySignificance = d0.l_xy_sig();
    record.lxyError = d0.l_xy_sig() > 0.f ? d0.l_xy() / d0.l_xy_sig()
                                               : -999.f;
    record.cosTheta2D = d0.cos_theta_2D();
    record.pointingAngle2D = pointingAngle(d0.cos_theta_2D());
    fillTrackFeatures(record.trackFeatures[2], record.daughterIndex2,
                      record.pt, -999.f);
    record.matchingEta = record.eta;
    record.matchingPhi = record.phi;
    validationDstarDeltaMass_.Fill(record.deltaMass, output_.weight);
    AppendBPHCandidate(record);
  }

  for (const auto lambda : lambdas) {
    BPHRecord record;
    record.species = BPHSpecies::Lambda;
    record.sourceIndex = static_cast<int>(lambda.index());
    record.pt = lambda.pt();
    record.eta = lambda.eta();
    record.phi = lambda.phi();
    record.rapidity =
        rapidity(lambda.pt(), lambda.eta(), lambda.phi(), lambda.mass());
    record.mass = lambda.mass();
    record.pdgId = 3122;
    record.charge = lambda.charge();
    record.inputJetIndex = lambda.jetIdx();
    record.inputJetDeltaR = lambda.jetDeltaR();
    record.genPartIndex = lambda.genPartIdx();
    record.genMatchCategory = lambda.genMatchCategory();
    record.fitPt = lambda.pt();
    record.fitEta = lambda.eta();
    record.fitPhi = lambda.phi();
    record.fitMass = lambda.mass();
    record.massHypothesis1 = lambda.mass();
    record.svProbability = lambda.svprob();
    record.svChi2 = lambda.sv_chi2();
    record.svNdof = lambda.sv_ndof();
    record.svChi2Ndof = lambda.sv_ndof() > 0.f
                            ? lambda.sv_chi2() / lambda.sv_ndof()
                            : -999.f;
    record.lxy = lambda.l_xy();
    record.lxySignificance = lambda.l_xy_sig();
    record.lxyError = lambda.l_xy_sig() > 0.f
                          ? lambda.l_xy() / lambda.l_xy_sig()
                          : -999.f;
    record.cosTheta2D = lambda.cos_theta_2D();
    record.pointingAngle2D = pointingAngle(lambda.cos_theta_2D());
    record.daughterDca3D = lambda.daughter_dca_3D();
    record.decayLength3D = lambda.decay_length_3D();
    record.decayLength3DSignificance = lambda.decay_length_3D_sig();
    record.daughterIndex1 = lambda.protonIdx();
    record.daughterIndex2 = lambda.pionIdx();
    fillTrackFeatures(record.trackFeatures[0], record.daughterIndex1,
                      record.pt, lambda.fit_proton_pt());
    fillTrackFeatures(record.trackFeatures[1], record.daughterIndex2,
                      record.pt, lambda.fit_pion_pt());
    record.daughterDeltaR =
        daughterDeltaR(record.trackFeatures[0], record.trackFeatures[1]);
    record.daughterAbsDz = std::abs(record.trackFeatures[0].dz -
                                    record.trackFeatures[1].dz);
    record.matchingEta = record.fitEta;
    record.matchingPhi = record.fitPhi;
    validationLambdaMass_.Fill(record.mass, output_.weight);
    AppendBPHCandidate(record);
  }

  for (const auto lambdaC : lambdaCs) {
    BPHRecord record;
    record.species = BPHSpecies::LambdaCToPKPi;
    record.sourceIndex = static_cast<int>(lambdaC.index());
    record.pt = lambdaC.pt();
    record.eta = lambdaC.eta();
    record.phi = lambdaC.phi();
    record.rapidity = rapidity(lambdaC.pt(), lambdaC.eta(), lambdaC.phi(),
                               lambdaC.mass());
    record.mass = lambdaC.mass();
    record.pdgId = lambdaC.charge() < 0 ? -4122 : 4122;
    record.charge = lambdaC.charge();
    record.inputJetIndex = lambdaC.jetIdx();
    record.inputJetDeltaR = lambdaC.jetDeltaR();
    record.hypothesisMask = lambdaC.hypothesisMask();
    record.prescaleWeight = lambdaC.prescaleWeight();
    record.genPartIndex = lambdaC.genPartIdx();
    record.genMatchCategory = lambdaC.genMatchCategory();
    record.fitPt = lambdaC.pt();
    record.fitEta = lambdaC.eta();
    record.fitPhi = lambdaC.phi();
    record.fitMass = lambdaC.mass();
    record.massHypothesis1 = lambdaC.mass();
    record.svProbability = lambdaC.svprob();
    record.svChi2 = lambdaC.sv_chi2();
    record.svNdof = lambdaC.sv_ndof();
    record.svChi2Ndof = lambdaC.sv_ndof() > 0.f
                            ? lambdaC.sv_chi2() / lambdaC.sv_ndof()
                            : -999.f;
    record.lxy = lambdaC.l_xy();
    record.lxySignificance = lambdaC.l_xy_sig();
    record.lxyError = lambdaC.l_xy_sig() > 0.f
                          ? lambdaC.l_xy() / lambdaC.l_xy_sig()
                          : -999.f;
    record.cosTheta2D = lambdaC.cos_theta_2D();
    record.pointingAngle2D = pointingAngle(lambdaC.cos_theta_2D());
    record.decayLength3D = lambdaC.decay_length_3D();
    record.decayLength3DSignificance = lambdaC.decay_length_3D_sig();
    record.daughterIndex1 = lambdaC.trk1Idx();
    record.daughterIndex2 = lambdaC.trk2Idx();
    record.daughterIndex3 = lambdaC.trk3Idx();
    fillTrackFeatures(record.trackFeatures[0], record.daughterIndex1,
                      record.pt, lambdaC.fit_trk1_pt());
    fillTrackFeatures(record.trackFeatures[1], record.daughterIndex2,
                      record.pt, lambdaC.fit_trk2_pt());
    fillTrackFeatures(record.trackFeatures[2], record.daughterIndex3,
                      record.pt, lambdaC.fit_trk3_pt());
    if (record.genMatchCategory > 0) {
      std::array<HadronAnalysis::LambdaCRecoTrackInput, 3> truthTracks;
      for (std::size_t track = 0; track < truthTracks.size(); ++track) {
        const auto &features = record.trackFeatures[track];
        truthTracks[track] =
            {features.pt, features.eta, features.phi, features.charge};
      }
      const auto truthHypothesis = lambdaCTruthMatcher_.Match(
          record.genPartIndex, truthTracks, lambdaCGenParticles);
      record.genHypothesisMask = truthHypothesis.mask;
      record.genHypothesisIndex = truthHypothesis.bestIndex;
    }
    record.pKDeltaR =
        daughterDeltaR(record.trackFeatures[0], record.trackFeatures[1]);
    record.kPiDeltaR =
        daughterDeltaR(record.trackFeatures[1], record.trackFeatures[2]);
    record.pPiDeltaR =
        daughterDeltaR(record.trackFeatures[0], record.trackFeatures[2]);
    record.minPairDeltaR =
        std::min({record.pKDeltaR, record.kPiDeltaR, record.pPiDeltaR});
    record.maxPairDeltaR =
        std::max({record.pKDeltaR, record.kPiDeltaR, record.pPiDeltaR});
    record.matchingEta = record.fitEta;
    record.matchingPhi = record.fitPhi;
    validationLambdaCMass_.Fill(record.mass, output_.weight);
    ForEachLambdaCHypothesisMass(lambdaC, [&](double mass) {
      validationLambdaCAcceptedHypothesisMass_.Fill(mass, output_.weight);
      if (lambdaC.genMatchCategory() > 0)
        validationLambdaCGenMatchedHypothesisMass_.Fill(mass, output_.weight);
    });
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
  if (outputTree_)
    outputTree_.Fill();
}
