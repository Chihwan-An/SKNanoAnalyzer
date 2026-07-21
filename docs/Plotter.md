# Schema-driven ROOT plotter

The plotter recursively catalogs TH1 and TH2 objects and assigns meaning to
their paths through a YAML tree schema. The Python engine has no analyzer,
process, channel, or directory-depth conventions built into it.

## First use with a ROOT file

```bash
python -m sknano_plotter inspect output.root --schema-out plots.yaml
```

The generated schema is deliberately a draft. Inspect its mutually exclusive
path rules, rename the neutral `level_N` dimensions, enable or add recipes, and
change `status: draft` to `status: confirmed`. Check complete and unique object
coverage before plotting:

```bash
python -m sknano_plotter validate output.root --schema plots.yaml --confirmed
python -m sknano_plotter plot output.root --schema plots.yaml \
  --output plots --workers 8
```

The repository setup adds `python/` to `PYTHONPATH`. Without that setup, run
`scripts/sknano_plot.py` (the `SKNanoOutput/plotter.py` path delegates to the
same CLI in the current workspace) or export `PYTHONPATH=$PWD/python`.

Each rule uses a full-match regular expression with named groups. Named groups
become dimensions usable by recipes. A file may contain variable-depth paths;
put mutually exclusive rules for its subtrees in the same schema.

## Recipe primitives

- `hist1d`: one raw or unit-normalized TH1 per matched object
- `overlay`: compare the values of `series`, grouped by `facets`
- `stack`: stacked components without an observed-data role
- `data_stack`: stack, data points, optional signal, total uncertainty and ratio
- `ratio`: compare one or more series to the explicitly selected reference
- `variation`: nominal/variation overlay
- `heatmap`: TH2 with axis metadata and a colorbar

`select` restricts named dimensions. Values can be exact strings, lists,
`re:<pattern>`, or mappings containing `regex`, `in`, or `not`. Plot transforms
(`rebin`, `flow`, `normalize`) and physical scales are explicit schema values.
Nothing is inferred from an analyzer name.

For `data_stack`, `roles.data`, `roles.stack`, and `roles.signal` classify the
series dimension. The optional `systematics` mapping names the variation
dimension, nominal value, Up/Down suffixes, and missing-variation policy.

## Output and batch execution

Local output contains PNG/PDF files, `tasks.json`, per-task records,
`manifest.json`, and a searchable static `index.html`.

```bash
python -m sknano_plotter submit output.root --schema plots.yaml \
  --output plots --chunk-size 20
```

Condor workers consume the same task manifest as local execution. The input is
always one ROOT file; merging sample files remains an explicit upstream step.

Example schemas are in `python/sknano_plotter/examples/`. They demonstrate a
hierarchical data/MC layout and a variable-depth validation layout, but are not
special-cased by the engine.
