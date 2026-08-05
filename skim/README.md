# HNWR pre-skim

Standalone skimmer for the W_R → lN → lljj analysis. Reads NanoAOD, throws away
events `Analyzers/src/Reproduce20_002_copy.cc` can never use, writes reduced
NanoAOD.

**Nothing here imports, links against, or needs a build of SKNanoAnalyzer.**
Only ROOT (`skim_hnwr.py`) or uproot+awkward (`verify_margin.py`).

## Why it is built this way

The analyzer's object corrections — EGM scale & smear, Rochester / Generalized
Endpoint, JES, JER — are re-tuned constantly. A skim that ran through
`AnalyzerCore::GetAllElectrons()` and friends would inherit every one of those,
so its acceptance would shift each time a correction changed and quietly go
stale.

So the skim reads **only raw NanoAOD branches** and cuts with a wide margin.
`Electron_pt`, `Muon_pt * Muon_tunepRelPt`, the ID bitmaps, the HLT bits — that
is the entire input. No correction can move any of them.

## The cut

```
(HLT OR over an era-specific superset)
AND >= 2 loose leptons with raw pT > 42, |eta| < 2.6 (e) / 2.5 (mu)
AND >= 1 of them with raw pT > 100 (electron) or > 45 (muon)
```

Necessary because every fill in `Reproduce20_002_copy.cc` sits downstream of
the resolved branch (`:1391`, 2 tight leptons) or the boosted branch (`:2008`,
≥1 tight lepton), both require a trigger (`:1444`, `:2041`), and every region
needs a second lepton — resolved wants two tight, boosted SR wants the
same-flavour loose lepton (`:2422`), boosted flavour CR the opposite-flavour one
(`:2661`), boosted DY CR the low-mll one (`:2052`).

| quantity | analyzer | skim | margin |
|---|---|---|---|
| lead electron pT | 130 | 100 | 23 % |
| lead muon pT | 60 | 45 | 25 % |
| sublead lepton pT | 53 | 42 | 21 % |
| electron \|eta\| | 2.5 | 2.6 | — |
| muon \|eta\| | 2.4 | 2.5 | — |

Isolation is never cut on — the mini-isolation studies
(`Analyzers/src/HNWR_miniiso*.cc`) are actively redefining it. Electrons need
`cutBased_HEEP || looseNoIso` (cut-based Tight is a subset of looseNoIso, so
switching the tight WP stays covered); muons need `highPtId >= 1 || tightId`.

**Jets, fat jets and the m(lljj) > 800 GeV cut are deliberately not applied.**
`AnalyzerCore::ScaleJets(..., "total")` multiplies 22 individual JES sources
together (`AnalyzerCore.cc:483-528`) rather than adding them in quadrature, so
AK4 jet pT can move by tens of percent between correction versions. Leptons are
the only objects stable enough to skim on. MET noise filters (0.07 % rejection)
and the jet veto map (5 %) are skipped too — no real saving, and both would
couple the skim to code that does change.

## Measured

2023 `TTLL_powheg`, one 183k-event file — the hardest case, since it genuinely
contains two prompt leptons:

```
all                   183000
trigger OR            114813   step 0.6274
leptons                16036   step 0.1397
kept                   16036   overall 0.08763  (11.4x reduction)
size  0.42 GB -> 0.061 GB (6.8x)     time 58.6 s with 4 threads
```

Backgrounds without two real leptons reduce far harder — the 2023 cutflow gives
per-group tight-lepton rejection factors of 6.1× (DY) to 20.5× (VV).

`verify_margin.py` scales every lepton pT by a factor, standing in for any
correction of any size in the worst direction, and counts events the analyzer
would then have used but the skim already discarded:

```
  correction   analyzer would use   LOST by skim
    x1.00             8309                 0
    x1.20            13528                 0
    x1.26            15306                 0
    x1.28            15898               308
    x1.50            22443              6407
```

**Zero loss up to a coherent +26 % shift on every lepton.** Real corrections:
electron EGM scale/smear 1–2 %, muon Rochester below 1 %, Generalized Endpoint
~11 % only at 1 TeV where muons are far above threshold anyway.

## What is kept

All 1782 `Events` branches — nothing is pruned, so the skim survives the
analyzer starting to read a branch it does not read today. `--drop '^Photon_'
'^Tau_'` prunes if disk matters more.

`Runs` (with `genEventSumw`, `genEventCount`, `LHEScaleSumw`, `LHEPdfSumw`,
`PSSumw`) and `LuminosityBlocks` are copied unfiltered, so normalisation is
recoverable from the file itself. `SKNano.py:290-296` does not need this — it
follows the skim alias back to the original PD's `CommonSampleInfo.json` entry —
but it makes the files self-describing.

## Files

