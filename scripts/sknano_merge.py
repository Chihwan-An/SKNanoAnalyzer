#!/usr/bin/env python3
"""Validated, atomic merger for SKNano ROOT/RNTuple output shards."""

from __future__ import annotations

import argparse
import math
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import uuid
from typing import NamedTuple


class Dataset(NamedTuple):
    entries: int
    schema: tuple[tuple[str, str], ...]


class Histogram(NamedTuple):
    class_name: str
    dimension: int
    cells: int
    entries: float


def _root_module():
    try:
        import ROOT  # type: ignore
    except ImportError as error:
        raise RuntimeError("PyROOT is required to validate merge inputs") from error
    ROOT.gROOT.SetBatch(True)
    return ROOT


def _rntuple_names(root, file_path: Path) -> list[str]:
    root_file = root.TFile.Open(str(file_path), "READ")
    if not root_file or root_file.IsZombie() or root_file.TestBit(root.TFile.kRecovered):
        raise RuntimeError(f"invalid or recovered ROOT file: {file_path}")
    names: list[str] = []

    def visit(directory, prefix: str = "") -> None:
        for key in directory.GetListOfKeys():
            name = str(key.GetName())
            path = f"{prefix}/{name}" if prefix else name
            class_name = str(key.GetClassName())
            if "RNTuple" in class_name:
                names.append(path)
                continue
            klass = root.TClass.GetClass(class_name)
            if klass and klass.InheritsFrom(root.TDirectory.Class()):
                child = directory.GetDirectory(name)
                if child:
                    visit(child, path)

    visit(root_file)
    root_file.Close()
    return sorted(names)


def inspect_file(root, file_path: Path) -> dict[str, Dataset]:
    result: dict[str, Dataset] = {}
    for name in _rntuple_names(root, file_path):
        reader = root.RNTupleReader.Open(name, str(file_path))
        descriptor = reader.GetDescriptor()
        schema = []
        def add_field(field) -> None:
            field_id = field.GetId()
            schema.append(
                (str(descriptor.GetQualifiedFieldName(field_id)),
                 str(field.GetTypeName()))
            )
            for child in descriptor.GetFieldIterable(field):
                add_field(child)

        for field in descriptor.GetTopLevelFields():
            add_field(field)
        result[name] = Dataset(int(reader.GetNEntries()), tuple(sorted(schema)))
    return result


def inspect_histograms(root, file_path: Path) -> dict[str, Histogram]:
    root_file = root.TFile.Open(str(file_path), "READ")
    if not root_file or root_file.IsZombie() or root_file.TestBit(root.TFile.kRecovered):
        raise RuntimeError(f"invalid or recovered ROOT file: {file_path}")
    result: dict[str, Histogram] = {}

    def visit(directory, prefix: str = "") -> None:
        for key in directory.GetListOfKeys():
            name = str(key.GetName())
            path = f"{prefix}/{name}" if prefix else name
            class_name = str(key.GetClassName())
            klass = root.TClass.GetClass(class_name)
            if klass and klass.InheritsFrom(root.TDirectory.Class()):
                child = directory.GetDirectory(name)
                if child:
                    visit(child, path)
                continue
            if not klass or not klass.InheritsFrom(root.TH1.Class()):
                continue
            histogram = key.ReadObj()
            if not histogram or not histogram.InheritsFrom(root.TH1.Class()):
                raise RuntimeError(
                    f"cannot read histogram {path} from {file_path}"
                )
            result[path] = Histogram(
                class_name,
                int(histogram.GetDimension()),
                int(histogram.GetNcells()),
                float(histogram.GetEntries()),
            )

    visit(root_file)
    root_file.Close()
    return result


