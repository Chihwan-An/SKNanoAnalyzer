# SKNanoAnalyzer 코어 개선 제안서 (v2 · 진행 추적 문서)

> **최초 작성**: 2026-07-10, 브랜치 `NanoAODv15_Vcb`, base HEAD `9d58833`
> **검토 반영**: 2026-07-22
> **목적**: 여러 세션에 걸쳐 코어 정확성·확장성·성능 개선을 재현 가능하게 추진한다.
> **현재 범위**: 안전한 P0/P1 hot-path 패치를 우선 구현하며, 장기 Track D는 opt-in 기반 API까지만 단계적으로 확장한다.
> **중요**: 코드 위치(file:line)는 현재 working tree 기준의 탐색 힌트다. 구현 전 반드시 재검색한다.

---

## 0. 결론과 작업 규칙

### 0.1 진단 결론

전면적인 RDataFrame/coffea 재작성은 현재 우선순위가 아니다. 기존 코어는 lazy branch I/O,
일부 컬렉션의 SoA view, 인덱스 선택, 선택적 materialization이라는 좋은 기반을 갖고 있다.
그러나 아직 이를 **SOTA라고 부를 측정 근거는 없으며**, 다음과 같은 큰 비용과 정확성 위험이 남아 있다.

1. `GetEvent()`가 매 이벤트 2024 trigger 목록 617개를 순회하고 실제 HLT branch를 읽어
   `std::map`을 재구축한다. 분석기가 실제 사용하는 trigger는 보통 소수다
   (`AnalyzerCoreObjects.cc:11-43`, `TriggerDecision.h:9-14`).
2. generated HLT wrapper와 runtime HLT wrapper가 같은 branch 이름으로 중복 등록될 수 있다.
   현재 `BranchManager`에는 이름 기반 소유권·중복 검사가 없다
   (`SKNanoLoader.cc:288-291`, `BranchManager.h:358-365`).
3. Vcb는 golden/trigger reject 전에 Muon/Electron/Jet/Gen과 JEC/JER를 먼저 구축한다
   (`Vcb.cc:1111-1121`).
4. Vcb histogram pre-booking이 통과 이벤트마다 모든 systematic 경로에 weight 0으로
   실제 `Fill`을 반복한다 (`Vcb.cc:1503-1515`). 이는 성능뿐 아니라 histogram
   `fEntries` 의미에도 영향을 준다.
5. hot view 접근은 필드·원소마다 virtual `ColumnSource::get()`, `ensure()`,
   `vector::at()`을 반복한다 (`ViewColumns.h:18-24`, `BranchManager.h:145-167`).
6. FatJet 경로에는 performance migration 전에 분리해야 할 기존 correctness bug가 있다.

따라서 전략은 다음 두 층으로 나눈다.

- **안전한 기본 경로(Track A/B)**: 현재 API와 물리 출력을 보존하며 명백한 중복 I/O,
  string lookup, 불필요한 객체화와 할당을 제거한다.
- **opt-in 고처리량 경로(Track D)**: block column engine, fused systematic lanes,
  persistent ML binding, SIMD, deterministic MT를 단계적으로 추가한다.
  각 단계가 correctness/performance gate를 통과하기 전에는 기본값으로 승격하지 않는다.

여기서 “SOTA 지향”은 목표이지 선언이 아니다. 고정된 benchmark에서 throughput, CPU/event,
RSS, 출력 정확성의 Pareto 개선을 외부 수치로 입증한 뒤에만 달성 여부를 말한다.

### 0.2 F0. 제출 시점 코드 아카이브 정책

이 문서의 최초 진단 base commit은 `9d58833`이다. 이후 핵심 진단 파일인
`Analyzers/src/AnalyzerCoreJets.cc`, 분리된 `MyCorrection*.cc`, `SKNanoCore/` 등이
단계적으로 반영되었으므로, 아래 코드 위치는 현재 checkout에서 다시 검색해야 한다.

다만 이 프로젝트 운영에서는 매번 완전한 release artifact를 만드는 것보다, Condor 제출 시점의
현재 코드를 master/output 디렉토리에 tarball 형태로 남기는 정도면 충분하다.

**F0 STATUS: `DONE` — `python/SKNano.py`가 제출 master 디렉토리에 current-code snapshot과 archive를 생성한다.**

운영 규칙:

1. Condor 제출 wrapper가 job master/output 디렉토리 아래에 현재 코드 아카이브를 저장한다.
2. 아카이브는 tracked diff만이 아니라 현재 분석에 필요한 untracked 파일도 포함한다.
3. build directory, 큰 ROOT 출력, 일반 log/output, 캐시성 파일은 제외한다.
4. 파일명에는 최소한 날짜, branch, short commit, dirty 여부를 넣는다.
5. benchmark 또는 production 결과를 문서에 인용할 때는 해당 아카이브 경로를 같이 적는다.

예시 이름:

~~~text
code_archive/SKNanoAnalyzer_20260713_120000_NanoAODv15_Vcb_9d58833_dirty.tar.gz
~~~

이 정책은 “나중에 어느 코드로 돌렸는지 확인”하는 목적이다. 엄밀한 논문 재현성 패키지나
환경 전체 freeze는 D0 benchmark artifact가 필요할 때만 별도로 만든다.

### 0.3 현재 확인된 기반과 한계

| 요소 | 확인된 기반 | 현재 한계 |
|---|---|---|
| Lazy branch activation | `SetBranchStatus("*",0)` 후 첫 접근 시 branch 활성화 | trigger eager loop가 수백 HLT를 모두 활성화할 수 있음 |
| TTreeCache | 200 MB, 100-event warmup 후 활성 branch 등록 | 늦게 활성화된 branch와 workload별 cache 크기 정책이 불명확 |
| SoA view/cache | Muon/Electron/Jet/Gen 계열 per-entry cache | element-wise virtual access와 per-event owner/view allocation이 남음 |
| Jet systematic cache | nominal/JER/JES derived array를 event당 생성 | 요청하지 않은 lane도 생성하며 data도 Gen matching 경로를 탐 |
| 선택적 materialization | 통과 index만 고수준 객체화 가능 | legacy API와 view lifetime 계약이 혼재 |
| SystematicHelper | weight-only와 event-loop-again 분리 | 매 이벤트 string/hash map 작업과 variation별 경로 재순회 |
| `ColumnSource<T>` | vector backend의 `get/size/valid` seam | scalar, entry cursor, schema transition, bulk span은 추상화하지 않음 |
| branch codegen | JSON에서 2,011 declarations / 79 collection 생성 | generator 기본 출력과 실제 compile 입력 위치가 이중화됨 |

### 0.4 절대 불변조건

1. **Physics first**: 최적화가 selection/category/weight 의미를 몰래 바꾸면 안 된다.
2. **One branch, one canonical address**: 같은 tree/branch에 둘 이상의 wrapper가
   `SetAddress()`를 경쟁하면 안 된다.
3. **Explicit lifetime**: view/span은 유효 event 또는 block epoch 밖에서 읽지 못하게 한다.
4. **No silent missing value**: required branch missing/type mismatch는 예외,
   optional branch는 `available()` 확인 후 접근한다. 암묵적 `T{}` fallback은 금지한다.
5. **Measured promotion**: 기본 경로의 물리 결과를 바꾸는 fast path는 smoke/correctness 검증을
   반드시 통과한다. 반복 대규모 benchmark는 release 승격 또는 성능 수치 공표 시에만 수행한다.
6. **Reference fallback**: Track D가 완성돼도 최소 한 릴리스 동안 검증된 scalar/event 경로를
   유지한다.
