#!/usr/bin/env python3
"""Generate RNTuple field scaffolding for SKNanoLoader."""

from __future__ import annotations

import argparse
import copy
import json
import re
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
VIEW_NAME = "generated_input_columns.inc"
BUNDLE_NAME = "generated_loader_api.inc"


def discover_companion_inputs(
    json_path: Path,
    overlay: Path | None,
    addons: List[Path],
) -> Tuple[Path | None, List[Path]]:
    """Find optional schema metadata without exposing it to the build system."""
    if overlay is None:
        candidate = json_path.parent / "branch_overlay.json"
        overlay = candidate if candidate.is_file() else None

    if not addons:
        addon_dir = json_path.parent / "custom"
        if addon_dir.is_dir():
            addons = sorted(addon_dir.glob("*.json"))

    return overlay, addons


def write_depfile(depfile: Path, outputs: List[Path], inputs: List[Path]) -> None:
    def escape(path: Path) -> str:
        return str(path.resolve()).replace("\\", "\\\\").replace(" ", "\\ ")

    depfile.parent.mkdir(parents=True, exist_ok=True)
    targets = " ".join(escape(path) for path in outputs)
    dependencies = " ".join(escape(path) for path in inputs)
    depfile.write_text(f"{targets}: {dependencies}\n")
    print(f"[generate_branch_code] wrote {depfile}")


def merge_addons(
    base_events: Dict[str, Dict[str, Dict[str, str]]],
    addons: List[dict],
) -> Tuple[Dict[str, Dict[str, Dict[str, str]]], List[dict]]:
    """Merge compact optional branch addons into the canonical schema.

    Addons contain only physical branch names and ROOT leaf types.  They are
    deliberately separate from NanoAODv15.json so a custom production cannot
    silently redefine the standard schema.
    """
    events = copy.deepcopy(base_events)
    view_specs: List[dict] = []
    used_view_names = set()

    for addon in addons:
        addon_name = addon.get("name", "unnamed addon")
        collections = addon.get("collections")
        if not isinstance(collections, dict):
            raise ValueError(f"addon '{addon_name}' has no collections object")

        for collection in sorted(collections):
            config = collections[collection]
            if not isinstance(config, dict):
                raise ValueError(
                    f"addon '{addon_name}' collection '{collection}' must be an object")
            count = config.get("count")
            fields = config.get("fields")
            if not isinstance(count, dict) or not isinstance(fields, dict) or not fields:
                raise ValueError(
                    f"addon '{addon_name}' collection '{collection}' requires count and fields")
            count_name = count.get("name")
            count_type = count.get("type")
            if not isinstance(count_name, str) or not isinstance(count_type, str):
                raise ValueError(
                    f"addon '{addon_name}' collection '{collection}' has invalid count")
            cpp_type(count_type)

            target = events.setdefault(collection, {})
            count_is_addon = count_name not in target

            def merge_leaf(branch_name: str, leaf_type: str) -> None:
                cpp_type(leaf_type)
                existing = target.get(branch_name)
                if existing is not None:
                    if existing.get("type") != leaf_type:
                        raise ValueError(
                            f"addon '{addon_name}' changes type of '{branch_name}' "
                            f"from {existing.get('type')} to {leaf_type}")
                    return
                target[branch_name] = {
                    "description": f"{addon_name} optional input branch",
                    "type": leaf_type,
                }

            merge_leaf(count_name, count_type)
            optional_fields = config.get("optional_fields", [])
            if not isinstance(optional_fields, list) or not all(
                    isinstance(name, str) for name in optional_fields):
                raise ValueError(
                    f"addon '{addon_name}' collection '{collection}' "
                    "optional_fields must be an array of branch names")
            optional_field_set = set(optional_fields)
            unknown_optional = sorted(optional_field_set - set(fields))
            if unknown_optional:
                raise ValueError(
                    f"addon '{addon_name}' collection '{collection}' has "
                    "unknown optional fields: " + ", ".join(unknown_optional))
            normalized_fields = []
            for branch_name in sorted(fields):
                leaf_type = fields[branch_name]
                if not isinstance(branch_name, str) or not isinstance(leaf_type, str):
                    raise ValueError(
                        f"addon '{addon_name}' collection '{collection}' has invalid field")
                merge_leaf(branch_name, leaf_type)
                normalized_fields.append(
                    (branch_name, leaf_type,
                     branch_name not in optional_field_set))

            view_name = config.get("view_name", collection)
            if not isinstance(view_name, str) or not view_name:
                raise ValueError(
                    f"addon '{addon_name}' collection '{collection}' has invalid view_name")
            view_name = cpp_identifier(view_name)
            if view_name in used_view_names:
                raise ValueError(f"duplicate addon view name '{view_name}'")
            used_view_names.add(view_name)
            presence_branch = config.get("presence_branch", count_name)
            known_names = {
                count_name, *(name for name, _, _ in normalized_fields)}
            if presence_branch not in known_names:
                raise ValueError(
                    f"addon view '{view_name}' presence branch '{presence_branch}' "
                    "is not part of the addon collection")
            view_specs.append({
                "addon_name": addon_name,
                "collection": collection,
                "view_name": view_name,
                "count_name": count_name,
                "count_type": count_type,
                "count_is_addon": count_is_addon,
                "presence_branch": presence_branch,
                "fields": normalized_fields,
            })

    return events, view_specs


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


