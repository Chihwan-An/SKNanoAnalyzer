# Analyzer development and ownership

[Documentation index](README.md)

SKNano separates the event-processing backend from runnable analyzers. This
boundary is enforced by CMake targets, ROOT dictionaries, installed include
paths, and the layer check.

## Repository ownership

- `AnalyzerFramework` contains the loader, branch manager, standard NanoAOD
  views, output services, and `AnalyzerCore`. Do not add a concrete physics
  analyzer here.
- `CommonAnalyzers` contains analyzers used without analysis-group-specific
  policy. Promotion to this directory requires a normal backend pull request.
- User and analysis-group analyzers live in an external module repository.
  Start from `examples/AnalysisModule` and do not add them to the backend.

The old flat `Analyzers` directory and flat includes are unsupported. Use:

```cpp
#include <AnalyzerFramework/AnalyzerCore.h>
#include <MyAnalysis/MyAnalyzer.h>
```

## Building an external module

An external module calls `sknano_add_analysis_module` from its `CMakeLists.txt`
and owns its headers, sources, ROOT LinkDef, tests, data, and documentation.

```bash
./scripts/build.sh --analysis-module-dir /path/to/MyAnalysis
```

Multiple directories may be supplied. A branch may pin a module as a Git
submodule and export `SKNANO_ANALYSIS_MODULE_DIRS` so the usual clean build
automatically includes it. Each build installs `share/sknano/analyzers.manifest`;
`SKNano.py` checks this file before creating jobs.

## Custom input schemas

The canonical schema in the backend describes only official NanoAOD fields.
Custom input schemas belong to the module that consumes them. Module-generated
row views use `GetColumnHandle<T>` and remain event-scoped and zero-copy:

```cpp
for (const auto candidate : customInput.lambdaC()) {
    const float pt = candidate.pt();
    const auto proton = candidate.proton();
    const float protonPt = proton.pt();
}
```

Required fields fail during analyzer initialization. Optional fields must be
checked with `available()`. A stale row view fails its event-epoch check.
Custom views are not backend `DataFormats`; promote an object there only when
it is stable, persistent, and useful to common analyzers.

## Git and submodules

Keep the backend and analysis histories independent. The backend branch records
only a submodule commit. Update it intentionally, rebuild, run the module smoke
tests, and commit the new gitlink. Batch run manifests record both revisions.
