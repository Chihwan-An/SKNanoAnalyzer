"""
Batched ONNX inference utilities for SKNanoAnalyzer.

Use ``python -m onnx_batch`` or ``python python/onnx_batch/main.py`` to run.
"""

from .config import (
    DEFAULT_BATCH_SIZE,
    DEFAULT_CHUNK_SIZE,
    DEFAULT_CLASS_PATTERN,
    DEFAULT_MODEL_DIR,
    DEFAULT_OUTPUT_SUFFIX,
    DEFAULT_RECO_PATTERN,
    MAX_JETS,
    MAX_JETS_CLASSIF,
    MAX_JETS_RECO,
    ONNX_OUTPUT_BRANCH_TYPES,
)
from .main import main

__all__ = [
    "DEFAULT_BATCH_SIZE",
    "DEFAULT_CHUNK_SIZE",
    "DEFAULT_CLASS_PATTERN",
    "DEFAULT_MODEL_DIR",
    "DEFAULT_OUTPUT_SUFFIX",
    "DEFAULT_RECO_PATTERN",
    "MAX_JETS",
    "MAX_JETS_CLASSIF",
    "MAX_JETS_RECO",
    "ONNX_OUTPUT_BRANCH_TYPES",
    "main",
]
