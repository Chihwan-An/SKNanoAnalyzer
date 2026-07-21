from __future__ import annotations

import os
import re
from pathlib import Path
from typing import Any

import numpy as np

from .histogram import (
    Histogram1D,
    ensure_compatible,
    normalize1d,
    read_hist1d,
    read_hist2d,
    rebin1d,
    total_uncertainty,
)
from .tasks import role_matches


PALETTE = [
    "#0072B2",
    "#E69F00",
    "#009E73",
    "#CC79A7",
    "#56B4E9",
    "#D55E00",
    "#F0E442",
    "#999999",
    "#332288",
    "#88CCEE",
    "#44AA99",
    "#117733",
    "#DDCC77",
    "#AA4499",
]


def _plot_imports():
    mpl_cache = os.environ.setdefault("MPLCONFIGDIR", f"/tmp/sknano-matplotlib-{os.getuid()}")
    xdg_cache = os.environ.setdefault("XDG_CACHE_HOME", f"/tmp/sknano-cache-{os.getuid()}")
    Path(mpl_cache).mkdir(parents=True, exist_ok=True)
    (Path(xdg_cache) / "fontconfig").mkdir(parents=True, exist_ok=True)
    try:
        import matplotlib

        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
        import mplhep as hep
    except ModuleNotFoundError as error:
        raise RuntimeError(
            "plotting requires matplotlib and mplhep; activate the SKNano environment or install plotting dependencies"
        ) from error
    hep.style.use("CMS")
    plt.rcParams.update(
        {
            "font.size": 13,
            "axes.labelsize": 16,
            "axes.titlesize": 16,
            "xtick.labelsize": 13,
            "ytick.labelsize": 13,
            "legend.fontsize": 11,
        }
    )
    return plt, hep


def _safe(value: str) -> str:
    cleaned = re.sub(r"[^A-Za-z0-9_.-]+", "_", value).strip("_.")
    return cleaned or "plot"


def _task_destination(task: dict[str, Any], output: Path) -> Path:
    if task.get("output"):
        try:
            relative = str(task["output"]).format(**task.get("facets", {}))
        except KeyError as error:
            raise ValueError(f"output template references missing facet {error}") from error
        return output / relative
    parts = [_safe(task["recipe"])]
    parts.extend(f"{_safe(key)}-{_safe(str(value))}" for key, value in sorted(task.get("facets", {}).items()))
    if len(task.get("members", [])) == 1:
        parts.append(_safe(task["members"][0]["path"]))
    return output / "/".join(parts)


def _series_style(name: str, styles: dict[str, Any], index: int) -> dict[str, Any]:
    configured = dict(styles.get("series", {}).get(name, {}))
    configured.setdefault("label", name)
    configured.setdefault("color", PALETTE[index % len(PALETTE)])
    configured.setdefault("order", index)
    configured.setdefault("scale", 1.0)
    return configured


def _apply_scale(hist: Histogram1D, scale: float) -> Histogram1D:
    return Histogram1D(hist.values * scale, hist.variances * scale**2, hist.edges, hist.labels)


def _load_hist1d(root_file: Any, member: dict[str, Any], task: dict[str, Any], styles: dict[str, Any]) -> Histogram1D:
    histogram = read_hist1d(root_file[member["path"]], flow=task.get("flow", "drop"))
    histogram = rebin1d(histogram, int(task.get("rebin", 1)))
    series_dimension = task.get("series")
    if series_dimension:
        series = member["dimensions"].get(series_dimension, "")
        scale = float(_series_style(series, styles, 0).get("scale", 1.0))
        histogram = _apply_scale(histogram, scale)
    if task.get("normalize") == "unit":
        histogram = normalize1d(histogram)
    return histogram


def _axis_title(member: dict[str, Any], axis: int, fallback: str) -> str:
    axes = member.get("axes", [])
    if axis < len(axes) and axes[axis].get("title"):
        return axes[axis]["title"]
    return fallback