def validate_inputs(
    root, inputs: list[Path]
) -> tuple[
    dict[str, tuple[tuple[str, str], ...]],
    dict[str, int],
    dict[str, tuple[str, int, int]],
    dict[str, float],
]:
    reference: dict[str, tuple[tuple[str, str], ...]] | None = None
    histogram_reference: dict[str, tuple[str, int, int]] | None = None
    totals: dict[str, int] = {}
    histogram_totals: dict[str, float] = {}
    for path in inputs:
        datasets = inspect_file(root, path)
        schemas = {name: data.schema for name, data in datasets.items()}
        if reference is None:
            reference = {}
        changed = sorted(
            name for name in set(reference) & set(schemas)
            if reference[name] != schemas[name]
        )
        if changed:
            raise RuntimeError(
                f"RNTuple schema mismatch in {path}: "
                f"changed={changed}"
            )
        reference.update(schemas)
        for name, data in datasets.items():
            totals[name] = totals.get(name, 0) + data.entries

        histograms = inspect_histograms(root, path)
        histogram_schemas = {
            name: (hist.class_name, hist.dimension, hist.cells)
            for name, hist in histograms.items()
        }
        if histogram_reference is None:
            histogram_reference = {}
        changed = sorted(
            name
            for name in set(histogram_reference) & set(histogram_schemas)
            if histogram_reference[name] != histogram_schemas[name]
        )
        if changed:
            raise RuntimeError(
                f"histogram schema mismatch in {path}: "
                f"changed={changed}"
            )
        histogram_reference.update(histogram_schemas)
        for name, histogram in histograms.items():
            histogram_totals[name] = (
                histogram_totals.get(name, 0.0) + histogram.entries
            )
    return reference or {}, totals, histogram_reference or {}, histogram_totals


def _run_hadd(
    output: Path,
    inputs: list[Path],
    jobs: int,
    temp_dir: Path,
) -> None:
    """Run one bounded hadd invocation.

    The caller is responsible for keeping ``inputs`` below the safe batch
    size.  Do not use hadd's ``-n`` option here: ROOT 6.40.02 produced corrupt
    histogram keys when its internal excess-file path was exercised.
    """
    command = ["hadd", "-fk404", "-v", "0"]
    workers = min(jobs, len(inputs))
    # Passing ``-j 1`` still selects hadd's parallel merge path.  Omit -j for
    # a genuinely sequential merge.
    if workers > 1:
        command.extend(["-j", str(workers), "-d", str(temp_dir)])
    command.append(str(output))
    command.extend(str(path) for path in inputs)
    environment = os.environ.copy()
    environment["TMPDIR"] = str(temp_dir)
    subprocess.run(command, check=True, env=environment)


def _validate_intermediate(root, path: Path) -> None:
    """Fully deserialize mergeable objects before using a staged partial."""
    inspect_file(root, path)
    inspect_histograms(root, path)
    root_file = root.TFile.Open(str(path), "READ")
    if not root_file or root_file.IsZombie():
        raise RuntimeError(f"intermediate ROOT file cannot be reopened: {path}")
    compression = int(root_file.GetCompressionSettings())
    root_file.Close()
    if compression != 404:
        raise RuntimeError(
            f"intermediate file compression is {compression}, "
            f"expected LZ4 level 4 (404): {path}"
        )


