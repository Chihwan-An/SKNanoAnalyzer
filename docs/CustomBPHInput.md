# Custom BPH NanoAOD input

The 2024 `TTLJ_powheg_CustomBPH` sample points to the site-local custom
production through one recursive `path_glob`.  Submit it like any other sample:

```bash
SKNano.py -a AnalyzerName -i TTLJ_powheg_CustomBPH -e 2024 -n 10
```

The custom schema is isolated in
`data/Run3_v15_Run2_v15/custom/CustomBPHBranches.json`.  It does not modify the
canonical NanoAOD v15 schema.  During a build this addon generates optional,
event-scoped lazy views for these collections:

- `GetAllD0ToKPiViews()`
- `GetAllDstarToD0PiViews()`
- `GetAllGenJetBPHViews()`
- `GetAllGenJetHadronViews()`
- `GetAllLambdaViews()`
- `GetAllLambdaCToPKPiViews()`

An analyzer reads only the fields it touches:

```cpp
auto d0s = GetAllD0ToKPiViews();
if (d0s.available()) {
    for (const auto d0 : d0s) {
        const float pt = d0.pt();
        const float mass = d0.mass();
        // use pt and mass in this event only
    }
}
```

`available()` is false on ordinary NanoAOD.  A partially present custom
collection is a configuration error instead of silently looking empty.  Views
and their rows are event-scoped and reject use after the loader advances.

To add or change a custom branch, edit only `CustomBPHBranches.json` and rebuild.
The generator updates declarations, registration, reset code, and row accessors
together.  Validate a production file with:

```bash
build/redhat/tests/custom_bph_file_smoke /path/to/input.root
```
