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
| `hnwr3sr_skim_cuts.py` | the `--cuts hnwr3sr` variant for `HNWR_BDT_presel_3SR.cc` (see below) |
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

## The 3SR cut set (`--cuts hnwr3sr`)

`Analyzers/src/HNWR_BDT_presel_3SR.cc` **cannot use `Skim_HNWR_*`.** Its third
signal region is defined by the absence of a second lepton:

```cpp
// HNWR_BDT_presel_3SR.cc:578
bool isBoostedNoLepCandidate = isBoostedBase && (n_boosted_cap_leptons == 0);
```

which is the exact complement of this skim's `MIN_N_LEPTON = 2`. Running 3SR on
a `Skim_HNWR_*` sample does not leave `SR_*_boosted_nolep` empty — it leaves an
eta-edge remnant: second leptons that passed the skim at |eta| 2.5–2.6 (e) or
2.4–2.5 (mu) and are then dropped by the analyzer's own `SelectElectrons(...,
2.5)` / `SelectMuons(..., 2.4)` (`3SR:224-225`). A silently biased SR is worse
than a missing one.

`hnwr3sr_skim_cuts.py` therefore inherits every threshold and trigger from
`hnwr_skim_cuts.py` and changes one number, `MIN_N_LEPTON = 1`. The three SRs
and every CR all sit downstream of the same lead-lepton requirement (`3SR:310`,
`:489`, `:566`, `:434-447`), so the lead is the only necessary condition there
is. The sublead only *classifies* an event; since the skim prunes no branches,
the analyzer re-derives its Cap / ResSub tiers (pT > 20, `POG_LOOSE` muons,
mvaNoIso electrons — all outside what this skim would count as a lepton) from
the surviving events untouched.

The margin table changes in one row: the binding lead-electron cut is the
trigger-safe **118** (`3SR:97`), not `Reproduce20_002_copy`'s 130, so 100 is a
15 % margin rather than 23 %. Measured on one 2023 `QCD_Pt-300to470_MuEnriched`
file (44233 events):

```
  correction   analyzer would use   LOST by skim
    x1.00            2446                 0
    x1.30            3263                 0
    x1.35            3271                 5
zero loss up to a coherent +30 % shift on every lepton pT
```

The cut set is a strict **superset** of `hnwr` — identical triggers and
thresholds, one fewer lepton required — so a `Skim_HNWR3SR_*` file also feeds
`Reproduce20_002_copy` and `HNWR_BDT_presel`. The price is reduction power, and
on QCD it is steep: the same file keeps **3266** events here against **102**
under `--cuts hnwr` (13.5× vs 434× reduction). Check one chunk before
committing to a sample.

### Measured cost of the second lepton (2023, one file each)

The two cut sets run back to back on the same input. The blow-up tracks the
lepton multiplicity of the process, not its size: dilepton samples pay ~3×,
single-lepton ones (and data, which is single-lepton dominated) pay 10-30×.

| sample | `hnwr` kept | `hnwr3sr` kept | events | output size |
|---|---|---|---|---|
| `TTLL_powheg` (2 prompt lep) | 16 036 / 183 000 | 56 660 | 3.5× | 0.061 → 0.199 GB (3.3×) |
| `TTLJ_powheg` (1 prompt lep) | 247 / 45 000 | 7 859 | 31.8× | 0.002 → 0.028 GB (14×) |
| `Muon0_C` (data) | 3 744 / 575 036 | 79 914 | 21.3× | 0.008 → 0.116 GB (14.5×) |

Scaling the existing 2023 `Skim_HNWR_*` footprint (**158 GB** MC incl. signals,
**5.0 GB** data) by that spread puts a full 2023 `Skim_HNWR3SR_*` at roughly
**0.7-1.3 TB**. It goes to `/gv0/DATA`, not the personal `/gv0/Users` quota.

Job counts for 2023, from `--dry-run`: 1537 (backgrounds, `--files-per-job 5`),
334 (`WR*N*`, 10), 237 (data, 5); **10 321** for all four Run 3 eras and all
three groups, with 1840 aliases skipped for having an empty `path` list (1832
of them signal points — only 228 of the 686 `WR*N*` aliases are actually
produced in the v12 bookkeeping).

`TTLJ` growing 32× is the physics warning, not just a disk one: semileptonic
ttbar is what `SR_*_boosted_nolep` sits in. `backgrounds_2023.txt` covers TTLJ
and `WJetM*`, but **not QCD** — for a one-lepton SR the multijet / fake-lepton
component is no longer negligible the way it was under the ≥2-lepton selection.
See the QCD command below if it needs to go in.

### One command for every era and group

`--era` takes several eras or an alias (`Run3` = 2022 2022EE 2023 2023BPix,
`Run2`, `all`); `--group` takes the three standard sample sets, each with its
own chunk size — `bkg` (`backgrounds_2023.txt`, 5 files/job), `signal`
(`WR*N*`, 10), `data` (`EGamma*` `Muon*`, 5). `--files-per-job` overrides all
of them.

`data` is `EGamma*` `Muon*` **`SingleMuon*`** — `Muon*` does not cover
`SingleMuon` (fnmatch anchors at the start), and 2022 Run C really was taken in
the SingleMuon PD before it merged into Muon, which is why the existing set has
a `Skim_HNWR_SingleMuon_C`. 2022EE has no SingleMuon; 2023 / 2023BPix carry
`SingleMuon_B` / `_C` aliases whose `path` lists are empty.

Three things make the combined command safe to re-run:

- An alias whose `path` list is **empty** is skipped. This is not rare: 1832 of
  the 2744 `WR*N*` entries across the four eras have no files, as do
  `ttWtoLNu_EWK` (2022), `WJetM*120HT40to100` (2023), `ttZtoQQ` (2023BPix) and
  the SingleMuon stubs. Submitting one would write `queue 0` and leave an empty
  output directory for `register_skim.py --all` to register as a real sample.

- A sample whose workdir already holds a `submit.sub` is **skipped**, because
  jobs still in flight have an *empty* output directory and the non-empty check
  cannot see them. `--resubmit` forces them through.
- A glob that matches nothing in one era is a warning, not an error — data PD
  names differ between eras (2022 `EGamma_C` vs 2023 `EGamma0_C`) and not every
  background alias exists everywhere. A literal alias that does not exist still
  aborts, since that is a typo. Both are listed in the closing summary.

Nothing aborts the run any more; skipped samples and unmatched patterns are
collected and printed at the end.

### Submitting

`condor_submit` is **not** on the `ai-tamsa1` login node, so `--validate` and
the real submit both have to run from a machine that has it. Everything up to
that point (`--dry-run`, single chunks) works here.

A chunk by hand on the login node needs the image, since ROOT lives inside it:

```bash
singularity exec -B /gv0 -B /data6 /data6/Users/achihwan/private-el9.sif bash -c '
  export PATH=/opt/conda/bin:$PATH; export MAMBA_ROOT_PREFIX=/opt/conda
  eval "$(micromamba shell hook -s bash)"; micromamba activate Nano
  python3 skim/skim_hnwr.py --era 2023 --cuts hnwr3sr --threads 4 --output out.root in.root'
