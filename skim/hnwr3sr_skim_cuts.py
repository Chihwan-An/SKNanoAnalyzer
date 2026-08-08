"""
HNWR 3SR pre-skim selection — the cut set for HNWR_BDT_presel_3SR.cc.

Same design rule as hnwr_skim_cuts.py (raw NanoAOD branches only, no object
correction ever enters), and it reuses that module's thresholds, trigger lists
and C++ helper verbatim. Exactly ONE number differs:

    MIN_N_LEPTON = 1      (hnwr_skim_cuts.py uses 2)

===============================================================================
WHY THE SECOND-LEPTON REQUIREMENT HAD TO GO
===============================================================================
hnwr_skim_cuts.py is built on a necessary condition read out of
Reproduce20_002_copy.cc: every region there needs a second lepton (resolved
wants two tight, boosted SR the same-flavour loose one, boosted flavour CR the
opposite-flavour one, boosted DY CR the low-mll one). HNWR_BDT_presel_3SR.cc
breaks that premise:

    // HNWR_BDT_presel_3SR.cc:578
    bool isBoostedNoLepCandidate = isBoostedBase && (n_boosted_cap_leptons == 0);

SR_*_BDTTree_boosted_nolep requires that there be NO capture lepton besides the
lead one — the N-side lepton is meant to be inside the fatjet or too soft to
reconstruct. That is the exact complement of ">= 2 loose leptons", so running
3SR over a Skim_HNWR_* sample does not merely lose events in that SR, it leaves
behind only the handful that survive for the wrong reason: a second lepton that
passed the skim at |eta| 2.5-2.6 (e) or 2.4-2.5 (mu) and is then dropped by the
analyzer's own SelectElectrons(..., 2.5) / SelectMuons(..., 2.4) at :224-225.
An eta-edge remnant masquerading as a signal region is worse than an empty one.

Dropping to MIN_N_LEPTON = 1 removes the conflict at the source.

===============================================================================
WHAT THE 3SR ANALYZER ACTUALLY NEEDS
===============================================================================
All three signal regions and every CR sit downstream of the SAME lead-lepton
requirement, and nothing else in the event selection is lepton-multiplicity
based:

  >= 1 tight lepton                       :310   (Tight tier: e POG_HEEP,
                                                  mu POG_GLOBAL_HIGH_PT +
                                                  TkRelIso < 0.1, both pT > 53)
  lead = Tight_leps[0], pT > 60           :489, :566
  lead passes trigger-safe pT + its HLT   :434-447
      electron channel   pT > 118  (Ele_Trigger_Safe_Pt_Cut, :97)
      muon channel       pT >  52  (Muon_Trigger_Safe_Pt_Cut, :96)
  HLT OR                                  :206-208

Combining the pT > 60 and the trigger-safe cut, the lead must have

      electron   pT > 118          muon   pT > 60

which is what LEAD_ELE_PT / LEAD_MU_PT are cut against, with margin.

The second lepton, where it exists, only CLASSIFIES the event; it never
selects it:

  SR boosted (with-lep)   captured SF pair, mll > 200        :577
  SR boosted_nolep        no capture lepton at all           :578
  SR resolved             ResSub sublead, mll > 200          :486-501
  capture lepton present but no mll > 200 pair -> no SR      (dropped, DY-rich)

So the skim must not cut on the sublead at all — and it does not have to, since
it prunes no branches: every Electron_* / Muon_* value of every surviving event
is still in the output, and the analyzer re-derives the Cap / ResSub tiers from
them. This matters because those tiers reach well outside what the skim would
be able to count as a lepton anyway:

  tier         3SR                                          in hnwr_skim_cuts?
  ------------------------------------------------------------------------
  Cap    (e)   LooseNoIso || HEEP || POG_MVANOISO_WP90  :251   MVA-only: no
  Cap    (mu)  POG_LOOSE                                :268   loose-only: no
  ResSub (e)   POG_LOOSE || HEEP                        :249   partly
  ResSub (mu)  POG_MEDIUM + MiniIso Loose               :266   no
  all four     pT > 20                                  :224   below 42: no

===============================================================================
THRESHOLD MARGINS
===============================================================================
  quantity            analyzer    skim    margin   what could move it
  ---------------------------------------------------------------------------
  lead electron pT       118       100     15 %    EGM scale/smear, ~1-2 %
  lead muon pT            60        45     25 %    Rochester <1 %, GE/TuneP
  electron |eta|         2.5       2.6       -     corrections do not move eta
  muon |eta|             2.4       2.5       -     corrections do not move eta

The electron margin is tighter than the 23 % of hnwr_skim_cuts.py because the
binding cut is now the trigger-safe 118 rather than Reproduce20_002_copy's lead
pT > 130. 15 % is still an order of magnitude above any EGM scale & smear shift,
and `verify_margin.py --cuts hnwr3sr` measures it directly on real files.

SUBLEAD_PT stays at 42 only because passLeptons() uses it to decide which
leptons to count; with MIN_N_LEPTON = 1 the count is satisfied by the lead
itself, so the value has no effect on which events are kept. It is inherited
rather than removed so the two cut sets stay diffable.

Jets, fat jets (incl. the >= 1 AK8 pT > 200 requirement at :576) and the mass
cuts are NOT applied, for the reason given in hnwr_skim_cuts.py: ScaleJets(...,
"total") can move AK4 pT by tens of percent between correction versions.

===============================================================================
RELATION TO Skim_HNWR_*
===============================================================================
This selection is a strict SUPERSET of hnwr_skim_cuts.py — identical trigger
list, identical thresholds, one fewer lepton required. Every event in a
Skim_HNWR_* file is also in the corresponding Skim_HNWR3SR_* file. So a
HNWR3SR skim can feed Reproduce20_002_copy and HNWR_BDT_presel as well, and
there is no reason to keep both for the same sample.

The cost is reduction power: the second-lepton cut is what does most of the
work on hadronic backgrounds (the 2023 TTLL measurement in skim/README.md shows
the lepton stage at 0.1397, and TTLL is the easiest case). Expect noticeably
larger output here, especially for QCD. Run one chunk with --dry-run first and
read the printed cutflow before committing to a full sample.
"""