def _decorate(ax: Any, metadata: dict[str, Any], hep: Any) -> None:
    text = str(metadata.get("cms_label", "Preliminary"))
    kwargs: dict[str, Any] = {"ax": ax, "loc": 0}
    if metadata.get("lumi") is not None:
        kwargs["lumi"] = float(metadata["lumi"])
    if metadata.get("energy") is not None:
        kwargs["com"] = float(metadata["energy"])
    kwargs["data"] = bool(metadata.get("data", False))
    hep.cms.label(text, fontsize=16, **kwargs)
    if metadata.get("plot_label"):
        ax.text(0.02, 0.94, str(metadata["plot_label"]), transform=ax.transAxes, ha="left", va="top", fontsize=11)


def _categorical_axis(ax: Any, hist: Histogram1D) -> None:
    if not hist.labels:
        return
    centers = (hist.edges[:-1] + hist.edges[1:]) / 2
    ax.set_xticks(centers)
    rotation = 45 if max(map(len, hist.labels), default=0) > 8 or len(hist.labels) > 6 else 0
    ax.set_xticklabels(hist.labels, rotation=rotation, ha="right" if rotation else "center")


def _apply_axis_style(ax: Any, style: dict[str, Any], include_y: bool = True) -> None:
    if style.get("x_label"):
        ax.set_xlabel(str(style["x_label"]))
    if include_y and style.get("y_label"):
        ax.set_ylabel(str(style["y_label"]))
    if style.get("x_range"):
        ax.set_xlim(float(style["x_range"][0]), float(style["x_range"][1]))
    if include_y and style.get("y_range"):
        ax.set_ylim(float(style["y_range"][0]), float(style["y_range"][1]))


def _set_y_scale(ax: Any, values: list[np.ndarray], log: bool) -> None:
    finite = np.concatenate([value[np.isfinite(value)] for value in values if value.size]) if values else np.array([])
    if not finite.size:
        return
    ymax = max(float(np.max(finite)), 0.0)
    if log:
        positive = finite[finite > 0]
        if not positive.size:
            raise ValueError("log scale requested but no positive bins exist")
        ymin = max(float(np.min(positive)) * 0.35, 1e-6)
        ax.set_yscale("log")
        ax.set_ylim(ymin, max(ymax * 30.0, ymin * 10.0))
    else:
        ymin = min(0.0, float(np.min(finite)) * 1.15)
        ax.set_ylim(ymin, ymax * 1.45 if ymax > 0 else 1.0)
        if ymax >= 1.0e4:
            from matplotlib.ticker import EngFormatter

            ax.yaxis.set_major_formatter(EngFormatter(sep=" "))


def _resolved_style(task: dict[str, Any], styles: dict[str, Any]) -> dict[str, Any]:
    observable = str(task.get("facets", {}).get("observable", ""))
    resolved = dict(styles.get("observables", {}).get(observable, {}))
    resolved.update(task.get("style", {}))
    return resolved


def _stairs_band(ax: Any, center: np.ndarray, low: np.ndarray, high: np.ndarray, edges: np.ndarray, label: str) -> None:
    ax.stairs(
        center + high,
        baseline=center - low,
        edges=edges,
        fill=True,
        facecolor="none",
        edgecolor="#333333",
        hatch="////",
        linewidth=0.0,
        alpha=0.5,
        label=label,
    )