7. **ROOT ABI/schema awareness**: `ClassDef` 객체 레이아웃 변경은 version bump와 구버전
   readback 검증 없이 수행하지 않는다.
8. **Build reproducibility**: 일반 build가 source tree의 generated file을 몰래 수정하면 안 된다.

### 0.5 F1. persistent object ABI/ROOT schema 정책

입력 객체는 event-scoped view이며 ROOT에 stream하지 않는다. 남아 있는 `ClassDef` value type
(`Event`, `Particle`, `Lepton`, `MuMu`)의 member type/order/개수를 바꾸는 작업은 F1을 먼저
완료해야 한다.

- 어떤 객체가 실제 ROOT output에 stream되는지 inventory.
- current class version과 기존 production file fixture.
- transient member 지정 또는 class version bump 기준.
- old → new readback과 new → current workflow compatibility test.
- public C++ ABI를 보장할 범위와 rebuild가 필요한 범위.

---

## 진행 상태 트래커

상태는 `TODO` / `IN PROGRESS` / `DONE` / `BLOCKED` / `DEFERRED` / `DROPPED`를 사용한다.
`TODO`라도 선행 조건이 완료되지 않았으면 착수하지 않는다. `DONE`에는 날짜, commit,
baseline ID, 검증 artifact 경로를 남긴다.

| ID | 항목 | 우선순위 | 선행 조건 | 상태 | 완료일/commit |
|---|---|---:|---|---|---|
| F0 | Condor 제출 시 current-code archive 생성 | P0 gate | — | DONE | 2026-07-13 / `python/SKNano.py` |
| F1 | persistent object ABI/ROOT schema 정책 | P1 gate | F0 | DONE | 2026-07-13 / `docs/RootSchemaCompatibility.md`: ClassDef v1 inventory·현재 output path audit·layout change readback policy |
| D0 | phase별 성능·I/O 계측 기반선 | P0 gate | F0 | DONE | 2026-07-13 / JSON telemetry·Vcb phase·ROOT I/O·median/MAD artifact runner; 5-run production baseline은 승격 시 실행 |
| B4a | event/block view lifetime 계약 + epoch guard | P1 | F0 | DONE | 2026-07-13 / `tests/test_view_and_branch_lifetime.cc`: N+1/N+2 stale access·copy/rebind 검증과 Clang ASan/UBSan smoke 통과 |
| B0a | canonical branch registry + lazy trigger provider | P0 | F0 | DONE | 2026-07-13 / canonical registry·lazy HLT·late cache hook·provider epoch guard; 0/1/N·Full·mixed-schema 검증 |
| B0b | cheap event gate 우선 + data Jet fast path | P0 | F0, B0a | DONE | 2026-07-13 / `Vcb`, `AnalyzerCoreJets` |
| B0c | histogram 1회 booking + typed handle | P0 | F0 | DONE | 2026-07-13 / zero-weight pre-book 제거·typed histogram handle·중복 schema 검증 |
| B1 | prepared correction cache/plan (기존 JES cache 포함) | P1 | F0 | DONE | 2026-07-13 / prepared JES correction refs |
| B5 | deterministic RNG transition contract | P1 | F0, D0 | DONE | 2026-07-13 / strict legacy와 keyed counter RNG 분리·repeat/thread/distribution test |
| A0 | raw schema + manual overlay + deterministic merge 계약 | P1 | F0 | DONE | 2026-07-13 / deterministic union·overlay 보호·conflict/fixed/variable/scalar/missing-count 실제 ROOT fixture 6-test 통과 |
| A3 | runtime typed column handle public API | P1 | B0a, B4a, A0 | DONE | 2026-07-13 / required·optional·canonical reuse·type/cardinality/count·mixed-tree·cache activation 검증, CalibrationTree 이관 |
| A1 | multi-file schema dump tool | P1 | A0 | DONE | 2026-07-13 / `python/dump_branch_schema.py`: repeatable input/manifest, union·intersection availability report, `--check` drift exit, ROOT fixture test |
| A2a | SoA input column + bind codegen | P1 | A0, A1 | DONE | 2026-07-13 / 7개 view collection composition·bind·required/optional metadata·epoch snapshot API 생성 및 fixture 통과 |
| A2b | materialized Property/accessor codegen | — | — | DROPPED | owning 입력 객체와 hidden materialization을 제거하고 column/view codegen으로 범위를 고정 |
| A4 | build-dir codegen + CMake/dictionary/CI 연결 | P1 | A2a | DONE | 2026-07-13 / build-dir generation·missing output dir·dictionary/install·2차 no-op·전체 Release build 통과 |
| B2a | FatJet correctness repair + golden test | P1 | F0 | DONE | 2026-07-13 / mapping·constituent·tagger fall-through 수정과 contract fixture 통과 |
| B2b | FatJet SoA/view migration | P2 | B2a, D0, F1 | TODO | |
| B3 | Jet unsmeared setter/temporary copy 제거 | P2 | D0 | DONE | 2026-07-13 / TLorentzVector setter·scalar correction setter |
| B4b | arena/buffer/view-owner 재사용 | P2 | B4a, D0 | DONE | 2026-07-13 / opt-in loader EventArena·`std::pmr` event/block reset·epoch guard |
| D1 | init-time compiled ExecutionPlan/ID interning | P1 experimental | B0a, B0c, B1, D0 | DONE | 2026-07-13 / dense typed IDs·manifest/schema validation·systematic topo order·loader cache 연결 |
| D2a | event-local contiguous span fast path | P1 experimental | B4a, D1 | DONE | 2026-07-13 / epoch-bound float/byte snapshot·generated ColumnView adapter |
| D2b | owned multi-event EventBlock engine | P1 experimental | D2a | DONE | 2026-07-13 / transactional owned ragged block·tree boundary·budget·event fallback·loader integration |
| D3 | typed correction batching + demand lanes | P1 experimental | B1, D2a | DONE | 2026-07-13 / JEC→JER→JES demand lanes·warm allocation test·MyCorrection adapter |
| D4 | fused selection/systematics/histogram kernel | P1 experimental | D1, D2a, D3 | DONE | 2026-07-13 / lane mask·selected index·Boost.Histogram weighted storage·zero-safe prefix/suffix weight plan; Vcb 승격 전 golden gate 유지 |
| D5 | EventArena + aligned/SIMD kernels | P2 experimental | D2b, B4a | DONE | 2026-07-13 / 축소된 `std::pmr` arena·Google Highway runtime dispatch·scalar tail·Strict/FastValidated profiles |
| D6 | basket/cluster-aware I/O + prefetch tuning | P2 experimental | D0, D1, D2b | DONE | 2026-07-13 / ROOT cluster planner·active compressed footprint cache tuner·독립 reader double buffer API |
| D7 | deterministic worker-local event MT | — | — | DROPPED | 2026-07-13 / single-core 운영 방침에 따라 CLI·loader hook·executor·test 제거 |
| D8a | prepared ML binding | P1 experimental | D0, D1 | DONE | 2026-07-13 / process Ort::Env·resolved metadata·실제 Ort::IoBinding·persistent result buffer |
| D8b | optional multi-event ML batching | — | — | DROPPED | 2026-07-13 / 운영하지 않을 queue·ordering·flush 코드와 test 제거; D8a prepared binding만 유지 |
| D9 | PGO/LTO + portable CPU dispatch | P2 experimental | D0, D4, D5, D8a | DONE | 2026-07-13 / strict portable·FastValidated·LTO·GCC PGO presets·Google Highway dispatch |
| C1 | RNTuple input/output backend | P1 | A0, D2a | DONE | 2026-07-22 / 기본 RNTuple 입력, typed RNTuple output, 변환기·merge 검증·backend smoke |
| C2 | schema-preserving RNTuple skim | P1 | F0, C1 | DONE | 2026-07-22 / 선택 global entry 기반 RDataFrame RNTuple Snapshot |
| C3 | RDataFrame/coffea 전면 재작성 | — | — | DROPPED | 현 단계 기각 |

