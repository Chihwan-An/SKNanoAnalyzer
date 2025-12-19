from __future__ import annotations

from typing import Dict, Tuple

import awkward as ak
import numpy as np

from .config import LEPTON_DIM, MAX_JETS, MET_DIM, MOM_FEAT_DIM


def detect_channel(tree_name: str, override: str | None = None) -> str:
    """
    Return "El" or "Mu" based on an explicit override or the tree name prefix.
    """
    if override and override != "auto":
        return override
    if tree_name.startswith("El_"):
        return "El"
    if tree_name.startswith("Mu_"):
        return "Mu"
    raise ValueError(
        f"Could not infer channel from tree name '{tree_name}'. "
        "Pass --channel {El,Mu} to override."
    )




def _pad_vector(array: ak.Array, length: int, fill) -> Tuple[np.ndarray, np.ndarray]:
    counts = ak.num(array, axis=1)              
    padded = ak.pad_none(array, length, axis=1, clip=True)
    filled = ak.fill_none(padded, fill)
    filled_np = ak.to_numpy(filled)            
    counts_np = ak.to_numpy(counts).astype(np.int64)
    idx = np.arange(length)[None, :]            
    mask_np = idx < counts_np[:, None]          

    return filled_np, mask_np



def build_spanet_inputs(
    batch: Dict[str, ak.Array],
    channel: str,
    max_jets: int = MAX_JETS,
) -> Tuple[Dict[str, np.ndarray], np.ndarray, Dict[str, np.ndarray]]:
    """
    Convert a ROOT batch to SPANet ONNX inputs.

    Returns (inputs_dict, jet_counts) where jet_counts respects padding.
    """
    jet_pt_np, jet_mask_np = _pad_vector(batch["Jet_Pt"], max_jets, 0.0)
    jet_eta_np, _ = _pad_vector(batch["Jet_Eta"], max_jets, 0.0)
    jet_phi_np, _ = _pad_vector(batch["Jet_Phi"], max_jets, 0.0)
    jet_mass_np, _ = _pad_vector(batch["Jet_Mass"], max_jets, 0.0)
    jet_cat_np, _ = _pad_vector(batch["Jet_Category"], max_jets, 0)
    jet_counts = np.asarray(ak.num(batch["Jet_Pt"]), dtype=np.int32)

    sin_phi = np.sin(jet_phi_np).astype(np.float32)
    cos_phi = np.cos(jet_phi_np).astype(np.float32)

    N0 = (jet_cat_np == 0).astype(np.float32)
    L0 = (jet_cat_np == 1).astype(np.float32)
    C_bits = [(jet_cat_np == (2 + i)).astype(np.float32) for i in range(5)]
    B_bits = [(jet_cat_np == (7 + i)).astype(np.float32) for i in range(5)]

    mom_components = [
        jet_pt_np[..., None],
        jet_eta_np[..., None],
        sin_phi[..., None],
        cos_phi[..., None],
        jet_mass_np[..., None],
        N0[..., None],
        L0[..., None],
        np.stack(C_bits, axis=-1),
        np.stack(B_bits, axis=-1),
    ]
    momenta_data = np.concatenate(mom_components, axis=2).astype(np.float32)
    if momenta_data.shape != (
        jet_pt_np.shape[0],
        max_jets,
        MOM_FEAT_DIM,
    ):
        raise RuntimeError(
            f"Unexpected Momenta_data shape {momenta_data.shape}, "
            f"expected (n_events, {max_jets}, {MOM_FEAT_DIM})"
        )

    lepton_pt, lepton_mask = _pad_vector(batch["Lepton_Pt"], 1, 0.0)
    lepton_eta, _ = _pad_vector(batch["Lepton_Eta"], 1, 0.0)
    lepton_phi, _ = _pad_vector(batch["Lepton_Phi"], 1, 0.0)
    lepton_mass, _ = _pad_vector(batch["Lepton_Mass"], 1, 0.0)

    lep_pt_np = np.asarray(ak.to_numpy(lepton_pt)).astype(np.float32).reshape(-1, 1)
    lep_eta_np = np.asarray(ak.to_numpy(lepton_eta)).astype(np.float32).reshape(-1, 1)
    lep_phi_np = np.asarray(ak.to_numpy(lepton_phi)).astype(np.float32).reshape(-1, 1)
    lep_mass_np = (
        np.asarray(ak.to_numpy(lepton_mass)).astype(np.float32).reshape(-1, 1)
    )
    lep_mask_np = np.asarray(ak.to_numpy(lepton_mask)).astype(np.bool_).reshape(-1, 1)

    lep_sin_phi = np.sin(lep_phi_np).astype(np.float32)
    lep_cos_phi = np.cos(lep_phi_np).astype(np.float32)

    if channel not in {"El", "Mu"}:
        raise ValueError(f"Unsupported channel '{channel}'")
    utag = np.where(channel == "Mu", 1.0, 0.0).astype(np.float32)
    etag = np.where(channel == "El", 1.0, 0.0).astype(np.float32)
    utag = np.broadcast_to(utag, lep_pt_np.shape)
    etag = np.broadcast_to(etag, lep_pt_np.shape)

    lepton_data = (
        np.concatenate(
            [
                lep_pt_np,
                lep_eta_np,
                lep_sin_phi,
                lep_cos_phi,
                lep_mass_np,
                utag,
                etag,
            ],
            axis=1,
        ).astype(np.float32)
    )
    lepton_data = lepton_data.reshape(-1, 1, LEPTON_DIM)

    met_pt = np.asarray(batch["Met_Pt"], dtype=np.float32).reshape(-1)
    met_phi = np.asarray(batch["Met_Phi"], dtype=np.float32).reshape(-1)
    met_data = np.stack(
        [met_pt, np.sin(met_phi).astype(np.float32), np.cos(met_phi).astype(np.float32)],
        axis=1,
    ).reshape(-1, 1, MET_DIM)

    inputs: Dict[str, np.ndarray] = {
        "Momenta_data": momenta_data,
        "Momenta_mask": jet_mask_np,
        "Met_data": met_data,
        "Met_mask": np.ones((met_data.shape[0], 1), dtype=np.bool_),
        "Lepton_data": lepton_data,
        "Lepton_mask": lep_mask_np.reshape(-1, 1),
    }
    jets = {
        "pt": jet_pt_np,
        "eta": jet_eta_np,
        "phi": jet_phi_np,
        "mass": jet_mass_np,
        "cat": jet_cat_np,
        "mask": jet_mask_np,
    }
    return inputs, jet_counts, jets