def _render_single(root_file: Any, task: dict[str, Any], styles: dict[str, Any], metadata: dict[str, Any]):
    plt, hep = _plot_imports()
    member = task["members"][0]
    hist = _load_hist1d(root_file, member, task, styles)
    fig, ax = plt.subplots(figsize=(8, 7))
    ax.stairs(hist.values, hist.edges, fill=False, linewidth=1.8, color=PALETTE[0])
    errors = np.sqrt(np.clip(hist.variances, 0, None))
    centers = (hist.edges[:-1] + hist.edges[1:]) / 2
    ax.errorbar(centers, hist.values, yerr=errors, fmt="none", color=PALETTE[0], linewidth=1)
    ax.set_xlabel(_axis_title(member, 0, member["dimensions"].get("observable", member["path"])))
    ax.set_ylabel("Normalized entries" if task.get("normalize") == "unit" else "Entries")
    _categorical_axis(ax, hist)
    _set_y_scale(ax, [hist.values + errors], bool(task.get("log")))
    _apply_axis_style(ax, task.get("style", {}))
    _decorate(ax, metadata, hep)
    return fig, {"yields": {member["path"]: float(np.sum(hist.values))}, "warnings": []}


def _render_overlay(root_file: Any, task: dict[str, Any], styles: dict[str, Any], metadata: dict[str, Any]):
    plt, hep = _plot_imports()
    loaded: list[tuple[str, Histogram1D, dict[str, Any]]] = []
    series_dimension = task["series"]
    for index, member in enumerate(task["members"]):
        if member["ndim"] != 1:
            continue
        series = member["dimensions"].get(series_dimension, member["path"])
        loaded.append((series, _load_hist1d(root_file, member, task, styles), member))
    ensure_compatible([hist for _, hist, _ in loaded])
    loaded.sort(key=lambda item: _series_style(item[0], styles, 0)["order"])
    fig, ax = plt.subplots(figsize=(8, 7))
    yields: dict[str, float] = {}
    for index, (series, hist, _) in enumerate(loaded):
        style = _series_style(series, styles, index)
        ax.stairs(hist.values, hist.edges, label=style["label"], color=style["color"], linewidth=1.8)
        yields[series] = float(np.sum(hist.values))
    first = loaded[0][1]
    member = loaded[0][2]
    ax.set_xlabel(_axis_title(member, 0, task.get("facets", {}).get("observable", "Observable")))
    ax.set_ylabel("Normalized entries" if task.get("normalize") == "unit" else "Entries")
    _categorical_axis(ax, first)
    _set_y_scale(ax, [hist.values for _, hist, _ in loaded], bool(task.get("log")))
    _apply_axis_style(ax, task.get("style", {}))
    ax.legend(
        frameon=False,
        fontsize=11,
        ncol=2 if len(loaded) > 7 else 1,
        loc=task.get("style", {}).get("legend_loc", "best"),
    )
    _decorate(ax, metadata, hep)
    return fig, {"yields": yields, "warnings": []}


