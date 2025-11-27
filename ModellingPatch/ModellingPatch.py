#!/usr/bin/env python3
import json
import os
import argparse
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, Dict, List, Optional, Tuple
import math
import multiprocessing as mp

import awkward as ak
import numpy as np
import rich
import uproot

from rich.progress import Progress, BarColumn, TimeRemainingColumn, TimeElapsedColumn
from rich.panel import Panel
from rich.table import Table
from rich import print

import onnxruntime as ort


# ----------------------------
# Data containers
# ----------------------------
@dataclass
class ResultSubProcess:
    name: str
    nMC: int = 0
    sumSign: float = 0.0
    sumScaleVariation: Optional[np.ndarray] = None
    sumPSVariation: Optional[np.ndarray] = None
    # TT 전용 합계
    sum_hdamp_up: float = 0.0
    sum_hdamp_down: float = 0.0
    sum_minnlo: float = 0.0

    def accumulate_common(
        self,
        sign: np.ndarray,
        scale: Optional[np.ndarray],
        ps: Optional[np.ndarray],
    ):
        """chunk 하나에 대한 공통 합계를 누적"""
        n = sign.shape[0]
        self.nMC += int(n)
        self.sumSign += float(np.sum(sign))

        if scale is not None:
            # scale: (N, nScale)
            scale_sum = np.sum(scale * sign.reshape(-1, 1), axis=0)
            if self.sumScaleVariation is None:
                self.sumScaleVariation = scale_sum
            else:
                self.sumScaleVariation += scale_sum

        if ps is not None:
            ps_sum = np.sum(ps * sign.reshape(-1, 1), axis=0)
            if self.sumPSVariation is None:
                self.sumPSVariation = ps_sum
            else:
                self.sumPSVariation += ps_sum

    def accumulate_tt(
        self,
        sign: np.ndarray,
        hdamp_up: np.ndarray,
        hdamp_down: np.ndarray,
        minnlo: np.ndarray,
    ):
        self.sum_hdamp_up += float(np.sum(hdamp_up * sign))
        self.sum_hdamp_down += float(np.sum(hdamp_down * sign))
        self.sum_minnlo += float(np.sum(minnlo * sign))


@dataclass
class ResultMain:
    PD: str
    subprocess_names: Optional[List[str]] = None
    subprocesses_dict: Dict[str, ResultSubProcess] = field(init=False)

    def __post_init__(self):
        if self.subprocess_names is None:
            self.subprocesses_dict = {self.PD: ResultSubProcess(self.PD)}
        else:
            self.subprocesses_dict = {
                name: ResultSubProcess(name) for name in self.subprocess_names
            }

    def get_subprocess(self, name: str) -> Optional[ResultSubProcess]:
        return self.subprocesses_dict.get(name)


# ----------------------------
# Helpers
# ----------------------------
def list_files(PD: str, era: str) -> List[str]:
    """SKNANO_DATA/era/ForSNU/PD.json 을 열어서 path 리스트만 뽑는다."""
    base = os.environ.get("SKNANO_DATA")
    if not base:
        raise RuntimeError("SKNANO_DATA 환경변수가 설정돼 있지 않습니다.")
    sampleinfo_path = Path(base) / era / "Sample" / "ForSNU" / f"{PD}.json"
    rich.print(f"[bold green]Reading sample info: {sampleinfo_path}[/bold green]")
    if not sampleinfo_path.exists():
        raise FileNotFoundError(f"Sample info file not found: {sampleinfo_path}")

    with open(sampleinfo_path, "r") as f:
        data = json.load(f)

    return [f"{fpath}:Events" for fpath in data["path"]]