def cpp_member_name(collection: str, branch_name: str) -> str:
    """Return the C++ identifier without changing the physical branch name."""
    if collection == "run" and branch_name == "run":
        return "RunNumber"
    return branch_name


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

        print(f"[generate_branch_code] processing collection '{collection}' with count key '{count_key}'")
        for branch_name in ordered_keys:
            info = leaves[branch_name]
            if "type" not in info:
                continue
            member_name = cpp_member_name(collection, branch_name)
            ctype = cpp_type(info["type"])
            is_counter = (branch_name == count_key)
            is_vector_group = count_key is not None and not is_counter

            if is_vector_group:
                count_info = leaves[count_key]
                count_ctype = cpp_type(count_info["type"])
                count_member_name = cpp_member_name(collection, count_key)
                decls.append(f"    BranchVector<{ctype}, {count_ctype}> {member_name}{{\"{branch_name}\", {count_member_name}}};")
                regs.append(f"    branchManager.registerVector({member_name});")
                resets.append(f"    branchManager.reset({member_name});")
            else:
                decls.append(f"    BranchScalar<{ctype}> {member_name}{{\"{branch_name}\"}};")
                regs.append(f"    branchManager.registerScalar({member_name});")
                resets.append(f"    branchManager.reset({member_name});")

        decls.append("")
        regs.append("")
        resets.append("")

    return decls, regs, resets


def cpp_identifier(value: str) -> str:
    identifier = re.sub(r"[^A-Za-z0-9_]", "_", value)
    if not identifier or identifier[0].isdigit():
        identifier = "_" + identifier
    return identifier


