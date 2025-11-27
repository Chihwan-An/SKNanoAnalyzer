#!/usr/bin/env python3
from __future__ import annotations

import argparse
from pathlib import Path
from typing import List, Union, Dict

import onnx
from onnx import mapping
import numpy as np


def get_value_info_shape(value_info) -> List[Union[str, int]]:
    """
    Return a shape list where:
      - symbolic dims: 'batch_size', 'seq_len', ...
      - known dims: integer
      - unknown dims: '?'
    """
    shape: List[Union[str, int]] = []

    if not value_info.type.HasField("tensor_type"):
        return shape

    tensor_type = value_info.type.tensor_type
    for d in tensor_type.shape.dim:
        if d.dim_param:
            shape.append(d.dim_param)
        elif d.dim_value > 0:
            shape.append(int(d.dim_value))
        else:
            shape.append("?")
    return shape


def get_elem_type_str(value_info) -> str:
    """Return numpy-style dtype string for a ValueInfo, or '?'."""
    if not value_info.type.HasField("tensor_type"):
        return "?"
    elem_type = value_info.type.tensor_type.elem_type
    try:
        np_dtype = mapping.TENSOR_TYPE_TO_NP_TYPE[elem_type]
        return str(np_dtype)
    except KeyError:
        return f"onnx_dtype_{elem_type}"


def make_concrete_shape(
    shape: List[Union[str, int]],
    batch_size: int,
    default_dim: int,
) -> List[int]:
    """
    shape: ['batch_size', 'num_Momenta', 17] 같은 리스트를
           [batch_size, default_dim, 17] 같은 실제 정수 리스트로 변환.

    규칙:
      - 첫 번째 축(보통 batch)은 무조건 batch_size로 설정
      - 정수 dim은 그대로 사용
      - '?', 심볼릭 dim은 default_dim으로 대체
    """
    concrete: List[int] = []
    for idx, d in enumerate(shape):
        if idx == 0:
            concrete.append(batch_size)
        else:
            if isinstance(d, int) and d > 0:
                concrete.append(d)
            else:
                concrete.append(default_dim)
    return concrete


def make_dummy_input(
    value_info,
    batch_size: int,
    default_dim: int,
) -> np.ndarray | None:
    """
    value_info로부터 dtype, shape를 보고 dummy numpy array 생성.
    자동으로 shape를 추론할 때만 사용 (수동 지정 없을 때).
    """
    shape_sym = get_value_info_shape(value_info)
    shape = make_concrete_shape(shape_sym, batch_size, default_dim)

    if not value_info.type.HasField("tensor_type"):
        return None

    elem_type = value_info.type.tensor_type.elem_type
    try:
        np_dtype = mapping.TENSOR_TYPE_TO_NP_TYPE[elem_type]
    except KeyError:
        np_dtype = np.float32

    if np.issubdtype(np_dtype, np.floating):
        arr = np.random.randn(*shape).astype(np_dtype)
    elif np.issubdtype(np_dtype, np.integer):
        arr = np.random.randint(low=0, high=3, size=shape, dtype=np_dtype)
    elif np_dtype == np.bool_:
        arr = (np.random.rand(*shape) > 0.5)
    else:
        arr = np.zeros(shape, dtype=np_dtype)

    return arr