def load_onnx_session(model_path: str, providers=None):
    if providers is None:
        providers = ["CUDAExecutionProvider","CPUExecutionProvider"]

    p = Path(model_path)
    if not p.exists():
        raise FileNotFoundError(f"ONNX model not found: {model_path}")

    sess = ort.InferenceSession(str(p), providers=providers)

    inputs = sess.get_inputs()
    outputs = sess.get_outputs()

    # 테이블 구성
    table = Table(show_header=True, header_style="bold magenta")
    table.add_column("IO", style="cyan", no_wrap=True)
    table.add_column("Name", style="green")
    table.add_column("Type", style="yellow")
    table.add_column("Shape", style="white")

    for inp in inputs:
        table.add_row("input", inp.name, inp.type, str(inp.shape))
    for out in outputs:
        table.add_row("output", out.name, out.type, str(out.shape))

    panel = Panel(
        table,
        title=f"[bold green]Loaded ONNX model[/bold green]",
        subtitle=f"path: {p} | providers: {sess.get_providers()}",
        expand=False,
    )
    print(panel)

    first_input_name = inputs[0].name
    first_output_name = outputs[0].name
    return sess, first_input_name, first_output_name


def _ptetaphim_to_pxpypzE(pt, eta, phi, mass):
    px = pt * np.cos(phi)
    py = pt * np.sin(phi)
    pz = pt * np.sinh(eta)
    E = np.sqrt(px * px + py * py + pz * pz + mass * mass)
    return px, py, pz, E


def _pxpypzE_to_ptetaphim(px, py, pz, E):
    pt = np.sqrt(px * px + py * py)
    p = np.sqrt(px * px + py * py + pz * pz)
    denom = np.clip(p - pz, 1e-7, None)
    num = np.clip(p + pz, 1e-7, None)
    eta = 0.5 * np.log(num / denom)
    phi = np.arctan2(py, px)
    mass2 = E * E - p * p
    mass2 = np.clip(mass2, 0.0, None)
    mass = np.sqrt(mass2)
    return pt, eta, phi, mass


def create_onnx_input_batch_with_ttbar(arrays: Dict[str, ak.Array], isFirstCopy: bool) -> np.ndarray:
    if not isFirstCopy:
        mask_lastcopy = (arrays["GenPart_statusFlags"] & (1 << 13)) != 0
        mask_top = arrays["GenPart_pdgId"] == 6
        mask_antitop = arrays["GenPart_pdgId"] == -6

        mask_lastcopy_top = mask_lastcopy & mask_top
        mask_lastcopy_antitop = mask_lastcopy & mask_antitop

        n_top_per_evt = ak.num(arrays["GenPart_pt"][mask_lastcopy_top])
        n_antitop_per_evt = ak.num(arrays["GenPart_pt"][mask_lastcopy_antitop])
    else:
        mask_firstcopy = (arrays["GenPart_statusFlags"] & (1 << 12)) != 0
        mask_top = arrays["GenPart_pdgId"] == 6
        mask_antitop = arrays["GenPart_pdgId"] == -6

        mask_lastcopy_top = mask_firstcopy & mask_top
        mask_lastcopy_antitop = mask_firstcopy & mask_antitop

        n_top_per_evt = ak.num(arrays["GenPart_pt"][mask_lastcopy_top])
        n_antitop_per_evt = ak.num(arrays["GenPart_pt"][mask_lastcopy_antitop])

    # chunk 단위라서, 문제 있는 이벤트만 에러 내도 됨
    if ak.any(n_top_per_evt != 1) or ak.any(n_antitop_per_evt != 1):
        bad_top_idx = ak.where(n_top_per_evt != 1)[0]
        bad_atop_idx = ak.where(n_antitop_per_evt != 1)[0]
        raise ValueError(
            f"non-unique top/antitop in events: top bad={bad_top_idx}, antitop bad={bad_atop_idx}"
        )

    top_pt = np.asarray(ak.firsts(arrays["GenPart_pt"][mask_lastcopy_top]))
    top_eta = np.asarray(ak.firsts(arrays["GenPart_eta"][mask_lastcopy_top]))
    top_phi = np.asarray(ak.firsts(arrays["GenPart_phi"][mask_lastcopy_top]))
    top_mass = np.asarray(ak.firsts(arrays["GenPart_mass"][mask_lastcopy_top]))

    atop_pt = np.asarray(ak.firsts(arrays["GenPart_pt"][mask_lastcopy_antitop]))
    atop_eta = np.asarray(ak.firsts(arrays["GenPart_eta"][mask_lastcopy_antitop]))
    atop_phi = np.asarray(ak.firsts(arrays["GenPart_phi"][mask_lastcopy_antitop]))
    atop_mass = np.asarray(ak.firsts(arrays["GenPart_mass"][mask_lastcopy_antitop]))

    t_px, t_py, t_pz, t_E = _ptetaphim_to_pxpypzE(top_pt, top_eta, top_phi, top_mass)
    at_px, at_py, at_pz, at_E = _ptetaphim_to_pxpypzE(atop_pt, atop_eta, atop_phi, atop_mass)

    tt_px = t_px + at_px
    tt_py = t_py + at_py
    tt_pz = t_pz + at_pz
    tt_E = t_E + at_E

    def _rapidity(E, pz):
        num = np.clip(E + pz, 1e-7, None)
        den = np.clip(E - pz, 1e-7, None)
        return 0.5 * np.log(num / den)
    
    


    tt_pt, tt_eta, tt_phi, tt_mass = _pxpypzE_to_ptetaphim(tt_px, tt_py, tt_pz, tt_E)
    
    top_y = _rapidity(t_E, t_pz)
    atop_y  = _rapidity(at_E,  at_pz)
    tt_y  = _rapidity(tt_E,  tt_pz)

    features = np.stack(
        [
            top_pt,
            top_y,
            top_phi,
            top_mass,
            atop_pt,
            atop_y,
            atop_phi,
            atop_mass,
            tt_pt,
            tt_y,
            tt_phi,
            tt_mass,
        ],
        axis=1,
    ).astype(np.float32)

    return features


