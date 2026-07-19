# RECOMMENDATIONS.md — HNWR 분석 코드베이스 개선 권고 (2026-07-12 기준)

> 대상: `SKNanoAnalyzer-v13`(분석기·BDT·plots), 구 리포 `SKNanoAnalyzer`(HNWR.cc),
> combine `data/HNWR`. 심층 코드 분석에서 나온 **증거 기반** 권고.
> 우선순위: 🔴 물리 결과에 영향 가능 / 🟠 유지보수 리스크 / 🟡 위생.

## 🔴 물리/정확성 — 먼저 확인·수정 권장

1. **두 분석기 간 셀렉션 드리프트** — `Reproduce20_002_copy`(히스토그램)와
   `HNWR_BDT_presel`(BDT ntuple)이 전자 트리거(Ele30 유무, safe-pt 35 vs 118),
   SR mll(400 vs 200), jet↔fatjet 클리닝, 팻젯 LSF/SDMass 컷, 전자 ID/Trig SF 구현까지
   서로 다름. 특히 **BDT_presel의 2022/2022EE 전자 ID SF는 사실상 1.0**
   (2023/23BPix 분기만 존재, HNWR_BDT_presel.cc:669-675). BDT 경로와 히스토그램 경로의
   CR/SR 정규화가 일치하지 않을 수 있음 → 어느 쪽이 기준인지 정하고 SF 로직 통일 필요.
2. **구 리포 HNWR.cc: 전 이벤트에 `GetTriggerLumi("HLT_Mu50")` 적용** (HNWR.cc:867) —
   전자 채널 정규화 오류 가능성. 채널별 트리거 lumi로 분리 권장.
3. **HNWR.cc ↔ v13 어느 쪽에도 완전판 없음**: v13에 2024 트리거/SF 없음(2024 실행 시
   전 이벤트 탈락), HNWR엔 2022~23BPix 실측 HEEP ID SF 없음(2023 상수 하드코딩).
   → v13의 `GetElectronHEEPIDSF_TnP` 테이블 + HNWR의 2024 블록을 **한 파일로 병합** 권장.
4. **HNWR.cc 뮤온 tight-charge 동결**: `Muon::TightCharge()` 주석화로 항상 기본값 →
   MM/Flav tight-charge 세부영역이 전부 not_tight로 붕괴. 살리거나 해당 영역 제거.
5. **누락 systematics** (양쪽 분석기 공통): b-tag SF, 뮤온 momentum scale(TuneP 전환으로
   사장), 전자 scale/smearing, L1 prefire 없음. JES는 "total" 단일 소스.
   전자 트리거 SF는 leading 전자에만 적용. 2024 뮤온 트리거 SF는 실측 아님(MyCorrection.cc).
   → AN 수준 결과 전 반드시 보강 목록화.
6. **combine 카드 생성기 잠복 버그**: `bn in ('rFlvCR')`는 튜플이 아닌 문자열 멤버십
   (make_datacardsEE.py:276, MM:274) — `('rFlvCR',)`로 수정.
   MM판 dict 안의 잘못 붙여넣은 shebang(MM:275) 제거.
7. **getrvalue.py**: xsec JSON 경로가 `/home/achihwan/...`(다른 곳은 `/data6`),
   MM 채널도 EE 키 단면적 사용(동일 가정 암묵적) — 경로 통일 + 가정 명시.
8. **2024 신호 대용 하드코딩**: 2023 신호 ×6.1147 스케일이 plots(EE.py:176-183)과
   combine(inject_2024_signal.py) 두 곳에 독립 구현 — 상수·로직 단일화, 결과 해석 시 주석 필수.

## 🟠 구조/유지보수 — 복붙 지옥 해소

9. **plots: 동일 플로터 124벌 복사** — `syst_plotter/integrated`의 EE.py/MM.py가
   바이트 동일 사본. 공유 모듈 1개 + 리프별 얇은 config로 통합 (출력 prefix 유도부
   `:704-709`만 파라미터화하면 됨). `make_combineroot.py`와 중복된
   `get_hist_from_file`/syst envelope도 같은 모듈로.
