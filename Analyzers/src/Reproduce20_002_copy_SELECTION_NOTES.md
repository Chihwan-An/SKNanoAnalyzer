# Reproduce20_002_copy 셀렉션 레퍼런스

`Reproduce20_002_copy.cc` (WR→Nℓ→ℓℓjj 분석기)의 이벤트 셀렉션을 NanoAOD 브랜치 수준으로 정리한 문서.
Python(uproot/awkward) 등 외부 도구로 셀렉션을 재현할 때 참조.
(2026-07-11 작성, Claude Code 조사 기반. 파이썬 이식본: `/data9/Users/achihwan/AN/For_claude/11_LSF_test/selection_test.ipynb`)

## 오브젝트 정의

### 뮤온 (`GetAllMuons`, AnalyzerCore.cc:566-643)
- **pt = `Muon_pt * Muon_tunepRelPt` (TuneP)** — 프레임워크가 4-벡터를 TuneP pt로 빌드함. Rochester 블록은 주석 처리됨.
  `Muon::OriginalPt()`도 TuneP 이후에 저장되므로 raw `Muon_pt`가 아님.
- 프리컷 (`SelectMuons(...,"NOCUT",53,2.4)`): `pt>53 && |eta|<2.4` (physics eta)
- **Tight** (cc:1103): `Muon_highPtId==2` (global high-pT) `&& Muon_tkRelIso<0.1`
- **Loose** (cc:1108): `Muon_highPtId==2` (iso 없음 — tight와 iso만 다름)
- `Muon_tkRelIso`는 NanoAOD 정의상 이미 tunePpt 기준 상대 iso — 그대로 0.1과 비교

### 전자 (`GetAllElectrons`, AnalyzerCore.cc:723-799)
- pt = raw `Electron_pt` (nominal에서는 scale/smear 미적용)
- **GAP 베토**: `1.444 < |scEta| < 1.566` 전자는 GetAllElectrons에서 제거.
  주의: SKNano 파일에 `Electron_scEta` 브랜치가 있으나 표준 NanoAOD엔 없음 → `scEta = Electron_eta + Electron_deltaEtaSC`
- 프리컷 (`SelectElectrons(...,"NOCUT",53,2.5)`): `pt>53 && |eta|<2.5` (physics eta, scEta 아님)
- **Tight** (cc:1066): `Electron_cutBased_HEEP` (POG_HEEP)
- **Loose** (cc:1079-1082): `isPassLooseNoIso || HEEP`
  - `isPassLooseNoIso` (cc:3514-3547): `Electron_vidNestedWPBitmap`의 10개 컷(3비트씩) 중
    **7번(GsfEleRelPFIsoScaledCut, iso) 제외** 전부 `((bitmap>>(3*cut))&7) >= 2` (Loose WP)
- `isPassCustomTightID`/`isPassCustomLooseID`(cc:3470-3512)는 현재 셀렉션에서 **미사용** (히스토그램 체크용만)

### 제트 (`GetAllJets`, AnalyzerCore.cc:998-1109)
- **C++은 JEC 재적용**: `pt = Jet_pt*(1-Jet_rawFactor)*JESSF(correctionlib)`, MC는 추가로 `SmearJets`(JER) 적용
- 셀렉션 (cc:1186): `pt>=40 && |eta|<=2.5 && TIGHTLEPVETO`
- Run3 TIGHTLEPVETO (Jet.cc:109-141):
  - tight: `|eta|<=2.7`: `Jet_jetId&2`; `2.7<|eta|<=3.0`: `&2 && neHEF<0.99`; `|eta|>3.0`: `&2 && neEmEF<0.4`
  - lepveto: `|eta|<=2.7`에서 추가로 `muEF<0.8 && chEmEF<0.8`
- **클리닝** (cc:1129, cc:3568-3585): loose 렙톤과 `dR<0.4`인 제트 제거 (하드코딩 0.4)

