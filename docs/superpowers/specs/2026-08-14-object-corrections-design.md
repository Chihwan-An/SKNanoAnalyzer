# 오브젝트 보정 계층 복원 — 설계 문서

작성일: 2026-08-14

## 참고 문서

| 문서 | 내용 |
|---|---|
| `docs/porting_v13_to_v15.md` | v13→v15 프레임워크 이식 가이드 (본 작업의 상위 맥락) |
| `docs/highptmuons.pdf` | L. Thomas (ULB), 2026-03-22, EXO-25-021. 2024/2025 GE κ_b 측정 |
| MUO POG twiki "High pT" | 고pT 뮤온 ID/SF/scale/resolution 공식 권고. κ_b 2022~2023BPix 값 |
| EGM POG "Scales and Smearings (Run3)" | 전자 scale/smear 공식 레시피 및 EtDependent JSON |
| `/data9/Users/achihwan/25-020/AN-25-020/chihwan/check_systematics/` | 분석 측 보정·systematic 전수 인벤토리 (폴더별 README) |
| `/data6/.../SKNanoAnalyzer-v13/docs/HNWR_Corrections.md` | v13 구현의 source of truth |
| AN-2018/008, MUO-17-001 | GE 방법론 원 문헌 |

## 1. 배경과 목표

v13(`/data6/Users/achihwan/SKNanoAnalyzer-v13`)에 있던 오브젝트 보정 중
NanoAODv15 프레임워크에서 소실된 것들을 복원한다. 이식 가이드에서 식별한
**"nominal부터 결과가 달라지는" 4개 항목**이 대상이다.

| # | 항목 | v15 현황 |
|---|---|---|
| 1 | 고pT 뮤온 모멘텀 (TuneP + 곡률 바이어스) | 전 구간 Rochester만 적용 |
| 2 | 전자 nominal 에너지 보정 (DATA scale / MC smear) | 진입점 없음. `ElectronView::Pt()`는 raw |
| 3 | AK8 JEC 재유도 + JER/JES 변형 | 보정 lane 자체가 없음 |
| 4 | AK8 SoftDrop mass 변형 | 없음 |

부수적으로 다음 **기존 v15 결함**도 함께 고친다 (모두 조사 중 확인됨).

- `GetElectronScaleUnc`가 2024에서 동작 불가 (JSON 스키마 불일치, §5.2)
- 뮤온 트리거 SF가 HLT 키가 없는 `muon_Z.json.gz`를 가리킴 (§4.1)
- `muon_HighPt.json.gz` 미로드로 HighPt ID/Iso/RECO SF 사용 불가 (§4.1)

### 전체 보정 목록과 본 작업의 커버리지

HNWR 분석이 필요로 하는 보정 전체(사용자 정리 + `check_systematics/README.md` 인벤토리)
대비 본 작업 범위는 다음과 같다. ✅ = 본 작업, 🔵 = 이미 동작, 🟣 = HNWR 모듈(범위 밖).

| 보정 | nuisance | v15 현황 | 본 작업 |
|---|---|---|---|
| High-pT electron ID SF (HEEP) | `ElectronID` | **2024용 JSON 부재** (아래 주) | 🟣 모듈 + 물리 입력 확보 필요 |
| Electron reco SF | `ElectronReco` | 동작 | 🔵 |
| Electron trigger SF | `ElectronTrig` | 분석 자체 TnP 하드코딩 | 🟣 모듈 |
| Electron scale (DATA) | `ElectronScale` | **nominal 없음 + syst 스키마 깨짐** | ✅ §5.1, §5.2 |
| Electron smearing & resolution | `ElectronRes` | **없음** | ✅ §5.1 |
| Muon reco SF | `MuonReco` | HighPt JSON 미로드 | ✅ §4.1 |
| High-pT muon ID SF | `MuonID` | HighPt JSON 미로드 | ✅ §4.1 |
| Muon iso SF | `MuonIso` | HighPt JSON 미로드 | ✅ §4.1 |
| Muon trigger SF | `MuonTrig` | **소스에 HLT 키 0개** | ✅ §4.1 |
| Muon scale (Rochester + GE) | `MuonScale` | Rochester만, GE 없음 | ✅ §4.3 |
| Muon resolution | `MuonRes` | **없음** | ✅ §4.4 |
| TuneP 200 GeV 분기 | — (정의) | **없음** | ✅ §4.3 |
| Jet ID / FatJet ID | — (selection, SF 없음) | 동작 | 🔵 |
| Jet veto map | — (event veto) | 동작 (로직 차이 검증 필요) | 🔵 |
| AK4 JES / JER | `JES` / `JER` | 동작 (lane) | 🔵 |
| AK8 JES / JER | `JES` / `JER` (**AK4와 단일 nuisance 병합**) | **없음** | ✅ §6.1, §6.2 |
| SoftDrop mass 전파 | `JES` / `JER` | **없음** | ✅ §6.3 |
| Pileup | `Pileup` | 동작 | 🔵 |
| μR / μF scale | `ScaleWeight_muR/muF` | 동작 | 🔵 |
| α_S | `AlphaS` | 동작 (`LHEPdfWeight[101,102]`) | 🔵 |
| PDF | offline template | 동작 (`LHEPdfWeight[0..100]`) | 🔵 |
| Luminosity | datacard lnN | 분석기 밖 | — |
| LSF3 cut efficiency | `LSF` | 분석 자체 측정 | 🟣 모듈 |
| DY C / EW / R | `ZPt*`, `DYReshape*` | 없음 (전제 재검토 필요) | 🟣 모듈 |

