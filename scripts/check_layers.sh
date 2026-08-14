#!/usr/bin/env bash
set -euo pipefail
shopt -s nullglob

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

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

# Concrete analyzers live in external <owner>_Analyzers module repositories,
# and the backend is not supposed to know any of their names -- listing them
# here would be the very coupling this check exists to prevent. So read the
# names off whichever modules happen to be checked out, via each module's ROOT
# LinkDef, instead of hardcoding anyone's analyses. A bare clone has no modules
# and nothing to check, which is the correct answer for a bare clone.
module_classes() {
  local linkdef
  for linkdef in "${repo_root}"/*_Analyzers/*/include/*/LinkDef.hpp \
                 "${repo_root}"/*_Analyzers/*/LinkDef.hpp; do
    sed -n 's/^[[:space:]]*#pragma[[:space:]]\{1,\}link[[:space:]]\{1,\}C++[[:space:]]\{1,\}class[[:space:]]\{1,\}\([A-Za-z_][A-Za-z0-9_]*\)[+-]\{0,1\};.*/\1/p' \
      "$linkdef"
  done | sort -u
}

module_class_pattern="$(module_classes | paste -sd'|' -)"
if [[ -n "${module_class_pattern}" ]]; then
  check_no_matches \
    "AnalyzerFramework must not contain concrete analysis modules" \
    rg -n "\b(class|struct)\s+(${module_class_pattern})\b" AnalyzerFramework
else
  echo "[check_layers] OK: no *_Analyzers module checked out, nothing to check"
fi

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
