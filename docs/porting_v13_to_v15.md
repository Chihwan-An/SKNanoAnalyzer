# v13 아날라이저 → NanoAODv15 프레임워크 이식 가이드

대상: `/data6/Users/achihwan/SKNanoAnalyzer-v13` 의 아래 6개 아날라이저를
현재 저장소(`/data9/Users/achihwan/nano15/SKNanoAnalyzer`, v2.0.0)로 이식.

| 아날라이저 | 줄 수 | 역할 | 이식 난이도 |
|---|---|---|---|
| `DYGenZpT` | 214 | Gen-level DY Z-pT 스펙트럼 (C(pT) 도출용) | **쉬움** |
| `jet_pt_reweight` | 498 | DY CR jet-pT 히스토그램 (R(pT) 도출용, Reproduce20_002_copy 상속) | 중간 |
| `Reproduce20_002_copy` | 4156 | 메인 cut-based WR 분석 (전체 systematics) | **어려움** |
| `HNWR_BDT_presel` | 1591 | BDT ntuple 생산 (구버전) | 어려움 |
| `HNWR_BDT_presel_3SR` | 2236 | BDT ntuple 생산 (3-SR, 현행) | **어려움** |
| `WRGenRecoMass` | 4779 | Reproduce20_002_copy 사본 + LHE 질량/fail-cutflow | 어려움 |

---

# 진행 현황 (2026-08-14 기준)

## ✅ 완료 — 오브젝트 보정 계층

이 문서가 처음 지적한 **프레임워크 블로커 6개 중 4개는 해결되었다.**
구현은 브랜치 `object-corrections-v13-parity`, 설계는
`docs/superpowers/specs/2026-08-14-object-corrections-design.md`.
**신규 API 목록은 §7.1에 있으며, 남은 이식 작업은 그것을 써야 한다.**

| 항목 | 결과 |
|---|---|
| AK8 JES/JER/SDMass 인프라 부재 | ✅ `fatJet_jerc.json.gz` 배선 + AK4와 동일한 lane 구조. SDMass는 **서브젯 기반**(AK4 JERC)으로 구현 |
| 고pT 뮤온 모멘텀 소실 | ✅ TuneP 200 GeV regime latch + GE scale. **opt-in**(`SelectHighPtMuonIndices`)이라 기존 동작 무변경 |
| HighPt SF 배선 부재 | ✅ `muon_HighPt.json.gz` 로드, ID/Iso/RECO/Trigger 4종 배선 |
| 전자 nominal scale/smear 부재 | ✅ `PopulateElectronMomentum` 신설. `ElectronView::Pt()`가 보정 후 값 |
| (보너스) `GetElectronScaleUnc` 2024 미동작 | ✅ EtDependent 스키마로 교체 |
| (보너스) 뮤온 트리거 SF 소스 오류 | ✅ HLT 키가 0개인 `muon_Z` → `muon_HighPt` |

## ✅ 완료 — 아날라이저 이식

| 아날라이저 | 상태 |
|---|---|
| `DYGenZpT` (214줄) | ✅ `/data9/Users/achihwan/nano15/HNWR/` 모듈로 이식 |

## 🔄 진행 중 — 나머지 5개 이식

`Reproduce20_002_copy`는 클래스명을 **`HNWR`**로 변경해 이식 중.
전부 `/data9/Users/achihwan/nano15/HNWR/` 모듈에 들어간다.

| 아날라이저 | 줄 수 | 특이사항 |
|---|---|---|
| `HNWR` (구 `Reproduce20_002_copy`) | 4156 | 메인. 전체 systematics |
| `WRGenRecoMass` | 4779 | 위의 사본 + LHE 질량/fail-cutflow |
| `HNWR_BDT_presel_3SR` | 2236 | **TTree → RNTuple 출력 재작성** |
| `HNWR_BDT_presel` | 1591 | 위와 동일 |
| `jet_pt_reweight` | 498 | `HNWR` 상속 |

---

# 남은 작업

## 1. 물리 입력 부재 — 코드로 해결 불가

| 항목 | 상태 | 영향 |
|---|---|---|
| **2024 GE κ_b 맵** | twiki 본문에 없음(링크로만), `docs/highptmuons.pdf` p.11에 있으나 격자가 다름 | **2024에서 GE 보정이 no-op** |
| **2024 뮤온 해상도 맵** | MUO POG가 Work In Progress로 명시 | **2024에서 고pT 스미어링이 no-op** |
| **2024 HEEP ID SF** | `electronID_highPt.json.gz`가 NanoAODv12 era에만 존재 | 전자 ID SF = 1.0 |

앞의 둘은 **코드 구조는 완성되어 있고 테이블만 비어 있다.** κ 맵은 격자를 데이터로 받도록
일반화해 두었으므로 수치만 넣으면 동작한다.
HEEP SF는 (a) EGM에 릴리스 문의, (b) 2023BPix v12 값 임시 사용, (c) 1.0 + 불확실성 중 선택 필요.

## 2. 셀렉션 변경 — 확정, 미적용

`/data9/Users/achihwan/nano15/HNWR/PENDING_SELECTION_CHANGES.md` 참조.
이식이 끝나는 대로 5개 파일에 일괄 적용한다.

- 전자 lead pT **130 → 60** (뮤온과 동일)
- 전자 트리거에 **`HLT_Ele30_WPTight_Gsf` OR 추가** (2개 → 3개)
- 전자 trigger-safe pT **118 → 35**

세 변경은 연동된다: safe 35 < lead 60이 되어 lead pT가 binding이 되고 dead zone이 사라진다.
적용 시 `HLT_Ele30_WPTight_Gsf`가 각 era `HLT_Path.json`에 실제로 있는지 확인할 것 —
없으면 `PassTrigger`가 경고만 찍고 `false`를 반환해 **조용히 트리거가 빠진다.**

## 3. 검증 — 미실행

컴파일은 통과했으나 **실제 이벤트로 돌려본 적이 없다.** 확인해야 할 것:

- 저pT 뮤온에서 `HighPtPt() == Pt()` (regime 분기가 새면 Rochester 이중 보정)
- MC에서 `SDMassNominal() != SDMass()` (서브젯 JER nominal이 실제로 적용됐는지)
- JER/JES up·down이 nominal을 감싸는지 (한쪽으로 몰리면 one-sided nuisance 버그)
- 전자 보정이 데이터에서 O(1%)인지

`/data9/Users/achihwan/nano15/ObjectCorrectionCheck/`에 점검용 아날라이저를 만들어 두었으나,
ROOT 매크로가 Cling 파싱 단계에서 실패해 아직 못 돌렸다 (원인 미규명, include 경로는 아님).
정규 job도 같은 매크로 방식이므로 **이식한 아날라이저를 돌릴 때 같은 문제가 재현되는지 확인 필요.**

## 4. 남아있는 프레임워크 블로커 2개

1. **입력이 RNTuple 전용** — v13의 NanoAODv12/v13 TTree 샘플은 읽을 수 없다.
   NanoAODv15 샘플이 있는 era에서만 돌릴 수 있다.
