#!/usr/bin/env python3
"""
Convert ModellingPatch analyzer ROOT output into a JSON summary, mirroring the
structure produced by ModellingPatch/ModellingPatch.py.
"""

import argparse
import json
from pathlib import Path
from typing import Any, Dict, List, Optional

import uproot


def _try_get_hist(directory: uproot.reading.ReadOnlyDirectory, name: str):
    """Return the histogram if it exists, otherwise None."""
    try:
        return directory[name]
    except KeyError:
        try:
            return directory[f"{name};1"]
        except KeyError:
            return None


def _sum_hist(directory: uproot.reading.ReadOnlyDirectory, name: str) -> Optional[float]:
    hist = _try_get_hist(directory, name)
    if hist is None:
        return None
    return float(hist.values().sum())


def _vector_hist(directory: uproot.reading.ReadOnlyDirectory, name: str) -> Optional[List[float]]:
    hist = _try_get_hist(directory, name)
    if hist is None:
        return None
    return [float(v) for v in hist.values()]


def _patch_list(sum_sign: float, values: Optional[List[float]]) -> Optional[List[Optional[float]]]:
    if values is None:
        return None
    patched: List[Optional[float]] = []
    for v in values:
        if v == 0:
            patched.append(None)
        else:
            patched.append(sum_sign / v)
    return patched


def _patch_scalar(sum_sign: float, value: Optional[float]) -> Optional[float]:
    if value is None or value == 0:
        return None
    return sum_sign / value


def _extract_subprocess(name: str, directory: uproot.reading.ReadOnlyDirectory) -> Dict[str, Any]:
    nmc = _sum_hist(directory, "NEvents") or 0.0
    sum_sign = _sum_hist(directory, "sumSign") or 0.0

    sum_scale = _vector_hist(directory, "sumScaleVariation")
    sum_ps = _vector_hist(directory, "sumPSVariation")

    sum_hdamp_up = _sum_hist(directory, "sum_hdamp_up")
    sum_hdamp_down = _sum_hist(directory, "sum_hdamp_down")
    sum_minnlo = _sum_hist(directory, "sum_minnlo")
    sum_bfrag_nom = _sum_hist(directory, "sum_bfrag_nom")
    sum_bfrag_up = _sum_hist(directory, "sum_bfrag_up")

    data: Dict[str, Any] = {
        "nMC": int(round(nmc)),
        "sumSign": sum_sign,
        "sumScaleVariation": sum_scale,
        "sumPSVariation": sum_ps,
        "sum_hdamp_up": sum_hdamp_up,
        "sum_hdamp_down": sum_hdamp_down,
        "sum_minnlo": sum_minnlo,
    }

    if sum_bfrag_nom is not None:
        data["sum_bfrag_nom"] = sum_bfrag_nom
    if sum_bfrag_up is not None:
        data["sum_bfrag_up"] = sum_bfrag_up

    data["patch_ScaleVariation"] = _patch_list(sum_sign, sum_scale)
    data["patch_PSVariation"] = _patch_list(sum_sign, sum_ps)
    data["patch_hdamp_up"] = _patch_scalar(sum_sign, sum_hdamp_up)
    data["patch_hdamp_down"] = _patch_scalar(sum_sign, sum_hdamp_down)
    data["patch_minnlo"] = _patch_scalar(sum_sign, sum_minnlo)

    if sum_bfrag_nom is not None:
        data["patch_bfrag_nom"] = _patch_scalar(sum_sign, sum_bfrag_nom)
    if sum_bfrag_up is not None:
        data["patch_bfrag_up"] = _patch_scalar(sum_sign, sum_bfrag_up)

    return data


def _derive_pd(input_path: Path, override: Optional[str]) -> str:
    if override:
        return override
    return input_path.stem


def _derive_era(input_path: Path, override: Optional[str]) -> str:
    if override:
        return override
    parent = input_path.parent.name
    return parent


def build_summary(input_path: Path, pd: str, era: str) -> Dict[str, Any]:
    with uproot.open(input_path) as f:
        subprocesses: Dict[str, Dict[str, Any]] = {}
        for key in f.keys():
            obj = f[key]
            if not isinstance(obj, uproot.reading.ReadOnlyDirectory):
                continue
            name = key.split(";")[0].split("/")[-1]
            subprocesses[name] = _extract_subprocess(name, obj)

    return {
        "PD": pd,
        "era": era,
        "input": str(input_path),
        "subprocesses": subprocesses,
    }


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Make ModellingPatch JSON summary from analyzer ROOT output.",
    )
    parser.add_argument(
        "input",
        help="Path to ModellingPatch ROOT file or directory of ROOT files (non-recursive)",
    )
    parser.add_argument(
        "-o",
        "--output",
        help=(
            "Output JSON path. For a single file: explicit file or directory."
            " For a directory input: must be a directory (default: alongside each ROOT file)"
        ),
    )
    parser.add_argument("--pd", help="PD name (default: input filename stem)")
    parser.add_argument("--era", help="Era string (default: parent directory name)")
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    input_path = Path(args.input).expanduser()
    if not input_path.exists():
        raise FileNotFoundError(f"Input path not found: {input_path}")

    def resolve_output_path(src: Path, pd: str, era: str) -> Path:
        if args.output:
            out_base = Path(args.output)
            if input_path.is_dir():
                if out_base.suffix:
                    raise ValueError(
                        "--output must be a directory when processing a directory of ROOT files"
                    )
                return (out_base / f"{pd}_{era}_summary.json").resolve()
            if out_base.is_dir() or not out_base.suffix:
                return (out_base / f"{pd}_{era}_summary.json").resolve()
            return out_base.resolve()
        if input_path.is_dir():
            return (src.parent / f"{pd}_{era}_summary.json").resolve()
        return (src.with_name(f"{pd}_{era}_summary.json")).resolve()

    def write_summary(src: Path) -> None:
        pd = _derive_pd(src, args.pd)
        era = _derive_era(src, args.era)
        summary = build_summary(src, pd=pd, era=era)
        out_path = resolve_output_path(src, pd, era)
        out_path.parent.mkdir(parents=True, exist_ok=True)
        with out_path.open("w") as f:
            json.dump(summary, f, indent=2)
        print(f"Wrote {out_path} from {src}")

    if input_path.is_dir():
        root_files = sorted(p for p in input_path.glob("*.root") if p.is_file())
        if not root_files:
            raise FileNotFoundError(f"No ROOT files found in directory: {input_path}")
        for rf in root_files:
            write_summary(rf)
    else:
        write_summary(input_path)


if __name__ == "__main__":
    main()