```

Note `setup.sh` must be sourced from the repo root — from `skim/` it resolves
`$SKNANO_DATA` to `skim/data/...` and fails on `scripts/install_lhapdf.sh`.

```bash
# margin check
./verify_margin.py --era 2023 --cuts hnwr3sr -- /gv0/.../NANOAOD_1.root

# everything: 4 Run 3 eras x (background, signal, data) in one command.
# A distinct --suffix is enforced, so the two cut sets can never share a
# Skim_<tag>_<alias> directory.
./submit_skim.py --era Run3 --cuts hnwr3sr --suffix HNWR3SR \
    --group bkg signal data --dry-run          # 10314 jobs
./submit_skim.py --era Run3 --cuts hnwr3sr --suffix HNWR3SR --group bkg signal data

for e in 2022 2022EE 2023 2023BPix; do
    ./register_skim.py --era $e --suffix HNWR3SR --all
done

# QCD, if the one-lepton SR needs it (separate bookkeeping, see below)
./submit_skim.py --era 2023 --cuts hnwr3sr --suffix HNWR3SR \
    --sample-dir $SKNANO_HOME/data/Run3_v13_Run2_v9/2023/Sample \
    --outbase /gv0/DATA/SKNano/Run3NanoAODv13p1 \
    --samples 'QCD_Pt-*_MuEnriched' --files-per-job 20 --dry-run
```

The QCD samples live only in the **v13** bookkeeping (`Run3NanoAODv13p1`,
`QCD_Pt-*_{MuEnriched,EMEnriched,bcToE}`); the inclusive `QCD_HT*` / `QCD_PT*`
aliases under the default `$SKNANO_DATA` (v12) have empty or dangling `path`
lists in every era, which is why `--sample-dir` and `--outbase` must be
overridden above. 3SR also sets no trigger paths for 2016preVFP / 2016postVFP /
2018 (`3SR:84-98`), so `--cuts hnwr3sr` refuses those eras outright.

### What this skim does and does not fix in place

It cuts on the **lead lepton only**, so every sublead / capture-tier variation
can be re-run on the same `Skim_HNWR3SR_*` files without re-skimming: the
tight-vs-`ResSub` resolved sublead, the `Cap` tier, the pT > 20 floor, the
mll > 200 classification boundary. That is the whole point of running the skim
before settling the presel.

The lead is the one thing frozen. Loosening it — lead pT below the analyzer's
trigger-safe 118 (e) / 60 (mu), or a lead ID outside HEEP / highPtId+TkRelIso —
falls outside `LEAD_ELE_PT = 100` / `LEAD_MU_PT = 45` and their ID cut in
`hnwr_skim_cuts.py`, and would need a new skim.

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