권장 착수 순서는 `D0 → B4a/B0a → B0b/B0c → B1 → A0/A3 → B2a`다.
Track D의 숫자가 커 보여도 이 안전 기반을 건너뛰지 않는다.

---

## Track A — 확장성: schema/codegen/runtime column

### 설계 원칙

컬럼을 다음 세 계층으로 구분한다.

1. **raw schema**: 입력 파일에서 기계적으로 추출한 branch 이름, leaf type, scalar/vector,
   count branch, fixed/variable cardinality 정보.
2. **manual overlay**: view-enabled collection, 이름 override, required/optional 정책,
   derived/manual field, lazy activation 정책. 자동 dump가 덮어쓰지 않는다.
3. **generated merge output**: raw + overlay를 결정론적으로 병합한 build 입력.

표준 view 컬럼은 codegen으로 생성하고, 사용자/포크 컬럼은 A3 typed handle로 접근한다.
사용자 컬럼이 표준화되면 overlay에 승격한다. 분석용 컬럼은 view/handle에 머무르며,
event lifetime 밖으로 보낼 값만 명시적인 output DTO로 복사한다.

### A0. canonical schema pipeline

제안 구조:

~~~text
input ROOT files
    -> raw schema dump (generated, deterministic)
    +  manual overlay (reviewed, never overwritten)
    -> merged schema IR
    -> generated C++ in build directory
~~~

계약:

- raw dump는 `TLeaf::GetLeafCount()`, exact leaf type, static length, variable count 정보를 우선한다.
  `prefix + n<Collection>` 규칙은 fallback heuristic일 뿐이다.
- data/MC, era, private production 파일 여러 개를 받아 union을 만든다.
- 같은 branch의 type/cardinality가 파일마다 다르면 자동 승격하지 않고 conflict로 실패한다.
- ordering과 JSON formatting은 결정론적이어야 한다.
- 기존 description과 수동 metadata는 overlay에 보존한다.
- unsupported ROOT type은 branch 이름과 실제 type을 포함한 명시적 오류를 낸다.

완료 조건:

- 같은 입력 순서를 바꿔도 byte-identical raw/merged output.
- 표준 fixture의 branch/type/count tuple이 기존 schema와 일치.
- conflict, fixed array, variable array, scalar, missing count에 대한 unit test.

### A1. multi-file schema dump — `python/dump_branch_schema.py`

- 하나가 아닌 여러 파일 또는 manifest를 입력받는다.
- file/tree별 availability를 기록하고 union/intersection 보고서를 낸다.
- raw schema만 생성하며 overlay를 수정하지 않는다.
- `--check` 모드에서 checked-in/reference raw schema와 drift가 있으면 non-zero로 종료한다.

### A2a. SoA input/bind codegen

- merged schema IR에서 다음을 생성한다.
  - generated branch declarations/register/reset
  - view-enabled `XxxInputColumnsGenerated` 멤버
  - `bind()` 문
  - optional/required availability metadata
- C++에는 “partial class”가 없으므로 모호한 부분 클래스 표현을 쓰지 않는다.
  generated base/composition 또는 struct 내부 generated include 중 하나를 명시적으로 선택한다.
- `correctedPt`, `jesPtUp/Down` 같은 derived buffer는 수동 `XxxDerivedState`로 분리한다.

### A2b. materialized object codegen — 폐기

입력 owning 객체와 hidden materialization을 제거했으므로 생성하지 않는다. codegen은 column
storage, availability metadata와 view binding까지만 담당한다.

### A3. runtime typed column handle

공개 API 이름은 다음으로 고정한다.

~~~cpp
myScore = GetColumnHandle<float>(
    "Jet_myScore", ColumnRequirement::Required);

optionalScore = GetColumnHandle<float>(
    "Jet_optionalScore", ColumnRequirement::Optional);

if (optionalScore.available()) {
    const float score = optionalScore[jetIndex];
}

htxsNjets = GetScalarHandle<int>(
    "HTXS_njets30", ColumnRequirement::Required);
~~~

“재컴파일 없이”의 정확한 뜻은 **코어 schema/codegen/DataFormats 재생성 없이**다.
사용자 analyzer에 새 C++ 호출을 추가하면 그 analyzer 자체 build는 필요하다.

| 계약 | 요구사항 |
|---|---|
| 소유권 | manager-owned registry가 dynamic wrapper를 안정 주소의 `unique_ptr`로 소유 |
| 이름 유일성 | branch 이름당 canonical wrapper 하나만 허용 |
| generated 중복 | name/type/cardinality가 같으면 기존 generated wrapper 재사용 |
| 충돌 | 같은 이름의 type 또는 scalar/vector/count mismatch는 `SetAddress` 전에 `ConfigError` |
| required | 현재 tree에서 missing이면 file/tree/branch를 포함해 예외 |
| optional | `available()`은 false; 확인 없는 `operator[]`는 예외. `T{}` fallback 금지 |
| vector count | 실제 leaf count와 count type을 확인하고 canonical count wrapper 재사용 |
| TChain 전환 | tree가 바뀔 때 availability/type/cardinality 재검증 |
| cache | warmup 후 새로 활성화된 branch도 TTreeCache에 증분 등록 |
| lifetime | handle은 loader보다 오래 살 수 없고 event span은 epoch 밖에서 무효 |

필수 test:

- 동일 branch를 두 번 resolve했을 때 같은 canonical source 사용.
- generated HLT + runtime resolve 중복.
- wrong type, scalar/vector mismatch, wrong count type.
- required/optional missing.
- 첫 파일에는 있고 다음 파일에는 없는 mixed-schema chain.
- tree 전환 후 address와 value가 올바르게 갱신됨.

CalibrationTree의 ad-hoc branch read는 optional semantics를 유지하면서 이 API로 옮긴다.

### A4. build integration

정책:

- canonical generated output은 build directory에 둔다.
- CMake `add_custom_command(OUTPUT ...)`는 generator script, raw schema, overlay 모두를
  `DEPENDS`로 가진다.
- generated target은 ROOT dictionary generation과 관련 library보다 먼저 완료돼야 한다.
- Python interpreter와 output directory를 명시적으로 전달한다.
- 두 번째 build는 no-op이어야 한다.
- CI `--check`는 재생성 drift를 검사한다. clean checkout에서는 build 후 source diff가 0이어야
  하며, dirty developer tree에서는 build 전 snapshot 대비 generated/source path에 추가 diff가
  생기지 않아야 한다.
- committed generated snapshot을 유지한다면 그것은 bootstrap/reference일 뿐 build의 이중 source가
  되면 안 된다.

---

## Track B — 안전한 고효율 패치

### B0a. canonical branch registry + lazy trigger provider — 최우선

현재 `GetEvent()`는 `TriggerMap` 전체를 순회하며 각 runtime `BranchScalar<Bool_t>`를 읽고
`TriggerDecisionMap = std::map`을 매 이벤트 재구축한다. 2024 trigger JSON은 617개 경로이고
generated declarations에도 716개 HLT scalar가 있다. 이 경로는 lazy I/O의 장점을 사실상 상쇄할
가능성이 높고, generated/runtime 중복 wrapper는 정확성 위험도 만든다.

