#!/usr/bin/env python3
"""
Write CommonSampleInfo.json entries for samples that were skimmed with
--dataset-base, i.e. straight off the storage element with no bookkeeping entry
of their own.

SKNano.py needs {isMC, PD, xsec, nmc, sumsign, sumW} for every sample it
normalises (SKNano.py:345-346 injects sumW / sumsign into the job macro, and
AnalyzerCore::MCweight() divides by sumSign because usesign defaults to true).
Two of those come from different places:

  xsec           physics, independent of the NanoAOD version -- copied from an
                 existing bookkeeping directory (--xsec-from, default the v13
                 tree, which does carry the EM/Mu-enriched QCD entries).
  nmc/sumW       depends on the exact files processed, so it is MEASURED from
                 the Runs tree the skim copied unfiltered. Do NOT copy these
                 from --xsec-from: the v12 and v13 productions of the same
                 dataset can differ (2022 QCD_PT-300toInf_EMEnriched has 908512
                 events here against v13's 891941), and a wrong sumW is a
                 silent normalisation error.

xsec is matched on the **PD** field rather than the alias, because CMS named the
same bin differently across campaigns: v13's QCD_Pt-300toInf_EMEnriched points
at QCD_PT-300_EMEnriched_... in 2022EE and QCD_PT-300toInf_EMEnriched_... in the
other eras. Matching on PD resolves that automatically, and a dataset with no
matching PD (the duplicate QCD_PT-300_EMEnriched that also exists in 2023 /
2023BPix) is reported and skipped rather than guessed at.

sumsign is set to nmc, which --verify-sign checks by counting genWeight signs in
the original NanoAOD. Measured on 2022: zero negative-weight events in all seven
EM-enriched bins.

  ./make_sample_info.py --era 2022 \\
      --skim-base /u/user/$USER/SE_UserHome/Skim_sample/QCD \\
      --dataset-base /pnfs/knu.ac.kr/data/cms/store/mc --dry-run
"""

import argparse
import glob
import json
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from submit_skim import CAMPAIGNS, TUNE_TAIL  # noqa: E402

import ROOT  # noqa: E402

ROOT.gROOT.SetBatch(True)


def runs_totals(files):
    """(genEventCount, genEventSumw) summed over the copied Runs tree."""
    n = w = 0.0
    for path in files:
        fh = ROOT.TFile.Open(path)
        if not fh or fh.IsZombie():
            sys.exit(f"cannot open {path}")
        runs = fh.Get("Runs")
        if not runs:
            sys.exit(f"{path} has no Runs tree -- it cannot be normalised. "
                     "A skim written straight to dCache loses it; see "
                     "submit_skim.py --stage-out")
        for entry in runs:
            n += entry.genEventCount
            w += entry.genEventSumw
        fh.Close()
    return n, w


def sign_totals(files):
    """(nEvents, sum of sign(genWeight)) over the ORIGINAL NanoAOD."""
    df = ROOT.RDataFrame("Events", files)
    df = df.Define("sgn", "genWeight>0 ? 1.0 : (genWeight<0 ? -1.0 : 0.0)")
    n, s = df.Count(), df.Sum("sgn")
    return int(n.GetValue()), float(s.GetValue())


