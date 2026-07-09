# 2026-07-07 작업 정리: Reproduce20_002_copy.cc electron trigger SF(tnp) 적용 + trigger 설정 변경

대상 저장소: `/data6/Users/achihwan/SKNanoAnalyzer-v13`
대상 파일: `Analyzers/src/Reproduce20_002_copy.cc`, `Analyzers/include/Reproduce20_002_copy.h`

## 목차

1. [작업 배경](#1-작업-배경)
2. [egamma-tnp 조사 결과](#2-egamma-tnp-조사-결과)
3. [사용자 확인 사항](#3-사용자-확인-사항)
4. [코드 변경 내용](#4-코드-변경-내용)
5. [처리하지 않은 부분](#5-처리하지-않은-부분)
6. [알려진 이슈 / 주의사항](#6-알려진-이슈--주의사항)
7. [빌드 검증](#7-빌드-검증)

---

## 1. 작업 배경

`Reproduce20_002_copy.cc`에 하드코딩돼 있던 electron ID scale factor / trigger scale factor를,
실제 tag-and-probe 측정 결과가 있는 `/data6/Users/achihwan/tnp/egamma-tnp`에서 찾아 반영해달라는 요청.
추가로:
- electron trigger 관련 safe pt cut을 52로 변경 (muon과 동일하게)
- electron min pt를 muon min pt와 동일하게 맞추기

## 2. egamma-tnp 조사 결과

`/data6/Users/achihwan/tnp/egamma-tnp`에서 Run3 4개 era(2022, 2022EE, 2023, 2023BPix)에 대해
`RUN3_ANALYSIS_LOG.md` 기준으로 Tag&Probe 파이프라인이 실행된 상태였음.

- **Trigger SF (실측 완료)**: `fitter_plots_<era>/fitter_results_<era>.json`
  - 측정 조건: `final_runner.json` / `runner.json`의 `Target_Trigger_OR` 필터
    = `HLT_Ele30_WPTight_Gsf | HLT_Photon200 | HLT_Ele115_CaloIdVT_GsfTrkIdT`
    (denominator는 `settings.json`의 `cutbased_id: cutBased_HEEP` 조건, 즉 HEEP ID를 만족하는
    probe 중에서 이 트리거들 중 하나라도 pass할 확률)
  - Binning: `binning.json` 기준 `el_eta_bins`(6구간) × `el_pt_bins`(13구간, 10~400 GeV)
  - bin index 규칙(`v1_afteranalysis_run3.py`): `bin{pt_idx * n_eta + eta_idx}` (eta가 먼저 도는 순서)
- **Electron ID(HEEP) SF (미실측)**: `Heep_runner.json` / `Heep_settings.json` / `Heep_binning.json`
  설정 파일만 존재하고, 대응하는 `root_hists`/`fitter_plots` 결과물은 아직 생성되지 않음
  (`find`로 확인, "Heep" 관련 산출물 없음).

## 3. 사용자 확인 사항

작업 전 4가지를 질문해서 답변받음:

1. **Electron ID SF**: 실측 결과가 없으므로 기존 하드코딩 값(`Barrel_ID_SF_2023_C/D`,
   `Endcap_ID_SF_2023_C/D` 등) 그대로 두기로 함.
2. **Trigger 목록**: 코드에 이미 주석 처리돼 있던 조합
   `{HLT_Ele30_WPTight_Gsf, HLT_Photon200, HLT_Ele115_CaloIdVT_GsfTrkIdT}`이 egamma-tnp의
   `Target_Trigger_OR`과 정확히 일치 → 이 조합을 활성화하고 safe pt cut을 52로 설정하기로 함
   (2017은 tnp 측정 대상이 아니므로 손대지 않음).
3. **불량 bin 처리**: eta crack 영역(`|eta| 1.4442~1.566`) × pt `[300,400)` bin이 2022/2023/2023BPix
   3개 era에서 동일한 값(SF=0.3006, err=0.57)으로 나옴 → MC 통계 0건으로 fit이 실패해서 나온
   fallback 값으로 판단. 같은 eta column의 인접 pt bin(`[200,300)`) 값으로 대체하기로 함.
4. **구현 방식**: correctionlib JSON을 새로 만들지 않고, 기존 코드 스타일(하드코딩 상수)에 맞춰
   C++ 2D 배열로 값을 박아넣기로 함.

## 4. 코드 변경 내용

### 4.1 `GetElectronTriggerSF_TnP()` 신규 함수

- 선언: `Reproduce20_002_copy.h`에 `float GetElectronTriggerSF_TnP(double eta, double pt, MyCorrection::variation var) const;` 추가.
- 정의: `Reproduce20_002_copy.cc` 상단(`executeEvent()` 직전)에 익명 네임스페이스로
  era별 `std::array<std::array<float,6>,13>` (pt × eta) SF 테이블과 오차 테이블을 하드코딩.
  - 값 출처: `fitter_plots_<era>/fitter_results_<era>.json`의 `scale_factors` / `scale_factors_errors`.
  - pt bin `[10,20)`, `[20,30)`은 tnp에서 probe pt cut(30 GeV)로 인해 데이터가 없어 `[30,40)` 값을
    그대로 채워 넣음 (실제로는 `Ele_Trigger_Safe_Pt_Cut=52`로 이 구간은 실행 중 도달하지 않음).
  - eta crack × pt `[300,400)`의 불량 bin은 3.의 결정대로 `[200,300)` 값으로 대체.
- 함수 로직: `DataEra` 문자열로 era별 테이블 포인터 선택 (2017 등 측정 없는 era는 1.0 반환) →
  eta/pt 선형 탐색으로 bin index 결정(범위 밖 값은 첫/마지막 bin으로 clamp) → `variation`에 따라
  `sf ± err` 반환.
- `<array>` include 추가.

### 4.2 중복 블록 4곳 교체

기존에 `E_Trig_Weight` 람다가 아래 4곳에 완전히 동일한 내용으로 복붙돼 있었음
(pt>130 여부 + barrel/endcap 여부로 나눠 era별 상수를 if/else로 대입하는 방식):

- Resolved EE 채널 (dilepton)
- Boosted DY CR (단일 lepton, `is_tmp_lead_el`)
- Boosted SR 진입부 (단일 lepton)
- Boosted Flavor CR (단일 lepton)

네 곳 모두 아래 형태로 교체:
```cpp
weight_function_map["E_Trig_Weight"] = [&](MyCorrection::variation var, TString source) {
    return GetElectronTriggerSF_TnP(Tight_electrons[0]->Eta(), Tight_electrons[0]->Pt(), var);
};
```
`if (DataEra != "2017") { ... }` 감싸는 조건은 그대로 유지(동작은 동일하지만 기존 구조를 최대한 보존).

### 4.3 Trigger 목록 / safe pt cut 변경

`initializeAnalyzer()`의 2022 / 2022EE / 2023 / 2023BPix 블록에서:
- `el_set.Ele_Trigger`: `{HLT_Photon200, HLT_Ele115_CaloIdVT_GsfTrkIdT}` (+주석 처리된 대안)
  → `{HLT_Ele30_WPTight_Gsf, HLT_Photon200, HLT_Ele115_CaloIdVT_GsfTrkIdT}`로 활성화
- `el_set.Ele_Trigger_Safe_Pt_Cut`: 118 → 52
- 2017 블록은 변경하지 않음 (기존 `{HLT_Ele35_WPTight_Gsf, ...}`, cut=38 유지)

## 5. 처리하지 않은 부분

- **Electron ID SF**: egamma-tnp 실측값이 없어 `Barrel_ID_SF_2023_C/D`, `Endcap_ID_SF_2023_C/D` 등
  기존 하드코딩 값을 그대로 둠. `Heep_runner.json` 등으로 실제 fit을 돌리면 이 부분도 추후 교체 필요.
- **Electron min pt**: 헤더 확인 결과 `Electron_MinPt = 53.`이 이미 `Muon_MinPt = 53.`과 동일해서
  변경하지 않음 (요청은 "muon과 동일하게"였는데 이미 동일했음).

## 6. 알려진 이슈 / 주의사항

- 4곳의 `E_Trig_Weight` 블록 모두 (boosted 단일 lepton 케이스 포함) `Tight_electrons[0]`의
  eta/pt를 사용함. Boosted 케이스는 원래 코드도 `LeadLep`이 아니라 `Tight_electrons[0]`을 썼는데,
  `is_tmp_lead_el` 분기에서는 `Tight_electrons[0]`이 곧 `LeadLep`이라 동작상 문제는 없음(기존 동작 유지,
  새로 만든 게 아님).
- `GetElectronTriggerSF_TnP()`의 eta bin은 signed eta 기준(`-2.5~2.5`, crack이 `±1.4442~1.566`)이고
  기존 ID SF 코드처럼 `abs(eta)`를 넘기면 안 됨 — 반드시 부호 있는 `Eta()` 값을 넘겨야 함 (현재 4곳 모두 그렇게 호출).
- pt/eta가 테이블 범위를 벗어나면(예: pt>400) 가장 바깥쪽 bin 값으로 clamp됨(외삽 아님).

## 7. 빌드 검증

이번 세션에서는 **사용자가 직접 빌드하기로 함** (`./scripts/rebuild.sh` 등). 이 세션에서는
`source setup.sh`까지만 시도했고 (`micromamba` PATH 문제로 실패, 환경 이슈로 보이며 이번 코드
변경과는 무관), 실제 컴파일 검증은 진행하지 않음.
