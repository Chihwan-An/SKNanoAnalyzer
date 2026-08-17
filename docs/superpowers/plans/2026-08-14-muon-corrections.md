# 뮤온 보정 계층 복원 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** NanoAODv15 프레임워크에 고pT 뮤온 보정(HighPt SF 배선, TuneP 200 GeV 분기, Generalized Endpoint scale, 고pT resolution smearing)을 복원하되 기존 뮤온 API의 동작은 유지한다.

**Architecture:** 기존 AK4 젯의 lazy-lane 패턴을 뮤온에 확장한다. `MyCorrection`이 correctionlib JSON 평가만 담당하고, `MuonSoA`가 보정 lane을 저장하며, `AnalyzerCore::PopulateMuonMomentum`이 lane을 채운다. 고pT 경로는 기존 `SelectMuonIndices`/`MuonView::Pt()`를 건드리지 않는 **opt-in** API(`SelectHighPtMuonIndices`, `MuonView::HighPtPt()`)로 노출한다.

**Tech Stack:** C++20, ROOT RNTuple, correctionlib, CMS CAT metadata JSON (cvmfs)

**설계 문서:** `docs/superpowers/specs/2026-08-14-object-corrections-design.md` (§4 전체)

## Global Constraints

- 대상 era는 **2024만** 배선한다. `MyCorrection::GetEraConfig`에 2024 블록만 존재하고 v15 RNTuple 샘플도 2024만 있다.
- `scripts/check_layers.sh` 규칙 준수: `AnalyzerTools`는 `AnalyzerCore.h`/`SKNanoLoader.h`/`BranchManager.h`/`Triggerinfo.h`를 include할 수 없고, `SKNanoCore`/`DataFormats`/`AnalyzerTools`에서 `exit()`/`abort()` 호출 금지.
- 실패는 예외로 처리한다: JSON/키 부재 → `SKNano::ConfigError`, 평가 실패 → `SKNano::CorrectionError`. 기존 `safeEvaluate`가 이미 이 규약을 따르므로 그대로 쓴다.
- **데이터 품질 문제로 인한 개별 오브젝트 이상은 예외가 아니라 경고 후 보정 생략**으로 처리한다. 기본 `FailurePolicy`가 `FailFast`라 예외 하나가 job을 죽인다.
- 맵/JSON이 없는 era는 throw하지 않고 **보정 없이 통과**시킨다.
- 기존 `SelectMuonIndices`와 `MuonView::Pt()`의 동작을 바꾸지 않는다. 단 Task 2는 예외이며 그 근거는 해당 태스크에 적는다.
- **빌드는 사용자가 직접 수행한다.** 각 태스크의 빌드 스텝에서 작업을 멈추고 사용자에게 빌드를 요청한 뒤 결과를 받아 진행한다. 절대 `scripts/build.sh`를 직접 실행하지 않는다.
- 커밋 메시지는 영어로 쓰고 `Co-Authored-By: Claude Fable 5 <noreply@anthropic.com>`로 끝낸다.
- 작업 브랜치는 `object-corrections-v13-parity`다.

## File Structure

| 파일 | 책임 | 태스크 |
|---|---|---|
| `scripts/checks/check_muon_corrections.py` | (신규) correctionlib으로 JSON 키 존재·평가 검증. C++ 빌드 없이 도는 유일한 자동 테스트 | 1, 4 |
| `AnalyzerTools/include/MyCorrection.h` | cset 멤버, EraConfig 필드, 메서드 선언 | 1, 2, 4, 5 |
| `AnalyzerTools/src/MyCorrection.cc` | JSON 로드, 2024 era config | 1 |
| `AnalyzerTools/src/MyCorrectionLeptons.cc` | 뮤온 SF 평가자, GE scale, 고pT 해상도 | 1, 2, 4, 5 |
| `DataFormats/include/MuonView.h` | `MuonSoA` lane, `MuonView` 접근자 | 3, 4, 5 |
| `AnalyzerFramework/src/AnalyzerCoreObjects.cc` | `PopulateMuonMomentum` 확장, 고pT 셀렉터 | 3, 4, 5 |
| `AnalyzerFramework/include/AnalyzerFramework/AnalyzerCore.h` | 셀렉터 선언 | 3 |

**핵심 경계:** `MyCorrection`은 순수 함수만 제공하고 뮤온 객체 상태를 모른다. regime 판정과 lane 채우기는 전부 `AnalyzerCore`가 한다. κ 맵과 σ 다항식 테이블은 `MyCorrectionLeptons.cc`의 익명 namespace에 두어 교체가 쉽도록 한다.

---

### Task 1: muon_HighPt.json.gz 로드와 HighPt 키 라우팅

**Files:**
- Create: `scripts/checks/check_muon_corrections.py`
- Modify: `AnalyzerTools/include/MyCorrection.h` (cset 멤버 `:379` 근처, EraConfig `:237` 근처)
- Modify: `AnalyzerTools/src/MyCorrection.cc` (`loadCorrectionSet` 블록 `:37` 근처, 2024 era config `:207` 근처)
- Modify: `AnalyzerTools/src/MyCorrectionLeptons.cc` (`GetMuonIDSF` / `GetMuonTriggerSF`)

**Interfaces:**
- Consumes: 없음 (첫 태스크)
- Produces: `cset_muon_highpt` (private member), `EraConfig::json_muon_highpt` (string). `GetMuonIDSF(const TString &key, const MuonView&, variation)`가 HighPt 키를 해석할 수 있게 된다.

**배경:** 2024 `muon_HighPt.json.gz`에 v13이 쓰던 4개 키가 모두 있으나 v15가 이 파일을 로드하지 않는다. 또 `json_muon_trig_sf`가 HLT 키가 0개인 `muon_Z.json.gz`를 가리켜 트리거 SF가 동작하지 않는다.

- [ ] **Step 1: JSON 키 검증 테스트 작성 (실패해야 함)**

`scripts/checks/check_muon_corrections.py`:

```python
#!/usr/bin/env python3
"""muon_HighPt.json.gz 의 키 존재와 평가 가능 여부를 확인한다.

C++ 빌드 없이 도는 검증. CI 나 로컬에서:
    python3 scripts/checks/check_muon_corrections.py
"""
import os
import sys

import correctionlib

ERA_TAG = "Run3-24CDEReprocessingFGHIPrompt-Summer24-NanoAODv15"

REQUIRED_KEYS = [
    "NUM_HighPtID_DEN_GlobalMuonProbes",
    "NUM_GlobalMuons_DEN_TrackerMuonProbes",
    "NUM_HLT_DEN_HighPtLooseRelIsoProbes",
    "NUM_probe_LooseRelTkIso_DEN_HighPtProbes",
]


def main() -> int:
    repo = os.environ.get("SKNANO_HOME", os.getcwd())
    path = os.path.join(
        repo, "CMS_corrections", "MUO", ERA_TAG, "latest", "muon_HighPt.json.gz"
    )
    if not os.path.exists(path):
        print(f"FAIL: missing {path}")
        return 1

    cset = correctionlib.CorrectionSet.from_file(path)
    failures = []
    for key in REQUIRED_KEYS:
        if key not in cset:
            failures.append(f"missing key: {key}")
            continue
        # eta, pt, scale_factors -- 대표값으로 평가가 되는지만 본다
        try:
            value = cset[key].evaluate(1.0, 200.0, "nominal")
        except Exception as exc:  # noqa: BLE001 - 진단 목적
            failures.append(f"{key}: evaluate raised {exc}")
            continue
        if not (0.5 < value < 1.5):
            failures.append(f"{key}: implausible SF {value}")
        else:
            print(f"OK  {key} -> {value:.4f}")

    if failures:
        for line in failures:
            print(f"FAIL: {line}")
        return 1
    print("all muon HighPt keys OK")
    return 0


if __name__ == "__main__":
    sys.exit(main())
```

