#!/bin/bash
set -euo pipefail

cd "[WORKDIR]"
shopt -s nullglob
inputs=(output/hists_*.root)
if (( ${#inputs[@]} == 0 )); then
  echo "No analyzer outputs found under [WORKDIR]/output" >&2
  exit 1
fi

mkdir -p "[TARGETDIR]"
mv -- "${inputs[@]}" "[TARGETDIR]/"
cp "[PROVENANCE]" "[TARGETDIR]/provenance.json"