2. **era가 2024만 배선됨** — `MyCorrection::GetEraConfig`에 `if (era == "2024")` 하나뿐.
   다른 era는 correction 경로, `HLT_Path.json`, golden JSON을 직접 추가해야 한다.

## 5. DY 보정 체계 재검토 필요 (범위 밖이나 중요)

v15 2024 DY 샘플은 `DYto2Mu_..._powheg-pythia8` — **POWHEG NLO 질량-binned**이고,
v13이 쓰던 `DYMLL` HT-binned **LO**가 없다. 따라서:

- `C(pT) = N_NLO/N_LO`는 **LO 샘플을 고치는 보정**이라 NLO 샘플에 적용하면 이중 계산
- `IsDYSample()`이 `MCSample.Contains("DYMLL")`이라 **v15 샘플명에 매칭되지 않아
  DY 보정 전체가 조용히 꺼진다**
- `DYGenZpT`와 `jet_pt_reweight`은 각각 C와 R을 유도하는 아날라이저인데 C의 전제가 사라졌다
  (R은 데이터 기반이라 유효하나 재유도 필요, EW 보정도 유효)
- **WR/LRSM 시그널 샘플이 v15 2024 목록에 없다** → `WRGenRecoMass`는 돌릴 대상이 없다

## 6. 기타

- **git 커밋 미완료** — identity 미설정으로 staged 상태 (`object-corrections-v13-parity` 브랜치)
- ~~jet veto map 로직 차이~~ — **해결됨**: v15 구현이 JERC Run3 권고(pT>15, tightLepVeto ID,
  chEmEF+neEmEF<0.9, 해당 젯이 veto region에 있으면 이벤트 버림)와 일치한다. 뮤온 오버랩
  제거를 넣은 v13 쪽이 권고에서 벗어나 있었다. v13 대비 veto율 +1.05%p는 의도된 차이다.
- `setup.sh`가 `ROOT_INCLUDE_PATH`를 설정하지 않음 — 외부 모듈을 매크로로 로컬 실행할 때 필요

---

# 참고 자료 (이식 작업 중 계속 사용)

아래 1–7장은 **레퍼런스**다. 완료 여부와 무관하게 남은 이식 작업에 계속 필요하다.
특히 **§7.1이 위 "완료" 항목으로 새로 생긴 API 목록**이므로, 남은 5개 이식은 반드시 그것을 쓴다.

## 0. 오브젝트 모델 전환 요약

**"똑같은 코드를 그대로 가져오면" 컴파일 자체가 안 된다.** 오브젝트 모델이
`RVec<Muon>`(소유 객체) → `MuonViewCollection`(읽기 전용 이벤트-스코프 View)으로
전면 교체되었고, 오브젝트 변형 API(`SmearJets`, `ScaleMuons` 등)가 전부 사라졌다.
다만 **분석 로직·셀렉션·systematics 구조(YAML/SystematicHelper)는 그대로 이식 가능**하다.

TTree 출력 API(`NewTree/SetBranch/FillTrees`)도 제거되어 BDT ntuple 아날라이저의
출력부는 `Output().Book()` (RNTuple) 기반으로 재작성해야 한다.

**빌드/제출은 이 작업 범위에서 하지 않는다** (사용자 지시).

---

## 1. 프레임워크 구조 차이

### 1.1 저장소 구조와 "아날라이저를 어디에 두는가"

| | v13 | v15 |
|---|---|---|
| 아날라이저 위치 | `Analyzers/src/*.cc` (glob으로 자동 포함) | **개인 아날라이저는 외부 모듈 저장소** (`examples/AnalysisModule` 템플릿). 공용은 `CommonAnalyzers/`(PR 필요) |
| 프레임워크 코어 | `Analyzers/` 안에 혼재 | `AnalyzerFramework/`(코어, **아날라이저 추가 금지**), `AnalyzerTools/`, `SKNanoCore/`(실행 엔진), `DataFormats/` |
| 등록 | `AnalyzersLinkDef.hpp`에 pragma 한 줄 | 모듈 CMake + LinkDef + **manifest 게이트** (`share/sknano/analyzers.manifest`에 없으면 `SKNano.py`가 제출 거부) |
| include | `#include "AnalyzerCore.h"` | `#include <AnalyzerFramework/AnalyzerCore.h>` |

외부 모듈 추가 방법 (`docs/AnalyzerDevelopment.md`):

```cmake
# MyAnalysis/CMakeLists.txt
sknano_add_analysis_module(
    NAME HNWR
    HEADERS include/HNWR/Reproduce20_002_copy.h ...
    SOURCES src/Reproduce20_002_copy.cc ...
    LINKDEF include/HNWR/LinkDef.hpp
    ANALYZERS Reproduce20_002_copy WRGenRecoMass ...
    PUBLIC_LIBRARIES nlohmann_json::nlohmann_json)   # TheoryNormK.json 파싱용
```

```bash
./scripts/build.sh --analysis-module-dir /path/to/HNWR
```

`jet_pt_reweight`가 `Reproduce20_002_copy`를 **C++ 상속**하므로 두 클래스는 반드시
**같은 모듈**에 넣어야 한다. (6개 전부 하나의 모듈에 넣는 것을 권장.)

### 1.2 실행 흐름

`SKNano.py` 인터페이스는 거의 동일 (`-a -i -n -e --userflags ...`).
새 옵션: `-p`(period 필터), `--exclude`, `--failure-policy`, `--max-event-errors`.
job 매크로 순서도 동일하게 `Init() → initializeAnalyzer() → Loop() → WriteHist()`.

차이: 기본 `FailurePolicy`가 `FailFast`(이벤트 하나가 던지면 job 사망)다.
v13처럼 `exit(EXIT_FAILURE)` 대신 **예외를 던지는** 방식으로 바꾸는 것을 권장
(에러 메시지가 `FailureContext`로 수집됨).

### 1.3 입력: TTree → RNTuple, 브랜치는 lazy

- 브랜치는 `generate_branch_code.py`가 생성한 멤버로 직접 접근:
  `PuppiMET_pt`, `genWeight`, `LHE_HT`, `nLHEPdfWeight`, `LHEPdfWeight[i]` 등 —
  **v13에서 쓰던 이름 대부분 그대로 존재**한다 (`LHE_HT`도 있음, `generated_branch_decls.inc:1612`).
- 단 NanoAOD `run` 필드의 C++ 이름은 `RunNumber`.
- 첫 접근 시 lazy 활성화. 수동 branch enable 불필요.
- `Electron_vidNestedWPBitmap` / `Electron_vidNestedWPBitmapHEEP` 브랜치도 존재
  (`generated_branch_decls.inc:130-131`) — 단 `ElectronView` 접근자로는 노출 안 되므로
  `Electron_vidNestedWPBitmap[el.rawIndex()]`처럼 **raw 브랜치 + rawIndex()**로 읽어야 한다
  (`isPassLooseNoIso` 재구현에 필요).

### 1.4 보정 데이터 출처: 로컬 → cvmfs 중앙 관리