- [ ] **Step 2: 테스트 실행해서 현재 상태 확인**

Run: `python3 scripts/checks/check_muon_corrections.py`

Expected: 4개 키 모두 `OK ... -> 0.9xxx` 출력 후 `all muon HighPt keys OK`.

이 테스트는 **JSON 쪽이 준비되어 있음을 확인**하는 것이므로 통과해야 정상이다. 만약 `evaluate raised` 가 나오면 인자 순서(`eta, pt, scale_factors`)나 `"nominal"` 문자열을 실제 JSON 스키마에 맞춰 이 스크립트를 먼저 고친다. C++ 쪽 인자도 여기에 맞춘다.

- [ ] **Step 3: EraConfig 필드와 cset 멤버 추가**

`AnalyzerTools/include/MyCorrection.h`, `string json_muon_trig_eff;` 다음 줄에 추가:

```cpp
        string json_muon_highpt;
```

같은 파일, `unique_ptr<CorrectionSet> cset_muon_trig_sf;` 다음 줄에 추가:

```cpp
    unique_ptr<CorrectionSet> cset_muon_highpt;
```

- [ ] **Step 4: JSON 로드와 2024 경로 배선**

`AnalyzerTools/src/MyCorrection.cc`, `loadCorrectionSet("muon SF", config.json_muon, cset_muon, true);` 다음 줄에 추가:

```cpp
  loadCorrectionSet("muon HighPt SF", config.json_muon_highpt, cset_muon_highpt,
                    true);
```

같은 파일 기본 경로 블록에서 `config.json_muon = json_pog_path_str + "/MUO";` 다음 줄에 추가:

```cpp
  config.json_muon_highpt = json_pog_path_str + "/MUO";
```

2024 블록(`if (era == "2024") {`)에서 `config.json_muon += tag + "muon_Z.json.gz";` 다음 줄에 추가하고, 바로 아래 `json_muon_trig_sf` 줄을 교체한다.

추가:
```cpp
    config.json_muon_highpt += tag + "muon_HighPt.json.gz";
```

교체 전:
```cpp
    config.json_muon_trig_sf += tag + "muon_Z.json.gz";
```
교체 후:
```cpp
    // muon_Z.json.gz 에는 HLT 키가 하나도 없다. 고pT 트리거 SF는 muon_HighPt 쪽에 있다.
    config.json_muon_trig_sf += tag + "muon_HighPt.json.gz";
```

- [ ] **Step 5: GetMuonIDSF 가 HighPt 키를 찾도록 라우팅**

`AnalyzerTools/src/MyCorrectionLeptons.cc`의 `GetMuonIDSF(const TString &key, const MuonView &muon, const variation syst)` 에서, `TopHNT` 분기 다음의 마지막 두 줄을 교체한다.

교체 전:
```cpp
  auto cset = cset_muon->at(string(key));
  return safeEvaluate(
      cset, "GetMuonIDSF",
      {fabs(muon.Eta()), muon.MiniAODPt(), getSystString_MUO(syst)});
```

교체 후:
```cpp
  // HighPt 계열 키는 muon_HighPt.json.gz 에만 있다. 먼저 그쪽을 보고,
  // 없으면 기존 muon_Z.json.gz 로 떨어진다 (GetMuonTriggerSF 와 같은 규약).
  const string keyStr(key.Data());
  if (cset_muon_highpt) {
    try {
      auto csetHighPt = cset_muon_highpt->at(keyStr);
      return safeEvaluate(
          csetHighPt, "GetMuonIDSF",
          {fabs(muon.Eta()), muon.MiniAODPt(), getSystString_MUO(syst)});
    } catch (const std::out_of_range &) {
      // 이 키는 HighPt 파일에 없다. 아래 기본 경로로 간다.
    }
  }
  auto cset = cset_muon->at(keyStr);
  return safeEvaluate(
      cset, "GetMuonIDSF",
      {fabs(muon.Eta()), muon.MiniAODPt(), getSystString_MUO(syst)});
```

- [ ] **Step 6: GetMuonTriggerSF 폴백 체인에 HighPt 추가**

같은 파일 `GetMuonTriggerSF(const TString &key, const MuonView &muon, const variation syst)` 에서 다음 한 줄을 교체한다.

교체 전:
```cpp
  if (tryEval(cset_muon_trig_sf) || tryEval(cset_muon))
```
교체 후:
```cpp
  if (tryEval(cset_muon_trig_sf) || tryEval(cset_muon_highpt) ||
      tryEval(cset_muon))
```

- [ ] **Step 7: 계층 규칙 확인**

Run: `bash scripts/check_layers.sh`

Expected: 모든 항목 `[check_layers] OK: ...`, 종료 코드 0.

- [ ] **Step 8: 빌드 요청 (사용자 수행)**

작업을 멈추고 사용자에게 요청한다:

> Task 1 코드 작성이 끝났습니다. `AnalyzerTools`만 변경했으니 빌드 부탁드립니다.
> 실패하면 에러 전문을 주세요.

빌드 성공을 확인하기 전에는 Step 9로 넘어가지 않는다.

- [ ] **Step 9: 커밋**

```bash
git add scripts/checks/check_muon_corrections.py \
        AnalyzerTools/include/MyCorrection.h \
        AnalyzerTools/src/MyCorrection.cc \
        AnalyzerTools/src/MyCorrectionLeptons.cc
git commit -m "$(cat <<'EOF'
Load muon_HighPt.json.gz and route high-pT muon scale factors

The high-pT muon ID, isolation, reco and trigger scale factors all live in
muon_HighPt.json.gz, which was never loaded, so none of those keys resolved.
The trigger SF was worse off than the rest: its era config pointed at
muon_Z.json.gz, which carries no HLT keys at all.

Loads the file into its own correction set, looks HighPt keys up there first
and falls back to the existing set, and repoints the trigger SF source.
Adds a correctionlib check that the four keys resolve and return plausible
values without needing a build.

Co-Authored-By: Claude Fable 5 <noreply@anthropic.com>
EOF
)"
```

---

### Task 2: 뮤온 SF 평가 pt 를 보정 후 pt 로 교정

**Files:**
- Modify: `AnalyzerTools/src/MyCorrectionLeptons.cc` (`GetMuonIDSF` 2곳, `GetMuonTriggerSF`, `GetMuonRECOSF`)

**Interfaces:**
- Consumes: Task 1의 `cset_muon_highpt` 라우팅
- Produces: 시그니처 변화 없음. 동작만 바뀐다.

