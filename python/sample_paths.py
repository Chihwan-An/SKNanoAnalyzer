"""Resolve compact or explicit SKNano sample input specifications."""

from __future__ import annotations

import glob
import os
from typing import Any, Mapping


def resolve_sample_paths(sample_info: Mapping[str, Any]) -> list[str]:
    """Return input ROOT files from a legacy list or a recursive glob.

    Existing sample JSON files keep using ``path: [...]``.  Site-local custom
    productions may instead use ``path_glob`` so their metadata does not need
    thousands of generated path entries.
    """
    if "path" in sample_info:
        paths = sample_info["path"]
        if not isinstance(paths, list) or not all(
                isinstance(path, str) for path in paths):
            raise ValueError("sample 'path' must be a list of strings")
        return list(paths)

    pattern = sample_info.get("path_glob")
    if not isinstance(pattern, str) or not pattern:
        raise ValueError("sample requires either 'path' or 'path_glob'")
    paths = sorted(
        path for path in glob.glob(pattern, recursive=True)
        if os.path.isfile(path) and path.endswith(".root")
    )
    if not paths:
        raise ValueError(f"sample path_glob matched no ROOT files: {pattern}")
    return paths
