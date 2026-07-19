# HNWR 분석기 상세 문서 — Reproduce20_002_copy & HNWR_BDT_presel

> W_R→ℓN→ℓℓjj (CMS AN-20-002 재현) 분석의 핵심 분석기 2개에 대한 구조/메커니즘 문서.
> 셀렉션 세부 수치의 1차 출처는 `Analyzers/src/Reproduce20_002_copy_SELECTION_NOTES.md`(한글, cc 라인 인용)이며,
> 이 문서는 전체 구조·출력·두 분석기의 차이·주의점을 다룬다.
> 자주 쓰는 명령어: [`../COMMANDS.md`](../COMMANDS.md) · 개선 권고: [`../RECOMMENDATIONS.md`](../RECOMMENDATIONS.md)

## 파일 지도

| 파일 | 줄수 | 역할 |
|---|---|---|
| `Analyzers/src/Reproduce20_002.cc` | 1472 | 조상 버전 (히스토그램만, 유지 안 함) |
| `Analyzers/src/Reproduce20_002_copy.cc` | 3629 | **메인 히스토그램 분석기** (SR/CR + 전체 systematic) |
| `Analyzers/src/HNWR_BDT_presel.cc` | 1472 | **BDT ntuple 생산기** (같은 셀렉션의 클린 재작성, `BDT/` 입력) |
| `Analyzers/include/AnalyzersLinkDef.hpp:26-28` | | 세 분석기 모두 등록 |

주의: `src/`에 백업 blob들이 커밋돼 있음 — `Reproduce20_002_copy.cc.bak`(1.14MB!),
`.cc.backup`(227KB), `_backup_20260226.txt`(130KB), `include/*.h.backup`. 소스가 아니므로 참조 금지.

관계: `Reproduce20_002.cc`(조상) → `Reproduce20_002_copy.cc`(T&P SF 테이블, 영역별 flat 변수,
2022~2023BPix era, 신호 cutflow 추가) → `HNWR_BDT_presel.cc`(동일 오브젝트 셀렉션을 람다 기반으로
재작성, 히스토그램 대신 카테고리별 BDT tree 출력). 옛 리포 `/data6/Users/achihwan/SKNanoAnalyzer/Analyzers/src/HNWR.cc`가
원조 구현이고 Reproduce20_002는 그 포팅임 (옛 리포의 `claude/2026-07-07_HNWR_reproduce20_port_and_2024_check.md` 참조).

## 공통 오브젝트 셀렉션

- **뮤온**: TuneP pt(`Muon_pt*tunepRelPt`), pt>53, |eta|<2.4.
  Tight = `POG_GLOBAL_HIGH_PT`(highPtId==2) && TkRelIso<0.1. Loose = 같은 ID, iso 없음.
- **전자**: raw pt, pt>53, |eta|<2.5, GAP(1.444<|scEta|<1.566) 제외.
  Tight = `POG_HEEP`. Loose = `isPassLooseNoIso || POG_HEEP`
  (`isPassLooseNoIso` = vidNestedWPBitmap Loose-WP에서 iso 비트(index 7)만 스킵; cc:3518).
  `isPassCustomTightID/LooseID`는 존재하지만 셀렉션에는 **미사용**(교차검증 히스토그램 전용).
- **젯**: JEC 재적용 + JER smearing(MC), pt≥40, |eta|≤2.5, TIGHTLEPVETO, loose 렙톤과 dR<0.4 클리닝.
  veto-map jet 있으면 이벤트 자체를 버림.
- **FatJet**: raw pt>200, |eta|<2.5, SDMass>40, TightID; boosted에서는 LSF3>0.75 부분집합 사용(_copy 한정).
- **렙톤 pt 문턱**: leading 60 / subleading 53.

## Reproduce20_002_copy 메커니즘

### 초기화 (cc:7-70)
- era별 트리거: 2017 → Mu50/OldMu100/TkMu100(safe 52) + Ele35/Photon200/Ele115(safe 38);
  Run3(2022~2023BPix) → Mu50/CascadeMu100/HighPtTkMu100(safe 52) + **Ele30**/Photon200/Ele115(safe 35).
