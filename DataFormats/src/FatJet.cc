#include "FatJet.h"
#include <stdexcept>
#include <limits>
#include <iostream>

ClassImp(FatJet)

void FatJet::initializeMembers(){
    storage_.reset();
    index_ = std::numeric_limits<std::size_t>::max();
    cachedProperties_.reset();
    lazy_.reset();
        // ───── Basic Jet Properties ─────────────
    fj_rawPt = -999.f;
    fj_originalPt = -999.f;
    fj_originalIndex = -1;
    fj_globalParT3_massCorrGeneric = -999.f;
    fj_globalParT3_massCorrX2p = -999.f;
    fj_particleNet_massCorr = -999.f;
    fj_rawFactor = -999.f;

    fj_globalParT3.fill(FatJet::kInvalidTagScore);
    fj_particleNet.fill(FatJet::kInvalidTagScore);

    // ───── Energy Fractions ────────────────
    // ───── Multiplicities ──────────────────
    fj_chEmEF = -999.f;
    fj_chHEF = -999.f;
    fj_neEmEF = -999.f;
    fj_neHEF = -999.f;
    fj_muEF = -999.f;
    fj_nConstituents = -999;
    fj_chMultiplicity = -1;
    fj_neMultiplicity = -1;
    
    // ───── Indices and Flavors ─────────────
    fj_genJetAK8Idx = -999;
    fj_subJetIdx1 = -999;
    fj_subJetIdx2 = -999;
    fj_hadronFlavour = -999;
    
    fj_muonIdx3SJ = -999;
    fj_electronIdx3SJ = -999;

    // ───── Jet Substructure ────────────────
    fj_msoftdrop = 0.;
    fj_n2b1 = -999.;
    fj_n3b1 = -999.;
    fj_lsf3 = 0.;
    fj_tau1 = -999.;
    fj_tau2 = -999.;
    fj_tau3 = -999.;
    fj_tau4 = -999.;

    fj_area = -999.f;
    fj_mass = -999.f;
    fj_unsmearedP4.SetPxPyPzE(-999., -999., -999., -999.);

    // -───── Corr ─────────────────────
    fj_massCorrGeneric = -999.f;
    fj_massCorrX2p = -999.f;
    fj_massCorr = -999.f;
}

FatJet::FatJet() : Particle() {
    initializeMembers();
}

FatJet::FatJet(std::shared_ptr<const FatJetSoA> storage, std::size_t index) : Particle() {
    initializeMembers();
    storage_ = std::move(storage);
    index_ = index;
    if (storage_) {
        const float pt = storage_->pt[index_];
        const float eta = storage_->eta[index_];
        const float phi = storage_->phi[index_];
        const float mass = storage_->mass[index_];
        SetPtEtaPhiM(pt, eta, phi, mass);

        fj_originalPt = storage_->pt[index_];
        fj_rawPt = storage_->pt[index_] * (1.f - storage_->rawFactor[index_]);
        fj_rawFactor = storage_->rawFactor[index_];
        fj_area = storage_->area[index_];
        
        SetEnergyFractions(storage_->chHEF[index_], storage_->neHEF[index_], storage_->neEmEF[index_], storage_->chEmEF[index_], storage_->muEF[index_]);
        SetHadronMultiplicities(storage_->chMultiplicity[index_], storage_->neMultiplicity[index_]);
        SetMultiplicities(storage_->nConstituents[index_]);
        
        const short e1 = storage_->electronIdx3SJ[index_];
        const short m1 = storage_->muonIdx3SJ[index_];
        const short sv1 = storage_->subJetIdx1[index_];
        const short sv2 = storage_->subJetIdx2[index_];
        const short gj8 = storage_->genJetAK8Idx[index_];
        SetMatchingIndices(e1, m1, sv1, sv2, gj8);
        
        SetJetFlavours(static_cast<unsigned char>(storage_->hadronFlavour[index_]));

        fj_massCorrGeneric = storage_->massCorrGeneric[index_]; //globalParT3_massCorrGeneric
        fj_massCorrX2p = storage_->massCorrX2p[index_]; // globalParT3_massCorrX2p
        fj_massCorr = storage_->massCorr[index_];  // particleNet_massCorr
        
        }
    }

    FatJet::~FatJet() {}

    void FatJet::AttachLazyPayload(void *context, EnsureCallback callback, int index) const {
        materialize();
        if (!lazy_) {
            lazy_ = std::make_shared<LazyPayload>();
        }
        lazy_->context = context;
        lazy_->callback = callback;
        lazy_->index = index;
        lazy_->loaded = cachedProperties_;
}