수정안:

1. 이름+type+cardinality 기반 canonical branch registry를 B0a에서 먼저 구현한다.
2. `TriggerDecisionProvider`가 trigger metadata와 canonical handle을 소유한다.
3. `PassTrigger(name)`가 호출된 trigger만 현재 entry에서 읽고 entry-local 결과를 cache한다.
4. 전체 trigger map을 명시적으로 요청한 legacy API만 eager snapshot을 만든다.
5. `Event`가 provider를 참조한다면 B0a 범위에서 trigger 전용 transient pointer + event epoch
   guard를 함께 구현한다. event 밖으로 escape하는 경우 요청된 decision만 snapshot한다.
   B4a는 이 규약을 모든 view/span으로 일반화한다.
6. 늦게 활성화된 trigger branch를 TTreeCache에 즉시 등록한다.

검증:

- 기존 trigger decision/lumi 결과와 exact match.
- 실제 활성 HLT branch 수와 branch read 수 기록.
- trigger 0/1/N개 사용 analyzer, Full trigger, missing optional trigger, tree transition test.
- duplicate `SetAddress`가 없음을 registry test로 확인.

채택 gate: trigger-heavy data workload에서 end-to-end ≥10% 또는 trigger/GetEvent phase ≥2배 개선,
다른 workload 회귀 ≤3%.

### B0b. cheap event gate 우선 + data Jet fast path

Vcb의 이벤트 순서를 다음처럼 바꾼다.

~~~text
run/lumi/event header
  -> golden JSON / requested trigger / scalar filters
  -> 필요한 lepton views
  -> jet correction/views
  -> 필요한 Gen views
  -> ML and output sinks
~~~

- `EventHeader`와 full `Event` 구축을 분리하거나, header accessor를 제공한다.
- data에서는 JER/GenJet matching/RNG 호출 전에 corrected pt/mass를 nominal smeared 값으로 복사하고
  즉시 반환한다. 현재 `SmearJetViews()`는 data도 `GetAllGenJetViews()`와 matching을 먼저 호출한 뒤
  하위 함수에서 반환한다 (`AnalyzerCoreJets.cc:690-699,432-443`).
- MC도 channel/trigger/golden/scalar reject 뒤에만 Gen/JEC/JER/ML을 수행한다.
- event 순서 변경 전 모든 RNG 사용처를 audit한다. MT 준비와 함께
  `run,lumi,event,object index,systematic id` 기반 stateless RNG로 전환하는 것이 최종 목표다.

검증:

- accepted/rejected entry ID가 exact match.
- accepted event의 모든 physics output bit-identical.
- reject 단계별 branch activation, JEC/JER/Gen 호출 수가 감소하는지 계측.

### B0c. histogram 1회 booking + typed handle

기존 weight 0 pre-book은 통과 이벤트마다 여러 systematic 경로의 수십 개 histogram을 실제로
`Fill`했다. 현재는 histogram 경로가 별도 구현으로 분리되었으며, booking을 `Fill`의 부작용에
의존하지 않는다는 계약을 유지한다.

1단계:

- `initializeAnalyzer()` 또는 첫 schema declaration에서 histogram을 정확히 한 번 `BookHist`한다.
- 이름+binning이 다르면 초기화 시 오류.
- `HistogramHandle`/`HistId`를 저장해 hot path에서 string 조립/hash lookup을 제거한다.
- weight 0 `Fill`을 booking 수단으로 사용하지 않는다.

주의: ROOT `TH1::Fill(weight=0)`도 `fEntries`를 증가시킨다. 따라서 이 변경은 bin content와
`sumw2`는 유지하되 잘못 부풀었던 `fEntries`를 의도적으로 고치는 correctness change가 될 수 있다.
승인된 delta를 golden artifact에 기록한다.

2단계의 dense raw-bin accumulator는 Track D4에서만 실험한다.

### B1. prepared correction plan

기존 JES cache 제안은 유지하되 “5개 같은 Ref”로 구현하지 않는다.

- non-2024: `correction::CompoundCorrection::Ref compoundJES`
- 2024: `correction::Correction::Ref` 타입의 L1/L2/L3/residual
- residual은 실제 data 경로에서만 필수

constructor/correction-set load 완료 시 eager immutable plan으로 resolve하는 것을 우선한다.
`SetEra`, `setIsData`, correction-set reload를 허용한다면 cache를 모두 invalidate해야 한다.
더 단순한 계약은 MyCorrection의 era/data/cset identity를 construction 이후 freeze하는 것이다.

JES 외에도 profiler가 확인하면 다음을 같은 plan으로 승격한다.

- Jet ID, PU, lepton, tagging correction의 fixed Ref lookup.
- correction input 순서와 invariant string/category의 init-time resolve.
- fixed-arity reusable argument buffer.
- JER matching에서 이미 계산한 resolution을 unmatched smearing에 재사용
  (`AnalyzerCoreJets.cc:366-372,470-475`).

correctionlib 수학을 임의 LUT/근사식으로 대체하지 않는다. specialized evaluator는 Track D의
별도 exhaustive boundary test 없이는 허용하지 않는다.

검증 matrix: 2024 MC/data + non-2024 MC/data. 모든 correction output과 physics histogram은
bit-identical이어야 한다.

### B2a. FatJet correctness repair

performance migration 전에 다음 현재 동작의 의도를 확정하고 각각 별도 test로 수정한다.

과거 owning `FatJet`의 constituent 누락, tagger fall-through, score routing, charged/neutral
multiplicity 순서 문제를 contract test로 고정해 수정했다. 현재는 `FatJetSoA`와
`FatJetView`가 branch를 직접 typed field로 노출하므로 vector setter나 owning mapping은 없다.

모든 vector setter를 typed field assignment로 바꾸기 전에 전체 branch→field mapping,
입력 길이, bounds를 audit한다. 각 tagger/score의 입력 branch 값과 반환값,
charged/neutral multiplicity, mass-correction, constituent index/content를 golden test로 고정한다.
bugfix의 의도된 출력 변화는 B2a artifact에만 기록하고 이후 B2b 기준선으로 사용한다.

### B2b. FatJet SoA/view migration — 완료

- `FatJetView` + event-scoped storage + index selection.
- tagger storage는 enum-indexed `std::array`로 바꾸되 tagger별 독립 배열을 유지한다.
- constituent 관계는 매 FatJet마다 전체 candidate를 스캔하지 말고 event당 한 번
  adjacency/offset을 만들어 O(nCandidate+nFatJet)로 구성한다.
- constituent branch는 실제 요청 시에만 활성화한다.
- in-tree analyzer 사용량을 먼저 조사한다. 사용하지 않으면 우선순위를 낮춘다.
- 입력 `FatJet`은 persistent ROOT 객체가 아니므로 schema evolution 대상이 아니다.

### B3. Jet owning 경로 제거 — 완료

입력 `Jet`과 materialization 경로를 제거했다. JEC/JER/JES는 `JetSoA`의
event-scoped derived lane과 `SelectedJetView` projection으로 표현하며, 입력
객체 복사나 mutable setter를 사용하지 않는다.

### B4a/B4b. lifetime 먼저, 재사용은 나중

현재 SoA의 입력 `ColumnView`는 값 snapshot이 아니라 현재 `BranchVector`를 가리킨다.
따라서 단순 double-buffer도 이전 event view가 다음 event branch buffer를 읽는 문제를 완전히
해결하지 못한다.