- v13: `data/<tag>/<era>/{MUO,EGM,JME,...}/*.json.gz` (로컬 관리)
- v15: `CMS_corrections -> /cvmfs/cms-griddata.cern.ch/cat/metadata` 심링크,
  `JSONPOG_REPO_PATH` 환경변수로 참조. 커스텀 파일(트리거 eff, tagging eff, ONNX)만
  `$SKNANO_DATA`(= `data/Run3_v15_Run2_v15`)에.
- cvmfs `MUO/Run3-24.../latest/`에는 `muon_HighPt.json.gz`, `muon_scalesmearing.json.gz`가
  **있다** — 파일은 있는데 `MyCorrection`이 로드하지 않는 것뿐이므로 배선만 추가하면 된다.
- cvmfs `EGM/.../latest/`에는 `electron.json.gz`, `electronHlt.json.gz`, `electronSS_EtDependent.json.gz`만
  있고 **HEEP용 `electronID_highPt.json.gz`는 없다** → v13 로컬 파일을 가져와야 함.

### 1.5 출력

- `FillHist(...)` 계열은 **전부 생존** (인자 타입만 `TString`→`std::string_view`, 호출부는 대부분 그대로 컴파일됨).
  디렉토리 구조 히스토그램(`"JES_Up/SR_..."`)도 동일하게 동작.
- 신규 권장 API: `Hists().Book1D(...)` + 핸들 `Fill` (선택 사항).
- **TTree 출력 제거**: `NewTree/SetBranch/FillTrees/GetTree` 없음.
  대체는 `Output().Book("name")` → `.Field("branch", var)`(zero-copy 바인딩) → `.Fill()`.
  필드는 한 번 바인딩하면 주소/타입 변경 불가 → 아날라이저 멤버 변수에 값을 담는 구조로 재설계.

---

## 2. 오브젝트 설정 API 대응표

### 2.1 패러다임: 소유 객체 → 읽기 전용 View + 인덱스

```cpp
// v13
RVec<Muon> muons = GetAllMuons();
RVec<Muon> sel = SelectMuons(muons, "NOCUT", 53., 2.4);
// 이후 muons[i].SetPt(...) 같은 변형 가능

// v15
MuonViewCollection muons = GetAllMuonViews();
std::vector<std::size_t> sel = SelectMuonIndices(muons, MuonView::MuonID::NOCUT, 53., 2.4);
const auto &mu = muons[sel[0]];   // 읽기 전용, 이벤트 넘기면 접근 시 throw
```

- View는 **수정 불가**. 오브젝트 변형(4-벡터 스케일 등)은 View를 복사한 값
  (`MakeLeptonSnapshot(mu)` → `Lepton`)에 적용해서 분석 로직에 사용해야 한다.
- View를 이벤트 경계 너머로 보관하면 접근 시 예외 발생 (event-scoped).
- 정렬된 컬렉션이 필요하면 `SelectMuonViews(muons, indices)` / `SelectElectronViews(...)`.

### 2.2 뮤온

| v13 | v15 | 비고 |
|---|---|---|
| `GetAllMuons()` | `GetAllMuonViews()` | |
| `SelectMuons(muons, "NOCUT", 53., 2.4)` | `SelectMuonIndices(muons, MuonView::MuonID::NOCUT, 53.f, 2.4f)` | 문자열 오버로드 없음(enum만). `"&&"` 복합 ID 미지원 |
| `mu.PassID(Muon::MuonID::POG_GLOBAL_HIGH_PT)` | `mu.PassID(MuonView::MuonID::POG_GLOBAL_HIGH_PT)` | enum 값·의미 동일 (`highPtId == 2`) |
| `mu.TkRelIso() < 0.1` | 동일 | |
| `mu.OriginalPt()` | **없음.** `mu.MiniAODPt()`는 **raw pt** | ⚠️ 의미가 다름: v13 `OriginalPt` = 모멘텀 보정 **후** pt |
| `mu.Pt()` (보정 적용됨) | `mu.Pt()` = **Rochester 보정 pt (전 구간)** | ⚠️ 고pT regime 없음 (아래) |
| `mu.MomentumScaleUp/Down()` | 동일 이름 존재 | 단 내용이 Rochester error 기반뿐 |
| `mu.IsHighPtRegime()` | **없음** | |
| `ScaleMuons(muons, "up")` | **없음** → `mu.MomentumScaleUp()` lane을 직접 사용 | |
| `SmearMuons(muons, "up")` (고pT 10% 추가 스미어링) | **없음** → 직접 구현 | MuonRes systematic |

**⚠️ 고pT 뮤온 (WR 분석에 치명적):**
v13 `GetAllMuons()`는 pt<200에서 Rochester, pt≥200에서 `Muon_pt*Muon_tunepRelPt`(TuneP)로
전환하고 DATA에는 Generalized-Endpoint scale, MC에는 고pT resolution smearing을 적용했다.
v15 `PopulateMuonMomentum`(`AnalyzerCoreObjects.cc:237`)은 **pt와 무관하게 전부 Rochester**.
`Muon_tunepRelPt` 브랜치는 존재하므로, 이식 시 v13의 regime 분기 로직을 아날라이저(또는
프레임워크 패치)로 직접 복원해야 한다. cvmfs의 `muon_scalesmearing.json.gz`를 함께 검토할 것.

또한 v15 `GetMuonIDSF`는 SF를 `MiniAODPt()`(raw pt)에서 평가한다
(`MyCorrectionLeptons.cc`). v13은 보정 후 pt(`OriginalPt`)로 평가했다 — 결과가 미세하게 달라짐.

### 2.3 전자

| v13 | v15 | 비고 |
|---|---|---|
| `GetAllElectrons()` | `GetAllElectronViews()` | ⚠️ **GAP(크랙) 제거 안 함** (아래) |
| `SelectElectrons(electrons, "NOCUT", 53., 2.5)` | `SelectElectronIndices(electrons, ElectronView::ElectronID::NOCUT, 53.f, 2.5f)` | |
| `el.PassID(Electron::ElectronID::POG_HEEP)` | `el.PassID(ElectronView::ElectronID::POG_HEEP)` | 동일. PassID는 GAP을 항상 거부 (v13과 같음) |
| `el.scEta()` | `el.ScEta()` | **대문자 S** |
| `el.VidNestedWPBitmap()` | **없음** → `Electron_vidNestedWPBitmap[el.rawIndex()]` | `isPassLooseNoIso` 재구현용 |
| `el.deltaEtaInSeed()`, `deltaPhiInSC/Seed()`, `ecal/hcalPFClusterIso()`, `dEsigmaUp/Down()` | **-999 스텁** (`ElectronView.h:149-156`) | BDT presel 브랜치에 사용 시 주의 |
| `el.TightCharge()`, `isCutBasedHEEP()`, `dr03TkSumPtHEEP()`, `r9()`, `SeedGain()` | 동일 존재 | |
| `ScaleElectrons(ev, electrons, "up")` | **없음** → `myCorr->GetElectronScaleUnc(...)`로 factor 얻어 스냅샷에 수동 적용 | |
| `SmearElectrons(electrons, "up")` | **없음, smear unc 메서드도 없음** → `electronSS_EtDependent.json.gz`에서 직접 구현 | |

