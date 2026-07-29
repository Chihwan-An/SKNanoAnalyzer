#!/usr/bin/env python3
import json
import os
import shutil
import fnmatch
import argparse
import ROOT
from pprint import pprint

parser = argparse.ArgumentParser()
parser.add_argument("--era", type=str, required=True, help="era")
parser.add_argument("--samples", type=str, action="append", default=None,
                    help="처리할 샘플만 지정. 콤마 구분, glob 사용 가능, 여러 번 지정 가능. "
                         '예: --samples "DYMLL*,TTLL_powheg". 생략하면 전체 샘플.')
parser.add_argument("--samples-file", type=str, default=None,
                    help="샘플 패턴이 한 줄에 하나씩 적힌 파일 (예: LRSM.txt). # 주석 허용.")
parser.add_argument("--dry-run", action="store_true",
                    help="파일을 쓰지 않고 어떤 값이 어떻게 바뀔지만 출력한다.")
parser.add_argument("--no-backup", action="store_true",
                    help="CommonSampleInfo.json 백업을 만들지 않는다. (기본은 .bak 생성)")
args = parser.parse_args()
SKNanoOutputDir = os.environ["SKNANO_OUTPUT"]
SKNanoDataDir = os.environ["SKNANO_DATA"]

commonInfoPath = f"{SKNanoDataDir}/{args.era}/Sample/CommonSampleInfo.json"
with open(commonInfoPath, "r") as f:
    common_info = json.load(f)


def build_sample_filter():
    # returns (matcher, patterns). patterns가 비면 전체 통과.
    patterns = []
    for spec in (args.samples or []):
        patterns += [p.strip() for p in spec.split(",") if p.strip()]
    if args.samples_file:
        with open(args.samples_file) as f:
            for line in f:
                line = line.split("#")[0].strip()
                if line:
                    patterns.append(line)
    if not patterns:
        return (lambda name: True), []
    return (lambda name: any(fnmatch.fnmatch(name, p) for p in patterns)), patterns


def write_json(path, data):
    # 임시 파일에 쓰고 rename -> 중간에 죽어도 기존 파일이 잘리지 않는다.
    if args.dry_run:
        print(f"    [dry-run] would write {path}")
        return
    tmpPath = path + ".tmp"
    with open(tmpPath, "w") as f:
        json.dump(data, f, indent=4)
    os.replace(tmpPath, path)

# Here we assume that we already parsed the sample paths using scripts/MakeSamplePathInfo.py
# and run GetEffLumi for all samples to get the inputs for this script.

# For data samples, only have to parse the number of events
def parseDataInfoFor(sample_name):
    # parse periods from sample_info
    periods = common_info[sample_name]["periods"]
    
    # Get existing NEvents if available
    if "NEvents" in common_info[sample_name]:
        nevts = common_info[sample_name]["NEvents"]
    else:
        nevts = [-1] * len(periods)

    updated = False
    for i, period in enumerate(periods):
        try:
            f = ROOT.TFile.Open(f"{SKNanoOutputDir}/GetEffLumi/{args.era}/{sample_name}_{period}.root")
            if not f or f.IsZombie():
                continue
            h = f.Get("NEvents")
            if h:
                nevts[i] = h.GetBinContent(1)
                updated = True
            f.Close()
        except:
            print(f"Error opening file for {sample_name} {period}")
    
    if not updated:
        print(f"  {sample_name}: GetEffLumi 출력을 못 읽었다 -> 기존 값 유지")
        return

    print(f"  {sample_name}: NEvents {common_info[sample_name].get('NEvents')} -> {nevts}")
    common_info[sample_name]["NEvents"] = nevts

    # add in ForSNU/$SAMPLE_NAME_PERIOD.json
    for i, period in enumerate(periods):
        json_path = f"{SKNanoDataDir}/{args.era}/Sample/ForSNU/{sample_name}_{period}.json"
        if os.path.exists(json_path):
            with open(json_path, "r") as f:
                for_snu_info = json.load(f)
            for_snu_info["NEvents"] = nevts
            write_json(json_path, for_snu_info)
    return True

def parseMCInfoFor(sample_name):
    rootPath = f"{SKNanoOutputDir}/GetEffLumi/{args.era}/{sample_name}.root"
    try:
        f = ROOT.TFile.Open(rootPath)
        if not f or f.IsZombie():
            print(f"  {sample_name}: {rootPath} 를 열 수 없다 -> 기존 값 유지")
            return

        h_nevts = f.Get("NEvents")
        h_sumsign = f.Get("sumSign")
        h_sumW = f.Get("sumW")

        if not h_nevts or not h_sumsign or not h_sumW:
            f.Close()
            print(f"  {sample_name}: 히스토그램이 없다 -> 기존 값 유지")
            return

        nevts = h_nevts.GetBinContent(1)
        sumsign = h_sumsign.GetBinContent(1)
        sumW = h_sumW.GetBinContent(1)
        f.Close()

        old = common_info[sample_name]
        print(f"  {sample_name}: nmc {old.get('nmc'):,.0f} -> {nevts:,.0f} | "
              f"sumsign {old.get('sumsign'):,.0f} -> {sumsign:,.0f} | "
              f"sumW {old.get('sumW'):,.1f} -> {sumW:,.1f}")

        common_info[sample_name]["nmc"] = nevts
        common_info[sample_name]["sumsign"] = sumsign
        common_info[sample_name]["sumW"] = sumW

        # add in ForSNU/$SAMPLE_NAME.json
        json_path = f"{SKNanoDataDir}/{args.era}/Sample/ForSNU/{sample_name}.json"
        if os.path.exists(json_path):
            with open(json_path, "r") as f:
                for_snu_info = json.load(f)
            for_snu_info["nmc"] = nevts
            for_snu_info["sumsign"] = sumsign
            for_snu_info["sumW"] = sumW
            write_json(json_path, for_snu_info)
        return True
    except Exception as e:
        print(f"Error opening file for {sample_name}: {type(e).__name__}: {e}")

def main():
    matches, patterns = build_sample_filter()
    selected = [name for name in common_info if matches(name)]
    if patterns:
        print(f"[{args.era}] 패턴 {patterns} -> {len(selected)}/{len(common_info)} 샘플 선택")
        if not selected:
            raise SystemExit("패턴에 맞는 샘플이 없다. 오타인지 확인하라. (아무것도 바꾸지 않았다)")
    else:
        print(f"[{args.era}] 전체 {len(selected)} 샘플 처리")
    if args.dry_run:
        print("[dry-run] 파일을 쓰지 않는다.")

    nUpdated = 0
    for sample_name in selected:
        if common_info[sample_name]["isMC"]:
            ok = parseMCInfoFor(sample_name)
        else:
            ok = parseDataInfoFor(sample_name)
        if ok:
            nUpdated += 1

    print(f"[{args.era}] {nUpdated}/{len(selected)} 샘플 갱신됨")
    if nUpdated == 0:
        print("갱신된 샘플이 없다 -> CommonSampleInfo.json 을 건드리지 않는다.")
        return

    # 선택되지 않은 샘플은 로드한 값 그대로 다시 쓰이므로 보존된다.
    # 그래도 통째로 덮어쓰는 작업이라 백업을 남긴다.
    if not args.dry_run and not args.no_backup:
        shutil.copy2(commonInfoPath, commonInfoPath + ".bak")
        print(f"백업: {commonInfoPath}.bak")

    # dump the updated sample_info
    write_json(commonInfoPath, common_info)

if __name__ == "__main__":
    main()
