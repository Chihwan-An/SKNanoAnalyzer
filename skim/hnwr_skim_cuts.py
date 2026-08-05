"""
HNWR pre-skim selection — the single source of truth for what gets thrown away.

This module is deliberately standalone: it imports nothing from SKNanoAnalyzer and
touches nothing but raw NanoAOD branch names. It is consumed by skim_hnwr.py.

===============================================================================
DESIGN RULE
===============================================================================
The skim reads ONLY raw NanoAOD branches. It never applies, and never depends
on, any object correction: no EGM scale & smear, no Rochester / Generalized
Endpoint, no JES, no JER, no MET propagation. That is the whole point — those
corrections are re-tuned constantly in this analysis, and a skim whose acceptance
moved with them would silently go stale every time one was updated.

Everything below is therefore expressed in terms of what is literally stored in
the file: Electron_pt, Muon_pt * Muon_tunepRelPt, the ID bitmaps, the HLT bits.

===============================================================================
WHY THESE CUTS CANNOT LOSE AN EVENT
===============================================================================
Every histogram fill in Analyzers/src/Reproduce20_002_copy.cc is downstream of
one of two branches, and both of them require the same three things:

  resolved (:1391)   n_Tight == 2, lead pT > 130(e) / 60(mu), sublead pT > 53
  boosted  (:2008)   n_Tight >= 1, lead pT > 130(e) / 60(mu)
  both     (:1444, :2041)   muon-trigger OR, or electron-trigger OR
  a second lepton    resolved needs 2 tight; boosted SR needs the same-flavour
                     loose lepton (:2422), boosted flavour CR needs the
                     opposite-flavour loose lepton (:2661), boosted DY CR needs
                     the low-mll loose lepton (:2052)

so the necessary condition for any fill at all is

      (HLT OR)  AND  (>= 2 loose leptons)  AND  (>= 1 hard lepton)

The trigger-safe pT cuts (electron 118 at :1395/:2018, muon 52 at
:1412/:1420/:2032) are strictly weaker than the lead-lepton thresholds, so they
add nothing here.

===============================================================================
THRESHOLD MARGINS
===============================================================================
Keep this table in sync with Reproduce20_002_copy.cc if its selection changes.

  quantity            analyzer    skim    margin   what could move it
  ---------------------------------------------------------------------------
  lead electron pT       130       100     23 %    EGM scale/smear, ~1-2 %
  lead muon pT            60        45     25 %    Rochester <1 %, GE/TuneP
  sublead lepton pT       53        42     21 %    as above
  electron |eta|         2.5       2.6       -     corrections do not move eta
  muon |eta|             2.4       2.5       -     corrections do not move eta

Jets, fat jets, and the m(lljj) / m(l+fatjet) > 800 GeV cut are deliberately NOT
applied. AnalyzerCore::ScaleJets(..., "total") multiplies 22 individual JES
sources together (AnalyzerCore.cc:483-528) rather than combining them in
quadrature, so AK4 jet pT can move by tens of percent between correction
versions. Leptons are the only objects stable enough to skim on.

MET noise filters (0.07 % rejection) and the jet veto map (5 %) are also not
applied: no meaningful saving, and both would couple the skim to code that does
change.

===============================================================================
A BUG IN THE OLD SKIMMER, FOR THE RECORD
===============================================================================
Analyzers/src/Skim_20002.cc:117 cut electrons at |eta| < 2.4 while the analyzer
uses 2.5, so it really did lose events. It also fell through to an unconditional
newtree->Fill() at :312, which is why its output has exactly the same number of
events as its input (measured: Skim_20002_ST_sch_top_Lep = 2,596,000 = nmc).
Neither mistake is repeated here.
"""

# --- thresholds -------------------------------------------------------------

LEAD_ELE_PT = 100.0   # analyzer 130
LEAD_MU_PT = 45.0     # analyzer  60
SUBLEAD_PT = 42.0     # analyzer  53
ELE_MAX_ETA = 2.6     # analyzer 2.5
MU_MAX_ETA = 2.5      # analyzer 2.4
MIN_N_LEPTON = 2

# --- triggers ---------------------------------------------------------------
#
# A superset of the ORs used by Reproduce20_002_copy.cc:28-66. Deliberately
# wider than the analyzer needs: measured on the 2023 cutflow, the trigger only
# rejects a further ~6 % once the lepton requirement is in place, so widening it
# costs almost nothing — and it means re-enabling e.g. HLT_Ele30_WPTight_Gsf
# later does not force a re-skim of 885 GB.
#
# Paths absent from a given file are dropped at runtime (some are in
# data/<ver>/<era>/Trigger/HLT_Path.json but not in every NanoAOD; e.g.
# HLT_Ele300_CaloIdVT_GsfTrkIdT is missing from the 2023 TTLL_powheg files).

TRIGGERS_RUN3 = [
    # muon
    "HLT_Mu50",
    "HLT_Mu55",
    "HLT_CascadeMu100",
    "HLT_HighPtTkMu100",
    "HLT_IsoMu24",
    # electron / photon
    "HLT_Photon200",
    "HLT_Ele115_CaloIdVT_GsfTrkIdT",
    "HLT_Ele300_CaloIdVT_GsfTrkIdT",
    "HLT_Ele30_WPTight_Gsf",
    "HLT_Ele32_WPTight_Gsf",
]

TRIGGERS_RUN2 = [
    # muon
    "HLT_Mu50",
    "HLT_OldMu100",
    "HLT_TkMu100",
    "HLT_IsoMu27",
    "HLT_IsoMu24",
    # electron / photon
    "HLT_Photon200",
    "HLT_Photon175",
    "HLT_Ele115_CaloIdVT_GsfTrkIdT",
    "HLT_Ele35_WPTight_Gsf",
    "HLT_Ele32_WPTight_Gsf",
    "HLT_Ele27_WPTight_Gsf",
]

