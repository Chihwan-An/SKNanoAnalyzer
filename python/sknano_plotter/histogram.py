from __future__ import annotations

from dataclasses import dataclass
from typing import Any

import numpy as np


@dataclass
class Histogram1D:
    values: np.ndarray
    variances: np.ndarray
    edges: np.ndarray
    labels: list[str] | None = None


@dataclass
class Histogram2D:
    values: np.ndarray
    variances: np.ndarray
    xedges: np.ndarray
    yedges: np.ndarray
    xlabels: list[str] | None = None
    ylabels: list[str] | None = None


def _variances(obj: Any, flow: bool) -> np.ndarray:
    variances = obj.variances(flow=flow)
    if variances is None:
        values = obj.values(flow=flow)
        return np.clip(np.asarray(values, dtype=float), 0.0, None)
    return np.asarray(variances, dtype=float)


def read_hist1d(obj: Any, flow: str = "drop") -> Histogram1D:
    values, edges = obj.to_numpy(flow=False)
    values = np.asarray(values, dtype=float)
    variances = _variances(obj, flow=False)
    if flow == "fold":
        flow_values = np.asarray(obj.values(flow=True), dtype=float)
        flow_vars = _variances(obj, flow=True)
        values = values.copy()
        variances = variances.copy()
        values[0] += flow_values[0]
        values[-1] += flow_values[-1]
        variances[0] += flow_vars[0]
        variances[-1] += flow_vars[-1]
    labels = obj.axes[0].labels()
    return Histogram1D(values, variances, np.asarray(edges, dtype=float), list(labels) if labels else None)


def read_hist2d(obj: Any, flow: str = "drop") -> Histogram2D:
    values, xedges, yedges = obj.to_numpy(flow=False)
    values = np.asarray(values, dtype=float)
    variances = _variances(obj, flow=False)
    if flow == "fold":
        full_values = np.asarray(obj.values(flow=True), dtype=float)
        full_vars = _variances(obj, flow=True)
        values = values.copy()
        variances = variances.copy()
        values[0, :] += full_values[0, 1:-1]
        values[-1, :] += full_values[-1, 1:-1]
        values[:, 0] += full_values[1:-1, 0]
        values[:, -1] += full_values[1:-1, -1]
        variances[0, :] += full_vars[0, 1:-1]
        variances[-1, :] += full_vars[-1, 1:-1]
        variances[:, 0] += full_vars[1:-1, 0]
        variances[:, -1] += full_vars[1:-1, -1]
    xlabels = obj.axes[0].labels()
    ylabels = obj.axes[1].labels()
    return Histogram2D(
        values,
        variances,
        np.asarray(xedges, dtype=float),
        np.asarray(yedges, dtype=float),
        list(xlabels) if xlabels else None,
        list(ylabels) if ylabels else None,
    )


def rebin1d(hist: Histogram1D, factor: int) -> Histogram1D:
    if factor == 1:
        return hist
    bins = len(hist.values)
    if bins % factor:
        raise ValueError(f"cannot rebin {bins} bins by {factor} without discarding bins")
    values = hist.values.reshape(-1, factor).sum(axis=1)
    variances = hist.variances.reshape(-1, factor).sum(axis=1)
    edges = hist.edges[::factor]
    if len(edges) != len(values) + 1:
        edges = np.append(edges, hist.edges[-1])
    labels = None
    if hist.labels:
        labels = [" / ".join(hist.labels[i : i + factor]) for i in range(0, bins, factor)]
    return Histogram1D(values, variances, edges, labels)


def normalize1d(hist: Histogram1D) -> Histogram1D:
    integral = float(np.sum(hist.values))
    if not np.isfinite(integral) or integral <= 0:
        raise ValueError("unit normalization requires a positive finite integral")
    return Histogram1D(hist.values / integral, hist.variances / integral**2, hist.edges, hist.labels)


def ensure_compatible(histograms: list[Histogram1D]) -> None:
    if not histograms:
        raise ValueError("plot task has no histograms")
    reference = histograms[0].edges
    for histogram in histograms[1:]:
        if reference.shape != histogram.edges.shape or not np.allclose(reference, histogram.edges, rtol=0, atol=1e-12):
            raise ValueError("histogram bin edges are incompatible")


def total_uncertainty(
    central_by_series: dict[str, Histogram1D],
    variation_by_series: dict[str, dict[str, Histogram1D]],
    up_suffix: str = "_Up",
    down_suffix: str = "_Down",
    missing: str = "error",
) -> tuple[np.ndarray, np.ndarray, list[str]]:
    names = sorted(central_by_series)
    ensure_compatible(list(central_by_series.values()))
    central = np.sum([central_by_series[name].values for name in names], axis=0)
    stat_var = np.sum([central_by_series[name].variances for name in names], axis=0)
    nuisances: set[str] = set()
    for variations in variation_by_series.values():
        for variation in variations:
            if variation.endswith(up_suffix):
                nuisances.add(variation[: -len(up_suffix)])
            elif variation.endswith(down_suffix):
                nuisances.add(variation[: -len(down_suffix)])

    up_var = stat_var.copy()
    down_var = stat_var.copy()
    warnings: list[str] = []
    for nuisance in sorted(nuisances):
        totals: dict[str, np.ndarray] = {}
        for direction, suffix in (("up", up_suffix), ("down", down_suffix)):
            pieces: list[np.ndarray] = []
            for name in names:
                variation = variation_by_series.get(name, {}).get(nuisance + suffix)
                if variation is None:
                    if missing == "central":
                        warnings.append(f"{name}/{nuisance + suffix}: using nominal fallback")
                        variation = central_by_series[name]
                    else:
                        raise ValueError(f"missing variation {nuisance + suffix} for series {name}")
                ensure_compatible([central_by_series[name], variation])
                pieces.append(variation.values)
            totals[direction] = np.sum(pieces, axis=0)
        up_delta = np.maximum.reduce([totals["up"] - central, totals["down"] - central, np.zeros_like(central)])
        down_delta = np.maximum.reduce([central - totals["up"], central - totals["down"], np.zeros_like(central)])
        up_var += up_delta**2
        down_var += down_delta**2
    return np.sqrt(down_var), np.sqrt(up_var), warnings