def main():
    p = argparse.ArgumentParser(description=__doc__,
                                formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--era", required=True)
    p.add_argument("--skim-base", required=True,
                   help="the --outbase used for the skim (flat layout)")
    p.add_argument("--dataset-base", required=True,
                   help="storage element root, to recover each alias's full PD name")
    p.add_argument("--campaign", help="override the era's CAMPAIGNS entry")
    p.add_argument("--xsec-from", default=os.path.join(
        os.path.dirname(HERE), "data", "Run3_v13_Run2_v9"),
        help="bookkeeping tree to copy xsec from (default the v13 tree)")
    p.add_argument("--sample-dir", help="override $SKNANO_DATA/<era>/Sample")
    p.add_argument("--samples", nargs="*", default=[],
                   help="restrict to these aliases (default: every dir under skim-base/era)")
    p.add_argument("--verify-sign", action="store_true",
                   help="count genWeight signs in the original NanoAOD instead of "
                        "assuming sumsign == nmc. Slow (reads one branch of every "
                        "input file) but exact")
    p.add_argument("--threads", type=int, default=6)
    p.add_argument("--dry-run", action="store_true")
    args = p.parse_args()

    if args.threads > 1:
        ROOT.EnableImplicitMT(args.threads)

    sampledir = args.sample_dir
    if not sampledir:
        data = os.environ.get("SKNANO_DATA")
        if not data:
            sys.exit("SKNANO_DATA is not set -- source setup.sh, or pass --sample-dir")
        sampledir = os.path.join(data, args.era, "Sample")

    campaign = args.campaign or CAMPAIGNS.get(args.era)
    if not campaign:
        sys.exit(f"no campaign known for era '{args.era}' -- pass --campaign")
    cdir = os.path.join(args.dataset_base, campaign)

    # PD -> xsec, from a bookkeeping tree that already has these samples.
    xsec_path = os.path.join(args.xsec_from, args.era, "Sample", "CommonSampleInfo.json")
    if not os.path.exists(xsec_path):
        sys.exit(f"no xsec source at {xsec_path}")
    by_pd = {}
    for key, info in json.load(open(xsec_path)).items():
        pd = info.get("PD")
        if pd:
            by_pd.setdefault(pd, (key, info.get("xsec")))

    target_path = os.path.join(sampledir, "CommonSampleInfo.json")
    common = json.load(open(target_path))

    skimdir = os.path.join(args.skim_base, args.era)
    aliases = args.samples or sorted(
        d for d in os.listdir(skimdir) if os.path.isdir(os.path.join(skimdir, d)))

    added, skipped = [], []
    for alias in aliases:
        files = sorted(glob.glob(os.path.join(skimdir, alias, "tree_*.root")))
        if not files:
            skipped.append(f"{alias}: no tree_*.root")
            continue

        # the alias is the dataset dir name cut at _Tune; recover the full name
        matches = [d for d in os.listdir(cdir)
                   if d.split(TUNE_TAIL)[0] == alias]
        if len(matches) != 1:
            skipped.append(f"{alias}: {len(matches)} dataset dirs match in {campaign}")
            continue
        pd = matches[0]

        if pd not in by_pd:
            skipped.append(f"{alias}: PD '{pd}' has no xsec in {args.xsec_from} "
                           "(duplicate production? do not stack it)")
            continue
        src_key, xsec = by_pd[pd]

        nmc, sumw = runs_totals(files)
        if args.verify_sign:
            orig = []
            for root, _dirs, fs in os.walk(os.path.join(cdir, pd)):
                orig += [os.path.join(root, f) for f in fs if f.endswith(".root")]
            n_orig, sumsign = sign_totals(sorted(orig))
            if n_orig != int(nmc):
                skipped.append(f"{alias}: Runs says {int(nmc)} events, the original "
                               f"files have {n_orig} -- the skim is incomplete")
                continue
        else:
            sumsign = nmc

        entry = {"isMC": 1, "PD": pd, "xsec": float(xsec),
                 "nmc": float(nmc), "sumsign": float(sumsign), "sumW": float(sumw)}
        was = common.get(alias)
        common[alias] = entry
        added.append((alias, entry, src_key, was))

    for alias, e, src, was in added:
        tag = "UPDATE" if was else "new   "
        print(f"  [{tag}] {alias:<30} xsec={e['xsec']:>12.1f} (from {src})")
        print(f"           nmc={e['nmc']:.0f} sumsign={e['sumsign']:.0f} sumW={e['sumW']:.1f}")
        if was and was != e:
            print(f"           was: {was}")

    if skipped:
        print(f"\n  {len(skipped)} skipped:")
        for s in skipped:
            print(f"    {s}")

    if added and not args.dry_run:
        backup = target_path + ".bak"
        if not os.path.exists(backup):
            with open(backup, "w") as fh:
                json.dump(json.load(open(target_path)), fh, indent=4)
            print(f"\n[info] backed up original to {backup}")
        with open(target_path, "w") as fh:
            json.dump(common, fh, indent=4)

    print(f"\n[sampleinfo] {len(added)} entry(s) "
          f"{'would be written' if args.dry_run else 'written'} to {target_path}")


if __name__ == "__main__":
    main()
