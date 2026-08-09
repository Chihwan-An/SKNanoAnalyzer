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

## Samples that are not in the bookkeeping (`--dataset-base`)

`submit_skim.py` normally takes its file lists from
`$SKNANO_DATA/<era>/Sample/ForSNU/<alias>.json`. Some datasets have no entry at
all: `data/Run3_v12_Run2_v9/*/Sample/ForSNU` carries only the **inclusive**
`QCD_PT*` aliases (`QCD_PT30to50`, …), every one of them with an empty `path`
list, and nothing for the `QCD_PT-*_EMEnriched` / `_MuEnrichedPt5` datasets that
actually matter for a fake-lepton background.

`--dataset-base DIR` resolves them straight off the storage element instead:
every `*.root` under `DIR/<campaign>/<dataset>/`, with the era → campaign map in
`CAMPAIGNS` (`2022` → `Run3Summer22NanoAODv12`, and so on). Dataset directory
names are cut at `_Tune` to give the alias, so
`QCD_PT-30to50_EMEnriched_TuneCP5_13p6TeV_pythia8` becomes
`QCD_PT-30to50_EMEnriched`; `--no-trim-name` keeps the full name. Chunking,
submission and the skim itself are unchanged — they never cared where the list
came from.

`--layout flat` writes `<outbase>/<era>/<alias>/tree_<i>.root` instead of the
`<era>/MC/Skim/$USER/Skim_<suffix>_<alias>` layout SKNano.py expects. Use it for
a skim area that is not going through `register_skim.py`; the cut set is then
recorded only in the workdir name and the `report_*.json` files, so do not point
two cut sets at one `--outbase`.

### `--stage-out`: dCache is write-once

At KNU the skim area sits on dCache (`SE_UserHome` → `/pnfs/...`), where a file
can be created and deleted but **not reopened for writing**. `skim_hnwr.py` does
reopen its output — `copy_aux_trees()` opens it `UPDATE` to append `Runs` and
`LuminosityBlocks` — so writing there directly fails after the `Events` snapshot
has already been written:

```
SysError in <TFile::ReadBuffer>: error reading from file .../tree_direct.root Input/output error
OSError: Failed to open file .../tree_direct.root
```

The result is a file that looks finished but has no `Runs` tree. `--stage-out`
(default `auto`, on when the output path resolves under `/pnfs`) writes to
`$_CONDOR_SCRATCH_DIR` and copies afterwards, with `rm -f` first because dCache
cannot overwrite. It also keeps a crashed job from leaving a truncated
`tree_N.root` behind. `transfer_output_files = ""` goes into the submit file so
condor does not try to bring the scratch copy back as well.

### Job environment

`--conda-bin` / `--mamba-root` / `--mamba-env` default to the **submitting
shell's** micromamba prefix (`mamba_job_env()`, mirroring
`SKNano.py:getMambaJobEnv()`), not to the image's `/opt/conda`. The image ships
its own `Nano` env, and since `/u/user` is mounted on the workers, activating it
loads a second ROOT into the process — every dictionary registers twice
(`already in TClassTable`) and the job dies with a double free at exit, *after*
the output is written, so the logs look like an unrelated teardown crash.

`--image` and `--workbase` fall back to `config/config.$USER` when `setup.sh`
has not been sourced, instead of the old hardcoded `/data6/...` paths.

### QCD for the one-lepton SR, all four Run 3 eras

```bash
./submit_skim.py --era Run3 --cuts hnwr3sr --suffix HNWR3SR \
    --dataset-base /pnfs/knu.ac.kr/data/cms/store/mc \
    --samples 'QCD_PT-*_EMEnriched_*' --layout flat \
    --outbase /u/user/$USER/SE_UserHome/Skim_sample/QCD \
    --files-per-job 4
```

30 samples, 149 jobs, 73 GB of input. Measured on one 2023
`QCD_PT-300toInf_EMEnriched` file — the hardest bin, since its electrons are
hard enough to reach the lead-pT cut:

```
all                   210532
trigger OR             13980   step 0.0664
leptons                 2892   step 0.2069
kept                    2892   overall 0.01374  (72.8x reduction)
size  0.43 GB -> 0.019 GB (22.7x)     time 72.6 s with 4 threads
```

Patterns match the **dataset directory name as it is on disk**, not the trimmed
alias — the `_Tune` cut happens after matching. So the trailing `_*` is
required: `QCD_PT-*_EMEnriched` matches nothing, because every directory is
really `QCD_PT-<bin>_EMEnriched_TuneCP5_13p6TeV_pythia8`.

The pattern is `QCD_PT-*_EMEnriched_*` rather than `QCD_PT-*` because that is the
only QCD flavour available here: the `QCD_PT-*_MuEnrichedPt5` datasets exist in
the 2023BPix campaign as a directory tree with no `*.root` in it, and in no
other era at all. `QCD_PT-*` would resolve them and then drop all seven on the
zero-file guard, which reads like an error rather than a decision.

The consequence is worth stating: this QCD estimate covers the **electron**
fake-lepton background only. Nothing here constrains muon fakes, even though the
skim's trigger OR and lepton cut both accept muons — the muon-enriched
statistics simply are not on this storage element. Getting them needs a rucio
transfer request to KNU first.

Note also that `QCD_PT-300_EMEnriched` and `QCD_PT-300toInf_EMEnriched` are
**both** present in 2023 and 2023BPix — they are separate datasets on disk and
both get skimmed, so do not stack both.

### From skim to plot: the whole QCD chain (done for 2022, 2026-08-09)

