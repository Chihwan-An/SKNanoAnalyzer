#!/usr/bin/env python3
"""Extract and deterministically merge NanoAOD TTree branch schemas.

The raw schema is a generated artifact.  Human-maintained policy belongs in a
separate overlay file, so dumping a new input never overwrites annotations such
as required/optional or view-enabled columns.
"""

from __future__ import annotations

import argparse
import copy
import json
import re
import sys
from pathlib import Path
from typing import Any, Iterable

SCHEMA_VERSION = 1

UPROOT_TYPE_MAP = {
    "bool": "Bool_t",
    "char": "Char_t",
    "int8_t": "Char_t",
    "uint8_t": "UChar_t",
    "int16_t": "Short_t",
    "uint16_t": "UShort_t",
    "int32_t": "Int_t",
    "uint32_t": "UInt_t",
    "int64_t": "Long64_t",
    "uint64_t": "ULong64_t",
    "float": "Float_t",
    "double": "Double_t",
}


class SchemaError(RuntimeError):
    """A file cannot participate in a single, type-safe schema union."""


def canonical_json(payload: Any) -> str:
    return json.dumps(payload, indent=2, sort_keys=True, ensure_ascii=False) + "\n"


def read_manifest(path: Path) -> list[Path]:
    entries: list[Path] = []
    for raw_line in path.read_text().splitlines():
        line = raw_line.strip()
        if line and not line.startswith("#"):
            entries.append(Path(line))
    return entries


def cardinality_signature(branch: dict[str, Any]) -> tuple[Any, ...]:
    cardinality = branch["cardinality"]
    return (
        branch["leaf_type"],
        cardinality["kind"],
        cardinality["static_length"],
        cardinality.get("count_branch"),
        cardinality.get("count_leaf_type"),
    )


def validate_file_schema(file_schema: dict[str, Any]) -> None:
    by_name = {branch["name"]: branch for branch in file_schema["branches"]}
    if len(by_name) != len(file_schema["branches"]):
        raise SchemaError(f"{file_schema['path']}: duplicate branch name in one tree")

    for branch in file_schema["branches"]:
        cardinality = branch["cardinality"]
        if cardinality["kind"] != "variable":
            continue
        count_name = cardinality.get("count_branch")
        if not count_name or count_name not in by_name:
            raise SchemaError(
                f"{file_schema['path']}: variable branch '{branch['name']}' "
                f"has missing count branch '{count_name}'"
            )
        expected_type = cardinality.get("count_leaf_type")
        actual_type = by_name[count_name]["leaf_type"]
        if expected_type and expected_type != actual_type:
            raise SchemaError(
                f"{file_schema['path']}: variable branch '{branch['name']}' "
                f"expects count '{count_name}' type {expected_type}, found {actual_type}"
            )


def merge_raw_schemas(file_schemas: Iterable[dict[str, Any]], tree_name: str) -> dict[str, Any]:
    """Return a deterministic union and reject type/cardinality conflicts."""
    ordered_files = sorted(copy.deepcopy(list(file_schemas)), key=lambda item: item["path"])
    if not ordered_files:
        raise SchemaError("no input schemas")

    union: dict[str, dict[str, Any]] = {}
    files: list[dict[str, Any]] = []
    for file_schema in ordered_files:
        if file_schema["tree"] != tree_name:
            raise SchemaError(
                f"{file_schema['path']}: expected tree '{tree_name}', found '{file_schema['tree']}'"
            )
        validate_file_schema(file_schema)
        index = len(files)
        names: list[str] = []
        for branch in sorted(file_schema["branches"], key=lambda item: item["name"]):
            names.append(branch["name"])
            existing = union.get(branch["name"])
            if existing is None:
                union[branch["name"]] = copy.deepcopy(branch)
                union[branch["name"]]["available_in"] = [index]
            elif cardinality_signature(existing) != cardinality_signature(branch):
                raise SchemaError(
                    f"schema conflict for branch '{branch['name']}': "
                    f"{cardinality_signature(existing)} vs {cardinality_signature(branch)} "
                    f"in {file_schema['path']}"
                )
            else:
                existing["available_in"].append(index)
        files.append({"path": file_schema["path"], "tree": tree_name, "branches": names})

    branches = [union[name] for name in sorted(union)]
    all_indices = list(range(len(files)))
    for branch in branches:
        branch["available_in"].sort()
        branch["required_in_all_inputs"] = branch["available_in"] == all_indices

    return {
        "schema_version": SCHEMA_VERSION,
        "tree": tree_name,
        "files": files,
        "branches": branches,
    }


