# RNTuple I/O

SKNanoAnalyzer uses ROOT 6.40 RNTuple input by default without changing
analyzer code. `SKNanoLoader` opens the `Events` RNTuple in each input file,
and the generated `BranchScalar`, `BranchVector`, collection views, and lazy
activation APIs also support explicitly selected legacy TTree input.

## Convert a NanoAOD file

After `source setup.sh` and a normal build, run:

```bash
sknano-convert-rntuple input.root output.rntuple.root \
  --compression-level 5 \
  --cluster-size-mib 64 \
  --page-buffer-mib 256
```

The converter writes `output.rntuple.root.partial`, verifies the entry count
and descriptor, then atomically renames it. It never modifies the TTree input.
Page checksums and buffered writes are enabled. Use `--max-entries N` for a
tuning sample and `--force` for an atomic replacement of an existing output.

## Run an analyzer

RNTuple input is the default:

```bash
SKNano.py -a ExampleRun -i 'MyRNTupleSample*' -e 2024 \
  --input-format rntuple
```

Use `--input-format auto` or `--input-format ttree` for legacy TTree files.
Generated jobs call `GetInputEntries()`, so reduction works for both backends.
A direct C++ setup is equivalent:

```cpp
ExampleRun module;
module.SetInputFormat("rntuple"); // framework default
module.SetTreeName("Events");
module.AddFile("input.rntuple.root");
```

All files in one job must use the same object format and schema. Multi-file
RNTuple jobs switch readers at file boundaries while preserving lazy branch
state. Event-block mode currently remains TTree-only.

## I/O behavior

- Scalar and vector views are created lazily on first access.
- Arithmetic `RVec<T>` fields expose page-backed contiguous views without an
  intermediate analyzer buffer copy.
- `RVec<bool>` is materialized as bytes because it has no stable `bool *`
  representation compatible with the existing view contract.
- ROOT's RNTuple cluster cache is enabled; TTree cache tuning is bypassed for
  RNTuple input.
- Analyzer datasets booked through `Output().Book()` are written as RNTuples.
  Histograms remain ordinary ROOT histogram objects in the same file.
- Analyzer RNTuple output uses LZ4 level 4, 64 MiB target clusters, buffered
  writes, page checksums, and bounded compression parallelism from `--ncpu`.
  The sparse multi-dataset profile uses a 4 MiB buffer budget per dataset.
- Batch merging goes through `scripts/sknano_merge.py`. It checks every shard's
  RNTuple and histogram schemas, merges with `hadd -fk404`, verifies merged
  dataset entries, histogram entries, and compression, atomically publishes
  the target, and deletes shards only after all checks pass. Parallel `hadd`
  partials are placed under the output filesystem (or `--temp-dir`) so a large
  merge cannot silently exhaust the node-local `/tmp` filesystem.
- Skimming records selected global entries during analysis and writes the
  original input schema through the RNTuple Snapshot backend; it works for both
  default RNTuple input and explicitly selected legacy TTree input.
Set `SKNANO_PERFORMANCE_REPORT=/path/report.json` to collect backend-tagged
event-loop telemetry. The standalone `test_rntuple_source` target validates
scalar and vector access against a converted file.
