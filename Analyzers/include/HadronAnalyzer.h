#ifndef HadronAnalyzer_h
#define HadronAnalyzer_h

#include <array>
#include <cstdint>
#include <vector>

#include "CharmFragmentation.h"
#include "LambdaCTruthMatcher.h"
#include "TopJetMatcher.h"
#include "Vcb_SL.h"

class HadronAnalyzer : public Vcb_SL {
public:
  HadronAnalyzer() = default;
  ~HadronAnalyzer() override = default;

  void initializeAnalyzer() override;
  void executeEvent() override;

private:
  static constexpr std::size_t kLeadingJets = 4;
  static constexpr std::size_t kJetFlavourCategories = 3;
  static constexpr std::size_t kWCharmSpecies = 3;
  static constexpr std::size_t kWCharmMatchCategories = 2;
  static constexpr float kBPHJetRadius = 0.4f;
  static constexpr int kLambdaCBDTBackgroundPrescale = 20;

  enum class BPHSpecies : int {
    D0ToKPi = 0,
    DstarToD0Pi = 1,
    Lambda = 2,
    LambdaCToPKPi = 3,
  };

  enum class JetFlavourCategory : std::size_t {
    Light = 0,
    Charm = 1,
    Bottom = 2,
  };

  struct BPHTrackFeatures {
    float pt = -999.f;
    float eta = -999.f;
    float phi = -999.f;
    int charge = 0;
    float ptFraction = -999.f;
    float dxy = -999.f;
    float dz = -999.f;
    float dxySignificance = -999.f;
    float dzSignificance = -999.f;
    float dcaSignificance = -999.f;
    float ptError = -999.f;
    float ptErrorRelative = -999.f;
    float normalizedChi2 = -999.f;
    float fitPt = -999.f;
    float fitPtFraction = -999.f;
    int nValidHits = -1;
    int nValidPixelHits = -1;
    int sourceType = -1;
    unsigned int sourceIndex = 0;
    int hasTrackDetails = 0;
    int highPurity = 0;
    int passVertexSelection = 0;
    int passSoftPionSelection = 0;
  };

  struct BPHRecord {
    BPHSpecies species = BPHSpecies::D0ToKPi;
    int sourceIndex = -1;
    float pt = -999.f;
    float eta = -999.f;
    float phi = -999.f;
    float rapidity = -999.f;
    float mass = -999.f;
    int pdgId = 0;
    int charge = 0;
    int inputJetIndex = -1;
    float inputJetDeltaR = -999.f;
    int hypothesisMask = 0;
    int prescaleWeight = 1;
    int genPartIndex = -1;
    int genMatchCategory = 0;
    int genHypothesisMask = 0;
    int genHypothesisIndex = -1;
    float fitPt = -999.f;
    float fitEta = -999.f;
    float fitPhi = -999.f;
    float fitMass = -999.f;
    float massHypothesis1 = -999.f;
    float massHypothesis2 = -999.f;
    float massError1 = -999.f;
    float massError2 = -999.f;
    float svProbability = -999.f;
    float svChi2 = -999.f;
    float svNdof = -999.f;
    float svChi2Ndof = -999.f;
    float lxy = -999.f;
    float lxyError = -999.f;
    float lxySignificance = -999.f;
    float cosTheta2D = -999.f;
    float pointingAngle2D = -999.f;
    float candidateDca = -999.f;
    float candidateDcaError = -999.f;
    float candidateDcaSignificance = -999.f;
    float daughterDeltaR = -999.f;
    float daughterAbsDz = -999.f;
    float daughterDca3D = -999.f;
    float decayLength3D = -999.f;
    float decayLength3DSignificance = -999.f;
    float softPionDxyD0Vertex = -999.f;
    float softPionDzD0Vertex = -999.f;
    float pKMass = -999.f;
    float kPiMass = -999.f;
    float pPiMass = -999.f;
    float pKDeltaR = -999.f;
    float kPiDeltaR = -999.f;
    float pPiDeltaR = -999.f;
    float minPairDeltaR = -999.f;
    float maxPairDeltaR = -999.f;
    // Species-dependent daughter order: D0=(trk1,trk2,-), D*=(-,-,soft pi),
    // Lambda=(proton,pion,-), LambdaC=(trk1,trk2,trk3).  Every index points
    // into the event-local compact HadronTrack collection.
    std::array<BPHTrackFeatures, 3> trackFeatures;
    float deltaMass = -999.f;
    int daughterIndex1 = -1;
    int daughterIndex2 = -1;
    int daughterIndex3 = -1;
    float matchingEta = -999.f;
    float matchingPhi = -999.f;
  };