Jet ID·FatJet ID는 SF를 적용하지 않는 selection cut이고, jet veto map은 weight가 아니라
event veto이므로 둘 다 nuisance가 없다. AK8 JES/JER은 2026-08-11자로 AK4와 **단일
nuisance로 병합**되었으므로 별도 nuisance를 만들지 않는다.

**주: 2024 HEEP ID SF는 현재 존재하지 않는다.**
`electronID_highPt.json.gz`는 CAT metadata에 있으나 **NanoAODv12 Run3 era(2022, 2022EE,
2023, 2023BPix)에만** 있고 NanoAODv15 era(2024, 2025, 2026)와 Run2에는 없다.
2024 `electron.json.gz`의 `Electron-ID-SF` WorkingPoint 목록에도 HEEP이 없다
(`RecoBelow20, Reco20to75, RecoAbove75, Veto, Loose, Medium, Tight, wp80iso, wp90iso,
wp80noiso, wp90noiso, PromptMVA-Medium, PromptMVA-Tight`).
⚠️ 여기의 `Tight`는 **cut-based Tight이지 HEEP이 아니다** — v13이 HEEP SF를 별도 파일에서
WP `"Tight"`로 읽었기 때문에 이름이 같아 혼동하기 쉽고, 잘못 쓰면 다른 SF가 적용된다.

코드가 아니라 **물리 입력의 문제**이므로 세 가지 중 선택이 필요하다:
(a) EGM에 2024 릴리스 일정 문의, (b) 2023BPix v12 값 임시 사용(v15가 뮤온 RoccoR에서
이미 쓰는 방식) + 불확실성 확대, (c) SF = 1.0 + 불확실성만(v13이 2017에 취한 방식).
HNWR 모듈 몫이며 **본 오브젝트 계층 작업의 블로커는 아니다.**

### 범위 밖 (명시적 제외)

- HNWR 분석 전용 보정(HEEP SF, LSF SF, DY C/R/EW, TheoryNormK) → 별도 HNWR 모듈
- 아날라이저 6종 이식 자체 → 본 작업 완료 후 별도 진행
- b/c-tagging SF (v15에 없으나 대상 아날라이저가 사용하지 않음)
- 2024 외 era 배선 (§8 참조)
- 빌드 및 condor 제출 (사용자가 직접 수행)

### 전제 조건 확인 완료

필요한 correctionlib JSON은 **전부 cvmfs에 이미 존재**하며, v15는 배선만 되어 있지 않다.
새로 생산해야 할 물리 입력은 없다 (κ 맵 수치 제외, §4.3).

| 입력 | 경로 (2024) | 상태 |
|---|---|---|
| AK8 JERC | `JME/Run3-24.../latest/fatJet_jerc.json.gz` | 존재. config에 주석 처리됨 |
| 뮤온 HighPt SF | `MUO/Run3-24.../latest/muon_HighPt.json.gz` | 존재. 미로드 |
| 전자 scale/smear | `EGM/Run3-24.../latest/electronSS_EtDependent.json.gz` | 로드됨. 코드가 옛 스키마 참조 |
| TuneP 브랜치 | `Muon_tunepRelPt`, `Muon_highPtId`, `Muon_nTrackerLayers` | 존재 |

## 2. 아키텍처

### 2.1 접근법: AK4 lane 패턴 미러링

기존 `JetSoA`가 쓰는 **lazy lane** 패턴을 다른 오브젝트에 동일하게 적용한다.
아날라이저는 `fj.SmearedPtNominal()`처럼 읽고, 첫 접근 시 View가
`store->ensureNominal()`을 호출 → SoA에 등록된 콜백이 `AnalyzerCore`의 populate
함수를 실행 → 전 오브젝트의 lane을 한 번에 채움 → 이후 캐시.
**사용하지 않으면 JSON 평가가 일어나지 않는다.**

이 방식을 택한 이유:
1. 기존 AK4 관용구와 100% 일치 — upstream PR 리뷰에서 새 패턴 도입 논란이 없다
2. 뮤온은 이미 `PopulateMuonMomentum()`이 존재해 확장이지 신규가 아니다
3. AK8은 JES·JER·SDMass 때문에 pt 다중화가 필수라, 다른 방식을 택해도 결국 lane 유사물을 만들게 된다

### 2.2 계층 배치

`scripts/check_layers.sh` 규칙을 준수한다 (AnalyzerTools는 AnalyzerFramework
헤더 include 금지, SKNanoCore/DataFormats/AnalyzerTools에서 `exit()`/`abort()` 금지).