**배경 및 주의:** 현재 v15는 SF를 `muon.MiniAODPt()`(raw NanoAOD pt)에서 평가한다. v13은 모멘텀 보정 후 pt(`OriginalPt`)를 썼고, POG 규약도 보정 후 pt 기준이다. **이 변경은 뮤온 SF를 쓰는 모든 분석의 결과를 바꾼다.** 설계 §2.3의 예외 5개 중 하나이며, 리뷰에서 반대가 있으면 되돌릴 수 있도록 별도 커밋으로 분리한다.

`MuonView::Pt()`가 이미 Rochester 보정 후 pt이므로 `MiniAODPt()` → `Pt()` 치환이면 된다.

- [ ] **Step 1: 치환 대상 확인**

Run: `grep -n "MiniAODPt()" AnalyzerTools/src/MyCorrectionLeptons.cc`

Expected: `GetMuonIDSF`(2곳: TopHNT 분기 3줄 + 일반 경로 2곳), `GetMuonTriggerSF`, `GetMuonRECOSF`에서 히트. 출력된 줄 번호를 기록한다.

- [ ] **Step 2: 일반 뮤온 SF 경로 치환**

`GetMuonIDSF(const TString &key, const MuonView &muon, ...)`의 **HighPt 분기와 기본 경로 두 곳** 모두에서:

```cpp
      {fabs(muon.Eta()), muon.MiniAODPt(), getSystString_MUO(syst)});
```
를
```cpp
      {fabs(muon.Eta()), muon.Pt(), getSystString_MUO(syst)});
```
로 바꾼다.

`GetMuonTriggerSF(const TString &key, const MuonView &muon, ...)`에서:

```cpp
                            {muon.Eta(), std::max(26.f, muon.MiniAODPt()),
```
를
```cpp
                            {muon.Eta(), std::max(26.f, muon.Pt()),
```
로 바꾼다.

`GetMuonRECOSF(const MuonView &muon, ...)`에서:

```cpp
                      {muon.Eta(), std::max(40.f, muon.MiniAODPt()),
```
를
```cpp
                      {muon.Eta(), std::max(40.f, muon.Pt()),
```
로 바꾼다.

**`TopHNT` 분기는 바꾸지 않는다.** 그 커스텀 JSON은 raw pt 기준으로 만들어졌고 다른 분석 소유다.

- [ ] **Step 3: TopHNT 분기가 그대로인지 확인**

Run: `grep -n "MiniAODPt()" AnalyzerTools/src/MyCorrectionLeptons.cc`

Expected: `TopHNT` 분기의 3줄만 남아 있어야 한다. 다른 히트가 남아 있으면 Step 2를 놓친 것이다.

- [ ] **Step 4: 계층 규칙 확인**

Run: `bash scripts/check_layers.sh`

Expected: 종료 코드 0.

- [ ] **Step 5: 빌드 요청 (사용자 수행)**

> Task 2 코드 작성이 끝났습니다. 빌드 부탁드립니다.

- [ ] **Step 6: 커밋**

```bash
git add AnalyzerTools/src/MyCorrectionLeptons.cc
git commit -m "$(cat <<'EOF'
Evaluate muon scale factors at the corrected pt

The muon ID, isolation, trigger and reco scale factors were being evaluated
at the raw NanoAOD pt while the POG derives them against the corrected one,
so every muon picked up a scale factor from slightly the wrong bin.

Switches those four call sites to MuonView::Pt(), which already carries the
momentum correction. The TopHNT branch keeps the raw pt: that custom JSON was
built against it and belongs to another analysis.

This changes yields for any analysis using muon scale factors.

Co-Authored-By: Claude Fable 5 <noreply@anthropic.com>
EOF
)"
```

---

### Task 3: TuneP 접근자와 고pT regime latch

**Files:**
- Modify: `DataFormats/include/MuonView.h` (`MuonSoA` 필드/lane, `MuonView` 접근자)
- Modify: `AnalyzerFramework/src/AnalyzerCoreObjects.cc` (`GetAllMuonViews` 바인딩, `PopulateMuonMomentum`)
- Modify: `AnalyzerFramework/include/AnalyzerFramework/AnalyzerCore.h` (`SelectHighPtMuonIndices` 선언)

**Interfaces:**
- Consumes: 없음 (Task 1·2와 독립)
- Produces:
  - `float MuonView::TunePPt() const` — `Muon_pt * Muon_tunepRelPt` (보정 전)
  - `bool MuonView::IsHighPtRegime() const` — `TunePPt() >= 200`, populate 시점에 latch됨
  - `std::vector<std::size_t> AnalyzerCore::SelectHighPtMuonIndices(const MuonViewCollection&, const MuonView::MuonID, const float ptmin, const float fetamax) const`

**배경:** MUO POG "High pT: Momentum Scale" approach 1. regime 판정은 **보정 전 TuneP pT**로 하고 결과를 latch해야 한다. 보정 후 pT로 재판정하면 GE/smearing이 200 GeV를 넘나들게 만든 뮤온이 오분류된다.

이 태스크는 regime 판정과 접근자만 만든다. GE 보정은 Task 4, smearing은 Task 5다.

- [ ] **Step 1: MuonSoA 에 tunepRelPt 컬럼과 regime lane 추가**

`DataFormats/include/MuonView.h`의 `MuonSoA` 안, `ColumnView<unsigned char> nTrackerLayers;` 근처에 추가:

```cpp
    ColumnView<float> tunepRelPt;
```

같은 struct의 lane 저장부(`correctedPt`, `momentumScaleUp` 등이 선언된 곳)에 추가:

```cpp
    std::vector<float> tunePPt;
    std::vector<unsigned char> highPtRegime;
```

- [ ] **Step 2: MuonView 접근자 추가**

같은 파일 `MuonView`의 `float MiniAODPt() const` 근처에 추가:

```cpp
    // 보정 전 TuneP pT. regime 판정 기준이며 고pT 구간의 momentum estimator다.
    float TunePPt() const {
        assertCurrentEvent();
        return idx < store->tunePPt.size() ? store->tunePPt[idx] : Pt();
    }
    // PopulateMuonMomentum 이 보정 전 TuneP pT 로 판정해 latch 해 둔 값.
    // 보정 후 pT 로 다시 판정하면 200 GeV 를 넘나든 muon 이 오분류된다.
    bool IsHighPtRegime() const {
        assertCurrentEvent();
        return idx < store->highPtRegime.size() && store->highPtRegime[idx] != 0;
    }
```

- [ ] **Step 3: 브랜치 바인딩 추가**

`AnalyzerFramework/src/AnalyzerCoreObjects.cc`의 `GetAllMuonViews()`에서 `storage->nTrackerLayers.bind(&Muon_nTrackerLayers);` 근처에 추가:

```cpp
  storage->tunepRelPt.bind(&Muon_tunepRelPt);
```

- [ ] **Step 4: PopulateMuonMomentum 에서 TuneP 계산과 regime latch**

같은 파일 `PopulateMuonMomentum`의 앞부분, 기존 `storage->momentumScaleDown.assign(count, 0.f);` 다음 줄에 추가:

```cpp
  storage->tunePPt.assign(count, 0.f);
  storage->highPtRegime.assign(count, 0);
```

루프 안에서 `const int charge = storage->charge[index];` 다음 줄에 추가:

```cpp
    // 보정 전 TuneP pT. MUO POG 의 200 GeV regime 경계 판정에 쓴다.
    const float tunePPt = pt * storage->tunepRelPt[index];
    storage->tunePPt[index] = tunePPt;
    storage->highPtRegime[index] =
        (tunePPt >= kHighPtMuonMinPt) ? 1 : 0;
```

같은 파일 상단의 익명 namespace(없으면 include 다음에 생성)에 추가:

```cpp
namespace {
// MUO POG "High pT" 처방의 regime 경계.
constexpr float kHighPtMuonMinPt = 200.f;
} // namespace
```

- [ ] **Step 5: 고pT 셀렉터 선언과 구현**

`AnalyzerFramework/include/AnalyzerFramework/AnalyzerCore.h`의 `SelectMuonIndices` 선언 두 개 다음에 추가:

```cpp
    // 고pT 뮤온 전용 셀렉터. pt 컷을 raw pT 가 아니라 HighPtPt() 로 판정한다.
    // 기본 SelectMuonIndices 의 동작은 바뀌지 않으므로 opt-in 이다.
    std::vector<std::size_t> SelectHighPtMuonIndices(const MuonViewCollection &muons, const MuonView::MuonID ID, const float ptmin, const float fetamax) const;
```

`AnalyzerFramework/src/AnalyzerCoreObjects.cc`의 `SelectMuonIndices` 구현 다음에 추가:

```cpp
std::vector<std::size_t> AnalyzerCore::SelectHighPtMuonIndices(
    const MuonViewCollection &muons, const MuonView::MuonID ID,
    const float ptmin, const float fetamax) const {
  std::vector<std::size_t> selected;
  selected.reserve(muons.size());
  for (std::size_t index = 0; index < muons.size(); ++index) {
    const auto &muon = muons[index];
    if (muon.HighPtPt() <= ptmin)
      continue;
    if (std::fabs(muon.Eta()) >= fetamax)
      continue;
    if (!muon.PassID(ID))
      continue;
    selected.push_back(index);
  }
  return selected;
}
```

**주의:** `HighPtPt()`는 Task 4에서 정의한다. 이 태스크에서는 컴파일이 되도록
Task 4의 Step 1을 먼저 적용하거나, 이 셀렉터를 Task 4로 미룬다.
**권장: 이 Step 5를 Task 4로 옮기고, Task 3은 Step 1–4 + 검증만으로 끝낸다.**

- [ ] **Step 6: 계층 규칙 확인**

Run: `bash scripts/check_layers.sh`

Expected: 종료 코드 0.

- [ ] **Step 7: 빌드 요청 (사용자 수행)**

> Task 3 코드 작성이 끝났습니다. `DataFormats` 헤더를 고쳤으니 **dictionary 재생성이 필요할 수 있습니다.**
> 일반 빌드가 실패하면 clean 빌드로 한 번 더 시도해 주시고, 에러 전문을 주세요.

- [ ] **Step 8: 커밋**

```bash
git add DataFormats/include/MuonView.h \
        AnalyzerFramework/src/AnalyzerCoreObjects.cc
git commit -m "$(cat <<'EOF'
Expose TuneP pt and latch the high-pT muon regime

The MUO POG high-pT prescription splits muons at 200 GeV, and the split has
to be decided on the pre-correction TuneP pt. Deciding it later, on corrected
values, misclassifies any muon that the scale correction or the smearing
pushed across the boundary.

Binds Muon_tunepRelPt, computes TuneP pt while populating muon momentum, and
stores the regime alongside it so later passes read the same answer.

Co-Authored-By: Claude Fable 5 <noreply@anthropic.com>
EOF
)"
```

---

### Task 4: Generalized Endpoint scale 보정

**Files:**
- Modify: `AnalyzerTools/include/MyCorrection.h` (`GetMuonGEScaledPt` 선언)
- Modify: `AnalyzerTools/src/MyCorrectionLeptons.cc` (κ 맵 테이블, `GetMuonGEScaledPt` 구현)
- Modify: `DataFormats/include/MuonView.h` (`HighPtPt`, `HighPtScaleUp/Down` lane과 접근자)
- Modify: `AnalyzerFramework/src/AnalyzerCoreObjects.cc` (lane 채우기, Task 3에서 미룬 셀렉터)
- Modify: `AnalyzerFramework/include/AnalyzerFramework/AnalyzerCore.h` (셀렉터 선언)
- Modify: `scripts/checks/check_muon_corrections.py` (κ 맵 sanity 추가)

**Interfaces:**
- Consumes: Task 3의 `MuonView::TunePPt()`, `IsHighPtRegime()`, `MuonSoA::tunePPt/highPtRegime`
- Produces:
  - `float MyCorrection::GetMuonGEScaledPt(const float pt, const float eta, const float phi, const int charge, const variation syst = variation::nom) const`
  - `float MuonView::HighPtPt() const`, `HighPtScaleUp() const`, `HighPtScaleDown() const`
  - `AnalyzerCore::SelectHighPtMuonIndices(...)` (Task 3 Step 5에서 이관)

**배경:** GE 보정식은 `pT_corr = pT / (1 - q · κ_b(η,φ) · pT[TeV])`이며 **DATA에만** 적용한다 (AN-2018/008: κ_b는 데이터에 속하는 바이어스). MUO POG twiki가 2022~2023BPix κ_b를 3φ × 6η 격자로 게시하고 v13 `GE_KAPPA`가 이 값과 일치한다. **2024 값은 격자가 달라 미확정**이므로(설계 §9 항목 1) 이 태스크에서는 2024 항목을 비워 둔다. 맵이 없는 era는 보정 없이 통과한다.

- [ ] **Step 1: κ 맵 테이블과 GE 평가자 구현**

`AnalyzerTools/src/MyCorrectionLeptons.cc` 상단 익명 namespace에 추가:

