# Typed RNTuple output

`AnalyzerCore` output is RNTuple-only. New datasets are created through the
output registry; there is no TTree output compatibility layer.

## Stable zero-copy buffers

Book once in `initializeAnalyzer()` and bind analyzer-owned buffers:

```cpp
output_ = Output().Book("Events");
output_.Field("run", row_.run)
       .Field("Jet_pt", row_.jetPt)
       .Field("Hadron_pdgId", row_.hadronPdgId);
```

Assign those buffers and call `output_.Fill()` for each row. Arithmetic
scalars, `std::array<T, N>`, and `std::vector<T>` are supported without a
per-event copy into a second framework buffer. A field cannot be rebound to a
different address or type after booking.

For buffers whose lifetime should be owned by the output registry:

```cpp
auto output = Output().Book("Training");
auto weight = output.MakeField<float>("weight");
auto category = output.MakeField<int>("category");

weight = eventWeight;
category = eventCategory;
output.Fill();
```

`Output().Get("Training")` retrieves an existing dataset. Its scalar `Set()`
helpers remain useful for dynamic legacy analysis code; they register storage
once and only update the stable slot on later events. Cached `OutputField<T>`
handles are preferred for new hot loops.

The default `Fast` profile uses LZ4 level 4, 64 MiB target clusters, buffered
writes, page checksums, and compression workers configured from the job's
`--ncpu`. Analyses that book many mostly sparse datasets can use:

```cpp
Output().Book(name, AnalyzerCore::RNTupleOutputProfile::Sparse);
```

The sparse profile reduces each dataset's page/cluster buffer budget to 4 MiB.
Datasets whose dynamic schema is never used are omitted from that shard; the
validated merger permits missing datasets while requiring identical schemas
where a dataset is present. This keeps empty categories mergeable without
inventing placeholder fields.

## Histogram groups

Use a group to define a common output directory and book fixed histograms once:

```cpp
const auto histograms = Hists("MyAnalyzer");
pt_ = histograms.Book1D("Object/Pt", 50, 0., 500.);
pt_.Fill(pt, weight);
```

For names determined dynamically by a systematic or category, use
`HistogramGroup::Fill`. The registry rejects incompatible duplicate schemas,
histogram binning changes, and RNTuple/histogram path collisions.

Outputs are first written to `<name>.root.partial`, reopened and checked for
the expected RNTuple entry counts, then atomically published to the requested
path. Failed jobs leave no apparently complete target file.
