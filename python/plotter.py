# ROOT 라이브러리 import - CERN에서 개발한 물리학 데이터 분석용 라이브러리
import ROOT
# CMS 스타일 라이브러리 import - CMS 실험의 표준 플롯 스타일을 제공
import cmsstyle as CMS
# Python의 array 모듈 import - 효율적인 배열 처리를 위해 사용
from array import array

# 기본 색상 팔레트 정의 (6개 색상) - 그래프에 사용할 색상들을 미리 정의
PALLETE = [
        ROOT.TColor.GetColor("#5790fc"),  # 파란색
        ROOT.TColor.GetColor("#f89c20"),  # 주황색
        ROOT.TColor.GetColor("#e42536"),  # 빨간색
        ROOT.TColor.GetColor("#964a8b"),  # 보라색
        ROOT.TColor.GetColor("#9c9ca1"),  # 회색
        ROOT.TColor.GetColor("#7a21dd")   # 진한 보라색
]

# 긴 색상 팔레트 정의 (10개 색상) - 더 많은 데이터셋이 있을 때 사용
PALLETE_LONG = [
        ROOT.TColor.GetColor("#3f90da"),  # 하늘색
        ROOT.TColor.GetColor("#ffa90e"),  # 노란색
        ROOT.TColor.GetColor("#bd1f01"),  # 진한 빨간색
        ROOT.TColor.GetColor("#94a4a2"),  # 연한 회색
        ROOT.TColor.GetColor("#832db6"),  # 자주색
        ROOT.TColor.GetColor("#a96b59"),  # 갈색
        ROOT.TColor.GetColor("#e76300"),  # 주황빨강
        ROOT.TColor.GetColor("#b9ac70"),  # 올리브색
        ROOT.TColor.GetColor("#717581"),  # 어두운 회색
        ROOT.TColor.GetColor("#92dadd")   # 청록색
]