- systHelper 설정: MC=`docs/MCLRSM.yaml`, DATA=`docs/DataLRSM.yaml`.
- **하드코딩 egamma-tnp T&P SF 테이블** (cc:72-324, float ~1900개): `ElTrigSF_<era>`(13pt×6eta),
  `ElHEEPIDSF_<era>`(11pt×6eta) + 오차 테이블, 2022/22EE/23/23BPix 전용. 2017은 1.0 반환.
  테이블 원본은 개인 tnp 작업 산출물(리포 외부) — 재현성 주의.

### 이벤트 흐름 (executeEventFromParameter, cc:396-3402)
1. PUPPI MET, MC weight = `MCweight() × GetTriggerLumi("HLT_Mu50")`.
2. Cutflow 채움 (`Cutflow_for_reseolved_SR`(오타 그대로 출력명에 박힘), `Cutflow_for_Boosted_SR`).
3. `this_syst`가 JER/JES_Up/Down이면 젯 컬렉션 자체를 다시 smear/scale (JES는 "total" 단일 소스).
4. NoiseFilter → 트리거 → 렙톤/젯/팻젯 빌드.
5. **Resolved** (cc:1211~): tight 렙톤 정확히 2, pt 60/53 → EE/MM/EM 채널 분기.
   trigger-safe-pt 실패 시 **hard return** (boosted 분기까지 죽음 — 의도된 동작이지만 리팩토링 시 최대 함정).
   njet≥2, 6가지 pairwise dR>0.4 → `IsResolvedEvent`. WRCand = l1+l2+j1+j2.
   - DY CR: 60≤mll<150 && m(lljj)>800 (EE/MM)
   - Flavor CR: mll>400 && m(lljj)>800 (EM)
   - SR: mll>400 && m(lljj)>800 (EE/MM), **`if(!IsDATA)` 로 블라인딩** (cc:1636)
6. **Boosted** (resolved 아님 && tight≥1 && lead pt>60): loose SF 렙톤쌍 60<mll<150 → Boosted DY CR;
   아니면 HNFatJet = |Δφ(lead,fj)|>2.0인 leading LSF 팻젯, 팻젯 안(dR<0.8) SF/OF loose 렙톤 탐색,
   추가 tight 렙톤 veto → Boosted SR (m(lead,SF)>200 && m(lead+fj)>800) / Boosted Flavor CR (EMJ/MEJ).

### 가중치/SF (systHelper weight map)
- PU / ElectronID(HEEP T&P) / ElectronReco / ElectronTrig(T&P, **leading 전자만**) /
  MuonID(HighPt) / MuonReco / MuonTrig / MuonIso — 2017은 전부 1.0.
- 이론: ScaleWeight_muF/muR, PDF(Hessian 1..100 quadrature), AlphaS(101/102).
- systematic 목록(MCLRSM.yaml): 위 weight 계열 + JER/JES(evtLoopAgain=true). DATA는 Central만.
- **없는 것**: b-tag SF, Rochester/전자 scale-smearing, L1 prefire, lumi/xsec nuisance, JES 소스 분리.

### 출력
히스토그램 전용 (FillHist 528회, tree 없음). 경로 규약: `<Syst>/<RegionPrefix>_<observable>`.
Region prefix 총 ~108개: `{DYCR,FlavCR,SR}_{Resolved,Boosted}_{EE,MM,EM,EMJ,MEJ}` ×
`{,OS,SS}` × tight 세부분류(`_0/1/2e_tight`, `_tight/_not_tight` 등).
영역당 관측량 ~35개 (`_mass,_mlljj,_leading_lep_pt,...`; boosted는 `_fatjetSDM,_fatjet_lsf3,_dphi_leadlep_fatjet` 등 추가).
신호 cutflow는 `_offshell/_onshell/_tb/_light` 사본 (on/off-shell 경계: WR2000→없음, WR4000→2000,
WR6000→4000, WR8000→5000; cc:3443-3446). 신호 PID: 9900012/9900014/34.

## HNWR_BDT_presel 메커니즘

