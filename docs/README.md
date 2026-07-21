# Documentation

SKNanoAnalyzer documentation is grouped by workflow. New users should start
with [Getting Started](GettingStarted.md); framework contributors should read
the core API documents before changing analyzer or I/O contracts.

## Setup and daily use

- [Getting Started](GettingStarted.md): installation, builds, job submission,
  sample metadata, skimming, and notifications.
- [Environment Setup](SettingEnv.md): detailed micromamba and Singularity
  setup for the SNU cluster.
- [Development Guide](DevelopmentGuide.md): repository workflow, systematic
  APIs, coding conventions, and pull requests.

## Core framework and I/O

- [Event-scoped Views](EventScopedViews.md): lazy NanoAOD collection access and
  object-lifetime rules.
- [Analyzer Task API](AnalyzerTaskAPI.md): shared selections with independently
  selectable analyzer tasks.
- [Typed Output](OutputHandles.md): RNTuple fields and histogram groups.
- [RNTuple I/O](RNTupleIO.md): input selection, conversion, merging, and
  performance telemetry.
- [ROOT Schema Compatibility](RootSchemaCompatibility.md): persistent-object
  ABI and schema-evolution policy.

These documents describe one connected data path:

```text
NanoAOD input -> event-scoped views -> analyzer tasks -> typed RNTuple/histogram output
```

## Analysis-specific guides

- [Custom BPH Input](CustomBPHInput.md): optional 2024 BPH collections and
  generated lazy views.
- [HadronAnalyzer](HadronAnalyzer.md): flags, tasks, matching, output fields,
  and histogram layout.

The YAML files [noSyst.yaml](noSyst.yaml) and
[ExampleSystematic.yaml](ExampleSystematic.yaml) are small systematic
configuration examples. [Nano-linux-64.lock](Nano-linux-64.lock) is the pinned
Linux environment used by the setup guide.

## Documentation maintenance

Keep commands executable from the repository root and link new pages from this
index. Put stable user and API contracts here; implementation plans, benchmark
notes, and temporary validation reports belong in issues or development
artifacts rather than the maintained documentation set.