`register_skim.py` needs the PD in `CommonSampleInfo.json`, and none of these
EM/Mu-enriched samples are there. `make_sample_info.py` writes those entries:

```bash
# 1. bookkeeping entries. xsec is copied from the v13 tree (it does carry
#    QCD_Pt-*_EMEnriched); nmc/sumW are MEASURED from the Runs tree the skim
#    preserved, because the v12 and v13 productions of the same dataset are not
#    always the same size -- 2022 QCD_PT-300toInf_EMEnriched has 908512 events
#    here against v13's 891941, and copying v13's sumW would misnormalise it.
#    --verify-sign counts genWeight signs in the original NanoAOD instead of
#    assuming sumsign == nmc (measured 2022: zero negative weights in all seven).
./make_sample_info.py --era 2022 \
    --skim-base /u/user/$USER/SE_UserHome/Skim_sample/QCD \
    --dataset-base /pnfs/knu.ac.kr/data/cms/store/mc \
    --sample-dir $SKNANO_HOME/data/Run3_v12_Run2_v9/2022/Sample --verify-sign

# 2. register (--layout flat must match how it was skimmed)
./register_skim.py --era 2022 --suffix HNWR3SR --layout flat --all \
    --outbase /u/user/$USER/SE_UserHome/Skim_sample/QCD \
    --sample-dir $SKNANO_HOME/data/Run3_v12_Run2_v9/2022/Sample

# 3. run the analyzer. --userflags NoSyst is NOT optional, see below
SKNano.py -a presel_3SR -i 'Skim_HNWR3SR_QCD*' -e 2022 -n 4 --userflags NoSyst

# 4. plot (plot_SR.py has a "QCD" background group)
cd $SKNANO_HOME/plots/HNWR/BDT/3SR && ./CR/run_nonprompt_cr.sh 6
```

xsec is matched on the **PD** field, not the alias: v13's
`QCD_Pt-300toInf_EMEnriched` points at `QCD_PT-300_EMEnriched_...` in 2022EE and
`QCD_PT-300toInf_EMEnriched_...` elsewhere. A dataset whose PD matches nothing —
the duplicate `QCD_PT-300_EMEnriched` that also exists in 2023 / 2023BPix — is
reported and skipped rather than guessed at.

`make_sample_info.py` backs `CommonSampleInfo.json` up to `.bak` before its
first write.

**Two failure modes cost a full submission cycle each; both are silent.**

- `--userflags NoSyst` missing. Without it the analyzer takes the full
  systematics path, `SystematicHelper` finds no weight function for `LSF` /
  `ZPtRw_*` and calls `exit(1)` (`AnalyzerTools/src/SystematicHepler.cc:280`).
  The message lands at the end of stderr behind ~30 correction-file warnings, so
  the job just looks like it stopped. It also changes the output path: with the
  flag it is `$SKNANO_OUTPUT/presel_3SR/NoSyst/<era>/`, which is where the
  plotter reads from.
- `setup.sh` resolves `SKNANO_HOME` from the **current directory**. Sourced from
  `skim/` it silently sets `$SKNANO_DATA` to `skim/data/...` and `SKNano.py`
  ends up not on PATH. Source it from the repo root.

### Statistical health of the QCD estimate

The 2022 EE `boosted_nolep` nonprompt CR gets 39301 QCD events, which moves
Data/MC from 1.767 to 1.158 -- QCD was the dominant missing background there.
But it rests on very few MC events, because the 3SR lead-electron cut (pT > 118)
sits far out on the EM-enriched spectrum:

| bin | weight per MC event | events passing |
|---|---:|---:|
| 10to30 | 55 532 | 0 |
| 30to50 | 55 955 | 0 |
| 50to80 | 18 200 | 0 |
| 80to120 | 3 439 | 0 |
| 120to170 | 626 | 7 |
| 170to300 | 142 | 288 |
| 300toInf | 11 | 758 |

A **single** event surviving in 10to30 or 30to50 would add more than the entire
current estimate. The zeros in the low bins are an absence of statistics, not an
absence of contribution, so 39301 is effectively a lower bound with a very long
upward tail. Good enough to establish that QCD belongs in this CR and roughly
what shape it has; not good enough to use as the background estimate. That needs
a data-driven fake rate.

`plot_SR.py` puts QCD in its own `BACKGROUND_GROUPS` entry (kOrange+1) rather
than as a Nonprompt subgroup, so the multijet yield can be read off directly.

### Framework bug this uncovered (fixed)

Pure-pythia8 samples segfaulted on every event. `SKNanoLoader::nLHEPart` was an
uninitialised member; QCD carries no LHE record at all, so `Init()`'s
`SafeSetBranchAddress` returns early and never writes it, the garbage value
passes `GetAllLHEs()`'s `nLHEPart <= 0` guard, and the loop indexes `LHEPart_*`
vectors that `SetMaxLeafSize()` sized to 0. Fixed in
`Analyzers/include/SKNanoLoader.h` (zero-initialise) and
`Analyzers/src/AnalyzerCore.cc` (clamp to the allocated size). Any pythia8-only
sample was affected, not just QCD.

### Submitting

`condor_submit` is **not** on the SNU `ai-tamsa1` login node, so `--validate` and
the real submit both have to run from a machine that has it. Everything up to
that point (`--dry-run`, single chunks) works there. At KNU it comes with the
`Nano` env and both work from the login node, ROOT included — no singularity
needed for a single chunk by hand.

Note that `--validate` writes `submit.sub`, which the already-submitted guard
then trips over; the real submission after a validate run needs `--resubmit`.

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
