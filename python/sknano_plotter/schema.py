from __future__ import annotations

import re
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any

import yaml

from .catalog import Catalog, CatalogEntry


class SchemaError(ValueError):
    pass


@dataclass(frozen=True)
class TreeRule:
    name: str
    pattern: str
    kinds: tuple[str, ...] = ("TH1", "TH2")

    def match(self, entry: CatalogEntry) -> dict[str, str] | None:
        if entry.kind not in self.kinds:
            return None
        match = re.fullmatch(self.pattern, entry.path)
        return match.groupdict() if match else None


@dataclass
class Recipe:
    name: str
    renderer: str
    source: str
    enabled: bool = False
    select: dict[str, Any] = field(default_factory=dict)
    facets: list[str] = field(default_factory=list)
    series: str | None = None
    roles: dict[str, Any] = field(default_factory=dict)
    normalize: str = "none"
    rebin: int = 1
    flow: str = "drop"
    log: bool = False
    ratio: bool = False
    ratio_range: tuple[float, float] = (0.5, 1.5)
    systematics: dict[str, Any] = field(default_factory=dict)
    style: dict[str, Any] = field(default_factory=dict)
    output: str | None = None


@dataclass
class PlotSchema:
    version: int
    status: str
    metadata: dict[str, Any]
    rules: list[TreeRule]
    recipes: list[Recipe]
    styles: dict[str, Any] = field(default_factory=dict)


@dataclass(frozen=True)
class MappedEntry:
    entry: CatalogEntry
    rule: str
    dimensions: dict[str, str]


@dataclass
class ValidationReport:
    mapped: list[MappedEntry] = field(default_factory=list)
    uncovered: list[str] = field(default_factory=list)
    duplicates: dict[str, list[str]] = field(default_factory=dict)
    errors: list[str] = field(default_factory=list)
    warnings: list[str] = field(default_factory=list)

    @property
    def ok(self) -> bool:
        return not self.errors and not self.uncovered and not self.duplicates


RENDERERS = {"hist1d", "overlay", "stack", "data_stack", "ratio", "variation", "heatmap"}


def _recipe_from_dict(raw: dict[str, Any]) -> Recipe:
    ratio_range = raw.get("ratio_range", [0.5, 1.5])
    return Recipe(
        name=str(raw["name"]),
        renderer=str(raw["renderer"]),
        source=str(raw["source"]),
        enabled=bool(raw.get("enabled", False)),
        select=dict(raw.get("select", {})),
        facets=list(raw.get("facets", [])),
        series=raw.get("series"),
        roles=dict(raw.get("roles", {})),
        normalize=str(raw.get("normalize", "none")),
        rebin=int(raw.get("rebin", 1)),
        flow=str(raw.get("flow", "drop")),
        log=bool(raw.get("log", False)),
        ratio=bool(raw.get("ratio", False)),
        ratio_range=(float(ratio_range[0]), float(ratio_range[1])),
        systematics=dict(raw.get("systematics", {})),
        style=dict(raw.get("style", {})),
        output=raw.get("output"),
    )


def load_schema(path: str | Path) -> PlotSchema:
    schema_path = Path(path)
    raw = yaml.safe_load(schema_path.read_text(encoding="utf-8"))
    if not isinstance(raw, dict):
        raise SchemaError("schema root must be a mapping")
    if int(raw.get("version", 0)) != 1:
        raise SchemaError("only schema version 1 is supported")
    tree = raw.get("tree", {})
    raw_rules = tree.get("rules", []) if isinstance(tree, dict) else []
    rules = [
        TreeRule(
            name=str(rule["name"]),
            pattern=str(rule["pattern"]),
            kinds=tuple(rule.get("kinds", ["TH1", "TH2"])),
        )
        for rule in raw_rules
    ]
    recipes = [_recipe_from_dict(recipe) for recipe in raw.get("recipes", [])]
    return PlotSchema(
        version=1,
        status=str(raw.get("status", "draft")),
        metadata=dict(raw.get("metadata", {})),
        rules=rules,
        recipes=recipes,
        styles=dict(raw.get("styles", {})),
    )


