from __future__ import annotations

import argparse
import json
import math
import sys
from pathlib import Path

from .catalog import inspect_root
from .draft import write_draft
from .runner import finalize_output, run_chunk, run_local
from .schema import load_schema, validate_schema
from .tasks import build_tasks, read_task_manifest, write_task_manifest


def _print_catalog(catalog) -> None:
    counts = catalog.counts
    print(f"Input: {catalog.input_path}")
    print(f"Histograms: TH1={counts['TH1']} TH2={counts['TH2']} unsupported={counts['unsupported']}")
    by_depth: dict[int, int] = {}
    for entry in catalog.entries:
        depth = len(entry.path.split("/"))
        by_depth[depth] = by_depth.get(depth, 0) + 1
    print("Path depths: " + ", ".join(f"{depth}:{count}" for depth, count in sorted(by_depth.items())))
    for entry in catalog.entries:
        print(f"  {entry.kind:3s}  {entry.path}")
    if catalog.unsupported:
        print("Unsupported objects:")
        for item in catalog.unsupported:
            print(f"  {item['classname']:12s}  {item['path']}")


def _print_report(report) -> None:
    print(f"Mapped: {len(report.mapped)}")
    print(f"Uncovered: {len(report.uncovered)}")
    print(f"Duplicate matches: {len(report.duplicates)}")
    for warning in report.warnings:
        print(f"WARNING: {warning}")
    for path in report.uncovered[:30]:
        print(f"UNCOVERED: {path}")
    for path, rules in list(report.duplicates.items())[:30]:
        print(f"DUPLICATE: {path}: {', '.join(rules)}")
    for error in report.errors:
        print(f"ERROR: {error}", file=sys.stderr)


def _prepare(args) -> tuple[dict, Path]:
    catalog = inspect_root(args.input)
    schema = load_schema(args.schema)
    report = validate_schema(catalog, schema, require_confirmed=True)
    _print_report(report)
    if not report.ok:
        raise ValueError("schema validation failed")
    tasks = build_tasks(schema, report)
    output = Path(args.output).expanduser().resolve()
    output.mkdir(parents=True, exist_ok=True)
    task_manifest = output / "tasks.json"
    formats = [item.strip().lower() for item in args.formats.split(",") if item.strip()]
    if not formats or set(formats) - {"png", "pdf"}:
        raise ValueError("--formats must contain png and/or pdf")
    manifest = write_task_manifest(
        task_manifest,
        catalog.input_path,
        str(output),
        str(Path(args.schema).expanduser().resolve()),
        tasks,
        schema.metadata,
        schema.styles,
        catalog.fingerprint,
        formats,
    )
    print(f"Prepared {len(tasks)} task(s): {task_manifest}")
    return manifest, task_manifest


def command_inspect(args) -> int:
    catalog = inspect_root(args.input)
    _print_catalog(catalog)
    if args.schema_out:
        write_draft(catalog, args.schema_out)
        print(f"Draft schema written to {args.schema_out}")
    if args.json_out:
        Path(args.json_out).write_text(json.dumps(catalog.to_dict(), indent=2), encoding="utf-8")
    return 0


def command_validate(args) -> int:
    catalog = inspect_root(args.input)
    schema = load_schema(args.schema)
    report = validate_schema(catalog, schema, require_confirmed=args.confirmed)
    _print_report(report)
    return 0 if report.ok else 2


def command_plot(args) -> int:
    manifest, _ = _prepare(args)
    records = run_local(manifest, workers=args.workers)
    print(f"Rendered {len(records)} plot(s) in {manifest['output']}")
    return 0


def command_submit(args) -> int:
    if args.chunk_size < 1:
        raise ValueError("--chunk-size must be positive")
    manifest, task_manifest = _prepare(args)
    try:
        import htcondor
    except ModuleNotFoundError as error:
        raise RuntimeError("HTCondor Python bindings are required for submit") from error
    chunks = math.ceil(len(manifest["tasks"]) / args.chunk_size)
    if chunks == 0:
        finalize_output(manifest["output"], manifest)
        print("No enabled plotting tasks")
        return 0
    log_dir = Path(manifest["output"]) / "logs"
    log_dir.mkdir(parents=True, exist_ok=True)
    executable = str(Path(sys.argv[0]).resolve())
    submit = htcondor.Submit(
        {
            "batch_name": f"SKPlot_{Path(args.input).stem}",
            "executable": executable,
            "arguments": f"worker --manifest {task_manifest} --chunk-index $(Process) --chunk-size {args.chunk_size}",
            "request_cpus": "1",
            "request_memory": args.memory,
            "request_disk": "1GB",
            "universe": "vanilla",
            "getenv": "True",
            "output": str(log_dir / "$(Process).out"),
            "error": str(log_dir / "$(Process).err"),
            "log": str(log_dir / "$(Process).log"),
        }
    )
    schedd = htcondor.Schedd()
    with schedd.transaction() as transaction:
        submit.queue(transaction, count=chunks)
    print(f"Submitted {chunks} chunk job(s) for {len(manifest['tasks'])} tasks")
    return 0


def command_worker(args) -> int:
    manifest = read_task_manifest(args.manifest)
    records = run_chunk(manifest, args.chunk_index, args.chunk_size)
    print(f"Rendered {len(records)} task(s) in chunk {args.chunk_index}")
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Schema-driven ROOT TH1/TH2 plotter")
    subparsers = parser.add_subparsers(dest="command", required=True)

    inspect_parser = subparsers.add_parser("inspect", help="catalog a ROOT file and optionally create a draft schema")
    inspect_parser.add_argument("input")
    inspect_parser.add_argument("--schema-out")
    inspect_parser.add_argument("--json-out")
    inspect_parser.set_defaults(func=command_inspect)

    validate_parser = subparsers.add_parser("validate", help="validate a schema against a ROOT file")
    validate_parser.add_argument("input")
    validate_parser.add_argument("--schema", required=True)
    validate_parser.add_argument("--confirmed", action="store_true")
    validate_parser.set_defaults(func=command_validate)

    def add_render_arguments(command_parser):
        command_parser.add_argument("input")
        command_parser.add_argument("--schema", required=True)
        command_parser.add_argument("--output", required=True)
        command_parser.add_argument("--formats", default="png,pdf")

    plot_parser = subparsers.add_parser("plot", help="render a confirmed schema locally")
    add_render_arguments(plot_parser)
    plot_parser.add_argument("--workers", type=int, default=0)
    plot_parser.set_defaults(func=command_plot)

    submit_parser = subparsers.add_parser("submit", help="submit rendering tasks to HTCondor")
    add_render_arguments(submit_parser)
    submit_parser.add_argument("--chunk-size", type=int, default=20)
    submit_parser.add_argument("--memory", default="2GB")
    submit_parser.set_defaults(func=command_submit)

    worker_parser = subparsers.add_parser("worker", help="internal HTCondor worker")
    worker_parser.add_argument("--manifest", required=True)
    worker_parser.add_argument("--chunk-index", type=int, required=True)
    worker_parser.add_argument("--chunk-size", type=int, required=True)
    worker_parser.set_defaults(func=command_worker)
    return parser


def main(argv: list[str] | None = None) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)
    try:
        return int(args.func(args))
    except (FileNotFoundError, ValueError, RuntimeError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        return 2