void FatJet::DetachLazyPayload() const {
    lazy_.reset();
}

void FatJet::ensure(Property property) const {
    const auto bit = static_cast<std::size_t>(property);
    if (cachedProperties_.test(bit))
        return;

    if (storage_) {
        loadFromStorage(property);
        return;
    }

    if (!lazy_ || !lazy_->callback)
        return;

    lazy_->callback(lazy_->context, const_cast<FatJet &>(*this), property);
    markLoaded(property);
}

void FatJet::markLoaded(Property property) const {
    const auto bit = static_cast<std::size_t>(property);
    cachedProperties_.set(bit);
    if (lazy_)
        lazy_->loaded.set(bit);
}

void FatJet::materialize() const {
    if (!storage_)
    return;

    auto self = const_cast<FatJet *>(this);
    const auto idx = index_;

    const float pt = storage_->pt[idx];
    const float eta = storage_->eta[idx];
    const float phi = storage_->phi[idx];
    const float mass = storage_->mass[idx];
    self->SetPtEtaPhiM(pt, eta, phi, mass);

    self->fj_originalPt = storage_->pt[idx];
    self->fj_rawPt = storage_->pt[idx] * (1.f - storage_->rawFactor[idx]);
    self->fj_rawFactor = storage_->rawFactor[idx];
    self->fj_area = storage_->area[idx];

    self->SetEnergyFractions(storage_->chHEF[idx], storage_->neHEF[idx], storage_->neEmEF[idx], storage_->chEmEF[idx], storage_->muEF[idx]);
    self->SetHadronMultiplicities(storage_->chMultiplicity[idx], storage_->neMultiplicity[idx]);
    self->SetMultiplicities(storage_->nConstituents[idx]);

    const short e1 = storage_->electronIdx3SJ[idx];
    const short m1 = storage_->muonIdx3SJ[idx];
    const short sv1 = storage_->subJetIdx1[idx];
    const short sv2 = storage_->subJetIdx2[idx];
    const short gj8 = storage_->genJetAK8Idx[idx];
    self->SetMatchingIndices(e1, m1, sv1, sv2, gj8);

    self->SetJetFlavours( static_cast<unsigned char>(storage_->hadronFlavour[idx]));

    self->fj_massCorrGeneric = storage_->massCorrGeneric[idx]; //globalParT3_massCorrGeneric
    self->fj_massCorrX2p = storage_->massCorrX2p[idx]; // globalParT3_massCorrX2p
    self->fj_massCorr = storage_->massCorr[idx];  // particleNet_massCorr


    for (std::size_t bit = 0; bit < static_cast<std::size_t>(Property::Count); ++bit) {
        if (!cachedProperties_.test(bit))
            loadFromStorage(static_cast<Property>(bit));
    }

    self->storage_.reset();
    self->index_ = std::numeric_limits<std::size_t>::max();
}