**⚠️ 전자 nominal 에너지 보정 소실 (조용한 물리 차이):**
공식 EGM 레시피(Run3 Scales and Smearings)는 **nominal을 데이터·MC 양쪽에 적용**한다:
DATA는 `pt_corrected = pt * scale`, MC는 `pt_corrected = pt * (1 + smear·N(0,1))`.
v13 `GetAllElectrons()`는 이를 정확히 따라 DATA에 `GetElectronScaleCorr(...)`,
MC에 `GetElectronSmearUnc(el, nom, seed)`를 pt에 곱해 넣었고, `ScaleElectrons`/
`SmearElectrons`는 그 위의 up/down만 담당했다. (Run3 NanoAOD에는 EGM 캘리브레이션이
미적용이라 필수. Run2는 이미 적용돼 있어 v13도 1.0 반환.)
v15에는 **nominal 진입점이 DATA·MC 모두 없고** `ElectronView::Pt()`는 raw다.
→ mll 윈도우·HEEP pt 컷 근처에서 v13과 다른 결과가 나온다.

**⚠️⚠️ 게다가 v15에 남은 `GetElectronScaleUnc`는 2024에서 이미 깨져 있다.**
2024 `electronSS_EtDependent.json.gz`의 실제 스키마는
compound `Scale(syst, run, ScEta, r9, pt, seedGain)` +
correction `SmearAndSyst(syst, pt, r9, ScEta)` 인데,
`MyCorrectionLeptons.cc:128-137`은 옛 스키마를 참조한다:
키 `EGM_keys["2024"]+"_ScaleJSON"`(**존재하지 않음**), compound를 simple accessor
`cset_electron_variation->at(key)`로 접근, 인자 `{"total_uncertainty", gain, run, scEta, r9, pt}`
(순서·syst 문자열 모두 불일치). nominal은 early-return이라 지금 죽지 않지만
**ElectronScale systematic을 켜면 예외**가 난다. **v13 코드도 옛 스키마용이라 복사로는 해결되지 않는다
— 새 스키마 기준으로 재작성해야 한다.**
새 스키마는 오히려 단순하다: `SmearAndSyst` 하나에서 `smear`/`scale_up`/`scale_down`/
`smear_up`/`smear_down`를 모두 얻으므로 v13식 variation÷nominal 비율 트릭이 불필요하다.

**⚠️ 물리 한계:** EGM 문서는 이 보정이 "may be ineffective at very high pT
(hundreds of GeV)"라고 명시한다. WR 분석의 HEEP 전자(130 GeV~수 TeV)는 정확히 그 영역이므로,
적용하되 고pT 신뢰도 한계를 인지할 것 (고pT 뮤온 regime과 같은 성격의 문제).

**⚠️ GAP 처리 차이 (조용한 물리 차이):** v13 `GetAllElectrons()`는 프레임워크에서
GAP 전자를 컬렉션에서 제거했다. v15 `GetAllElectronViews()`는 제거하지 않는다
(skipCrack 미사용, `AnalyzerCoreObjects.cc:339`). 따라서 `"NOCUT"` 기반 loose-lepton
카운팅(`isPassLooseNoIso`)에 GAP 전자가 흘러들어올 수 있다 →
loose 정의에 `el.etaRegion() != GAP` 조건을 **명시적으로** 추가할 것.
(HEEP/POG ID 경유 판정은 PassID가 GAP을 거부하므로 안전.)

### 2.4 AK4 젯

| v13 | v15 | 비고 |
|---|---|---|
| `GetAllJets()` (JEC 재유도 + MC 자동 JER 스미어링) | `GetAllJetViews()` (raw; 보정은 lazy lane) | |
| `SelectJets(jets, Jet::JetID::TIGHTLEPVETO, 40., 2.5)` | `SelectJetIndices(jets, JetView::JetID::TIGHTLEPVETO, 40.f, 2.5f)` → `SelectJetViews(jets, indices)` | `SelectedJetView.Pt()`가 nominal(smeared) 투영 반환 |
| `SmearJets(jets, genjets, var, "total")` | `SelectJetIndices(..., /*JES*/nom, /*JER*/var)` + `SelectJetViews(..., nom, var)` | 스미어링은 lane으로 자동 (`SmearedPtUp/Down`) |
| `ScaleJets(jets, var, "total")` | `SelectJetIndices(..., /*JES*/var, /*JER*/nom)` (source="total"은 기본) | 개별 소스는 `ApplyJetScaleVariation(jets, source)` / `PrepareJetJESVariations` |
| JES와 JER 동시 변형 | — | **동시 non-nominal이면 throw** (`AnalyzerCoreJets.cc:722`) — v13 사용 패턴(한 번에 하나)과는 호환 |
| `jet.PassID("tight")` | `myCorr->PassJetID(jet, JetView::JetID::TIGHT)` | JetView에 PassID 없음 |
| `PassVetoMap(jets, mu_set.AllMuons, "jetvetomap")` | `PassJetVetoMap(jets, "jetvetomap")` | 뮤온 인자 없음 — **JERC Run3 권고대로다**(뮤온 오버랩 제거 요구 없음, tightLepVeto ID). v13이 TIGHT + ΔR<0.2 뮤온 클리닝을 쓴 쪽이 권고에서 벗어나 있었다. **Run3 전용**(Run2에서 throw). v13 대비 veto율 +1.05%p (측정값) |
| `PassNoiseFilter(jets, ev, MET_Type::PUPPI)` | 동일 시그니처 존재 | |
| `GetHT(jets)` | `GetHT(jets, indices, jesVar, jerVar)` 또는 `GetHT(selectedJets)` | |

젯–렙톤 클리닝: v13의 `Clean_jet_with_loose_leptons`류는 아날라이저 자체 함수였으므로
View 기반으로 고쳐서 그대로 이식 (또는 `JetsVetoLeptonInside(...)` 활용, 단 이는
lepton-inside-jet 제거라 ΔR 클리닝과 방향이 다름 — 기존 자체 함수 이식 권장).

### 2.5 AK8 팻젯 — **가장 큰 공백**

| v13 | v15 | 비고 |
|---|---|---|
| `GetAllFatJets()` (AK8 JEC 재유도) | `GetAllFatJets()` (raw만; **보정 lane 없음**) | 이름은 그대로지만 내용이 다름 |
| `fj.PassID("Tight")` | `SelectFatJets(fatjets, FatJetView::ID::TIGHT, pt, eta)` 또는 `myCorr->PassFatJetID(fj, ...)` | |
| `fj.SDMass()`, `fj.LSF3()`, `GetTaggerResult(...)` | 동일 존재 | SDMass는 **raw** |
| `SmearFatJets / ScaleFatJets / VarySoftDropMass` | **전부 없음** | `GetFJER/GetFJERSF/GetFJESSF/GetFJESUncertainty`도 없음 |
| `GetAllGenJetAK8()` | **확인 필요** — `GetAllGenJetViews()`는 AK4 GenJet | AK8 gen-jet 브랜치(`GenJetAK8_*`)는 raw 브랜치로 접근 가능 |