def _render_ratio(root_file: Any, task: dict[str, Any], styles: dict[str, Any], metadata: dict[str, Any]):
    plt, hep = _plot_imports()
    series_dimension = task["series"]
    loaded: dict[str, tuple[Histogram1D, dict[str, Any]]] = {}
    for member in task["members"]:
        if member["ndim"] != 1:
            continue
        series = member["dimensions"].get(series_dimension, member["path"])
        if series in loaded:
            raise ValueError(f"multiple histograms for ratio series {series}")
        loaded[series] = (_load_hist1d(root_file, member, task, styles), member)
    ensure_compatible([item[0] for item in loaded.values()])
    references = [name for name in loaded if role_matches(name, task.get("roles", {}), "reference")]
    if len(references) != 1:
        raise ValueError(f"ratio requires exactly one reference series, found {references}")
    reference_name = references[0]
    reference, reference_member = loaded[reference_name]
    valid_reference = reference.values != 0
    reference_error = np.sqrt(np.clip(reference.variances, 0, None))
    relative_reference_error = np.divide(
        reference_error, np.abs(reference.values), out=np.zeros_like(reference.values), where=valid_reference
    )

    fig, ax = plt.subplots(figsize=(8, 5.5))
    _stairs_band(
        ax,
        np.ones_like(reference.values),
        relative_reference_error,
        relative_reference_error,
        reference.edges,
        f"{_series_style(reference_name, styles, 0)['label']} unc.",
    )
    yields: dict[str, float] = {reference_name: float(np.sum(reference.values))}
    index = 0
    for name, (hist, _) in sorted(loaded.items()):
        if name == reference_name:
            continue
        ratio = np.divide(hist.values, reference.values, out=np.full_like(hist.values, np.nan), where=valid_reference)
        numerator_error = np.sqrt(np.clip(hist.variances, 0, None))
        ratio_error = np.divide(numerator_error, np.abs(reference.values), out=np.zeros_like(hist.values), where=valid_reference)
        centers = (hist.edges[:-1] + hist.edges[1:]) / 2
        style = _series_style(name, styles, index + 1)
        valid = np.isfinite(ratio)
        ax.errorbar(
            centers[valid], ratio[valid], yerr=ratio_error[valid], fmt="o-",
            color=style["color"], markersize=3.5, linewidth=1.2, label=style["label"],
        )
        yields[name] = float(np.sum(hist.values))
        index += 1
    ax.axhline(1.0, color="#555555", linestyle="--", linewidth=1)
    ax.set_ylim(*task.get("ratio_range", [0.5, 1.5]))
    ax.set_xlabel(_axis_title(reference_member, 0, task.get("facets", {}).get("observable", "Observable")))
    ax.set_ylabel("Ratio to reference")
    _categorical_axis(ax, reference)
    _apply_axis_style(ax, task.get("style", {}))
    ax.legend(frameon=False, fontsize=11, loc=task.get("style", {}).get("legend_loc", "best"))
    _decorate(ax, metadata, hep)
    return fig, {"yields": yields, "warnings": []}


def _nominal_members(task: dict[str, Any]) -> tuple[list[dict[str, Any]], dict[str, list[dict[str, Any]]]]:
    systematics = task.get("systematics", {})
    dimension = systematics.get("dimension")
    nominal = str(systematics.get("nominal", "Central"))
    if not dimension:
        return list(task["members"]), {}
    central: list[dict[str, Any]] = []
    variations: dict[str, list[dict[str, Any]]] = {}
    for member in task["members"]:
        value = member["dimensions"].get(dimension)
        if value == nominal:
            central.append(member)
        elif value is not None:
            variations.setdefault(value, []).append(member)
    return central, variations


