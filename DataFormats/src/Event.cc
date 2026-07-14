#include "Event.h"

#include <iostream>
#include <stdexcept>

ClassImp(Event)

Event::Event() {
    j_nPV = -999;
    j_nTrueInt = -999.;
    j_nPVsGood = -999;
    j_DataYear = -999;
    j_DataEra = "";
}

Event::~Event() {}

void Event::SetTrigger(const TriggerDecisionMap_t& map) {
    j_HLTmap = &map;
    j_triggerProvider = nullptr;
    j_triggerProviderEpoch = 0;
}

void Event::SetTriggerProvider(const SKNano::TriggerDecisionProvider *provider) {
    j_HLTmap = nullptr;
    j_triggerProvider = provider;
    j_triggerProviderEpoch = provider ? provider->triggerEpoch() : 0;
}

void Event::AssertCurrentTriggerProvider() const {
    if (j_triggerProvider &&
        j_triggerProvider->triggerEpoch() != j_triggerProviderEpoch) {
        throw SKNano::LogicError(
            "[Event] stale trigger provider access after event transition");
    }
}

bool Event::PassTrigger(const TString trig) const {
    if (j_triggerProvider) {
        AssertCurrentTriggerProvider();
        SKNano::TriggerDecision decision;
        if (!j_triggerProvider->lookupTrigger(trig.Data(), decision)) {
            std::cout << "[Event::PassTrigger] WARNING: " << trig
                      << " not found\n";
            return false;
        }
        return decision.pass;
    }
    if (!j_HLTmap) {
        std::cerr << "[Event::PassTrigger] HLT decisions not set\n";
        return false;
    }
    auto it = j_HLTmap->find(trig.Data());
    if (it == j_HLTmap->end()) {
        std::cout << "[Event::PassTrigger] WARNING: " << trig
                  << " not found\n";
        return false;
    }
    return it->second.pass;
}

bool Event::PassTrigger(const RVec<TString> trigs) const {
    for (const auto &trig: trigs) {
        if (PassTrigger(trig)) return true;
    }
    return false;
}

// NOTE
// trigger lumi calcuated from brilcalc
// e.g. brilcalc lumi -b "STABLE BEAMS" \
//               -u /pb -i /afs/cern.ch/user/c/choij/private/brilcalc/json/Run3/2023.json \
//               --hltpath "HLT_IsoMu24_v*"
// /cvmfs/cms-bril.cern.ch/cms-lumi-pog/Normtags/normtag_PHYSICS.json for 2022, normtag_BRIL.json for 2023 (2024.07.11)
float Event::GetTriggerLumi(const TString& trig) const
{
    if (j_triggerProvider) {
        AssertCurrentTriggerProvider();
        SKNano::TriggerDecision decision;
        return j_triggerProvider->lookupTrigger(trig.Data(), decision)
                   ? decision.lumi
                   : -999.f;
    }
    if (!j_HLTmap) {                     
        std::cerr << "[Event::GetTriggerLumi] HLT map not set\n";
        return -999.f;
    }

    const auto it = j_HLTmap->find(trig.Data());
    if (it == j_HLTmap->end()) {
        std::cerr << "[Event::GetTriggerLumi] Trigger " << trig
                  << " not found\n";
        return -999.f;
    }

    return it->second.lumi;
}

bool Event::IsPDForTrigger(TString trig, TString PD) {
    cerr << "WARNING: IsPDForTrigger() not implemented yet" << endl;
    return false;
}

void Event::SetMET(RVec<float> MET_pt, RVec<float> MET_phi) {
    if (MET_pt.size() < 3 || MET_phi.size() < 3)
        throw std::runtime_error("[Event::SetMET] PUPPI MET requires central, unclustered up, and unclustered down values");
    j_METVector_PUPPI.SetPtEtaPhiM(MET_pt[0], 0, MET_phi[0], 0);
    j_METVector_PUPPI_UE_UP.SetPtEtaPhiM(MET_pt[1], 0, MET_phi[1], 0);
    j_METVector_PUPPI_UE_Down.SetPtEtaPhiM(MET_pt[2], 0, MET_phi[2], 0);
}

Particle Event::GetMETVector(Event::MET_Type MET_type, SKNano::Variation syst, Event::MET_Syst source) const
{
    if(MET_type!=MET_Type::PUPPI) {
        throw SKNano::LogicError("[Event::GetMETVector] Only PUPPI MET is implemented");
    }
    switch (syst) {
        case SKNano::Variation::nom:
            switch (source) {
                case MET_Syst::CENTRAL:
                    return j_METVector_PUPPI;
                default:
                    throw SKNano::LogicError("[Event::GetMETVector] Source is not MET_Syst::CENTRAL but variation is nominal");
            }
        case SKNano::Variation::up:
            switch (source) {
                case MET_Syst::UE:
                    return j_METVector_PUPPI_UE_UP;
                case MET_Syst::JER:
                case MET_Syst::JES:
                    throw SKNano::EventDataError("[Event::GetMETVector] JER/JES MET shifts are propagated from corrected jets; direct Event MET storage only contains unclustered shifts");
                default:
                    throw SKNano::LogicError("[Event::GetMETVector] Source is not MET_Syst::UE, JER, or JES but variation is up");
            }
        case SKNano::Variation::down:
            switch (source) {
                case MET_Syst::UE:
                    return j_METVector_PUPPI_UE_Down;
                case MET_Syst::JER:
                case MET_Syst::JES:
                    throw SKNano::EventDataError("[Event::GetMETVector] JER/JES MET shifts are propagated from corrected jets; direct Event MET storage only contains unclustered shifts");
                default:
                    throw SKNano::LogicError("[Event::GetMETVector] Source is not MET_Syst::UE, JER, or JES but variation is down");
            }
        default:
            throw SKNano::LogicError("[Event::GetMETVector] Unknown variation");
    }
}
