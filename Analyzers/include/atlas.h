#ifndef atlas_h
#define atlas_h

#include "AnalyzerCore.h"
#include "SystematicHelper.h"
#include "Electron.h"
#include "Muon.h"
#include "Jet.h"
#include "FatJet.h"

class atlas : public AnalyzerCore {
public:
    atlas();
    ~atlas();

    void initializeAnalyzer();
    void executeEvent();
    void executeEventFromParameter();

    unique_ptr<SystematicHelper> systHelper;

    struct TriggerConfig {
        RVec<TString> ResolvedEE;
        RVec<TString> ResolvedMM;
        RVec<TString> ResolvedEM;
        RVec<TString> BoostedElectron;
        RVec<TString> BoostedMuon;
    } triggers;

    struct AnalysisCuts {
        float resolved_electron_pt = 25.;
        float resolved_muon_pt = 25.;
        float resolved_leading_lepton_pt = 40.;
        float resolved_jet_pt = 100.;
        float resolved_smallr_jet_pt = 20.;
        float resolved_mll_sr = 400.;
        float resolved_mjj = 110.;
        float resolved_ht = 400.;
        float resolved_ss_max_drll = 3.9;

        float boosted_electron_pt = 25.;
        float boosted_muon_pt = 28.;
        float boosted_leading_lepton_pt = 200.;
        float boosted_large_r_jet_pt = 200.;
        float boosted_large_r_jet_eta = 2.0;
        float boosted_dphi_lep_large_r_jet = 2.0;
        float boosted_met = 200.;
        float boosted_mll = 200.;
        float boosted_dilepton_pt = 200.;
        float boosted_cos_theta = 0.7;
        float boosted_delta_eta_e_large_r_jet = 2.0;

        float electron_eta = 2.47;
        float muon_eta = 2.5;
        float smallr_jet_eta = 2.5;
        float overlap_dr = 0.4;
    } cuts;

    RVec<Electron> AllElectrons;
    RVec<Muon> AllMuons;
    RVec<Jet> AllJets;
    RVec<FatJet> AllFatJets;

    JetTagging::JetFlavTagger btagger;
    JetTagging::JetFlavTaggerWP btag_wp_name;
    float btag_wp;

    void setupTriggers();

    float eventWeight(Event& ev, const TString& syst);

    bool passAtlasElectronEta(const Electron& electron) const;
    bool passResolvedElectron(const Electron& electron) const;
    bool passBoostedElectron(const Electron& electron) const;
    bool passBoostedLeadingElectron(const Electron& electron) const;
    bool passResolvedMuon(const Muon& muon) const;
    bool passBoostedMuon(const Muon& muon) const;
    bool passBoostedLeadingMuon(const Muon& muon) const;

    RVec<Electron> selectResolvedElectrons(const RVec<Electron>& electrons) const;
    RVec<Electron> selectBoostedElectrons(const RVec<Electron>& electrons) const;
    RVec<Muon> selectResolvedMuons(const RVec<Muon>& muons) const;
    RVec<Muon> selectBoostedMuons(const RVec<Muon>& muons) const;
    RVec<Jet> selectSmallRJets(const RVec<Jet>& jets, float ptmin) const;
    RVec<FatJet> selectLargeRJets(const RVec<FatJet>& fatjets) const;

    RVec<Lepton*> makeLeptonPtrs(RVec<Electron>& electrons, RVec<Muon>& muons) const;
    RVec<Jet> cleanJetsWithLeptons(const RVec<Jet>& jets, const RVec<Lepton*>& leptons, float dr) const;
    int countBTaggedJets(const RVec<Jet>& jets) const;
    bool isBTagged(const Jet& jet) const;

    bool passResolvedTrigger(const Event& ev, const TString& channel) const;
    bool passBoostedTrigger(const Event& ev, const Lepton& leading_lepton) const;

    float calculateCosTheta(const Lepton& lepton, const Particle& met) const;
    TString chargeCategory(const Lepton& lep1, const Lepton& lep2) const;
    TString resolvedChannelName(const TString& flavor, const TString& charge) const;

    void fillCutflowBin(const TString& histname, int step, float weight, int nbins = 20);

    void fillResolved(const Event& ev, const Particle& met, float weight, const TString& syst);
    void fillBoosted(const Event& ev, const Particle& met, float weight, const TString& syst);
    void fillResolvedCandidate(const TString& base,
                               const Lepton& lep1,
                               const Lepton& lep2,
                               const Jet& jet1,
                               const Jet& jet2,
                               float weight,
                               const TString& syst);
    void fillBoostedCandidate(const TString& base,
                              const Particle& wr_candidate,
                              const FatJet& large_r_jet,
                              float weight,
                              const TString& syst);
};

#endif
