# 2026-07-07 작업 정리: Reproduce20_002_copy → HNWR 포팅 + 2024 era 코렉션/시스터매틱 점검

## 목차

1. [작업 배경](#1-작업-배경)
2. [포팅 작업 내용](#2-포팅-작업-내용)
3. [저장소 간 API 차이 및 수정 사항](#3-저장소-간-api-차이-및-수정-사항)
4. [발견 및 복구한 회귀: 2024 era 트리거 설정 누락](#4-발견-및-복구한-회귀-2024-era-트리거-설정-누락)
5. [2024 era 코렉션/시스터매틱 정적 점검 결과](#5-2024-era-코렉션시스터매틱-정적-점검-결과)
6. [발견된 이슈 (포팅과 무관, 소스에 이미 있던 한계)](#6-발견된-이슈-포팅과-무관-소스에-이미-있던-한계)
7. [빌드 검증 관련](#7-빌드-검증-관련)
8. [백업 파일 위치](#8-백업-파일-위치)
9. [2024 electron ID / Trigger SF 실측값 반영](#9-2024-electron-id--trigger-sf-실측값-반영)
10. [소스 재동기화: egamma-tnp 기반 electron trigger SF (2022~2023BPix)](#10-소스-재동기화-egamma-tnp-기반-electron-trigger-sf-2022~2023bpix)

---

## 1. 작업 배경

`/data6/Users/achihwan/SKNanoAnalyzer-v13/Analyzers/src/Reproduce20_002_copy.cc` (3668줄, 참고용 별도 저장소)에
새로 추가된 로직을 현재 저장소의 `Analyzers/src/HNWR.cc` (기존 3257줄)에 코드 구조와 저장 히스토그램이
완전히 동일하도록 그대로 이식하는 작업. 이후 사용자가 era **2024** 데이터로 실제 실행할 예정이라
코렉션(Correction)과 시스터매틱이 제대로 적용되는지 함께 점검.

## 2. 포팅 작업 내용

- `Analyzers/include/HNWR.h`, `Analyzers/src/HNWR.cc`를 `Reproduce20_002_copy` 클래스명을 `HNWR`로
  치환한 버전으로 전체 교체.
- v13 소스 대비 새로 추가된 주요 내용:
  - `LHECollection`(`lhe_set`) 멤버 및 `LHE.h` include 추가 (현재 저장소에도 `LHE.h`/`GetAllLHEs()` 존재 확인 후 반영)
  - 시그널 분류 플래그 `sig_isSignal / sig_isOffshell / sig_isOnshell / sig_isTb`
  - `SetSignalFlags()`, `FillSignalCutflow()` 메서드
  - DY CR / Flavor CR 카테고리 대폭 확장 (`is_Resolved_DY_EE_SS/OS`, `is_Resolved_Flav_EM_*` 등)
  - 이론(theory) 웨이트 시스터매틱: `ScaleWeight_muF`, `ScaleWeight_muR`, `PDF_Weight`, `AlphaS_Weight`
    (`weight_function_map`에 신규 target으로 추가, `GetScaleVariation()` / `LHEPdfWeight[]` 기반 계산)
- 두 파일 모두 git에 커밋된 적 없는 로컬 untracked 파일이었음 (`git status`로 확인) → 히스토리 손실 없음.

## 3. 저장소 간 API 차이 및 수정 사항

두 저장소(`SKNanoAnalyzer` vs `SKNanoAnalyzer-v13`)의 `AnalyzerCore`/`Event` API를 비교해
포팅한 코드가 현재 저장소에서 그대로 컴파일 가능한 형태인지 확인.

| 항목 | v13 (소스) | 현재 저장소 (타겟) | 조치 |
|---|---|---|---|
| `Event::GetMETVector()` | `(MET_Type, MET_Syst)` 2-인자 | `(MET_Type, MyCorrection::variation, MET_Syst)` 3-인자 | 호출부에 `MyCorrection::variation::nom` 추가 |
| `GetAllLHEs`, `GetScaleVariation`, `myCorr->Get*SF`, `systHelper->*`, `SelectElectrons/Muons/Jets`, `ScaleJets/SmearJets`, `PassNoiseFilter` 등 | 동일 | 동일 | 시그니처 일치 확인, 수정 불필요 |

그 외 파일 전체에서 사용된 모든 bare 함수 호출(`GetAll*`, `Clean_*`, `FillHist`, `SelectElectrons` 등)을
현재 저장소의 `AnalyzerCore.h` / `DataFormats` 헤더와 대조하여 시그니처 불일치가 더 없음을 확인.

## 4. 발견 및 복구한 회귀: 2024 era 트리거 설정 누락

v13 소스 파일(`Reproduce20_002_copy.cc`)의 `initializeAnalyzer()`에는 `DataEra == "2024"` 분기가
아예 없었음 (2017 / 2022 / 2022EE / 2023 / 2023BPix만 존재). 반면 기존 `HNWR.cc`에는 2024 트리거 설정이
있었음. 그대로 덮어썼다면 **2024 era 실행 시 트리거 리스트가 비어 이벤트가 전부 걸러지는 문제**가
발생했을 것 → 기존 블록을 그대로 복원함:

```cpp
if (DataEra == "2024")
{
    mu_set.Muon_Trigger = {"HLT_Mu50", "HLT_CascadeMu100", "HLT_HighPtTkMu100"};
    mu_set.Muon_Trigger_Safe_Pt_Cut = 52.;
    el_set.Ele_Trigger = {"HLT_Photon200","HLT_Ele115_CaloIdVT_GsfTrkIdT"};
    el_set.Ele_Trigger_Safe_Pt_Cut = 118.;
}
```

## 5. 2024 era 코렉션/시스터매틱 정적 점검 결과

빌드 없이(사용자 지시에 따라 빌드는 직접 하지 않음) 코드/설정 파일을 대조하여 확인한 내용.

- `AnalyzerTools/src/MyCorrection.cc`에 era `"2024"` 매핑(`LUM_keys`, `EGM_keys`, `JME_JER_GT`,
  `JME_JES_GT`, `JME_vetomap_keys`, GetEraConfig의 `Run3-24CDEReprocessingFGHIPrompt-Summer24-NanoAODv15`
  태그)이 이미 프레임워크 차원에 구현되어 있음을 확인.
- `CMS_corrections/{LUM,EGM,MUO,JME,BTV}/Run3-24CDEReprocessingFGHIPrompt-Summer24-NanoAODv15/latest/`
  경로에 실제 correction 파일이 존재함을 확인.
- `docs/MCLRSM.yaml` / `docs/DataLRSM.yaml`의 systematic `target` 이름이 포팅된 코드의
  `weight_function_map` 키와 전부 일치함을 확인: `PU_Weight`, `E_Id_Weight`, `E_Reco_Weight`,
  `E_Trig_Weight`, `M_Id_Weight`, `M_Reco_Weight`, `M_Trig_Weight`, `M_Iso_Weight`, `JER_Variation`,
  `JES_Variation`, `ScaleWeight_muF`, `ScaleWeight_muR`, `PDF_Weight`, `AlphaS_Weight`.
- 코드에서 쓰는 muon correctionlib 키 이름(`NUM_HighPtID_DEN_GlobalMuonProbes`,
  `NUM_HLT_DEN_HighPtLooseRelIsoProbes`, `NUM_probe_LooseRelTkIso_DEN_HighPtProbes`)이 실제
  2024 `muon_HighPt.json.gz` 안에 존재하는 correction 이름인지 `zcat`으로 직접 확인함 (모두 존재).
- 헤더의 `Muon_IS_SF_Key = "HighPtMuonIDSF"`, `Muon_Trigger_SF_Key = "POGHighPtLooseTrkIso"`는
  실제로는 코드 어디에서도 참조되지 않는 죽은 설정값 (실제 SF 호출은 위의 `NUM_*` 문자열을 직접 하드코딩해서 사용).

## 6. 발견된 이슈 (포팅과 무관, 소스에 이미 있던 한계)

- **Electron ID SF가 correctionlib을 타지 않고 하드코딩됨**: `HNWR.cc`의 934, 988, 1675번째 줄 등
  4곳에서 electron ID SF를 `el_set.Barrel_ID_SF_2023_C/D`, `Endcap_ID_SF_2023_C/D` 상수값으로 계산하며
  era 분기가 전혀 없음. **2024 era로 실행하면 electron ID SF에 2023년 값(1.007/0.988 등)이 그대로
  적용됨.** v13 소스(`Reproduce20_002_copy.cc`)에도 동일하게 존재하는 한계라 포팅 과정에서 생긴 문제는
  아니지만, 2024 물리 결과에 영향을 줄 수 있으므로 사용자 확인 필요 (2024 전용 SF 값 추가 여부 등 방향 미정).
- **Muon trigger SF도 2024 전용 측정치 없음**: `MyCorrection.cc`에 `json_muon_trig_sf = ... ; // temporary
  due to no mu trig sf for 2024`라는 기존 주석 존재. analyzer 코드 문제가 아니라 프레임워크 차원의
  기존 한계.

## 7. 빌드 검증 관련

- 처음에는 실제 컴파일까지 시도했으나 (host에는 micromamba 없음 / singularity 컨테이너엔 ROOT·correctionlib은
  있으나 `make`/`ninja` 없음 등 환경 제약 확인 중), **사용자 지시로 빌드는 직접 하지 않기로 함**.
  이후 검증은 전부 코드/설정 파일 정적 대조로만 수행.
- 따라서 실제 컴파일 성공 여부는 사용자가 `./scripts/rebuild.sh` 등으로 직접 확인 필요.

## 8. 백업 파일 위치

포팅 전 원본은 다음 경로에 백업되어 있음 (세션 임시 디렉터리이므로 필요 시 복사해둘 것):

- `/tmp/HNWR.cc.orig_backup`
- `/tmp/HNWR.h.new_backup`

## 9. 2024 electron ID / Trigger SF 실측값 반영

6번 섹션에서 지적한 "electron ID SF가 2023년 하드코딩 값을 그대로 쓴다"는 문제에 대해, 사용자가
`tnpsf/WR_efficiencies_SF_errors.xlsx`(출처: `Alpana_WR_LRSM_update_22June2026.pdf`, UMN subgroup meeting,
2026-06-22, 109 fb⁻¹ 2024 EGamma T&P)에 2024 HEEP ID SF와 electron trigger SF 실측값을 정리해두어 이를
코드에 반영함.

### 엑셀 데이터 구조
- 시트 `HEEP ID (p6)`, `Trigger HLT (p2)` 각각에 Data Eff / MC Eff / SF / 상대오차[%] 2D 테이블 존재.
- eta는 **부호 있는(signed) 4구간**으로 분리되어 있음: `[-2.5,-1.57]`, `[-1.44,0]`, `[0,1.44]`, `[1.57,2.5]`
  (ECAL crack `|eta| in [1.44,1.57]`는 측정 없음, 기존 코드와 동일하게 SF=1.0 처리).
- pT는 HEEP ID 11구간(35~40 ~ >300 GeV), Trigger 11구간(30~40 ~ >300 GeV)으로 기존 코드보다 훨씬 세분화됨
  (기존 코드는 barrel/endcap 단일값 혹은 pT>130 GeV 단일 컷).
- 오차는 "relative error [%]"로 라벨링되어 있어 `절대오차 = SF × (상대오차%/100)`로 변환.

### 코드 반영 내용
- `Analyzers/include/HNWR.h`의 `struct Electrons`에 `GetHEEP2024SF(pt, eta, var)`,
  `GetTrigger2024SF(pt, eta, var)` 메서드 선언 추가.
- `Analyzers/src/HNWR.cc`에 `isPassLooseNoIso` 구현 직후, 익명 네임스페이스로 2024 SF/오차 2D lookup
  테이블(`kHEEPSF`, `kHEEPSFRelErrPct`, `kTrigSF`, `kTrigSFRelErrPct`)과 pT/eta bin 매칭 헬퍼
  (`GetEEEtaColumn2024`, `GetPtBin2024`, `ApplyVariation2024`)를 추가하고, 두 멤버 함수를 구현.
- 기존 코드에서 electron ID SF를 계산하는 `weight_function_map["E_Id_Weight"]` 람다 3곳(모두
  2023 하드코딩 barrel/endcap 상수 사용)과, electron trigger SF를 계산하는
  `weight_function_map["E_Trig_Weight"]` 람다 4곳(2022/2022EE/2023/2023BPix만 있고 2024 없음)
  **각각의 맨 앞에 `if (DataEra == "2024") { return el_set.GetHEEP2024SF(...)` /
  `return el_set.GetTrigger2024SF(...); }` 얼리 리턴 분기를 추가**. 다른 era(2017/2022/2022EE/2023/2023BPix)의
  기존 로직은 전혀 건드리지 않고 그대로 유지.
- `Muon_IS_SF_Key`/`Muon_Trigger_SF_Key` 등 죽은 설정값은 이번 작업과 무관하여 손대지 않음.

### 참고
- 값 자체(효율/SF/오차)는 전부 엑셀에서 그대로 가져온 것이며, pT bin 하한/eta 구간 분류만 코드로 옮기며
  판단(예: 최저 pT 미만은 최저 bin으로 clamp, crack/acceptance 밖은 SF=1.0)을 내림.
- 빌드는 이번에도 직접 하지 않았으므로 (`./scripts/rebuild.sh`로) 사용자가 직접 컴파일 확인 필요.