```cpp
namespace {
// Generalized Endpoint 곡률 바이어스 κ_b (단위 1/TeV) 와 그 오차.
// 출처: MUO POG twiki "High pT: Momentum Scale" (방법론은 AN-2018/008).
// 격자는 [phi bin][eta bin]. phi 는 [-180,-60), [-60,60), [60,180] degree,
// eta 는 [-2.4,-2.1), [-2.1,-1.2), [-1.2,0), [0,1.2), [1.2,2.1), [2.1,2.4].
// eta 는 부호를 유지한다 - |eta| 로 접지 말 것.
//
// 2024/2025 는 격자 정의가 달라(docs/highptmuons.pdf) 아직 넣지 않았다.
// 맵이 없는 era 는 보정 없이 통과한다.
struct GECell {
  float kappa;
  float sigma;
};
using GEMap = std::array<std::array<GECell, 6>, 3>;

const std::unordered_map<std::string, GEMap> kGeKappa = {
    {"2022",
     {{
         {{{-0.16f, 0.10f}, {-0.03f, 0.05f}, {-0.05f, 0.04f},
           {0.00f, 0.04f}, {0.07f, 0.06f}, {-0.06f, 0.11f}}},
         {{{0.11f, 0.10f}, {-0.01f, 0.06f}, {0.06f, 0.04f},
           {0.02f, 0.03f}, {0.05f, 0.05f}, {-0.06f, 0.10f}}},
         {{{0.17f, 0.11f}, {0.16f, 0.04f}, {-0.04f, 0.04f},
           {-0.01f, 0.03f}, {0.04f, 0.06f}, {-0.00f, 0.09f}}},
     }}},
    {"2022EE",
     {{
         {{{-0.12f, 0.05f}, {-0.03f, 0.03f}, {0.013f, 0.022f},
           {0.029f, 0.023f}, {-0.04f, 0.03f}, {-0.28f, 0.05f}}},
         {{{0.24f, 0.05f}, {0.10f, 0.03f}, {-0.006f, 0.022f},
           {-0.047f, 0.022f}, {-0.14f, 0.03f}, {-0.48f, 0.05f}}},
         {{{0.28f, 0.05f}, {0.07f, 0.03f}, {-0.028f, 0.022f},
           {0.018f, 0.022f}, {0.07f, 0.03f}, {0.05f, 0.05f}}},
     }}},
    {"2023",
     {{
         {{{-0.21f, 0.06f}, {-0.01f, 0.04f}, {0.010f, 0.027f},
           {-0.045f, 0.030f}, {0.01f, 0.04f}, {0.00f, 0.07f}}},
         {{{0.08f, 0.07f}, {-0.04f, 0.04f}, {0.070f, 0.025f},
           {0.030f, 0.027f}, {-0.13f, 0.04f}, {-0.36f, 0.06f}}},
         {{{0.27f, 0.07f}, {0.05f, 0.04f}, {0.054f, 0.026f},
           {0.020f, 0.027f}, {0.00f, 0.04f}, {-0.04f, 0.06f}}},
     }}},
    {"2023BPix",
     {{
         {{{-0.25f, 0.08f}, {0.07f, 0.05f}, {0.02f, 0.04f},
           {-0.02f, 0.04f}, {0.08f, 0.06f}, {0.12f, 0.09f}}},
         {{{-0.05f, 0.08f}, {0.00f, 0.05f}, {0.05f, 0.03f},
           {-0.05f, 0.04f}, {-0.17f, 0.05f}, {-0.33f, 0.09f}}},
         {{{0.24f, 0.09f}, {0.09f, 0.05f}, {-0.01f, 0.03f},
           {0.03f, 0.04f}, {0.04f, 0.06f}, {-0.20f, 0.07f}}},
     }}},
};

int GeEtaBin(const float eta) {
  if (eta < -2.4f || eta > 2.4f)
    return -1;
  if (eta < -2.1f)
    return 0;
  if (eta < -1.2f)
    return 1;
  if (eta < 0.f)
    return 2;
  if (eta < 1.2f)
    return 3;
  if (eta < 2.1f)
    return 4;
  return 5;
}

int GePhiBin(const float phi) {
  const float deg = phi * 180.f / static_cast<float>(M_PI);
  if (deg < -60.f)
    return 0;
  if (deg < 60.f)
    return 1;
  return 2;
}
} // namespace
```

같은 파일에 평가자 구현을 추가:

```cpp
float MyCorrection::GetMuonGEScaledPt(const float pt, const float eta,
                                      const float phi, const int charge,
                                      const variation syst) const {
  const auto it = kGeKappa.find(GetEra().Data());
  if (it == kGeKappa.end())
    return pt; // 맵이 없는 era (Run2, 2024+) 는 보정하지 않는다.

  const int ieta = GeEtaBin(eta);
  if (ieta < 0)
    return pt;
  const GECell &cell = it->second[GePhiBin(phi)][ieta];

  float kappa = cell.kappa;
  if (syst == variation::up)
    kappa += cell.sigma;
  else if (syst == variation::down)
    kappa -= cell.sigma;

  // k_meas = k_true + kappa  =>  pT_corr = pT / (1 - q * kappa * pT[TeV])
  const float ptTeV = pt / 1000.f;
  const float denom = 1.f - static_cast<float>(charge) * kappa * ptTeV;

  // 극 근처 가드: kappa*pT -> 1 이면 보정이 발산한다
  // (예: kappa = -0.48/TeV, pT = 2 TeV). 폭주한 muon 을 내보내느니 raw pT 를 쓴다.
  if (std::fabs(denom) < 0.1f) {
    std::cerr << "[MyCorrection::GetMuonGEScaledPt] Near-singular GE correction"
              << " (denom = " << denom << ", kappa = " << kappa
              << " TeV^-1, pT = " << pt << " GeV, q = " << charge
              << "); keeping the uncorrected pT." << std::endl;
    return pt;
  }
  return pt / denom;
}
```

`AnalyzerTools/include/MyCorrection.h`의 `GetMuonScaleAndError` 선언 다음에 추가:

```cpp
    // Generalized Endpoint 고pT 모멘텀 scale (DATA 전용, pT >= 200 GeV).
    // 맵이 없는 era 는 pt 를 그대로 돌려준다.
    float GetMuonGEScaledPt(const float pt, const float eta, const float phi,
                            const int charge,
                            const variation syst = variation::nom) const;
```

- [ ] **Step 2: MuonSoA lane 과 MuonView 접근자 추가**

`DataFormats/include/MuonView.h`의 `MuonSoA` lane 저장부에 추가:

```cpp
    std::vector<float> highPtPt;
    std::vector<float> highPtScaleUp;
    std::vector<float> highPtScaleDown;
```

`MuonView`에 `TunePPt()` 다음으로 추가:

```cpp
    // 고pT 처방을 적용한 pT.
    // TuneP pT < 200 이면 medium-pT 보정 pT, 그 이상이면 TuneP + GE 보정.
    float HighPtPt() const {
        assertCurrentEvent();
        return idx < store->highPtPt.size() ? store->highPtPt[idx] : Pt();
    }
    float HighPtScaleUp() const {
        assertCurrentEvent();
        return idx < store->highPtScaleUp.size() ? store->highPtScaleUp[idx] : HighPtPt();
    }
    float HighPtScaleDown() const {
        assertCurrentEvent();
        return idx < store->highPtScaleDown.size() ? store->highPtScaleDown[idx] : HighPtPt();
    }
```

- [ ] **Step 3: PopulateMuonMomentum 에서 고pT lane 채우기**

`AnalyzerFramework/src/AnalyzerCoreObjects.cc`의 `PopulateMuonMomentum` 앞부분 assign 블록에 추가:

```cpp
  storage->highPtPt.assign(count, 0.f);
  storage->highPtScaleUp.assign(count, 0.f);
  storage->highPtScaleDown.assign(count, 0.f);
```

루프 끝, `storage->correctedPt[index] = pt * scale.scale;` 다음에 추가:

