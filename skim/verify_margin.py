#!/usr/bin/env python3
"""
Prove the skim cannot lose an event the analyzer would have used.

The skim cuts on raw pT with thresholds well below the analyzer's. Whether that
margin is big enough is an empirical question, and this script answers it
directly: it scales every lepton pT by a factor f -- standing in for any
correction, of any size, applied coherently in the worst possible direction --
and counts events that

    the analyzer's necessary condition WOULD accept at that scale
    but the skim (which sees only raw pT) has thrown away.

If that count is zero up to some f, then no correction smaller than f can cost
the analysis a single event.

Measured on 2023 TTLL_powheg (183k events, the hardest case since it genuinely
has two prompt leptons):

    correction   analyzer would use   LOST by skim
      x1.00             8309                 0
      x1.20            13528                 0
      x1.26            15306                 0
      x1.28            15898               308
      x1.50            22443              6407

i.e. zero loss up to a coherent +26 % shift on every lepton. Real corrections
are: electron EGM scale/smear 1-2 %, muon Rochester below 1 %, Generalized
Endpoint ~11 % only at 1 TeV (where muons are far above threshold anyway).

Uses uproot + awkward, so it needs no ROOT and no SKNanoAnalyzer build.

  ./verify_margin.py --era 2023 file1.root [file2.root ...]
"""

import argparse
import os
import sys

import awkward as ak
import numpy as np
import uproot

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import hnwr_skim_cuts as cuts  # noqa: E402

# The analyzer's necessary condition, on corrected pT
# (Reproduce20_002_copy.cc:1391 resolved, :2008 boosted).
ANA_LEAD_ELE_PT = 130.0
ANA_LEAD_MU_PT = 60.0
ANA_SUBLEAD_PT = 53.0

DEFAULT_SCALES = [1.00, 1.05, 1.10, 1.15, 1.20, 1.25, 1.26, 1.28, 1.30, 1.35, 1.50]


def loose_masks(arr):
    """Loose lepton masks and pT, exactly as hnwr_skim_cuts defines them."""
    vid = arr["Electron_vidNestedWPBitmap"]
    loose = ak.ones_like(vid, dtype=bool)
    for cut_nr in range(10):
        if cut_nr == 7:  # isolation, deliberately ignored
            continue
        loose = loose & (((vid >> (cut_nr * 3)) & 7) >= 2)

    el_ok = (abs(arr["Electron_eta"]) < cuts.ELE_MAX_ETA) & (
        arr["Electron_cutBased_HEEP"] | loose
    )
    mu_ok = (abs(arr["Muon_eta"]) < cuts.MU_MAX_ETA) & (
        (arr["Muon_highPtId"] >= 1) | arr["Muon_tightId"]
    )
    el_pt = ak.mask(arr["Electron_pt"], el_ok)
    mu_pt = ak.mask(
        np.maximum(arr["Muon_pt"], arr["Muon_pt"] * arr["Muon_tunepRelPt"]), mu_ok
    )
    return el_pt, mu_pt


def main():
    p = argparse.ArgumentParser(description=__doc__,
                                formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("files", nargs="+")
    p.add_argument("--era", required=True)
    p.add_argument("--scales", nargs="*", type=float, default=DEFAULT_SCALES)
    args = p.parse_args()

    wanted = cuts.triggers_for_era(args.era)

    total = 0
    n_skim = 0
    n_ana = {f: 0 for f in args.scales}
    n_lost = {f: 0 for f in args.scales}

    for path in args.files:
        events = uproot.open(path)["Events"]
        present = set(events.keys())
        trigs = [t for t in wanted if t in present]
        if not trigs:
            sys.exit(f"{path}: none of the {args.era} trigger paths present")

        arr = events.arrays(cuts.REQUIRED_BRANCHES + trigs, library="ak")

        trig = np.zeros(len(arr), dtype=bool)
        for t in trigs:
            trig |= ak.to_numpy(arr[t])

        el_pt, mu_pt = loose_masks(arr)
        as_int = lambda x: ak.to_numpy(ak.fill_none(x, 0))       # noqa: E731
        as_bool = lambda x: ak.to_numpy(ak.fill_none(x, False))  # noqa: E731

        skim = (
            trig
            & as_bool(
                (ak.sum(el_pt > cuts.LEAD_ELE_PT, axis=1) > 0)
                | (ak.sum(mu_pt > cuts.LEAD_MU_PT, axis=1) > 0)
            )
            & (
                as_int(
                    ak.sum(el_pt > cuts.SUBLEAD_PT, axis=1)
                    + ak.sum(mu_pt > cuts.SUBLEAD_PT, axis=1)
                )
                >= cuts.MIN_N_LEPTON
            )
        )

        total += len(arr)
        n_skim += int(skim.sum())

        for f in args.scales:
            e, m = el_pt * f, mu_pt * f
            lead = (ak.sum(e > ANA_LEAD_ELE_PT, axis=1) > 0) | (
                ak.sum(m > ANA_LEAD_MU_PT, axis=1) > 0
            )
            nlep = ak.sum(e > ANA_SUBLEAD_PT, axis=1) + ak.sum(m > ANA_SUBLEAD_PT, axis=1)
            ana = trig & as_bool(lead) & (as_int(nlep) >= 2)
            n_ana[f] += int(ana.sum())
            n_lost[f] += int((ana & ~skim).sum())

    print(f"files {len(args.files)}   total {total}   skim keeps {n_skim} "
          f"({n_skim / total:.4f}, {total / n_skim if n_skim else 0:.1f}x reduction)\n")
    print("  correction   analyzer would use   LOST by skim")
    safe_up_to = None
    for f in args.scales:
        flag = "" if n_lost[f] else "  <- safe"
        if not n_lost[f]:
            safe_up_to = f
        print(f"    x{f:.2f}      {n_ana[f]:>10}          {n_lost[f]:>8}{flag}")

    if safe_up_to is not None:
        print(f"\nno event lost up to a coherent +{100 * (safe_up_to - 1):.0f} % "
              f"shift on every lepton pT")
    if n_lost[args.scales[0]]:
        print("\nWARNING: events are lost already at the smallest scale tested -- "
              "the skim thresholds do not match the analyzer's necessary condition")
        sys.exit(1)


if __name__ == "__main__":
    main()