def make_minnlo_normalizers():
    # 그대로 너가 쓰던 값
    return [
        [
            (3.6520673599656903, 1.0123402362573612, True),
            (0.0001718810581680775, 1.0362455506718102, False),
            (2.8943571877384285e-05, 1.8139038706413384, False),
            (6.21729978047307, 0.2771419580231537, True),
        ],
        [
            (4.595855742518925, 0.7101176940989488, True),
            (0.00022746366634849002, 1.213207643109532, False),
            (-0.00028213870737636996, 1.8136544140703632, False),
            (171.93706459943778, 6.9652037622153, False),
        ],
        [
            (4.5986175957604045, 0.7103218938891299, True),
            (0.00011712322394057398, 1.2076422016031159, False),
            (0.0003628069129526392, 1.8139415747773364, False),
            (171.93691192651536, 6.9500586980501575, False),
        ],
    ]


def normalize_matrix(X: np.ndarray, specs):
    X = X.astype(np.float32, copy=True)
    for j, (mean, std, use_log) in enumerate(specs):
        col = X[:, j]
        if use_log:
            col = np.log(np.clip(col, 1e-6, None))
        col = col - mean
        if std >= 1e-2:
            col = col / std
        X[:, j] = col
    return X


# ----------------------------
# chunk processors
# ----------------------------
def process_common_chunk(chunk: ak.Array, sp: ResultSubProcess):
    sign = ak.where(chunk["genWeight"] > 0, 1, -1).to_numpy()
    scale = chunk["LHEScaleWeight"].to_numpy() if "LHEScaleWeight" in chunk.fields else None
    ps = chunk["PSWeight"].to_numpy() if "PSWeight" in chunk.fields else None
    sp.accumulate_common(sign, scale, ps)


