from __future__ import annotations

import hashlib
import json
from dataclasses import asdict, dataclass, field
from pathlib import Path
from typing import Any


SUPPORTED_PREFIXES = ("TH1", "TH2")


@dataclass(frozen=True)
class AxisInfo:
    title: str
    edges: tuple[float, ...]
    labels: tuple[str, ...] | None = None


@dataclass(frozen=True)
class CatalogEntry:
    path: str
    classname: str
    ndim: int
    title: str
    axes: tuple[AxisInfo, ...]

    @property
    def kind(self) -> str:
        return f"TH{self.ndim}"


@dataclass
class Catalog:
    input_path: str
    entries: list[CatalogEntry]
    unsupported: list[dict[str, str]] = field(default_factory=list)
    fingerprint: dict[str, Any] = field(default_factory=dict)

    def to_dict(self) -> dict[str, Any]:
        return {
            "input_path": self.input_path,
            "entries": [asdict(entry) for entry in self.entries],
            "unsupported": self.unsupported,
            "fingerprint": self.fingerprint,
        }

    @property
    def counts(self) -> dict[str, int]:
        return {
            "TH1": sum(entry.ndim == 1 for entry in self.entries),
            "TH2": sum(entry.ndim == 2 for entry in self.entries),
            "unsupported": len(self.unsupported),
        }


def _axis_info(axis: Any) -> AxisInfo:
    try:
        title = str(axis.member("fTitle") or "")
    except Exception:
        title = ""
    try:
        labels = axis.labels()
        label_tuple = tuple(str(label) for label in labels) if labels else None
    except Exception:
        label_tuple = None
    return AxisInfo(
        title=title,
        edges=tuple(float(value) for value in axis.edges()),
        labels=label_tuple,
    )


def inspect_root(path: str | Path) -> Catalog:
    try:
        import uproot
    except ModuleNotFoundError as error:
        raise RuntimeError("uproot is required: install the plotting dependencies") from error

    root_path = Path(path).expanduser().resolve()
    if not root_path.is_file():
        raise FileNotFoundError(f"ROOT input is not a file: {root_path}")

    entries: list[CatalogEntry] = []
    unsupported: list[dict[str, str]] = []
    with uproot.open(root_path) as root_file:
        latest: dict[str, tuple[int, str, str]] = {}
        for cycle_path, classname in root_file.classnames(recursive=True).items():
            path_without_cycle, separator, cycle_text = cycle_path.rpartition(";")
            if not separator:
                path_without_cycle, cycle = cycle_path, 0
            else:
                try:
                    cycle = int(cycle_text)
                except ValueError:
                    cycle = 0
            if path_without_cycle not in latest or cycle > latest[path_without_cycle][0]:
                latest[path_without_cycle] = (cycle, cycle_path, classname)
        for path_without_cycle, (_, cycle_path, classname) in sorted(latest.items()):
            if classname.startswith("TDirectory"):
                continue
            if not classname.startswith(SUPPORTED_PREFIXES):
                unsupported.append({"path": path_without_cycle, "classname": classname})
                continue
            obj = root_file[cycle_path]
            ndim = len(obj.axes)
            if ndim not in (1, 2):
                unsupported.append({"path": path_without_cycle, "classname": classname})
                continue
            entries.append(
                CatalogEntry(
                    path=path_without_cycle,
                    classname=classname,
                    ndim=ndim,
                    title=str(getattr(obj, "title", "") or ""),
                    axes=tuple(_axis_info(axis) for axis in obj.axes),
                )
            )

    stat = root_path.stat()
    digest_input = json.dumps(
        [(entry.path, entry.classname, [axis.edges for axis in entry.axes]) for entry in entries],
        sort_keys=True,
    ).encode("utf-8")
    fingerprint = {
        "size": stat.st_size,
        "mtime_ns": stat.st_mtime_ns,
        "catalog_sha256": hashlib.sha256(digest_input).hexdigest(),
    }
    return Catalog(str(root_path), entries, unsupported, fingerprint)