  struct OutputBuffers {
    unsigned int run = 0;
    unsigned int luminosityBlock = 0;
    unsigned long long event = 0;
    int channel = -1;
    float weight = 1.f;
    float ht = 0.f;
    float metPt = 0.f;
    float metPhi = 0.f;
    float leptonPt = 0.f;
    float leptonEta = 0.f;
    float leptonPhi = 0.f;
    int nJet = 0;
    int nBTaggedJet = 0;
    int nCTaggedJet = 0;

    int topJetTopologyStatus = -1;
    int topJetHadronicTopPdgId = 0;
    int topJetNGenMatched = 0;
    int topJetNRecoMatched = 0;
    int topJetComplete = 0;
    float topJetTotalCost = -1.f;
    float topJetAssignmentMargin = -1.f;
    float topJetHadronicWRecoMass = -1.f;
    int topJetWCharmRole = -1;
    int topJetWCharmJetIndex = -1;
    int topJetWCharmJetOriginalIndex = -1;
    std::array<int, HadronAnalysis::kNumTopJetRoles> topJetGenPartIndex;
    std::array<int, HadronAnalysis::kNumTopJetRoles> topJetPartonPdgId;
    std::array<int, HadronAnalysis::kNumTopJetRoles> topJetGenJetIndex;
    std::array<int, HadronAnalysis::kNumTopJetRoles> topJetRecoJetIndex;
    std::array<int, HadronAnalysis::kNumTopJetRoles>
        topJetRecoJetOriginalIndex;
    std::array<float, HadronAnalysis::kNumTopJetRoles>
        topJetPartonGenJetDeltaR;
    std::array<float, HadronAnalysis::kNumTopJetRoles>
        topJetGenJetRecoDeltaR;
    std::array<float, HadronAnalysis::kNumTopJetRoles> topJetCost;
    std::array<int, HadronAnalysis::kNumTopJetRoles> topJetMatchSource;
    std::array<int, HadronAnalysis::kNumTopJetRoles>
        topJetFlavourFallback;

    std::array<float, kLeadingJets> jetPt;
    std::array<float, kLeadingJets> jetEta;
    std::array<float, kLeadingJets> jetPhi;
    std::array<float, kLeadingJets> jetMass;
    std::array<int, kLeadingJets> jetOriginalIndex;
    std::array<int, kLeadingJets> jetHadronFlavour;
    std::array<int, kLeadingJets> jetPartonFlavour;
    std::array<int, kLeadingJets> jetGenJetIndex;
    std::array<float, kLeadingJets> genJetPt;
    std::array<float, kLeadingJets> genJetEta;
    std::array<float, kLeadingJets> genJetPhi;
    std::array<float, kLeadingJets> genJetMass;
    std::array<float, kLeadingJets> recoGenJetDeltaR;
    std::array<int, kLeadingJets> genJetHadronFlavour;
    std::array<int, kLeadingJets> genJetPartonFlavour;

    std::array<int, kLeadingJets> genJetNBottomBaryon;
    std::array<int, kLeadingJets> genJetNCharmBaryon;
    std::array<int, kLeadingJets> genJetND0;
    std::array<int, kLeadingJets> genJetNDHadron;
    std::array<int, kLeadingJets> genJetNDplus;
    std::array<int, kLeadingJets> genJetNDstar;
    std::array<int, kLeadingJets> genJetNFeeddownD;
    std::array<int, kLeadingJets> genJetNLambdaB;
    std::array<int, kLeadingJets> genJetNLambdaBaryon;
    std::array<int, kLeadingJets> genJetNLambdaC;
    std::array<int, kLeadingJets> genJetNLightLambda;
    std::array<int, kLeadingJets> genJetNPromptD;

    std::array<int, kLeadingJets> nD0;
    std::array<int, kLeadingJets> nDstar;
    std::array<int, kLeadingJets> nLambda;
    std::array<int, kLeadingJets> nLambdaC;
    std::array<int, kLeadingJets> nGenHadron;

