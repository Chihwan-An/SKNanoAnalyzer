#!/usr/bin/env python3
"""Generate C++ branch scaffolding for SKNanoLoader without TTreeReader."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Dict, Iterable, List, Tuple
import os

TYPE_MAP: Dict[str, str] = {
    "Float_t": "float",
    "Double_t": "double",
    "Int_t": "int",
    "UInt_t": "unsigned int",
    "Bool_t": "bool",
    "Short_t": "short",
    "UShort_t": "unsigned short",
    "UChar_t": "unsigned char",
    "ULong64_t": "ULong64_t",
}

HEADER_NAME = "generated_branch_decls.inc"
REGISTER_NAME = "generated_branch_register.inc"
RESET_NAME = "generated_branch_reset.inc"


def discover_json(base_dir: Path) -> Path:
    print(f"[generate_branch_code] looking for NanoAODv15.json in {base_dir}")
    candidates = [
        base_dir / Path("NanoAODv15.json"),
        base_dir / Path("data") / Path("Run3_v15_Run2_v15") / Path("NanoAODv15.json"),
    ]
    for path in candidates:
        if path.exists():
            return path
    raise FileNotFoundError("Could not locate NanoAODv15.json")


def cpp_type(root_type: str) -> str:
    if root_type not in TYPE_MAP:
        raise KeyError(f"Unsupported ROOT type '{root_type}'")
    return TYPE_MAP[root_type]


def find_count_key(collection: str, leaves: Iterable[str]) -> str | None:
    variants = {
        f"n{collection}",
        f"n{collection[0].upper()}{collection[1:]}" if collection else "",
        f"n{collection.lower()}",
        f"n{collection.capitalize()}",
    }
    for candidate in variants:
        if candidate and candidate in leaves:
            return candidate
    return None


def build_blocks(events: Dict[str, Dict[str, Dict[str, str]]]) -> Tuple[List[str], List[str], List[str]]:
    decls: List[str] = []
    regs: List[str] = []
    resets: List[str] = []

    for collection, leaves in events.items():
        if not isinstance(leaves, dict):
            continue
        keys_in_order = list(leaves.keys())
        count_key = find_count_key(collection, keys_in_order)

        decls.append(f"    // ===== {collection} =====")
        regs.append(f"    // ===== {collection} =====")
        resets.append(f"    // ===== {collection} =====")

        if count_key:
            ordered_keys = [count_key] + [k for k in keys_in_order if k != count_key]
        else:
            ordered_keys = keys_in_order

        # Small hard-coding, if key == run->RunNumber
        print(f"[generate_branch_code] processing collection '{collection}' with count key '{count_key}'")
        if collection == "run":
            ordered_keys = [k if k != "run" else "RunNumber" for k in ordered_keys]
            leaves["RunNumber"] = leaves.pop("run")

        for leaf in ordered_keys:
            info = leaves[leaf]
            if "type" not in info:
                continue
            ctype = cpp_type(info["type"])
            is_counter = (leaf == count_key)
            is_vector_group = count_key is not None and not is_counter

            if is_vector_group:
                count_info = leaves[count_key]
                count_ctype = cpp_type(count_info["type"])
                decls.append(f"    BranchVector<{ctype}, {count_ctype}> {leaf}{{\"{leaf}\", {count_key}}};")
                regs.append(f"    branchManager.registerVector({leaf});")
                resets.append(f"    branchManager.reset({leaf});")
            else:
                decls.append(f"    BranchScalar<{ctype}> {leaf}{{\"{leaf}\"}};")
                regs.append(f"    branchManager.registerScalar({leaf});")
                resets.append(f"    branchManager.reset({leaf});")

        decls.append("")
        regs.append("")
        resets.append("")

    return decls, regs, resets


def write_file(path: Path, lines: Iterable[str]) -> None:
    content = "\n".join(lines).rstrip() + "\n"
    path.write_text(content)
    print(f"[generate_branch_code] wrote {path}")


def main() -> None:
    parser = argparse.ArgumentParser(description="Generate branch C++ includes")
    parser.add_argument("--json", type=Path, help="Path to NanoAODv15.json")
    parser.add_argument("--out-dir", type=Path, help="Dir for generated files")
    args = parser.parse_args()

    base_dir = os.environ.get("SKNANO_DATA")
    json_path = args.json or discover_json(base_dir)
    out_dir = args.out_dir or json_path.parent

    with json_path.open() as jf:
        payload = json.load(jf)

    events = payload.get("Events Content")
    if not isinstance(events, dict):
        raise ValueError("JSON missing 'Events Content'")

    decls, regs, resets = build_blocks(events)

    write_file(out_dir / HEADER_NAME, decls)
    write_file(out_dir / REGISTER_NAME, regs)
    write_file(out_dir / RESET_NAME, resets)


if __name__ == "__main__":
    main()
