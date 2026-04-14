#include "Electron.h"
#include <algorithm>
#include <cmath>

ClassImp(Electron)

Electron::Electron() {
    this->SetLeptonFlavour(ELECTRON);
    // uncertainties
    j_energyErr = -999.;
    j_dEsigmaUp = -999.;
    j_dEsigmaDown = -999.;
    // ID variables
    j_convVeto = false;
    j_lostHits = 0;
    j_seedGain = 0;
    j_tightCharge = 0;
    j_sieie = -999.;
    j_hoe = -999.;
    j_eInvMinusPInv = -999.;
    j_dr03EcalRecHitSumEt = -999.;
    j_dr03HcalDepth1TowerSumEt = -999.;
    j_dr03TkSumPt = -999.;
    j_dr03TkSumPtHEEP = -999.;
    // supercluster
    j_scEta = -999.;
    j_deltaEtaInSC = -999.;
    j_deltaEtaInSeed = -999.;
    j_deltaPhiInSC = -999.;
    j_deltaPhiInSeed = -999.;
    j_ecalPFClusterIso = -999.;
    j_hcalPFClusterIso = -999.;
    // ID bits
    j_mvaIso_WP80 = false;
    j_mvaIso_WP90 = false;
    j_mvaIso_WPL = false;
    j_mvaNoIso_WP80 = false;
    j_mvaNoIso_WP90 = false;
    j_mvaNoIso_WPL = false;
    j_cutBased_HEEP = false;
    j_cutBased = 0;
    j_mvaIso = -999.;
    j_mvaNoIso = -999.;
    j_mvaPrompt = -999.;
    // others
    j_r9 = -999.;
    j_rho = -999.;
    j_vidNestedWPBitmap = 0;
    j_vidNestedWPBitmapHEEP = 0;
    j_scEtOverPt = -999.;
    j_genPartFlav = 0;
    j_genPartIdx = -1;
    j_jetIdx = -1;
}

Electron::~Electron() {}

void Electron::AttachLazyPayload(void *context, EnsureCallback callback, int index) const {
    if (!lazy_) {
        lazy_ = std::make_shared<LazyPayload>();
    }
    lazy_->context = context;
    lazy_->callback = callback;
    lazy_->index = index;
    lazy_->loaded.reset();
}

void Electron::DetachLazyPayload() const {
    lazy_.reset();
}

void Electron::ensure(Property property) const {
    if (!lazy_) return;
    const auto bit = static_cast<std::size_t>(property);
    if (lazy_->loaded.test(bit)) return;
    if (!lazy_->callback) return;
    lazy_->callback(lazy_->context, const_cast<Electron &>(*this), property);
    lazy_->loaded.set(bit);
}

void Electron::markLoaded(Property property) const {
    if (!lazy_) return;
    lazy_->loaded.set(static_cast<std::size_t>(property));
}

void Electron::SetBIDBit(BooleanID id, bool idbit) {
    switch (id) {
        case BooleanID::MVAISOWP80:
            j_mvaIso_WP80 = idbit;
            markLoaded(Property::MvaIsoWP80);
            break;
        case BooleanID::MVAISOWP90:
            j_mvaIso_WP90 = idbit;
            markLoaded(Property::MvaIsoWP90);
            break;
        case BooleanID::MVAISOWPL:
            j_mvaIso_WPL = idbit;
            break;
        case BooleanID::MVANOISOWP80:
            j_mvaNoIso_WP80 = idbit;
            markLoaded(Property::MvaNoIsoWP80);
            break;
        case BooleanID::MVANOISOWP90:
            j_mvaNoIso_WP90 = idbit;
            markLoaded(Property::MvaNoIsoWP90);
            break;
        case BooleanID::MVANOISOWPL:
            j_mvaNoIso_WPL = idbit;
            break;
        case BooleanID::CUTBASEDHEEP:
            j_cutBased_HEEP = idbit;
            markLoaded(Property::CutBasedHEEP);
            break;
        default:
            break;
    }
}

void Electron::SetCBIDBit(CutBasedID id, unsigned int idbit) {
    switch (id) {
        case CutBasedID::CUTBASED:
            j_cutBased = idbit;
            markLoaded(Property::CutBased);
            break;
        default:
            break;
    }
}

void Electron::SetMVA(MVATYPE type, float score) {
    switch (type) {
        case MVATYPE::MVAISO:
            j_mvaIso = score;
            markLoaded(Property::MvaIso);
            break;
        case MVATYPE::MVANOISO:
            j_mvaNoIso = score;
            markLoaded(Property::MvaNoIso);
            break;
        case MVATYPE::MVAPROMPT:
            j_mvaPrompt = score;
            markLoaded(Property::PromptMVA);
            break;
        default:
            break;
    }
}

bool Electron::PassID(const TString ID) const {
    if (etaRegion() == ETAREGION::GAP) return false;

    if (ID == "")                   return true;
    if (ID == "NOCUT")              return true;
    if (ID == "POGVeto")            return static_cast<int>(CutBased()) >= static_cast<int>(WORKINGPOINT::VETO);
    if (ID == "POGLoose")           return static_cast<int>(CutBased()) >= static_cast<int>(WORKINGPOINT::LOOSE);
    if (ID == "POGMedium")          return static_cast<int>(CutBased()) >= static_cast<int>(WORKINGPOINT::MEDIUM);
    if (ID == "POGTight")           return static_cast<int>(CutBased()) >= static_cast<int>(WORKINGPOINT::TIGHT);
    if (ID == "POGHEEP")            return isCutBasedHEEP();
    if (ID == "POGMVAIsoWP80")      return isMVAIsoWP80();
    if (ID == "POGMVAIsoWP90")      return isMVAIsoWP90();
    if (ID == "POGMVANoIsoWP80")    return isMVANoIsoWP80();
    if (ID == "POGMVANoIsoWP90")    return isMVANoIsoWP90();
    if (ID == "HcToWATight")        return Pass_HcToWATight();
    if (ID == "HcToWALooseRun2")    return Pass_HcToWALooseRun2();
    if (ID == "HcToWALooseRun3")    return Pass_HcToWALooseRun3();

    cerr << "[Electron::PassID] " << ID << " is not implemented" << endl;
    exit(ENODATA);
}