    std::vector<int> bphJetRank;
    std::vector<int> bphSpecies;
    std::vector<int> bphSourceIndex;
    std::vector<float> bphDeltaR;
    std::vector<float> bphPt;
    std::vector<float> bphEta;
    std::vector<float> bphPhi;
    std::vector<float> bphRapidity;
    std::vector<float> bphMass;
    std::vector<int> bphPdgId;
    std::vector<int> bphCharge;
    std::vector<int> bphInputJetIndex;
    std::vector<float> bphInputJetDeltaR;
    std::vector<int> bphHypothesisMask;
    std::vector<int> bphPrescaleWeight;
    std::vector<int> bphGenPartIndex;
    std::vector<int> bphGenMatchCategory;
    std::vector<std::uint8_t> bphGenHypothesisMask;
    std::vector<std::int16_t> bphGenHypothesisIndex;
    std::vector<float> bphWCharmZ;
    std::vector<float> bphWCharmDeltaR;
    std::vector<float> bphFitPt;
    std::vector<float> bphFitEta;
    std::vector<float> bphFitPhi;
    std::vector<float> bphFitMass;
    std::vector<float> bphMassHypothesis1;
    std::vector<float> bphMassHypothesis2;
    std::vector<float> bphMassError1;
    std::vector<float> bphMassError2;
    std::vector<float> bphSvProbability;
    std::vector<float> bphSvChi2;
    std::vector<float> bphSvNdof;
    std::vector<float> bphSvChi2Ndof;
    std::vector<float> bphLxy;
    std::vector<float> bphLxyError;
    std::vector<float> bphLxySignificance;
    std::vector<float> bphCosTheta2D;
    std::vector<float> bphPointingAngle2D;
    std::vector<float> bphCandidateDca;
    std::vector<float> bphCandidateDcaError;
    std::vector<float> bphCandidateDcaSignificance;
    std::vector<float> bphDaughterDeltaR;
    std::vector<float> bphDaughterAbsDz;
    std::vector<float> bphDaughterDca3D;
    std::vector<float> bphDecayLength3D;
    std::vector<float> bphDecayLength3DSignificance;
    std::vector<float> bphSoftPionDxyD0Vertex;
    std::vector<float> bphSoftPionDzD0Vertex;
    std::vector<float> bphPKMass;
    std::vector<float> bphKPiMass;
    std::vector<float> bphPPiMass;
    std::vector<float> bphPKDeltaR;
    std::vector<float> bphKPiDeltaR;
    std::vector<float> bphPPiDeltaR;
    std::vector<float> bphMinPairDeltaR;
    std::vector<float> bphMaxPairDeltaR;
    std::array<std::vector<float>, 3> bphTrackPt;
    std::array<std::vector<float>, 3> bphTrackEta;
    std::array<std::vector<float>, 3> bphTrackPhi;
    std::array<std::vector<int>, 3> bphTrackCharge;
    std::array<std::vector<float>, 3> bphTrackPtFraction;
    std::array<std::vector<float>, 3> bphTrackDxy;
    std::array<std::vector<float>, 3> bphTrackDz;
    std::array<std::vector<float>, 3> bphTrackDxySignificance;
    std::array<std::vector<float>, 3> bphTrackDzSignificance;
    std::array<std::vector<float>, 3> bphTrackDcaSignificance;
    std::array<std::vector<float>, 3> bphTrackPtError;
    std::array<std::vector<float>, 3> bphTrackPtErrorRelative;
    std::array<std::vector<float>, 3> bphTrackNormalizedChi2;
    std::array<std::vector<float>, 3> bphTrackFitPt;
    std::array<std::vector<float>, 3> bphTrackFitPtFraction;
    std::array<std::vector<int>, 3> bphTrackNValidHits;
    std::array<std::vector<int>, 3> bphTrackNValidPixelHits;
    std::vector<float> bphDeltaMass;
    std::vector<int> bphDaughterIndex1;
    std::vector<int> bphDaughterIndex2;
    std::vector<int> bphDaughterIndex3;

    std::vector<int> genHadronJetRank;
    std::vector<int> genHadronGenJetIndex;
    std::vector<int> genHadronSourceIndex;
    std::vector<int> genHadronPdgId;
    std::vector<int> genHadronMotherPdgId;
    std::vector<int> genHadronAncestorPdgId;
    std::vector<int> genHadronFlavour;
    std::vector<int> genHadronStatus;
    std::vector<float> genHadronPt;
    std::vector<float> genHadronEta;
    std::vector<float> genHadronPhi;
    std::vector<float> genHadronMass;
    std::vector<float> genHadronDeltaR;
    std::vector<float> genHadronRecoJetDeltaR;
    std::vector<float> genHadronVx;
    std::vector<float> genHadronVy;
    std::vector<float> genHadronVz;
    std::vector<int> genHadronIsBottomBaryon;
    std::vector<int> genHadronIsCharmBaryon;
    std::vector<int> genHadronIsD0;
    std::vector<int> genHadronIsDHadron;
    std::vector<int> genHadronIsDplus;
    std::vector<int> genHadronIsDstar;
    std::vector<int> genHadronIsFeeddown;
    std::vector<int> genHadronIsFromB;
    std::vector<int> genHadronIsFromC;
    std::vector<int> genHadronIsFromDstar;
    std::vector<int> genHadronIsLambdaB;
    std::vector<int> genHadronIsLambdaBaryon;
    std::vector<int> genHadronIsLambdaC;
    std::vector<int> genHadronIsLastCopy;
    std::vector<int> genHadronIsLightLambda;
    std::vector<int> genHadronIsPrompt;