```cpp
    // 고pT 경로. 저pT 는 medium-pT 보정 결과를 그대로 쓰고,
    // 고pT 는 TuneP 로 갈아탄 뒤 DATA 에만 GE 보정을 얹는다.
    // MC 는 GE nominal 이 없다 (바이어스는 데이터에 속한다).
    if (storage->highPtRegime[index] == 0) {
      storage->highPtPt[index] = storage->correctedPt[index];
      storage->highPtScaleUp[index] = storage->momentumScaleUp[index];
      storage->highPtScaleDown[index] = storage->momentumScaleDown[index];
    } else if (!myCorr) {
      storage->highPtPt[index] = tunePPt;
      storage->highPtScaleUp[index] = tunePPt;
      storage->highPtScaleDown[index] = tunePPt;
    } else if (IsDATA) {
      storage->highPtPt[index] = myCorr->GetMuonGEScaledPt(
          tunePPt, eta, phi, charge, MyCorrection::variation::nom);
      storage->highPtScaleUp[index] = myCorr->GetMuonGEScaledPt(
          tunePPt, eta, phi, charge, MyCorrection::variation::up);
      storage->highPtScaleDown[index] = myCorr->GetMuonGEScaledPt(
          tunePPt, eta, phi, charge, MyCorrection::variation::down);
    } else {
      storage->highPtPt[index] = tunePPt;
      storage->highPtScaleUp[index] = myCorr->GetMuonGEScaledPt(
          tunePPt, eta, phi, charge, MyCorrection::variation::up);
      storage->highPtScaleDown[index] = myCorr->GetMuonGEScaledPt(
          tunePPt, eta, phi, charge, MyCorrection::variation::down);
    }
```

- [ ] **Step 4: 고pT 셀렉터 추가 (Task 3 Step 5에서 이관)**

Task 3 Step 5의 선언과 구현을 지금 적용한다. 코드는 Task 3 Step 5에 그대로 있다.

- [ ] **Step 5: κ 맵 sanity 테스트 추가**

`scripts/checks/check_muon_corrections.py`의 `main()` 안, `print("all muon HighPt keys OK")` 앞에 추가:

```python
    # GE 보정식의 극 가드가 필요한 구간인지 확인한다.
    # kappa * pT[TeV] 가 1 에 가까우면 pT/(1-q*kappa*pT) 가 발산한다.
    worst_kappa = 0.48  # 2022EE forward, 1/TeV
    for pt_gev in (200.0, 1000.0, 2000.0, 4000.0):
        denom = 1.0 - worst_kappa * pt_gev / 1000.0
        if abs(denom) < 0.1:
            print(
                f"NOTE kappa={worst_kappa}/TeV at pT={pt_gev:.0f} GeV gives "
                f"denom={denom:.3f} -> guard must fire"
            )
```

- [ ] **Step 6: 테스트 실행**

Run: `python3 scripts/checks/check_muon_corrections.py`

Expected: 기존 4개 키 OK + `NOTE kappa=0.48/TeV at pT=2000 GeV gives denom=0.040 -> guard must fire` (2000, 4000 GeV에서). 이 NOTE가 나오는 것이 정상이며, C++ 가드가 실제로 필요함을 보여준다.

- [ ] **Step 7: 계층 규칙 확인**

Run: `bash scripts/check_layers.sh`

Expected: 종료 코드 0. 특히 `AnalyzerTools` 에서 `exit()` 금지 규칙에 걸리지 않아야 한다 (`std::cerr` 경고만 쓰고 있으므로 통과).

- [ ] **Step 8: 빌드 요청 (사용자 수행)**

> Task 4 코드 작성이 끝났습니다. `DataFormats` 헤더가 또 바뀌었으니 clean 빌드가 필요할 수 있습니다. 빌드 부탁드립니다.

- [ ] **Step 9: 커밋**

```bash
git add AnalyzerTools/include/MyCorrection.h \
        AnalyzerTools/src/MyCorrectionLeptons.cc \
        DataFormats/include/MuonView.h \
        AnalyzerFramework/include/AnalyzerFramework/AnalyzerCore.h \
        AnalyzerFramework/src/AnalyzerCoreObjects.cc \
        scripts/checks/check_muon_corrections.py
git commit -m "$(cat <<'EOF'
Add the Generalized Endpoint scale for high-pT muons

Above 200 GeV the momentum comes largely from the muon system, so the scale
bias measured at the Z peak with tracker-only information does not describe
it. The MUO POG measures that bias with the Generalized Endpoint method and
recommends correcting Run 3 momenta with it rather than only assigning an
uncertainty.

Adds the kappa_b maps for 2022 through 2023BPix from the POG twiki, applies
the correction to data only, and exposes it through HighPtPt() and a
SelectHighPtMuonIndices() selector so the default muon path is unchanged.
2024 is left out: its map uses a different binning and is not settled yet,
and eras without a map pass through uncorrected.

Guards the pole at kappa*pT -> 1, which a multi-TeV muon can reach.

Co-Authored-By: Claude Fable 5 <noreply@anthropic.com>
EOF
)"
```

---

### Task 5: 고pT 해상도 스미어링과 MuonRes variation

**Files:**
- Modify: `AnalyzerTools/include/MyCorrection.h` (선언 2개)
- Modify: `AnalyzerTools/src/MyCorrectionLeptons.cc` (σ 다항식 테이블, 구현 2개)
- Modify: `DataFormats/include/MuonView.h` (`HighPtResUp/Down` lane과 접근자)
- Modify: `AnalyzerFramework/src/AnalyzerCoreObjects.cc` (lane 채우기)

**Interfaces:**
- Consumes: Task 3의 `IsHighPtRegime()`, Task 4의 `highPtPt` lane
- Produces:
  - `float MyCorrection::GetMuonHighPtResolution(const float p, const float eta) const`
  - `float MyCorrection::GetMuonHighPtSmearFactor(const float p, const float eta, const unsigned int seed, const variation syst = variation::nom) const`
  - `float MuonView::HighPtResUp() const`, `HighPtResDown() const`

**배경:** **MC 전용, 고pT regime 전용.** nominal은 σ(p,η) 3차 다항식 × era·η별 f 계수이고, systematic은 era·η 무관 평탄 10%다. 변수는 pT가 아니라 **전체 운동량 p**다. **2024는 POG가 WIP로 두어 맵이 없다** — 테이블이 비면 자동 no-op이 된다.

f_nom = 0인 era·η(2022 barrel, 2023BPix barrel)에서는 down이 nominal과 같아져 **one-sided nuisance**가 된다.

- [ ] **Step 1: σ 다항식·f 테이블과 평가자 구현**

`AnalyzerTools/src/MyCorrectionLeptons.cc` 익명 namespace(Task 4에서 만든 것)에 추가:

```cpp
// 고pT muon 해상도. 변수는 pT 가 아니라 전체 운동량 p 다
// (원 측정이 (1/p - 1/p_gen)/(1/p_gen) 를 p bin 으로 fit 했다).
// 출처: Schulte & Zhong, "High-pT muon resolution measurement for 2022 and
// 2023" (2024-06-03).
// 인덱스 [0] = barrel |eta| < 1.2, [1] = forward 1.2 - 2.4.
//
// 2024 는 MUO POG 가 Work In Progress 로 두고 있어 넣지 않았다. 테이블이 없으면
// nominal smearing 은 no-op 이 되고 uncertainty 만 남는다.
struct MuonResPoly {
  float a0, a1, a2, a3;
};
using MuonResSet = std::array<MuonResPoly, 2>;

const std::unordered_map<std::string, MuonResSet> kMuonResPoly = {
    {"2022",
     {{{0.01152f, 5.95e-5f, -2.92e-8f, 5.14e-12f},
       {0.01405f, 5.28e-5f, -1.90e-8f, 3.01e-12f}}}},
    {"2022EE",
     {{{0.0126f, 5.89e-5f, -2.85e-8f, 4.92e-12f},
       {0.0150f, 4.81e-5f, -1.42e-8f, 1.95e-12f}}}},
    {"2023",
     {{{0.0172f, 6.15e-5f, -3.14e-8f, 5.82e-12f},
       {0.01424f, 5.31e-5f, -1.92e-8f, 3.21e-12f}}}},
    {"2023BPix",
     {{{0.0118f, 6.14e-5f, -3.12e-8f, 5.74e-12f},
       {0.0141f, 5.38e-5f, -2.00e-8f, 3.42e-12f}}}},
};

// 추가 smearing 계수 f = sqrt(smearfac^2 - 1). 10% -> 0.458, 5% -> 0.320.
// f = 0 은 그 era/eta 에서 추가 smearing 이 필요 없다는 뜻이다.
const std::unordered_map<std::string, std::array<float, 2>> kMuonSmearF = {
    {"2022", {{0.000f, 0.320f}}},
    {"2022EE", {{0.320f, 0.460f}}},
    {"2023", {{0.320f, 0.460f}}},
    {"2023BPix", {{0.000f, 0.568f}}},
};

// systematic 은 era/eta 무관 평탄 10%: sqrt(1.10^2 - 1).
constexpr float kMuonSmearSystF = 0.4583f;

int MuonResEtaBin(const float eta) {
  return (std::fabs(eta) < 1.2f) ? 0 : 1;
}
```

같은 파일에 구현 추가:

```cpp
float MyCorrection::GetMuonHighPtResolution(const float p,
                                            const float eta) const {
  const auto it = kMuonResPoly.find(GetEra().Data());
  if (it == kMuonResPoly.end())
    return 0.f;
  const MuonResPoly &c = it->second[MuonResEtaBin(eta)];
  return c.a0 + c.a1 * p + c.a2 * p * p + c.a3 * p * p * p;
}

float MyCorrection::GetMuonHighPtSmearFactor(const float p, const float eta,
                                             const unsigned int seed,
                                             const variation syst) const {
  if (IsDATA)
    return 1.f;
  const auto itF = kMuonSmearF.find(GetEra().Data());
  if (itF == kMuonSmearF.end())
    return 1.f; // 맵이 없는 era (Run2, 2024+) 는 smearing 하지 않는다.

  const float fNom = itF->second[MuonResEtaBin(eta)];
  float f = fNom;
  if (syst == variation::up) {
    // 독립 가우시안이므로 폭은 quadrature 로 더한다.
    f = std::sqrt(fNom * fNom + kMuonSmearSystF * kMuonSmearSystF);
  } else if (syst == variation::down) {
    // 가우시안 convolution 은 해상도를 나쁘게만 만들 수 있으므로 0 에서 자른다.
    // 따라서 f_nom = 0 인 era/eta 에서 MuonRes 는 one-sided 가 된다.
    f = std::sqrt(std::max(fNom * fNom - kMuonSmearSystF * kMuonSmearSystF, 0.f));
  }
  if (f <= 0.f)
    return 1.f;

  const float sigma = GetMuonHighPtResolution(p, eta);
  if (sigma <= 0.f)
    return 1.f;

  // nominal 과 variation 이 같은 seed 를 쓴다. pull 을 공유해야 coherent shift 가
  // 되고, 독립 re-smearing 이면 nominal 위에 해상도가 또 더해져 double-count 다.
  TRandom3 rng(seed);
  return 1.f + rng.Gaus(0.f, sigma * f);
}
```

`AnalyzerTools/include/MyCorrection.h`의 `GetMuonGEScaledPt` 선언 다음에 추가:

```cpp
    // 고pT muon 해상도 sigma(p, eta). 변수는 pT 가 아니라 전체 운동량 p 다.
    float GetMuonHighPtResolution(const float p, const float eta) const;
    // MC 전용 추가 smearing factor. 맵이 없는 era 는 1 을 돌려준다.
    float GetMuonHighPtSmearFactor(const float p, const float eta,
                                   const unsigned int seed,
                                   const variation syst = variation::nom) const;
```

`MyCorrection.h` 상단 include에 `TRandom3.h`가 없으면 추가한다:

```cpp
#include "TRandom3.h"
```

- [ ] **Step 2: MuonSoA lane 과 MuonView 접근자 추가**

`DataFormats/include/MuonView.h`의 `MuonSoA` lane 저장부에 추가:

```cpp
    std::vector<float> highPtResUp;
    std::vector<float> highPtResDown;
```

`MuonView`에 `HighPtScaleDown()` 다음으로 추가:

```cpp
    // MuonRes variation. 고pT regime 의 MC muon 에만 nominal 과 다르다.
    float HighPtResUp() const {
        assertCurrentEvent();
        return idx < store->highPtResUp.size() ? store->highPtResUp[idx] : HighPtPt();
    }
    float HighPtResDown() const {
        assertCurrentEvent();
        return idx < store->highPtResDown.size() ? store->highPtResDown[idx] : HighPtPt();
    }
```

- [ ] **Step 3: PopulateMuonMomentum 에서 해상도 lane 채우기**

`AnalyzerFramework/src/AnalyzerCoreObjects.cc`의 assign 블록에 추가:

```cpp
  storage->highPtResUp.assign(count, 0.f);
  storage->highPtResDown.assign(count, 0.f);
```

Task 4 Step 3에서 추가한 고pT 블록 **다음에** 추가:

```cpp
    // 고pT 해상도 smearing. MC 의 고pT regime 에만 적용한다.
    // 저pT 구간 해상도는 Rochester 가 이미 처리하고 그 오차는 MuonScale 로
    // 들어가므로, 여기서 또 흔들면 이중계산이 된다.
    if (IsDATA || storage->highPtRegime[index] == 0 || !myCorr) {
      storage->highPtResUp[index] = storage->highPtPt[index];
      storage->highPtResDown[index] = storage->highPtPt[index];
    } else {
      const float momentum =
          storage->highPtPt[index] * std::cosh(eta);
      const unsigned int seed = MuonSmearSeed(event.get(), eta, phi);
      const float nomFactor = myCorr->GetMuonHighPtSmearFactor(
          momentum, eta, seed, MyCorrection::variation::nom);
      const float upFactor = myCorr->GetMuonHighPtSmearFactor(
          momentum, eta, seed, MyCorrection::variation::up);
      const float downFactor = myCorr->GetMuonHighPtSmearFactor(
          momentum, eta, seed, MyCorrection::variation::down);
      // nominal smearing 을 lane 에 반영하고, variation 은 비율로 얹는다.
      storage->highPtPt[index] *= nomFactor;
      storage->highPtResUp[index] =
          storage->highPtPt[index] * (nomFactor > 0.f ? upFactor / nomFactor : 1.f);
      storage->highPtResDown[index] =
          storage->highPtPt[index] * (nomFactor > 0.f ? downFactor / nomFactor : 1.f);
    }
```

