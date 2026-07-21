# HadronAnalyzer

[Documentation index](README.md)

`HadronAnalyzer` applies the `Vcb_SL` semileptonic ttbar baseline and stores BPH
and generator-hadron information for the four leading selected jets.

## Running

Muon channel:

```bash
SKNano.py -a HadronAnalyzer -i TTLJ_powheg_CustomBPH -e 2024 -n 10 --userflags Mu
```

Electron channel:

```bash
SKNano.py -a HadronAnalyzer -i TTLJ_powheg_CustomBPH -e 2024 -n 10 --userflags El
```

The analyzer enables the `BaselineOnly` and `QuadJet` flags itself and exactly
one of `Mu` and `El` is required.
`RemoveFlavTagCut` can be added when the same selection without the Vcb
flavor-tag requirement is needed.
`HadronHistOnly` books and fills the complete histogram set without writing
`BPHEvents`. This is useful for recovering or validating histograms without
creating another large training ntuple:

```bash
SKNano.py -a HadronAnalyzer -i TTLJ_powheg_CustomBPH -e 2024 -n 10 \
  --userflags Mu,HadronHistOnly
```

`LambdaCBDT` writes a compact LambdaC training RNTuple while leaving the full
histogram workflow unchanged:

```bash
SKNano.py -a HadronAnalyzer -i TTLJ_powheg_CustomBPH -e 2024 -n 10 \
  --userflags Mu,LambdaCBDT
```

Only `LambdaCToPKPi` rows are stored in `BPH_*`. Every row with nonzero
`BPH_genMatchCategory` is retained. Unmatched combinatorial rows are retained
with a deterministic 1/20 prescale based on run, luminosity block, event, and
candidate source index. Their `BPH_prescaleWeight` includes this additional
factor of 20, so training or yield studies should use
`weight * BPH_prescaleWeight`. The deterministic selection is independent of
job splitting. Generator-jet detail and flattened `GenHadron_*` fields are
omitted; candidate truth labels, reco-jet context, top-jet matching, daughter
features, and all hypothesis masks remain available.

## Tasks

The baseline selection is shared by independently selectable tasks. The
current BPH tree and histogram workflow is the `BPHStudy` task:

```bash
SKNano.py -a HadronAnalyzer -i TTLJ_powheg_CustomBPH -e 2024 -n 10 \
  --userflags Mu,BPHStudy
```

With no task flag, `BPHStudy` is enabled for backward compatibility.
`HadronAll` enables every registered task, and multiple individual task flags
are additive. Each task registers its input validation, output booking, event
hook, and either central-only or all-variations systematic policy through the
shared `AnalyzerCore::RegisterTask()` API. New Hadron studies should be added
to `HadronAnalyzer::RegisterTasks()` rather than adding another top-level
branch to `executeEvent()`.

## Matching and output

The `BPHEvents` tree contains fixed four-element `Jet_*` and `GenJet_*`
branches. Reconstructed candidates are flattened into `BPH_*`; `BPH_jetRank`
identifies the nearest leading jet within `deltaR < 0.4`. Species codes are:

- `0`: `D0ToKPi`
- `1`: `DstarToD0Pi`
- `2`: `Lambda`
- `3`: `LambdaCToPKPi`

Generator hadrons are flattened into `GenHadron_*`. They are associated by
the NanoAOD indices `Jet_genJetIdx == GenJetHadron_genJetIdx`, and
`GenHadron_jetRank` gives the corresponding leading reco-jet rank.

For a gen-matched `LambdaCToPKPi` row, `BPH_genHypothesisMask` records every
truth-compatible p/K/pi assignment and `BPH_genHypothesisIdx` records the
lowest-cost compatible assignment. Their bit/index order is the same as
`BPH_hypothesisMask`: `pKpi`, `ppiK`, `Kppi`, `pipK`, `Kpip`, `piKp`.
Unmatched rows use mask `0` and index `-1`. The labels are reconstructed from
the candidate's `genPartIdx`, the retained `GenPart` descendant graph, and the
three compact `HadronTrack` rows; no additional input NanoAOD branch is
required.

For a hypothesis-expanded training row with assignment index `h`, use
`(BPH_genHypothesisMask & (1 << h)) != 0` as the binary truth label. Use
`(BPH_hypothesisMask & (1 << h)) != 0` separately to select hypotheses that
passed the reconstruction mass window. The two masks need not overlap: a
truth-matched triplet can be retained because a wrong mass assignment passed
the window while its truth assignment did not.

Top-decay jets are exposed in fixed four-element `TopJet_*` arrays with role
order `[hadronic b, leptonic b, hadronic-W up-type, hadronic-W down-type]`.
The matcher follows the `GenPart_genPartIdxMother` graph while collapsing
same-PDG copy chains, prefers `isLastCopyBeforeFSR` partons, and first matches
partons to signed `GenJet_partonFlavour` inside `deltaR < 0.4`. If no signed
candidate exists, an absolute-flavour fallback is allowed and recorded in
`TopJet_flavourFallback`.

