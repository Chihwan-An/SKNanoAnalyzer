#!/usr/bin/env python3
"""
Standalone HNWR pre-skim worker.

Reads NanoAOD files with ROOT RDataFrame, drops events that can never pass the
Reproduce20_002_copy selection, and writes a reduced NanoAOD-format file.

This script does not import, link against, or build anything from
SKNanoAnalyzer. It only needs ROOT. See hnwr_skim_cuts.py for the selection and
the argument for why it cannot lose an event.

What is preserved:
  * every branch of Events (all 1782 of them for Run3 v12) -- nothing is
    dropped, so the skim stays usable if the analyzer starts reading a branch
    it does not read today. Use --drop to prune collections if disk matters
    more than that.
  * the Runs tree (genEventSumw, genEventCount, LHEScaleSumw, LHEPdfSumw,
    PSSumw) and LuminosityBlocks, copied unfiltered. Normalisation therefore
    survives inside the file itself, independent of any JSON bookkeeping.

Examples
--------
  # one output file from an explicit list
  ./skim_hnwr.py --era 2023 --output /path/tree_0.root a.root b.root

  # from a file list, 4 threads
  ./skim_hnwr.py --era 2023 --filelist files.txt --output out.root --threads 4
"""

import argparse
import json
import os
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import hnwr_skim_cuts as cuts  # noqa: E402

import ROOT  # noqa: E402

ROOT.gROOT.SetBatch(True)

COMPRESSION_ALGOS = {"zlib": 1, "lzma": 2, "lz4": 4, "zstd": 5}


