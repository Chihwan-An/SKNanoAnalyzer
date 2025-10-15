#include "Jet.h"
#include <stdexcept>
#include <iostream>

ClassImp(Jet)

Jet::Jet() : Particle() {

  // Corrections
  j_PNetRegPtRawCorr = -999.0;
  j_PNetRegPtRawCorrNeutrino = -999.0;
  j_PNetRegPtRawRes = -999.0;
  j_UParTAK4RegPtRawCorr = -999.0;
  j_UParTAK4RegPtRawCorrNeutrino = -999.0;
  j_UParTAK4RegPtRawRes = -999.0;
  j_UParTAK4V1RegPtRawCorr = -999.0;
  j_UParTAK4V1RegPtRawCorrNeutrino = -999.0;
  j_UParTAK4V1RegPtRawRes = -999.0;
  j_rawFactor = -999.0;

  // Jet Substructure
  j_chEmEF = -999.0;
  j_chHEF = -999.0;
  j_neEmEF = -999.0;
  j_neHEF = -999.0;
  j_muEF = -999.0;
  j_nConstituents = -999;
  j_nElectrons = -999;
  j_nMuons = -999;
  j_nSVs = -999;
  j_chMultiplicity = -1;
  j_neMultiplicity = -1;

  // Matching Information
  j_electronIdx1 = -999;
  j_electronIdx2 = -999;
  j_muonIdx1 = -999;
  j_muonIdx2 = -999;
  j_svIdx1 = -999;
  j_svIdx2 = -999;
  j_genJetIdx = -999;

  // Jet Flavour
  j_hadronFlavour = -999;
  j_partonFlavour = -999;

  // Others
  j_area = -999.0;
  j_originalIndex = -1;
  j_puIDScore = -999.;
  j_m = -999.0;
  j_unsmearedP4.SetPxPyPzE(-999., -999., -999., -999.);
}

Jet::~Jet() {}

void Jet::AttachLazyPayload(void *context, EnsureCallback callback, int index) const {
  if (!lazy_) {
    lazy_ = std::make_shared<LazyPayload>();
  }
  lazy_->context = context;
  lazy_->callback = callback;
  lazy_->index = index;
  lazy_->loaded.reset();
}

void Jet::DetachLazyPayload() const {
  lazy_.reset();
}

void Jet::ensure(Property property) const {
  if (!lazy_) return;
  const auto bit = static_cast<std::size_t>(property);
  if (lazy_->loaded.test(bit)) return;
  if (!lazy_->callback) return;
  lazy_->callback(lazy_->context, const_cast<Jet &>(*this), property);
  lazy_->loaded.set(bit);
}

void Jet::markLoaded(Property property) const {
  if (!lazy_) return;
  lazy_->loaded.set(static_cast<std::size_t>(property));
}

Jet::Property Jet::propertyFor(JetTagging::JetFlavTagger tagger, JetTagging::JetFlavTaggerScoreType scoreType) {
  using Tagger = JetTagging::JetFlavTagger;
  using Score = JetTagging::JetFlavTaggerScoreType;
  switch (tagger) {
    case Tagger::DeepJet:
      switch (scoreType) {
        case Score::B: return Property::DeepFlavB;
        case Score::CvB: return Property::DeepFlavCvB;
        case Score::CvL: return Property::DeepFlavCvL;
        case Score::QvG: return Property::DeepFlavQG;
        default: break;
      }
      break;
    case Tagger::ParticleNet:
      switch (scoreType) {
        case Score::B: return Property::PNetB;
        case Score::CvB: return Property::PNetCvB;
        case Score::CvL: return Property::PNetCvL;
        case Score::CvNotB: return Property::PNetCvNotB;
        case Score::QvG: return Property::PNetQvG;
        case Score::TauVJet: return Property::PNetTauVJet;
        default: break;
      }
      break;
    case Tagger::ParT:
      switch (scoreType) {
        case Score::B: return Property::ParTB;
        case Score::CvB: return Property::ParTCvB;
        case Score::CvL: return Property::ParTCvL;
        case Score::CvNotB: return Property::ParTCvNotB;
        case Score::Ele: return Property::ParTEle;
        case Score::Mu: return Property::ParTMu;
        case Score::QvG: return Property::ParTQvG;
        case Score::SvCB: return Property::ParTSvCB;
        case Score::SvUDG: return Property::ParTSvUDG;
        case Score::TauVJet: return Property::ParTTauVJet;
        case Score::probUDG: return Property::ParTUDG;
        case Score::probB: return Property::ParTProbB;
        case Score::probBB: return Property::ParTProbBB;
        default: break;
      }
      break;
    default:
      break;
  }
  throw std::runtime_error("[Jet::propertyFor] Unsupported tagger/score combination");
}

void Jet::ensureScore(JetTagging::JetFlavTagger tagger, JetTagging::JetFlavTaggerScoreType scoreType) const {
  ensure(propertyFor(tagger, scoreType));
}

void Jet::SetTaggerScore(JetTagging::JetFlavTagger tagger, JetTagging::JetFlavTaggerScoreType scoreType, float value) {
  switch (tagger) {
    case JetTagging::JetFlavTagger::DeepJet:
      j_btagDeepFlav[scoreType] = value;
      break;
    case JetTagging::JetFlavTagger::ParticleNet:
      j_btagPNet[scoreType] = value;
      break;
    case JetTagging::JetFlavTagger::ParT:
      j_btagUParTAK4[scoreType] = value;
      break;
    default:
      break;
  }
  markLoaded(propertyFor(tagger, scoreType));
}

float Jet::GetTaggerResult(JetTagging::JetFlavTagger tagger, JetTagging::JetFlavTaggerScoreType scoreType) const {
    ensureScore(tagger, scoreType);
    const unordered_map<JetTagging::JetFlavTaggerScoreType, float> *taggerResult = nullptr;
    switch (tagger)
    {
    case JetTagging::JetFlavTagger::DeepJet:
        taggerResult = &j_btagDeepFlav;
        break;
    case JetTagging::JetFlavTagger::ParticleNet:
        taggerResult = &j_btagPNet;
        break;
    case JetTagging::JetFlavTagger::ParT:
        taggerResult = &j_btagUParTAK4;
        break;

    default:
        cout << "[FatJet::GetTaggerResult] No tagger " << JetTagging::GetTaggerCorrectionLibStr(tagger) << endl;
        exit(ENODATA);
    }
    auto it = taggerResult->find(scoreType);
    if (it == taggerResult->end())
    {
        cout << "[FatJet::GetTaggerResult] No tagger score type "
            << JetTagging::GetJetFlavTaggerScoreTypeStr(scoreType) 
            << " for tagger " << JetTagging::GetTaggerCorrectionLibStr(tagger) << endl;
        exit(ENODATA);
    }
    return it->second;
}

TLorentzVector Jet::GetUnsmearedP4() const{
  return j_unsmearedP4;
}