B4a:

- view/span을 event-scoped 또는 block-scoped로 공식화한다.
- loader가 증가시키는 epoch/generation을 view에 기록하고 debug build에서 stale access를 예외 처리한다.
- event 밖으로 보낼 materialized 객체는 필요한 값을 완전히 detach/snapshot한다.
- copy/move/cache return semantics를 문서화한다.

B4b:

- B4a 이후에만 event arena와 high-watermark vector capacity 재사용,
  owner 1개 + index proxy collection을 비교한다.
- 각 View가 `shared_ptr`을 보유하는 구조와 cache-hit collection 값 복사를 제거하는 방안을 측정한다.
- escape를 허용해야 하면 immutable snapshot pool을 사용하고 단순 member overwrite는 금지한다.

ASan/UBSan stale-view test와 event N의 객체를 N+1/N+2에서 읽는 negative test가 필수다.

### B5. deterministic RNG transition contract

B0b의 strict path는 accepted event의 현재 RNG 호출·seed·결과를 그대로 유지한다. 조기 reject로
사라지는 호출이 다른 accepted event에 영향을 주는지 먼저 audit한다.

event-level MT와 fused variation을 위한 counter RNG 전환은 별도 `INTENTIONAL_FIX`다.

- algorithm과 version을 고정한다.
- key는 최소 `run,lumi,event,object index,systematic id,draw id`를 포함한다.
- 같은 key는 thread 수와 scheduling에 무관하게 같은 값을 내야 한다.
- nominal/up/down lane 사이에서 공유해야 하는 random draw와 독립이어야 하는 draw를 명시한다.
- strict reference mode는 기존 RNG를 유지하고 fast mode만 승인된 counter RNG를 사용한다.
- 고정-key golden, 반복/병렬 재현성, 분포 test, JER/physics closure를 모두 통과해야 한다.

counter RNG는 기존 `gRandom` bit pattern과 같다고 주장하지 않는다.

---

## Track D — opt-in High-Throughput 실행 엔진 (SOTA 지향)

### 목표와 경계

Track D는 기존 lazy branch/SoA/view를 폐기하지 않고 그 위에 fast path를 추가한다.
legacy `executeEvent()`와 scalar/event reference path는 fallback으로 유지한다.

2026-07-13 현재 `DONE`은 이 문서의 현재 범위인 **실험/opt-in API와 production seam 구현 및
correctness smoke 완료**를 뜻한다. Vcb 기본 경로 승격이나 아래의 처리량 목표 달성을 뜻하지
않는다. 승격 전에는 실제 production input의 golden 비교와 D0 5-run artifact가 별도로 필요하다.
D7 event MT와 D8b multi-event ML batching은 운영하지 않기로 결정해 구현과 CLI를 제거했다.
실행 모델은 single-core event loop이며 D8a의 prepared ML binding은 유지한다.

“zero-copy”는 ROOT의 압축 basket decode가 공짜라는 뜻이 아니다. decode된 column buffer에서
view → selection → correction → sink 사이의 재복사와 객체화를 최소화한다는 제한된 뜻이다.
TTree에서 page borrowing이 불가능한 branch는 기존 entry-wise reader로 fallback한다.

야심찬 목표:

- systematic-heavy workload에서 고정된 D0 baseline artifact 대비 **single-thread ≥2배**.
- IO-bound 또는 작은 analyzer에는 보편적인 배수를 약속하지 않는다.

### D0. 성능 계약과 계측

workload:

1. trigger/golden/branch-I/O 중심 data.
2. jet-rich correction 중심 MC.
3. JES/JER + weight systematic-heavy Vcb.
4. ONNX-heavy Vcb_SL.

각 workload에 대해 local warm/cold cache를 분리하고 remote storage는 별도 결과로 기록한다.
개발 중에는 compile + representative smoke run과 출력 비교만 필수로 한다. 최소 5회 median/MAD
benchmark는 release 승격, 성능 수치 공표, 또는 regression 의심 시에만 수행한다.

필수 metric:

- wall time, CPU time, events/s, CPU/event, peak RSS.
- branch read count/bytes, unzip/cache statistics, active branch 수.
- phase별 시간: header/trigger, object view, correction, selection, materialization,
  systematic, histogram/tree, ONNX.
- heap allocation count/bytes per event.
- correction/ML 호출 수와 batch size.
- 가능하면 instructions, cycles, branch misses, LLC misses.

모든 optimization에는 사전 채택/중단 gate를 적는다. 기본 기준은
end-to-end ≥5% 또는 target hotspot ≥20%, 비대상 workload 회귀 ≤3%다.

### D1. init-time compiled ExecutionPlan

`initializeAnalyzer()`에서 문자열 기반 설정을 dense ID/table로 compile한다.

- `ColumnId`, `TriggerId`, `CorrectionId`, `SystematicId`, `HistId`, `ModelInputId`.
- required/optional branch manifest.
- correction input order와 invariant argument.
- YAML correlation/variation dependency graph.
- histogram schema와 model input/output binding.

hot path에서는 `unordered_map<string,...>`, path string 조립, correction name lookup을 하지 않는다.
오류 메시지용 ID→원래 이름 table과 legacy fallback은 유지한다.

추가로 analyzer가 필요로 하는 branch manifest가 완성되면 entry 0 전에 TTreeCache를 구성하고,
조건부 branch는 activation 시 증분 등록한다. manifest에 없는 동적 요청은 안전한 slow path로 간다.

### D2a. event-local contiguous span fast path

첫 단계는 multi-event block이 아니라 현재 event buffer를 직접 바라보는 span이다.
핵심 API는 element-wise virtual `get()` 대신 컬럼당 한 번 snapshot을 얻는 것이다.

~~~cpp
auto pt = columns.jetPt.snapshot(entry);      // ContiguousView<const float>
auto eta = columns.jetEta.snapshot(entry);
validate_same_size(pt, eta);                  // event당 한 번

for (std::size_t i = 0; i < pt.size(); ++i) {
    // hot kernel: virtual dispatch/ensure/vector::at 없이 contiguous access
}
~~~

- C++20이면 `std::span`, 아니면 동등한 작은 `ContiguousView<T>`를 사용한다.
- bool은 `uint8_t` span으로 별도 처리한다.
- 기존 `BranchVector`가 decode한 event buffer를 복사하지 않고 직접 참조한다.
- 기존 `ColumnView`에는 event span adapter를 제공한다.
- view/column snapshot에는 event epoch를 붙인다.
- 컬럼별 `ensure()`와 size/cardinality 검사는 event당 한 번 수행한다.

채택 gate: target collection kernel ≥20% 또는 end-to-end ≥5%, 다른 workload 회귀 ≤3%.

### D2b. owned multi-event EventBlock engine

TTree의 variable-length branch는 event별 buffer이므로 multi-event `values + offsets`를 만들 때
대부분 명시적인 gather/flatten copy가 필요하다. 이를 zero-copy라고 부르지 않는다.

- block은 tree/file 경계를 넘지 않고 memory budget으로 크기를 조절한다.
- jagged collection을 owned `RaggedBlock<T> = values + offsets`로 평탄화한다.
- gather/decode와 correction/selection을 pipeline 또는 double buffer로 overlap할 수 있다.
- backend가 안전한 bulk/page borrowing을 제공하는 column만 별도 zero-copy adapter를 허용한다.
- 미지원 type/branch는 D2a event-local path로 fallback한다.
- block epoch로 모든 subspan/view를 무효화한다.

반드시 별도로 계측할 것:

- decode 시간과 gather/flatten copy 시간.
- copied bytes/event, block high-watermark RSS, allocator 비용.
- block 크기별 throughput/latency.
- 작은 multiplicity workload의 copy 손익분기점.

작은 event의 jet 수만으로 SIMD를 시도하지 않는다. 여러 event를 block에 평탄화한 뒤 net 이득이
확인될 때만 D5 SIMD로 넘긴다.

채택 gate: copy/RSS를 포함한 net wall이 2개 이상 workload에서 ≥15% 개선되거나
remote I/O workload에서 ≥20% 개선, 다른 workload 회귀 ≤3%, RSS budget 준수.

### D3. typed CorrectionPlan + batching

B1 cache 위에 다음을 추가한다.

- 필요한 correction/variation lane만 demand-driven 생성.
- jet input을 contiguous arrays로 모아 JEC→JER→JES 순서의 tight loop에서 처리.
- correctionlib가 scalar API만 제공하면 scalar `evaluate`는 유지하되 lookup, string,
  argument container allocation을 loop 밖으로 hoist한다.
- correctionlib 바깥의 순수 산술만 profiler와 golden test 후 SIMD화한다.
- JER resolution과 random sample을 nominal/up/down lane에서 재사용한다.
- strict mode는 현재 RNG 결과를 유지한다. fast/MT mode의 counter RNG는 B5 승인 계약을 사용한다.

채택 gate: strict RNG path는 lane별 bitwise golden, B5 counter-RNG path는 승인된 reproducibility와
physics-closure 기준을 통과해야 한다. correction phase ≥1.5배 또는 end-to-end ≥10%,
hot correction call의 allocation이 warmup 후 거의 0이어야 한다.

### D4. fused selection + systematic lanes + sink

`SystematicHelper`의 YAML을 `VariationPlan`으로 compile한다.

- 공통 eta/ID/overlap 조건은 jet당 한 번 계산.
- nominal/JES/JER/lepton/MET variation은 필요한 lane만 계산.
- lane별 통과 결과를 bit mask로 보관하고 64개 초과 시 chunk한다.
- weight-only systematic은 dense vector/tensor로 계산한다.
- nominal weight가 0인 경우 단순 ratio를 쓰지 않고 stable prefix/suffix product 또는
  명시적 재계산을 사용한다.
- 선택된 index만 마지막 physics/ONNX/tree 경계에서 materialize한다.
- histogram 누산은 Boost.Histogram weighted storage와 merge 연산을 사용한다.

임의의 C++ analyzer control flow를 자동 fusion하려 하지 않는다. 먼저 Vcb hot selection이 opt-in하는
`SelectionPlan`/kernel API를 만들고 기존 `executeEventFromParameter()`를 reference로 유지한다.
정렬 순서, tie-break, floating multiplication order는 reference와 동일하게 고정한다.

채택 gate:

- variation별 selected event/jet/category exact match.
- strict mode histogram bitwise match.
- systematic-heavy single-thread end-to-end 목표 ≥2배, 최소 채택 기준 ≥25%.

### D5. EventArena + aligned/SIMD kernels

- `std::pmr::monotonic_buffer_resource`를 event/block마다 reset하고 epoch로 stale view를 막는다.
- indices, masks, temporary P4, correction lanes, histogram update를 arena/high-watermark buffer에 둔다.
- warmup 이후 hot path heap allocation/event ≈ 0을 목표로 한다.
- aligned arrays의 pt/eta mask와 ΔR²는 Google Highway에 runtime SIMD dispatch를 맡긴다.
- scalar tail과 portable fallback을 항상 유지한다.
- `-ffast-math`를 correctness 증명 없이 SIMD 수단으로 사용하지 않는다.

현재 Release의 전역 `-ffast-math`는 strict bitwise 기준과 충돌할 수 있으므로 D0에서 먼저 audit하고,
`StrictReference`와 `FastValidated` build profile을 분리한다.

### D6. basket/cluster-aware I/O

- D1 branch manifest로 가능한 active branch를 entry 0 전에 cache에 등록한다.
- ROOT cluster 경계에 맞춰 EventBlock/task를 나눈다.
- active basket footprint와 memory budget으로 cache/block 크기를 auto-tune한다.
- next-block async prefetch는 별도 read context와 double buffer가 있을 때만 허용한다.
- 같은 `TTree/TBranch`를 여러 thread가 동시에 읽지 않는다.
- local SSD와 remote/XRootD 결과를 분리하고, I/O·decompression·ONNX worker의
  중앙 thread budget으로 oversubscription을 막는다.

채택 gate: remote I/O wall ≥20%와 read-call 감소, local 회귀 ≤3%, RSS/file handle budget 준수.

### D7. deterministic optional event MT — 제거

single-core 운영 방침에 따라 `--threads`/`--event-threads`, loader MT hook,
worker executor와 ROOT worker test를 제거했다. 재도입 계획은 없다.

### D8a. prepared ML binding

현재 `MLHelper::Run_ONNX_Model`은 inference마다 map/variant lookup, shape vector,
`Ort::MemoryInfo`, `Ort::Value`, `RunOptions`, output copy를 반복한다
(`MLHelper.cc:125-260`). model/session별 `Ort::Env`도 중복 생성된다.

안전 단계:

- process-wide `Ort::Env`.
- load 시 node index, dtype, static shape resolve.
- typed span API와 persistent input/output buffer.
- reusable tensor binding/IO binding.
- release run verbosity 0.
- dynamic shape일 때만 재할당.

채택 gate: inference phase ≥1.5배 또는 ML-heavy end-to-end ≥10%, output bitwise 통과.

### D8b. optional multi-event ML batching — 제거

사용하지 않을 model/fold queue와 event-order 복원 계층은 제거했다. D8a의 prepared binding API는
event 단위 호출 비용을 줄이는 용도로 계속 사용한다.

### D9. PGO/LTO + portable CPU dispatch

- portable Release baseline을 유지하고 LTO/ThinLTO와 representative-workload PGO preset을 추가한다.
- heterogeneous HTCondor worker에서 전역 `-march=native`를 기본값으로 쓰지 않는다.
- kernel multiversion과 runtime CPU dispatch는 Google Highway에 맡긴다.
- CI는 non-PGO portable build를 계속 검증한다.

Toolchain 패치는 알고리즘/할당/I/O 개선 뒤의 마지막 5–20% 후보이며 측정 없이 배수를 약속하지 않는다.

### 피해야 할 “가짜 SOTA”

- correctionlib, `gRandom`, ROOT histogram에 무작정 `par_unseq` 적용.
- 요청되지 않은 모든 JES/JER universe 선계산.
- lifetime 계약 없이 하나의 SoA buffer를 다음 event에 overwrite.
- event당 jet 수가 작은 상태에서 SIMD library만 추가.
- batch=1 GPU 강제.
- heterogeneous worker에 기본 `-march=native`.
- global fast-math 변화 후 bit-identical이라고 주장.
- benchmark 없이 “몇 배 빨라짐”을 문서에 확정값으로 기록.

---

## Track C — backend 전환

### C1. RNTuple input/output backend — DONE

초기 feasibility spike를 거쳐 RNTuple backend를 기본 입력 경로로 구현했다.
generated branch wrapper와 analyzer view/selection API는 유지하고, backend별 entry navigation,
file transition, schema/type 검증과 scalar/vector source ownership은 `SKNanoLoader`와
`RNTupleSource`가 담당한다.

반영된 범위:

- RNTuple scalar/vector source와 page-backed contiguous arithmetic view.
- multi-file entry navigation, schema/type 검증과 lazy branch activation.
- typed RNTuple analyzer output, atomic publish와 validated merge.
- TTree → RNTuple 변환기와 두 backend를 비교하는 smoke test.
- 명시적으로 선택 가능한 legacy TTree fallback.

상세한 사용자 계약과 명령은 `docs/RNTupleIO.md`에 기록한다. D2b `EventBlock`은
현재 TTree 전용이며 RNTuple input에서는 entry-wise page view를 사용한다.

### C2. schema-preserving RNTuple skim — DONE

skim은 analyzer가 선택한 global entry 목록을 모은 뒤 RDataFrame의 RNTuple Snapshot으로
원본 입력 schema를 기록한다. 기본 RNTuple 입력과 명시적으로 선택한 legacy TTree 입력에서
동일한 출력 계약을 사용하며, analyzer의 lazy branch status/address를 변경하지 않는다.

현재 범위는 전체 schema 보존이다. keep/drop wildcard는 별도 기능으로 추가할 때 count field
포함, wildcard precedence, backend별 동일 schema를 독립적으로 검증해야 한다.

### C3. RDataFrame/coffea 전면 재작성 — 현 단계 DROPPED

현재 코어 자산을 버리는 전면 재작성은 근거가 없다. 다만 특정 component나 prototype을 금지하지는
않는다. 재논의 조건:

- Track D prototype이 구조적 한계로 목표를 달성하지 못함.
- CMS 표준 toolchain이 backend/execution model을 강제함.
- 동일 workload prototype에서 유지보수성과 end-to-end 성능 우위가 재현됨.

---

## 공통 검증과 benchmark 판정

### 정확성 정책

| 정책 | 적용 | 판정 |
|---|---|---|
| BITWISE | lookup/cache/lifetime/order 불변 패치 | bin content, sumw2, selected indices, tree values bitwise |
| INTENTIONAL_FIX | B0c `fEntries`, B2a FatJet bug, B5 counter RNG | 승인된 delta만 manifest에 기록, 나머지는 bitwise |
| PHYSICS_TOLERANCE | SIMD/provider/parallel reduction 등 명시 승인 항목 | entry/category exact + 변수별 사전 tolerance |

전체 ROOT 파일 byte 비교만 사용하지 않는다. timestamp/compression metadata와 physics object를
분리해 다음을 검사한다.

- histogram bin/underflow/overflow, sumw2, entries.
- TTree schema, entry count, branch value.
- selected run/lumi/event와 object index/category.
- error count/policy와 skim entry set.

### 항목별 최소 검증 matrix

| 항목 | 필수 검증 |
|---|---|
| F0 | Condor/job master directory에 current-code archive 경로 기록 |
| D0 | exact command/input checksum/toolchain/5-run raw artifact |
| B0a | active trigger branch/read count + all trigger decision equivalence + duplicate registry test |
| B0b | reject stage별 entry set, accepted output bitwise, data에서 Gen/JER 호출 0 확인 |
| B0c | bin/sumw2 동일, 승인된 `fEntries` delta, duplicate binning 오류 |
| A0-A4 | deterministic output, multi-file conflict, clean build, second build no-op, clean checkout source diff 0 |
| A3 | duplicate/wrong type/count/required/optional/mixed-tree/cache activation |
| B1/D3 | era × data/MC correction value matrix와 boundary value |
| B5 | fixed-key/repeat/thread determinism + RNG distribution + JER/physics closure |
| B2a | tagger별 golden + constituent content; intended delta manifest |
| B2b/B3 | corrected baseline bitwise + ROOT readback/ABI 해당 시 |
| B4/D2a/D2b/D5 | stale epoch negative test + ASan/UBSan + copy bytes/allocation/RSS |
| D4 | 모든 variation의 event/object/category/weight/histogram |
| D7 | 제거됨(single-core 운영) |
| D8a | model/fold별 prepared output bitwise + warmup 후 binding allocation |
| D8b | 제거됨(event 단위 prepared binding만 유지) |
| D9 | portable/PGO/CPU dispatch별 correctness와 worker CPU compatibility |
| C2 | skim schema/value/entry + source state restore |

### 성능 실행 규약

1. Release build와 정확한 compiler flags를 기록한다.
2. CPU affinity, thread 수, ORT 내부 thread 수를 고정한다.
3. local warm/cold cache와 remote storage를 섞지 않는다.
4. 최소 5회 실행하고 median + MAD를 기록한다.
5. `/usr/bin/time -v`뿐 아니라 phase timer, ROOT I/O statistics, allocation count를 저장한다.
6. benchmark 전/후 commit과 raw log artifact 경로를 세션 로그에 남긴다.
7. 기본 채택 기준은 end-to-end ≥5% 또는 hotspot ≥20%, 다른 workload 회귀 ≤3%.
8. 성능 개선이 threshold 미만이면 복잡도를 유지하지 말고 patch를 축소하거나 `DROPPED` 처리한다.

Build 검증:

~~~bash
source setup.sh
./scripts/build.sh
./scripts/rebuild.sh
~~~

`rebuild.sh`만으로 clean checkout build 가능성을 대신하지 않는다.

### 2026-07-13 구현 검증 artifact

- 일반 경로: `source setup.sh`, 전체 Release build, install, `rebuild.sh` 성공.
- test: `build/redhat` CTest 18/18 통과. schema/codegen Python test와 ROOT fixture 포함.
- D7 MT test 2개와 D8b batching test 1개는 기능 삭제와 함께 test suite에서 제거했다.
- D5/D9: 수동 AVX dispatch를 Google Highway 1.4.0으로 교체한 뒤 strict Release와
  FastValidated의 `test_event_arena_kernels`를 통과했다. FastValidated ΔR²는 scalar
  reference의 연산 순서를 고정한 상태에서 1 ULP 이내다.
- 새 preset의 offline configure는 yaml-cpp/json `FetchContent` source cache를 명시해 실행했다.
  네트워크 없는 완전 신규 checkout은 dependency archive/cache 준비가 별도 필요하다.
- single-core production smoke는 `CalibrationTree`, `WCharm_Mu`, 2024D `Muon0` data
  28,206 events로 수행했다. data에 없는 pileup/truth branch를 materialization이 읽던 문제와
  `TChain::GetClusterIterator()` 종료 abort를 수정한 뒤 CTest 18/18을 재통과했다.
- 현재 Release warm 5-run artifact는
  `benchmark_artifacts/calibration_tree_single_core/release_warm_5runs.json`이다. event-loop
  telemetry median은 1.826 s, MAD 0.004 s, 15,446 events/s, CPU 1.819 s,
  peak RSS median 828,524 KiB다. 측정 전 1회 warm-up을 별도로 실행했으며, 측정 5회 범위는
  1.822–1.867 s였다.
- 두 full-run output은 ROOT-native exact 비교에서 histogram 69개 bin/error, tree 202 entries와
  99 leaves가 모두 동일했다. 비교기는
  `benchmark_artifacts/calibration_tree_single_core/compare_outputs.C`에 둔다.
- 이 수치는 현재 코드의 재현 가능한 baseline이지 개선 전후 speedup 주장이 아니다.
  CalibrationTree는 아직 Highway kernel과 EventArena를 production hot path에서 호출하지 않으므로,
  이 benchmark로 두 library의 end-to-end 효과를 주장하지 않는다.