def merge_overlay(raw_schema: dict[str, Any], overlay: dict[str, Any]) -> dict[str, Any]:
    """Attach reviewed metadata without allowing the overlay to alter raw facts."""
    merged = copy.deepcopy(raw_schema)
    overrides = overlay.get("overrides", {})
    derived = overlay.get("derived", {})
    if not isinstance(overrides, dict) or not isinstance(derived, dict):
        raise SchemaError("overlay 'overrides' and 'derived' must be objects")

    by_name = {branch["name"]: branch for branch in merged["branches"]}
    unknown = sorted(set(overrides) - set(by_name))
    if unknown:
        raise SchemaError("overlay references raw branches that do not exist: " + ", ".join(unknown))

    protected = {"name", "leaf_type", "cardinality", "available_in", "required_in_all_inputs"}
    for name in sorted(overrides):
        metadata = overrides[name]
        if not isinstance(metadata, dict):
            raise SchemaError(f"overlay '{name}' must be an object")
        attempted_fact_override = protected & set(metadata)
        if attempted_fact_override:
            raise SchemaError(
                f"overlay '{name}' cannot override raw schema fields: "
                + ", ".join(sorted(attempted_fact_override))
            )
        by_name[name]["metadata"] = copy.deepcopy(metadata)

    merged["derived"] = {name: derived[name] for name in sorted(derived)}
    merged["overlay_version"] = overlay.get("overlay_version", 1)
    return merged


def availability_report(raw_schema: dict[str, Any]) -> dict[str, Any]:
    """Produce a human-readable union/intersection report from raw schema only."""
    files = raw_schema["files"]
    availability = []
    for branch in raw_schema["branches"]:
        indices = branch["available_in"]
        availability.append(
            {
                "name": branch["name"],
                "available_in": [files[index]["path"] for index in indices],
                "present_in_all_inputs": branch["required_in_all_inputs"],
            }
        )
    return {
        "schema_version": raw_schema["schema_version"],
        "tree": raw_schema["tree"],
        "input_count": len(files),
        "union_branch_count": len(raw_schema["branches"]),
        "intersection_branch_count": sum(
            branch["required_in_all_inputs"] for branch in raw_schema["branches"]
        ),
        "availability": availability,
    }


def leaf_schema(branch: Any, leaf: Any) -> dict[str, Any]:
    class_name = branch.GetClassName()
    if class_name:
        raise SchemaError(
            f"branch '{branch.GetName()}' has unsupported object type '{class_name}'"
        )
    count_leaf = leaf.GetLeafCount()
    static_length = int(leaf.GetLenStatic())
    if count_leaf:
        cardinality = {
            "kind": "variable",
            "static_length": static_length,
            "count_branch": str(count_leaf.GetName()),
            "count_leaf_type": str(count_leaf.GetTypeName()),
        }
    elif static_length > 1:
        cardinality = {"kind": "fixed", "static_length": static_length}
    else:
        cardinality = {"kind": "scalar", "static_length": 1}
    return {
        "name": str(branch.GetName()),
        "leaf_type": str(leaf.GetTypeName()),
        "cardinality": cardinality,
    }


def extract_file_schema(path: Path, tree_name: str) -> dict[str, Any]:
    try:
        import ROOT  # type: ignore[import-not-found]
    except ImportError:
        return extract_file_schema_uproot(path, tree_name)

    root_file = ROOT.TFile.Open(str(path), "READ")
    if not root_file or root_file.IsZombie():
        raise SchemaError(f"cannot open ROOT file '{path}'")
    try:
        tree = root_file.Get(tree_name)
        if not tree:
            raise SchemaError(f"{path}: tree '{tree_name}' does not exist")
        branches: list[dict[str, Any]] = []
        for branch in tree.GetListOfBranches():
            leaves = branch.GetListOfLeaves()
            if leaves.GetEntries() != 1:
                raise SchemaError(
                    f"{path}: branch '{branch.GetName()}' has {leaves.GetEntries()} leaves; unsupported"
                )
            branches.append(leaf_schema(branch, leaves.At(0)))
        return {"path": str(path.resolve()), "tree": tree_name, "branches": branches}
    finally:
        root_file.Close()


def uproot_leaf_type(typename: str) -> str:
    root_type = UPROOT_TYPE_MAP.get(typename)
    if root_type is None:
        raise SchemaError(f"unsupported uproot leaf type '{typename}'")
    return root_type


