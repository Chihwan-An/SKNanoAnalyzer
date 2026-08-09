#!/usr/bin/env python3
"""
Register finished skims so `SKNano.py -i 'Skim_HNWR_*'` can run over them.

Writes, into $SKNANO_DATA/<era>/Sample/Skim/ :
  skimTreeInfo.json                  upserted with {suffix, PD, isMC[, periods]}
  Skim_<suffix>_<PD>[_<period>].json {suffix, PD, isMC, name, path: [...]}

Normalisation needs no help from us: SKNano.py:290-296 looks the skim alias up
in skimTreeInfo.json, follows its 'PD' back to CommonSampleInfo.json, and injects
that sample's pre-skim xsec / sumW / sumsign into the job macro. The skim files
also carry the untouched Runs tree, so genEventSumw is available independently.

  ./register_skim.py --era 2023 --suffix HNWR --samples TTLL_powheg 'DYMLL*'
  ./register_skim.py --era 2023 --suffix HNWR --all      # every dir found on disk

Safety: refuses to register a directory containing a leftover 'Temp_*' subdir.
SKNano.py's own postproc (SKNano.py:487-522) does `mv Temp_X <parent>/X`, which
nests instead of replacing when the target already exists -- the resulting
duplicate paths are why Skim_20002_DYJets.json currently double-counts events.
"""

import argparse
import fnmatch
import glob
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import skim_naming  # noqa: E402


def tree_index(path):
    m = re.search(r"tree_(\d+)\.root$", path)
    return int(m.group(1)) if m else -1


def collect_root_files(directory):
    found = []
    for root, _dirs, files in os.walk(directory):
        for f in files:
            if f.endswith(".root"):
                found.append(os.path.join(root, f))
    return sorted(found, key=tree_index)


def main():
    p = argparse.ArgumentParser(description=__doc__,
                                formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--era", required=True)
    p.add_argument("--suffix", default="HNWR")
    p.add_argument("--samples", nargs="*", default=[],
                   help="original aliases (e.g. TTLL_powheg, EGamma0_C) or globs")
    p.add_argument("--all", action="store_true",
                   help="register every skim directory found under --outbase")
    p.add_argument("--layout", default="sknano", choices=["sknano", "flat"],
                   help="must match the layout submit_skim.py wrote with. flat is "
                        "<outbase>/<era>/<alias>, and since those directory names "
                        "carry no Skim_<suffix>_ prefix, --all takes every "
                        "subdirectory of <outbase>/<era>")
    p.add_argument("--outbase", default=os.environ.get(
        "SKNANO_RUN3_NANOAODPATH", "/gv0/DATA/SKNano/Run3NanoAODv12"))
    p.add_argument("--user", default=os.environ.get("USER", "unknown"))
    p.add_argument("--sample-dir", help="override $SKNANO_DATA/<era>/Sample")
    p.add_argument("--dry-run", action="store_true")
    args = p.parse_args()

    sampledir = args.sample_dir
    if not sampledir:
        data = os.environ.get("SKNANO_DATA")
        if not data:
            sys.exit("SKNANO_DATA is not set -- source setup.sh, or pass --sample-dir")
        sampledir = os.path.join(data, args.era, "Sample")

    common_path = os.path.join(sampledir, "CommonSampleInfo.json")
    common = json.load(open(common_path))

    aliases = []
    if args.all and args.layout == "flat":
        base = os.path.join(args.outbase, args.era)
        aliases = sorted(d for d in os.listdir(base)
                         if os.path.isdir(os.path.join(base, d)))
    elif args.all:
        for kind in ("MC", "DATA"):
            base = os.path.join(args.outbase, args.era, kind, "Skim", args.user)
            for d in sorted(glob.glob(os.path.join(base, f"Skim_{args.suffix}_*"))):
                pd = os.path.basename(d)[len(f"Skim_{args.suffix}_"):]
                if kind == "MC":
                    aliases.append(pd)
                else:
                    for per in sorted(glob.glob(os.path.join(d, "Period*"))):
                        aliases.append(f"{pd}_{os.path.basename(per)[len('Period'):]}")
    else:
        known = sorted(
            os.path.basename(f)[:-5]
            for f in glob.glob(os.path.join(sampledir, "ForSNU", "*.json"))
        )
        for pat in args.samples:
            hits = fnmatch.filter(known, pat) if any(c in pat for c in "*?[") else (
                [pat] if pat in known else [])
            if not hits:
                sys.exit(f"no sample matches '{pat}'")
            aliases += hits
    aliases = sorted(set(aliases))
    if not aliases:
        sys.exit("nothing to register (give --samples or --all)")

    skimdir = os.path.join(sampledir, "Skim")
    os.makedirs(skimdir, exist_ok=True)
    summary_path = os.path.join(skimdir, "skimTreeInfo.json")
    summary = json.load(open(summary_path)) if os.path.exists(summary_path) else {}

    registered = 0
    for alias in aliases:
        pd, period, is_mc = skim_naming.split_alias(alias)

        if pd not in common:
            print(f"  SKIP {alias}: '{pd}' is not in CommonSampleInfo.json")
            continue
        if int(is_mc) != int(common[pd]["isMC"]):
            print(f"  SKIP {alias}: isMC mismatch vs CommonSampleInfo.json")
            continue

        if args.layout == "flat":
            outdir = os.path.join(args.outbase, args.era, alias)
        else:
            outdir = skim_naming.output_dir(args.outbase, args.era, args.suffix,
                                            alias, args.user)
        if not os.path.isdir(outdir):
            print(f"  SKIP {alias}: {outdir} does not exist")
            continue

        temps = [d for d in os.listdir(outdir)
                 if d.startswith("Temp_") and os.path.isdir(os.path.join(outdir, d))]
        if temps:
            print(f"  SKIP {alias}: leftover {temps} inside {outdir} would "
                  "double-count events -- clean it up first")
            continue

        files = collect_root_files(outdir)
        if not files:
            print(f"  SKIP {alias}: no .root files in {outdir}")
            continue

        key = skim_naming.skim_name(args.suffix, pd)
        entry = summary.get(key, {})
        entry["suffix"] = args.suffix
        entry["PD"] = pd
        entry["isMC"] = int(is_mc)
        if not is_mc:
            periods = entry.get("periods", [])
            if period not in periods:
                periods.append(period)
            entry["periods"] = sorted(periods)
        summary[key] = entry

        payload = {k: v for k, v in entry.items() if k != "periods"}
        payload["name"] = key
        payload["path"] = files

        out_json = os.path.join(skimdir, skim_naming.json_name(args.suffix, alias) + ".json")
        print(f"  {alias:<32} {len(files):>5} files -> {os.path.basename(out_json)}")
        if not args.dry_run:
            with open(out_json, "w") as fh:
                json.dump(payload, fh, indent=4)
        registered += 1

    if not args.dry_run and registered:
        with open(summary_path, "w") as fh:
            json.dump(summary, fh, indent=4)

    print(f"\n[register] {registered} sample(s) "
          f"{'would be registered' if args.dry_run else 'registered'} in {skimdir}")
    if registered and not args.dry_run:
        print("[register] now run, e.g.:")
        print(f"    SKNano.py -a Reproduce20_002_copy -i 'Skim_{args.suffix}_*' "
              f"-e {args.era} -n -1")


if __name__ == "__main__":
    main()