### 팻젯 (`GetAllFatJets`, AnalyzerCore.cc:1275-)
- pt = raw `FatJet_pt` (**무보정** — JEC 블록 주석 처리됨)
- **클리닝** (cc:1128, cc:3549-3566): tight 렙톤과 `dR<0.4`인 팻젯 제거
- 셀렉션 (cc:1142-1148): `pt>200 && |eta|<2.5 && FatJet_msoftdrop>40 && (FatJet_jetId&2)`
- **LSF 서브셋**: `FatJet_lsf3 > 0.75` → `fatjets_LSF` (부스티드 SR/Flav CR에서 사용; DY CR은 LSF 컷 없는 팻젯 사용)

## 이벤트 레벨

### 노이즈 필터 (`PassNoiseFilter`, AnalyzerCore.cc:43-68, Run3)
`Flag_goodVertices && Flag_globalSuperTightHalo2016Filter && Flag_EcalDeadCellTriggerPrimitiveFilter && Flag_BadPFMuonFilter && Flag_BadPFMuonDzFilter && Flag_hfNoisyHitsFilter && Flag_eeBadScFilter`
(+ DATA RunNumber 362433-367144에서만 ECAL-hole 제트 체크)

### 제트 베토맵 (`PassVetoMap`, AnalyzerCore.cc:1244-1258, Run3)
- 후보: (loose렙톤 클리닝된) 제트 중 `pt>15, |eta|<5, tight ID, 모든 NanoAOD 뮤온과 dR>0.2, chEmEF+neEmEF<0.9`
- correctionlib `jetvetomaps.json.gz` (`external/jsonpog-integration/POG/JME/2022_Summer22EE/...`),
  키: 2022=`Summer22_23Sep2023_RunCD_V1`, 2022EE=`Summer22EE_23Sep2023_RunEFG_V1`, 2023=`Summer23Prompt23_RunC_V1`, 2023BPix=`Summer23BPixPrompt23_RunD_V1`
- `evaluate("jetvetomap", eta, phi) > 0`인 제트가 하나라도 있으면 이벤트 폐기

### 트리거 (2022/2022EE/2023/2023BPix)
- 뮤온: `HLT_Mu50 || HLT_CascadeMu100 || HLT_HighPtTkMu100`, 세이프 pt 52
- 전자: `HLT_Ele30_WPTight_Gsf || HLT_Photon200 || HLT_Ele115_CaloIdVT_GsfTrkIdT`, 세이프 pt 35
- 2017: mu=`Mu50|OldMu100|TkMu100`(52), el=`Ele35_WPTight_Gsf|Photon200|Ele115_...`(38)

## Resolved 셀렉션 (cc:1207-1809)

1. `n_tight==2 && l1.pt>60 && l2.pt>53` (tight 렙톤 pt순 정렬, 플레이버 통합)
2. 카테고리: EE(tight el 2 / mu 0), MM(mu 2 / el 0), EM(각 1)
3. **트리거-세이프 pt: 실패시 `return` → 이벤트 완전 폐기 (부스티드로도 안 감!)**
   - EE: 리딩 tight el pt < 35 → 폐기 / MM·EM: 리딩 tight mu pt < 52 → 폐기
4. 트리거: EE→전자 OR, MM·EM→뮤온 OR
5. `njet>=2` && dR: `j1-l1, j1-l2, j2-l1, j2-l2, l1-l2, j1-j2` 전부 `>0.4` → **IsResolvedEvent**
6. `WRCand = l1+l2+j1+j2`, `mll = (l1+l2).M()`:
   - **DY CR**: `60<=mll<150 && m(lljj)>800` (EE/MM, SS/OS)
   - **Flav CR**: `mll>400 && m(lljj)>800` (EM, SS/OS)
   - **SR**: `mll>400 && m(lljj)>800` (EE/MM, SS/OS; C++은 `!IsDATA`에서만 = 블라인딩)

## Boosted 셀렉션 (cc:1811-3400, `!IsResolvedEvent`만)

