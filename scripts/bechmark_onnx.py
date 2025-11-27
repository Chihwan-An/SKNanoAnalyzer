#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
지정한 폴더와 '원본 ONNX 경로' 한 개에 대해서만:

  - <원본>.onnx
  - <folder>/optimized/<stem>_simp.onnx
  - <folder>/optimized/<stem>_simp_int8.onnx

세 모델을 대상으로
  1) 동일 입력(랜덤)으로 출력 차이 (max/mean abs diff) 체크
  2) warmup 후 N번 반복 추론 시간 측정
  3) orig 대비 simp / quant 속도 향상 정도(speedup) 계산

추가 기능:
  - ONNX input 메타데이터 기반으로 자동 shape 추정 (dynamic dim → 1)
  - --input-shape 로 사용자가 직접 shape 지정 가능
    * 단일 입력:  --input-shape 1,11,128
    * 다중 입력:  --input-shape input_ids=1,11,128 --input-shape attention_mask=1,11
  - ONNX declared shape / override shape / 실제 numpy shape 를 rich 테이블로 출력

사용 예시:
  python benchmark_onnx_single.py /path/to/onnx_dir model.onnx
  python benchmark_onnx_single.py /path/to/onnx_dir subdir/model.onnx --runs 100 --warmup 20 --use-cuda
  python benchmark_onnx_single.py /path/to/onnx_dir subdir/model.onnx --input-shape 1,11,128
  python benchmark_onnx_single.py /path/to/onnx_dir subdir/model.onnx --input-shape input_ids=1,11,128 --input-shape attention_mask=1,11