def build_view_blocks(events: Dict[str, Dict[str, Dict[str, str]]],
                      overlay: Dict[str, object]) -> List[str]:
    """Generate composition structs and bind methods for opted-in collections."""
    configurations = overlay.get("view_collections", {})
    if not isinstance(configurations, dict):
        raise ValueError("overlay 'view_collections' must be an object")

    lines: List[str] = []
    for collection in sorted(configurations):
        configuration = configurations[collection]
        if not isinstance(configuration, dict):
            raise ValueError(f"view collection '{collection}' must be an object")
        leaves = events.get(collection)
        if not isinstance(leaves, dict):
            raise ValueError(f"view collection '{collection}' is absent from schema")
        count_key = find_count_key(collection, leaves.keys())
        if not count_key:
            raise ValueError(f"view collection '{collection}' has no count branch")
        overrides = configuration.get("fields", {})
        if not isinstance(overrides, dict):
            raise ValueError(f"view collection '{collection}' fields must be an object")
        default_required = bool(configuration.get("required", True))
        struct_name = cpp_identifier(collection) + "InputColumnsGenerated"
        fields = []
        used_members = set()
        for branch_name, info in leaves.items():
            if branch_name == count_key or "type" not in info:
                continue
            field_config = overrides.get(branch_name, {})
            if not isinstance(field_config, dict):
                raise ValueError(f"field override '{branch_name}' must be an object")
            suffix = (branch_name[len(collection) + 1:]
                      if branch_name.startswith(collection + "_") else branch_name)
            member = cpp_identifier(str(field_config.get("member", suffix)))
            if member in used_members:
                raise ValueError(f"duplicate generated member '{member}' in {collection}")
            used_members.add(member)
            ctype = cpp_type(info["type"])
            required = bool(field_config.get("required", default_required))
            fields.append((branch_name, member, ctype, required))

        lines.append(f"    struct {struct_name} {{")
        for _, member, ctype, required in fields:
            view_type = "BoolColumnView" if ctype == "bool" else f"ColumnView<{ctype}>"
            lines.append(f"        {view_type} {member};")
            lines.append(
                f"        static constexpr bool {member}Required = "
                f"{'true' if required else 'false'};")
            lines.append(
                f"        bool {member}Available() const {{ return {member}.branch() && "
                f"{member}.branch()->valid(); }}")
        lines.append("    };")
        lines.append(f"    void Bind({struct_name} &columns) {{")
        for branch_name, member, _, required in fields:
            branch_member = cpp_member_name(collection, branch_name)
            lines.append(f"        columns.{member}.bind(&{branch_member});")
            if required:
                lines.append(f"        if (!{branch_member}.valid())")
                lines.append(
                    f"            throw SKNano::ConfigError(\"[generated input] required "
                    f"branch '{branch_name}' is unavailable\");")
        lines.append("    }")
        lines.append(f"    {struct_name} Get{cpp_identifier(collection)}InputColumns() {{")
        lines.append(f"        {struct_name} columns;")
        lines.append("        Bind(columns);")
        lines.append("        return columns;")
        lines.append("    }")
        lines.append("")
    return lines


