#!/usr/bin/env python3
import os
import json
import fnmatch
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('--era', type=str, default='2016preVFP', help='era')
parser.add_argument('--samples', type=str, action='append', default=None,
                    help='처리할 샘플만 지정. 콤마 구분, glob 사용 가능, 여러 번 지정 가능. '
                         '예: --samples "DYMLL*,TTLL_powheg". 생략하면 전체 샘플.')
parser.add_argument('--samples-file', type=str, default=None,
                    help='샘플 패턴이 한 줄에 하나씩 적힌 파일 (예: LRSM.txt). # 주석 허용.')
parser.add_argument('--dry-run', action='store_true',
                    help='파일을 쓰지 않고 무엇이 어떻게 바뀔지만 출력한다.')
parser.add_argument('--allow-empty', action='store_true',
                    help='새로 수집한 파일 리스트가 비어 있어도 기록한다. '
                         '기본값은 거부하고 기존 리스트를 보존한다.')
args = parser.parse_args()

#This script is used to generate the path information for the sample data
#Sample information is stored in the CommonSampleInfo.json
#This script will generate the path information for the sample data and save it to the ForSNU folder
if args.era == "2016preVFP" or args.era == "2016postVFP":
    YEAR = "2016"
    BASEPATH = os.environ["SKNANO_RUN2_NANOAODPATH"]
elif args.era == "2017":
    YEAR = "2017"
    BASEPATH = "/gv0/Users/achihwan/SKNano/Run3NanoAODv13p1"
elif args.era == "2018":
    YEAR = "2018"
    BASEPATH = "/gv0/Users/achihwan/SKNano/Run3NanoAODv13p1"
elif args.era == "2022":
    YEAR = "2022"
    BASEPATH = "/gv0/Users/achihwan/SKNano/Run3NanoAODv12"
elif args.era == "2022EE":
    YEAR = "2022"
    BASEPATH = "/gv0/Users/achihwan/SKNano/Run3NanoAODv12"
elif args.era == "2023":
    YEAR = "2023"
    BASEPATH = "/gv0/Users/achihwan/SKNano/Run3NanoAODv12"
elif args.era == "2023BPix":
    YEAR = "2023"
    BASEPATH = "/gv0/Users/achihwan/SKNano/Run3NanoAODv12"
else:
    raise ValueError(f"Unknown era: {args.era}")

def build_sample_filter():
    # returns (matcher, patterns). patterns가 비면 전체 통과.
    patterns = []
    for spec in (args.samples or []):
        patterns += [p.strip() for p in spec.split(',') if p.strip()]
    if args.samples_file:
        with open(args.samples_file) as f:
            for line in f:
                line = line.split('#')[0].strip()
                if line:
                    patterns.append(line)
    if not patterns:
        return (lambda alias: True), []
    return (lambda alias: any(fnmatch.fnmatch(alias, p) for p in patterns)), patterns


def write_json(path, data):
    # 임시 파일에 쓰고 rename -> 중간에 죽어도 기존 파일이 잘리지 않는다.
    if args.dry_run:
        print(f"    [dry-run] would write {path}")
        return
    tmpPath = path + '.tmp'
    with open(tmpPath, 'w') as f:
        json.dump(data, f, indent=4)
    os.replace(tmpPath, path)


def parse_rootfiles_from(basePath):
    if not os.path.exists(basePath):
        print(f"No {basePath}")

    filePaths = []
    for root, _, files in os.walk(basePath):
        for file in files:
            if file.endswith(".root"):
                filePaths.append(os.path.join(root, file))
    
    # sort the file paths by the number in the file name
    def extract_number(file_path):
        file_name = os.path.basename(file_path)
        # Remove the file extension
        file_name = os.path.splitext(file_name)[0]
        # Extract the number after the last underscore
        try:
            number = int(file_name.split('_')[-1])
            return number
        except ValueError:
            # If conversion fails, return the original string to maintain stable sorting
            return file_name

    filePaths.sort(key=extract_number)
    return filePaths

