#ifndef BDT_CR_h
#define BDT_CR_h

#include "AnalyzerCore.h"
#include "SystematicHelper.h"
#include "Muon.h"
#include "Electron.h"

// BDT control-region histogram producer.
// Uses the HNWR_BDT_presel object/topology definitions, then fills
// Reproduce20_002_copy-style histogram names:
// - resolved DY CR: exactly 2 tight SF leptons, 60 < mll < 150, >=2 jets, resolved dR pass
// - resolved flavor CR: exactly 2 tight OF leptons, mll > 200, >=2 jets, resolved dR pass
// - boosted DY CR: leading tight lepton + extra loose SF lepton, 60 < mll < 150, fatjet >= 1, resolved dR fail
// - boosted flavor CR: leading tight lepton + extra loose OF lepton, mll > 200, fatjet >= 1, resolved dR fail
// No mlljj/WRMass > 800 cut is applied.
// Histograms are filled for Central plus weight-only systematic up/down variations.
// JES/JER object variations and SS/OS split histograms are intentionally omitted.
class BDT_CR : public AnalyzerCore {
public:
    BDT_CR();
    ~BDT_CR();

    void initializeAnalyzer();
    void executeEvent();
    void executeEventFromParameter();

    unique_ptr<SystematicHelper> systHelper;

    struct Electrons {
        RVec<Electron> AllElectrons;

        RVec<Electron::ElectronID> Electron_Tight_ID = {Electron::ElectronID::POG_HEEP};
        RVec<Electron::ElectronID> Electron_Loose_ID = {Electron::ElectronID::POG_LOOSE};
        float Electron_MinPt = 53.;

        RVec<std::string> Ele_Trigger;
        float Ele_Trigger_Safe_Pt_Cut = 0.;

        bool isPassCustomLooseID(const Electron& el) const;
        bool isPassCustomTightID(const Electron& el, const BDT_CR::Electrons& eset) const;
        bool isPassLooseNoIso(const Electron& el) const;
    } el_set;

    struct Muons {
        RVec<Muon> AllMuons;

        RVec<Muon::MuonID> Muon_Tight_ID = {Muon::MuonID::POG_GLOBAL_HIGH_PT};
        RVec<Muon::MuonID> Muon_Loose_ID = {Muon::MuonID::POG_GLOBAL_HIGH_PT};
        float Muon_MinPt = 53.;

        RVec<std::string> Muon_Trigger;
        float Muon_Trigger_Safe_Pt_Cut = 0.;
        float Muon_Iso_Cut = 0.1;
    } mu_set;

    struct Jets {
        RVec<Jet> AllJets;
        RVec<Jet::JetID> Jet_ID = {Jet::JetID::TIGHTLEPVETO};
        float Jet_MinPt = 40.;
        float Jet_MaxEta = 2.5;
    } jet_set;

    struct FatJets {
        RVec<FatJet> AllFatJets;
        TString FatJet_ID = "Tight";
        float Fatjet_LSF = 0.75;
        float FatJet_MinPt = 200.;
        float FatJet_MaxEta = 2.5;
        float FatJet_SDM = 40;
    } fatjet_set;

    RVec<FatJet> Clean_Fatjet_with_tight_leptons(const RVec<FatJet>& fatjets, const RVec<Lepton*>& tight_leps);
    RVec<Jet> Clean_jet_with_loose_leptons(const RVec<Jet>& jets, const RVec<Lepton*>& loose_leps);

    float dR_Separation = 0.4;
};

#endif
