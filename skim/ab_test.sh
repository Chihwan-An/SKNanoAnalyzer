#!/bin/bash
# A/B test: run Reproduce20_002_copy over raw NanoAOD and over the skim of the
# SAME input files, then compare every histogram bin by bin.
#
# This is the check the margin scan cannot do. verify_margin.py assumes the
# necessary condition I read out of Reproduce20_002_copy.cc is correct; this
# tests that assumption against the analyzer itself.
#
#   ./ab_test.sh                          # 1 file of TTLL_powheg
#   ./ab_test.sh TTLL_powheg 2            # 2 files
#   ./ab_test.sh DYMLL120HT2500 1
#
# NOTE: MaxEvent is deliberately not capped. Limiting it would make the raw run
# process the first N raw events and the skim run the first N *skimmed* events,
# which are different sets of events, and the comparison would be meaningless.

set -euo pipefail

SAMPLE="${1:-TTLL_powheg}"
NFILES="${2:-1}"
ERA="${3:-2023}"

SKNANO=/data6/Users/achihwan/SKNanoAnalyzer-v13
IMAGE=/data6/Users/achihwan/private-el9.sif
WORK="${AB_WORKDIR:-/gv0/Users/achihwan/SKNanoRunlog/ab_test_${SAMPLE}_${ERA}}"

# Both analyzer runs must use the SAME library, otherwise the comparison is
# meaningless. Override to pin a known-good build, e.g. a previous runlog's
# snapshot:  AB_LIB=<runlog>/lib AB_INC=<runlog>/include ./ab_test.sh
AB_LIB="${AB_LIB:-$SKNANO/install/redhat/lib}"
AB_INC="${AB_INC:-$SKNANO/install/redhat/include}"

echo "=== A/B test: $SAMPLE, $NFILES file(s), era $ERA"
echo "=== workdir: $WORK"
rm -rf "$WORK"
mkdir -p "$WORK"/{raw,skimin,skimout}

# ---------------------------------------------------------------- input files
python3 - "$SKNANO" "$ERA" "$SAMPLE" "$NFILES" "$WORK" <<'PY'
import json, sys, os
sknano, era, sample, nfiles, work = sys.argv[1:6]
sdir = os.path.join(sknano, "data", "Run3_v12_Run2_v9", era, "Sample")
info = json.load(open(os.path.join(sdir, "ForSNU", sample + ".json")))
common = json.load(open(os.path.join(sdir, "CommonSampleInfo.json")))[sample]
files = info["path"][: int(nfiles)]
open(os.path.join(work, "inputs.txt"), "w").write("\n".join(files) + "\n")
json.dump({"xsec": common["xsec"], "sumW": common["sumW"],
           "sumsign": common["sumsign"], "files": files},
          open(os.path.join(work, "meta.json"), "w"), indent=2)
print(f"  {len(files)} input file(s)")
PY

# ------------------------------------------------------- generate job macros
python3 - "$WORK" "$SAMPLE" "$ERA" <<'PY'
import json, os, sys
work, sample, era = sys.argv[1:4]
meta = json.load(open(os.path.join(work, "meta.json")))

def macro(jobname, files, outpath):
    adds = "\n".join(f'    module.AddFile("{f}");' for f in files)
    return f"""#include <algorithm>
void {jobname}() {{
    Reproduce20_002_copy module;
    module.SetTreeName("Events");
    module.LogEvery = 20000;
    module.IsDATA = false;
    module.MCSample = "{sample}";
    module.xsec = {meta['xsec']};
    module.sumW = {meta['sumW']};
    module.sumSign = {meta['sumsign']};
    module.SetEra("{era}");
    module.SetPeriod("");
{adds}
    module.SetOutfilePath("{outpath}");
    module.Init();
    module.initializeAnalyzer();
    module.Loop();
    module.WriteHist();
}}
"""

open(os.path.join(work, "job_raw.cc"), "w").write(
    macro("job_raw", meta["files"], os.path.join(work, "hists_raw.root")))
# the skim job's file list is filled in after the skim runs
PY