def _staged_hadd(
    root,
    inputs: list[Path],
    output: Path,
    jobs: int,
    batch_size: int,
    temp_dir: Path,
) -> None:
    """Merge through explicitly bounded and validated intermediate files."""
    current = inputs
    stage = 0
    while len(current) > batch_size:
        next_stage: list[Path] = []
        batches = [
            current[start:start + batch_size]
            for start in range(0, len(current), batch_size)
        ]
        print(
            f"Merge stage {stage + 1}: {len(current)} inputs -> "
            f"{len(batches)} validated partials",
            flush=True,
        )
        for batch_index, batch in enumerate(batches):
            partial = temp_dir / f"stage_{stage:02d}_{batch_index:05d}.root"
            _run_hadd(partial, batch, jobs, temp_dir)
            _validate_intermediate(root, partial)
            next_stage.append(partial)
        current = next_stage
        stage += 1

    print(
        f"Final merge stage: {len(current)} inputs -> {output}",
        flush=True,
    )
    _run_hadd(output, current, jobs, temp_dir)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument(
        "--jobs",
        type=int,
        default=1,
        help="hadd worker count (default: 1; parallel RNTuple merge is experimental)",
    )
    parser.add_argument(
        "--batch-size",
        type=int,
        default=100,
        help=(
            "maximum inputs per explicit hadd stage (default: 100; "
            "safe maximum: 100)"
        ),
    )
    parser.add_argument(
        "--temp-dir",
        type=Path,
        help=(
            "directory for staged and parallel partials "
            "(default: output directory)"
        ),
    )
    parser.add_argument("--delete-inputs", action="store_true")
    parser.add_argument("inputs", nargs="+", type=Path)
    args = parser.parse_args()

    if args.jobs < 1:
        parser.error("--jobs must be positive")
    if not 2 <= args.batch_size <= 100:
        parser.error("--batch-size must be between 2 and 100")
    inputs = [path.resolve() for path in args.inputs]
    if len(set(inputs)) != len(inputs):
        parser.error("duplicate input paths are not allowed")
    output = args.output.resolve()
    if output in inputs:
        parser.error("output must not also be an input")
    for path in inputs:
        if not path.is_file():
            parser.error(f"input does not exist: {path}")

    root = _root_module()
    output.parent.mkdir(parents=True, exist_ok=True)
    temp_parent = (args.temp_dir or output.parent).resolve()
    temp_parent.mkdir(parents=True, exist_ok=True)
    if len(inputs) > args.batch_size or args.jobs > 1:
        input_bytes = sum(path.stat().st_size for path in inputs)
        # Staged and parallel merges can keep an intermediate generation while
        # assembling the next one, so budget for both generations at once.
        required = 2 * input_bytes
        available = shutil.disk_usage(temp_parent).free
        if available < required:
            raise RuntimeError(
                f"merge temporary directory {temp_parent} has "
                f"{available} bytes free but needs about {required} bytes "
                f"for {input_bytes} bytes of input shards"
            )

    schemas, totals, histogram_schemas, histogram_totals = validate_inputs(
        root, inputs
    )
    partial = output.with_name(
        f".{output.name}.partial.{uuid.uuid4().hex}"
    )

    try:
        merge_temp = Path(tempfile.mkdtemp(
            prefix=f".{output.name}.hadd.",
            dir=temp_parent,
        ))
        try:
            _staged_hadd(
                root,
                inputs,
                partial,
                args.jobs,
                args.batch_size,
                merge_temp,
            )
        finally:
            # NFS can briefly retain an entry after hadd has closed and
            # unlinked it.  Cleanup must not invalidate a complete merge; the
            # output is validated below before publication or input deletion.
            shutil.rmtree(merge_temp, ignore_errors=True)
        merged = inspect_file(root, partial)
        if {name: data.schema for name, data in merged.items()} != schemas:
            raise RuntimeError("merged RNTuple schema differs from the inputs")
        merged_counts = {name: data.entries for name, data in merged.items()}
        if merged_counts != totals:
            raise RuntimeError(
                f"merged RNTuple entry counts differ: expected={totals}, "
                f"actual={merged_counts}"
            )
        merged_histograms = inspect_histograms(root, partial)
        merged_histogram_schemas = {
            name: (hist.class_name, hist.dimension, hist.cells)
            for name, hist in merged_histograms.items()
        }
        if merged_histogram_schemas != histogram_schemas:
            raise RuntimeError("merged histogram schema differs from the inputs")
        for name, expected_entries in histogram_totals.items():
            actual_entries = merged_histograms[name].entries
            if not math.isclose(
                actual_entries, expected_entries, rel_tol=1.0e-12, abs_tol=1.0e-9
            ):
                raise RuntimeError(
                    f"merged histogram entry count differs for {name}: "
                    f"expected={expected_entries}, actual={actual_entries}"
                )
        check_file = root.TFile.Open(str(partial), "READ")
        if not check_file or check_file.IsZombie():
            raise RuntimeError("merged ROOT file cannot be reopened")
        compression = int(check_file.GetCompressionSettings())
        check_file.Close()
        if compression != 404:
            raise RuntimeError(
                f"merged file compression is {compression}, expected LZ4 level 4 (404)"
            )
        os.replace(partial, output)
    except Exception:
        if partial.exists():
            partial.unlink()
        raise

    if args.delete_inputs:
        for path in inputs:
            path.unlink()
    print(f"Merged {len(inputs)} shards into {output}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as error:
        print(f"sknano-merge: {error}", file=sys.stderr)
        raise SystemExit(1)