def extract_file_schema_uproot(path: Path, tree_name: str) -> dict[str, Any]:
    """Fallback for environments that ship ROOT but not its Python bindings.

    Uproot exposes the same leaf-count relation through ``count_branch``.  The
    fallback deliberately supports only primitive leaves; object branches must
    remain an explicit schema error just as they are in the PyROOT path.
    """
    try:
        import uproot  # type: ignore[import-not-found]
    except ImportError as error:
        raise SchemaError("PyROOT or uproot is required to inspect ROOT inputs") from error

    try:
        root_file = uproot.open(path)
        tree = root_file[tree_name]
    except Exception as error:
        raise SchemaError(f"cannot open tree '{tree_name}' in ROOT file '{path}': {error}") from error

    branches: list[dict[str, Any]] = []
    fixed_pattern = re.compile(r"^(?P<type>.+)\[(?P<length>[1-9][0-9]*)\]$")
    try:
        for name in tree.keys():
            branch = tree[name]
            typename = str(branch.typename)
            count_branch = getattr(branch, "count_branch", None)
            if count_branch is not None:
                base_type = typename.removesuffix("[]")
                if base_type == typename:
                    raise SchemaError(
                        f"{path}: branch '{name}' has a count branch but non-jagged type '{typename}'"
                    )
                cardinality = {
                    "kind": "variable",
                    "static_length": 1,
                    "count_branch": str(count_branch.name),
                    "count_leaf_type": uproot_leaf_type(str(count_branch.typename)),
                }
            else:
                fixed_match = fixed_pattern.fullmatch(typename)
                if fixed_match:
                    base_type = fixed_match.group("type")
                    cardinality = {
                        "kind": "fixed",
                        "static_length": int(fixed_match.group("length")),
                    }
                else:
                    base_type = typename
                    cardinality = {"kind": "scalar", "static_length": 1}
            branches.append(
                {
                    "name": str(name),
                    "leaf_type": uproot_leaf_type(base_type),
                    "cardinality": cardinality,
                }
            )
    except SchemaError:
        raise
    except Exception as error:
        raise SchemaError(f"cannot inspect ROOT file '{path}': {error}") from error
    finally:
        root_file.close()

    return {"path": str(path.resolve()), "tree": tree_name, "branches": branches}


def load_json(path: Path) -> dict[str, Any]:
    try:
        payload = json.loads(path.read_text())
    except (OSError, json.JSONDecodeError) as error:
        raise SchemaError(f"cannot read JSON '{path}': {error}") from error
    if not isinstance(payload, dict):
        raise SchemaError(f"JSON '{path}' must contain an object")
    return payload


def write_json(path: Path, payload: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(canonical_json(payload))


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input", action="append", type=Path, default=[], help="ROOT input (repeatable)")
    parser.add_argument("--manifest", type=Path, help="newline-delimited ROOT inputs")
    parser.add_argument("--tree", default="Events", help="tree name (default: Events)")
    parser.add_argument("--raw-out", type=Path, required=True, help="generated raw schema output")
    parser.add_argument("--overlay", type=Path, help="manual overlay JSON")
    parser.add_argument("--merged-out", type=Path, help="merged schema output (requires --overlay)")
    parser.add_argument("--report-out", type=Path, help="availability union/intersection report")
    parser.add_argument("--check", type=Path, help="fail unless raw output is byte-identical to this reference")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    paths = list(args.input)
    if args.manifest:
        paths.extend(read_manifest(args.manifest))
    if not paths:
        raise SchemaError("provide at least one --input or --manifest")
    if args.merged_out and not args.overlay:
        raise SchemaError("--merged-out requires --overlay")

    raw_schema = merge_raw_schemas(
        [extract_file_schema(path, args.tree) for path in paths], args.tree
    )
    raw_text = canonical_json(raw_schema)
    if args.check and args.check.read_text() != raw_text:
        raise SchemaError(f"schema drift: '{args.check}' differs from generated raw schema")
    write_json(args.raw_out, raw_schema)
    if args.report_out:
        write_json(args.report_out, availability_report(raw_schema))

    if args.overlay:
        merged = merge_overlay(raw_schema, load_json(args.overlay))
        write_json(args.merged_out or args.raw_out.with_name("merged_schema.json"), merged)
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except SchemaError as error:
        print(f"dump_branch_schema.py: error: {error}", file=sys.stderr)
        raise SystemExit(2)
