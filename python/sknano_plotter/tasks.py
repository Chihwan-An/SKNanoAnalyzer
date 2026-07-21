from __future__ import annotations

import hashlib
import json
import re
from collections import defaultdict
from dataclasses import asdict
from pathlib import Path
from typing import Any

from .catalog import Catalog
from .schema import MappedEntry, PlotSchema, Recipe, ValidationReport


def _matches(value: str | None, expected: Any) -> bool:
    if value is None:
        return False
    if isinstance(expected, list):
        return any(_matches(value, item) for item in expected)
    if isinstance(expected, dict):
        if "regex" in expected:
            return re.fullmatch(str(expected["regex"]), value) is not None
        if "in" in expected:
            return value in {str(item) for item in expected["in"]}
        if "not" in expected:
            return not _matches(value, expected["not"])
    text = str(expected)
    if text.startswith("re:"):
        return re.fullmatch(text[3:], value) is not None
    return value == text


def role_matches(value: str, roles: dict[str, Any], role: str) -> bool:
    return role in roles and _matches(value, roles[role])


def _selected(mapped: MappedEntry, recipe: Recipe) -> bool:
    return all(_matches(mapped.dimensions.get(name), expected) for name, expected in recipe.select.items())


def _member(mapped: MappedEntry) -> dict[str, Any]:
    return {
        "path": mapped.entry.path,
        "classname": mapped.entry.classname,
        "ndim": mapped.entry.ndim,
        "title": mapped.entry.title,
        "dimensions": mapped.dimensions,
        "axes": [asdict(axis) for axis in mapped.entry.axes],
    }


def build_tasks(schema: PlotSchema, report: ValidationReport) -> list[dict[str, Any]]:
    tasks: list[dict[str, Any]] = []
    for recipe in schema.recipes:
        if not recipe.enabled:
            continue
        source = [item for item in report.mapped if item.rule == recipe.source and _selected(item, recipe)]
        if recipe.renderer in {"hist1d", "heatmap"}:
            for mapped in source:
                expected_dim = 2 if recipe.renderer == "heatmap" else 1
                if mapped.entry.ndim != expected_dim:
                    continue
                tasks.append(_make_task(recipe, [mapped], mapped.dimensions))
            continue

        groups: dict[tuple[str, ...], list[MappedEntry]] = defaultdict(list)
        for mapped in source:
            key = tuple(mapped.dimensions.get(dimension, "") for dimension in recipe.facets)
            groups[key].append(mapped)
        for key, members in sorted(groups.items()):
            facets = dict(zip(recipe.facets, key))
            tasks.append(_make_task(recipe, members, facets))

    for index, task in enumerate(tasks):
        task["task_id"] = f"{index:06d}-{task['recipe']}"
    return tasks


def _make_task(recipe: Recipe, members: list[MappedEntry], facets: dict[str, str]) -> dict[str, Any]:
    return {
        "recipe": recipe.name,
        "renderer": recipe.renderer,
        "facets": facets,
        "series": recipe.series,
        "roles": recipe.roles,
        "normalize": recipe.normalize,
        "rebin": recipe.rebin,
        "flow": recipe.flow,
        "log": recipe.log,
        "ratio": recipe.ratio,
        "ratio_range": list(recipe.ratio_range),
        "systematics": recipe.systematics,
        "style": recipe.style,
        "output": recipe.output,
        "members": [_member(member) for member in members],
    }


def write_task_manifest(
    path: str | Path,
    input_path: str,
    output_path: str,
    schema_path: str,
    tasks: list[dict[str, Any]],
    metadata: dict[str, Any],
    styles: dict[str, Any],
    fingerprint: dict[str, Any],
    formats: list[str],
) -> dict[str, Any]:
    run_payload = {
        "input": input_path,
        "schema": schema_path,
        "fingerprint": fingerprint,
        "formats": formats,
        "metadata": metadata,
        "styles": styles,
        "tasks": tasks,
    }
    run_id = hashlib.sha256(json.dumps(run_payload, sort_keys=True).encode("utf-8")).hexdigest()[:16]
    manifest = {
        "version": 1,
        "run_id": run_id,
        "input": input_path,
        "output": output_path,
        "schema": schema_path,
        "metadata": metadata,
        "styles": styles,
        "fingerprint": fingerprint,
        "formats": formats,
        "tasks": tasks,
    }
    Path(path).write_text(json.dumps(manifest, indent=2, sort_keys=True), encoding="utf-8")
    return manifest


def read_task_manifest(path: str | Path) -> dict[str, Any]:
    return json.loads(Path(path).read_text(encoding="utf-8"))
