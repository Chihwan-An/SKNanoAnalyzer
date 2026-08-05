#!/usr/bin/env python3
"""
Compare two analyzer output files bin by bin: raw NanoAOD vs pre-skimmed input.

If the skim is sound these must agree everywhere except the cutflow bins that
count events before the skim's own cuts -- the skim removed those from the
denominator on purpose.

  EXPECTED to differ
    */Cutflow_for_skim              every bin
    */Cutflow_for_Boosted_SR        bins 1-3   ("All", "Noise filter", "Not resolved")
    */Cutflow_for_reseolved_SR      bins 1-2   ("All", "Noise filter")
    */CutFlow                       low bins
  EXPECTED to be IDENTICAL
    everything else -- every SR/CR distribution, in every systematic directory

Reproduce20_002_copy writes ~150k histograms across 41 systematic directories,
which takes a few minutes per file to read. Use --dirs for a quick look first.

  # quick: nominal only, ~30 s
  ./compare_hists.py raw.root skim.root --dirs Central

  # thorough: everything, ~15 min
  ./compare_hists.py raw.root skim.root

  # the kinematic systematics, where a too-tight skim would bite first
  ./compare_hists.py raw.root skim.root --dirs Central 'JES_*' 'JER_*' \\
      'ElectronScale_*' 'ElectronRes_*' 'MuonScale_*' 'FatJet*'
"""

import argparse
import fnmatch
import re
import sys
import time

import numpy as np
import uproot

# Histograms that legitimately differ, with the highest bin index allowed to.
# values(flow=True) index 0 is the underflow bin, so the index equals ROOT's bin
# number. NOTE FillHist(h, N, ...) fills the bin CONTAINING the value N, i.e.
# ROOT bin N+1 -- the cutoffs below are ROOT bin numbers, not FillHist values.
#
# This list is a liability: every entry is a place where a real regression could
# hide. Each one is here because the skim's own cuts (trigger OR, >=2 leptons)
# legitimately remove events that the analyzer still counts at that stage, and
# each cutoff was read off a verified-clean A/B run rather than guessed. Do not
# widen it to make a FAIL go away -- find out why the bin moved first.
#
# Measured on TTLL_powheg (2023, 1 file), fresh build, 91955 histograms:
#   Cutflow_for_Boosted_SR    ROOT bins 2-9 differ; bin 10 (FillHist 9.0 =
#     `hassflooselepton`, Reproduce20_002_copy.cc:2550) onward is bit-identical
#     -- the moment a second lepton is required, raw and skim agree.
#   Cutflow_for_reseolved_SR  ROOT bins 2-4 differ; bin 5 (FillHist 4.0 =
#     trigger, :1445) onward is bit-identical.
#   All 1084 SR/CR physics distributions were bit-identical.
EXPECTED_DIFF = [
    # cutflows, up to the stage where the skim's cuts become the analyzer's
    (re.compile(r"Cutflow_for_skim"), 10**9),
    (re.compile(r"Cutflow_for_Boosted_SR(_raw)?$"), 9),
    (re.compile(r"Cutflow_for_reseolved_SR(_raw)?$"), 4),
    (re.compile(r"Cutflow_for_e_mujet$"), 10**9),
    (re.compile(r"Cutflow_for_mu_ejet$"), 10**9),
    (re.compile(r"Boost_cutflow_(DY|FLV)$"), 10**9),
    (re.compile(r"(^|/)CutFlow$"), 10**9),
    # object/multiplicity monitors filled before any region requirement, so they
    # count exactly the events the skim is designed to drop
    (re.compile(r"(^|/)(N_Loose_Lepton|N_Tight_Lepton|Boost_tightlepnum|"
                r"tightmuons|Num_of_ExtraTightLepton|"
                r"Electron_Tight_and_Loose_ID_Check|"
                r"Check_is_tmp_lead_muon_ok|Tightleps_pt_(lead|sublead)|"
                r"Selected_Jetnum|Non_Selected_Jetnum|"
                r"Jet_num_total_(before|after)_clean_looselep|"
                r"JESJER_(before|after)_jetpt|"
                r"Fatjet_(pt|eta|SDM|LSF3)_beforecut|"
                r"Fatjet_(num_aftercut|LSF_num_aftercut|num_total"
                r"(_before_clean_tightlep|_after_clean_tightlep)?)|"
                r"deltaPhi_LeadLep_Fatjet_FLV|numofhnfatjet_FLV)$"), 10**9),
]


