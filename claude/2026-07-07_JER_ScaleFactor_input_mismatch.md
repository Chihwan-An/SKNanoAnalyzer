# 2026-07-07 작업 정리: `MyCorrection::GetJERSF()` "Too many inputs" 수정

## 증상

2024 JEC V2→V3 태그 수정 후 다음 진행 시 발생:

```
[MyCorrection::GetJERSF] Error during evaluation: Too many inputs
[MyCorrection::GetJERSF] Arguments (3): -3.37402 39.3007 nom
```

`safeEvaluate()`는 예외를 잡아서 로그를 남긴 뒤 `exit(EXIT_FAILURE)`로 종료하므로, gdb 없이도 매번
이 시점에서 조용히(그러나 확실하게) 죽음.

## 원인 (era 한정 버그 아님 — 전체 era 공통)

`AnalyzerTools/src/MyCorrection.cc`의 `GetJERSF()`가 JER Scale Factor correction을
`{eta, pt, syst_string}` 3개 인자로 evaluate 했는데, 실제 jsonPOG-integration JER
`"...ScaleFactor..."` correction 노드는 애초에 **`JetEta`, `JetPt` 2개 입력만** 받도록 정의되어
있어서 systematic(`nom`/`up`/`down`) 차원이 없음. `jet_jerc.json.gz`를 직접 풀어서 확인한 결과
2024(`Summer24Prompt24_JRV1`)뿐 아니라 2018 UL(`Summer19UL18_JRV3`), 2023BPix
(`Summer23BPixPrompt23_RunD_JRV2`) 등 다른 era 파일들도 전부 동일하게 2-input이었음 → 이 함수는
era에 관계없이 애초부터 correctionlib 스키마와 안 맞았던 오래된 버그로 보임 (Run2 분기
`{eta, getSystString_JME(syst)}`는 한술 더 떠서 pt조차 빠져있었음).

## 수정

각 correction 파일에 동일 패턴으로 존재하는 `"...SFUncertainty..."` 노드(입력은 마찬가지로
eta, pt)를 이용해서, 기존에 JES 쪽에서 쓰던 `GetJESUncertaintySF()`의 `nominal ± uncertainty`
패턴과 동일하게 조합하도록 변경 (`MyCorrection.cc`, `GetJERSF()`):

```cpp
float MyCorrection::GetJERSF(const float eta, const float pt,
                             const variation syst,
                             const TString &source) const {
  string cset_string = JME_JER_GT.at(GetEra().Data());
  cset_string.replace(cset_string.find("######"), 6, "ScaleFactor");
  correction::Correction::Ref cset = cset_jerc->at(cset_string);
  const float nominal = safeEvaluate(cset, "GetJERSF", {eta, pt});
  if (syst == variation::nom)
    return nominal;

  string unc_cset_string = JME_JER_GT.at(GetEra().Data());
  unc_cset_string.replace(unc_cset_string.find("######"), 6, "SFUncertainty");
  correction::Correction::Ref unc_cset = cset_jerc->at(unc_cset_string);
  const float unc = safeEvaluate(unc_cset, "GetJERSF", {eta, pt});
  return (syst == variation::up) ? nominal + unc : nominal - unc;
}
```

Run2/Run3 분기를 없애고 하나의 로직으로 통일함 (기존에 Run2/Run3 모두 어차피 잘못된 입력 개수를 넘기고
있었기 때문).

## 확인 방법

- `zcat jet_jerc.json.gz`로 여러 era 파일(2018 UL, 2023BPix, 2024)의 `"...ScaleFactor..."`,
  `"...SFUncertainty..."` 노드 `inputs` 필드를 직접 파싱해서 전부 2-input(JetEta, JetPt)이고
  systematic 입력이 없음을 확인.
- 빌드/재실행은 사용자가 직접 확인.

## 관련

- [[2026-07-07_2024_JEC_V2_V3_mismatch]] — 같은 세션에서 먼저 발견/수정한 2024 JES V2→V3 태그
  불일치 크래시. 그 크래시를 고친 뒤 다음 단계(JER smearing)에서 이 버그가 드러남.