| 계층 | 책임 | 주요 파일 |
|---|---|---|
| **AnalyzerTools** (`MyCorrection`) | correctionlib JSON 평가자. 상태 없는 순수 함수 | `MyCorrection.h`, `MyCorrectionLeptons.cc`, `MyCorrectionJets.cc`, `MyCorrection.cc`(era config) |
| **DataFormats** | 보정 lane 저장 + lazy 접근자. 계산 로직 없음 | `MuonView.h`, `ElectronView.h`, `FatJetView.h` |
| **AnalyzerFramework** (`AnalyzerCore`) | populate 함수: 평가자 호출 + gen 매칭 + RNG | `AnalyzerCoreObjects.cc`, `AnalyzerCoreJets.cc`, `AnalyzerCore.h` |

### 2.3 하위 호환 원칙

**기존 API의 기본 동작을 바꾸지 않는다.** 고pT 뮤온 보정처럼 일부 분석에만
해당하는 것은 별도 opt-in 함수로 노출한다. 이유는 이 저장소가 CMSSNU 공용이고
다른 분석이 기존 동작에 의존하고 있기 때문이다.

예외적으로 기본 동작이 바뀌는 것은 **현재 상태가 잘못된 경우**뿐이다.
아래 5개가 전부이며, 모두 PR 설명에 영향 범위를 명시한다.

| 변경 | 영향 | 근거 |
|---|---|---|
| 전자 nominal scale/smear 적용 (§5.1) | 전자를 쓰는 모든 분석 | 현재 미적용은 EGM Run3 권고 위반 |
| `GetElectronScaleUnc` 스키마 교정 (§5.2) | ElectronScale syst 사용자 | 2024에서 현재 예외 발생 |
| 뮤온 트리거 SF 소스 교정 (§4.1) | 뮤온 트리거 SF 사용자 | 현재 소스에 HLT 키가 0개 |
| 뮤온 SF 평가 pt: raw → 보정 후 (§4.2) | 뮤온 SF를 쓰는 모든 분석 | POG 규약은 보정 후 pt 기준 |
| AK8 JEC 재유도 도입 (§6.4) | AK8을 쓰는 모든 분석 | 현재는 프로덕션 시점 JEC 고정 |

뒤의 두 항목은 버그라기보다 **정확도 개선**이며 nominal 수율을 바꾼다.
리뷰에서 반대가 있으면 opt-in으로 후퇴할 수 있도록 구현을 분리해 둔다.

## 3. 컴포넌트 개요

```
MyCorrection  (평가자)
├── 뮤온:   GetMuonHighPtScale(eta, phi, charge, tunePpt, syst)   [신규]
│           GetMuonHighPtSmearFactor(p, eta, seed, syst)          [신규, v13 이식]
│           GetMuonIDSF/TriggerSF  → muon_HighPt.json 라우팅       [수정]
├── 전자:   GetElectronScaleCorr(...)                             [신규, 새 스키마]
│           GetElectronSmearValue(...)                            [신규, 새 스키마]
│           GetElectronScaleUnc(...)                              [수정, 스키마 교정]
└── AK8:    GetFJESSF / GetFJESUncertainty / GetFJER / GetFJERSF  [신규]

DataFormats  (lane + 접근자)
├── MuonSoA/View:     TunePPt(), HighPtPt(), HighPtScaleUp/Down()
├── ElectronSoA/View: CorrectedPt(), ScaleUp/Down(), SmearUp/Down()
└── FatJetSoA/View:   CorrectedPt/Mass, SmearedPt{Nominal,Up,Down},
                      JesPt{Up,Down}, SDMass 변형 lane

AnalyzerCore  (populate + 셀렉터)
├── PopulateMuonMomentum()        [확장: 고pT 분기]
├── PopulateElectronMomentum()    [신규]
├── SmearFatJetViews()            [신규]
└── SelectHighPtMuonIndices()     [신규 opt-in 셀렉터]
    SelectFatJets(..., jesVar, jerVar)  [시그니처 확장]
```

## 4. 뮤온

### 4.1 HighPt SF 배선 (이견 없음)

`MyCorrection`에 `cset_muon_highpt`를 추가하고 `muon_HighPt.json.gz`를 로드한다.
2024 JSON에 v13이 쓰던 4개 키가 모두 존재함을 확인했다.

| 용도 | 키 |
|---|---|
| MuonID | `NUM_HighPtID_DEN_GlobalMuonProbes` |
| MuonReco | `NUM_GlobalMuons_DEN_TrackerMuonProbes` |
| MuonTrig | `NUM_HLT_DEN_HighPtLooseRelIsoProbes` |
| MuonIso | `NUM_probe_LooseRelTkIso_DEN_HighPtProbes` |

