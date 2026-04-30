import json
import re

with open("xsdb_exports.json") as f:
    data = json.load(f)

output = {}

for entry in data:
    name = entry["process_name"]
    xs   = entry["cross_section"]

    wr = re.search(r'MWR(\d+)', name)
    mn = re.search(r'_N(\d+)_TuneCP5', name)

    if not (wr and mn):
        continue

    mwr    = int(wr.group(1))
    mn_val = int(mn.group(1))

    if "ElEl" in name:
        ch = "EE"
    elif "MuMu" in name:
        ch = "MM"
    else:
        ch = "EM"

    key = f"WR{mwr}N{mn_val}{ch}"

    output[key] = {
        "isMC"    : 1,
        "PD"      : name,
        "xsec"    : xs,
        "nmc"     : -1,
        "sumsign" : -1,
        "sumW"    : -1
    }

out_path = "xsec_signal.json"
with open(out_path, "w") as f:
    json.dump(output, f, indent=4)

print(f"총 {len(output)}개 저장 → {out_path}")