**⚠️ nominal부터 다르다:** v13 `GetAllFatJets()`는 raw pt에서 최신 JSON JEC를 재유도해
적용했지만, v15 `FatJetView::Pt()`는 NanoAOD 프로덕션 당시 JEC가 반영된 저장값을 그대로
반환한다 (재유도 없음). CAT JSON의 JEC가 프로덕션보다 새 버전이면 nominal AK8 pt 자체가
v13과 달라진다. (SubJet/GenJetAK8도 View는 없지만 `SubJet_pt`, `GenJetAK8_pt` 등 raw
브랜치는 전부 존재하므로 직접 읽어서 재구현할 수 있다.)

`cset_jerc_fatjet`은 `MyCorrection`이 로드하지만(private) 이를 쓰는 public 메서드가 없다.
**AK8 JEC/JER/JES/SDMass 변형은 v13 코드를 참조해 직접 구현해야 한다** — 방법은 둘 중 하나:
(a) `MyCorrection`에 `GetFJESSF` 등을 추가하는 프레임워크 패치,
(b) 아날라이저에서 correctionlib으로 `fatJet_jerc` JSON을 직접 로드.
SDMass>40 컷과 `VarySoftDropMass`(subjet JER 포함)를 쓰는 `Reproduce20_002_copy`/`WRGenRecoMass`에 필수.

### 2.6 이벤트 / MET / 트리거

| v13 | v15 | 비고 |
|---|---|---|
| `GetEvent()` | 동일 | 트리거는 lazy provider 방식 (동작 동일) |
| `ev.PassTrigger(trigs)`, `ev.GetTriggerLumi("HLT_Mu50")` | 동일 | 단 `$SKNANO_DATA/<era>/Trigger/HLT_Path.json`이 era별로 있어야 함 (현재 2024만) |
| `ev.GetMETVector(MET_Type::PUPPI, MET_Syst::CENTRAL)` | `ev.GetMETVector(MET_Type::PUPPI, SKNano::Variation::nom, MET_Syst::CENTRAL)` | 인자 순서/타입 변경. `MET_Type::CHS` → `PF` |
| `ev.nTrueInt()`, `ev.nPV()`, `ev.nPVsGood()` | 동일 (nPV=nPU alias 버그도 동일) | |
| `MCweight()` | 동일 | |
| `GetScaleVariation(muF, muR)` | 동일 존재 | |
| `GetScaleVariationIndex(muF, muR)` | **없음** → v13에서 복사 (TheoryNormK 인덱싱에 필요) | |
| `GetAllGens()` / `Gen::PID()/isHardProcess()` | `GetAllGenViews()` / 동일 접근자 | |
| `GetAllLHEs()` | `GetAllLHEViews()` | |
| `GetGenMatchedLepton / GetLeptonType / TrackGenSelfHistory` | **전부 없음** → 직접 구현 (`deltaRMatchingViews` 활용 가능) | BDT presel의 `genCharge`/`genPartFlav_truth` 브랜치에 필요 |
| `FillingCorrShapes()/FillCorrWeight()` (corrShapes userflag) | **없음** → 해당 훅 제거 또는 자체 이식 | |
| `HasFlag`, `Userflags`, `PtComparing` | 동일 | |

---

## 3. Correction(MyCorrection) API 대응표

`MyCorrection` 생성은 동일: `myCorr = new MyCorrection(DataEra, DataPeriod, IsDATA ? DataStream : MCSample, IsDATA);`
`variation` enum도 동일(`nom/up/down`, 이제 `SKNano::Variation`의 alias).

### 3.1 6개 아날라이저가 실제로 쓰는 메서드의 지원 현황

| v13 호출 | v15 상태 | 대응 방법 |
|---|---|---|
| `GetPUWeight(nTrueInt, var)` | ✅ 동일 시그니처 | 그대로 |
| `GetElectronRECOSF(scEta, pt, phi, var)` | ✅ 동일 (+View 오버로드) | 그대로 |
| `GetElectronHighPtIDSF(scEta, pt, var, "Tight")` | ❌ **없음** | v13 `electronID_highPt.json.gz`를 가져와 correctionlib으로 직접 로드하거나 MyCorrection 확장 |
| `GetMuonIDSF("NUM_HighPtID_DEN_GlobalMuonProbes", mu, var)` | ⚠️ 메서드는 있음, **키가 로드 안 됨** | v15는 `muon_Z.json.gz`만 로드. HighPt TnP 키는 cvmfs의 `muon_HighPt.json.gz`에 있음 → 로드 경로 추가 |
| `GetMuonIDSF("NUM_probe_LooseRelTkIso_DEN_HighPtProbes", ...)` (Iso) | ⚠️ 위와 동일 | 위와 동일 |
| `GetMuonTriggerSF("NUM_HLT_DEN_HighPtLooseRelIsoProbes", RVec<Muon*>, var)` | ⚠️ 메서드 존재 (`MuonViewCollection` 오버로드), 키 로드 필요 | `RVec<Muon*>` → `MuonViewCollection`+indices 로 변경 |
| `GetMuonHighPtRECOSF(mu, var)` | ❌ **없음** | `NUM_GlobalMuons_DEN_TrackerMuonProbes` 키 (muon_HighPt.json) 직접 평가 구현 |
| `GetMuonRECOSF(mu, var)` (jet_pt_reweight/presel) | ✅ 존재 | 그대로 |
| `SmearJets/ScaleJets` | ❌ 삭제 | §2.4의 lane 방식으로 |
| `SmearFatJets/ScaleFatJets/VarySoftDropMass` | ❌ 삭제, 대체 없음 | §2.5 — 직접 구현 |
| `ScaleMuons/SmearMuons` | ❌ 삭제 | `MomentumScaleUp/Down` lane + 고pT smearing 직접 구현 |
| `ScaleElectrons/SmearElectrons` | ❌ 삭제 | `GetElectronScaleUnc` + smear 직접 구현 |
| `GetScaleVariation(muF, muR)` | ✅ (AnalyzerCore) | 그대로 |
| `GetJESSF/GetJERSF/GetJESUncertainty` | ✅ 존재 (`GetJESUncertaintySF`로 개명된 factor 버전 포함) | 필요 시 사용 |
| `IsJetVetoZone` | ✅ | |
| `METXYCorrection` | ✅ (미사용) | |

### 3.2 하드코딩 상수·분석 자체 코렉션 (전부 이식 가능 ✅)

다음은 프레임워크가 아니라 **아날라이저 코드 안**에 있으므로 그대로 복사하면 된다:

- `GetElectronTriggerSF_TnP` (era별 하드코딩 T&P 상수)
- `GetLSFSF` (1.06±0.10 / 1.11±0.11)
- DY 코렉션 3종 로딩/적용: C(genZpT) (`dy_zpt_nlo_lo.root`), R(jetPt) (`dy_jetpt_ratio.root`),
  EW(genZpT) (`ZPtEWCorr.root`) — 환경변수 `DY_ZPT_CORRECTION`/`DY_JETPT_CORRECTION`/
  `DY_ZPT_EW_CORRECTION`/`DY_NO_CORRECTION` 포함. 파일이 `/data9`에 있으므로 이 서버에서 접근 가능