# KinematicCanvas 클래스 정의 - 운동학적 변수들을 비교하는 캔버스 클래스
class KinematicCanvas():
    # 생성자 함수 - 히스토그램들, 설정, 참조 히스토그램을 받아서 초기화
    def __init__(self, hists, config, ref=None):
        # 부모 클래스의 생성자 호출
        super().__init__()

        # x축 범위가 대칭적인지 확인 (예: -5에서 5까지)
        if abs(config["xRange"][0]) == abs(config["xRange"][1]):
            # xRange가 대칭적이면 오버플로우 처리가 필요 없음
            # Sumw2()는 오차 계산을 위해 호출해야 함
            if ref is not None:  # 참조 히스토그램이 있으면
                ref.Sumw2()  # 참조 히스토그램에 오차 계산 활성화
            for hist in hists.values():  # 모든 히스토그램에 대해
                hist.Sumw2()  # 오차 계산 활성화
        else:
            # xRange가 비대칭적이면 오버플로우를 마지막 빈에 추가해야 함
            if ref is not None:  # 참조 히스토그램이 있으면
                last_bin = ref.FindBin(config["xRange"][-1])  # 마지막 빈의 인덱스 찾기
                overflow, overflow_err = 0., 0.  # 오버플로우 값과 오차 초기화
                # 마지막 빈 이후의 모든 빈들을 순회하며 오버플로우 계산
                for idx in range(last_bin+1, ref.GetNbinsX()+1):
                    overflow += ref.GetBinContent(idx)  # 오버플로우 값 누적
                    overflow_err += ref.GetBinError(idx)**2  # 오차의 제곱 누적
                overflow_err = ROOT.TMath.Sqrt(overflow_err)  # 오차의 제곱근 계산
                # 마지막 빈에 오버플로우 값 추가
                ref.SetBinContent(last_bin, ref.GetBinContent(last_bin)+overflow)
                # 마지막 빈의 오차에 오버플로우 오차 추가 (오차는 제곱합의 제곱근)
                ref.SetBinError(last_bin, ROOT.TMath.Sqrt(ref.GetBinError(last_bin)**2 + overflow_err**2))

            # 모든 히스토그램에 대해 동일한 오버플로우 처리 수행
            for hist in hists.values():
                last_bin = hist.FindBin(config["xRange"][-1])  # 마지막 빈의 인덱스 찾기
                overflow, overflow_err = 0., 0.  # 오버플로우 값과 오차 초기화
                # 마지막 빈 이후의 모든 빈들을 순회하며 오버플로우 계산
                for idx in range(last_bin+1, hist.GetNbinsX()+1):
                    overflow += hist.GetBinContent(idx)  # 오버플로우 값 누적
                    overflow_err += hist.GetBinError(idx)**2  # 오차의 제곱 누적
                overflow_err = ROOT.TMath.Sqrt(overflow_err)  # 오차의 제곱근 계산
                # 마지막 빈에 오버플로우 값 추가
                hist.SetBinContent(last_bin, hist.GetBinContent(last_bin)+overflow)
                # 마지막 빈의 오차에 오버플로우 오차 추가
                hist.SetBinError(last_bin, ROOT.TMath.Sqrt(hist.GetBinError(last_bin)**2 + overflow_err**2))
        
        # 클래스 변수들 저장
        self.ref = ref  # 참조 히스토그램 저장
        self.hists = hists  # 비교할 히스토그램들 저장
        # 비율 플롯에서 1.0 기준선을 그리기 위한 선 객체 생성
        self.ref_line = ROOT.TLine(config["xRange"][0], 1., config["xRange"][-1], 1.)

        # 히스토그램 개수에 따라 색상 팔레트 선택
        if len(hists) > 6:  # 히스토그램이 6개보다 많으면
            self.PALLETE = PALLETE_LONG  # 긴 팔레트 사용 (10개 색상)
        else:  # 6개 이하면
            self.PALLETE = PALLETE  # 기본 팔레트 사용 (6개 색상)

        # 히스토그램 빈 설정 처리
        if "rebin" in config.keys():  # 설정에 rebin이 있으면
            self.ref.Rebin(config["rebin"])  # 참조 히스토그램 리빈
            for hist in self.hists.values():  # 모든 히스토그램에 대해
                hist.Rebin(config["rebin"])  # 동일한 리빈 적용
        elif len(config["xRange"]) > 2:  # xRange가 3개 이상의 값이 있으면 (가변 빈 크기)
            # 설정에서 가변 빈 크기 배열 생성
            bins = array('d', config["xRange"])  # double 타입 배열로 변환
            # 가변 빈 크기로 히스토그램 리빈
            self.ref = self.ref.Rebin(len(bins)-1, self.ref.GetName()+"_rebin", bins)
            for name, hist in self.hists.items():  # 모든 히스토그램에 대해
                self.hists[name] = hist.Rebin(len(bins)-1, hist.GetName()+"_rebin", bins)
        else:  # 리빈 설정이 없으면
            pass  # 아무것도 하지 않음

        # 참조 히스토그램이 있으면 비율 히스토그램 생성
        if ref is not None:
            self.ratio_hists = {}  # 비율 히스토그램 딕셔너리 초기화
            for name, hist in self.hists.items():  # 모든 히스토그램에 대해
                self.ratio_hists[name] = hist.Clone(f"{name}_ratio")  # 히스토그램 복사
                self.ratio_hists[name].Divide(self.ref)  # 참조 히스토그램으로 나누어 비율 계산
        
        # y축 범위 설정
        ymin, ymax = 0, ref.GetMaximum()*2  # 기본 y축 범위: 0부터 최대값의 2배까지
        if "logy" in config.keys() and config['logy']:  # 로그 스케일이 설정되어 있으면
            if ref.GetMinimum() > 0:  # 최소값이 0보다 크면
                ymin = ref.GetMinimum()*0.5  # 최소값의 절반으로 설정
            else:  # 최소값이 0 이하면
                ymin = 1.  # 1로 설정 (로그 스케일에서 0은 불가능)
            ymax = ref.GetMaximum()*1e3  # 최대값의 1000배로 설정

        # 캔버스 생성
        CMS.SetEnergy(13.6)  # CMS 에너지 설정 (13.6 TeV)
        CMS.SetLumi("")  # 루미노시티 텍스트 설정 (빈 문자열)
        CMS.SetExtraText("Simulation Preliminary")  # 추가 텍스트 설정
        # 이중 캔버스 생성 (위쪽: 메인 플롯, 아래쪽: 비율 플롯)
        self.canv = CMS.cmsDiCanvas("", 
                                    config["xRange"][0], config["xRange"][-1],  # x축 범위
                                    ymin, ymax,  # y축 범위
                                    config["yRange"][0], config["yRange"][1],  # 비율 플롯 y축 범위
                                    config["xTitle"], config["yTitle"],  # 축 제목
                                    config["ratioTitle"],  # 비율 플롯 제목
                                    square=True,  # 정사각형 모양
                                    iPos=11,  # CMS 로고 위치
                                    extraSpace=0)  # 추가 공간
        if "logy" in config.keys() and config['logy']:  # 로그 스케일이 설정되어 있으면
            self.canv.cd(1).SetLogy()  # 첫 번째 패드에 로그 스케일 적용
        # 범례 생성 (위치: x=0.7~0.99, y=0.54~0.89, 텍스트 크기=0.04, 1열)
        self.leg = CMS.cmsLeg(0.7, 0.89 - 0.05 * 7, 0.99, 0.89, textSize=0.04, columns=1)

    # 운동학적 패드 그리기 함수 (위쪽 패드)
    def drawKinematicPad(self):
        self.canv.cd(1)  # 첫 번째 패드로 이동
        # y축 숫자 표시 자릿수를 3자리로 제한
        CMS.GetcmsCanvasHist(self.canv.GetPad(1)).GetYaxis().SetMaxDigits(3)
        # 참조 히스토그램 그리기 (점과 선, 오차막대 포함)
        CMS.cmsDraw(self.ref, "PLE", fcolor=ROOT.kWhite, lcolor=ROOT.kBlack, lwidth=2, mcolor=ROOT.kBlack, msize=1.)
        self.leg.AddEntry(self.ref, "LEGACY", "PLE")  # 범례에 참조 히스토그램 추가
        # 모든 히스토그램에 대해 반복
        for idx, (name, hist) in enumerate(self.hists.items()):
            # 히스토그램을 히스토그램 스타일로 그리기 (점선)
            CMS.cmsDraw(hist, "Hist", fcolor=ROOT.kWhite, lcolor=self.PALLETE[idx], lwidth=2, lstyle=ROOT.kDashed)
            # 히스토그램을 오차막대와 함께 그리기
            CMS.cmsDraw(hist, "LE", lcolor=self.PALLETE[idx], lwidth=2, lstyle=ROOT.kDashed,
                                    fcolor=ROOT.kWhite, msize=0.)
            self.leg.AddEntry(hist, name, "LE")  # 범례에 히스토그램 추가
        self.canv.cd(1).RedrawAxis()  # 축 다시 그리기

    # 비율 패드 그리기 함수 (아래쪽 패드)
    def drawRatioPad(self):
        self.canv.cd(2)  # 두 번째 패드로 이동
        # y축 제목 크기 설정
        CMS.GetcmsCanvasHist(self.canv.GetPad(2)).GetYaxis().SetTitleSize(0.08)
        # y축 제목 오프셋 설정
        CMS.GetcmsCanvasHist(self.canv.GetPad(2)).GetYaxis().SetTitleOffset(0.8)
        # 1.0 기준선 그리기 (점선)
        CMS.cmsDrawLine(self.ref_line, lcolor=ROOT.kBlack, lstyle=ROOT.kDotted)
        # 모든 비율 히스토그램에 대해 반복
        for idx, (name, hist) in enumerate(self.ratio_hists.items()):
            # 비율 히스토그램을 히스토그램 스타일로 그리기
            CMS.cmsDraw(hist, "Hist", fcolor=ROOT.kWhite, lcolor=self.PALLETE[idx], lwidth=2, lstyle=ROOT.kDashed)
            # 비율 히스토그램을 오차막대와 함께 그리기
            CMS.cmsDraw(hist, "PLE", lcolor=self.PALLETE[idx], lwidth=2, lstyle=ROOT.kDashed,
                                    fcolor=ROOT.kWhite, msize=0.)
        self.canv.cd(2).RedrawAxis()  # 축 다시 그리기