목적: SR을 **느슨하게**(mll>200, mlljj 컷 없음) 열어 XGBoost가 판별을 학습하도록 카테고리별 ntuple 생산.
`BDT/SR_R_B` 등에서 사용.

### _copy와 의도적으로 다른 점 (중요 — 두 분석기는 수치가 일치하지 않음)
| 항목 | _copy | BDT_presel |
|---|---|---|
| 전자 트리거(Run3) | Ele30\|Photon200\|Ele115, safe-pt 35 | **Ele30 제외**, Photon200\|Ele115, safe-pt **118** |
| Resolved SR | mll>400 && mlljj>800 | **mll>200, mlljj 컷 없음** |
| jet↔fatjet 교차 클리닝 | 주석 처리된 죽은 코드 | **활성** (region-aware, cc:308-309) |
| Boosted 팻젯 | LSF3>0.75, SDMass>40 컷 | LSF/SDMass **컷 없음** (BDT 입력 변수로만 저장) |
| 전자 ID/Trig SF | 풀 T&P 테이블 | 인라인 근사 상수 (**2022/22EE ID SF는 사실상 1.0**, trig는 pt≤130→1) |

### 출력 트리
- 명명: `<Syst>/<Region>_<ch>_BDTTree_<resolved|boosted>`.
- **SR 트리만** JES/JER_Up/Down 디렉토리별 사본 존재; weight 계열 systematic은
  `weight_<SystName>` 브랜치로 저장 (JES/JER 트리에는 nominal weight만 — 다운스트림에서 특수 처리 필요).
- Central 전용 보조 트리: `CR_DY_{EE,MM}_*`, `CR_SS_{EE,MM}_resolved`(SS, 60<mll<300),
  `SR_LL_{EE,MM}_resolved`(fake-factor용 loose-loose 사이드밴드), `CR_FLV_{EM,ME}_*`,
  `CF_EE_Tree`(charge-flip 측정: tight 전자 2, 50<mll<130, AN-19-206 레시피, genCharge 매칭).
  주의: `CR_FLV_ME_resolved` 트리는 생성만 되고 **항상 비어 있음** (resolved FLV는 전자를 lead로 강제).
- 브랜치 (~224개/트리): 다중도(nJet, nBJetMedium, maxBTag...), 영역 플래그(category/region/isSameFlavor...),
  렙톤별 상세 ID 변수(전자 ~50개: vidNestedWPBitmap, HEEP 변수, genCharge...; 뮤온 ~35개),
  jet1..4(태거 스코어, 구성 성분 분율), fatjet1..2(SDMass, LSF3, ParticleNet 스코어),
  페어/이벤트 운동학(mlljj, ml1fatjet(boosted fit 관측량), ht/lt/st, MET 관련 전부).
- b-tagger는 Run2=DeepJet, Run3=ParticleNet 자동 분기.

## Claude Code용 작업 주의사항

1. **셀렉션을 바꿀 땐 두 분석기 모두 확인** — 이미 드리프트가 있으므로 (위 표) 어느 쪽이 기준인지 사용자에게 확인.
2. resolved 분기의 trigger-safe-pt `return`은 boosted까지 죽이는 의도된 동작. 함부로 continue로 바꾸지 말 것.
3. SR 블라인딩은 `if(!IsDATA)` 가드 (cc:1636, 2253, 2494). 수정 시 실수로 언블라인드하지 않게 주의.
4. 히스토그램 출력명의 `reseolved` 오타는 다운스트림(플로터/combine)이 그대로 참조하므로 고치려면 전체 체인 동기화 필요.
5. `_copy`의 영역별 flat 변수(~360개)와 fill 람다는 대규모 복붙 구조 — 관측량 하나 추가 시 6곳 이상 수정 필요.
6. 빈 스텁 블록 다수 (cc:1525,1534,1622-1629, 2823-2832) — 낮은 질량 CR 자리 표시자, 채워진 적 없음.
7. 실행: `SKNano.py -a Reproduce20_002_copy -i '<샘플>' -e <era> -n <N>` (COMMANDS.md 참조).
   출력: `/gv0/Users/achihwan/SKNanoOutput/<Analyzer>/<era>/`.