void FatJet::loadFromStorage(Property property) const {
    if (!storage_)
        return;

    const auto idx = index_;
    auto self = const_cast<FatJet *>(this);

    using Tagger = JetTagging::FatJetTagger;
    using Score = JetTagging::FatJetTaggerScoreType;

    switch (property) {
    case Property::Part3QCD:
        self->SetTaggerScore(Tagger::ParT, Score::QCD, storage_->globalParT3_QCD[idx]);
        break;
    case Property::Part3TopbWev:
        self->SetTaggerScore(Tagger::ParT, Score::TopbWev, storage_->globalParT3_TopbWev[idx]);
        break;
    case Property::Part3TopbWmv:
        self->SetTaggerScore(Tagger::ParT, Score::TopbWmv, storage_->globalParT3_TopbWmv[idx]);
        break;
    case Property::Part3TopbWq:
        self->SetTaggerScore(Tagger::ParT, Score::TopbWq, storage_->globalParT3_TopbWq[idx]);
        break;
    case Property::Part3TopbWqq:
        self->SetTaggerScore(Tagger::ParT, Score::TopbWqq, storage_->globalParT3_TopbWqq[idx]);
        break;
    case Property::Part3TopbWtauhv:
        self->SetTaggerScore(Tagger::ParT, Score::TopbWtauhv, storage_->globalParT3_TopbWtauhv[idx]);
        break;
    case Property::Part3WvsQCD:
        self->SetTaggerScore(Tagger::ParT, Score::WvsQCD, storage_->globalParT3_WvsQCD[idx]);
        break;
    case Property::Part3Xww3Q:
        self->SetTaggerScore(Tagger::ParT, Score::XWW3q, storage_->globalParT3_XWW3q[idx]);
        break;
    case Property::Part3Xww4Q:
        self->SetTaggerScore(Tagger::ParT, Score::XWW4q, storage_->globalParT3_XWW4q[idx]);
        break;
    case Property::Part3Xwwqqev:
        self->SetTaggerScore(Tagger::ParT, Score::XWWqqev, storage_->globalParT3_XWWqqev[idx]);
        break;
    case Property::Part3Xwwqqmv:
        self->SetTaggerScore(Tagger::ParT, Score::XWWqqmv, storage_->globalParT3_XWWqqmv[idx]);
        break;
    case Property::Part3Xbb:
        self->SetTaggerScore(Tagger::ParT, Score::Xbb, storage_->globalParT3_Xbb[idx]);
        break;
    case Property::Part3Xcc:
        self->SetTaggerScore(Tagger::ParT, Score::Xcc, storage_->globalParT3_Xcc[idx]);
        break;
    case Property::Part3Xcs:
        self->SetTaggerScore(Tagger::ParT, Score::Xcs, storage_->globalParT3_Xcs[idx]);
        break;
    case Property::Part3Xqq:
        self->SetTaggerScore(Tagger::ParT, Score::Xqq, storage_->globalParT3_Xqq[idx]);
        break;
    case Property::Part3Xtauhtaue:
        self->SetTaggerScore(Tagger::ParT, Score::Xtauhtaue, storage_->globalParT3_Xtauhtaue[idx]);
        break;
    case Property::Part3Xtauhtauh:
        self->SetTaggerScore(Tagger::ParT, Score::Xtauhtauh, storage_->globalParT3_Xtauhtauh[idx]);
        break;
    case Property::Part3Xtauhtaum:
        self->SetTaggerScore(Tagger::ParT, Score::Xtauhtaum, storage_->globalParT3_Xtauhtaum[idx]);
        break;
    case Property::Part3WithMassTvsQcd:
        self->SetTaggerScore(Tagger::ParTWithMass, Score::TvsQCD, storage_->particleNetWithMass_TvsQCD[idx]);
        break;
    case Property::Part3WithMassWvsQcd:
        self->SetTaggerScore(Tagger::ParTWithMass, Score::WvsQCD, storage_->particleNetWithMass_WvsQCD[idx]);
        break;
    case Property::Part3WithMassZvsQcd:
        self->SetTaggerScore(Tagger::ParTWithMass, Score::ZvsQCD, storage_->particleNetWithMass_ZvsQCD[idx]);
        break;
    case Property::PNetLegacyQcd:
        self->SetTaggerScore(Tagger::ParticleNet, Score::LegacyQCD, storage_->particleNetLegacy_QCD[idx]);
        break;
    case Property::PNetLegacyXbb:
        self->SetTaggerScore(Tagger::ParticleNet, Score::LegacyXbb, storage_->particleNetLegacy_Xbb[idx]);
        break;
    case Property::PNetLegacyXcc:
        self->SetTaggerScore(Tagger::ParticleNet, Score::LegacyXcc, storage_->particleNetLegacy_Xcc[idx]);
        break;
    case Property::PNetLegacyXqq:
        self->SetTaggerScore(Tagger::ParticleNet, Score::LegacyXqq, storage_->particleNetLegacy_Xqq[idx]);
        break;
    case Property::PNetLegacyMass:
        self->SetTaggerScore(Tagger::ParticleNet, Score::LegacyMass, storage_->particleNetLegacy_mass[idx]);
        break;
    case Property::PNetWithmassH4Qvsqcd:
        self->SetTaggerScore(Tagger::ParticleNetWithMass, Score::H4qvsQCD, storage_->particleNetWithMass_H4qvsQCD[idx]);
        break;
    case Property::PNetWithmassHbbvsqcd:
        self->SetTaggerScore(Tagger::ParticleNetWithMass, Score::HbbvsQCD, storage_->particleNetWithMass_HbbvsQCD[idx]);
        break;
    case Property::PNetWithmassHccvsqcd:
        self->SetTaggerScore(Tagger::ParticleNetWithMass, Score::HccvsQCD, storage_->particleNetWithMass_HccvsQCD[idx]);
        break;
    case Property::PNetWithmassQcd:
        self->SetTaggerScore(Tagger::ParticleNetWithMass, Score::QCD, storage_->particleNetWithMass_QCD[idx]);
        break;
    case Property::PNetWithMassTvsQcd:
        self->SetTaggerScore(Tagger::ParticleNetWithMass, Score::TvsQCD, storage_->particleNetWithMass_TvsQCD[idx]);
        break;
    case Property::PNetWithMassWvsQcd:
        self->SetTaggerScore(Tagger::ParticleNetWithMass, Score::WvsQCD, storage_->particleNetWithMass_WvsQCD[idx]);
        break;
    case Property::PNetWithMassZvsQcd:
        self->SetTaggerScore(Tagger::ParticleNetWithMass, Score::ZvsQCD, storage_->particleNetWithMass_ZvsQCD[idx]);
        break;
    case Property::PNetQcd:
        self->SetTaggerScore(Tagger::ParticleNet, Score::QCD, storage_->particleNet_QCD[idx]);
        break;
    case Property::PNetQcd0Hf:
        self->SetTaggerScore(Tagger::ParticleNet, Score::QCD0HF, storage_->particleNet_QCD0HF[idx]);
        break;
    case Property::PNetQcd1Hf:
        self->SetTaggerScore(Tagger::ParticleNet, Score::QCD1HF, storage_->particleNet_QCD1HF[idx]);
        break;
    case Property::PNetQcd2Hf:
        self->SetTaggerScore(Tagger::ParticleNet, Score::QCD2HF, storage_->particleNet_QCD2HF[idx]);
        break;
    case Property::PNetWvsQcd:
        self->SetTaggerScore(Tagger::ParticleNet, Score::WvsQCD, storage_->particleNet_WVsQCD[idx]);
        break;
    case Property::PNetXbbvsQcd:
        self->SetTaggerScore(Tagger::ParticleNet, Score::XbbVsQCD, storage_->particleNet_XbbVsQCD[idx]);
        break;
    case Property::PNetXccvsQcd:
        self->SetTaggerScore(Tagger::ParticleNet, Score::XccVsQCD, storage_->particleNet_XccVsQCD[idx]);
        break;
    case Property::PNetXqqvsQcd:
        self->SetTaggerScore(Tagger::ParticleNet, Score::XqqVsQCD, storage_->particleNet_XqqVsQCD[idx]);
        break;
    case Property::PNetXggvsQcd:
        self->SetTaggerScore(Tagger::ParticleNet, Score::XggVsQCD, storage_->particleNet_XggVsQCD[idx]);
        break;
    case Property::PNetXtevsQcd:
        self->SetTaggerScore(Tagger::ParticleNet, Score::XteVsQCD, storage_->particleNet_XteVsQCD[idx]);
        break;
    case Property::PNetXtmvsQcd:
    self->SetTaggerScore(Tagger::ParticleNet, Score::XtmVsQCD, storage_->particleNet_XtmVsQCD[idx]);
        break;
    case Property::PNetXttvsQcd:
        self->SetTaggerScore(Tagger::ParticleNet, Score::XttVsQCD, storage_->particleNet_XttVsQCD[idx]);
        break;
    default:
        break;
    }

    markLoaded(property);
}

