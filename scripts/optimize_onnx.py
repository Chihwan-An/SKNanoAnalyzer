#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
지정한 폴더 안의 모든 ONNX 모델에 대해:

  1) (옵션) 첫 번째 축(배치)이 dynamic이면 1로 고정 (static batch=1)
  2) onnxsim으로 simplify
  3) simplify 결과에 대해 dynamic quantization(INT8)

두 가지 결과를 <입력폴더>/optimized/ 아래에 저장:

  - <원본>_simp.onnx
  - <원본>_simp_int8.onnx

사용 예시:
  python optimize_onnx.py /path/to/onnx_dir
  python optimize_onnx.py /path/to/onnx_dir --recursive
  python optimize_onnx.py /path/to/onnx_dir --no-fix-batch
"""

import argparse
import pathlib
import sys

import onnx
from onnxsim import simplify
from onnxruntime.quantization import quantize_dynamic, QuantType


def fix_dynamic_batch_dim_to_1(model: onnx.ModelProto) -> onnx.ModelProto:
    """
    그래프의 모든 value_info/input/output에 대해,
    첫 번째 dim이 dynamic(batch) 이면 1로 고정.

    - dim_param 이 설정되어 있고 dim_value 가 비어 있는 경우:
        dim_value = 1, dim_param = ""
    """
    graph = model.graph

    def fix_vi(vi):
        ttype = vi.type.tensor_type
        if not ttype.HasField("shape"):
            return
        dims = ttype.shape.dim
        if len(dims) == 0:
            return
        d0 = dims[0]
        # dynamic batch: dim_param 존재, dim_value 비어 있음
        if d0.dim_param and not d0.dim_value:
            print(f"      - fixing batch dim in '{vi.name}' "
                  f"({d0.dim_param} -> 1)")
            d0.dim_value = 1
            d0.dim_param = ""

    for vi in list(graph.input) + list(graph.output) + list(graph.value_info):
        fix_vi(vi)

    return model


def simplify_model(input_path: pathlib.Path, output_path: pathlib.Path, fix_batch: bool) -> pathlib.Path:
    print(f"[info] Simplifying: {input_path}")
    model = onnx.load(str(input_path))

    if fix_batch:
        print("      - fixing dynamic batch dim to 1 (if present)")
        model = fix_dynamic_batch_dim_to_1(model)

    model_simp, check = simplify(model)

    if not check:
        print(f"[warn] onnxsim check failed for {input_path.name}. 그래도 저장은 합니다.", file=sys.stderr)

    onnx.save(model_simp, str(output_path))
    print(f"[info]  -> saved simplified: {output_path}")
    return output_path


def quantize_model(
    input_path: pathlib.Path,
    output_path: pathlib.Path,
    weight_type: QuantType = QuantType.QInt8,
) -> pathlib.Path:
    print(f"[info] Quantizing (dynamic INT8) from: {input_path}")
    quantize_dynamic(
        model_input=str(input_path),
        model_output=str(output_path),
        weight_type=weight_type,
    )
    print(f"[info]  -> saved quantized:  {output_path}")
    return output_path


def find_onnx_files(root: pathlib.Path, recursive: bool = False):
    if recursive:
        yield from root.rglob("*.onnx")
    else:
        yield from root.glob("*.onnx")


def main():
    parser = argparse.ArgumentParser(
        description="폴더 내 모든 ONNX에 대해: (옵션) batch=1로 고정 + onnxsim + dynamic quantization 수행"
    )
    parser.add_argument(
        "folder",
        type=pathlib.Path,
        help="ONNX 파일들이 들어 있는 폴더",
    )
    parser.add_argument(
        "--recursive",
        action="store_true",
        help="하위 디렉토리까지 재귀적으로 검색",
    )
    parser.add_argument(
        "--no-fix-batch",
        action="store_true",
        help="첫 번째 축(dynamic batch)을 1로 고정하지 않음",
    )

    args = parser.parse_args()
    root: pathlib.Path = args.folder

    if not root.exists() or not root.is_dir():
        print(f"[error] 유효한 디렉토리가 아닙니다: {root}", file=sys.stderr)
        sys.exit(1)

    optimized_dir = root / "optimized"
    optimized_dir.mkdir(parents=True, exist_ok=True)

    onnx_files = []
    for p in find_onnx_files(root, recursive=args.recursive):
        # optimized 폴더 아래의 onnx는 스킵
        if optimized_dir in p.parents:
            continue
        onnx_files.append(p)

    if not onnx_files:
        print(f"[info] *.onnx 파일을 찾지 못했습니다: {root}")
        return

    print(f"[info] 찾은 ONNX 파일 개수: {len(onnx_files)}")
    print(f"[info] optimized 폴더: {optimized_dir}")
    print(f"[info] fix dynamic batch dim -> 1: {not args.no_fix_batch}\n")

    for idx, onnx_path in enumerate(sorted(onnx_files), start=1):
        print(f"\n[info] ({idx}/{len(onnx_files)}) 처리 중: {onnx_path}")

        stem = onnx_path.stem
        simp_path = optimized_dir / f"{stem}_simp.onnx"
        quant_path = optimized_dir / f"{stem}_simp_int8.onnx"

        try:
            # 1) (옵션) dynamic batch -> 1 + onnxsim
            simp_model_path = simplify_model(
                onnx_path,
                simp_path,
                fix_batch=not args.no_fix_batch,
            )

            # 2) dynamic quantization(INT8)
            quantize_model(simp_model_path, quant_path, weight_type=QuantType.QInt8)

        except Exception as e:
            print(f"[error] {onnx_path} 처리 중 에러: {e}", file=sys.stderr)

    print("\n[done] 모든 ONNX 처리 완료.")
    print(f"       결과 위치: {optimized_dir}")


if __name__ == "__main__":
    main()