# --------------------------------------------------------------- environment
run_in_image() {
    singularity exec --bind /data6:/data6,/home/achihwan:/home/achihwan,/gv0:/gv0,/tmp:/tmp \
        "$IMAGE" bash -c "
export PATH=/opt/conda/bin:\$PATH
export MAMBA_ROOT_PREFIX=/opt/conda
eval \"\$(micromamba shell hook -s bash)\"
micromamba activate Nano
export SKNANO_HOME=$SKNANO
export SKNANO_DATA=$SKNANO/data/Run3_v12_Run2_v9
export PATH=\$PATH:\$SKNANO_HOME/external/lhapdf/bin
export LHAPDF_INCLUDE_DIR=\$SKNANO_HOME/external/lhapdf/redhat/include
export LHAPDF_LIB_DIR=\$SKNANO_HOME/external/lhapdf/redhat/lib
export CORRECTION_INCLUDE_DIR=\$(correction config --incdir)
export CORRECTION_LIB_DIR=\$(correction config --libdir)
export JSONPOG_REPO_PATH=\$SKNANO_HOME/external/jsonpog-integration
export ROCCOR_PATH=\$SKNANO_HOME/external/RoccoR
export ONNXRUNTIME_INCLUDE_DIR=\${CONDA_PREFIX}/include/onnxruntime/core/session
export ONNXRUNTIME_LIB_DIR=\${CONDA_PREFIX}/lib
export LD_LIBRARY_PATH=\$LD_LIBRARY_PATH:\$SKNANO_HOME/external/lhapdf/redhat/lib:\$CORRECTION_LIB_DIR:$AB_LIB
export ROOT_INCLUDE_PATH=$AB_INC
export ROOT_HIST=0
$1"
}

# ------------------------------------------------------------------ 1. raw
echo
echo "=== [1/4] analyzer over RAW NanoAOD"
run_in_image "cd $WORK && root -l -b -q job_raw.cc" > "$WORK/raw.log" 2>&1 || true
tail -3 "$WORK/raw.log"
grep -q "Writing histograms done" "$WORK/raw.log" || { echo "!! raw analyzer FAILED, see $WORK/raw.log"; exit 1; }

# ----------------------------------------------------------------- 2. skim
echo
echo "=== [2/4] skimming the same files"
run_in_image "cd $SKNANO/skim && python3 skim_hnwr.py --era $ERA --threads 4 \
    --filelist $WORK/inputs.txt --output $WORK/skimout/tree_0.root \
    --report-json $WORK/skim_report.json" 2>&1 | grep -vE "RuntimeWarning|TClass::Init" | tail -12

# ------------------------------------------------------------ 3. on the skim
echo
echo "=== [3/4] analyzer over the SKIM"
python3 - "$WORK" "$SAMPLE" "$ERA" <<'PY'
import json, os, sys
work, sample, era = sys.argv[1:4]
meta = json.load(open(os.path.join(work, "meta.json")))
skim = [os.path.join(work, "skimout", "tree_0.root")]
adds = "\n".join(f'    module.AddFile("{f}");' for f in skim)
open(os.path.join(work, "job_skim.cc"), "w").write(f"""#include <algorithm>
void job_skim() {{
    Reproduce20_002_copy module;
    module.SetTreeName("Events");
    module.LogEvery = 20000;
    module.IsDATA = false;
    module.MCSample = "{sample}";
    module.xsec = {meta['xsec']};
    module.sumW = {meta['sumW']};
    module.sumSign = {meta['sumsign']};
    module.SetEra("{era}");
    module.SetPeriod("");
{adds}
    module.SetOutfilePath("{os.path.join(work, 'hists_skim.root')}");
    module.Init();
    module.initializeAnalyzer();
    module.Loop();
    module.WriteHist();
}}
""")
PY
run_in_image "cd $WORK && root -l -b -q job_skim.cc" > "$WORK/skim.log" 2>&1 || true
tail -3 "$WORK/skim.log"
grep -q "Writing histograms done" "$WORK/skim.log" || { echo "!! skim analyzer FAILED, see $WORK/skim.log"; exit 1; }

# --------------------------------------------------------------- 4. compare
echo
echo "=== [4/4] comparing"
python3 "$SKNANO/skim/compare_hists.py" "$WORK/hists_raw.root" "$WORK/hists_skim.root"
