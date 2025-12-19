from __future__ import annotations

import os
import subprocess
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, Iterable, List, Mapping, Sequence

from rich.console import Console

from .config import ModelConfig

try:
    import onnxruntime as ort
except ImportError:  # pragma: no cover - handled at runtime
    ort = None


class OnnxRuntimeUnavailable(RuntimeError):
    pass


@dataclass
class ProviderInfo:
    available: Sequence[str]
    requested: Sequence[str]
    provider_options: Sequence[Mapping[str, object]]
    using_cuda: bool
    cuda_device: int | None


class OnnxRunner:
    """
    Cache and run ONNX models per fold with provider fallback.
    """

    def __init__(self, model_cfg: ModelConfig, console: Console, threads: int = 1):
        if ort is None:
            raise OnnxRuntimeUnavailable(
                "onnxruntime is not installed. Install onnxruntime or "
                "onnxruntime-gpu in the HTCondor worker environment."
            )

        self.model_cfg = model_cfg
        self.console = console
        self.threads = max(1, int(threads))
        self.provider_info = self._select_providers(model_cfg.prefer_cuda)
        self._class_sessions: Dict[int, ort.InferenceSession] = {}
        self._reco_sessions: Dict[int, ort.InferenceSession] = {}
        self._loaded_models: List[Path] = []

    def _parse_visible_devices(self) -> List[int] | None:
        visible = os.environ.get("CUDA_VISIBLE_DEVICES")
        if not visible:
            return None
        devices: List[int] = []
        for part in visible.split(","):
            part = part.strip()
            if not part:
                continue
            try:
                devices.append(int(part))
            except ValueError:
                return None
        return devices or None

    def _query_gpu_free_memory(self, candidates: Sequence[int] | None) -> List[tuple[int, float]]:
        cmd = ["nvidia-smi"]
        if candidates:
            cmd += ["-i", ",".join(str(i) for i in candidates)]
        cmd += ["--query-gpu=index,memory.free", "--format=csv,noheader,nounits"]
        try:
            result = subprocess.run(
                cmd,
                check=True,
                capture_output=True,
                text=True,
                timeout=2,
            )
        except Exception:
            return []
        entries = []
        for line in result.stdout.splitlines():
            line = line.strip()
            if not line:
                continue
            parts = [p.strip() for p in line.split(",")]
            if len(parts) < 2:
                continue
            try:
                idx = int(parts[0])
                free = float(parts[1])
            except ValueError:
                continue
            entries.append((idx, free))
        return entries

    def _pick_cuda_device(self) -> int:
        visible = self._parse_visible_devices()
        free_mem = self._query_gpu_free_memory(visible)
        if free_mem:
            best_physical, best_free = max(free_mem, key=lambda x: (x[1], -x[0]))
            if visible is not None:
                logical_map = {phys: i for i, phys in enumerate(visible)}
                if best_physical in logical_map:
                    logical_id = logical_map[best_physical]
                else:
                    logical_id = 0
            else:
                logical_id = best_physical
            self.console.log(
                f"[cyan]CUDA auto-select:[/] device {logical_id} "
                f"(physical {best_physical}, ~{best_free:.0f} MiB free)"
            )
            return int(logical_id)
        # Fallback: first visible device if set, else GPU 0.
        if visible:
            self.console.log("[yellow]CUDA auto-select fell back to first visible GPU.[/]")
            return 0
        self.console.log("[yellow]CUDA auto-select fell back to GPU 0 (nvidia-smi unavailable).[/]")
        return 0

    def _select_providers(self, prefer_cuda: bool) -> ProviderInfo:
        available = ort.get_available_providers()
        requested: List[str] = []
        provider_options: List[Mapping[str, object]] = []
        using_cuda = False
        cuda_device: int | None = None
        if prefer_cuda and "CUDAExecutionProvider" in available:
            cuda_device = self._pick_cuda_device()
            requested.append("CUDAExecutionProvider")
            provider_options.append({"device_id": cuda_device})
            using_cuda = True
        requested.append("CPUExecutionProvider")
        provider_options.append({})
        return ProviderInfo(
            available=available,
            requested=requested,
            provider_options=provider_options,
            using_cuda=using_cuda,
            cuda_device=cuda_device,
        )

    def _make_session(self, model_path: Path) -> ort.InferenceSession:
        if not model_path.exists():
            raise FileNotFoundError(f"ONNX model not found: {model_path}")
        session_opts = ort.SessionOptions()
        session_opts.enable_mem_pattern = False
        session_opts.enable_cpu_mem_arena = True
        # Constrain threading to avoid exceeding Condor CPU allocations
        session_opts.intra_op_num_threads = self.threads
        session_opts.inter_op_num_threads = 1

        try:
            session = ort.InferenceSession(
                str(model_path),
                providers=list(self.provider_info.requested),
                provider_options=list(self.provider_info.provider_options),
                sess_options=session_opts,
            )
            self._loaded_models.append(model_path)
            return session
        except Exception as e:
            if self.provider_info.using_cuda:
                self.console.log(
                    f"[yellow]CUDAExecutionProvider failed for {model_path.name}; retrying on CPU.[/]"
                )
                try:
                    session = ort.InferenceSession(
                        str(model_path),
                        providers=["CPUExecutionProvider"],
                        provider_options=[{}],
                        sess_options=session_opts,
                    )
                    self._loaded_models.append(model_path)
                    return session
                except Exception as retry_exc:  # pragma: no cover - runtime safety
                    raise RuntimeError(
                        f"Failed to load ONNX model {model_path} "
                        f"with CPU fallback: {retry_exc}"
                    ) from retry_exc
            raise RuntimeError(f"Failed to load ONNX model {model_path}: {e}") from e

    def _get_class_session(self, fold: int) -> ort.InferenceSession:
        if fold not in self._class_sessions:
            path = self.model_cfg.model_dir / self.model_cfg.class_pattern.format(fold=fold)
            self._class_sessions[fold] = self._make_session(path)
            self.console.log(f"[green]Loaded CLASSIF model:[/] {path}")
        return self._class_sessions[fold]

    def _get_reco_session(self, fold: int) -> ort.InferenceSession:
        if fold not in self._reco_sessions:
            path = self.model_cfg.model_dir / self.model_cfg.reco_pattern.format(fold=fold)
            self._reco_sessions[fold] = self._make_session(path)
            self.console.log(f"[green]Loaded RECO model:[/] {path}")
        return self._reco_sessions[fold]

    def _run_session(
        self, session: ort.InferenceSession, feeds: Mapping[str, object]
    ) -> Dict[str, object]:
        outputs = session.run(None, feeds)
        names = [o.name for o in session.get_outputs()]
        return {name: out for name, out in zip(names, outputs)}

    def run_spanet(
        self,
        fold: int,
        feeds: Mapping[str, object],
    ) -> Dict[str, Dict[str, object]]:
        class_sess = self._get_class_session(fold)
        reco_sess = self._get_reco_session(fold)
        return {
            "class": self._run_session(class_sess, feeds),
            "reco": self._run_session(reco_sess, feeds),
        }

    @property
    def loaded_models(self) -> List[Path]:
        return list(self._loaded_models)
