# HadronAnalyzer

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

The analyzer enables the `BaselineOnly` and `QuadJet` flags itself. Exactly
one of `Mu` and `El` is required. `RemoveFlavTagCut` can be added when the same
selection without the Vcb flavor-tag requirement is needed.

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

Custom BPH collections are read only after an event passes the baseline. Their
individual payload branches remain lazy and activate when a non-empty
collection is first inspected.

The analyzer is also the reference implementation for the typed output API:
`TreeHandle` owns the branch contract, while pre-booked `HistogramGroup`
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

`PtFraction` uses the fitted candidate momentum over reco-jet momentum for
reconstructed candidates and gen-hadron momentum over gen-jet momentum for
generator hadrons. `OriginCategory` is exclusive, with priority
`FromDstar`, `FromB`, `FromC`, `Feeddown`, `Prompt`, then `Other`.

Reco-versus-gen multiplicity histograms for each leading-jet rank are stored
under `HadronAnalyzer/RecoVsGen/Jet{0..3}` for D0, Dstar, Lambda, LambdaC, and
the total selected hadron count.