def main():
    matches, patterns = build_sample_filter()

    sampleInfoJson = os.path.join(os.environ['SKNANO_DATA'], args.era, 'Sample', 'CommonSampleInfo.json')
    sampleInfos = json.load(open(sampleInfoJson))
    os.makedirs(os.path.join(os.environ['SKNANO_DATA'], args.era, 'Sample', 'ForSNU'), exist_ok=True)

    selected = [alias for alias in sampleInfos if matches(alias)]
    if patterns:
        print(f"[{args.era}] 패턴 {patterns} -> {len(selected)}/{len(sampleInfos)} 샘플 선택")
        if not selected:
            raise SystemExit("패턴에 맞는 샘플이 없다. 오타인지 확인하라. (아무것도 바꾸지 않았다)")
    else:
        print(f"[{args.era}] 전체 {len(selected)} 샘플 처리")
    if args.dry_run:
        print("[dry-run] 파일을 쓰지 않는다.")

    for alias in selected:
        sampleInfo = sampleInfos[alias]
        print(f"Processing {alias}...")
        isMC = sampleInfo["isMC"]
        if isMC:
            basePath = os.path.join(BASEPATH, args.era, sampleInfo["PD"])
            fileJsonPath = os.path.join(os.environ['SKNANO_DATA'], args.era, 'Sample', 'ForSNU', alias+'.json')

            if os.path.exists(fileJsonPath):
                with open(fileJsonPath, 'r') as f:
                    jsonData = json.load(f)
                oldPD = jsonData.get("PD")
                jsonData["PD"] = sampleInfo["PD"]
                jsonData["xsec"] = sampleInfo["xsec"]
                paths = jsonData.get("path", [])
                if oldPD != sampleInfo["PD"] or not paths or any(not os.path.exists(path) for path in paths):
                    newPaths = parse_rootfiles_from(basePath)
                    # 빈 리스트로 덮어쓰면 기존 정보가 날아간다. 마운트가 안 붙었거나
                    # PD 이름이 틀린 경우가 대부분이므로 기본적으로 거부한다.
                    if newPaths or args.allow_empty:
                        print(f"    path: {len(paths)} -> {len(newPaths)}")
                        jsonData["path"] = newPaths
                    else:
                        print(f"    !! {basePath} 에서 root 파일을 찾지 못했다 "
                              f"-> 기존 path {len(paths)}개를 그대로 보존하고 건너뛴다 "
                              f"(정말 비우려면 --allow-empty)")
                        continue
            else:
                newPaths = parse_rootfiles_from(basePath)
                if not newPaths and not args.allow_empty:
                    print(f"    !! {basePath} 에서 root 파일을 찾지 못했다 -> {alias}.json 생성하지 않는다")
                    continue
                jsonData = {
                    "name": alias,
                    "isMC": 1,
                    "PD": sampleInfo["PD"],
                    "xsec": sampleInfo["xsec"],
                    "sumsign": -1,
                    "sumW": -1,
                    "nmc": -1,
                    "path": newPaths,
                }
                print(f"    new file, path: {len(newPaths)}")

            write_json(fileJsonPath, jsonData)
        else:
            for period in sampleInfo["periods"]:
                basePath = os.path.join(BASEPATH, args.era, f"{alias}/Run{YEAR}{period}")
                fileJsonPath = os.path.join(os.environ['SKNANO_DATA'], args.era, 'Sample', 'ForSNU', alias+'_'+period+'.json')

                if os.path.exists(fileJsonPath):
                    with open(fileJsonPath, 'r') as f:
                        jsonData = json.load(f)
                    # path 건드리지 않음
                else:
                    newPaths = parse_rootfiles_from(basePath)
                    if not newPaths and not args.allow_empty:
                        print(f"    !! {basePath} 에서 root 파일을 찾지 못했다 "
                              f"-> {alias}_{period}.json 생성하지 않는다")
                        continue
                    jsonData = {
                        "name": alias,
                        "isMC": 0,
                        "NEvents": [],
                        "path": newPaths,
                    }

                write_json(fileJsonPath, jsonData)


if __name__ == "__main__":
    main()