- Highway production 후보도 별도 A/B했다. 2024D data의 contiguous jet pt/eta selection은
  scalar 1.857 s 대비 Highway 1.881 s로 1.3% 느렸고, TT semileptonic MC 10k의
  GenJet matching/JER 경로는 scalar 1.528 s 대비 Highway 1.560 s로 2.1% 느렸다
  (correction phase 4.5% 회귀). 두 경우 checksum 또는 ROOT payload는 exact 일치했지만
  threshold를 통과하지 못해 production 연결 코드는 제거했다. raw artifact는 각각
  `scalar_jet_selection_warm_5runs.json`, `highway_jet_selection_warm_5runs.json`,
  `scalar_genjet_matching_warm_5runs.json`, `highway_genjet_matching_warm_5runs.json`이다.
- AVX3 microbenchmark에서는 ΔR² batch가 16 elements부터 빨라졌지만, TT sample은 event당
  GenJet 평균 7.75, 최대 21이고 10k event 중 `nGenJet >= 16`은 25개뿐이었다. 따라서
  현재 NanoAOD jet/genjet multiplicity에서는 dispatch/tail/temporary 비용이 이득을 넘는다.
  재사용 가능한 size-crossover 측정 코드는 `benchmark_delta_r.cc`에 둔다.
- AnalyzerCore 전수검토 결과, 다음 Highway 후보는 GenPart SoA 기반의 다수-particle matching이다.
  truth 입력과 matching은 `GenViewCollection`을 직접 사용한다. JEC/JER는 correctionlib/RNG, JES uncertainty는
  correctionlib source lookup, overlap removal은 lepton 0–2개, HT/MET propagation은 jet 약 7개가
  지배하므로 현 구조에서는 Highway 대상에서 제외한다.

---

## 세션 로그

| 날짜 | 작업 | 결과 / 다음 할 일 |
|---|---|---|
| 2026-07-10 | 코어 탐색과 최초 제안서 작성 | B1부터 착수 제안 |
| 2026-07-12 | 문서와 실제 구현 교차검토 | snapshot 재현성, A3 수명/중복, FatJet correctness, 검증 계약 문제 확인 |
| 2026-07-13 | 문서 v2 재작성 + SOTA 지향 Track D 추가 | **문서만 변경, 구현 없음.** 다음 필수 작업은 F0 정책 단순화와 D0 baseline 계측 |
| 2026-07-13 | F0 단순화 | **DONE.** Condor 제출 시점 current-code tarball을 master/output 디렉토리에 남기는 정책으로 변경. 다음: 실제 제출 wrapper에 archive 생성 hook 추가 여부 결정 |
| 2026-07-13 | F0 구현 | **DONE.** `python/SKNano.py`에서 `source_snapshot/` 생성 후 `code_archive/*.tar.gz`를 만들고 `run_manifest.json`과 제출 summary에 archive 경로, sha256, size를 기록 |
| 2026-07-13 | P0/P1 hot-path batch | **IN PROGRESS (light verification).** canonical scalar registry + lazy HLT provider, header-first Vcb gate, data Jet fast path, zero-weight histogram pre-book 제거, prepared JES refs, FatJet mapping/constituent fixes, Jet temporary-copy 제거, ColumnView event-epoch stale-access guard. 대규모 benchmark는 release 전으로 이연 |
| 2026-07-13 | A0 schema contract | **IN PROGRESS.** `dump_branch_schema.py`에 multi-input raw union, leaf-count cardinality, deterministic JSON, reviewed overlay merge, conflict/missing-count error와 `--check`를 구현했다. ROOT fixture 및 A4 build-dir generation 연결이 다음 단계다. |
| 2026-07-13 | F1 ABI/schema audit | **DONE.** 현재 output은 primitive/vector 또는 input clone이며 DataFormats object 직접 streaming call site가 없음을 확인했다. ClassDef v1 inventory와 future layout version/readback fixture 정책을 `RootSchemaCompatibility.md`에 고정했다. |
| 2026-07-13 | B0a/B4a validation + D2a groundwork | **DONE (B0a/B4a).** 2-file synthetic TChain으로 0/1/N trigger activation, `Full`, missing optional, rebind를 검증했고 late activation cache hook을 추가했다. `ColumnSource`에 epoch-bound contiguous float/byte snapshot을 추가하고 ASan/UBSan stale-snapshot smoke를 통과했다. |
| 2026-07-13 | 이전 구현 재감사 | codegen의 `run` key mutation, stale `Event` trigger provider, persistent runtime pointer, silent missing `ColumnView`를 수정했다. 생성물 3종 byte-identical, schema 6-test, C++ branch/trigger, Clang ASan/UBSan을 재통과했다. |
| 2026-07-13 | A3 typed handle | **DONE.** `GetColumnHandle<T>`/`GetScalarHandle<T>`와 required/optional, canonical wrapper, count leaf 검증, tree-transition 재검증, cache activation을 구현했다. `CalibrationTree` MyUParT ad-hoc branch read를 optional handle로 교체했다. |
| 2026-07-13 | A2a/A4 codegen | **DONE (A2a), IN PROGRESS (A4).** reviewed overlay에서 7개 input composition/bind/availability metadata를 build directory에 생성하고 dictionary dependency와 설치 경로를 연결했다. 생성 API fixture와 second-build no-op는 통과했으나 현재 shell은 Boost/correctionlib가 없어 전체 clean build를 실행하지 못했다. |
| 2026-07-13 | Track D opt-in engine 구현·재감사 | **DONE (API/smoke 범위).** D0–D9 infrastructure와 loader/MyCorrection/SystematicHelper/ORT seam을 연결했다. 전체 Release build/install과 CTest 21/21, targeted sanitizer, LTO/FastValidated/PGO kernel을 통과했다. D7 간헐 ROOT segfault를 찾아 thread-safety 초기화로 수정했다. Vcb 기본 승격과 production 5-run 성능 판정은 별도 adoption gate로 유지한다. |
| 2026-07-13 | 범용 유틸 단순화 | **DONE.** D7 event MT와 D8b batching 제거, RawHistogram1D→Boost.Histogram, PortableKernels→Google Highway, EventArena→`std::pmr`+epoch guard로 축소. Nano 환경에 libhwy 1.4.0 설치, 전체 Release build/install·CTest 18/18·FastValidated Highway 1 ULP test 통과. |
| 2026-07-13 | CalibrationTree single-core production gate | **BASELINE DONE.** 2024D Muon0 data 28,206 events, WCharm_Mu로 warm-up 1회 후 5-run median 1.826 s/MAD 0.004 s, 15.45k events/s. data optional branch 및 TChain cluster iterator correctness bug를 수정했고, 69 histograms + 202×99 tree payload의 2-run exact 일치를 확인했다. Highway/EventArena는 이 analyzer의 production path에 아직 연결되지 않아 성능 채택 판정은 보류한다. |
| 2026-07-13 | AnalyzerCore Highway A/B 및 전수검토 | **DROPPED for current jet workloads.** data jet selection은 1.3%, TT MC GenJet matching/JER는 2.1% 느려 production patch를 제거했다. AVX3 ΔR² kernel은 batch 16부터 1.08–2.43배, 32 이상에서 4배 이상 빨랐지만 실제 TT의 평균 GenJet은 7.75였다. 차기 후보는 호출 빈도와 GenPart multiplicity를 먼저 계측한 뒤 SoA gen-particle matching으로 한정한다. |
| 2026-07-13 | input owning DataFormats 제거 | **DONE.** hidden lazy materialization과 입력 owning 클래스를 제거하고 자동 first-access lazy view, event-scoped range, selected jet projection으로 통일했다. clean Release build/install, CTest 18/18, PyROOT API smoke와 production output exact comparison을 통과했다. |