# StackedCanvas 클래스 정의 - 스택된 히스토그램을 그리는 캔버스 클래스
class StackedCanvas():
    # 생성자 함수 - 데이터, 히스토그램들, 설정을 받아서 초기화
    def __init__(self, data, hists, config):
        # 부모 클래스의 생성자 호출
        super().__init__()
        # 히스토그램과 설정 저장
        self.data = data  # 데이터 히스토그램 저장
        self.hists = hists  # 시뮬레이션 히스토그램들 저장
        self.stack = ROOT.THStack("stack", "stack")  # 스택 객체 생성

        # 오차 계산을 위해 Sumw2() 호출
        self.data.Sumw2()  # 데이터 히스토그램에 오차 계산 활성화
        for hist in self.hists.values():  # 모든 시뮬레이션 히스토그램에 대해
            hist.Sumw2()  # 오차 계산 활성화

        # 히스토그램 개수에 따라 색상 팔레트 선택
        if len(hists) > 6:  # 히스토그램이 6개보다 많으면
            self.PALLETE = PALLETE_LONG  # 긴 팔레트 사용
        else:  # 6개 이하면
            self.PALLETE = PALLETE  # 기본 팔레트 사용

        # 히스토그램 빈 설정 처리
        if "rebin" in config.keys():  # 설정에 rebin이 있으면
            self.data.Rebin(config["rebin"])  # 데이터 히스토그램 리빈
            for hist in self.hists.values():  # 모든 히스토그램에 대해
                hist.Rebin(config["rebin"])  # 동일한 리빈 적용
        elif len(config["xRange"]) > 2:  # xRange가 3개 이상의 값이 있으면 (가변 빈 크기)
            # 설정에서 가변 빈 크기 배열 생성
            bins = array('d', config["xRange"])  # double 타입 배열로 변환
            # 가변 빈 크기로 히스토그램 리빈
            self.data = self.data.Rebin(len(bins)-1, self.data.GetName()+"_rebin", bins)
            for name, hist in self.hists.items():  # 모든 히스토그램에 대해
                self.hists[name] = hist.Rebin(len(bins)-1, hist.GetName()+"_rebin", bins)
        else:  # 리빈 설정이 없으면
            pass  # 아무것도 하지 않음

        # 필요한 복사본 생성
        self.systematics = None  # 시스템틱 히스토그램 초기화
        for hist in self.hists.values():  # 모든 히스토그램에 대해
            if self.systematics is None:  # 첫 번째 히스토그램이면
                self.systematics = hist.Clone("syst")  # 복사본 생성
            else:  # 그 이후 히스토그램들이면
                self.systematics.Add(hist)  # 기존 시스템틱에 추가

        # 데이터/시뮬레이션 비율 히스토그램 생성
        self.ratio = data.Clone("ratio")  # 데이터 히스토그램 복사
        self.ratio.Divide(self.systematics)  # 시스템틱으로 나누어 비율 계산
        # y축 범위 설정
        ymin = 0.  # 최소값 0
        ymax = self.systematics.GetMaximum()*2  # 최대값은 시스템틱 최대값의 2배
        if "logy" in config.keys() and config['logy']:  # 로그 스케일이 설정되어 있으면
            ymin = 1e-3  # 최소값 0.001
            ymax = self.systematics.GetMaximum()*1e3  # 최대값은 시스템틱 최대값의 1000배
  
        # 기본 설정
        CMS.SetEnergy(13.6)  # CMS 에너지 설정 (13.6 TeV)
        CMS.SetExtraText("Preliminary")  # 추가 텍스트 설정
        # 이중 캔버스 생성 (위쪽: 스택 플롯, 아래쪽: 비율 플롯)
        self.canv = CMS.cmsDiCanvas("", config["xRange"][0], config["xRange"][-1], ymin, ymax, config["yRange"][0], config["yRange"][1], config["xTitle"], config["yTitle"], "Data / Pred", square=True, iPos=11, extraSpace=0)
        # 범례 생성
        self.leg = CMS.cmsLeg(0.7, 0.89 - 0.05 * 7, 0.99, 0.89, textSize=0.04, columns=1)

    # 위쪽 패드 그리기 함수 (스택 플롯)
    def drawPadUp(self):
        self.canv.cd(1)  # 첫 번째 패드로 이동
        self.leg.AddEntry(self.data, "Data", "PE")  # 범례에 데이터 추가
        # 스택 히스토그램 그리기
        CMS.cmsDrawStack(self.stack, self.leg, self.hists)
        # 시스템틱 오차를 점선으로 그리기
        CMS.cmsDraw(self.systematics, "E2", fcolor=ROOT.kBlack, fstyle=3004, msize=0.)
        # 데이터를 점과 오차막대로 그리기
        CMS.cmsDraw(self.data, "PE", mcolor=ROOT.kBlack, msize=1.)
        self.leg.AddEntry(self.systematics, "Stat+Syst", "FE2")  # 범례에 시스템틱 추가
        self.canv.cd(1).RedrawAxis()  # 축 다시 그리기

    # 아래쪽 패드 그리기 함수 (비율 플롯)
    def drawPadDown(self):
        self.canv.cd(2)  # 두 번째 패드로 이동
        # 비율 히스토그램을 오차막대와 함께 그리기
        CMS.cmsDraw(self.ratio, "PE2", fcolor=ROOT.kBlack, fstyle=3004, msize=1.)
        self.canv.cd(2).RedrawAxis()  # 축 다시 그리기