def process_tt_chunk(
    chunk: ak.Array,
    container: ResultMain,
    sessions: Dict[str, Dict[str, Any]],
):
    arrays = chunk
    genTtbarId = arrays["genTtbarId"]

    bb_mask = (genTtbarId % 100 >= 51) & (genTtbarId % 100 <= 55)
    cc_mask = (genTtbarId % 100 >= 41) & (genTtbarId % 100 <= 45)
    jj_mask = ~(bb_mask | cc_mask)

    # 공통 part 먼저
    sign_all = ak.where(arrays["genWeight"] > 0, 1, -1).to_numpy()
    scale_all = arrays["LHEScaleWeight"].to_numpy()
    ps_all = arrays["PSWeight"].to_numpy()

    masks = {
        "BB": ak.to_numpy(bb_mask),
        "CC": ak.to_numpy(cc_mask),
        "JJ": ak.to_numpy(jj_mask),
    }

    # ONNX 입력 만들기
    features_from_lastcopy = create_onnx_input_batch_with_ttbar(arrays, isFirstCopy=False)
    features_from_firstcopy = create_onnx_input_batch_with_ttbar(arrays, isFirstCopy=True)
    features = features_from_firstcopy
    N = features.shape[0]

    # hdamp input
    input_hdamp_up_top = np.stack(
        [
            np.log10(features[:, 0]),
            features[:, 1],
            features[:, 2],
            features[:, 3] / 243.95,
            np.full(N, 0.1, dtype=np.float32),
            np.full(N, 1.379, dtype=np.float32),
        ],
        axis=1,
    )
    input_hdamp_down_top = np.stack(
        [
            np.log10(features[:, 0]),
            features[:, 1],
            features[:, 2],
            features[:, 3] / 243.95,
            np.full(N, 0.1, dtype=np.float32),
            np.full(N, 1.379, dtype=np.float32),
        ],
        axis=1,
    )
    input_hdamp_up_antitop = np.stack(
        [
            np.log10(features[:, 4]),
            features[:, 5],
            features[:, 6],
            features[:, 7] / 243.95,
            np.full(N, 0.2, dtype=np.float32),
            np.full(N, 1.379, dtype=np.float32),
        ],
        axis=1,
    )
    input_hdamp_down_antitop = np.stack(
        [
            np.log10(features[:, 4]),
            features[:, 5],
            features[:, 6],
            features[:, 7] / 243.95,
            np.full(N, 0.2, dtype=np.float32),
            np.full(N, 1.379, dtype=np.float32),
        ],
        axis=1,
    )

    input_hdamp_up = np.stack([input_hdamp_up_top, input_hdamp_up_antitop], axis=1)
    input_hdamp_down = np.stack([input_hdamp_down_top, input_hdamp_down_antitop], axis=1)

    # MiNNLO 입력
    features = features.astype(np.float32, copy=True)
    nrm_array = make_minnlo_normalizers()
    top_feats = normalize_matrix(features[:, 0:4], nrm_array[1])
    antitop_feats = normalize_matrix(features[:, 4:8], nrm_array[2])
    ttbar_feats = normalize_matrix(features[:, 8:12], nrm_array[0])

    input_minnlo_top = np.column_stack(
        [top_feats, 0.6 * np.ones(top_feats.shape[0], dtype=np.float32)]
    )
    input_minnlo_antitop = np.column_stack(
        [antitop_feats, -0.6 * np.ones(antitop_feats.shape[0], dtype=np.float32)]
    )
    input_minnlo_ttbar = np.column_stack(
        [ttbar_feats, np.zeros(ttbar_feats.shape[0], dtype=np.float32)]
    )
    input_minnlo = np.stack([input_minnlo_ttbar, input_minnlo_top, input_minnlo_antitop], axis=1)
    # 추론
    hdamp_up_outputs = sessions["hdamp_up"]["session"].run(
        None, {sessions["hdamp_up"]["input_name"]: input_hdamp_up.astype(np.float32)}
    )[0]
    hdamp_down_outputs = sessions["hdamp_down"]["session"].run(
        None, {sessions["hdamp_down"]["input_name"]: input_hdamp_down.astype(np.float32)}
    )[0]
    minnlo_outputs = sessions["minnlo_up"]["session"].run(
        None, {sessions["minnlo_up"]["input_name"]: input_minnlo.astype(np.float32)}
    )[0]

    hdamp_up_outputs = np.nan_to_num(hdamp_up_outputs, nan=1.0)
    hdamp_down_outputs = np.nan_to_num(hdamp_down_outputs, nan=1.0)
    minnlo_outputs = np.nan_to_num(minnlo_outputs, nan=1.0)

    # tt pT > 1TeV 보호
    mask_tt_pt_gt_1tev = features[:, 8] > 1000.0
    hdamp_up_final = np.where(
        mask_tt_pt_gt_1tev,
        1.0,
        hdamp_up_outputs[:, 0] / hdamp_up_outputs[:, 1],
    )
    hdamp_down_final = np.where(
        mask_tt_pt_gt_1tev,
        1.0,
        hdamp_down_outputs[:, 0] / hdamp_down_outputs[:, 1],
    )
    minnlo_final = np.where(
        mask_tt_pt_gt_1tev,
        1.0,
        minnlo_outputs[:, 1] / minnlo_outputs[:, 0],
    )

    # 서브프로세스별로 누적
    for name, mask_np in masks.items():
        sp = container.get_subprocess(name)
        # 공통
        sp.accumulate_common(sign_all[mask_np], scale_all[mask_np], ps_all[mask_np])
        # TT 전용
        sp.accumulate_tt(
            sign_all[mask_np],
            hdamp_up_final[mask_np],
            hdamp_down_final[mask_np],
            minnlo_final[mask_np],
        )