10. **plots: 변형 누락 처리 불일치** — year_sep은 한쪽(down) 계상, year_combined는
    Central 대체. `check_inputs.py` 가드를 플로터 내부로 넣어 fail-fast로.
11. **BDT: 학습 엔진 fork** — `proto/run_dedicated_masspoints.py`와 `SR_R_B/pysrc/` 사본이
    분기. proto를 삭제 또는 pysrc import로 전환. `proto2/`, `resolve_boost/`는 폐기 표시
    완료(README 배너) — 실제 삭제/아카이브 권장.
12. **BDT: cross-tree 역참조** — `make_fit_template.py`가 `sys.path.insert`로
    `../../combine/scripts`의 `sample_groups.py` 참조 — 패키지화/PYTHONPATH로.
13. **combine: datacard 생성기 ~10벌** (EE/MM × inclusive/OS/SS/SS+OS/scaled) —
    채널·charge·경로를 인자로 받는 단일 스크립트로 통합 (6번 버그도 자동 해결).
    `post_processor.py` 4벌도 charge 선택 파라미터화로 단일화.
14. **절대경로 하드코딩 전면 제거**: `RB_DIR`(config.sh:21), `PYTHON_EXE`,
    plots 8개 파일의 `/gv0/.../SKNanoOutput`, condor .sub/.dag의 `/data6/...`,
    job_worker.sh의 CMSSW 경로 등 → env var(`$SKNANO_OUTPUT` 재사용)/`BASH_SOURCE` 기반으로.

## 🟡 위생/정리

15. **백업 blob git 추적 제거**: `Analyzers/src/Reproduce20_002_copy.cc.bak`(1.14MB!),
    `.cc.backup`, `_backup_20260226.txt`, `include/*.h.backup`,
    BDT `make_fit_template.py.bak-*`, combine `*.root.bak-20260710` 다수 —
    git이 이력 관리하므로 삭제 + `.gitignore`에 `*.bak*` 추가.
16. **combine 결과 디렉토리 정리**: `limits_semiblind_failed_*`, `_negnorm_*`,
    `_contaminated_*`, `_pre2024fix` 등 → 날짜별 `archive/`로 이동 (잘못된 입력으로
    r값 추출하는 사고 방지).
17. **죽은 코드 제거**: 빈 CR 스텁(low-mass CR 등), 미사용 `isPassCustomTightID`,
    HNWR.h의 죽은 config 멤버(`Muon_IS_SF_Key` 등 — SF 키는 하드코딩이 실제),
    plots의 `plotall.sh`(깨진 경로), `plotting_code_test/`.
18. **출력명 오타**: `Cutflow_for_reseolved_SR`, `plots/HNWR/singnal_sample` —
    고치려면 다운스트림(플로터, combine 입력) 동시 수정 필요하므로 일괄 작업으로.
19. **히스토그램 비닝 정리** (HNWR.cc/v13 공통): mlljj 8000bin, pt 2000bin 등 과대 비닝 —
    파일 크기/hadd 시간 절감 여지 큼.
20. **문서 동기화 유지**: `best_bin/` README·config는 4-era 시절 스냅샷(2024 없음,
    RESULT의 승자 비닝이 현재 config에 없음) — "아카이브" 명시 완료, 재사용 시 5-era로 갱신
    필요. git 커밋 메시지 "backup"/"upd" 습관도 의미 있는 메시지로.

## 진행 상황 추적

- [ ] 1–8 (물리): 사용자 결정 필요 — 기준 분석기 선정, SF 병합 방향
- [x] proto2/resolve_boost/best_bin 문서에 아카이브 표시 (2026-07-12)
- [x] 각 영역 README 최신화 (plots, BDT, BDT/combine, combine data/HNWR) (2026-07-12)
- [ ] 9–14 (구조): 통합 리팩토링은 별도 세션에서 단계적으로
- [ ] 15–19 (위생): 반나절 작업, 물리 검증 불필요한 것부터
