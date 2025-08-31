## W_tau_test.h
```c
	//default setting
```
## W_tau_test.c
```c
#include "Wtau_test.h"

void Wtau_test::initializeAnalyzer(){
}

void Wtau_test::executeEvent(){

AnalyzerParameter param;

executeEventFromParameter(param);

}

void Wtau_test::executeEventFromParameter(AnalyzerParameter param){

if(!PassMETFilter()) return;
FillHist("Not triggered/Cutflow", 0., 1., 10, 0., 10.); // 메트 필터를 통과하지 못했을 경우의 히스토그램
Event ev = GetEvent();

if(ev.PassTrigger("HLT_MediumChargedIsoPFTau180HighPtRelaxedIso_Trk50_eta2p1_v")){

FillHist("Wtau_test/Cutflow", 0., 1., 10, 0., 10.); // 타우만을 통과했을 경우의 히스토그램

}
if(ev.PassTrigger({"HLT_Mu50_v","HLT_MediumChargedIsoPFTau180HighPtRelaxedIso_Trk50_eta2p1_v"})){

FillHist("Wtau_test/Cutflow", 1., 1., 10, 0., 10.); // 뮤온과 타우를 모두 통과했을 경우의 히스토그램

}

}
Wtau_test::Wtau_test(){
}
Wtau_test::~Wtau_test(){

}

//mutrigger HLT_Mu50_v

//HLT_MediumChargedIsoPFTau180HighPtRelaxedIso_Trk50_eta2p1_v
```
- 이 후에 
```c
/data9/Users/snuintern1/SKFlatAnalyzer/data/Run2UltraLegacy_v3/2018/Sample
```
에서 CommonSampleInfo 와 ForSNU에 원하는 샘플 파일의 Alias 정식명칭 , crossection등의 정보를 담은 파일을 넣어야 함.

최종적으로 사용하는 alias  코드를 담은 텍스트파일을 최상위 폴더에 작성하고 
SKFlat.py -a Wtau_test -l Wtau_test.txt -n 10 -e 2018 이 코드를 입력해서 잡 시작 