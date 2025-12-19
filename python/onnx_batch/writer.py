from __future__ import annotations

import time
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, Iterable, List, Mapping, Sequence

import awkward as ak
import numpy as np
import uproot
from contextlib import nullcontext
from rich.console import Console
from rich.progress import (
    BarColumn,
    Progress,
    SpinnerColumn,
    TaskID,
    TaskProgressColumn,
    TextColumn,
    TimeElapsedColumn,
    TimeRemainingColumn,
)
import traceback

from .config import ONNX_OUTPUT_BRANCH_TYPES, RuntimeConfig
from .data_prep import build_spanet_inputs, detect_channel
from .onnx_runner import OnnxRunner


@dataclass
class TreeStats:
    name: str
    entries: int
    inferred: int
    valid: int
    duration: float


def _reshape_hw_logits(arr: np.ndarray, max_jets: int) -> np.ndarray:
    """
    Normalize hw logits to shape (batch, max_jets, max_jets).
    Accepts shapes like (batch,1,max_jets,max_jets), (batch,max_jets,max_jets),
    (batch,max_jets*max_jets), or flat (max_jets*max_jets) for single entry.
    """
    logits = np.asarray(arr)
    if logits.ndim == 4 and logits.shape[1] == 1:
        logits = logits[:, 0, :, :]
    elif logits.ndim == 2 and logits.shape[0] == 1 and logits.shape[1] == 1:
        logits = logits.reshape(1, max_jets, max_jets)
    if logits.ndim == 3:
        return logits
    if logits.ndim == 2 and logits.shape[1] == max_jets * max_jets:
        return logits.reshape(logits.shape[0], max_jets, max_jets)
    if logits.ndim == 1 and logits.shape[0] == max_jets * max_jets:
        return logits.reshape(1, max_jets, max_jets)
    raise RuntimeError(
        f"Unexpected hw_45_assignment_log_probability shape {logits.shape}"
    )


def _ensure_rows(arr: np.ndarray, target_rows: int, fill_value: float = np.nan) -> np.ndarray:
    """
    Ensure a 2D array has target_rows by trimming or padding with fill_value.
    """
    arr = np.asarray(arr)
    if arr.ndim == 1:
        arr = arr.reshape(1, -1)
    n_rows, n_cols = arr.shape
    if n_rows == target_rows:
        return arr
    if n_rows > target_rows:
        return arr[:target_rows]
    pad = np.full((target_rows - n_rows, n_cols), fill_value, dtype=arr.dtype)
    return np.concatenate([arr, pad], axis=0)


def _flatten_detection(arr: np.ndarray) -> np.ndarray:
    detection = np.asarray(arr)
    if detection.ndim > 1:
        detection = np.squeeze(detection)
    return detection.astype(np.float32)


def _extract_class_scores(arr: np.ndarray) -> np.ndarray:
    """
    Return scores as (batch, n_class). Accepts (batch,1,n), (batch,n),
    (n,), or (1,n).
    """
    scores = np.asarray(arr)
    if scores.ndim == 3 and scores.shape[1] == 1:
        scores = scores[:, 0, :]
    elif scores.ndim == 1:
        scores = scores.reshape(1, -1)
    if scores.ndim != 2:
        raise RuntimeError(f"Unexpected EVENT/signal output shape {scores.shape}")
    return scores.astype(np.float32)