| | |
|---|---|
| `hnwr_skim_cuts.py` | the selection and its justification — **the single source of truth** |
| `skim_hnwr.py` | worker: N input files → one skimmed file |
| `verify_margin.py` | proves the margin is sufficient; needs no ROOT |
| `ab_test.sh` | runs the analyzer on raw and on the skim, then compares |
| `compare_hists.py` | bin-by-bin diff of two analyzer outputs |
| `submit_skim.py` | chunks a sample over HTCondor |
| `register_skim.py` | writes the JSONs so `SKNano.py -i 'Skim_HNWR_*'` works |
| `skim_naming.py` | alias/path conventions shared by submit and register |
| `backgrounds_2023.txt` | the seven cutflow background groups as glob patterns |

## Two levels of verification

`verify_margin.py` scales lepton pT and shows nothing is lost — but it assumes
the necessary condition read out of `Reproduce20_002_copy.cc` is right.

`ab_test.sh` tests that assumption against the analyzer itself: same input
files, analyzer run twice (raw and skimmed), every histogram compared. Takes
about 15 minutes for one TTLL file (the analyzer needs ~10.6 min on 183k raw
events, ~1 min on the 16k that survive).

```bash
./ab_test.sh                       # TTLL_powheg, 1 file, era 2023
./ab_test.sh DYMLL120HT2500 2      # 2 files of another sample
```

Everything must match except the cutflow bins counting events from before the
skim's own cuts — `compare_hists.py` knows which those are and reports them
separately.

Result on TTLL_powheg (2023, 1 file, 91955 histograms):

```
  expected differences : 27828 bins (pre-skim cutflow bins)
  REAL differences     : 0
PASS -- skim is lossless for the analysis
```

All 1084 SR/CR physics distributions are bit-identical, across all 17 systematic
directories. The differing bins are confined to cutflows and object-multiplicity
monitors, and stop exactly where the skim's own cuts become part of the
analyzer's selection:

| histogram | last differing ROOT bin | first identical bin |
|---|---|---|
| `Cutflow_for_Boosted_SR` | 9 (`hasnoextralep`) | 10 = `hassflooselepton` (`:2550`) — a second lepton is required |
| `Cutflow_for_reseolved_SR` | 4 (2 tight leptons) | 5 = trigger (`:1445`) |

Note `FillHist(h, N, ...)` fills ROOT bin **N+1**; the table uses ROOT bin
numbers.

### Measured reduction, 2023 background MC (54 samples, 1537 chunks)

```
1.85 TB  ->  110 GB      16.8x smaller
993.5M   ->   30.9M events   32.2x fewer
```

Per-sample reduction spans 10x (TTLL, ST_tW_Lep — two real prompt leptons) to
>10^5 (WWto4Q, ZZto2Nu2Q — fully hadronic).

## Usage

```bash
source setup.sh                     # for $SKNANO_DATA and $SKNANO_RUN3_NANOAODPATH

cd skim

# 1. check the margin on a sample before committing to it
./verify_margin.py --era 2023 /gv0/.../tree_1.root

# 2. see what would be submitted
./submit_skim.py --era 2023 --samples 'TTLL_powheg' --files-per-job 5 --dry-run

# 3. let condor parse the submit files without queueing anything
./submit_skim.py --era 2023 --samples-from backgrounds_2023.txt --files-per-job 5 --validate

# 4. submit
./submit_skim.py --era 2023 --samples-from backgrounds_2023.txt --files-per-job 5
./submit_skim.py --era 2023 --samples 'EGamma*' 'Muon*' --files-per-job 5
./submit_skim.py --era 2023 --samples 'WR*N*'   --files-per-job 10

# 4. register when the jobs finish
./register_skim.py --era 2023 --suffix HNWR --all

# 5. run the analysis on the skims
SKNano.py -a Reproduce20_002_copy -i 'Skim_HNWR_*' -e 2023 -n -1
```

A single chunk can also be run by hand, no condor:

```bash
./skim_hnwr.py --era 2023 --threads 4 --output /path/tree_0.root in1.root in2.root
```

## Before re-skimming a sample

Delete the output directory first. `register_skim.py` walks the tree for
`*.root`, so leftovers from an earlier run get registered too and events are
double-counted. `submit_skim.py` refuses to write into a non-empty output
directory for the same reason, and `register_skim.py` refuses to register one
containing a `Temp_*` subdirectory — that is exactly the state
`Skim_20002_DYJets.json` is in today (100 real + 100 nested `Temp_` paths).

## Note on the old `Skim_20002`

`Analyzers/src/Skim_20002.cc` does not reduce anything: it falls through to an
unconditional `newtree->Fill()` at `:312`, so every event survives. Measured on
`Skim_20002_ST_sch_top_Lep`: 2,596,000 skim entries against `nmc` = 2,596,000.
It also cut electrons at `|eta| < 2.4` (`:117`) where the analyzer uses 2.5, so
it lost real events on top of that. Roughly 2.6 TB of it sits in
`/gv0/DATA/SKNano/Run3NanoAODv12/2023/MC/Skim/achihwan/` unused — the current
production reads raw NanoAOD directly.