- `TheoryNormK.json` 로딩 (`$SKNANO_DATA/<era>/HNWR/TheoryNormK.json` — **새 SKNANO_DATA 밑으로 복사 필요**)
- `GetGenZpT`, `IsDYSample("DYMLL")`, `SetSignalFlags`, DYReshape per-bin nuisance
- `isPassLooseNoIso`(vidNestedWPBitmap 기반) — §2.3의 raw 브랜치 접근으로 재작성

---

## 4. `Reproduce20_002_copy` Systematics 전수 점검 (질문 답변)

v13 기준: MC는 `$SKNANO_HOME/docs/MCLRSM.yaml`, 데이터는 `DataLRSM.yaml`(빈 파일 → Central만).
루프 구조 = `SystematicHelper` 범위-for + `calculateWeight()`.

**v15의 `SystematicHelper`는 v13과 사실상 동일한 클래스가 그대로 존재한다**
(`AnalyzerTools/include/SystematicHelper.h`, 구현 파일명 오타 `SystematicHepler.cc`까지 동일).
API 동일: `assignWeightFunctionMap`, `calculateWeight`, 범위-for, `getCurrentSysName()` 등
(+신규 `compileVariationPlan()`). 현재 v15에서 아무 아날라이저도 안 쓰고 있을 뿐이며,
**v13과 같은 YAML 파일을 그대로 사용 가능**하다. 따라서:

> `MCLRSM.yaml`, `DataLRSM.yaml`, `MCLRSM_nosyst.yaml`, `DYGenZpT.yaml`,
> `jet_pt_reweight.yaml`을 v13 `docs/`에서 v15 `docs/`(또는 모듈 내)로 복사하면
> systematic **루프 구조는 무수정 이식**된다.

### 4.1 Weight 기반 systematics (evtLoopAgain: false) — 20종

| syst (yaml) | 구현 (v13) | v15 지원 | 조치 |
|---|---|---|---|
| `Pileup` | `GetPUWeight(nTrueInt, var)` | ✅ | 그대로 |
| `ElectronID` | HEEP highPt ID SF (EGM JSON) | ⚠️ | JSON 지참 + 평가 함수 직접 구현 (§3.1) |
| `ElectronReco` | `GetElectronRECOSF(scEta, pt, phi, var)` | ✅ | 그대로 |
| `ElectronTrig` | 하드코딩 상수 | ✅ | 아날라이저 코드 복사 |
| `MuonID` | `GetMuonIDSF("NUM_HighPtID_DEN_GlobalMuonProbes",...)` | ⚠️ | `muon_HighPt.json.gz` 배선 후 사용 가능 |
| `MuonReco` | `GetMuonHighPtRECOSF` | ⚠️ | 직접 구현 (키는 cvmfs JSON에 존재) |
| `MuonTrig` | `GetMuonTriggerSF("NUM_HLT_DEN_HighPtLooseRelIsoProbes",...)` | ⚠️ | 위와 동일 |
| `MuonIso` | `GetMuonIDSF("NUM_probe_LooseRelTkIso_DEN_HighPtProbes",...)` | ⚠️ | 위와 동일 |
| `LSF` | 분석 내 상수 | ✅ | 코드 복사 |
| `ScaleWeight_muF` / `ScaleWeight_muR` | `GetScaleVariation` ÷ TheoryNormK | ✅ | `GetScaleVariationIndex`만 v13에서 복사 |
| `AlphaS` | `LHEPdfWeight[101/102]` ÷ K | ✅ | 브랜치 동일 |
| (PDF — retired) | `RunXsecSyst` userflag, PDFmem 히스토그램 | ✅ | 코드 복사 (LHEPdfWeight 존재) |
| `ZPtRw_MC_stat` | C ± σ_stat (분석 ROOT 파일) | ✅ | 코드 복사 |
| `ZPtRwQCDScale` / `ZPtRwQCDPDFError` / `ZPtRwQCDPDFAlphaS` | C 파일의 변형 키 비율 | ✅ | 코드 복사 |
| `ZPtRwEW1/EW2/EW3` | EW 파일 (arXiv:1705.04664) | ✅ | 코드 복사 |
| (yaml 외) DYReshape bin nuisances | R 파일 bin ±σ | ✅ | 코드 복사 |

**결론(weight 계열): 20종 전부 이식 가능.** 다만 lepton SF 6종은 "메서드/JSON 배선"
선행 작업이 필요하고, 나머지 14종은 분석-레벨 코드라 복사만 하면 된다.

### 4.2 Object-variation systematics (evtLoopAgain: true) — 6종

| syst | v13 적용 방식 | v15 상태 | 조치 |
|---|---|---|---|
| `JER` | `SmearJets`+`SmearFatJets`+`VarySoftDropMass(use_jer)` | AK4 ✅ (JER lane) / **AK8 ❌** | AK4는 `SelectJetIndices(..., jer_var)`; AK8+subjet SDMass는 직접 구현 |
| `JES` | `ScaleJets`+`ScaleFatJets`+`VarySoftDropMass` | AK4 ✅ (JES lane, "total") / **AK8 ❌** | 위와 동일 |
| `MuonScale` | Rochester err(<200)+GE kappa(≥200) | **부분** — Rochester err lane만 | 고pT GE kappa 분기 직접 복원 필요 |
| `MuonRes` | 고pT 10% 추가 스미어링 | ❌ | 직접 구현 |
| `ElectronScale` | EGM total_uncertainty | **부분** — `GetElectronScaleUnc`로 factor는 얻을 수 있음 | 스냅샷 4-벡터에 수동 적용 |
| `ElectronRes` | EGM err_rho 스미어링 | ❌ (smear 메서드 없음) | `electronSS_EtDependent.json.gz`로 직접 구현 |

**결론(object 계열): 6종 모두 "루프 구조"는 이식되지만, 변형 적용 자체는 재작성 대상.**
AK4 JES/JER만 프레임워크가 대신해 주고, 나머지(AK8 전부, 뮤온 고pT, 전자 scale/smear)는
v13 구현을 참조한 자체 코드가 필요하다. 특히 **`VarySoftDropMass`는 nominal 패스에서도
호출되었으므로(서브젯 JER nominal 스미어링) 이것이 빠지면 Central 결과 자체가 달라진다.**

### 4.3 부수 확인 사항

- `HNWR_BDT_presel`(구버전)은 v13에서도 full `MCLRSM.yaml`과 불일치(MuonRes/LSF/ZPt 타깃 미등록,
  `NoSyst` 전제)였다. **이식한다면 `HNWR_BDT_presel_3SR`을 기준으로 하고 구버전은
  `MCLRSM_nosyst.yaml` 전용으로 유지하는 것을 권장.**
- `jet_pt_reweight`의 MM RECO SF가 부모와 다르게 `GetMuonRECOSF`(generic)인 것은 v13의
  기존 비일관성 — 이식 시 그대로 둘지 통일할지 결정 필요.
- MET은 이 분석에서 noise filter에만 쓰이므로 MET 변형 부재는 문제 없음.

---

## 5. 아날라이저별 이식 검토

### 5.1 DYGenZpT — 쉬움, 사실상 기계적 치환