def _render_data_stack(root_file: Any, task: dict[str, Any], styles: dict[str, Any], metadata: dict[str, Any]):
    plt, hep = _plot_imports()
    central_members, variation_members = _nominal_members(task)
    series_dimension = task["series"]
    by_series: dict[str, Histogram1D] = {}
    member_by_series: dict[str, dict[str, Any]] = {}
    for member in central_members:
        series = member["dimensions"].get(series_dimension, member["path"])
        if series in by_series:
            raise ValueError(f"multiple nominal histograms for series {series}")
        by_series[series] = _load_hist1d(root_file, member, task, styles)
        member_by_series[series] = member
    ensure_compatible(list(by_series.values()))

    data_names = [name for name in by_series if role_matches(name, task.get("roles", {}), "data")]
    signal_names = [name for name in by_series if role_matches(name, task.get("roles", {}), "signal")]
    explicit_stack = "stack" in task.get("roles", {})
    stack_names = [
        name
        for name in by_series
        if (role_matches(name, task.get("roles", {}), "stack") if explicit_stack else name not in data_names + signal_names)
    ]
    if len(data_names) > 1:
        raise ValueError(f"data_stack expects at most one data series, found {data_names}")
    if not stack_names:
        raise ValueError("data_stack has no stacked series")
    stack_names.sort(key=lambda name: _series_style(name, styles, 0)["order"])
    stack_hists = [by_series[name] for name in stack_names]
    edges = stack_hists[0].edges
    total = np.sum([hist.values for hist in stack_hists], axis=0)

    variation_by_series: dict[str, dict[str, Histogram1D]] = {name: {} for name in stack_names}
    for variation, members in variation_members.items():
        for member in members:
            series = member["dimensions"].get(series_dimension, member["path"])
            if series in variation_by_series:
                variation_by_series[series][variation] = _load_hist1d(root_file, member, task, styles)
    syst = task.get("systematics", {})
    low, high, warnings = total_uncertainty(
        {name: by_series[name] for name in stack_names},
        variation_by_series,
        up_suffix=str(syst.get("up_suffix", "_Up")),
        down_suffix=str(syst.get("down_suffix", "_Down")),
        missing=str(syst.get("missing", "error")),
    )

    has_ratio = bool(task.get("ratio")) and bool(data_names)
    if has_ratio:
        fig, (ax, rax) = plt.subplots(
            2, 1, figsize=(8, 8), sharex=True,
            gridspec_kw={"height_ratios": [3.4, 1], "hspace": 0.06},
        )
    else:
        fig, ax = plt.subplots(figsize=(8, 7))
        rax = None

    baseline = np.zeros_like(total)
    for index, name in enumerate(stack_names):
        hist = by_series[name]
        style = _series_style(name, styles, index)
        ax.stairs(
            baseline + hist.values,
            baseline=baseline,
            edges=edges,
            fill=True,
            color=style["color"],
            label=style["label"],
            linewidth=0.6,
        )
        baseline = baseline + hist.values
    _stairs_band(ax, total, low, high, edges, r"Stat. $\oplus$ syst. unc.")

    for index, name in enumerate(signal_names):
        hist = by_series[name]
        style = _series_style(name, styles, len(stack_names) + index)
        ax.stairs(hist.values, edges, color=style["color"], linewidth=2, label=style["label"])

    data_hist = by_series[data_names[0]] if data_names else None
    if data_hist is not None:
        centers = (edges[:-1] + edges[1:]) / 2
        errors = np.sqrt(np.clip(data_hist.variances, 0, None))
        ax.errorbar(centers, data_hist.values, yerr=errors, fmt="o", color="black", markersize=4, label="Data")
    first_member = member_by_series[stack_names[0]]
    ax.set_ylabel("Events")
    _set_y_scale(ax, [total + high] + ([data_hist.values] if data_hist is not None else []), bool(task.get("log")))
    _apply_axis_style(ax, task.get("style", {}))
    ax.legend(
        frameon=False,
        fontsize=11,
        ncol=2 if len(stack_names) + len(signal_names) > 7 else 1,
        loc=task.get("style", {}).get("legend_loc", "best"),
    )
    metadata = dict(metadata)
    metadata["data"] = data_hist is not None
    _decorate(ax, metadata, hep)

    if rax is not None and data_hist is not None:
        valid = total > 0
        ratio = np.divide(data_hist.values, total, out=np.full_like(total, np.nan), where=valid)
        data_error = np.sqrt(np.clip(data_hist.variances, 0, None))
        ratio_error = np.divide(data_error, total, out=np.zeros_like(total), where=valid)
        centers = (edges[:-1] + edges[1:]) / 2
        rax.errorbar(centers[valid], ratio[valid], yerr=ratio_error[valid], fmt="o", color="black", markersize=3.5)
        rel_low = np.divide(low, total, out=np.zeros_like(low), where=valid)
        rel_high = np.divide(high, total, out=np.zeros_like(high), where=valid)
        _stairs_band(rax, np.ones_like(total), rel_low, rel_high, edges, "")
        rax.axhline(1.0, color="#555555", linestyle="--", linewidth=1)
        rax.set_ylim(*task.get("ratio_range", [0.5, 1.5]))
        rax.set_ylabel("Data / Pred.")
        rax.set_xlabel(_axis_title(first_member, 0, task.get("facets", {}).get("observable", "Observable")))
        _categorical_axis(rax, stack_hists[0])
        if task.get("style", {}).get("x_label"):
            rax.set_xlabel(str(task["style"]["x_label"]))
    else:
        ax.set_xlabel(_axis_title(first_member, 0, task.get("facets", {}).get("observable", "Observable")))
        _categorical_axis(ax, stack_hists[0])

    yields = {name: float(np.sum(hist.values)) for name, hist in by_series.items()}
    return fig, {"yields": yields, "warnings": sorted(set(warnings))}


