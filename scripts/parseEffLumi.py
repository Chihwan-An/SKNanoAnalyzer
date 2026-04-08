#!/usr/bin/env python3
import json
import os
import argparse
import ROOT
from pprint import pprint

parser = argparse.ArgumentParser()
parser.add_argument("--era", type=str, required=True, help="era")
args = parser.parse_args()
SKNanoOutputDir = os.environ["SKNANO_OUTPUT"]
SKNanoDataDir = os.environ["SKNANO_DATA"]

with open(f"{SKNanoDataDir}/{args.era}/Sample/CommonSampleInfo.json", "r") as f:
    common_info = json.load(f)

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
        return

    common_info[sample_name]["NEvents"] = nevts

    # add in ForSNU/$SAMPLE_NAME_PERIOD.json
    for i, period in enumerate(periods):
        json_path = f"{SKNanoDataDir}/{args.era}/Sample/ForSNU/{sample_name}_{period}.json"
        if os.path.exists(json_path):
            with open(json_path, "r") as f:
                for_snu_info = json.load(f)
            for_snu_info["NEvents"] = nevts
            with open(json_path, "w") as f:
                json.dump(for_snu_info, f, indent=4)

def parseMCInfoFor(sample_name):
    try:
        f = ROOT.TFile.Open(f"{SKNanoOutputDir}/GetEffLumi/{args.era}/{sample_name}.root")
        if not f or f.IsZombie():
            return
        
        h_nevts = f.Get("NEvents")
        h_sumsign = f.Get("sumSign")
        h_sumW = f.Get("sumW")
        
        if not h_nevts or not h_sumsign or not h_sumW:
            f.Close()
            return

        nevts = h_nevts.GetBinContent(1)
        sumsign = h_sumsign.GetBinContent(1)
        sumW = h_sumW.GetBinContent(1)
        f.Close()

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
            with open(json_path, "w") as f:
                json.dump(for_snu_info, f, indent=4)
    except:
        print(f"Error opening file for {sample_name}")

def main():
    for sample_name in common_info.keys():
        if common_info[sample_name]["isMC"]:
            parseMCInfoFor(sample_name)
        else:
            parseDataInfoFor(sample_name)
    
    # dump the updated sample_info
    with open(f"{SKNanoDataDir}/{args.era}/Sample/CommonSampleInfo.json", "w") as f:
        json.dump(common_info, f, indent=4)

if __name__ == "__main__":
    main()