- Gen 전용: `GetAllGens()→GetAllGenViews()`, `Gen::PID()/Status()/isHardProcess()/Pt()` 전부 동일.
- `LHE_HT`, `nLHEPdfWeight`, `LHEPdfWeight`, `nLHEScaleWeight` 브랜치 동일 존재.
- `GetScaleVariation` 동일. `FillHist` 동일. `MCweight()` 동일.
- `SystematicHelper` + `DYGenZpT.yaml` 그대로 (yaml 복사).
- 주의: `exit(EXIT_FAILURE)` 2곳 → 예외로 교체 권장.

### 5.2 jet_pt_reweight — 중간 (부모에 종속)

- `Reproduce20_002_copy`를 상속하므로 **부모 이식이 끝나야** 이식 가능. 같은 모듈에 배치.
- Z-pT 파일 로딩(`DY_ZPT_CORRECTION`), `GetGenZpT` 등은 부모에서 상속 — 추가 작업 없음.
- 자체적으로 쓰는 `SmearJets/SmearFatJets`(nominal) 호출부만 §2.4/2.5 방식으로 수정.
- `jet_pt_reweight.yaml`(빈 systematics) 복사.

### 5.3 Reproduce20_002_copy — 어려움 (이식의 본체)

컴파일 단계에서 걸리는 것: §7 치트시트 전부 + `GetScaleVariationIndex` 복사,
corrShapes 훅 제거, nlohmann_json 링크.

재작성이 필요한 블록:
1. **오브젝트 변형 6종** (§4.2) — `executeEventFromParameter` 상단의 문자열 매칭 분기 유지,
   내부 적용만 교체. 뮤온/전자는 `MakeLeptonSnapshot` 후 4-벡터 스케일 →
   이후 셀렉션 로직은 스냅샷 `Lepton` 벡터 기반으로 변경 (v13도 사실상 값 복사본을 다뤘으므로
   구조 변화는 크지 않음).
2. **AK8 보정 스택** — nominal JER 스미어링 + `VarySoftDropMass` nominal 포함 (§4.2 주의).
3. **lepton SF 배선** (§3.1).
4. **고pT 뮤온 regime** (§2.2).

조용히 달라질 수 있는 것 (검증 항목):
- ~~jet veto map 로직 차이~~ — 해결됨, v15가 JERC 권고와 일치 (§2.4).
- GAP 전자 (§2.3) — loose 정의에 GAP 제외 명시.
- JER 스미어링 난수: v15도 `gRandom` 시드를 `PuppiMET_pt`로 잡는 legacy 방식이라
  이벤트별 시드 개념은 유사하나 **호출 순서가 다르면 개별 이벤트 결과는 달라진다**
  (통계적으로만 동등). bin-by-bin 일치 검증은 불가능하다고 전제할 것.
- 뮤온 SF 평가 pt (raw vs 보정 후, §2.2).

### 5.4 HNWR_BDT_presel / HNWR_BDT_presel_3SR — 어려움 (출력부 전면 재작성)

- 위 5.3의 모든 사항 +
- **TTree → RNTuple**: `NewTree/SetBranch/FillTrees` 기반의 트리 40여 개
  (`<sys>/SR_{EE,MM}_BDTTree_{resolved,boosted}` 등) → `Output().Book()` 재설계.
  `SetBranch(tree, name, val)` 대신 멤버 변수 + `Field()` 바인딩이므로,
  브랜치 값 대입부를 구조체 하나로 모으는 리팩토링이 사실상 강제됨.
- `GetGenMatchedLepton`(genCharge/genPartFlav_truth 브랜치) — 직접 구현.
- 전자 브랜치 중 `deltaEtaInSeed` 등 **-999 스텁 7종** — BDT 입력에서 제외하거나 raw 브랜치 확인.
- b-tag: 컷에는 안 쓰고 브랜치만 채우므로 `GetBTaggingWP(...)`(✅ 존재)로 충분.
  (참고: v15에는 b-tag **SF** 메서드가 아예 없지만 이 분석은 사용하지 않으므로 무관.)
- fatjet ParticleNet 스코어: `GetTaggerResult(FatJetTagger, ...)` 동일 존재.
- 구버전 presel의 알려진 비일관성(§4.3) — 3SR 기준 이식 권장.

### 5.5 WRGenRecoMass — 어려움 (그러나 5.3과 동일 작업)

- `Reproduce20_002_copy`의 바이트 단위 사본 + LHE 질량/fail-cutflow 추가이므로,
  **5.3을 끝낸 뒤 diff를 적용하는 방식**이 안전하다.
- 추가 의존성: `GetAllLHEs()→GetAllLHEViews()` (`LHEView::PdgId()/Status()` 동일),
  LHE 질량 합산 로직은 순수 분석 코드 → 그대로.
- 장기적으로는 사본 유지 대신 `Reproduce20_002_copy` 상속 또는 공용 베이스로
  리팩토링할 좋은 기회 (v13에서도 "keep in sync" 주석으로 수동 동기화하던 부분).

---

## 6. 권장 이식 절차

1. **모듈 골격**: `examples/AnalysisModule`을 복사해 HNWR 모듈 생성 (6개 클래스 등록,
   nlohmann_json 링크). YAML 5종을 v13 `docs/`에서 복사.
2. **era 배선**: `MyCorrection::GetEraConfig`에 필요한 era(최소 2024, 이후 22/23) 추가
   + `$SKNANO_DATA/<era>/` 에 `Trigger/HLT_Path.json`, `HNWR/TheoryNormK.json` 배치.
   ⚠️ 단 2022/2023은 **NanoAODv15 RNTuple 재처리 샘플이 있는지 먼저 확인** — 없으면 이식해도 돌릴 데이터가 없다.
3. **공용 헬퍼 계층 작성** (모듈 내 `HNWRCorrectionHelper` 같은 클래스 하나 권장):
   - HEEP highPt ID SF (v13 JSON 지참)
   - muon HighPt TnP SF 4종 (cvmfs `muon_HighPt.json.gz`)
   - AK8 JEC/JER/JES/SDMass (cvmfs fatjet_jerc; v13 `SmearFatJets/ScaleFatJets/VarySoftDropMass` 로직 참조)
   - 고pT 뮤온 모멘텀 regime (TuneP/GE/smearing; v13 `GetAllMuons`/`ScaleMuons`/`SmearMuons` 참조)
   - 전자 scale/smear 변형
4. **DYGenZpT 먼저 이식** — 프레임워크 감을 잡는 파일럿으로 최적 (gen 전용, 의존성 없음).
5. **Reproduce20_002_copy 이식** → Central 히스토그램을 v13 결과와 비교(같은 이벤트 셋이
   있다면; 없으면 수율 수준 비교) → systematics 6종 순차 활성화.
6. **WRGenRecoMass** = 5의 결과 + diff. **jet_pt_reweight** = 부모 완료 후 소폭 수정.
7. **BDT presel 3SR** — 출력부 재설계 포함, 마지막에.

검증 팁: v13과 v15는 스미어링 난수·SF 평가 pt·veto map 로직이 달라 **이벤트 단위 일치는
기대할 수 없다**. cutflow 비율과 히스토그램 모양(KS/χ²) 수준에서 비교할 것.