# ----------------------------
# worker for multiprocess
# ----------------------------
def _worker_process_one_file(args):
    (
        PD,
        filename,
        era,
        is_tt,
        model_paths,
    ) = args

    # 여기서는 "그 파일 하나"만 처리
    if is_tt:
        container = ResultMain(PD=PD, subprocess_names=["BB", "CC", "JJ"])
        # 워커마다 모델을 매번 여는 건 비싸긴 한데, 가장 간단한 버전
        sessions = {}
        for key, path in model_paths.items():
            sess, in_name, out_name = load_onnx_session(path)
            sessions[key] = {
                "session": sess,
                "input_name": in_name,
                "output_name": out_name,
            }

        for chunk in uproot.iterate(
            filename,
            [
                "genWeight",
                "LHEScaleWeight",
                "PSWeight",
                "GenPart_eta",
                "GenPart_mass",
                "GenPart_pdgId",
                "GenPart_statusFlags",
                "GenPart_pt",
                "GenPart_phi",
                "genTtbarId",
            ],
            step_size="50 MB",
        ):
            process_tt_chunk(chunk, container, sessions)
    else:
        container = ResultMain(PD=PD, subprocess_names=None)
        sp = container.get_subprocess(PD)
        for chunk in uproot.iterate(
            filename,
            [
                "genWeight",
                "LHEScaleWeight",
                "PSWeight",
            ],
            step_size="50 MB",
        ):
            process_common_chunk(chunk, sp)

    # 한 파일에 대한 결과만 리턴
    out = {
        "PD": container.PD,
        "era": era,
        "subprocesses": {},
    }
    for name, sp in container.subprocesses_dict.items():
        out["subprocesses"][name] = {
            "nMC": sp.nMC,
            "sumSign": sp.sumSign,
            "sumScaleVariation": sp.sumScaleVariation.tolist()
            if sp.sumScaleVariation is not None else None,
            "sumPSVariation": sp.sumPSVariation.tolist()
            if sp.sumPSVariation is not None else None,
            "sum_hdamp_up": sp.sum_hdamp_up,
            "sum_hdamp_down": sp.sum_hdamp_down,
            "sum_minnlo": sp.sum_minnlo,
        }
    return out

def _worker_process_files(args) -> Dict[str, Any]:
    (
        PD,
        file_list,
        era,
        is_tt,
        model_paths,
    ) = args

    if is_tt:
        container = ResultMain(PD=PD, subprocess_names=["BB", "CC", "JJ"])
        # onnx load
        sessions = {}
        for key, path in model_paths.items():
            sess, in_name, out_name = load_onnx_session(path)
            sessions[key] = {
                "session": sess,
                "input_name": in_name,
                "output_name": out_name,
            }
        for f in file_list:
            for chunk in uproot.iterate(
                f,
                [
                    "genWeight",
                    "LHEScaleWeight",
                    "PSWeight",
                    "GenPart_eta",
                    "GenPart_mass",
                    "GenPart_pdgId",
                    "GenPart_statusFlags",
                    "GenPart_pt",
                    "GenPart_phi",
                    "genTtbarId",
                ],
                step_size="50 MB",
            ):
                process_tt_chunk(chunk, container, sessions)

    else:
        container = ResultMain(PD=PD, subprocess_names=None)
        for f in file_list:
            for chunk in uproot.iterate(
                f,
                [
                    "genWeight",
                    "LHEScaleWeight",
                    "PSWeight",
                ],
                step_size="50 MB",
            ):
                sp = container.get_subprocess(PD)
                process_common_chunk(chunk, sp)

    # 결과를 직렬화 가능한 dict로 반환
    out = {
        "PD": container.PD,
        "era": era,
        "subprocesses": {},
    }
    for name, sp in container.subprocesses_dict.items():
        out["subprocesses"][name] = {
            "nMC": sp.nMC,
            "sumSign": sp.sumSign,
            "sumScaleVariation": sp.sumScaleVariation.tolist()
            if sp.sumScaleVariation is not None
            else None,
            "sumPSVariation": sp.sumPSVariation.tolist() if sp.sumPSVariation is not None else None,
            "sum_hdamp_up": sp.sum_hdamp_up,
            "sum_hdamp_down": sp.sum_hdamp_down,
            "sum_minnlo": sp.sum_minnlo,
        }
    return out


