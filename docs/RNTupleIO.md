# RNTuple I/O

[Documentation index](README.md)

SKNanoAnalyzer uses ROOT 6.40 RNTuple input exclusively. `SKNanoLoader` opens
the `Events` RNTuple in each input file, and the generated `BranchScalar`,
`BranchVector`, collection views, and lazy activation APIs bind directly to
RNTuple fields.

The framework code generator consumes only the canonical NanoAOD schema.
External modules bind custom scalar and vector fields through typed runtime
handles and provide their own event-scoped row views.

## Run an analyzer

```bash
SKNano.py -a ExampleRun -i 'MyRNTupleSample*' -e 2024
```

Generated jobs call `GetInputEntries()`, so reduction works across multi-file
RNTuple inputs. A direct C++ setup is equivalent:

```cpp
ExampleRun module;
module.SetRNTupleName("Events");
module.AddFile("input.rntuple.root");
```

All files in one job must use a compatible RNTuple schema. Multi-file jobs
switch readers at file boundaries while preserving lazy field state.

## I/O behavior

- Scalar and vector views are created lazily on first access.
- Arithmetic `RVec<T>` fields expose page-backed contiguous views without an
  intermediate analyzer buffer copy.
- `RVec<bool>` is materialized as bytes because it has no stable `bool *`
  representation compatible with the existing view contract.
- ROOT's RNTuple cluster cache is enabled.
- Analyzer datasets booked through `Output().Book()` are written as RNTuples.
  Histograms remain ordinary ROOT histogram objects in the same file.
- Analyzer RNTuple output uses LZ4 level 4, 64 MiB target clusters, buffered
  writes, page checksums, and bounded compression parallelism from `--ncpu`.
  The sparse multi-dataset profile uses a 4 MiB buffer budget per dataset.
- Batch merging goes through `scripts/sknano_merge.py`. It checks every shard's
  RNTuple and histogram schemas, explicitly stages `hadd -fk404` in batches of
  at most 100 inputs, and fully deserializes every intermediate file before it
  can feed the next stage. It then verifies merged dataset entries, histogram
  entries, and compression, atomically publishes the target, and deletes
  shards only after all checks pass. Staged and parallel partials are placed
  under the output filesystem (or `--temp-dir`) so a large merge cannot
  silently exhaust the node-local `/tmp` filesystem.
- Skimming records selected global entries during analysis and writes the
  original input schema through the RNTuple Snapshot backend.
Set `SKNANO_PERFORMANCE_REPORT=/path/report.json` to collect backend-tagged
event-loop telemetry. The standalone `test_rntuple_source` target validates
scalar and vector access against an RNTuple file.
