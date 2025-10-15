#include "Muon.h"
#include <cmath>

ClassImp(Muon)

Muon::Muon() {
    this->SetLeptonFlavour(MUON);
    // boolean ID bits
    j_looseId = false;
    j_mediumId = false;
    j_mediumPromptId = false;
    j_tightId = false;
    j_softId = false;
    j_softMvaId = false;
    j_triggerIdLoose = false;

    // unsigned char ID bits
    j_highPtId = 0;     // 1 = tracker high pT, 2 = global high pT
    j_miniIsoId = 0;    // 1=Loose, 2=Medium, 3=Tight, 4=VeryTight
    j_multiIsoId = 0;   // 1=Loose, 2=Medium
    j_mvaMuId = 0;      // 2=Medium, 3=Tight
    j_pfIsoId = 0;      // 1=VLoose, 2=Loose, 3=Medium, 4=Tight, 5=VTight, 6=VVTight
    j_puppiIsoId = 0;   // 1=Loose, 2=Medium, 3=Tight
    j_tkIsoId = 0;      // 1=Loose, 2=Tight

    // MVA ID scores
    j_softMva = -999.;
    j_mvaLowPt = -999.;
    j_mvaPrompt = -999.;

    // jet matching
    j_jetIdx = -1;
}

Muon::~Muon() {}

void Muon::AttachLazyPayload(void *context, EnsureCallback callback, int index) const {
    if (!lazy_) {
        lazy_ = std::make_shared<LazyPayload>();
    }
    lazy_->context = context;
    lazy_->callback = callback;
    lazy_->index = index;
    lazy_->loaded.reset();
}

void Muon::DetachLazyPayload() const {
    lazy_.reset();
}

void Muon::ensure(Property property) const {
    if (!lazy_) return;
    const auto bit = static_cast<std::size_t>(property);
    if (lazy_->loaded.test(bit)) return;
    if (!lazy_->callback) return;
    lazy_->callback(lazy_->context, const_cast<Muon &>(*this), property);
    lazy_->loaded.set(bit);
}

void Muon::markLoaded(Property property) const {
    if (!lazy_) return;
    lazy_->loaded.set(static_cast<std::size_t>(property));
}

void Muon::SetBIDBit(BooleanID id, bool idbit) {
    switch (id) {
        case BooleanID::LOOSE:
            j_looseId = idbit;
            markLoaded(Property::LooseId);
            break;
        case BooleanID::MEDIUM:
            j_mediumId = idbit;
            markLoaded(Property::MediumId);
            break;
        case BooleanID::MEDIUMPROMPT:
            j_mediumPromptId = idbit;
            markLoaded(Property::MediumPromptId);
            break;
        case BooleanID::TIGHT:
            j_tightId = idbit;
            markLoaded(Property::TightId);
            break;
        case BooleanID::SOFT:
            j_softId = idbit;
            markLoaded(Property::SoftId);
            break;
        case BooleanID::SOFTMVA:
            j_softMvaId = idbit;
            markLoaded(Property::SoftMvaId);
            break;
        case BooleanID::TRIGGERLOOSE:
            j_triggerIdLoose = idbit;
            markLoaded(Property::TriggerLooseId);
            break;
        default:
            break;
    }
}

void Muon::SetWIDBit(WorkingPointID id, unsigned char wp) {
    switch (id) {
        case WorkingPointID::HIGHPT:
            j_highPtId = static_cast<unsigned char>(wp + 1);
            markLoaded(Property::HighPtId);
            break;
        case WorkingPointID::MINIISO:
            j_miniIsoId = static_cast<unsigned char>(wp + 1);
            markLoaded(Property::MiniIsoId);
            break;
        case WorkingPointID::MULTIISO:
            j_multiIsoId = static_cast<unsigned char>(wp + 1);
            markLoaded(Property::MultiIsoId);
            break;
        case WorkingPointID::MVAMU:
            j_mvaMuId = static_cast<unsigned char>(wp + 2);
            markLoaded(Property::MvaMuId);
            break;
        case WorkingPointID::PFISO:
            j_pfIsoId = static_cast<unsigned char>(wp);
            markLoaded(Property::PfIsoId);
            break;
        case WorkingPointID::PUPPIISO:
            j_puppiIsoId = static_cast<unsigned char>(wp + 1);
            markLoaded(Property::PuppiIsoId);
            break;
        case WorkingPointID::TKISO:
            j_tkIsoId = static_cast<unsigned char>(wp + 1);
            markLoaded(Property::TkIsoId);
            break;
        default:
            break;
    }
}