def allowed_bin(name, ibin):
    return any(p.search(name) and ibin <= mx for p, mx in EXPECTED_DIFF)


def load(path, dir_patterns, label):
    """Stream every TH1 into {name: values array}.

    When --dirs is given we descend into the matching top-level directories
    directly. Filtering the output of a whole-file iteritems() would still
    decompress all ~150k histograms first, which is the slow part.
    """
    out = {}
    t0 = time.time()
    with uproot.open(path) as f:
        if dir_patterns:
            tops = [k.split(";")[0] for k in f.keys(recursive=False, cycle=False)]
            selected = [t for t in tops
                        if any(fnmatch.fnmatch(t, p) for p in dir_patterns)]
            if not selected:
                sys.exit(f"{path}: no top-level directory matches {dir_patterns}\n"
                         f"  available: {', '.join(sorted(tops))}")
            for top in selected:
                for name, obj in f[top].iteritems(recursive=True,
                                                  filter_classname="TH1*"):
                    out[f"{top}/{name.split(';')[0]}"] = obj.values(flow=True)
        else:
            for name, obj in f.iteritems(recursive=True, filter_classname="TH1*"):
                out[name.split(";")[0]] = obj.values(flow=True)
    print(f"  {label:<5} {len(out):>7} histograms   {time.time() - t0:.0f} s")
    return out


def main():
    p = argparse.ArgumentParser(description=__doc__,
                                formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("raw")
    p.add_argument("skim")
    p.add_argument("--dirs", nargs="*", default=None,
                   help="only these top-level directories (glob), e.g. Central 'JES_*'")
    p.add_argument("--tolerance", type=float, default=1e-9,
                   help="relative tolerance on bin contents (default 1e-9)")
    p.add_argument("--show", type=int, default=25, help="max problems to print")
    args = p.parse_args()

    print(f"raw  {args.raw}")
    print(f"skim {args.skim}")
    if args.dirs:
        print(f"dirs {' '.join(args.dirs)}")
    print("reading:")
    hr = load(args.raw, args.dirs, "raw")
    hs = load(args.skim, args.dirs, "skim")

    only_raw = sorted(set(hr) - set(hs))
    only_skim = sorted(set(hs) - set(hr))
    common = sorted(set(hr) & set(hs))

    real_problems = []
    n_expected = 0
    n_compared = 0

    for name in common:
        a, b = hr[name], hs[name]
        if a.shape != b.shape:
            real_problems.append((name, None, "shape", a.shape, b.shape))
            continue
        n_compared += 1

        a, b = a.ravel(), b.ravel()
        denom = np.maximum(np.maximum(np.abs(a), np.abs(b)), 1e-300)
        bad = np.abs(a - b) / denom > args.tolerance
        if not bad.any():
            continue

        for i in np.flatnonzero(bad):
            if allowed_bin(name, int(i)):
                n_expected += 1
            else:
                real_problems.append((name, int(i), "value", a[i], b[i]))

    print()
    if only_raw:
        print(f"!! {len(only_raw)} histogram(s) only in RAW -- the skim lost these "
              "regions entirely:")
        for n in only_raw[: args.show]:
            print(f"     {n}")
        print()
    if only_skim:
        print(f"?? {len(only_skim)} histogram(s) only in SKIM (unexpected):")
        for n in only_skim[: args.show]:
            print(f"     {n}")
        print()

    print(f"compared {n_compared} histograms bin by bin (tolerance {args.tolerance:g})")
    print(f"  expected differences : {n_expected} bins (pre-skim cutflow bins -- fine)")
    print(f"  REAL differences     : {len(real_problems)}")

    if real_problems:
        print("\nfirst mismatches:")
        for name, ibin, kind, x, y in real_problems[: args.show]:
            if kind == "shape":
                print(f"  {name}: shape {x} vs {y}")
            else:
                print(f"  {name}  bin {ibin}:  raw {x!r}  skim {y!r}  diff {x - y!r}")

    ok = not real_problems and not only_raw
    print("\n" + ("PASS -- skim is lossless for the analysis"
                  if ok else "FAIL -- the skim changed the result"))
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