import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import hnwr_skim_cuts as base  # noqa: E402

# --- thresholds -------------------------------------------------------------
#
# Inherited unchanged, except MIN_N_LEPTON. Importing rather than copying keeps
# a later retune of the shared numbers from silently applying to only one of the
# two skims.

LEAD_ELE_PT = base.LEAD_ELE_PT     # 100.0, analyzer 118 (trigger-safe)
LEAD_MU_PT = base.LEAD_MU_PT       #  45.0, analyzer  60
SUBLEAD_PT = base.SUBLEAD_PT       #  42.0, no effect at MIN_N_LEPTON = 1
ELE_MAX_ETA = base.ELE_MAX_ETA     #   2.6, analyzer 2.5
MU_MAX_ETA = base.MU_MAX_ETA       #   2.5, analyzer 2.4

MIN_N_LEPTON = 1                   # <-- the only real difference

# --- triggers ---------------------------------------------------------------
#
# The shared superset already covers the 3SR OR: HNWR_BDT_presel_3SR.cc:88-98
# uses {Mu50, CascadeMu100, HighPtTkMu100} + {Photon200, Ele115_CaloIdVT_GsfTrkIdT}
# for Run 3 and {Mu50, OldMu100, TkMu100} + {Ele35_WPTight_Gsf, Photon200,
# Ele115_CaloIdVT_GsfTrkIdT} for 2017, all of which are in TRIGGERS_RUN3 /
# TRIGGERS_RUN2.
#
# Note 3SR sets no trigger at all for 2016preVFP / 2016postVFP / 2018 (:84-98
# has an if for "2017" and one for the Run 3 eras, nothing else), so on those
# eras every event fails the HLT cut at :208. Skimming them for 3SR is pointless
# until that is filled in.

TRIGGERS_RUN3 = base.TRIGGERS_RUN3
TRIGGERS_RUN2 = base.TRIGGERS_RUN2
RUN2_ERAS = base.RUN2_ERAS
ERAS_WITHOUT_3SR_TRIGGERS = ("2016preVFP", "2016postVFP", "2018")

triggers_for_era = base.triggers_for_era

REQUIRED_BRANCHES = base.REQUIRED_BRANCHES

# Same helper, same passLeptons(): minNLepton is already a parameter, so no new
# C++ is needed. The header guard inside makes a second Declare() harmless.
CPP_HELPERS = base.CPP_HELPERS

trigger_filter_expression = base.trigger_filter_expression


def lepton_filter_expression():
    """RDataFrame filter string for the lepton requirement."""
    return (
        "hnwrskim::passLeptons("
        "Electron_pt, Electron_eta, Electron_cutBased_HEEP, Electron_vidNestedWPBitmap, "
        "Muon_pt, Muon_eta, Muon_tunepRelPt, Muon_highPtId, Muon_tightId, "
        f"{LEAD_ELE_PT}f, {LEAD_MU_PT}f, {SUBLEAD_PT}f, "
        f"{ELE_MAX_ETA}f, {MU_MAX_ETA}f, {MIN_N_LEPTON})"
    )