"""

import argparse
import pathlib
import time
import statistics
from typing import Dict, List, Tuple, Any, Optional

import numpy as np
import onnxruntime as ort

from rich.console import Console
from rich.table import Table
from rich.panel import Panel
from rich.text import Text
from rich import box

console = Console()


# ---------- providers & shape helpers ----------

def pick_providers(use_cuda: bool) -> List[str]:
    available = ort.get_available_providers()
    if use_cuda and "CUDAExecutionProvider" in available:
        return ["CUDAExecutionProvider", "CPUExecutionProvider"]
    else:
        return ["CPUExecutionProvider"]


def resolve_shape(shape: List[Any]) -> List[int]:
    """
    ONNX input 메타데이터의 shape 리스트를 받아서,
    int 아니거나 <=0 인 것은 전부 1로 치환.
    (예: ['batch', 'seq_len', 128] -> [1, 1, 128])
    """
    resolved = []
    for d in shape:
        if isinstance(d, int) and d > 0:
            resolved.append(d)
        else:
            resolved.append(1)
    return resolved


def make_random_input_from_shape(resolved_shape: List[int], dtype: np.dtype) -> np.ndarray:
    if np.issubdtype(dtype, np.floating):
        return np.random.randn(*resolved_shape).astype(dtype)
    elif np.issubdtype(dtype, np.integer):
        return np.random.randint(-10, 10, size=resolved_shape, dtype=dtype)
    elif dtype == np.bool_:
        return (np.random.rand(*resolved_shape) > 0.5).astype(np.bool_)
    else:
        # 기타 타입은 float32로 fallback
        return np.random.randn(*resolved_shape).astype(np.float32)


def onnx_type_to_np_dtype(onnx_type_str: str) -> np.dtype:
    t = onnx_type_str
    if "float16" in t:
        return np.float16
    if "float" in t:  # float32
        return np.float32
    if "double" in t:
        return np.float64
    if "int64" in t:
        return np.int64
    if "int32" in t:
        return np.int32
    if "int16" in t:
        return np.int16
    if "int8" in t:
        return np.int8
    if "uint8" in t:
        return np.uint8
    if "bool" in t:
        return np.bool_
    return np.float32


def parse_shape_overrides(
    shape_args: Optional[List[str]],
    session_inputs: List[ort.NodeArg],
) -> Dict[str, List[int]]:
    """
    --input-shape 인자들을 파싱해서
      { input_name: [dim1, dim2, ...] }
    형태의 dict 로 반환.

    허용 형식:
      - "1,11,128" (이름 생략: 단일 입력 모델에서만 허용)
      - "input_ids=1,11,128"
    """
    overrides: Dict[str, List[int]] = {}
    if not shape_args:
        return overrides

    input_names = [i.name for i in session_inputs]

    for spec in shape_args:
        spec = spec.strip()
        if not spec:
            continue

        if "=" in spec:
            name, shape_str = spec.split("=", 1)
            name = name.strip()
            if not name:
                raise ValueError(f"잘못된 --input-shape 형식: '{spec}' (이름이 비어 있음)")
        else:
            # 이름 생략: 단일 입력 모델에서만 허용
            if len(input_names) != 1:
                raise ValueError(
                    f"--input-shape '{spec}' 에 이름이 없는데, 모델 입력이 {len(input_names)}개입니다. "
                    "이름을 명시해 주세요 (예: input_ids=1,11,128)."
                )
            name = input_names[0]
            shape_str = spec

        dims: List[int] = []
        for tok in shape_str.split(","):
            tok = tok.strip()
            if not tok:
                continue
            try:
                d = int(tok)
            except ValueError:
                raise ValueError(f"shape dim이 int가 아닙니다: '{tok}' in '{spec}'")
            if d <= 0:
                raise ValueError(f"shape dim은 양수여야 합니다: '{tok}' in '{spec}'")
            dims.append(d)

        if not dims:
            raise ValueError(f"빈 shape는 허용되지 않습니다: '{spec}'")

        overrides[name] = dims

    return overrides


def build_input_feed(session, shape_overrides=None):
    if shape_overrides is None:
        shape_overrides = {}

    feeds = {}
    for inp in session.get_inputs():
        name = inp.name
        np_dtype = onnx_type_to_np_dtype(inp.type)

        if name in shape_overrides:
            resolved = shape_overrides[name]
        else:
            resolved = resolve_shape(list(inp.shape))

        # --- 여기부터 특수 처리: SPANet 스타일 입력 예시 ---
        if name == "Momenta_data":
            batch, max_jet, feat = resolved
            arr = np.zeros((batch, max_jet, feat), dtype=np_dtype)
            # 3 < n_jets <= max_jet
            n_jets = np.random.randint(4, max_jet + 1)
            arr[:, :n_jets, :] = np.random.randn(batch, n_jets, feat).astype(np_dtype)
            feeds[name] = arr

        elif name == "Momenta_mask":
            batch, max_jet = resolved
            mask = np.zeros((batch, max_jet), dtype=np.bool_)
            # Momenta_data에서 쓴 n_jets를 재사용하려면 위에서 저장해두는게 베스트지만
            # 간단하게 다시 뽑자:
            n_jets = np.random.randint(4, max_jet + 1)
            mask[:, :n_jets] = True
            feeds[name] = mask

        else:
            # 기본 처리
            arr = make_random_input_from_shape(resolved, np_dtype)
            feeds[name] = arr

    return feeds

def format_shape_list(shape: List[Any]) -> str:
    """
    ONNX 메타데이터 shape (int 또는 str/None) 을 보기 좋게 문자열로 변환.
    예: [1, 'seq', 128] -> "(1, seq, 128)"
    """
    parts = []
    for d in shape:
        if d is None:
            parts.append("None")
        else:
            parts.append(str(d))
    return "(" + ", ".join(parts) + ")"


def format_override_shape(shape_overrides: Dict[str, List[int]], name: str) -> str:
    if name not in shape_overrides:
        return "-"
    return "(" + ", ".join(str(d) for d in shape_overrides[name]) + ")"


def print_input_shape_table(
    session: ort.InferenceSession,
    feeds: Dict[str, np.ndarray],
    shape_overrides: Dict[str, List[int]],
) -> None:
    """
    ONNX가 선언한 input shape vs override shape vs 실제 numpy 배열의 shape를
    rich 테이블로 출력.
    """
    table = Table(
        title="Input Tensor Shapes (ONNX / Override / Actual)",
        box=box.MINIMAL_DOUBLE_HEAD,
        show_lines=False,
    )
    table.add_column("name", style="bold")
    table.add_column("onnx type")
    table.add_column("declared shape", justify="left")
    table.add_column("override shape", justify="left")
    table.add_column("actual shape", justify="left")
    table.add_column("np dtype", justify="left")

    for inp in session.get_inputs():
        name = inp.name
        onnx_type = inp.type
        declared_shape = format_shape_list(list(inp.shape))
        override_shape = format_override_shape(shape_overrides, name)

        if name in feeds:
            actual_shape = str(tuple(feeds[name].shape))
            np_dtype = str(feeds[name].dtype)
        else:
            actual_shape = "-"
            np_dtype = "-"

        table.add_row(name, onnx_type, declared_shape, override_shape, actual_shape, np_dtype)

    console.print(table)
    console.print()  # 한 줄 띄우기


# ---------- benchmarking helpers ----------

def benchmark_session(
    session: ort.InferenceSession,
    inputs: Dict[str, np.ndarray],
    warmup: int,
    runs: int,
) -> Tuple[float, float, float]:
    """
    session.run(None, inputs)를 여러 번 실행하여
    평균/중앙값/p90 latency(ms) 를 반환.
    """
    # warmup
    for _ in range(warmup):
        session.run(None, inputs)

    latencies = []
    for _ in range(runs):
        t0 = time.perf_counter()
        session.run(None, inputs)
        t1 = time.perf_counter()
        latencies.append((t1 - t0) * 1000.0)

    mean_ms = float(statistics.mean(latencies))
    median_ms = float(statistics.median(latencies))
    p90_ms = float(np.percentile(latencies, 90.0))
    return mean_ms, median_ms, p90_ms


def flatten_outputs(outs: List[np.ndarray]) -> np.ndarray:
    flat_list = []
    for a in outs:
        flat_list.append(a.ravel())
    if not flat_list:
        return np.array([], dtype=np.float32)
    return np.concatenate(flat_list)


def compare_outputs(
    sess_ref: ort.InferenceSession,
    sess_test: ort.InferenceSession,
    inputs: Dict[str, np.ndarray],
) -> Tuple[float, float]:
    outs_ref = sess_ref.run(None, inputs)
    outs_test = sess_test.run(None, inputs)

    if len(outs_ref) != len(outs_test):
        console.print(
            "[bold yellow]⚠ 출력 개수가 다릅니다. diff 측정 불가.[/bold yellow]"
        )
        return float("nan"), float("nan")

    flat_ref = flatten_outputs(outs_ref)
    flat_test = flatten_outputs(outs_test)

    if flat_ref.shape != flat_test.shape:
        console.print(
            "[bold yellow]⚠ 출력 shape가 다릅니다. diff 측정 불가.[/bold yellow]"
        )
        return float("nan"), float("nan")

    diff = flat_ref - flat_test
    max_abs = float(np.max(np.abs(diff)))
    mean_abs = float(np.mean(np.abs(diff)))
    return max_abs, mean_abs


# ---------- main ----------

def main():
    parser = argparse.ArgumentParser(
        description="원본 ONNX 한 개와 그에 대응하는 *_simp / *_simp_int8 를 rich 출력으로 벤치마크"
    )
    parser.add_argument(
        "folder",
        type=pathlib.Path,
        help="optimize_onnx.py 기준 root 폴더 (optimized/가 생성된 위치)",
    )
    parser.add_argument(
        "model",
        type=pathlib.Path,
        help="벤치마크할 원본 ONNX 경로 (절대경로 또는 folder 기준 상대경로)",
    )
    parser.add_argument(
        "--runs",
        type=int,
        default=50,
        help="측정용 반복 추론 횟수 (기본: 50)",
    )
    parser.add_argument(
        "--warmup",
        type=int,
        default=10,
        help="워밍업 추론 횟수 (기본: 10)",
    )
    parser.add_argument(
        "--use-cuda",
        action="store_true",
        help="가능하면 CUDAExecutionProvider 사용",
    )
    parser.add_argument(
        "--input-shape",
        action="append",
        help=(
            "입력 텐서 shape를 직접 지정. 여러 번 사용할 수 있음.\n"
            " - 단일 입력:  --input-shape 1,11,128\n"
            " - 다중 입력:  --input-shape input_ids=1,11,128 --input-shape attention_mask=1,11"
        ),
    )

    args = parser.parse_args()
    root: pathlib.Path = args.folder
    model_path: pathlib.Path = args.model

    if not root.exists() or not root.is_dir():
        console.print(
            f"[bold red]✘ 유효한 디렉토리가 아닙니다:[/bold red] {root}"
        )
        raise SystemExit(1)

    # model 경로 해석: 상대경로면 root 기준
    if not model_path.is_absolute():
        model_path = (root / model_path).resolve()

    if not model_path.exists():
        console.print(
            f"[bold red]✘ 원본 ONNX 파일을 찾을 수 없습니다:[/bold red] {model_path}"
        )
        raise SystemExit(1)

    stem = model_path.stem
    optimized_dir = root / "optimized"
    simp_path = optimized_dir / f"{stem}_simp.onnx"
    quant_path = optimized_dir / f"{stem}_simp_int8.onnx"

    # 헤더 패널
    header_text = Text.assemble(
        ("ONNX Benchmark (single model)\n", "bold magenta"),
        (f"\nroot:   {str(root)}\n", "cyan"),
        (f"model:  {str(model_path)}\n", "cyan"),
        (f"runs:   {args.runs}, warmup: {args.warmup}\n", "cyan"),
        (f"use_cuda: {args.use_cuda}", "cyan"),
    )
    console.print(Panel(header_text, title="[bold]Benchmark Config[/bold]", expand=False))

    # 파일 존재 체크
    miss = []
    if not simp_path.exists():
        miss.append(str(simp_path))
    if not quant_path.exists():
        miss.append(str(quant_path))

    if miss:
        console.print("[bold red]✘ 필요한 파일이 없습니다:[/bold red]")
        for m in miss:
            console.print(f"  - {m}")
        console.print(
            "[yellow]먼저 optimize_onnx.py를 돌려서 *_simp / *_simp_int8 를 만들어 주세요.[/yellow]"
        )
        raise SystemExit(1)

    providers = pick_providers(args.use_cuda)
    console.print(f"[bold]사용 providers:[/bold] {providers}\n")

    # 세션 로드
    console.print("[bold green]세션 로드 중...[/bold green]")
    sess_orig = ort.InferenceSession(str(model_path), providers=providers)
    sess_simp = ort.InferenceSession(str(simp_path), providers=providers)
    sess_quant = ort.InferenceSession(str(quant_path), providers=providers)

    # shape override 파싱
    try:
        shape_overrides = parse_shape_overrides(args.input_shape, sess_orig.get_inputs())
    except ValueError as e:
        console.print(f"[bold red]✘ --input-shape 파싱 에러:[/bold red] {e}")
        raise SystemExit(1)

    # 입력 feed 생성 (원본 기준, 자동 + override)
    console.print("[bold]입력 텐서 생성 (ONNX 메타데이터 + --input-shape override)[/bold]")
    feeds = build_input_feed(sess_orig, shape_overrides)

    # 어떤 shape가 실제로 쓰였는지 테이블 출력
    print_input_shape_table(sess_orig, feeds, shape_overrides)

    # 출력 차이 비교
    console.print("[bold blue]1) 출력 차이 (orig vs simp / quant)[/bold blue]")
    max_abs_simp, mean_abs_simp = compare_outputs(sess_orig, sess_simp, feeds)
    max_abs_quant, mean_abs_quant = compare_outputs(sess_orig, sess_quant, feeds)

    diff_table = Table(
        title="Output Difference (single run)",
        box=box.MINIMAL_DOUBLE_HEAD,
        show_lines=False,
    )
    diff_table.add_column("Variant", style="bold")
    diff_table.add_column("max |Δ|", justify="right")
    diff_table.add_column("mean |Δ|", justify="right")

    diff_table.add_row(
        "simp vs orig",
        f"{max_abs_simp:.3e}",
        f"{mean_abs_simp:.3e}",
    )
    diff_table.add_row(
        "quant vs orig",
        f"{max_abs_quant:.3e}",
        f"{mean_abs_quant:.3e}",
    )

    console.print(diff_table)
    console.print()

    # 속도 측정
    console.print(
        f"[bold blue]2) 추론 속도 측정[/bold blue] "
        f"(warmup={args.warmup}, runs={args.runs})"
    )

    mean_o, med_o, p90_o = benchmark_session(sess_orig, feeds, args.warmup, args.runs)
    mean_s, med_s, p90_s = benchmark_session(sess_simp, feeds, args.warmup, args.runs)
    mean_q, med_q, p90_q = benchmark_session(sess_quant, feeds, args.warmup, args.runs)

    speedup_s = mean_o / mean_s if mean_s > 0 else float("nan")
    speedup_q = mean_o / mean_q if mean_q > 0 else float("nan")

    lat_table = Table(
        title="Latency Benchmark (ms)",
        box=box.MINIMAL_DOUBLE_HEAD,
        show_lines=False,
    )
    lat_table.add_column("Variant", style="bold")
    lat_table.add_column("mean", justify="right")
    lat_table.add_column("median", justify="right")
    lat_table.add_column("p90", justify="right")
    lat_table.add_column("speedup vs orig", justify="right")

    lat_table.add_row(
        "orig",
        f"{mean_o:.3f}",
        f"{med_o:.3f}",
        f"{p90_o:.3f}",
        "1.00x",
    )
    lat_table.add_row(
        "simp",
        f"{mean_s:.3f}",
        f"{med_s:.3f}",
        f"{p90_s:.3f}",
        f"{speedup_s:.2f}x",
    )
    lat_table.add_row(
        "quant",
        f"{mean_q:.3f}",
        f"{med_q:.3f}",
        f"{p90_q:.3f}",
        f"{speedup_q:.2f}x",
    )

    console.print()
    console.print(lat_table)

    # 요약 패널
    summary_text = Text()
    summary_text.append("orig → simp speedup: ", style="bold")
    summary_text.append(f"{speedup_s:.2f}x\n", style="green")
    summary_text.append("orig → quant speedup: ", style="bold")
    summary_text.append(f"{speedup_q:.2f}x\n", style="green")
    summary_text.append("\n입력 텐서 shape는 위 'Input Tensor Shapes' 테이블을 참고.")

    console.print()
    console.print(Panel(summary_text, title="[bold]Summary[/bold]", expand=False))


if __name__ == "__main__":
    main()