def merge_results(results: List[Dict[str, Any]]) -> Dict[str, Any]:
    """멀티프로세스 결과 합치기"""
    if not results:
        return {}
    base = results[0]
    for other in results[1:]:
        for sp_name, sp_data in other["subprocesses"].items():
            base_sp = base["subprocesses"].setdefault(
                sp_name,
                {
                    "nMC": 0,
                    "sumSign": 0.0,
                    "sumScaleVariation": None,
                    "sumPSVariation": None,
                    "sum_hdamp_up": 0.0,
                    "sum_hdamp_down": 0.0,
                    "sum_minnlo": 0.0,
                },
            )
            base_sp["nMC"] += sp_data["nMC"]
            base_sp["sumSign"] += sp_data["sumSign"]
            # arrays
            for key in ["sumScaleVariation", "sumPSVariation"]:
                if sp_data[key] is None:
                    continue
                if base_sp[key] is None:
                    base_sp[key] = sp_data[key]
                else:
                    base_sp[key] = (np.array(base_sp[key]) + np.array(sp_data[key])).tolist()
            # tt stuff
            for key in ["sum_hdamp_up", "sum_hdamp_down", "sum_minnlo"]:
                base_sp[key] += sp_data[key]
                
            
    return base

def calculate_patch(final_data: Dict[str, Any]) -> Dict[str, Any]:
    """Modelling Patch 계산 추가"""
    for sp_name, sp_data in final_data["subprocesses"].items():
        nMC = sp_data["nMC"]
        sumSign = sp_data["sumSign"]
        if nMC == 0:
            continue
        # 평균값 계산
        sp_data["patch_ScaleVariation"] = (
            list(np.array(sp_data["sumScaleVariation"]) / sumSign)
            if sp_data["sumScaleVariation"] is not None
            else None
        )
        sp_data["patch_PSVariation"] = (
            list(np.array(sp_data["sumPSVariation"]) / sumSign)
            if sp_data["sumPSVariation"] is not None
            else None
        )
        sp_data["patch_hdamp_up"] = sp_data["sum_hdamp_up"] / sumSign
        sp_data["patch_hdamp_down"] = sp_data["sum_hdamp_down"] / sumSign
        sp_data["patch_minnlo"] = sp_data["sum_minnlo"] / sumSign
    return final_data

# ----------------------------
# main logic
# ----------------------------
def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Modelling Patch Tool")
    parser.add_argument("--PD", type=str, help="name of the input PD file")
    parser.add_argument(
        "--output",
        "-o",
        type=str,
        help="name of the output file (json)",
        default=None,
    )
    parser.add_argument("--era", type=str, help="era of the data", default="2024")
    parser.add_argument(
        "--workers",
        type=int,
        default=1,
        help="number of processes to use (1 = no multiprocessing)",
    )
    return parser


