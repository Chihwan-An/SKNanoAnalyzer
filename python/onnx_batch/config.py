from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
from typing import Dict, Sequence

# Geometry for the SPANet inputs
MAX_JETS = 8
MOM_FEAT_DIM = 17
MET_DIM = 3
LEPTON_DIM = 7

# Default runtime knobs
DEFAULT_BATCH_SIZE = 8192
DEFAULT_CHUNK_SIZE = 65536 * 64
DEFAULT_OUTPUT_SUFFIX = "_onnx.root"

# Model naming
DEFAULT_CLASS_PATTERN = "spanet_version_{fold}_CLASSIF.onnx"
DEFAULT_RECO_PATTERN = "spanet_version_{fold}_RECO.onnx"

# Locations
REPO_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_MODEL_DIR = REPO_ROOT / "data"

# Branches added to the output tree. Values are uproot compatible type strings.
ONNX_OUTPUT_BRANCH_TYPES: Dict[str, str] = {
    "logp_class_0": "float32",
    "logp_class_1": "float32",
    "logp_class_2": "float32",
    "logp_class_3": "float32",
    "logp_class_4": "float32",
    "logp_class_5": "float32",
    "detection_score_logp": "float32",
    "assignment_logp": "float32",
    "assignment_w1_idx": "int32",
    "assignment_w2_idx": "int32",
    "assignment_hb_idx": "int32",
    "assignment_lb_idx": "int32",
    "onnx_assignment_valid": "bool",
    # TemplateTraining-style outputs
    "m_had_w": "float32",
    "pt_w_u": "float32",
    "pt_w_d": "float32",
    "eta_w_u": "float32",
    "eta_w_d": "float32",
    "Cat_w_u": "int32",
    "Cat_w_d": "int32",
    "N0_w_u": "float32",
    "L0_w_u": "float32",
    "N0_w_d": "float32",
    "L0_w_d": "float32",
    "C0_w_u": "float32",
    "C1_w_u": "float32",
    "C2_w_u": "float32",
    "C3_w_u": "float32",
    "C4_w_u": "float32",
    "B0_w_u": "float32",
    "B1_w_u": "float32",
    "B2_w_u": "float32",
    "B3_w_u": "float32",
    "B4_w_u": "float32",
    "C0_w_d": "float32",
    "C1_w_d": "float32",
    "C2_w_d": "float32",
    "C3_w_d": "float32",
    "C4_w_d": "float32",
    "B0_w_d": "float32",
    "B1_w_d": "float32",
    "B2_w_d": "float32",
    "B3_w_d": "float32",
    "B4_w_d": "float32",
}

# Expected folds
SPANET_FOLDS: Sequence[int] = (0, 1, 2, 3)


@dataclass(frozen=True)
class ModelConfig:
    model_dir: Path = DEFAULT_MODEL_DIR
    class_pattern: str = DEFAULT_CLASS_PATTERN
    reco_pattern: str = DEFAULT_RECO_PATTERN
    prefer_cuda: bool = True


@dataclass(frozen=True)
class RuntimeConfig:
    batch_size: int = DEFAULT_BATCH_SIZE
    chunk_size: int = DEFAULT_CHUNK_SIZE
    max_events: int | None = None
    channel_hint: str = "auto"  # "auto", "El", "Mu"
