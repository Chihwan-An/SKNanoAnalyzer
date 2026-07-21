#!/bin/bash
set -euo pipefail

export PATH="/opt/conda/bin:${PATH}"
export MAMBA_ROOT_PREFIX="/opt/conda"
eval "$(micromamba shell hook -s bash)"
micromamba activate Nano

cd [WORKDIR]
shopt -s nullglob
inputs=(output/hists_*.root)
if (( ${#inputs[@]} == 0 )); then
  echo "No analyzer outputs found under [WORKDIR]/output" >&2
  exit 1
fi

python3 "[SKNANO_HOME]/scripts/sknano_merge.py" \
  --output [TARGET] --jobs 8 --delete-inputs "${inputs[@]}"
cp [PROVENANCE] [TARGET_PROVENANCE]