def build_addon_view_blocks(view_specs: List[dict]) -> List[str]:
    """Generate optional, event-scoped row views for custom branch addons."""
    lines: List[str] = []
    signed_types = {"Int_t", "Short_t", "Long64_t"}

    for spec in view_specs:
        collection = spec["collection"]
        view_name = spec["view_name"]
        count_name = spec["count_name"]
        count_type = spec["count_type"]
        count_ctype = cpp_type(count_type)
        count_member = cpp_member_name(collection, count_name)
        struct_name = f"{view_name}ViewCollectionGenerated"
        fields = []
        used_members = set()
        for branch_name, leaf_type, required in spec["fields"]:
            suffix = (branch_name[len(collection) + 1:]
                      if branch_name.startswith(collection + "_") else branch_name)
            member = cpp_identifier(suffix)
            if member in used_members:
                raise ValueError(f"duplicate addon member '{member}' in {view_name}")
            used_members.add(member)
            fields.append((branch_name, member, cpp_type(leaf_type), required))

        presence_branch = spec["presence_branch"]
        if presence_branch == count_name:
            presence_expression = "count && count->valid()"
        else:
            presence_member = next(
                member for branch, member, _, _ in fields
                if branch == presence_branch)
            presence_expression = f"{presence_member}.available()"

        lines.append(f"    // Optional {spec['addon_name']} collection: {collection}")
        lines.append(f"    struct {struct_name} {{")
        lines.append("        enum class Availability { Missing, Complete, Partial };")
        lines.append(f"        const BranchScalar<{count_ctype}> *count = nullptr;")
        lines.append("        const std::uint64_t *epochSource = nullptr;")
        lines.append("        std::uint64_t boundEpoch = 0;")
        for _, member, ctype, required in fields:
            view_type = "BoolColumnView" if ctype == "bool" else f"ColumnView<{ctype}>"
            lines.append(f"        {view_type} {member};")
            lines.append(
                f"        bool {member}Available() const {{ return {member}.available(); }}")
            lines.append(
                f"        static constexpr bool {member}Required = "
                f"{'true' if required else 'false'};")
        lines.append("")
        lines.append("        void assertCurrentEvent() const {")
        lines.append("            if (epochSource && *epochSource != boundEpoch)")
        lines.append(
            f"                throw SKNano::LogicError(\"[{view_name}View] stale event view access\");")
        lines.append("        }")
        lines.append("        Availability availability() const {")
        lines.append("            assertCurrentEvent();")
        lines.append(f"            const bool presence = {presence_expression};")
        any_terms = [f"{member}.available()" for _, member, _, _ in fields]
        if spec["count_is_addon"]:
            any_terms.insert(0, "count && count->valid()")
        lines.append(f"            const bool anyAddon = {' || '.join(any_terms)};")
        lines.append("            if (!presence)")
        lines.append("                return anyAddon ? Availability::Partial : Availability::Missing;")
        lines.append("            if (!count || !count->valid())")
        lines.append("                return Availability::Partial;")
        for _, member, _, required in fields:
            if required:
                lines.append(f"            if (!{member}.available())")
                lines.append("                return Availability::Partial;")
        lines.append("            return Availability::Complete;")
        lines.append("        }")
        lines.append("        bool available() const {")
        lines.append("            return availability() == Availability::Complete;")
        lines.append("        }")
        lines.append("        void requireAvailable() const {")
        lines.append("            const auto state = availability();")
        lines.append("            if (state == Availability::Complete)")
        lines.append("                return;")
        lines.append("            if (state == Availability::Missing)")
        lines.append(
            f"                throw SKNano::ConfigError(\"[{view_name}View] custom collection is unavailable\");")
        lines.append(
            f"            throw SKNano::ConfigError(\"[{view_name}View] custom collection is only partially available\");")
        lines.append("        }")
        lines.append("        std::size_t size() const {")
        lines.append("            requireAvailable();")
        lines.append("            const auto value = count->get();")
        if count_type in signed_types:
            lines.append("            if (value < 0)")
            lines.append(
                f"                throw SKNano::LogicError(\"[{view_name}View] negative collection size\");")
        lines.append("            return static_cast<std::size_t>(value);")
        lines.append("        }")
        lines.append("        bool empty() const { return size() == 0; }")
        lines.append("")
        lines.append("        class value_type {")
        lines.append("        public:")
        lines.append("            value_type() = default;")
        lines.append(
            f"            value_type(const {struct_name} *columns, std::size_t index)")
        lines.append("                : columns_(columns), index_(index) {}")
        lines.append("            std::size_t index() const { return index_; }")
        missing_values = {
            "float": "-999.f", "double": "-999.", "int": "-999",
            "short": "static_cast<short>(-999)",
            "Long64_t": "static_cast<Long64_t>(-999)",
            "unsigned int": "0u", "unsigned short": "0u",
            "unsigned char": "0u", "ULong64_t": "0u", "bool": "false",
        }
        for _, member, ctype, required in fields:
            if required:
                lines.append(
                    f"            {ctype} {member}() const {{ return columns_->{member}[index_]; }}")
            else:
                missing = missing_values[ctype]
                lines.append(
                    f"            {ctype} {member}() const {{ return "
                    f"columns_->{member}.available() ? "
                    f"columns_->{member}[index_] : {missing}; }}")
        lines.append("        private:")
        lines.append(f"            const {struct_name} *columns_ = nullptr;")
        lines.append("            std::size_t index_ = 0;")
        lines.append("        };")
        lines.append("        value_type operator[](std::size_t index) const {")
        lines.append("            const auto countValue = size();")
        lines.append("            if (index >= countValue)")
        lines.append(
            f"                throw SKNano::LogicError(\"[{view_name}View] object index out of range\");")
        lines.append("            return value_type(this, index);")
        lines.append("        }")
        lines.append("        value_type at(std::size_t index) const { return (*this)[index]; }")
        lines.append("")
        lines.append("        class const_iterator {")
        lines.append("        public:")
        lines.append("            const_iterator() = default;")
        lines.append(
            f"            const_iterator(const {struct_name} *columns, std::size_t index)")
        lines.append("                : columns_(columns), index_(index) {}")
        lines.append("            value_type operator*() const { return (*columns_)[index_]; }")
        lines.append("            const_iterator &operator++() { ++index_; return *this; }")
        lines.append("            bool operator==(const const_iterator &other) const {")
        lines.append("                return columns_ == other.columns_ && index_ == other.index_;")
        lines.append("            }")
        lines.append("            bool operator!=(const const_iterator &other) const {")
        lines.append("                return !(*this == other);")
        lines.append("            }")
        lines.append("        private:")
        lines.append(f"            const {struct_name} *columns_ = nullptr;")
        lines.append("            std::size_t index_ = 0;")
        lines.append("        };")
        lines.append("        const_iterator begin() const { return const_iterator(this, 0); }")
        lines.append("        const_iterator end() const { return const_iterator(this, size()); }")
        lines.append("    };")
        lines.append(f"    void Bind({struct_name} &columns) {{")
        lines.append(f"        columns.count = &{count_member};")
        lines.append("        columns.epochSource = &eventEpoch;")
        lines.append("        columns.boundEpoch = eventEpoch;")
        for branch_name, member, _, _ in fields:
            branch_member = cpp_member_name(collection, branch_name)
            lines.append(f"        columns.{member}.bind(&{branch_member});")
        lines.append("        if (columns.availability() == ")
        lines.append(f"            {struct_name}::Availability::Partial)")
        lines.append(
            f"            throw SKNano::ConfigError(\"[{view_name}View] custom collection is only partially available\");")
        lines.append("    }")
        lines.append(f"    {struct_name} GetAll{view_name}Views() {{")
        lines.append(f"        {struct_name} columns;")
        lines.append("        Bind(columns);")
        lines.append("        return columns;")
        lines.append("    }")
        lines.append(f"    bool Has{view_name}Views() {{")
        lines.append(f"        return GetAll{view_name}Views().available();")
        lines.append("    }")
        lines.append("")

    return lines