def parse_args():
    p = argparse.ArgumentParser(description=__doc__,
                                formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("files", nargs="*", help="input NanoAOD files")
    p.add_argument("--filelist", help="text file with one input path per line")
    p.add_argument("--output", required=True, help="output ROOT file")
    p.add_argument("--era", required=True,
                   help="2016preVFP|2016postVFP|2017|2018|2022|2022EE|2023|2023BPix")
    p.add_argument("--tree", default="Events")
    p.add_argument("--threads", type=int, default=1,
                   help="ROOT implicit MT threads (default 1, matching a 1-cpu condor slot)")
    p.add_argument("--compression", default="zstd", choices=sorted(COMPRESSION_ALGOS),
                   help="output compression (default zstd; inputs are lzma:9, which is "
                        "much slower to both write and read)")
    p.add_argument("--compression-level", type=int, default=5)
    p.add_argument("--drop", nargs="*", default=[],
                   help="regexes of Events branches to drop, e.g. '^Photon_' '^Tau_'")
    p.add_argument("--max-files", type=int, help="use only the first N inputs (testing)")
    p.add_argument("--report-json", help="write per-stage counts here")
    p.add_argument("--dry-run", action="store_true",
                   help="resolve inputs and print the selection, then stop")
    return p.parse_args()


def collect_inputs(args):
    files = list(args.files)
    if args.filelist:
        with open(args.filelist) as fh:
            files += [ln.strip() for ln in fh if ln.strip() and not ln.startswith("#")]
    if not files:
        sys.exit("no input files given (positional args or --filelist)")
    if args.max_files:
        files = files[: args.max_files]
    missing = [f for f in files if not f.startswith(("root:", "http")) and not os.path.exists(f)]
    if missing:
        sys.exit(f"input files not found:\n  " + "\n  ".join(missing[:10]))
    return files


def branches_of(path, treename):
    """(branch names, n entries) for `treename`, or (None, 0) if it is absent."""
    fh = ROOT.TFile.Open(path)
    if not fh or fh.IsZombie():
        sys.exit(f"cannot open {path}")
    tree = fh.Get(treename)
    if not tree:
        fh.Close()
        return None, 0
    names = {b.GetName() for b in tree.GetListOfBranches()}
    n = tree.GetEntries()
    fh.Close()
    return names, n


def common_branches(files, treename):
    """Intersection across all inputs, ignoring empty files.

    A path present in only some files would make the JIT-ed filter blow up
    partway through the chain, so the intersection is the safe branch set.

    Empty inputs are dropped rather than allowed to poison it: the production
    NanoAOD contains stub files -- e.g. DYMLL50to120HT70to100's
    6ed4f6d2-...root is 2.4 MB with 4 branches and 0 entries -- and letting one
    of those collapse the intersection would abort a whole chunk of good files.
    Nothing is lost by skipping them; a file with 0 entries has no events to
    contribute. Files that DO have entries but lack a needed branch still abort,
    because that would be real data loss.
    """
    common = None
    good, empty = [], []
    for f in files:
        names, n = branches_of(f, treename)
        if names is None:
            sys.exit(f"{f} has no '{treename}' tree")
        if n == 0:
            empty.append(f)
            continue
        good.append(f)
        common = names if common is None else (common & names)
    if empty:
        print(f"[skim] skipping {len(empty)} empty file(s) (0 entries in "
              f"'{treename}'):")
        for f in empty:
            print(f"         {f}")
    if not good:
        sys.exit("every input file is empty -- nothing to skim")
    return common, good, empty


def copy_aux_trees(inputs, output, treenames):
    """Append unfiltered auxiliary trees (Runs, LuminosityBlocks) to the output."""
    copied = {}
    out = ROOT.TFile.Open(output, "UPDATE")
    for name in treenames:
        chain = ROOT.TChain(name)
        n_with = 0
        for f in inputs:
            if branches_of(f, name)[0] is not None:
                chain.Add(f)
                n_with += 1
        if n_with == 0:
            continue
        out.cd()
        merged = chain.CloneTree(-1, "fast")
        if merged:
            merged.Write(name, ROOT.TObject.kOverwrite)
            copied[name] = int(merged.GetEntries())
    out.Close()
    return copied


def main():
    args = parse_args()
    inputs = collect_inputs(args)

    if args.threads > 1:
        ROOT.EnableImplicitMT(args.threads)

    ROOT.gInterpreter.Declare(cuts.CPP_HELPERS)

    available, inputs, empty_inputs = common_branches(inputs, args.tree)

    missing_required = [b for b in cuts.REQUIRED_BRANCHES if b not in available]
    if missing_required:
        sys.exit("input is missing branches the selection needs: "
                 + ", ".join(missing_required))

    wanted_triggers = cuts.triggers_for_era(args.era)
    trig_available = [t for t in wanted_triggers if t in available]
    trig_missing = [t for t in wanted_triggers if t not in available]
    if trig_missing:
        print(f"[skim] trigger paths absent from these files, ignored: {', '.join(trig_missing)}")
    if not trig_available:
        sys.exit(f"none of the {args.era} trigger paths exist in the input")

    trig_expr = cuts.trigger_filter_expression(trig_available)
    lep_expr = cuts.lepton_filter_expression()

    print(f"[skim] era        : {args.era}")
    print(f"[skim] inputs     : {len(inputs)} file(s)")
    print(f"[skim] output     : {args.output}")
    print(f"[skim] triggers   : {len(trig_available)} path(s)")
    print(f"[skim] trigger cut: {trig_expr}")
    print(f"[skim] lepton cut : {lep_expr}")

    if args.dry_run:
        return

    outdir = os.path.dirname(os.path.abspath(args.output))
    if outdir:
        os.makedirs(outdir, exist_ok=True)

    chain = ROOT.TChain(args.tree)
    for f in inputs:
        chain.Add(f)

    df = ROOT.RDataFrame(chain)
    df = df.Filter(trig_expr, "trigger OR")
    df = df.Filter(lep_expr, "leptons")

    keep = ROOT.std.vector("string")()
    if args.drop:
        import re
        patterns = [re.compile(p) for p in args.drop]
        for name in sorted(available):
            if not any(p.search(name) for p in patterns):
                keep.push_back(name)
        print(f"[skim] keeping {keep.size()} / {len(available)} Events branches")
    else:
        for name in sorted(available):
            keep.push_back(name)

    opts = ROOT.RDF.RSnapshotOptions()
    opts.fCompressionAlgorithm = COMPRESSION_ALGOS[args.compression]
    opts.fCompressionLevel = args.compression_level
    opts.fMode = "RECREATE"

    report = df.Report()

    t0 = time.time()
    df.Snapshot(args.tree, args.output, keep, opts)
    elapsed = time.time() - t0

    # Report() was booked before Snapshot triggered the loop, so it is filled.
    stages = [(r.GetName(), int(r.GetAll()), int(r.GetPass())) for r in report]
    total = stages[0][1] if stages else 0
    passed = stages[-1][2] if stages else 0

    aux = copy_aux_trees(inputs, args.output, ["Runs", "LuminosityBlocks"])

    in_bytes = sum(os.path.getsize(f) for f in inputs if os.path.exists(f))
    out_bytes = os.path.getsize(args.output)

    print("\n[skim] cutflow")
    print(f"  {'all':<16}{total:>12}")
    for name, all_, pass_ in stages:
        frac = pass_ / all_ if all_ else 0.0
        print(f"  {name:<16}{pass_:>12}   step {frac:.4f}")
    print(f"  {'kept':<16}{passed:>12}   "
          f"overall {passed / total if total else 0:.5f}  "
          f"({total / passed if passed else float('inf'):.1f}x reduction)")
    for name, n in aux.items():
        print(f"  [aux] {name}: {n} entries copied unfiltered")
    print(f"  size  {in_bytes / 1e9:.2f} GB -> {out_bytes / 1e9:.3f} GB "
          f"({in_bytes / out_bytes if out_bytes else 0:.1f}x)")
    print(f"  time  {elapsed:.1f} s")

    if args.report_json:
        with open(args.report_json, "w") as fh:
            json.dump({
                "era": args.era,
                "output": os.path.abspath(args.output),
                "n_input_files": len(inputs),
                "n_empty_skipped": len(empty_inputs),
                "total": total,
                "kept": passed,
                "stages": [{"name": n, "all": a, "pass": p} for n, a, p in stages],
                "aux_trees": aux,
                "input_bytes": in_bytes,
                "output_bytes": out_bytes,
                "seconds": elapsed,
                "triggers": trig_available,
            }, fh, indent=2)


if __name__ == "__main__":
    main()
