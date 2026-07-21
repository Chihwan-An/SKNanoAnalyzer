#!/usr/bin/env python3
"""Run a workload repeatedly and preserve a reproducible D0 baseline artifact."""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import statistics
import subprocess
import sys
import tempfile
from pathlib import Path
from typing import Any


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def tool_version(command: list[str]) -> str:
    try:
        result = subprocess.run(command, check=True, capture_output=True, text=True)
    except (OSError, subprocess.CalledProcessError):
        return "unavailable"
    return (result.stdout or result.stderr).splitlines()[0].strip()


def flatten_metrics(report: dict[str, Any]) -> dict[str, float]:
    metrics = {
        "wall_seconds": float(report["wall_seconds"]),
        "cpu_seconds": float(report["cpu_seconds"]),
        "events_per_second": float(report.get("events_per_second", 0.0)),
        "cpu_seconds_per_event": float(
            report.get("cpu_seconds_per_event", 0.0)),
        "peak_rss_kib": float(report["peak_rss_kib"]),
    }
    for name, value in report.get("counters", {}).items():
        metrics[f"counter.{name}"] = float(value)
    for name, value in report.get("phases", {}).items():
        metrics[f"phase.{name}.seconds"] = float(value["seconds"])
        metrics[f"phase.{name}.calls"] = float(value["calls"])
    return metrics


def aggregate(reports: list[dict[str, Any]]) -> dict[str, dict[str, float]]:
    flattened = [flatten_metrics(report) for report in reports]
    keys = sorted(set.intersection(*(set(item) for item in flattened)))
    result: dict[str, dict[str, float]] = {}
    for key in keys:
        values = [item[key] for item in flattened]
        median = statistics.median(values)
        deviations = [abs(value - median) for value in values]
        result[key] = {
            "median": median,
            "mad": statistics.median(deviations),
            "minimum": min(values),
            "maximum": max(values),
        }
    return result


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--workload", required=True)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--runs", type=int, default=5)
    parser.add_argument("--warmup-runs", type=int, default=0,
                        help="successful unmeasured runs before collection")
    parser.add_argument("--cache-mode", choices=("warm", "cold", "remote"),
                        required=True)
    parser.add_argument("--input", action="append", type=Path, default=[])
    parser.add_argument("command", nargs=argparse.REMAINDER)
    args = parser.parse_args()
    if args.command and args.command[0] == "--":
        args.command = args.command[1:]
    if not args.command:
        parser.error("provide a command after --")
    if args.runs < 1:
        parser.error("--runs must be positive")
    if args.warmup_runs < 0:
        parser.error("--warmup-runs must not be negative")
    return args


def main() -> int:
    args = parse_args()
    missing = [str(path) for path in args.input if not path.is_file()]
    if missing:
        raise RuntimeError("missing benchmark inputs: " + ", ".join(missing))

    reports: list[dict[str, Any]] = []
    run_logs: list[dict[str, Any]] = []
    warmup_logs: list[dict[str, Any]] = []
    with tempfile.TemporaryDirectory(prefix="sknano-d0-") as directory:
        for index in range(args.warmup_runs):
            report_path = Path(directory) / f"warmup-{index}.json"
            environment = os.environ.copy()
            environment["SKNANO_PERFORMANCE_REPORT"] = str(report_path)
            result = subprocess.run(args.command, env=environment,
                                    capture_output=True, text=True)
            warmup_logs.append({
                "run": index,
                "returncode": result.returncode,
                "stdout": result.stdout,
                "stderr": result.stderr,
            })
            if result.returncode != 0:
                raise RuntimeError(
                    f"benchmark warmup {index} failed with "
                    f"{result.returncode}: {result.stderr[-1000:]}")
            if not report_path.is_file():
                raise RuntimeError(
                    f"benchmark warmup {index} did not write {report_path}")

        for index in range(args.runs):
            report_path = Path(directory) / f"run-{index}.json"
            environment = os.environ.copy()
            environment["SKNANO_PERFORMANCE_REPORT"] = str(report_path)
            result = subprocess.run(args.command, env=environment,
                                    capture_output=True, text=True)
            run_logs.append({
                "run": index,
                "returncode": result.returncode,
                "stdout": result.stdout,
                "stderr": result.stderr,
            })
            if result.returncode != 0:
                raise RuntimeError(
                    f"benchmark run {index} failed with {result.returncode}: "
                    f"{result.stderr[-1000:]}")
            if not report_path.is_file():
                raise RuntimeError(
                    f"benchmark run {index} did not write {report_path}")
            reports.append(json.loads(report_path.read_text()))

    artifact = {
        "schema_version": 1,
        "workload": args.workload,
        "cache_mode": args.cache_mode,
        "warmup_runs": args.warmup_runs,
        "runs": args.runs,
        "command": args.command,
        "inputs": [
            {"path": str(path.resolve()), "sha256": sha256(path),
             "size": path.stat().st_size}
            for path in sorted(args.input)
        ],
        "toolchain": {
            "python": sys.version.splitlines()[0],
            "cxx": tool_version([os.environ.get("CXX", "c++"), "--version"]),
            "root": tool_version(["root-config", "--version"]),
            "cmake": tool_version(["cmake", "--version"]),
        },
        "aggregate": aggregate(reports),
        "reports": reports,
        "warmup_logs": warmup_logs,
        "run_logs": run_logs,
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(
        json.dumps(artifact, indent=2, sort_keys=True, ensure_ascii=False) + "\n")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as error:
        print(f"run_performance_baseline: {error}", file=sys.stderr)
        raise SystemExit(1)
