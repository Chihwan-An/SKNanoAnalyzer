#!/usr/bin/env bash
set -euo pipefail

fail=0

check_no_matches() {
  local label="$1"
  shift
  if "$@"; then
    echo "[check_layers] FAIL: ${label}" >&2
    fail=1
  else
    echo "[check_layers] OK: ${label}"
  fi
}

check_no_matches \
  "DataFormats must not include AnalyzerTools/AnalyzerFramework headers" \
  rg -n '#include\s*[<"]([^">]*/)?(AnalyzerCore|SKNanoLoader|BranchManager|MyCorrection|SystematicHelper|Triggerinfo)\.h[>"]' DataFormats

check_no_matches \
  "AnalyzerTools must not include AnalyzerFramework headers" \
  rg -n '#include\s*[<"]([^">]*/)?(AnalyzerCore|SKNanoLoader|BranchManager|Triggerinfo)\.h[>"]' AnalyzerTools

check_no_matches \
  "AnalyzerFramework must not contain concrete analysis modules" \
  rg -n '\b(class|struct)\s+(Vcb|CalibrationTree|HadronAnalyzer)\b' AnalyzerFramework

if [[ -n "$(git ls-files 'Analyzers/**')" ]]; then
  echo "[check_layers] FAIL: the retired flat Analyzers directory is tracked" >&2
  fail=1
else
  echo "[check_layers] OK: the retired flat Analyzers directory is not tracked"
fi

check_no_matches \
  "SKNanoCore/DataFormats/AnalyzerTools must not terminate the process directly" \
  rg -n '\b(exit|abort)\s*\(' SKNanoCore DataFormats AnalyzerTools

check_no_matches \
  "SKNanoCore/DataFormats/AnalyzerTools must not use production asserts" \
  rg -n '(^|[^_[:alnum:]])assert\s*\(' SKNanoCore DataFormats AnalyzerTools

exit "$fail"