RUN2_ERAS = ("2016preVFP", "2016postVFP", "2017", "2018")


def triggers_for_era(era):
    return list(TRIGGERS_RUN2 if era in RUN2_ERAS else TRIGGERS_RUN3)


# Branches the selection needs. Used to fail loudly and early rather than
# silently skimming on a missing collection.
REQUIRED_BRANCHES = [
    "Electron_pt",
    "Electron_eta",
    "Electron_cutBased_HEEP",
    "Electron_vidNestedWPBitmap",
    "Muon_pt",
    "Muon_eta",
    "Muon_tunepRelPt",
    "Muon_highPtId",
    "Muon_tightId",
]


# --- the selection itself, as C++ for cling to JIT --------------------------
#
# Templated on the container types so it does not matter whether RDataFrame
# hands us RVec<bool>, RVec<unsigned char>, etc.

CPP_HELPERS = r"""
#ifndef HNWR_SKIM_HELPERS
#define HNWR_SKIM_HELPERS

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace hnwrskim {

// Electron_vidNestedWPBitmap: 10 cuts x 3 bits.
//   0 MinPtCut                            5 GsfEleHadronicOverEMEnergyScaledCut
//   1 GsfEleSCEtaMultiRangeCut            6 GsfEleEInverseMinusPInverseCut
//   2 GsfEleDEtaInSeedCut                 7 GsfEleRelPFIsoScaledCut  <-- IGNORED
//   3 GsfEleDPhiInCut                     8 GsfEleConversionVetoCut
//   4 GsfEleFull5x5SigmaIEtaIEtaCut       9 GsfEleMissingHitsCut
// Each 3-bit value: 0 fail, 1 Veto, 2 Loose, 3 Medium, 4 Tight.
//
// Isolation (cut 7) is skipped on purpose. The mini-isolation studies
// (Analyzers/src/HNWR_miniiso*.cc) are actively changing what "isolated" means,
// so the skim must not bake in a definition of it.
//
// Mirrors Reproduce20_002_copy::Electrons::isPassLooseNoIso (:3878).
inline bool passLooseNoIso(int bitmap) {
    for (int cut_nr = 0; cut_nr < 10; ++cut_nr) {
        if (cut_nr == 7) continue;
        if (((bitmap >> (cut_nr * 3)) & 7) < 2) return false;
    }
    return true;
}

template <typename VElPt, typename VElEta, typename VElHeep, typename VElVid,
          typename VMuPt, typename VMuEta, typename VMuTuneP, typename VMuHighPt,
          typename VMuTight>
bool passLeptons(const VElPt &el_pt, const VElEta &el_eta,
                 const VElHeep &el_heep, const VElVid &el_vid,
                 const VMuPt &mu_pt, const VMuEta &mu_eta,
                 const VMuTuneP &mu_tunep, const VMuHighPt &mu_highpt,
                 const VMuTight &mu_tight,
                 float leadElePt, float leadMuPt, float subLeadPt,
                 float eleMaxEta, float muMaxEta, int minNLepton) {
    int nLep = 0;
    bool hasLead = false;

    for (std::size_t i = 0; i < el_pt.size(); ++i) {
        if (std::abs(el_eta[i]) >= eleMaxEta) continue;
        if (el_pt[i] <= subLeadPt) continue;
        // Loosest electron ID the analyzer ever uses
        // (Reproduce20_002_copy.cc:1253-1256). cutBased Tight is a subset of
        // looseNoIso, so switching the tight WP later stays covered.
        if (!(el_heep[i] || passLooseNoIso(el_vid[i]))) continue;
        ++nLep;
        if (el_pt[i] > leadElePt) hasLead = true;
    }

    for (std::size_t i = 0; i < mu_pt.size(); ++i) {
        if (std::abs(mu_eta[i]) >= muMaxEta) continue;
        // Whichever momentum estimator is larger, so that flipping
        // Muon_UseTuneP or moving the 200 GeV TuneP branch point
        // (AnalyzerCore.cc:736) cannot push a muon out of the skim.
        const float pt = std::max(float(mu_pt[i]), float(mu_pt[i] * mu_tunep[i]));
        if (pt <= subLeadPt) continue;
        // Analyzer loose muon is POG_GLOBAL_HIGH_PT (highPtId == 2);
        // accept tracker-highPt and POG Tight as well.
        if (!(int(mu_highpt[i]) >= 1 || mu_tight[i])) continue;
        ++nLep;
        if (pt > leadMuPt) hasLead = true;
    }

    return hasLead && nLep >= minNLepton;
}

}  // namespace hnwrskim
#endif
"""


def lepton_filter_expression():
    """RDataFrame filter string for the lepton requirement."""
    return (
        "hnwrskim::passLeptons("
        "Electron_pt, Electron_eta, Electron_cutBased_HEEP, Electron_vidNestedWPBitmap, "
        "Muon_pt, Muon_eta, Muon_tunepRelPt, Muon_highPtId, Muon_tightId, "
        f"{LEAD_ELE_PT}f, {LEAD_MU_PT}f, {SUBLEAD_PT}f, "
        f"{ELE_MAX_ETA}f, {MU_MAX_ETA}f, {MIN_N_LEPTON})"
    )


def trigger_filter_expression(available_triggers):
    """RDataFrame filter string for the trigger OR over paths present in the file."""
    if not available_triggers:
        raise RuntimeError("no trigger paths available in the input files")
    return "(" + " || ".join(available_triggers) + ")"