---

## 7. 부록: 컴파일 에러 치트시트 (v13 → v15)

| v13 코드 | v15 대체 |
|---|---|
| `#include "AnalyzerCore.h"` | `#include <AnalyzerFramework/AnalyzerCore.h>` |
| `RVec<Muon> / RVec<Electron> / RVec<Jet> / RVec<FatJet>` | `MuonViewCollection / ElectronViewCollection / JetViewCollection(+SelectedJetViewCollection) / FatJetViewCollection` |
| `GetAllMuons()` | `GetAllMuonViews()` |
| `GetAllElectrons()` | `GetAllElectronViews()` (+GAP 수동 처리, §2.3) |
| `GetAllJets()` | `GetAllJetViews()` (+lane 선택) |
| `GetAllGens()/GetAllLHEs()/GetAllGenJets()` | `GetAllGenViews()/GetAllLHEViews()/GetAllGenJetViews()` |
| `GetAllGenJetAK8()` | 없음 — raw 브랜치 `GenJetAK8_*` 직접 사용 |
| `SelectMuons(v, "NOCUT", pt, eta)` | `SelectMuonIndices(v, MuonView::MuonID::NOCUT, pt, eta)` |
| `SelectElectrons / SelectJets` | `SelectElectronIndices / SelectJetIndices(+SelectJetViews)` |
| `Muon::MuonID::POG_GLOBAL_HIGH_PT` | `MuonView::MuonID::POG_GLOBAL_HIGH_PT` (값 동일) |
| `Electron::ElectronID::POG_HEEP` | `ElectronView::ElectronID::POG_HEEP` |
| `Jet::JetID::TIGHTLEPVETO` | `JetView::JetID::TIGHTLEPVETO` |
| `el.scEta()` | `el.ScEta()` |
| `el.VidNestedWPBitmap()` | `Electron_vidNestedWPBitmap[el.rawIndex()]` |
| `mu.OriginalPt()` | 없음 (`MiniAODPt()`는 raw — 의미 다름, §2.2) |
| `jet.PassID(...)` | `myCorr->PassJetID(jet, id)` |
| `fj.PassID("Tight")` | `myCorr->PassFatJetID(fj, FatJetView::ID::TIGHT)` |
| `PassVetoMap(jets, muons, "jetvetomap")` | `PassJetVetoMap(jets, "jetvetomap")` (뮤온 인자 없음이 정상) |
| `SmearJets/ScaleJets/SmearFatJets/ScaleFatJets/VarySoftDropMass/ScaleMuons/SmearMuons/ScaleElectrons/SmearElectrons` | 삭제 — §2.4/2.5/4.2 |
| `ev.GetMETVector(PUPPI, CENTRAL)` | `ev.GetMETVector(MET_Type::PUPPI, SKNano::Variation::nom, MET_Syst::CENTRAL)` |
| `NewTree/SetBranch/FillTrees` | `Output().Book(...)/.Field(...)/.Fill()` |
| `GetGenMatchedLepton / GetLeptonType` | 직접 구현 (`deltaRMatchingViews` 참고) |
| `GetScaleVariationIndex` | v13에서 함수째 복사 |
| `FillingCorrShapes()/FillCorrWeight()` | 삭제 또는 자체 이식 |
| `GetMuonHighPtRECOSF / GetElectronHighPtIDSF` | 직접 구현 (§3.1) |
| `exit(EXIT_FAILURE)` | `throw`(SKNano 예외 또는 std::runtime_error) 권장 |

## 7.1 부록: 2026-08-14에 v15에 추가된 API (이식 시 이것을 쓸 것)

오브젝트 보정 계층 복원 작업으로 아래가 새로 생겼다. v13 코드를 옮길 때
"없음"이라고 적힌 항목 중 상당수가 이제 대체 API를 갖는다.

| v13 | 새 v15 API |
|---|---|
| 고pT 뮤온 regime (TuneP 200 GeV 분기) | `MuonView::TunePPt()`, `IsHighPtRegime()`, `HighPtPt()` |
| `ScaleMuons(muons, "up"/"down")` | `MuonView::HighPtScaleUp()/HighPtScaleDown()` |
| `SmearMuons(muons, "up"/"down")` | `MuonView::HighPtResUp()/HighPtResDown()` |
| 고pT 뮤온 셀렉션 | `AnalyzerCore::SelectHighPtMuonIndices(muons, ID, ptmin, etamax)` |
| `GetMuonHighPtRECOSF(muon, var)` | `myCorr->GetMuonHighPtRECOSF(eta, p, var)` — **p는 운동량**(`mu.HighPtMomentum()`) |
| `GetMuonIDSF("NUM_HighPtID_...", mu, var)` | `myCorr->GetMuonHighPtSF(key, eta, pt, var)` — 명시적 수치 |
| 전자 nominal scale/smear | 자동 적용됨. `ElectronView::Pt()`가 보정 후, `MiniAODPt()`가 raw |
| `ScaleElectrons(ev, el, "up")` | `ElectronView::ScaleUpPt()/ScaleDownPt()` |
| `SmearElectrons(el, "up")` | `ElectronView::SmearUpPt()/SmearDownPt()` |
| `SmearFatJets/ScaleFatJets` | `FatJetView::SmearedPt{Nominal,Up,Down}()`, `JesPt{Up,Down}()`, `CorrectedPt()` |
| `VarySoftDropMass(...)` | `FatJetView::SDMassNominal()`, `SDMassJes{Up,Down}()`, `SDMassJer{Up,Down}()` — **`SDMass()`는 raw이므로 쓰지 말 것** |
| AK8 셀렉션의 JES/JER 패스 | `SelectFatJets(fatjets, id, ptmin, etamax, jesVar, jerVar)` |
| GAP 전자 제거 | `GetAllElectronViews(/*skipCrack=*/true)` |

**주의 3가지.**
1. AK4처럼 AK8도 **JES와 JER을 동시에 non-nominal로 요청하면 예외**를 던진다.
   v13의 systematic 루프가 한 번에 하나만 켜므로 그대로 이식하면 문제없다.
2. 뮤온 SF는 이제 **보정 후 pt**에서 평가된다. v13의 `OriginalPt()` 의미와 같다.
3. **2024는 GE κ 맵과 고pT 해상도 맵이 없어 두 보정이 no-op**이다. 코드는 그대로 쓰되
   현재 2024 결과에는 반영되지 않는다는 점을 알고 있을 것.

## 7.2 그대로 가는 것들

변경 없이 그대로 컴파일/동작하는 것들: `FillHist` 전 계열, `FillCutFlow`, `MCweight()`,
`HasFlag`, `Userflags`, `PtComparing`, `ev.PassTrigger/GetTriggerLumi`, `PassNoiseFilter`,
`GetScaleVariation`, `GetPUWeight`, `GetElectronRECOSF`, `SystematicHelper`+YAML 전체,
`Gen`/`LHE` 접근자, `SDMass()/LSF3()/TkRelIso()/TightCharge()/GetTaggerResult` 등 대부분의 접근자.