bool Electron::PassID(ElectronID ID) const {
    switch (ID) {
        case ElectronID::POG_VETO:         return PassID("POGVeto");
        case ElectronID::POG_LOOSE:        return PassID("POGLoose");
        case ElectronID::POG_MEDIUM:       return PassID("POGMedium");
        case ElectronID::POG_TIGHT:        return PassID("POGTight");
        case ElectronID::POG_HEEP:         return PassID("POGHEEP");
        case ElectronID::POG_MVAISO_WP80:  return PassID("POGMVAIsoWP80");
        case ElectronID::POG_MVAISO_WP90:  return PassID("POGMVAIsoWP90");
        case ElectronID::POG_MVANOISO_WP80:return PassID("POGMVANoIsoWP80");
        case ElectronID::POG_MVANOISO_WP90:return PassID("POGMVANoIsoWP90");
        default: break;
    }
    return false;
}

// Private IDs
bool Electron::Pass_CaloIdL_TrackIdL_IsoVL() const {
    const bool ApplyEA = true;
    if (etaRegion() == ETAREGION::GAP) {
        return false;
    } else if  (etaRegion() == ETAREGION::IB || etaRegion() == ETAREGION::OB) {
        if (!(sieie() < 0.013)) return false;
        if (!(fabs(deltaEtaInSC()) < 0.01)) return false;
        if (!(fabs(deltaPhiInSeed()) < 0.07)) return false;
        if (!(hoe() < 0.13)) return false;
        if (!(std::max(0.f, ecalPFClusterIso() - rho()*(ApplyEA ? 0.16544f : 0.f)) < 0.5f*Pt())) return false;
        if (!(std::max(0.f, hcalPFClusterIso() - rho()*(ApplyEA ? 0.05956f : 0.f)) < 0.3f*Pt())) return false;
        if (!(dr03TkSumPt() < 0.2f*Pt())) return false;
        return true;
    } else {
        if (!(sieie() < 0.035)) return false;
        if (!(fabs(deltaEtaInSC()) < 0.015)) return false;
        if (!(fabs(deltaPhiInSeed()) < 0.1)) return false;
        if (!(hoe() < 0.13)) return false;
        if (!(std::max(0.f, ecalPFClusterIso() - rho()*(ApplyEA ? 0.13212f : 0.f)) < 0.5f*Pt())) return false;
        if (!(std::max(0.f, hcalPFClusterIso() - rho()*(ApplyEA ? 0.13052f : 0.f)) < 0.3f*Pt())) return false;
        if (!(dr03TkSumPt() < 0.2f*Pt())) return false;
        return true;
    }
}

bool Electron::Pass_HcToWABaseline() const {
    if (!Pass_CaloIdL_TrackIdL_IsoVL()) return false;
    if (!ConvVeto()) return false;
    if (!(LostHits() < 2)) return false;
    if (!(fabs(dZ()) < 0.1)) return false;
    return true;
}

bool Electron::Pass_HcToWATight() const {
    if (!Pass_HcToWABaseline()) return false;
    if (!isMVANoIsoWP90()) return false;
    if (!(SIP3D() < 4.)) return false;
    if (!(MiniPFRelIso() < 0.1)) return false;
    return true;
}

bool Electron::Pass_HcToWALooseRun2() const {
    if (!Pass_HcToWABaseline()) return false;
    if (!(SIP3D() < 8.)) return false;
    if (!(MiniPFRelIso() < 0.4)) return false;
    const float cutIB=0.985, cutOB=0.96, cutEC=0.85;
    bool passMVAIDNoIsoCut = false;
    switch(etaRegion()) {
        case ETAREGION::IB:
            if (!(MvaNoIso() > cutIB)) passMVAIDNoIsoCut = true;
            break;
        case ETAREGION::OB:
            if (!(MvaNoIso() > cutOB)) passMVAIDNoIsoCut = true;
            break;
        case ETAREGION::EC:
            if (!(MvaNoIso() > cutEC)) passMVAIDNoIsoCut = true;
            break;
        default: break;
    }
    if (!(isMVANoIsoWP90() || passMVAIDNoIsoCut)) return false;
    return true;
}

bool Electron::Pass_HcToWALooseRun3() const {
    if (!Pass_HcToWABaseline()) return false;
    if (!(SIP3D() < 8.)) return false;
    if (!(MiniPFRelIso() < 0.4)) return false;
    const float cutIB=0.5f, cutOB=-0.8f, cutEC=-0.5f;
    bool passMVAIDNoIsoCut = false;
    switch(etaRegion()) {
        case ETAREGION::IB:
            if (!(MvaNoIso() > cutIB)) passMVAIDNoIsoCut = true;
            break;
        case ETAREGION::OB:
            if (!(MvaNoIso() > cutOB)) passMVAIDNoIsoCut = true;
            break;
        case ETAREGION::EC:
            if (!(MvaNoIso() > cutEC)) passMVAIDNoIsoCut = true;
            break;
        default: break;
    }
    if (!(isMVANoIsoWP90() || passMVAIDNoIsoCut)) return false;
    return true;
}