FatJet::Property FatJet::propertyFor(JetTagging::FatJetTagger tagger, JetTagging::FatJetTaggerScoreType scoreType) {
    using Tagger = JetTagging::FatJetTagger;
    using Score = JetTagging::FatJetTaggerScoreType;
    switch (tagger) {
        case Tagger::ParT:
        switch (scoreType) {
            case Score::QCD: return Property::Part3QCD;
            case Score::TopbWev: return Property::Part3TopbWev;
            case Score::TopbWmv: return Property::Part3TopbWmv;
            case Score::TopbWq: return Property::Part3TopbWq;
            case Score::TopbWqq: return Property::Part3TopbWqq;
            case Score::TopbWtauhv: return Property::Part3TopbWtauhv;
            case Score::WvsQCD: return Property::Part3WvsQCD;
            case Score::XWW3q: return Property::Part3Xww3Q;
            case Score::XWW4q: return Property::Part3Xww4Q;
            case Score::XWWqqev: return Property::Part3Xwwqqev;
            case Score::XWWqqmv: return Property::Part3Xwwqqmv;
            case Score::Xbb: return Property::Part3Xbb;
            case Score::Xcc: return Property::Part3Xcc;
            case Score::Xcs: return Property::Part3Xcs;
            case Score::Xqq: return Property::Part3Xqq;
            case Score::Xtauhtaue: return Property::Part3Xtauhtaue;
            case Score::Xtauhtauh: return Property::Part3Xtauhtauh;
            case Score::Xtauhtaum: return Property::Part3Xtauhtaum;
            default: break;
        }
        break;
        case Tagger::ParTWithMass:
        switch (scoreType) {
            case Score::TvsQCD: return Property::Part3WithMassTvsQcd;
            case Score::WvsQCD: return Property::Part3WithMassWvsQcd;
            case Score::ZvsQCD: return Property::Part3WithMassZvsQcd;
            default: break;
        }
        break;
        case Tagger::ParticleNet:
        switch (scoreType) {
            case Score::LegacyQCD: return Property::PNetLegacyQcd;
            case Score::LegacyXbb: return Property::PNetLegacyXbb;
            case Score::LegacyXcc: return Property::PNetLegacyXcc;
            case Score::LegacyXqq: return Property::PNetLegacyXqq;
            case Score::LegacyMass: return Property::PNetLegacyMass;
            case Score::QCD: return Property::PNetQcd;
            case Score::QCD0HF: return Property::PNetQcd0Hf;
            case Score::QCD1HF: return Property::PNetQcd1Hf;
            case Score::QCD2HF: return Property::PNetQcd2Hf;
            case Score::WvsQCD: return Property::PNetWvsQcd;
            case Score::XbbVsQCD: return Property::PNetXbbvsQcd;
            case Score::XccVsQCD: return Property::PNetXccvsQcd;
            case Score::XqqVsQCD: return Property::PNetXqqvsQcd;
            case Score::XggVsQCD: return Property::PNetXggvsQcd;
            case Score::XteVsQCD: return Property::PNetXtevsQcd;
            case Score::XtmVsQCD: return Property::PNetXtmvsQcd;
            case Score::XttVsQCD: return Property::PNetXttvsQcd;
            default: break;
        }
        break;
        case Tagger::ParticleNetWithMass:
        switch (scoreType) {
            case Score::H4qvsQCD: return Property::PNetWithmassH4Qvsqcd;
            case Score::HbbvsQCD: return Property::PNetWithmassHbbvsqcd;
            case Score::HccvsQCD: return Property::PNetWithmassHccvsqcd;
            case Score::QCD: return Property::PNetWithmassQcd;
            case Score::TvsQCD: return Property::PNetWithMassTvsQcd;
            case Score::WvsQCD: return Property::PNetWithMassWvsQcd;
            case Score::ZvsQCD: return Property::PNetWithMassZvsQcd;
            default: break;
        }
        break;
    default:
        break;
    }
    throw std::runtime_error("[Jet::propertyFor] Unsupported tagger/score combination");
}

