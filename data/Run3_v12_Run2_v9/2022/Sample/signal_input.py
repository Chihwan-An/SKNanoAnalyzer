import json
import re

with open("WRtoNELtoEEJJ_cross_sections.json") as f:
    data = json.load(f)

output = {}

for filename, info in data.items():
    # key example: WRtoNELtoEEJJ_WR10000_N100.out
    wr = re.search(r'_WR(\d+)', filename)
    mn = re.search(r'_N(\d+)\.out', filename)

    if not (wr and mn):
        continue

    mwr    = int(wr.group(1))
    mn_val = int(mn.group(1))
    xs     = info["cross_section_pb"]

    key = f"WR{mwr}N{mn_val}EE"

    output[key] = {
        "isMC"    : 1,
        "PD"      : filename.replace(".out", ""),
        "xsec"    : xs,
        "nmc"     : -1,
        "sumsign" : -1,
        "sumW"    : -1
    }

sorted_output = dict(
    sorted(output.items(), key=lambda x: (int(re.search(r'WR(\d+)', x[0]).group(1)),
                                          int(re.search(r'N(\d+)', x[0]).group(1))))
)

out_path = "xsec_signal.json"
with open(out_path, "w") as f:
    json.dump(sorted_output, f, indent=4)

print(f"총 {len(output)}개 저장 → {out_path}")