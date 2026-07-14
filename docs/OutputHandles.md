# Typed analysis output

`AnalyzerCore` provides typed handles for ROOT trees and histograms. New
analyzers should use these handles instead of calling `TTree::Branch`,
`NewTree`, `SetBranch`, `FillTrees`, or `FillHist` directly.

## Trees with stable buffers

Book the tree once during `initializeAnalyzer()` and keep all branch buffers as
analyzer members:

```cpp
outputTree_ = BookTree("Events");
outputTree_
    .Branch("run", output_.run)
    .Branch("Jet_pt", output_.jetPt)
    .Branch("Hadron_pdgId", output_.hadronPdgId);
```

The branch type is inferred for arithmetic scalars, `std::array<T, N>`, and
`std::vector<T>`. Fill it after assigning the buffers:

```cpp
outputTree_.Fill();
```

For legacy training trees whose scalar branches are assigned by name, use the
same handle with `Set`:

```cpp
OutputTree("Training_Tree")
    .Set("weight", weight)
    .Set("category", category)
    .Fill();
```

## Histogram groups

Use a group to define a common output directory and book fixed histograms once:

```cpp
const auto histograms = Hists("MyAnalyzer");
pt_ = histograms.Book1D("Object/Pt", 50, 0., 500.);
pt_.Fill(pt, weight);
```

For names determined dynamically by a systematic or category, use the lazy
compatibility path:

```cpp
Hists("MyAnalyzer").Fill(dynamicName, value, weight, 50, 0.f, 1.f);
```

The registry rejects incompatible duplicate branch schemas, histogram binning
changes, and tree/histogram paths where one ROOT object would have to be used
as a directory for another.