- `GetMuonIDSF`/`GetMuonTriggerSF`가 HighPt 계열 키를 받으면 `cset_muon_highpt`를 조회하도록 라우팅
- era config의 `json_muon_trig_sf`를 `muon_Z.json.gz` → `muon_HighPt.json.gz`로 교정
  (`muon_Z.json.gz`에는 HLT 키가 0개이므로 현재 트리거 SF는 사용 불가 상태)
- v13의 `GetMuonHighPtRECOSF` 복원: p를 `[50.1, 1499]`로 clamp

### 4.2 SF 평가 pt 교정

v15는 SF를 `MiniAODPt()`(raw)에서 평가하나 v13은 보정 후 pt를 사용했다.
보정 후 pt로 통일한다. 고pT 셀렉터를 쓰는 경우 `HighPtPt()`가 평가 pt가 된다.

### 4.3 고pT 모멘텀 — opt-in 경로

**기본 경로는 무변경.** `SelectMuonIndices`와 `MuonView::Pt()`는 지금처럼
전 구간 Rochester를 유지한다.

신규 opt-in API:

```cpp
// DataFormats/include/MuonView.h
float TunePPt() const;        // Muon_pt * Muon_tunepRelPt (raw TuneP)
float HighPtPt() const;       // 아래 규칙에 따른 고pT 보정 pt
float HighPtScaleUp() const;  // κ + σ
float HighPtScaleDown() const;// κ - σ

// AnalyzerFramework/include/AnalyzerFramework/AnalyzerCore.h
std::vector<std::size_t> SelectHighPtMuonIndices(
    const MuonViewCollection &muons, const MuonView::MuonID ID,
    const float ptmin, const float fetamax) const;
```

`HighPtPt()` 규칙 (MUO POG "High pT: Momentum Scale" approach 1):

**regime 판정은 보정 전 TuneP pT(`Muon_pt × Muon_tunepRelPt`)로 하고, 결과를 lane에 latch한다.**
보정 후 pT로 다시 판정하면 GE/smearing 때문에 200 GeV를 넘나든 뮤온이 오분류된다
(v13이 `SetHighPtRegime()`을 둔 이유).

| regime | 사용 momentum | DATA | MC |
|---|---|---|---|
| TuneP pT < 200 | **raw `Muon_pt`** (tracker/PF). TuneP 쓰지 않음 | Rochester `kScaleDT` | Rochester `kSpreadMC`/`kSmearMC` |
| TuneP pT ≥ 200 | **TuneP pT** | GE 곡률 바이어스 보정 | GE nominal 없음 + 고pT resolution smearing (§4.4) |

즉 저pT 구간은 기존 `correctedPt`(Rochester)를 그대로 쓰고, 고pT 구간만 TuneP + GE로 간다.
GE 보정식은 v13 `GetMuonGEScaledPt`와 동일하다:

```
pT_corr = pT / (1 - q · κ_b(η,φ) · pT[TeV])
```

κ_b는 1/TeV 단위 additive curvature bias이며 **DATA nominal에만 적용**한다
(AN-2018/008: κ_b는 MC를 데이터처럼 보이게 하는 양이므로 바이어스는 데이터에 속한다).
twiki 일반 처방(approach 1)은 "pT>200은 무보정, GE는 불확실성으로만"이지만,
**Run 3 탭이 이를 명시적으로 뒤집어** "correct the transverse momentum of high pT muons"를
권고하므로 nominal 보정을 적용한다.

**MuonScale nuisance는 MC에 ±σ만 싣는다.** 데이터를 κ로 보정했으므로 그 측정의
불확실성은 σ이고, 데이터카드는 MC 템플릿만 흔들 수 있으므로 상대적 data/MC 이동은 동일하다.

| | nominal | up/down |
|---|---|---|
| DATA | `GE(κ)` 적용 | 변형 없음 (nominal과 동일) |
| MC | 무보정 TuneP | `pT/(1 − q·(±σ)·pT[TeV])` — **κ 없이 σ만** |

MC 변형에 κ 전체를 넣으면 nuisance가 "보정의 크기"가 되어 "보정 오차의 크기"보다
훨씬 커진다. TeV 뮤온에서는 κ·pT가 크므로 차이가 상당하다.
전용 평가자 `GetMuonGESigmaShiftedPt`로 분리했다.

**알려진 한계:** 두 regime이 서로 다른 momentum estimator를 쓰므로 200 GeV에서 구조적으로
kink가 가능하다. 190–210 GeV 구간 pT 스펙트럼 연속성 확인은 분석 단계 TODO다.

**κ_b 맵 수치 출처.**
MUO POG twiki "High pT: Momentum Scale"이 2022/2022EE/2023/2023BPix의 κ_b(오차 포함)를
3φ × 6η 격자로 게시하고 있으며, **v13의 `GE_KAPPA` 테이블은 이 값과 일치한다**
(2022 preEE 첫 행 `-0.16 (0.10), -0.03 (0.05), -0.05 (0.04), 0.00 (0.04), 0.07 (0.06), -0.06 (0.11)` 확인).
따라서 2022~2023BPix는 v13 테이블을 그대로 가져오면 된다.

