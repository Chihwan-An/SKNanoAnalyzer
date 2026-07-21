from __future__ import annotations

import re
from collections import defaultdict
from pathlib import Path
from typing import Any

import yaml

from .catalog import Catalog


def _safe_regex_segment(value: str) -> str:
    return re.escape(value)


def build_draft(catalog: Catalog) -> dict[str, Any]:
    """Build a conservative draft: one mutually-exclusive rule per path depth/kind."""
    groups: dict[tuple[int, str], list[list[str]]] = defaultdict(list)
    for entry in catalog.entries:
        groups[(len(entry.path.split("/")), entry.kind)].append(entry.path.split("/"))

    rules: list[dict[str, Any]] = []
    recipes: list[dict[str, Any]] = []
    for index, ((depth, kind), paths) in enumerate(sorted(groups.items())):
        common: list[str | None] = []
        for level in range(depth):
            values = {parts[level] for parts in paths}
            common.append(next(iter(values)) if len(values) == 1 else None)
        pieces: list[str] = []
        used_names: set[str] = set()
        for level, value in enumerate(common):
            is_leaf = level == depth - 1
            if value is not None and not is_leaf:
                pieces.append(_safe_regex_segment(value))
                continue
            name = "observable" if is_leaf else f"level_{level}"
            if name in used_names:
                name = f"{name}_{level}"
            used_names.add(name)
            pieces.append(f"(?P<{name}>[^/]+)")
        rule_name = f"paths_{depth}_{kind.lower()}_{index}"
        rules.append({"name": rule_name, "kinds": [kind], "pattern": "/".join(pieces)})
        recipes.append(
            {
                "name": f"single_{rule_name}",
                "renderer": "heatmap" if kind == "TH2" else "hist1d",
                "source": rule_name,
                "enabled": False,
                "facets": [name for name in used_names],
            }
        )
        if kind == "TH1":
            variable_dimensions = [name for name in used_names if name != "observable"]
            for dimension in sorted(variable_dimensions):
                recipes.append(
                    {
                        "name": f"overlay_{rule_name}_{dimension}",
                        "renderer": "overlay",
                        "source": rule_name,
                        "enabled": False,
                        "facets": [name for name in sorted(used_names) if name != dimension],
                        "series": dimension,
                        "normalize": "none",
                    }
                )

    return {
        "version": 1,
        "status": "draft",
        "metadata": {
            "title": Path(catalog.input_path).stem,
            "cms_label": "Preliminary",
            "catalog_fingerprint": catalog.fingerprint,
        },
        "tree": {"rules": rules},
        "styles": {},
        "recipes": recipes,
    }


def write_draft(catalog: Catalog, path: str | Path) -> None:
    Path(path).write_text(
        yaml.safe_dump(build_draft(catalog), sort_keys=False, allow_unicode=True),
        encoding="utf-8",
    )