def main():
    parser = build_parser()
    args = parser.parse_args()

    PD = args.PD
    era = args.era
    workers = max(1, args.workers)
    file_list = list_files(PD, era)
    rich.print(f"[bold]Files ({len(file_list)}):[/bold] first 5 -> {file_list[:5]} ...")

    is_tt = "TT" in PD

    # 모델 경로는 TT일 때만
    model_paths = {
        "hdamp_up": "mymodel12_hdamp_up_13.6TeV.onnx",
        "hdamp_down": "mymodel12_hdamp_down_13.6TeV.onnx",
        "minnlo_up": "mymodel12_13TeV_MiNNLO_afterShower.onnx",
    }

    if workers == 1:
        # 그냥 순차
        results = []
        # progress bar
        with Progress(
            "[progress.description]{task.description}",
            BarColumn(),
            "{task.completed}/{task.total}",
            TimeElapsedColumn(),
            TimeRemainingColumn(),
        ) as progress:
            task = progress.add_task("Processing files", total=len(file_list))

            if is_tt:
                container = ResultMain(PD=PD, subprocess_names=["BB", "CC", "JJ"])
                sessions = {}
                for key, path in model_paths.items():
                    sess, in_name, out_name = load_onnx_session(path)
                    sessions[key] = {
                        "session": sess,
                        "input_name": in_name,
                        "output_name": out_name,
                    }

                for f in file_list:
                    for chunk in uproot.iterate(
                        f,
                        [
                            "genWeight",
                            "LHEScaleWeight",
                            "PSWeight",
                            "GenPart_eta",
                            "GenPart_mass",
                            "GenPart_pdgId",
                            "GenPart_statusFlags",
                            "GenPart_pt",
                            "GenPart_phi",
                            "genTtbarId",
                        ],
                        step_size="50 MB",
                    ):
                        process_tt_chunk(chunk, container, sessions)
                    progress.advance(task)
                # container -> dict
                final_data = {
                    "PD": container.PD,
                    "era": era,
                    "subprocesses": {},
                }
                for name, sp in container.subprocesses_dict.items():
                    final_data["subprocesses"][name] = {
                        "nMC": sp.nMC,
                        "sumSign": sp.sumSign,
                        "sumScaleVariation": sp.sumScaleVariation.tolist()
                        if sp.sumScaleVariation is not None
                        else None,
                        "sumPSVariation": sp.sumPSVariation.tolist()
                        if sp.sumPSVariation is not None
                        else None,
                        "sum_hdamp_up": sp.sum_hdamp_up,
                        "sum_hdamp_down": sp.sum_hdamp_down,
                        "sum_minnlo": sp.sum_minnlo,
                    }
            else:
                container = ResultMain(PD=PD, subprocess_names=None)
                sp = container.get_subprocess(PD)
                for f in file_list:
                    for chunk in uproot.iterate(
                        f,
                        [
                            "genWeight",
                            "LHEScaleWeight",
                            "PSWeight",
                        ],
                        step_size="50 MB",
                    ):
                        process_common_chunk(chunk, sp)
                    progress.advance(task)
                final_data = {
                    "PD": container.PD,
                    "era": era,
                    "subprocesses": {
                        PD: {
                            "nMC": sp.nMC,
                            "sumSign": sp.sumSign,
                            "sumScaleVariation": sp.sumScaleVariation.tolist()
                            if sp.sumScaleVariation is not None
                            else None,
                            "sumPSVariation": sp.sumPSVariation.tolist()
                            if sp.sumPSVariation is not None
                            else None,
                            "sum_hdamp_up": sp.sum_hdamp_up,
                            "sum_hdamp_down": sp.sum_hdamp_down,
                            "sum_minnlo": sp.sum_minnlo,
                        }
                    },
                }

    else:
        tasks = [
            (PD, fname, era, is_tt, model_paths)
            for fname in file_list
        ]
        results = []
        with Progress(
            "[progress.description]{task.description}",
            BarColumn(),
            "{task.completed}/{task.total}",
            TimeElapsedColumn(),
            TimeRemainingColumn(),
        ) as progress:
            task_id = progress.add_task("Processing files (mp)", total=len(tasks))
            with mp.Pool(processes=workers) as pool:
                for res in pool.imap_unordered(_worker_process_one_file, tasks):
                    results.append(res)
                    progress.advance(task_id)

        final_data = merge_results(results)
        final_data = calculate_patch(final_data)

    # 결과 저장
    out_path = args.output
    if out_path is None:
        out_path = f"{PD}_{era}_summary.json"

    with open(out_path, "w") as f:
        json.dump(final_data, f, indent=2)
    rich.print(f"[bold green]Saved result to {out_path}[/bold green]")


if __name__ == "__main__":
    main()