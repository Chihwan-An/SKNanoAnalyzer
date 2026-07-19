# COMMANDS.md — HNWR 분석에서 자주 쓰는 명령어 모음

> 디렉토리별 실전 명령어 치트시트. (환경: tamsa 클러스터, micromamba `Nano` env, HTCondor)

## 0. 공통 환경

```bash
cd /data6/Users/achihwan/SKNanoAnalyzer-v13
source setup.sh          # 세션마다 필수. SKNANO_OUTPUT=/gv0/Users/achihwan/SKNanoOutput 등 설정
./scripts/build.sh       # 전체 빌드
./scripts/rebuild.sh     # 증분 빌드 (분석기 수정 후)
```

## 1. 분석기 실행 (Analyzers/) — SKNano.py

```bash
# 메인 히스토그램 분석기 (SR/CR + systematics)
SKNano.py -a Reproduce20_002_copy -i 'WR*'      -e 2022 -n 10
SKNano.py -a Reproduce20_002_copy -i 'DYJets*'  -e 2023BPix -n 50
SKNano.py -a Reproduce20_002_copy -i 'EGamma*'  -e 2024 -n 30    # 2024는 구 리포 HNWR 사용 권장 (트리거 블록 차이)

# BDT ntuple 생산 (BDT/ 입력)
SKNano.py -a HNWR_BDT_presel -i 'WR*' -e 2022 -n 10

# 유효 lumi 계산
SKNano.py -a GetEffLumi -i '<샘플>' -e <era>
```

- 출력: `/gv0/Users/achihwan/SKNanoOutput/<Analyzer>/<era>/<샘플>.root`
- 런로그/DAG: `/gv0/Users/achihwan/SKNanoRunlog/<날짜>_<Analyzer>/`
- 조업 확인: `condor_q`, `condor_q -dag`, 실패 시 런로그의 `dags/*.nodes.log` 확인
- era 병합본은 `SKNanoOutput/Reproduce20_002_copy/combined_22_23{,_24}/` (hadd 산출)

```bash
# 샘플별 출력 병합 (조각 job 출력 → 단일 파일)
hadd -f merged.root output_*.root
```

## 2. BDT/ — 학습~limit 체인

```bash
cd /data6/Users/achihwan/SKNanoAnalyzer-v13/BDT/SR_R_B
bash run_pipeline.sh test 2022        # 학습→템플릿→카드→limit→수집 전체 (test masspoint 세트)
bash run_pipeline.sh --set=<이름> <era>
bash submit.sh                        # 학습 condor만 수동 제출
condor_q                              # 진행 확인

# 피처 스터디
cd finding_best_feature && bash find_features.sh

# BDT limit 전체 (4 era + combined)
cd /data6/Users/achihwan/SKNanoAnalyzer-v13/BDT/combine
bash run_all.sh
INCLUDE_CR=1 bash run_all.sh                    # SR+CR rateParam 카드
HNWR_COMBINE_TAG=<tag> bash run_all.sh          # 출력을 scenarios/<tag>/로 분리
```

## 3. plots/ — 플로팅

```bash
cd plots/HNWR/reproduce/syst_plotter/integrated

# 제출 전 입력 검증 (syst 변형 누락 샘플 차단 — 필수 습관)
python check_inputs.py

# 단일 플롯 (리프 폴더에서)
python EE.py --hist SR_Resolved_EE_mlljj --output sr_ree_mlljj --rebin 4 --year 2022
python MM.py --hist DYCR_Boosted_MM_mass --year 2023 --show-sb

# 일괄: 리프 plotter.sh → era별 all.sh → condor
bash plotter.sh
condor_submit_dag condor_files/plotter_all.dag

# ATLAS 재현 플롯
cd plots/atlas && bash run_atlas_plots.sh
DATA_PATH=/gv0/Users/achihwan/SKNanoOutput/atlas/2022 SIGNALS=WR2000N1100 bash run_atlas_plots.sh
```

## 4. combine (CMSSW 영역) — 상세는 그쪽 COMMANDS.md

```bash
cd /data6/Users/achihwan/combine/CMSSW_14_1_0_pre4/src/HiggsAnalysis/CombinedLimit/data/HNWR
# → COMMANDS.md 참조 (shapes 재생성, datacard, condor limit, r값 추출)
```

## 5. Claude Code 활용 팁 (이 프로젝트에서 유용했던 패턴)

- 대형 .cc 분석기(20만줄대 파일)는 통째로 읽지 말고 함수/영역명 grep 후 offset/limit으로 부분 읽기.
- 셀렉션 수치 확인은 `Analyzers/src/Reproduce20_002_copy_SELECTION_NOTES.md`(한글) 먼저.
- 디렉토리 개요 파악: 각 영역 README.md → `docs/HNWR_Analyzers.md`, `BDT/README.md`,
  `plots/README.md`, combine `data/HNWR/README.md`.
- 코드 리뷰: `/code-review` (수정 diff 검증), 정리: `/simplify`.
- 개선 작업 시작 전 `RECOMMENDATIONS.md`에서 알려진 함정(문자열 튜플 버그 등) 확인.