def _pick_assignments(
    logits: np.ndarray, jet_counts: np.ndarray, max_jets: int
) -> tuple[np.ndarray, np.ndarray, np.ndarray, np.ndarray]:
    """
    Mirror the original Vcb_SL InferONNX logic:
    - iterate hw45 logits in descending order
    - pick the first (w1,w2) with c1!=c2 and within J (=min(nJets, max_jets))
    - otherwise leave invalid.
    """
    batch = logits.shape[0]
    w1 = np.full(batch, -1, dtype=np.int32)
    w2 = np.full(batch, -1, dtype=np.int32)
    assignment_logp = np.full(batch, np.nan, dtype=np.float32)
    valid = np.zeros(batch, dtype=bool)

    flat_logits = logits.reshape(batch, max_jets * max_jets)
    order = np.argsort(-flat_logits, axis=1)

    for i in range(batch):
        J = int(min(jet_counts[i], max_jets))
        if J < 2:
            raise RuntimeError("Cannot pick assignments with less than 2 jets")
        for idx in order[i]:
            c1 = (idx // max_jets) % max_jets
            c2 = idx % max_jets
            if c1 >= J or c2 >= J or c1 == c2:
                continue
            w1[i] = int(c1)
            w2[i] = int(c2)
            assignment_logp[i] = flat_logits[i, idx]
            valid[i] = True
            break
        if not valid[i]:
            raise RuntimeError(f"Failed to pick valid assignments for some events: J={J}, logits={logits[i]}")
    return w1, w2, assignment_logp, valid


def _category_bits(cat: np.ndarray) -> tuple[np.ndarray, np.ndarray, np.ndarray, np.ndarray]:
    """
    Return N0, L0, C[5], B[5] arrays for category integers.
    """
    N0 = (cat == 0).astype(np.float32)
    L0 = (cat == 1).astype(np.float32)
    C = [(cat == (2 + i)).astype(np.float32) for i in range(5)]
    B = [(cat == (7 + i)).astype(np.float32) for i in range(5)]
    return N0, L0, C, B


def _compute_template_features(
    jets: Dict[str, np.ndarray],
    jet_counts: np.ndarray,
    w1_idx: np.ndarray,
    w2_idx: np.ndarray,
    valid_mask: np.ndarray,
    max_jets: int,
) -> Dict[str, np.ndarray]:
    n = len(w1_idx)
    m_had_w = np.full(n, np.nan, dtype=np.float32)
    pt_w_u = np.full(n, np.nan, dtype=np.float32)
    pt_w_d = np.full(n, np.nan, dtype=np.float32)
    eta_w_u = np.full(n, np.nan, dtype=np.float32)
    eta_w_d = np.full(n, np.nan, dtype=np.float32)
    cat_w_u = np.full(n, -1, dtype=np.int32)
    cat_w_d = np.full(n, -1, dtype=np.int32)

    N0_w_u = np.zeros(n, dtype=np.float32)
    L0_w_u = np.zeros(n, dtype=np.float32)
    N0_w_d = np.zeros(n, dtype=np.float32)
    L0_w_d = np.zeros(n, dtype=np.float32)
    C_w_u = [np.zeros(n, dtype=np.float32) for _ in range(5)]
    B_w_u = [np.zeros(n, dtype=np.float32) for _ in range(5)]
    C_w_d = [np.zeros(n, dtype=np.float32) for _ in range(5)]
    B_w_d = [np.zeros(n, dtype=np.float32) for _ in range(5)]

    jet_pt = jets["pt"]
    jet_eta = jets["eta"]
    jet_phi = jets["phi"]
    jet_mass = jets["mass"]
    jet_cat = jets["cat"]

    for i in range(n):
        if not valid_mask[i]:
            continue
        J = int(min(jet_counts[i], max_jets))
        w1 = int(w1_idx[i])
        w2 = int(w2_idx[i])
        if w1 < 0 or w2 < 0 or w1 >= J or w2 >= J:
            continue
        pt1 = jet_pt[i, w1]
        pt2 = jet_pt[i, w2]
        eta1 = jet_eta[i, w1]
        eta2 = jet_eta[i, w2]
        phi1 = jet_phi[i, w1]
        phi2 = jet_phi[i, w2]
        m1 = jet_mass[i, w1]
        m2 = jet_mass[i, w2]

        px1 = pt1 * np.cos(phi1)
        py1 = pt1 * np.sin(phi1)
        pz1 = pt1 * np.sinh(eta1)
        e1 = np.sqrt(px1 * px1 + py1 * py1 + pz1 * pz1 + m1 * m1)

        px2 = pt2 * np.cos(phi2)
        py2 = pt2 * np.sin(phi2)
        pz2 = pt2 * np.sinh(eta2)
        e2 = np.sqrt(px2 * px2 + py2 * py2 + pz2 * pz2 + m2 * m2)

        px = px1 + px2
        py = py1 + py2
        pz = pz1 + pz2
        e = e1 + e2
        mass2 = e * e - (px * px + py * py + pz * pz)
        m_had_w[i] = np.sqrt(mass2) if mass2 > 0 else 0.0

        pt_w_u[i] = pt1
        pt_w_d[i] = pt2
        eta_w_u[i] = eta1
        eta_w_d[i] = eta2

        cat_u = int(jet_cat[i, w1])
        cat_d = int(jet_cat[i, w2])
        cat_w_u[i] = cat_u
        cat_w_d[i] = cat_d

        n0, l0, C, B = _category_bits(np.array([cat_u]))
        N0_w_u[i] = n0[0]
        L0_w_u[i] = l0[0]
        for k in range(5):
            C_w_u[k][i] = C[k][0]
            B_w_u[k][i] = B[k][0]
        n0, l0, C, B = _category_bits(np.array([cat_d]))
        N0_w_d[i] = n0[0]
        L0_w_d[i] = l0[0]
        for k in range(5):
            C_w_d[k][i] = C[k][0]
            B_w_d[k][i] = B[k][0]

    return {
        "m_had_w": m_had_w,
        "pt_w_u": pt_w_u,
        "pt_w_d": pt_w_d,
        "eta_w_u": eta_w_u,
        "eta_w_d": eta_w_d,
        "Cat_w_u": cat_w_u,
        "Cat_w_d": cat_w_d,
        "N0_w_u": N0_w_u,
        "L0_w_u": L0_w_u,
        "N0_w_d": N0_w_d,
        "L0_w_d": L0_w_d,
        "C0_w_u": C_w_u[0],
        "C1_w_u": C_w_u[1],
        "C2_w_u": C_w_u[2],
        "C3_w_u": C_w_u[3],
        "C4_w_u": C_w_u[4],
        "B0_w_u": B_w_u[0],
        "B1_w_u": B_w_u[1],
        "B2_w_u": B_w_u[2],
        "B3_w_u": B_w_u[3],
        "B4_w_u": B_w_u[4],
        "C0_w_d": C_w_d[0],
        "C1_w_d": C_w_d[1],
        "C2_w_d": C_w_d[2],
        "C3_w_d": C_w_d[3],
        "C4_w_d": C_w_d[4],
        "B0_w_d": B_w_d[0],
        "B1_w_d": B_w_d[1],
        "B2_w_d": B_w_d[2],
        "B3_w_d": B_w_d[3],
        "B4_w_d": B_w_d[4],
    }


class OnnxBatchProcessor:
    def __init__(
        self,
        runner: OnnxRunner,
        runtime_cfg: RuntimeConfig,
        console: Console,
        max_jets: int,
    ):
        self.runner = runner
        self.runtime_cfg = runtime_cfg
        self.console = console
        self.max_jets = max_jets

    def _select_trees(
        self, infile: uproot.ReadOnlyFile, include: Sequence[str] | None
    ) -> List[str]:
        candidate = []
        seen = set()
        for key in infile.keys(cycle=False):  # avoid iterating backup cycles
            if key in seen:
                continue
            seen.add(key)
            obj = infile[key]  # resolves to the latest cycle if multiple exist
            if not isinstance(obj, uproot.behaviors.TTree.TTree):
                continue
            name = obj.name
            if not (name.startswith("El_") or name.startswith("Mu_")):
                continue
            if include and name not in include:
                continue
            candidate.append(name)
        return candidate

    def _process_batch(
        self,
        batch: Mapping[str, ak.Array],
        channel: str,
    ) -> tuple[Dict[str, object], int, int]:
        inputs, jet_counts, jets = build_spanet_inputs(batch, channel, self.max_jets)
        fold_ids = np.asarray(batch["index_fold_spanet"], dtype=np.int64)
        n_events = fold_ids.shape[0]

        # Basic consistency check to surface length mismatches early
        lengths = {name: len(arr) for name, arr in batch.items()}
        if any(l != n_events for l in lengths.values()):
            raise ValueError(
                f"Branch length mismatch in batch (expected {n_events} events): {lengths}"
            )

        # We only keep new outputs; original branches are not carried over.
        output_arrays: Dict[str, object] = {}

        logp_classes = np.full((n_events, 6), np.nan, dtype=np.float32)
        detection = np.full(n_events, np.nan, dtype=np.float32)
        assignment_logp = np.full(n_events, np.nan, dtype=np.float32)
        w1_idx = np.full(n_events, -1, dtype=np.int32)
        w2_idx = np.full(n_events, -1, dtype=np.int32)
        hb_idx = np.full(n_events, -1, dtype=np.int32)
        lb_idx = np.full(n_events, -1, dtype=np.int32)
        valid = np.zeros(n_events, dtype=bool)

        eligible = (jet_counts >= 3) & inputs["Lepton_mask"].reshape(-1)
        if np.any(~eligible):
            raise RuntimeError("Some events are not eligible for processing")
        
        inferred = int(np.count_nonzero(eligible))

        if inferred > 0:
            unique_folds = np.unique(fold_ids[eligible])
            batch_size = max(self.runtime_cfg.batch_size, 1)
            for fold in unique_folds:
                fold_mask = eligible & (fold_ids == fold)
                indices = np.nonzero(fold_mask)[0]
                for start in range(0, len(indices), batch_size):
                    sub_idx = indices[start : start + batch_size]
                    feeds = {k: v[sub_idx] for k, v in inputs.items()}

                    outputs = self.runner.run_spanet(int(fold), feeds)
                    class_scores_raw = outputs["class"]["EVENT/signal"]
                    hw_logits_raw = outputs["reco"]["hw_45_assignment_log_probability"]
                    detection_raw = outputs["reco"]["hw_45_detection_log_probability"]

                    class_scores = _extract_class_scores(class_scores_raw)
                    class_scores = _ensure_rows(class_scores, len(sub_idx))

                    hw_logits = _reshape_hw_logits(hw_logits_raw, self.max_jets)
                    if hw_logits.shape[0] != len(sub_idx):
                        if hw_logits.shape[0] == 1:
                            hw_logits = np.repeat(hw_logits, len(sub_idx), axis=0)
                        else:
                            hw_logits = hw_logits[: len(sub_idx)]

                    detection_logits = _flatten_detection(detection_raw)

                    w1, w2, assign_lp, valid_mask = _pick_assignments(
                        hw_logits, jet_counts[sub_idx], self.max_jets
                    )

                    logp_classes[sub_idx] = class_scores
                    detection[sub_idx] = detection_logits
                    assignment_logp[sub_idx] = assign_lp
                    w1_idx[sub_idx] = w1
                    w2_idx[sub_idx] = w2
                    valid[sub_idx] = valid_mask

        invalid_mask = eligible & (~valid)
        if invalid_mask.any():
            bad_indices = np.nonzero(invalid_mask)[0]
            preview = []
            for idx in bad_indices[:5]:
                J = int(min(jet_counts[idx], self.max_jets))
                preview.append(
                    {
                        "event_idx": int(idx),
                        "J": J,
                        "fold": int(fold_ids[idx]),
                        "w1": int(w1_idx[idx]),
                        "w2": int(w2_idx[idx]),
                        "logp": float(assignment_logp[idx])
                        if np.isfinite(assignment_logp[idx])
                        else None,
                    }
                )
            raise RuntimeError(
                f"Failed to pick valid assignments for some events "
                f"(count={bad_indices.size}). Examples: {preview}"
            )

        template_feats = _compute_template_features(
            jets, jet_counts, w1_idx, w2_idx, valid, self.max_jets
        )

        output_arrays.update(
            {
                "logp_class_0": logp_classes[:, 0],
                "logp_class_1": logp_classes[:, 1],
                "logp_class_2": logp_classes[:, 2],
                "logp_class_3": logp_classes[:, 3],
                "logp_class_4": logp_classes[:, 4],
                "logp_class_5": logp_classes[:, 5],
                "detection_score_logp": detection,
                "assignment_logp": assignment_logp,
                "assignment_w1_idx": w1_idx,
                "assignment_w2_idx": w2_idx,
                "assignment_hb_idx": hb_idx,
                "assignment_lb_idx": lb_idx,
                "onnx_assignment_valid": valid,
            }
        )
        output_arrays.update(template_feats)
        return output_arrays, inferred, int(np.count_nonzero(valid))

    def process_tree(
        self,
        infile: uproot.ReadOnlyFile,
        outfile: uproot.WritableDirectory,
        tree_name: str,
        progress: Progress | None = None,
        overall_task: TaskID | None = None,
    ) -> TreeStats:
        tree = infile[tree_name]
        channel = detect_channel(tree_name, self.runtime_cfg.channel_hint)
        out_tree = None
        branches = list(tree.keys())
        entry_stop = self.runtime_cfg.max_events

        total_entries = tree.num_entries
        if entry_stop is not None:
            total_entries = min(total_entries, entry_stop)

        start = time.perf_counter()
        inferred_total = 0
        valid_total = 0

        own_progress = progress is None
        if own_progress:
            progress = Progress(
                SpinnerColumn(),
                TextColumn("{task.description}"),
                BarColumn(),
                TaskProgressColumn(),
                TimeElapsedColumn(),
                TimeRemainingColumn(),
                console=self.console,
            )

        progress_cm = progress if own_progress else nullcontext(progress)
        task_ids: list[TaskID] = []

        with progress_cm:
            assert progress is not None  # for type checkers
            load_task = progress.add_task(
                f"[cyan]{tree_name}[/] load", total=total_entries
            )
            convert_task = progress.add_task(
                f"[cyan]{tree_name}[/] to numpy/batch", total=total_entries
            )
            infer_task = progress.add_task(
                f"[cyan]{tree_name}[/] inference", total=total_entries
            )
            write_task = progress.add_task(
                f"[cyan]{tree_name}[/] write", total=total_entries
            )
            task_ids.extend([load_task, convert_task, infer_task, write_task])
            for batch in tree.iterate(
                expressions=branches,
                step_size=self.runtime_cfg.chunk_size,
                library="ak",
                entry_stop=entry_stop,
            ):
                if isinstance(batch, dict):
                    first = next(iter(batch.values()))
                    batch_events = len(first)
                    batch_map = batch
                else:
                    batch_events = len(batch)
                    batch_map = {name: batch[name] for name in branches}

                progress.advance(load_task, batch_events)
                try:
                    processed, inferred, valid = self._process_batch(batch_map, channel)
                except Exception as exc:
                    lengths = {k: len(v) for k, v in batch_map.items()}
                    self.console.print(
                        f"[red]Batch processing failed in tree {tree_name}:[/] {exc}\n"
                        f"Branch lengths: {lengths}\n"
                        f"Batch size: {batch_events}"
                    )
                    self.console.print(traceback.format_exc())
                    raise
                progress.advance(convert_task, batch_events)
                progress.advance(infer_task, batch_events)
                if out_tree is None:
                    outfile[tree.name] = processed
                    out_tree = outfile[tree.name]
                else:
                    out_tree.extend(processed)
                inferred_total += inferred
                valid_total += valid
                progress.advance(write_task, batch_events)
                if overall_task is not None:
                    progress.advance(overall_task, batch_events)

        if not own_progress:
            for task in task_ids:
                progress.remove_task(task)

        duration = time.perf_counter() - start
        return TreeStats(
            name=tree_name,
            entries=total_entries,
            inferred=inferred_total,
            valid=valid_total,
            duration=duration,
        )

    def process_file(
        self,
        input_path: Path,
        output_path: Path,
        trees: Sequence[str] | None = None,
    ) -> List[TreeStats]:
        with uproot.open(input_path, object_cache=None, array_cache=None) as infile:
            selected = self._select_trees(infile, trees)
            if not selected:
                raise RuntimeError("No TTrees found matching the selection.")
            totals = []
            for name in selected:
                tree = infile[name]
                entries = tree.num_entries
                if self.runtime_cfg.max_events is not None:
                    entries = min(entries, self.runtime_cfg.max_events)
                totals.append(entries)
            overall_total = sum(totals)

            with uproot.recreate(output_path) as outfile:
                with Progress(
                    SpinnerColumn(),
                    TextColumn("{task.description}"),
                    BarColumn(),
                    TaskProgressColumn(),
                    TimeElapsedColumn(),
                    TimeRemainingColumn(),
                    console=self.console,
                ) as progress:
                    overall_task = progress.add_task("overall", total=overall_total)
                    stats = []
                    for name in selected:
                        stats.append(
                            self.process_tree(
                                infile,
                                outfile,
                                name,
                                progress=progress,
                                overall_task=overall_task,
                            )
                        )
                    return stats