1. `n_tight>0 && lead_tight.pt>60`
2. 리딩 플레이버별 트리거-세이프 pt (el<35 / mu<52 → `return`) + 해당 트리거
3. `Loose_SF` = 리딩과 같은 플레이버 loose 컬렉션(pt순), `Loose_OF` = 반대
4. **low-mll 탐색**: pt순 첫 SF loose(≠리딩, 포인터 비교)와 `60<mll<150` → has_lowmll (첫 매치에서 break)
5. **has_lowmll → Boosted DY CR**:
   - `fatjets`(LSF 컷 없음) 중 pt순 첫 `|Δφ(lead,fj)|>2.0` (break)
   - `Ncand = fj` (loose가 fj 안, dR<0.8) 또는 `fj + loose` (밖)
   - `m(lead+Ncand)>800` → EE/MM, SS/OS(리딩×loose charge)
6. **no low-mll**: `fatjets_LSF` 중 pt순 첫 `|Δφ|>2.0` (break) = HNFatJet
   - SFLooseLepton = pt순 첫 SF loose (`dR(lead)>=0.01, pt>=53`) 중 `dR(fj)<0.8` (break)
   - OFLooseLepton = 동일 로직 OF loose
   - **extra tight lepton veto**: 리딩/SF/OF 제외 tight 0개 (SR·Flav 공통 전제, 중괄호 중첩 확인됨)
   - **Boosted SR**: SF 있음 && OF 없음 && `m(lead,SF)>200` && `m(lead+fj)>800` (EE/MM, SS/OS)
   - **Boosted Flav CR**: SF 없음 && OF 있음 && `m(lead,OF)>200` && `m(lead+fj)>800`
     (리딩 el = EMJ / 리딩 mu = MEJ, SS/OS)
   - 부스티드 SR/Flav의 WRCand는 **lead + HNFatJet** (loose 렙톤 미포함)

## 기타

- `TightCharge`: `Muon_tightCharge`/`Electron_tightCharge` (2=pass) — 영역 서브분류(`_tight`/`_2e_tight` 등)에 사용
- MET: `PuppiMET_pt/phi` (셀렉션에는 미사용, 노이즈필터 데이터 체크만)
- `SetSignalFlags` (cc:3404-3458): gen `|PID| ∈ {9900012, 9900014, 34}` 시그널 태깅, LHE lljj 질량으로 on/off-shell 분류
- 시그널 SF 관련: HEEP ID SF·전자 트리거 SF는 egamma-tnp T&P 테이블이 cc 상단에 하드코딩 (2022~2023BPix)
- 입력 샘플: `data/Run3_v12_Run2_v9/<era>/Sample/ForSNU/<alias>.json`의 `"path"` 배열
  (예: 2022EE DYJets = `/gv0/DATA/SKNano/Run3NanoAODv12/2022EE/MC/DYto2L-2Jets_MLL-50_.../tree_*.root`,
  WR 시그널 = `/gv0/Users/achihwan/SKNano/Run3NanoAODv12/2022EE/WRtoN.../*.root`)

## 파이썬 이식시 주의점 (검증 완료)

1. 뮤온 pt는 반드시 TuneP (`Muon_pt*Muon_tunepRelPt`), 정렬도 TuneP pt 기준
2. `Electron_scEta` 없으면 `eta + deltaEtaSC`
3. 트리거-세이프 pt 컷의 `return` 시맨틱 재현 필수 (resolved에서 폐기된 이벤트는 boosted 진입 불가)
4. C++ 루프의 `break` = "pt순 첫 번째" (low-mll 렙톤, Δφ 팻젯, in-fatjet loose 렙톤 모두)
5. 제트만 JEC 재적용+JER 차이 존재 (NanoAOD pt 그대로 쓰면 수 % 차이)
6. 검증 결과: WR2000N1100EE → Resolved SR EE 지배적, SS/OS ≈ 50:50 (Majorana);
   DYJets → DY CR OS 지배적, SR 거의 0