    void clear();
  } output_;

  HadronAnalysis::TopJetMatcher topJetMatcher_;
  HadronAnalysis::LambdaCTruthMatcher lambdaCTruthMatcher_;
  bool lambdaCBDTMode_ = false;

  AnalyzerCore::RNTupleHandle outputTree_;
  Hist1DHandle selectedEvents_;
  Hist1DHandle hadronTrackMultiplicity_;
  Hist1DHandle d0InclusiveMultiplicity_;
  Hist1DHandle dstarInclusiveMultiplicity_;
  Hist1DHandle lambdaInclusiveMultiplicity_;
  Hist1DHandle lambdaCInclusiveMultiplicity_;
  Hist1DHandle hadronTrackPt_;
  Hist1DHandle hadronTrackPtErrorRelative_;
  Hist1DHandle validationD0Mass_;
  Hist1DHandle validationDstarDeltaMass_;
  Hist1DHandle validationLambdaMass_;
  Hist1DHandle validationLambdaCMass_;
  Hist1DHandle validationLambdaCAcceptedHypothesisMass_;
  Hist1DHandle validationLambdaCGenMatchedHypothesisMass_;
  Hist1DHandle wCharmRecoMass_;
  Hist1DHandle wCharmJetPt_;
  std::array<std::array<Hist2DHandle, kWCharmMatchCategories>,
             kWCharmSpecies>
      wCharmZVsRecoMass_;
  std::array<std::array<
                 std::array<Hist1DHandle,
                            HadronAnalysis::kNumRecoWMassBins>,
                 kWCharmMatchCategories>,
             kWCharmSpecies>
      wCharmZByRecoMass_;
  std::array<Hist1DHandle, kLeadingJets> d0Multiplicity_;
  std::array<Hist1DHandle, kLeadingJets> dstarMultiplicity_;
  std::array<Hist1DHandle, kLeadingJets> lambdaMultiplicity_;
  std::array<Hist1DHandle, kLeadingJets> lambdaCMultiplicity_;
  std::array<Hist1DHandle, kLeadingJets> genHadronMultiplicity_;

  using FlavourHistograms =
      std::array<Hist1DHandle, kJetFlavourCategories>;
  FlavourHistograms d0FitMassKPi_;
  FlavourHistograms d0PtFraction_;
  FlavourHistograms d0DeltaRJet_;
  FlavourHistograms d0LxySignificance_;
  FlavourHistograms d0CosTheta2D_;
  FlavourHistograms dstarDeltaMass_;
  FlavourHistograms lambdaFitMass_;
  FlavourHistograms lambdaCFitMass_;
  FlavourHistograms lambdaCPtFraction_;
  FlavourHistograms genHadronPtFraction_;
  FlavourHistograms genHadronDeltaRRecoJet_;
  FlavourHistograms genHadronOriginCategory_;
  FlavourHistograms genHadronPdgCategory_;

  std::array<Hist2DHandle, kLeadingJets> recoVsGenD0_;
  std::array<Hist2DHandle, kLeadingJets> recoVsGenDstar_;
  std::array<Hist2DHandle, kLeadingJets> recoVsGenLambda_;
  std::array<Hist2DHandle, kLeadingJets> recoVsGenLambdaC_;
  std::array<Hist2DHandle, kLeadingJets> recoVsGenTotal_;

  void RegisterTasks();
  void ValidateBPHStudy();
  void BookBPHStudy();
  void RunBPHStudy();
  void BookOutputRNTuple();
  void BookHistograms();
  void FillTopJetMatches();
  void FillWCharmFragmentation(const BPHRecord &record, int jetRank,
                               float &z, float &deltaR);
  void FillSelectedEvent();
  void AppendBPHCandidate(const BPHRecord &record);
  bool StoreBPHCandidate(const BPHRecord &record) const;
  void FillBPHHistograms(const BPHRecord &record, int jetRank, float deltaR);
  std::size_t JetFlavourIndex(int jetRank) const;
  static int GenOriginCategory(bool isPrompt, bool isFromB, bool isFromC,
                               bool isFromDstar, bool isFeeddown);
  static int GenPdgCategory(int pdgId, bool isBottomBaryon);
  int FindNearestLeadingJet(float eta, float phi, float &deltaR) const;
};

#endif