void Muon::SetMVAID(MVAID id, float score) {
    switch (id) {
        case MVAID::SOFTMVA:
            j_softMva = score;
            markLoaded(Property::SoftMva);
            break;
        case MVAID::MVALOWPT:
            j_mvaLowPt = score;
            markLoaded(Property::MvaLowPt);
            break;
        case MVAID::MVAPROMPT:
            j_mvaPrompt = score;
            markLoaded(Property::MvaPrompt);
            break;
        default:
            break;
    }
}

bool Muon::PassID(const TString ID) const {
    if (ID == "")                 return true;
    if (ID == "NOCUT")            return true;
    if (ID == "POGTight")         return isPOGTightId();
    if (ID == "POGMedium")        return isPOGMediumId();
    if (ID == "POGMediumPrompt")  return isPOGMediumPromptId();
    if (ID == "POGLoose")         return isPOGLooseId();
    if (ID == "POGSoft")          return isPOGSoftId();
    if (ID == "POGSoftMVA")       return isPOGSoftMvaId();
    if (ID == "POGTriggerLoose")  return isPOGTriggerIdLoose();
    if (ID == "POGTrackerHighPt") return static_cast<int>(HighPtId()) == 1;
    if (ID == "POGGlobalHighPt")  return static_cast<int>(HighPtId()) == 2;
    if (ID == "POGMiniIsoLoose")  return static_cast<int>(MiniIsoId()) >= static_cast<int>(WorkingPoint::LOOSE);
    if (ID == "POGMiniIsoMedium") return static_cast<int>(MiniIsoId()) >= static_cast<int>(WorkingPoint::MEDIUM);
    if (ID == "POGMiniIsoTight")  return static_cast<int>(MiniIsoId()) >= static_cast<int>(WorkingPoint::TIGHT);
    if (ID == "POGMiniIsoVTight") return static_cast<int>(MiniIsoId()) >= static_cast<int>(WorkingPoint::VTIGHT);
    if (ID == "POGMultiIsoLoose") return static_cast<int>(MultiIsoId()) >= static_cast<int>(WorkingPoint::LOOSE);
    if (ID == "POGMultiIsoMedium")return static_cast<int>(MultiIsoId()) >= static_cast<int>(WorkingPoint::MEDIUM);
    if (ID == "POGMvaMuMedium")   return static_cast<int>(MvaMuId()) >= static_cast<int>(WorkingPoint::MEDIUM);
    if (ID == "POGMvaMuTight")    return static_cast<int>(MvaMuId()) >= static_cast<int>(WorkingPoint::TIGHT);
    if (ID == "POGPfIsoVLoose")   return static_cast<int>(PfIsoId()) >= static_cast<int>(WorkingPoint::VLOOSE);
    if (ID == "POGPfIsoLoose")    return static_cast<int>(PfIsoId()) >= static_cast<int>(WorkingPoint::LOOSE);
    if (ID == "POGPfIsoMedium")   return static_cast<int>(PfIsoId()) >= static_cast<int>(WorkingPoint::MEDIUM);
    if (ID == "POGPfIsoTight")    return static_cast<int>(PfIsoId()) >= static_cast<int>(WorkingPoint::TIGHT);
    if (ID == "POGPfIsoVTight")   return static_cast<int>(PfIsoId()) >= static_cast<int>(WorkingPoint::VTIGHT);
    if (ID == "POGPfIsoVVTight")  return static_cast<int>(PfIsoId()) >= static_cast<int>(WorkingPoint::VVTIGHT);
    if (ID == "POGPuppiIsoLoose") return static_cast<int>(PuppiIsoId()) >= static_cast<int>(WorkingPoint::LOOSE);
    if (ID == "POGPuppiIsoMedium")return static_cast<int>(PuppiIsoId()) >= static_cast<int>(WorkingPoint::MEDIUM);
    if (ID == "POGPuppiIsoTight") return static_cast<int>(PuppiIsoId()) >= static_cast<int>(WorkingPoint::TIGHT);
    if (ID == "POGTkIsoLoose")    return static_cast<int>(TkIsoId()) == 1;
    if (ID == "POGTkIsoTight")    return static_cast<int>(TkIsoId()) == 2;
    if (ID == "HcToWATight")      return Pass_HcToWATight();
    if (ID == "HcToWALoose")      return Pass_HcToWALoose();
    cerr << "[Muon::PassID] " << ID << " is not implemented." << endl;
    exit(ENODATA);

    return false;
}

