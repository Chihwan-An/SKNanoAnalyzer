# 2026-07-07 작업 정리: HNWR.cc 빌드 에러 수정 + Muon TuneP 적용

## 1. HNWR.cc / HNWR.h 빌드 에러 원인

`Analyzers/src/HNWR.cc`, `Analyzers/include/HNWR.h`는 git에 커밋된 적 없는 새 파일(untracked)이었고,
예전(리팩터링 이전) DataFormats API를 기준으로 작성되어 있어 현재 `Muon`/`FatJet` API(lazy-loading /
view-based 구조, 커밋 `8c44c73`, `3f17018`)와 맞지 않아 빌드가 실패했음.

### 에러별 원인 및 조치

| # | 에러 | 원인 | 조치 |
|---|------|------|------|
| 1 | `FatJet::SDMass()` 없음 (15곳) | 실제 메서드명은 `GetSDM()` | rename |
| 2 | `FatJet::PassID(TString)` 없음 | FatJet에는 애초에 ID 판정 함수가 구현된 적 없음 (`enum FatJetID`만 존재) | `fatjet_set.FatJet_ID`를 `TString`→`FatJet::FatJetID`로 바꾸고, 기존에 있던 `AnalyzerCore::SelectFatJets()`(pt/eta/ID 선택, 내부에서 `myCorr->PassFatJetID()` 사용)로 교체. SDM/LSF3 컷은 그 결과에 대해 별도 루프로 유지 |
| 3 | `AnalyzerCore::PassVetoMap` 없음 | 실제 함수명은 `PassJetVetoMap` (`RVec<Jet>`, `RVec<Muon>` 오버로드 존재, `jets`/`mu_set.AllMuons` 타입과 일치) | rename |
| 4 | `Muon::TightCharge()` 없음 (10곳; Muon만 없어짐, `Electron::TightCharge()`는 아직 존재) | 리팩터링(`8c44c73`,`3f17018`) 과정에서 Muon에서만 제거되고 복구 안 됨 (예전 구현은 커밋 `2fe7069`에 있음) | **사용자 결정: DataFormats는 안 건드리고 HNWR.cc 쪽 호출부만 주석 처리.** `muon1/2_tight_charge` 변수 선언부(HNWR.cc:690 부근)에 왜/무엇을 주석 처리했는지, 복구 방법(Muon.h/.cc에 `j_tightCharge`+`TightCharge()`/`SetTightCharge()` 재추가, NanoAOD `Muon_tightCharge` 브랜치를 SoA/loader 경로에 연결)을 설명하는 주석을 남김. 각 호출부에도 짧은 참조 주석 추가. 결과적으로 `muon*_tight_charge`는 기본값(1)을 유지하므로 `==2` 비교는 항상 false |
| 5 | `Muon::OriginalPt()` 없음 (5곳) | 예전 코드의 `SetOriginalPt()`는 `j_miniAODPt`에 저장되던 것과 동일한 값 → 현재는 `MiniAODPt()` | rename |
| 6 | `RVec<Muon*> → RVec<Muon>` 변환 static_assert 실패 (3곳) | `MyCorrection::GetMuonTriggerSF()`가 `RVec<Muon>`(값 타입)을 요구하는데 `RVec<Muon*>`을 그대로 넘김 | `RVec<Muon*> trig_muons` → `RVec<Muon> trig_muons`로 바꾸고 `push_back(*Tight_muons[0])`처럼 역참조 |

빌드 자체는 매번 사용자가 직접 실행(`./scripts/rebuild.sh` 등)해서 확인.

## 2. Muon 모멘텀 보정: Rochester → TuneP 전환

사용자 확인 결과, 원래 이 저장소는 muon Pt 보정에 **Rochester correction(RoccoR)**을 쓰고 있었고
TuneP은 `DataFormats/include/Muon.h` 상단에 "Need update - TuneP object"로만 남아있던 미구현 TODO였음.

참고 저장소 `/data6/Users/achihwan/SKNanoAnalyzer-v13`의 `AnalyzerCore.cc::GetAllMuons()`를 보면
Rochester 계산 블록을 통째로 주석 처리하고, 대신 NanoAOD의 `Muon_tunepRelPt` 브랜치를 이용해
`muon_tunept = Muon_pt[i] * Muon_tunepRelPt[i]`로 Pt를 교체하는 방식이었음. 이 패턴을 현재(뷰/SoA 기반)
아키텍처에 맞춰 이식함.

### 변경 파일

- **`DataFormats/include/MuonView.h`**: `MuonSoA`에 `ColumnView<float> tunepRelPt;` 필드 추가
- **`Analyzers/src/AnalyzerCore.cc`** (`GetAllMuonViews()`):
  - `storage->tunepRelPt.bind(&Muon_tunepRelPt);` 추가
    (`Muon_tunepRelPt` 브랜치는 `Analyzers/include/generated_branch_decls.inc`에 이미 자동 생성되어 있어 바로 사용 가능했음)
  - 기존 Rochester 계산(`myCorr->GetMuonScaleSF(...)`, Gen 매칭 등)을 제거하고
    `correctedPt[i] = miniAODPt * tunepRelPt[i]`로 대체
  - `momentumScaleUp/Down`은 v13도 TuneP 경로에서 별도 variation을 계산하지 않으므로(주석 처리된 채 방치)
    동일하게 nominal 값(`correctedPt`)으로 채움 → **muon momentum scale up/down systematic은 현재 사실상 비활성화 상태**
- **`DataFormats/include/Muon.h`**: 상단 TODO 주석 정리, `Pt()`가 이제 TuneP 기준 보정값이라는 설명 추가

`AnalyzerTools/MyCorrection.h/.cc`(RoccoR 로딩 코드)는 v13에서도 그대로 남아있고 호출만 우회하는 방식이라
현재 저장소에서도 건드리지 않음.

### 영향 범위 / 주의

- `GetAllMuonViews()`/`GetAllMuons()`를 쓰는 **모든 analyzer**의 muon Pt가 이제 TuneP 기준으로 바뀜 (HNWR 한정 아님).
- muon momentum scale 계통 오차(`ScaleMuons()`가 사용하는 `MomentumScaleUp/Down`)가 지금은 nominal과 동일 →
  실질적으로 죽어있는 상태. 나중에 TuneP 자체의 불확실성을 반영하려면 별도 계산 로직이 필요함.
