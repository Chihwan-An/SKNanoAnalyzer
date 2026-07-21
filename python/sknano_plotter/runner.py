from __future__ import annotations

import json
import os
from concurrent.futures import ProcessPoolExecutor, as_completed
from pathlib import Path
from typing import Any

from .gallery import write_gallery
from .render import render_task


def _render_payload(payload: tuple[Any, ...]) -> dict[str, Any]:
    return render_task(*payload)


def _record_path(output: Path, run_id: str, task_id: str) -> Path:
    return output / ".records" / run_id / f"{task_id}.json"


def write_record(output: str | Path, run_id: str, record: dict[str, Any]) -> None:
    output_path = Path(output)
    record_path = _record_path(output_path, run_id, record["task_id"])
    record_path.parent.mkdir(parents=True, exist_ok=True)
    temporary = record_path.with_suffix(".tmp")
    temporary.write_text(json.dumps(record, indent=2, sort_keys=True), encoding="utf-8")
    temporary.replace(record_path)


def read_records(output: str | Path, run_id: str) -> list[dict[str, Any]]:
    record_dir = Path(output) / ".records" / run_id
    if not record_dir.exists():
        return []
    return [json.loads(path.read_text(encoding="utf-8")) for path in sorted(record_dir.glob("*.json"))]


def finalize_output(output: str | Path, run_manifest: dict[str, Any]) -> list[dict[str, Any]]:
    output_path = Path(output)
    records = read_records(output_path, run_manifest["run_id"])
    write_gallery(output_path, records)
    result = dict(run_manifest)
    result["completed"] = len(records)
    result["records"] = records
    destination = output_path / "manifest.json"
    temporary = output_path / f".manifest.{os.getpid()}.tmp"
    temporary.write_text(json.dumps(result, indent=2, sort_keys=True), encoding="utf-8")
    temporary.replace(destination)
    return records


def run_local(manifest: dict[str, Any], workers: int = 0) -> list[dict[str, Any]]:
    output = Path(manifest["output"])
    output.mkdir(parents=True, exist_ok=True)
    tasks = manifest["tasks"]
    if not tasks:
        finalize_output(output, manifest)
        return []
    worker_count = workers if workers > 0 else (os.cpu_count() or 1)
    worker_count = max(1, min(worker_count, len(tasks)))
    payloads = [
        (
            manifest["input"], task, output, manifest["formats"],
            manifest.get("metadata", {}), manifest.get("styles", {}),
        )
        for task in tasks
    ]
    records: list[dict[str, Any]] = []
    if worker_count == 1:
        for payload in payloads:
            record = _render_payload(payload)
            write_record(output, manifest["run_id"], record)
            records.append(record)
    else:
        with ProcessPoolExecutor(max_workers=worker_count) as executor:
            futures = {executor.submit(_render_payload, payload): payload[1]["task_id"] for payload in payloads}
            for future in as_completed(futures):
                record = future.result()
                write_record(output, manifest["run_id"], record)
                records.append(record)
    finalize_output(output, manifest)
    return sorted(records, key=lambda record: record["task_id"])


def run_chunk(manifest: dict[str, Any], chunk_index: int, chunk_size: int) -> list[dict[str, Any]]:
    if chunk_index < 0 or chunk_size < 1:
        raise ValueError("chunk index must be non-negative and chunk size must be positive")
    start = chunk_index * chunk_size
    tasks = manifest["tasks"][start : start + chunk_size]
    records: list[dict[str, Any]] = []
    for task in tasks:
        record = render_task(
            manifest["input"], task, manifest["output"], manifest["formats"],
            manifest.get("metadata", {}), manifest.get("styles", {}),
        )
        write_record(manifest["output"], manifest["run_id"], record)
        records.append(record)
    finalize_output(manifest["output"], manifest)
    return records
