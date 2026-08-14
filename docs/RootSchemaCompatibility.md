# ROOT persistent-object ABI and schema policy

[Documentation index](README.md)

Last audited: 2026-07-13.

## Current inventory

The only dictionary-enabled persistent value classes with `ClassDef` are
`Event`, `Particle`, `Lepton`, and `MuMu`; all are version 1. NanoAOD input
types are non-streaming event-scoped views (`#pragma link ...-`) and therefore
have no persistent-object schema.

The output code paths audited in `AnalyzerCoreOutput.cc` and in the analysis
modules write primitive, fixed-array, and STL-vector RNTuple fields.
Schema-preserving skims use RDataFrame's RNTuple Snapshot backend. No current
call site persists a `DataFormats` value object, so view and branch-manager
changes do not alter a persisted `DataFormats` layout.

`Event` may reference a trigger map/provider and capture the provider epoch at
runtime. These three members are marked ROOT-transient (`//!`); they are not
part of the persisted schema, so `ClassDef(Event, 1)` is intentionally
unchanged. This still changes the C++ layout and therefore requires the full
project rebuild mandated below.

## Change policy

1. Changing the type, order, or number of non-transient data members of a
   `ClassDef` class requires a `ClassDef` version bump in the same change.
2. A change must add an old-file fixture and prove old → new readback before it
   may be merged. If a supported consumer must read newly written files, add
   the corresponding new → current workflow check as well.
3. Runtime-only handles, branch providers, caches, spans, and epoch state are
   not to be placed in persistent objects. Keep them outside the class or mark
   them ROOT-transient (`//!`) after confirming dictionary behavior.
4. Public C++ ABI is only supported within a full project rebuild. External
   users must rebuild against headers when DataFormats changes.
5. The fixture command, ROOT version, class version, and expected values must
   be recorded next to the test. A new persistent output call site must update
   this inventory before changing the relevant class.

## Required test for a future layout change

Create the fixture using the previous library, then run a test linked against
the new dictionary that opens the file, validates every persisted member and
checks the expected class version. Do not substitute an in-memory write/read
test: it cannot exercise ROOT schema evolution.