def map_catalog(catalog: Catalog, schema: PlotSchema) -> ValidationReport:
    report = ValidationReport()
    for entry in catalog.entries:
        matches: list[MappedEntry] = []
        for rule in schema.rules:
            dimensions = rule.match(entry)
            if dimensions is not None:
                matches.append(MappedEntry(entry, rule.name, dimensions))
        if not matches:
            report.uncovered.append(entry.path)
        elif len(matches) > 1:
            report.duplicates[entry.path] = [match.rule for match in matches]
        else:
            report.mapped.append(matches[0])
    return report


def validate_schema(catalog: Catalog, schema: PlotSchema, require_confirmed: bool = False) -> ValidationReport:
    report = map_catalog(catalog, schema)
    if require_confirmed and schema.status != "confirmed":
        report.errors.append("schema status must be 'confirmed' before plotting")
    rule_names = [rule.name for rule in schema.rules]
    if len(rule_names) != len(set(rule_names)):
        report.errors.append("tree rule names must be unique")
    recipe_names = [recipe.name for recipe in schema.recipes]
    if len(recipe_names) != len(set(recipe_names)):
        report.errors.append("recipe names must be unique")
    for rule in schema.rules:
        try:
            compiled = re.compile(rule.pattern)
        except re.error as error:
            report.errors.append(f"rule {rule.name}: invalid regex: {error}")
            continue
        if not compiled.groupindex:
            report.warnings.append(f"rule {rule.name}: pattern has no named dimensions")
        invalid_kinds = set(rule.kinds) - {"TH1", "TH2"}
        if invalid_kinds:
            report.errors.append(f"rule {rule.name}: invalid kinds {sorted(invalid_kinds)}")
    available_by_rule: dict[str, set[str]] = {}
    for mapped in report.mapped:
        available_by_rule.setdefault(mapped.rule, set()).update(mapped.dimensions)
    for recipe in schema.recipes:
        if recipe.renderer not in RENDERERS:
            report.errors.append(f"recipe {recipe.name}: unknown renderer {recipe.renderer}")
        if recipe.source not in rule_names:
            report.errors.append(f"recipe {recipe.name}: unknown source rule {recipe.source}")
            continue
        dimensions = available_by_rule.get(recipe.source, set())
        referenced = set(recipe.facets) | set(recipe.select)
        if recipe.series:
            referenced.add(recipe.series)
        missing = referenced - dimensions
        if missing and available_by_rule.get(recipe.source):
            report.errors.append(f"recipe {recipe.name}: unknown dimensions {sorted(missing)}")
        if recipe.renderer in {"overlay", "stack", "data_stack", "ratio", "variation"} and not recipe.series:
            report.errors.append(f"recipe {recipe.name}: renderer requires 'series'")
        if recipe.renderer == "ratio" and "reference" not in recipe.roles:
            report.errors.append(f"recipe {recipe.name}: ratio renderer requires roles.reference")
        if recipe.renderer == "heatmap":
            wrong = [m.entry.path for m in report.mapped if m.rule == recipe.source and m.entry.ndim != 2]
            if wrong:
                report.warnings.append(f"recipe {recipe.name}: non-TH2 entries will be skipped")
        if recipe.normalize not in {"none", "unit"}:
            report.errors.append(f"recipe {recipe.name}: normalize must be none or unit")
        if recipe.flow not in {"drop", "fold"}:
            report.errors.append(f"recipe {recipe.name}: flow must be drop or fold")
        if recipe.rebin < 1:
            report.errors.append(f"recipe {recipe.name}: rebin must be positive")
        if recipe.renderer == "heatmap" and recipe.rebin != 1:
            report.errors.append(f"recipe {recipe.name}: TH2 rebinning is not supported")
    if report.uncovered:
        report.errors.append(f"{len(report.uncovered)} supported histogram(s) are not covered by any rule")
    if report.duplicates:
        report.errors.append(f"{len(report.duplicates)} supported histogram(s) match multiple rules")
    return report