Reco matching treats `Jet_genJetIdx` as the primary association. A
`deltaR < 0.2` fallback is considered only for jets without a GenJet index.
All four roles are optimized together with unique GenJet and reco-jet
constraints, maximizing reco then gen completeness before minimizing the
geometric and pT-balance cost. `TopJet_matchSource` is `0` for no reco match,
`1` for the NanoAOD index, and `2` for the deltaR fallback.
`TopJet_assignmentMargin` is the cost gap to the next assignment with equal
completeness (`-1` when none exists). `TopJet_topologyStatus` is `0` on a
resolved semileptonic topology, `1` for a missing top pair, `2` for an
incomplete top decay, and `3` for a non-semileptonic decay.

## W-charm fragmentation versus reconstructed W mass

For resolved `W -> cq` decays, the analyzer identifies the W-daughter role
with `abs(TopJet_partonPdgId) == 4`. The reconstructed W mass is formed from
the two matched W-daughter reco jets. `TopJet_hadronicWRecoMass`,
`TopJet_wCharmRole`, `TopJet_wCharmJetIdx`, and
`TopJet_wCharmJetOriginalIdx` expose this event-level assignment.

For `D0ToKPi`, `DstarToD0Pi`, and `LambdaCToPKPi` candidates assigned to that
charm jet, the longitudinal fragmentation fraction is

```text
z_parallel = p_hadron . p_jet / |p_jet|^2
```

where the dot product uses three-momenta. Candidate rows store this in
`BPH_wCharmZ` and the association distance in `BPH_wCharmDeltaR`; unmatched
rows contain `-999`. The producer `jetIdx` must agree with the matched raw
NanoAOD jet index when that foreign key is available, and the candidate must
also satisfy `deltaR(candidate, charm jet) < 0.4`.

Histograms are written below
`HadronAnalyzer/WCharmFragmentation/{D0ToKPi,DstarToD0Pi,LambdaCToPKPi}`.
Each species has `Inclusive` and `GenMatchedNoBottom`
(`genMatchCategory == 1`) subgroups containing `ZVsRecoWMass` and
one-dimensional z distributions in the reconstructed-W-mass bins `<60`,
`60--70`, `70--80`, `80--90`, `90--100`, and `>=100 GeV`. The signal group
therefore excludes bottom feed-down (`genMatchCategory == 2`). Candidate
prescale weights are applied to these histograms.

Custom BPH collections are read only after an event passes the baseline. Their
individual payload branches remain lazy and activate when a non-empty
collection is first inspected.

Candidate daughters are resolved through the compact `HadronTrack` table.
Out-of-range candidate or D0 foreign keys are treated as malformed input and
stop the job. Quantities removed from the compact schema (`ptErr/pt`,
`sv_chi2/sv_ndof`, rapidity, and pointing angle) are derived in the analyzer.

The analyzer is also the reference implementation for the typed output API:
`RNTupleHandle` owns the field contract, while pre-booked `HistogramGroup`
handles own histogram paths and binning.

## Histograms

The original leading-rank multiplicities remain under
`HadronAnalyzer/Jet{0..3}`. Shape histograms are grouped by the matched reco
jet's generator flavour:

```text
HadronAnalyzer/{LightJet,CharmJet,BottomJet}/
  D0/FitMassKPi
  D0/PtFraction
  D0/DeltaRJet
  D0/LxySignificance
  D0/CosTheta2D
  Dstar/DeltaMass
  Lambda/FitMass
  LambdaC/FitMass
  LambdaC/PtFraction
  GenHadron/PtFraction
  GenHadron/DeltaRRecoJet
  GenHadron/OriginCategory
  GenHadron/PdgCategory
```

Basic schema and content checks are available without a jet match under
`HadronAnalyzer/Validation`: collection multiplicities, the four candidate
mass observables, `HadronTrackPt`, and `HadronTrackPtErrorRelative`.
For `LambdaCToPKPi`, `LambdaCAcceptedHypothesisMass` reconstructs every p/K/pi
assignment enabled by `hypothesisMask`, while
`LambdaCGenMatchedHypothesisMass` keeps the same assignments only for rows
with a nonzero `genMatchCategory`. Both use 5 MeV bins over 2.13--2.45 GeV;
the stored `LambdaCMass` is the generic
assignment-0 value and is retained as a schema check, not as the preferred
signal-peak observable.
These validation histograms are filled only after the same semileptonic ttbar
baseline and four-leading-jet requirement as the rest of the analyzer. They
therefore use the selected-event weight and verify compact track/D0 foreign
keys only for selected events.

`PtFraction` uses the fitted candidate momentum over reco-jet momentum for
reconstructed candidates and gen-hadron momentum over gen-jet momentum for
generator hadrons. `OriginCategory` is exclusive, with priority
`FromDstar`, `FromB`, `FromC`, `Feeddown`, `Prompt`, then `Other`.

Reco-versus-gen multiplicity histograms for each leading-jet rank are stored
under `HadronAnalyzer/RecoVsGen/Jet{0..3}` for D0, Dstar, Lambda, LambdaC, and
the total selected hadron count.