def inspect_onnx(
    path: Path,
    *,
    show_runtime_shapes: bool = False,
    batch_size: int = 1,
    default_dim: int = 4,
    manual_input_shapes: Dict[str, List[int]] | None = None,
) -> None:
    if manual_input_shapes is None:
        manual_input_shapes = {}

    model = onnx.load(path)
    graph = model.graph
    initializer_names = {init.name for init in graph.initializer}

    print("=" * 80)
    print(f"Model: {path}")
    print("-" * 80)

    # ---- Static (symbolic) I/O info ----
    print("[Inputs] (static graph shapes)")
    real_inputs = []
    for i in graph.input:
        if i.name in initializer_names:
            # weight/bias initializer는 스킵
            continue
        shape = get_value_info_shape(i)
        dtype = get_elem_type_str(i)
        print(f"  name={i.name}, dtype={dtype}, shape={shape}")
        real_inputs.append(i)

    print("\n[Outputs] (static graph shapes)")
    for o in graph.output:
        shape = get_value_info_shape(o)
        dtype = get_elem_type_str(o)
        print(f"  name={o.name}, dtype={dtype}, shape={shape}")

    # ---- Runtime shapes (by actually running the model) ----
    if not show_runtime_shapes:
        print()
        return

    try:
        import onnxruntime as ort
    except ImportError:
        print("\n[Runtime shapes]")
        print("  onnxruntime is not installed; "
              "install with `pip install onnxruntime` to enable this feature.\n")
        return

    print("\n[Runtime shapes] (with dummy or manual inputs)")
    print(f"  batch_size={batch_size}, default_dim={default_dim}")

    sess = ort.InferenceSession(str(path), providers=["CPUExecutionProvider"])

    feeds: Dict[str, np.ndarray] = {}
    for i in real_inputs:
        # 1) 수동 지정 shape가 있으면 그걸 우선 사용
        if i.name in manual_input_shapes:
            shape = manual_input_shapes[i.name]
            if not i.type.HasField("tensor_type"):
                continue
            elem_type = i.type.tensor_type.elem_type
            try:
                np_dtype = mapping.TENSOR_TYPE_TO_NP_TYPE[elem_type]
            except KeyError:
                np_dtype = np.float32

            if np.issubdtype(np_dtype, np.floating):
                dummy = np.random.randn(*shape).astype(np_dtype)
            elif np.issubdtype(np_dtype, np.integer):
                dummy = np.random.randint(low=0, high=3, size=shape, dtype=np_dtype)
            elif np_dtype == np.bool_:
                dummy = (np.random.rand(*shape) > 0.5)
            else:
                dummy = np.zeros(shape, dtype=np_dtype)
        else:
            # 2) 자동 추론(default_dim) 사용
            dummy = make_dummy_input(i, batch_size=batch_size, default_dim=default_dim)

        if dummy is None:
            continue
        feeds[i.name] = dummy

    try:
        outputs = sess.run(None, feeds)
    except Exception as e:
        print("\n[Runtime shapes] ERROR during forward pass:")
        print(f"  {type(e).__name__}: {e}")
        print("  -> Dummy input shapes are inconsistent with the model's internal reshapes.")
        print("     Use --input-shape to match the real expected shapes (e.g. Momenta_data=1,8,17).")
        print()
        return

    out_names = [o.name for o in sess.get_outputs()]
    for name, arr in zip(out_names, outputs):
        print(f"  {name}: shape={arr.shape}, dtype={arr.dtype}")

    print()


def main():
    ap = argparse.ArgumentParser(
        description="Inspect ONNX model I/O shapes; optionally run once to see actual output shapes."
    )
    ap.add_argument("onnx", nargs="+", help="ONNX model file(s)")
    ap.add_argument(
        "--runtime",
        action="store_true",
        help="Run a forward pass with dummy inputs to show actual output shapes.",
    )
    ap.add_argument(
        "--batch-size",
        type=int,
        default=1,
        help="Batch size to use for dummy inputs when --runtime is set.",
    )
    ap.add_argument(
        "--default-dim",
        type=int,
        default=4,
        help="Default size for non-batch dynamic dimensions when --runtime is set.",
    )
    ap.add_argument(
        "--input-shape",
        action="append",
        default=[],
        metavar="NAME=dim0,dim1,...",
        help=(
            "Override dummy input shape for a given input tensor. "
            "Can be passed multiple times. "
            "Example: --input-shape Momenta_data=1,8,17"
        ),
    )
    args = ap.parse_args()

    # parse --input-shape into dict[str, List[int]]
    manual_shapes: Dict[str, List[int]] = {}
    for spec in args.input_shape:
        try:
            name, dims_str = spec.split("=", 1)
            dims = [int(x) for x in dims_str.split(",") if x]
            manual_shapes[name.strip()] = dims
        except Exception:
            raise SystemExit(
                f"Invalid --input-shape spec: {spec!r} (expected NAME=d0,d1,...)"
            )

    for p in args.onnx:
        inspect_onnx(
            Path(p),
            show_runtime_shapes=args.runtime,
            batch_size=args.batch_size,
            default_dim=args.default_dim,
            manual_input_shapes=manual_shapes,
        )


if __name__ == "__main__":
    main()