void FatJet::ensureScore(JetTagging::FatJetTagger tagger, JetTagging::FatJetTaggerScoreType scoreType) const {
    ensure(propertyFor(tagger, scoreType));
}

void FatJet::SetTaggerScore(JetTagging::FatJetTagger tagger, JetTagging::FatJetTaggerScoreType scoreType, float value) {
    const auto idx = JetTagging::FatJetFlavTaggerScoreIndex(scoreType);
    switch (tagger) {
    case JetTagging::FatJetTagger::ParT:
        fj_globalParT3[idx] = value;
        break;
    case JetTagging::FatJetTagger::ParticleNet:
        fj_particleNet[idx] = value;
        break;
    default:
        break;
    }
    markLoaded(propertyFor(tagger, scoreType));
}

float FatJet::GetTaggerResult(JetTagging::FatJetTagger tagger, JetTagging::FatJetTaggerScoreType scoreType) const{
    ensureScore(tagger, scoreType);
    const auto idx = JetTagging::FatJetFlavTaggerScoreIndex(scoreType);
    switch (tagger)
    {
    case JetTagging::FatJetTagger::ParT:
        return fj_globalParT3[idx];
    case JetTagging::FatJetTagger::ParticleNet:
        return fj_particleNet[idx];
    
        default:
        cout << "[FatJet::GetTaggerResult] No tagger " << JetTagging::GetFatJetTaggerCorrectionLibStr(tagger) << endl;
        exit(ENODATA);
    }
    return kInvalidTagScore;
}


TLorentzVector FatJet::GetUnsmearedP4() const{
    return fj_unsmearedP4;
}