twiki는 "The values for 2024 and 2025 can be found in these tables"라며 별도 링크를 가리키고,
`docs/highptmuons.pdf` (L. Thomas, ULB, 2026-03-22, EXO-25-021)가 그 측정을 담고 있다.
**2024 값은 격자 정의가 twiki(3φ×6η)와 다르므로 단순 복사가 불가능하다** (§9 미해결 항목 1).

Run3 처방이 "불확실성이 아니라 보정으로 적용"임은 twiki와 PDF 요약이 모두 확인한다
(twiki: "the recommendation for Run3 is to correct the transverse momentum of high pT muons,
as done for the medium pT regime, using the values derived from the GE method").
`docs/highptmuons.pdf` (L. Thomas, ULB, 2026-03-22, EXO-25-021)가 2024/2025 측정을 담고 있다.

- p.11 — GEM 1단계(single muon, φ modulation): **κ ± σ**, 촘촘한 격자
- p.16 — GEM 2단계(dimuon, absolute η correction): 추가 보정, "2024/2025는 대체로 0과 무관하지 않음"
- p.12 — medium-pT 보정에서 유도한 additive bias: **중심값만, σ 없음**. p.11의 교차검증용
  ("Results very similar with medium pT scale corrections for 2022-2024")
- p.24 요약 — "Should consider applying the corrections instead of treating them as an uncertainty"
  → 불확실성이 아니라 **보정으로 적용**하는 것이 권고

MuonScale systematic에는 σ가 필요하므로 p.11/16 값이 필요하다. 사용자는 p.12를
지목했고 두 값은 서로 일치하는 것으로 발표에 제시되어 있다.

설계상 **κ 맵은 교체 가능한 데이터로 분리**한다 (era → (η,φ) 격자 → `{kappa, sigma}`).
v13은 `MyCorrection.cc` 내부 `GE_KAPPA` 상수 테이블(3φ×6η)로 두었으므로 같은 방식을 쓰되,
격자 크기를 PDF 격자에 맞춰 일반화하고 출처 주석을 단다. 수치 확정 전까지는
**2024 항목을 비워 두고, 맵이 없는 era는 보정 없이 TuneP pt를 그대로 반환**한다
(v13과 동일한 fallback 동작).

가드 (v13 실전 경험 반영): 분모 `|1 - q·κ·pT_TeV| < 0.1`이면 보정을 포기하고 raw
TuneP pt를 반환한다. TeV 뮤온에서 발산하기 때문이다.

### 4.4 고pT 해상도 스미어링과 MuonRes systematic

**MC 전용, 고pT regime(TuneP pT ≥ 200) 전용.** nominal과 systematic이 서로 다른 양이다.

```
factor  = 1 + Gaus(0, σ(p, η) · f)
σ(p, η) = a0 + a1·p + a2·p² + a3·p³          // 변수는 pT가 아니라 전체 운동량 p
```

- **nominal**: σ 다항식 계수와 f는 **era·η(barrel/forward)별 하드코딩 테이블**이다.
  f는 `sqrt(smearfac² − 1)` 변환값(10% → 0.458, 5% → 0.320)이며,
  era에 따라 barrel f = 0(스미어링 없음)인 경우도 있다.
- **systematic (MuonRes)**: era·η 무관 **평탄 10%** (`f_syst = 0.4583`).
  독립 가우시안이므로 quadrature 합성한다.

  ```
  f_up   = sqrt(f_nom² + f_syst²)
  f_down = sqrt(max(f_nom² − f_syst², 0))
  ```

  **같은 seed**로 pull을 공유해 coherent shift로 만든다(독립 re-smearing이면 nominal 위에
  해상도가 또 더해져 double-count). floor가 0인 이유는 가우시안 convolution이 해상도를
  나쁘게만 만들 수 있기 때문이며, 따라서 **f_nom = 0인 era·η에서 MuonRes는 one-sided**가 된다.

- **저pT 가드 필수**: 평가 함수 자체에 pT threshold가 없으므로 §4.3에서 latch한 regime으로
  저pT 뮤온을 건너뛴다. 가드가 없으면 저pT 뮤온이 흔들리고, 그 구간 해상도는 Rochester가
  이미 처리하므로 `MuonScale`과 이중계산이 된다.

**⚠ 2024는 nominal 스미어링 맵이 없다.** MUO POG twiki가 2024 momentum resolution을
"Work In Progress"로 명시하고, "non closure를 각 분석의 phase space에서 직접 평가해
MC 스미어링 필요 여부를 판단하라. 다른 연도처럼 불확실성을 적용하는 것이 좋은 proxy"라고
권고한다. 따라서 2024는 **nominal no-op + 불확실성만** 적용하고, 실제 스미어링 필요 여부는
분석 단계에서 결정한다. 구현은 era 테이블이 비면 자동으로 no-op이 되는 구조라 별도 분기가 필요 없다.

**외삽 한계:** 해상도 측정의 pT bin은 800 GeV에서 끝나고 GE scale은 Z peak 기반이다.
WR 시그널 영역(TeV)에서는 둘 다 외삽이며, MUO POG는 **500 GeV 이상 뮤온을 쓰는 모든 분석에
별도 sanity check**(측정된 scale bias만큼 pT를 수동으로 shift해 결과 영향 확인)를 요구한다.
본 작업 범위 밖이며 분석 단계에서 수행해야 한다.

## 5. 전자

### 5.1 nominal 보정 신설

`PopulateMuonMomentum`과 대칭인 `PopulateElectronMomentum()`을 신설하고
`ElectronSoA`에 lane을 추가한다. 공식 EGM Run3 레시피를 그대로 따른다.

- **DATA**: `correctedPt = pt * scale`
  compound correction `Scale`, 인자 `("scale", run, ScEta, r9, pt, seedGain)`
- **MC**: `correctedPt = pt * (1 + smear · N(0,1))`
  correction `SmearAndSyst`, 인자 `("smear", pt, r9, ScEta)`
  난수는 결정론적 시드(브랜치 `event`(ULong64_t), `ScEta`, `phi` 기반 — v13 `ObjectSmearSeed` 방식.
  v13의 `EventNumber`는 v15에서 `event`로 이름이 다르다)

변형 lane (레시피의 적용 대상 구분을 정확히 지킨다):

| lane | 계산 |
|---|---|
| `ScaleUp/Down()` | **스미어링된** MC pt × `scale_up`/`scale_down` |
| `SmearUp/Down()` | **원본** MC pt × `(1 + smear_up/down · N(0,1))`, **nominal과 같은 난수** |

`SmearAndSyst` 하나에서 `smear`/`scale_up`/`scale_down`/`smear_up`/`smear_down`를
모두 얻으므로, v13식 "variation ÷ nominal 비율" 트릭은 불필요하다.

Run2는 NanoAOD에 이미 캘리브레이션이 적용되어 있으므로 nominal 보정을 하지 않는다
(v13과 동일).

### 5.2 `GetElectronScaleUnc` 스키마 교정 (버그 수정)

현재 `MyCorrectionLeptons.cc:128-137`은 옛 JSON 스키마를 참조해 2024에서 동작하지 않는다.

| | 2024 JSON 실제 | 현재 코드 |
|---|---|---|
| 키 | compound `Scale` / correction `SmearAndSyst` | `EGM_keys["2024"]+"_ScaleJSON"` (존재하지 않음) |
| 접근 | `Scale`은 compound_correction | `cset_electron_variation->at(key)` (simple accessor) |
| 인자 | `(syst, run, ScEta, r9, pt, seedGain)` | `("total_uncertainty", gain, run, scEta, r9, pt)` |
| syst 문자열 | `"scale_up"` / `"scale_down"` | `"total_uncertainty"` |

nominal은 early-return이라 현재 job이 죽지는 않으나, ElectronScale systematic을 켜면
예외가 난다. §5.1의 신규 평가자로 대체한다.
**v13 코드도 옛 스키마용이므로 복사로는 해결되지 않는다.**

### 5.3 GAP(크랙) 전자 처리

v13 `GetAllElectrons()`는 GAP 전자를 컬렉션에서 제거했으나 v15
`GetAllElectronViews()`는 제거하지 않는다. 기본 동작 변경은 다른 사용자에게 영향을
주므로 하지 않는다. 대신 `ElectronViewCollection`에 이미 존재하나 사용되지 않는
`skipCrack` 생성자 인자를 `GetAllElectronViews(bool skipCrack = false)`로 노출해
호출자가 선택할 수 있게 한다.

`PassID` 경유 판정은 GAP을 이미 거부하므로 영향이 없고, `"NOCUT"` 기반 loose 카운팅에만
해당한다는 점을 문서화한다.

### 5.4 물리 한계 (문서화 대상)

EGM 문서는 이 보정이 **"may be ineffective at very high pT (hundreds of GeV)"**라고
명시한다. WR 분석의 HEEP 전자(130 GeV~수 TeV)는 정확히 그 영역이므로, 적용하되
신뢰도 한계를 코드 주석과 이식 가이드에 남긴다.

## 6. AK8 팻젯

### 6.1 JSON 배선

era config의 `json_jerc_fatjet` 주석을 해제하고 `fatJet_jerc.json.gz`를 연결한다.
2024 키는 AK4와 명명 규칙이 동일하다.

| 용도 | 키 |
|---|---|
| JEC (MC/DATA) | compound `Summer24Prompt24_V5_{MC,DATA}_L1L2L3Res_AK8PFPuppi` |
| JES 불확실성 | `Summer24Prompt24_V5_MC_Total_AK8PFPuppi` |
| JER 해상도 | `Summer24Prompt24_JRV2_MC_PtResolution_AK8PFPuppi` |
| JER SF | `Summer24Prompt24_JRV2_MC_ScaleFactor_AK8PFPuppi` |

`MyCorrection`에 `GetFJESSF`, `GetFJESUncertainty`, `GetFJER`, `GetFJERSF`를
추가한다 (AK4 대응 함수와 동일한 시그니처 규약: 마지막 인자가
`const variation syst, const TString &source = "total"`).

### 6.2 lane 구조

`FatJetSoA`에 `JetSoA`와 동일한 lane 세트와 lazy 콜백을 추가한다.

```
correctedPt / correctedMass
smearedPtNominal / smearedMassNominal
smearedPtUp / smearedPtDown / smearedMassUp / smearedMassDown
jesPtUp / jesPtDown / jesMassUp / jesMassDown
ensureNominal() / ensureJerVariations()
```

`AnalyzerCore::SmearFatJetViews()`가 콜백으로 등록되며, gen 매칭은
`GenJetAK8_*` raw 브랜치를 직접 사용한다 (`GenJetAK8ViewCollection`은 없으며,
이번 범위에서 신설하지 않는다 — YAGNI).

`SelectFatJets`에 AK4와 동일한 `jesVariation`/`jerVariation` 인자를 추가한다.
AK4와 마찬가지로 **JES와 JER을 동시에 non-nominal로 요청하면 예외**를 던진다.

### 6.3 SoftDrop mass 변형

`SDMass()`는 현재 raw `FatJet_msoftdrop`이다. v13 `VarySoftDropMass`는 서브젯 기반으로
JES/JER 변형을 적용했고, **nominal 패스에서도 호출**되어 서브젯 JER nominal
스미어링을 넣었다. 즉 이것이 빠지면 Central 결과가 달라진다.

**AK8 JERC를 m_SD에 적용하면 안 된다.** AK8 보정은 `FatJet_pt`/`FatJet_mass`를 고치는
것이고 `FatJet_msoftdrop`은 건드리지 않는다 — 다른 관측량이다. JME 권고(CMS Talk,
"AK8 Puppi Jet JERC for Soft Drop Mass", 2026-01)는 **soft-drop 서브젯에 AK4 JERC를
적용해 m_SD를 다시 계산**하는 것이다.

```
m_SD → m_SD × (sj1' + sj2').M() / (sj1 + sj2).M()
```

NanoAOD의 `SubJet_pt`에 이미 서브젯 JEC가 들어있고 `FatJet_msoftdrop`이
`(sj1+sj2).M()`과 일치하므로(v13 측정: 240/240 fatjet에서 중앙값 −0.0035%, 전부 |diff|<0.1%)
비율로 쓰는 것이 타당하며, 같은 분모를 모든 pass에서 써서 잔차도 상쇄된다.

lane 구성 `sdMassNominal`, `sdMassJes{Up,Down}`, `sdMassJer{Up,Down}`:

| pass | 서브젯에 적용 |
|---|---|
| nominal | JES nom + **JER nom** |
| JES up/down | JES var + JER nom |
| JER up/down | JES nom + JER var |

**nominal JER 스미어링은 필수다.** NanoAOD의 msoftdrop은 서브젯 JEC는 있지만 JER
스미어링이 없어서, 이것 없이는 m_SD가 무스미어링 기준이 되고 JER 변형이 폭을 **더하기만**
할 수 있어 Up과 Down이 **같은 방향으로** 움직인다. v13 2022 측정: m_SD 평균 +1.19 GeV(Up),
+0.72 GeV(Down) — 반면 JES는 +1.16/−1.21로 정상적으로 nominal을 감쌌다.
데이터카드가 one-sided "불확실성"을 보게 되는 상황이다.

서브젯별 가우시안 draw는 pass 간 공유해 coherent shift로 만든다.
DATA는 JERC가 MC 전용이므로 no-op이다.
v13은 `SubJet_area` 부재로 raw pT에서의 완전한 JEC 재적용을 못 했으나
**v15에는 `SubJet_area`가 있어** 필요하면 확장 가능하다.

### 6.4 JEC 재유도 관련 주의

v13 `GetAllFatJets()`는 raw pt에서 JEC를 재유도했다. v15는 NanoAOD 프로덕션 시점
JEC가 반영된 저장값을 쓴다. CAT JSON의 JEC가 프로덕션보다 새 버전이면
**nominal AK8 pt 자체가 v13과 달라진다.** 재유도를 도입하므로 이 차이는 해소되지만,
현재 v15 결과와도 달라진다는 점을 PR에 명시한다.

## 7. 에러 처리

- `check_layers.sh`가 SKNanoCore/DataFormats/AnalyzerTools에서 `exit()`/`abort()`를
  금지하므로 **모든 실패는 예외로 처리**한다: JSON/키 부재 → `SKNano::ConfigError`,
  평가 실패 → `SKNano::CorrectionError` (기존 `safeEvaluate`가 이미 이 규약을 따름)
- 기본 `FailurePolicy`가 `FailFast`이므로 예외 하나가 job을 죽인다. 따라서
  **데이터 품질 문제로 인한 개별 오브젝트 이상은 예외가 아니라 경고 후 보정 생략**으로
  처리한다. v13에서 실제로 문제가 되었던 두 경우를 처음부터 반영한다:
  - `seedGain ∉ {1, 6, 12}`: correctionlib이 throw함 (2023 Muon 데이터 250만 건 중 O(1)건).
    경고 1회 출력 후 scale 1.0 반환
  - κ 보정 분모가 0 근처: TeV 뮤온에서 발산. 경고 후 raw TuneP pt 반환
- 맵/JSON이 없는 era는 **보정 없이 통과**시킨다 (throw하지 않음). 2024 외 era에서
  기존 동작이 깨지지 않도록 하기 위함이다

## 8. era 지원 범위

본 작업은 **2024만 배선**한다. `MyCorrection::GetEraConfig`가 현재 2024 블록만
가지고 있고, v15 RNTuple 샘플도 2024만 존재하기 때문이다.

코드는 era별 분기 구조를 유지해 다른 era 추가가 config 블록 추가만으로 되도록 한다.
κ 맵도 era 키 기반이므로 2022/2023 값은 v13 `GE_KAPPA`에서 가져올 수 있으나,
**격자 정의가 다르므로(v13 3φ×6η vs PDF 촘촘한 격자) 단순 복사는 불가**하다.

## 9. 미해결 항목 (구현 전 확정 필요)

1. **2024 κ_b 맵의 수치와 격자** — `highptmuons.pdf` p.11(κ±σ, GEM 본 측정)과
   p.12(중심값만, medium-pT 유도 교차검증) 중 어느 것을 쓸지. MuonScale systematic에는
   σ가 필요하므로 p.11이 필요하다. 격자도 twiki의 3φ×6η와 달라 테이블 구조를
   일반화해야 한다. 2022~2023BPix는 twiki/v13 값으로 확정.
2. **p.16 GEM 2단계(dimuon absolute η) 보정 적용 여부** — "2024/2025는 대체로 0과
   compatible"이라 생략 가능할 수 있으나 확인 필요.
3. **2024 뮤온 해상도 스미어링 필요 여부** — POG가 WIP로 두고 분석별 non-closure 평가를
   요구한다(§4.4). 코드는 no-op으로 두고 분석 단계에서 결정한다.

세 항목 모두 **코드 구조에 영향을 주지 않고 데이터 테이블/설정만 바뀐다.**
구현을 막지 않으므로 2024 맵을 비워 둔 채 진행한다.

## 10. 검증 계획

이 저장소에는 C++ 단위 테스트 프레임워크가 없다 (`check_layers.sh` 셸 검사만 존재).
사용자가 직접 빌드하므로 다음으로 검증한다.

1. `scripts/check_layers.sh` 통과 (계층 경계 + `exit()` 금지)
2. 사용자 빌드로 컴파일 확인
3. 보정별 sanity check (구현 시 각 단계에서 수행):
   - 뮤온: `TunePPt() < 200`에서 `HighPtPt() == correctedPt`인지 (경로 분기 정확성)
   - 전자: MC에서 nominal smear가 평균 1 근처, DATA scale이 O(1%)인지
   - AK8: `CorrectedPt()`가 raw pt 대비 합리적 범위인지, JER up/down이 nominal을 감싸는지
4. v13과의 비교는 **분포 수준**으로만 가능하다. 스미어링 난수·SF 평가 pt·veto map
   로직이 달라 이벤트 단위 일치는 기대할 수 없다.

## 11. 구현 순서

의존성과 위험도 순으로 진행한다. 각 단계는 독립적으로 빌드 가능하다.

의존성과 위험도 순으로 진행하며, **각 단계 끝에 빌드 체크포인트**를 둔다.
빌드는 사용자가 직접 수행하므로, 각 단계 완료 시 작업을 멈추고 빌드를 요청한 뒤
결과(성공 또는 컴파일 에러)를 받아 다음 단계로 넘어간다.

| 순서 | 대상 | 근거 | 빌드 체크포인트 |
|---|---|---|---|
| 1 | 뮤온 HighPt SF 배선 (§4.1, §4.2) | 가장 단순. JSON 로드 + 키 라우팅뿐이고 lane 변경 없음 | AnalyzerTools만 |
| 2 | 뮤온 고pT 모멘텀 opt-in (§4.3, §4.4) | 기존 populate 확장. κ 맵은 비워 둔 채 구조만 | DataFormats + Framework |
| 3 | 전자 nominal + 스키마 교정 (§5) | 신규 populate이나 뮤온 패턴을 그대로 따름 | DataFormats + Framework |
| 4 | AK8 JERC lane (§6.1, §6.2) | 가장 큼. DataFormats 변경 최대 | 전체 |
| 5 | AK8 SDMass 변형 (§6.3) | 4에 의존. 서브젯 처리로 별도 분리 | 전체 |

1~3과 4~5는 서로 독립이므로 필요하면 순서를 바꿀 수 있다.

각 단계에서 dictionary 생성(LinkDef/ROOT dictionary)이 관련된 변경은
`DataFormats` 헤더 수정 시 함께 발생하므로, 2·3·4단계는 빌드 실패 시
dictionary 재생성 문제인지 먼저 확인한다.