같은 파일 익명 namespace(Task 3에서 만든 것)에 결정론적 시드 헬퍼를 추가:

```cpp
// 같은 muon 에 대해 nominal 과 variation 이 같은 난수를 뽑도록 하는 시드.
// 이벤트 번호와 방향만으로 정해지므로 재현 가능하다.
inline unsigned int MuonSmearSeed(const unsigned long long eventNumber,
                                  const float eta, const float phi) {
  const unsigned int etaBits =
      static_cast<unsigned int>(std::fabs(eta) * 1000.f);
  const unsigned int phiBits =
      static_cast<unsigned int>(std::fabs(phi) * 1000.f);
  return static_cast<unsigned int>(eventNumber & 0xFFFFFFFFull) ^
         (etaBits << 8) ^ phiBits;
}
```

**주의:** `event.get()`은 `SKNanoLoader`의 `BranchScalar<ULong64_t> event` 멤버다. `PopulateMuonMomentum` 안에서 접근 가능한지 확인하고, 아니면 `CurrentEntry()`로 대체한다.

- [ ] **Step 4: 계층 규칙 확인**

Run: `bash scripts/check_layers.sh`

Expected: 종료 코드 0.

- [ ] **Step 5: 빌드 요청 (사용자 수행)**

> Task 5 코드 작성이 끝났습니다. 뮤온 서브시스템의 마지막 태스크입니다. 빌드 부탁드립니다.

- [ ] **Step 6: 커밋**

```bash
git add AnalyzerTools/include/MyCorrection.h \
        AnalyzerTools/src/MyCorrectionLeptons.cc \
        DataFormats/include/MuonView.h \
        AnalyzerFramework/src/AnalyzerCoreObjects.cc
git commit -m "$(cat <<'EOF'
Smear high-pT muon momentum resolution in simulation

Simulation resolves high-pT muons better than data in several eta regions,
so the MUO POG asks for extra smearing there. The width is a cubic in the
full momentum p, not pT, with coefficients per era and eta region, and the
uncertainty is a flat ten percent added in quadrature.

Applies it to simulated muons in the high-pT regime only; below the boundary
the Rochester correction already covers resolution and its error is carried
by the scale nuisance, so smearing there would double count. Nominal and
varied factors share a seed, which keeps the variation a coherent shift.

Where no extra smearing is needed the down variation floors at the nominal,
leaving the nuisance one sided, since convolution can only widen resolution.
2024 has no table: the POG has that measurement in progress, so it stays a
no-op until the numbers land.

Co-Authored-By: Claude Fable 5 <noreply@anthropic.com>
EOF
)"
```

---

## Self-Review

**1. Spec coverage (설계 §4):**

| 설계 항목 | 태스크 |
|---|---|
| §4.1 HighPt SF 배선 (4개 키 + 트리거 소스 교정) | Task 1 |
| §4.1 `GetMuonHighPtRECOSF` 복원 | ⚠️ **누락 — 아래 참조** |
| §4.2 SF 평가 pt 교정 | Task 2 |
| §4.3 TuneP 접근자·regime latch | Task 3 |
| §4.3 GE scale·κ 맵·opt-in 셀렉터 | Task 4 |
| §4.4 해상도 smearing·MuonRes | Task 5 |

**보완:** 설계 §4.1은 v13 `GetMuonHighPtRECOSF`(키 `NUM_GlobalMuons_DEN_TrackerMuonProbes`, p를 `[50.1, 1499]`로 clamp)를 별도 메서드로 복원한다고 적었다. Task 1의 라우팅만으로는 기존 `GetMuonRECOSF`가 Run3에서 `return 1.f`로 빠져나가므로 이 키가 쓰이지 않는다. **Task 1 Step 5 다음에 아래를 추가한다.**

`AnalyzerTools/include/MyCorrection.h`의 `GetMuonRECOSF` 선언 다음:

```cpp
    // 고pT muon 의 tracker -> global reco 효율 SF.
    // 일반 GetMuonRECOSF 와 달리 momentum p 의 함수다.
    float GetMuonHighPtRECOSF(const MuonView &muon,
                              const variation syst = variation::nom) const;
```

`AnalyzerTools/src/MyCorrectionLeptons.cc`:

```cpp
float MyCorrection::GetMuonHighPtRECOSF(const MuonView &muon,
                                        const variation syst) const {
  if (!cset_muon_highpt)
    return 1.f;
  auto cset = cset_muon_highpt->at("NUM_GlobalMuons_DEN_TrackerMuonProbes");
  // 이 SF 는 pT 가 아니라 momentum p 의 함수이고, 측정 범위를 벗어나면
  // correctionlib 이 throw 하므로 유효 구간으로 clamp 한다.
  const float momentum = muon.Pt() * std::cosh(muon.Eta());
  const float clamped = std::min(std::max(momentum, 50.1f), 1499.f);
  return safeEvaluate(cset, "GetMuonHighPtRECOSF",
                      {std::fabs(muon.Eta()), clamped,
                       getSystString_MUO(syst)});
}
```

Task 1 Step 9의 `git add`에 변경 파일이 이미 포함되어 있으므로 커밋 명령은 그대로 쓴다.

**2. Placeholder scan:** "TBD"/"적절히 처리"류 없음. 모든 코드 스텝에 실제 코드가 있다. Task 3 Step 5는 의도적으로 Task 4로 이관하도록 명시했다.

**3. Type consistency:**
- `MuonView::TunePPt()`/`IsHighPtRegime()` (Task 3) → Task 4·5에서 동일 이름 사용 ✓
- `MuonSoA::tunePPt`/`highPtRegime` (Task 3) → Task 4·5에서 동일 ✓
- `MyCorrection::GetMuonGEScaledPt` 시그니처 (Task 4 선언) = 구현 = 호출부 ✓
- `MyCorrection::GetMuonHighPtSmearFactor` 시그니처 (Task 5 선언) = 구현 = 호출부 ✓
- `GECell`/`GEMap` (Task 4), `MuonResPoly`/`MuonResSet` (Task 5) — 같은 익명 namespace, 이름 충돌 없음 ✓
- `MuonSmearSeed` (Task 5)는 Task 3에서 만든 익명 namespace에 추가 ✓

**4. 구현 중 확인이 필요한 항목** (코드 작성 시 실제 파일을 보고 맞춘다):
- `MuonSoA`의 lane 선언 위치와 기존 명명 규칙
- `PopulateMuonMomentum` 안에서 `event` 브랜치 접근 가능 여부 (불가하면 `CurrentEntry()`)
- `MyCorrection.h`의 `TRandom3.h` include 여부
- `MyCorrectionLeptons.cc`의 익명 namespace 존재 여부

## 남은 서브시스템

이 계획은 설계 §4(뮤온)만 다룬다. 나머지는 각각 별도 계획으로 작성한다.

- **전자** (설계 §5): nominal scale/smear 신설 + `GetElectronScaleUnc` 스키마 교정 + GAP 처리
- **AK8** (설계 §6): JERC lane + SoftDrop mass 변형
