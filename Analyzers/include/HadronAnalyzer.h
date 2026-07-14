#ifndef HadronAnalyzer_h
#define HadronAnalyzer_h

#include <array>
#include <cstdint>
#include <vector>

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
  static constexpr float kBPHJetRadius = 0.4f;

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

  struct BPHRecord {
    BPHSpecies species = BPHSpecies::D0ToKPi;
    int sourceIndex = -1;
    float pt = -999.f;
    float eta = -999.f;
    float phi = -999.f;
    float mass = -999.f;
    int pdgId = 0;
    int charge = 0;
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
    float lxy = -999.f;
    float lxyError = -999.f;
    float cosTheta2D = -999.f;
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
    std::vector<float> bphMass;
    std::vector<int> bphPdgId;
    std::vector<int> bphCharge;
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
    std::vector<float> bphLxy;
    std::vector<float> bphLxyError;
    std::vector<float> bphCosTheta2D;
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

  AnalyzerCore::TreeHandle outputTree_;
  Hist1DHandle selectedEvents_;
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

  void BookOutputTree();
  void BookHistograms();
  void FillSelectedEvent();
  void AppendBPHCandidate(const BPHRecord &record);
  void FillBPHHistograms(const BPHRecord &record, int jetRank, float deltaR);
  std::size_t JetFlavourIndex(int jetRank) const;
  static int GenOriginCategory(bool isPrompt, bool isFromB, bool isFromC,
                               bool isFromDstar, bool isFeeddown);
  static int GenPdgCategory(int pdgId, bool isBottomBaryon);
  int FindNearestLeadingJet(float eta, float phi, float &deltaR) const;
};

#endif