bool Muon::PassID(const MuonID ID) const {
    switch(ID){
        case MuonID::NOCUT:
            return true;
        case MuonID::POG_TIGHT:
            return isPOGTightId();
        case MuonID::POG_MEDIUM:
            return isPOGMediumId();
        case MuonID::POG_MEDIUM_PROMPT:
            return isPOGMediumPromptId();
        case MuonID::POG_LOOSE:
            return isPOGLooseId();
        case MuonID::POG_SOFT:
            return isPOGSoftId();
        case MuonID::POG_SOFT_MVA:
            return isPOGSoftMvaId();
        case MuonID::POG_TRIGGER_LOOSE:
            return isPOGTriggerIdLoose();
        case MuonID::POG_TRACKER_HIGH_PT:
            return static_cast<int>(HighPtId()) == 1;
        case MuonID::POG_GLOBAL_HIGH_PT:
            return static_cast<int>(HighPtId()) == 2;
        case MuonID::POG_MINISO_LOOSE:
            return static_cast<int>(MiniIsoId()) >= static_cast<int>(WorkingPoint::LOOSE);
        case MuonID::POG_MINISO_MEDIUM:
            return static_cast<int>(MiniIsoId()) >= static_cast<int>(WorkingPoint::MEDIUM);
        case MuonID::POG_MINISO_TIGHT:
            return static_cast<int>(MiniIsoId()) >= static_cast<int>(WorkingPoint::TIGHT);
        case MuonID::POG_MINISO_VTIGHT:
            return static_cast<int>(MiniIsoId()) >= static_cast<int>(WorkingPoint::VTIGHT);
        case MuonID::POG_MULTISO_LOOSE:
            return static_cast<int>(MultiIsoId()) >= static_cast<int>(WorkingPoint::LOOSE);
        case MuonID::POG_MULTISO_MEDIUM:
            return static_cast<int>(MultiIsoId()) >= static_cast<int>(WorkingPoint::MEDIUM);
        case MuonID::POG_MVA_MU_MEDIUM:
            return static_cast<int>(MvaMuId()) >= static_cast<int>(WorkingPoint::MEDIUM);
        case MuonID::POG_MVA_MU_TIGHT:
            return static_cast<int>(MvaMuId()) >= static_cast<int>(WorkingPoint::TIGHT);
        case MuonID::POG_PFISO_VLOOSE:
            return static_cast<int>(PfIsoId()) >= static_cast<int>(WorkingPoint::VLOOSE);
        case MuonID::POG_PFISO_LOOSE:
            return static_cast<int>(PfIsoId()) >= static_cast<int>(WorkingPoint::LOOSE);
        case MuonID::POG_PFISO_MEDIUM:
            return static_cast<int>(PfIsoId()) >= static_cast<int>(WorkingPoint::MEDIUM);
        case MuonID::POG_PFISO_TIGHT:
            return static_cast<int>(PfIsoId()) >= static_cast<int>(WorkingPoint::TIGHT);
        case MuonID::POG_PFISO_VTIGHT:
            return static_cast<int>(PfIsoId()) >= static_cast<int>(WorkingPoint::VTIGHT);
        case MuonID::POG_PFISO_VVTIGHT:
            return static_cast<int>(PfIsoId()) >= static_cast<int>(WorkingPoint::VVTIGHT);
        case MuonID::POG_PUPPIISO_LOOSE:
            return static_cast<int>(PuppiIsoId()) >= static_cast<int>(WorkingPoint::LOOSE);
        case MuonID::POG_PUPPIISO_MEDIUM:
            return static_cast<int>(PuppiIsoId()) >= static_cast<int>(WorkingPoint::MEDIUM);
        case MuonID::POG_PUPPIISO_TIGHT:
            return static_cast<int>(PuppiIsoId()) >= static_cast<int>(WorkingPoint::TIGHT);
        case MuonID::POG_TKISO_LOOSE:
            return static_cast<int>(TkIsoId()) == 1;
        case MuonID::POG_TKISO_TIGHT:
            return static_cast<int>(TkIsoId()) == 2;
        default:
            break;
    }
    return false;
}

bool Muon::Pass_HcToWATight() const {
    if (! isPOGMediumId()) return false;
    if (! (fabs(dZ()) < 0.1)) return false;
    if (! (SIP3D() < 3.)) return false;
    if (! (TkRelIso() < 0.4*Pt())) return false;
    if (! (MiniPFRelIso() < 0.1)) return false;
    return true;
}

bool Muon::Pass_HcToWALoose() const {
    if (! isPOGMediumId()) return false;
    if (! (fabs(dZ()) < 0.1)) return false;
    if (! (SIP3D() < 5.)) return false;
    if (! (TkRelIso() < 0.4*Pt())) return false;
    if (! (MiniPFRelIso() < 0.6)) return false;
    return true;
}