def _render_heatmap(root_file: Any, task: dict[str, Any], metadata: dict[str, Any]):
    plt, hep = _plot_imports()
    member = task["members"][0]
    hist = read_hist2d(root_file[member["path"]], flow=task.get("flow", "drop"))
    fig, ax = plt.subplots(figsize=(8, 7))
    mesh = ax.pcolormesh(hist.xedges, hist.yedges, hist.values.T, cmap=task.get("style", {}).get("cmap", "viridis"))
    fig.colorbar(mesh, ax=ax, label=task.get("style", {}).get("colorbar", "Entries"))
    ax.set_xlabel(_axis_title(member, 0, "x"))
    ax.set_ylabel(_axis_title(member, 1, "y"))
    if hist.xlabels:
        ax.set_xticks((hist.xedges[:-1] + hist.xedges[1:]) / 2, hist.xlabels, rotation=45, ha="right")
    if hist.ylabels:
        ax.set_yticks((hist.yedges[:-1] + hist.yedges[1:]) / 2, hist.ylabels)
    if task.get("style", {}).get("annotate", False) and hist.values.size <= 400:
        for ix in range(hist.values.shape[0]):
            for iy in range(hist.values.shape[1]):
                ax.text((hist.xedges[ix] + hist.xedges[ix + 1]) / 2, (hist.yedges[iy] + hist.yedges[iy + 1]) / 2,
                        f"{hist.values[ix, iy]:g}", ha="center", va="center", fontsize=7)
    _apply_axis_style(ax, task.get("style", {}))
    _decorate(ax, metadata, hep)
    return fig, {"yields": {member["path"]: float(np.sum(hist.values))}, "warnings": []}


def render_task(
    input_path: str,
    task: dict[str, Any],
    output: str | Path,
    formats: list[str],
    metadata: dict[str, Any],
    styles: dict[str, Any],
) -> dict[str, Any]:
    try:
        import uproot
    except ModuleNotFoundError as error:
        raise RuntimeError("uproot is required to render ROOT histograms") from error
    output_path = Path(output)
    task = dict(task)
    task["style"] = _resolved_style(task, styles)
    destination = _task_destination(task, output_path)
    destination.parent.mkdir(parents=True, exist_ok=True)
    with uproot.open(input_path) as root_file:
        renderer = task["renderer"]
        if renderer == "hist1d":
            fig, details = _render_single(root_file, task, styles, metadata)
        elif renderer in {"overlay", "variation"}:
            fig, details = _render_overlay(root_file, task, styles, metadata)
        elif renderer == "ratio":
            fig, details = _render_ratio(root_file, task, styles, metadata)
        elif renderer in {"stack", "data_stack"}:
            fig, details = _render_data_stack(root_file, task, styles, metadata)
        elif renderer == "heatmap":
            fig, details = _render_heatmap(root_file, task, metadata)
        else:
            raise ValueError(f"unsupported renderer {renderer}")
    files: dict[str, str] = {}
    for extension in formats:
        extension = extension.lower()
        if extension not in {"png", "pdf"}:
            raise ValueError(f"unsupported output format {extension}")
        path = destination.with_suffix(f".{extension}")
        fig.savefig(path, dpi=180 if extension == "png" else None, bbox_inches="tight")
        files[extension] = str(path)
    plt, _ = _plot_imports()
    plt.close(fig)
    return {
        "task_id": task["task_id"],
        "recipe": task["recipe"],
        "facets": task.get("facets", {}),
        "title": " / ".join(str(value) for value in task.get("facets", {}).values()) or task["recipe"],
        "files": files,
        **details,
    }