def write_file(path: Path, lines: Iterable[str]) -> None:
    content = "\n".join(lines).rstrip() + "\n"
    path.write_text(content)
    print(f"[generate_branch_code] wrote {path}")


def main() -> None:
    parser = argparse.ArgumentParser(description="Generate branch C++ includes")
    parser.add_argument("--json", type=Path, help="Path to NanoAODv15.json")
    parser.add_argument(
        "--addon", action="append", type=Path, default=[],
        help="Optional compact custom-branch addon (repeatable)")
    parser.add_argument("--overlay", type=Path,
                        help="Reviewed metadata for generated input views")
    parser.add_argument("--out-dir", type=Path, help="Dir for generated files")
    parser.add_argument("--depfile", type=Path,
                        help="Optional Make/Ninja dependency file")
    args = parser.parse_args()

    base_dir = Path(os.environ.get("SKNANO_DATA", Path.cwd()))
    json_path = args.json or discover_json(base_dir)
    out_dir = args.out_dir or json_path.parent
    out_dir.mkdir(parents=True, exist_ok=True)
    overlay_path, addon_paths = discover_companion_inputs(
        json_path, args.overlay, args.addon)

    with json_path.open() as jf:
        payload = json.load(jf)

    canonical_events = payload.get("Events Content")
    if not isinstance(canonical_events, dict):
        raise ValueError("JSON missing 'Events Content'")

    addons = []
    for addon_path in addon_paths:
        with addon_path.open() as addon_file:
            addons.append(json.load(addon_file))
    events, addon_view_specs = merge_addons(canonical_events, addons)

    decls, regs, resets = build_blocks(events)

    overlay = {}
    if overlay_path:
        with overlay_path.open() as overlay_file:
            overlay = json.load(overlay_file)
    # Standard views stay tied to the canonical NanoAOD schema.  Addon fields
    # are exposed only through their optional row-view APIs below.
    views = build_view_blocks(canonical_events, overlay)
    views.extend(build_addon_view_blocks(addon_view_specs))

    outputs = [
        out_dir / HEADER_NAME,
        out_dir / REGISTER_NAME,
        out_dir / RESET_NAME,
        out_dir / VIEW_NAME,
        out_dir / BUNDLE_NAME,
    ]
    write_file(outputs[0], decls)
    write_file(outputs[1], regs)
    write_file(outputs[2], resets)
    write_file(outputs[3], views)
    # Keep declarations and their generated API in one include generation.
    # The quoted includes resolve relative to this bundle, preventing an old
    # source-tree declaration file from mixing with a new build-tree API.
    write_file(outputs[4], [
        f'#include "{HEADER_NAME}"',
        "public:",
        f'#include "{VIEW_NAME}"',
    ])
    if args.depfile:
        inputs = [json_path]
        if overlay_path:
            inputs.append(overlay_path)
        inputs.extend(addon_paths)
        write_depfile(args.depfile, outputs, inputs)


if __name__ == "__main__":
    main()
