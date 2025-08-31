```python 

import glob

from ROOT import *

  

# 디렉토리 경로

dirW1000N100 = "/data6/Users/snuintern1/nano3/CMSSW_10_6_22/src/PhysicsTools/NanoAODTools/python/postprocessing/analyzer/analyzer/DeltaRplot/result/WR1000/N100"

dirW1000N500 = "/data6/Users/snuintern1/nano3/CMSSW_10_6_22/src/PhysicsTools/NanoAODTools/python/postprocessing/analyzer/analyzer/DeltaRplot/result/WR1000/N500"

dirW1000N900 = "/data6/Users/snuintern1/nano3/CMSSW_10_6_22/src/PhysicsTools/NanoAODTools/python/postprocessing/analyzer/analyzer/DeltaRplot/result/WR1000/N900"

dirW2000N100 = "/data6/Users/snuintern1/nano3/CMSSW_10_6_22/src/PhysicsTools/NanoAODTools/python/postprocessing/analyzer/analyzer/DeltaRplot/result/WR2000/N100"

dirW2000N1000 = "/data6/Users/snuintern1/nano3/CMSSW_10_6_22/src/PhysicsTools/NanoAODTools/python/postprocessing/analyzer/analyzer/DeltaRplot/result/WR2000/N1000"

dirW2000N1900 = "/data6/Users/snuintern1/nano3/CMSSW_10_6_22/src/PhysicsTools/NanoAODTools/python/postprocessing/analyzer/analyzer/DeltaRplot/result/WR2000/N1900"

dirW4000N100 = "/data6/Users/snuintern1/nano3/CMSSW_10_6_22/src/PhysicsTools/NanoAODTools/python/postprocessing/analyzer/analyzer/DeltaRplot/result/WR4000/N100"

dirW4000N2000 = "/data6/Users/snuintern1/nano3/CMSSW_10_6_22/src/PhysicsTools/NanoAODTools/python/postprocessing/analyzer/analyzer/DeltaRplot/result/WR4000/N2000"

dirW4000N3900 = "/data6/Users/snuintern1/nano3/CMSSW_10_6_22/src/PhysicsTools/NanoAODTools/python/postprocessing/analyzer/analyzer/DeltaRplot/result/WR4000/N3900"

  
  

## 루트 파일 가져오기

file_list_W1000N100 = glob.glob(f"{dirW1000N100}/*.root")

file_list_W1000N500 = glob.glob(f"{dirW1000N500}/*.root")

file_list_W1000N900 = glob.glob(f"{dirW1000N900}/*.root")

file_list_W2000N100 = glob.glob(f"{dirW2000N100}/*.root")

file_list_W2000N1000 = glob.glob(f"{dirW2000N1000}/*.root")

file_list_W2000N1900 = glob.glob(f"{dirW2000N1900}/*.root")

file_list_W4000N100 = glob.glob(f"{dirW4000N100}/*.root")

file_list_W4000N2000 = glob.glob(f"{dirW4000N2000}/*.root")

file_list_W4000N3900 = glob.glob(f"{dirW4000N3900}/*.root")

  

# 출력 ROOT 파일 생성

output_file = TFile("results.root", "RECREATE")

  
  

# 히스토그램 병합을 위한 빈 히스토그램 생성

combined_W1000N100 = TH1F("W1000N100", "W1000N100", 50, 0, 5)

combined_W1000N500 = TH1F("W1000N500", "W1000N500", 50, 0, 5)

combined_W1000N900 = TH1F("W1000N900", "W1000N900", 50, 0, 5)

combined_W2000N100 = TH1F("W2000N100", "W2000N100", 50, 0, 5)

combined_W2000N1000 = TH1F("W2000N1000", "W2000N1000", 50, 0, 5)

combined_W2000N1900 = TH1F("W2000N1900", "W2000N1900", 50, 0, 5)

combined_W4000N100 = TH1F("W4000N100", "W4000N100", 50, 0, 5)

combined_W4000N2000 = TH1F("W4000N2000", "W4000N2000", 50, 0, 5)

combined_W4000N3900 = TH1F("W4000N3900", "W4000N3900", 50, 0, 5)

  
  
  

# 히스토그램 색상 설정

combined_W1000N100.SetLineColor(kRed)

combined_W1000N500.SetLineColor(kGreen)

combined_W1000N900.SetLineColor(kBlue)

  

combined_W2000N100.SetLineColor(kRed)

combined_W2000N1000.SetLineColor(kGreen)

combined_W2000N1900.SetLineColor(kBlue)

  

combined_W4000N100.SetLineColor(kRed)

combined_W4000N2000.SetLineColor(kGreen)

combined_W4000N3900.SetLineColor(kBlue)

  
  

# 병합 히스토그램 생성

combined_WR1000 = THStack("combined_WR1000", "Combined WR1000; delta R; Events")

combined_WR2000 = THStack("combined_WR2000", "Combined WR2000; delta R; Events")

combined_WR4000 = THStack("combined_WR4000", "Combined WR4000; delta R; Events")

  
  

# 각 파일을 읽고 히스토그램 병합

  

for files in file_list_W1000N100:

print(f"Processing file: {files}")

file_plots = TFile.Open(files) # ROOT 파일 열기

plots_dir = file_plots.Get("plots") # "plots" 디렉토리 가져오기

  

# 각 히스토그램 가져오기

W1000N100 = plots_dir.Get("deltaR_dist")

#integral = W1000N100.Integral() # 히스토그램의 이벤트 수 (Integral)

#W1000N100.Scale(1.0 / integral) # 정규화

# 가져온 히스토그램 데이터를 병합 히스토그램에 추가

combined_W1000N100.Add(W1000N100)

file_plots.Close()

  

for files in file_list_W1000N500:

print(f"Processing file: {files}")

file_plots = TFile.Open(files) # ROOT 파일 열기

plots_dir = file_plots.Get("plots") # "plots" 디렉토리 가져오기

  

# 각 히스토그램 가져오기

W1000N500 = plots_dir.Get("deltaR_dist")

#integral = W1000N500.Integral() # 히스토그램의 이벤트 수 (Integral)

#W1000N500.Scale(1.0 / integral) # 정��화

# 가져온 히스토그램 데이터를 병합 히스토그램에 추가

combined_W1000N500.Add(W1000N500)

file_plots.Close()

  

for files in file_list_W1000N900:

print(f"Processing file: {files}")

file_plots = TFile.Open(files) # ROOT 파일 열기

plots_dir = file_plots.Get("plots") # "plots" 디렉토리 가져오기

  

# 각 히스토그램 가져오기

W1000N900 = plots_dir.Get("deltaR_dist")

# 가져온 히스토그램 데이터를 병합 히스토그램에 추가

#integral = W1000N900.Integral() # 히스토그램의 이벤트 수 (Integral)

#W1000N900.Scale(1.0 / integral) # 정��화

combined_W1000N900.Add(W1000N900)

file_plots.Close()

  
  

for files in file_list_W2000N100:

print(f"Processing file: {files}")

file_plots = TFile.Open(files) # ROOT 파일 열기

plots_dir = file_plots.Get("plots") # "plots" 디렉토리 가져오기

  

# 각 히스토그램 가져오기

W2000N100 = plots_dir.Get("deltaR_dist")

#integral = W2000N100.Integral() # 히스토그램의 이벤트 수 (Integral)

#W2000N100.Scale(1.0 / integral) # 정���화

# 가져온 히스토그램 데이터를 병합 히스토그램에 추가

combined_W2000N100.Add(W2000N100)

file_plots.Close()

  
  

for files in file_list_W2000N1000:

print(f"Processing file: {files}")

file_plots = TFile.Open(files) # ROOT 파일 ��기

plots_dir = file_plots.Get("plots") # "plots" 디��토리 가져오기

  

# 각 ���스토그램 가져오기

W2000N1000 = plots_dir.Get("deltaR_dist")

#integral = W2000N1000.Integral() # 히스토그램의 이벤트 수 (Integral)

#W2000N1000.Scale(1.0 / integral) # 정��화

# 가져�� ���스토그램 데이터를 ��합 ���스토그램에 추가

combined_W2000N1000.Add(W2000N1000)

file_plots.Close()

  

for files in file_list_W2000N1900:

print(f"Processing file: {files}")

file_plots = TFile.Open(files) # ROOT 파일 ��기

plots_dir = file_plots.Get("plots") # "plots" 디��토리 가져오기

  

# 각 ���스토그램 가져오기

W2000N1900 = plots_dir.Get("deltaR_dist")

# 가져�� ���스토그램 데이터를 ��합 ���스토그램에 추가

#integral = W2000N1900.Integral() # 히스토그램의 이벤트 수 (Integral)

#W2000N1900.Scale(1.0 / integral) # 정��화

combined_W2000N1900.Add(W2000N1900)

file_plots.Close()

  

for files in file_list_W4000N100:

print(f"Processing file: {files}")

file_plots = TFile.Open(files) # ROOT 파일 ��기

plots_dir = file_plots.Get("plots") # "plots" 디��토리 가져오기

  

# 각 ���스토그램 가져오기

W4000N100 = plots_dir.Get("deltaR_dist")

#integral = W4000N100.Integral() # ���스토그램의 이���트 수 (Integral)

#W4000N100.Scale(1.0 / integral) # 정��화

# 가져�� ���스토그램 데이터를 ��합 ���스토그램에 추가

combined_W4000N100.Add(W4000N100)

file_plots.Close()

  

for files in file_list_W4000N2000:

print(f"Processing file: {files}")

file_plots = TFile.Open(files) # ROOT 파일 ��기

plots_dir = file_plots.Get("plots") # "plots" 디��토리 가져오기

  

# 각 ���스토그램 가져오기

W4000N2000 = plots_dir.Get("deltaR_dist")

#integral = W4000N2000.Integral() # ���스토그램의 이���트 수 (Integral)

#W4000N2000.Scale(1.0 / integral) # 정��화

# 가져�� ���스토그램 데이터를 ��합 ���스토그램에 추가

combined_W4000N2000.Add(W4000N2000)

file_plots.Close()

  

for files in file_list_W4000N3900:

print(f"Processing file: {files}")

file_plots = TFile.Open(files) # ROOT 파일 ��기

plots_dir = file_plots.Get("plots") # "plots" 디��토리 가져오기

  

# 각 ���스토그램 가져오기

W4000N3900 = plots_dir.Get("deltaR_dist")

#integral = W4000N3900.Integral() # ���스토그램의 이���트 수 (Integral)

#W4000N3900.Scale(1.0 / integral) # 정��화

# 가져�� ���스토그램 데이터를 ��합 ���스토그램에 추가

combined_W4000N3900.Add(W4000N3900)

file_plots.Close()

  

# TCanvas 생성

canvas_WR1000 = TCanvas("canvas_WR1000", "WR1000 Comparison", 800, 600)

canvas_WR2000 = TCanvas("canvas_WR2000", "WR2000 Comparison", 800, 600)

canvas_WR4000 = TCanvas("canvas_WR4000", "WR4000 Comparison", 800, 600)

  

# 히스토그램 겹쳐 그리기 설정

def draw_histograms(canvas, hists, title):

"""

canvas : TCanvas 객체

hists : 그릴 히스토그램 리스트 (예: [hist1, hist2, hist3])

title : 첫 번째 히스토그램에 표시할 Title

"""

# 1) 캔버스 설정

canvas.cd()

canvas.SetGrid(1, 1) # (X축, Y축) 격자

# 필요 시 canvas.Clear() 등으로 초기화 가능

  

# 2) 레전드 생성

# 위치를 넓게 잡아서 히스토그램이 레전드를 가리지 않도록 함

legend = TLegend(0.55, 0.65, 0.88, 0.88)

legend.SetTextSize(0.03)

legend.SetBorderSize(1) # 테두리 있음 (0이면 테두리 없음)

legend.SetFillStyle(1001) # 레전드 박스 채우기 (0으로 하면 투명)

legend.SetFillColor(kWhite) # 하얀 배경

legend.SetTextColor(kBlack) # 검정색 글자

  

# 3) 히스토그램 Draw 및 레전드 등록

first_hist = True

for i, hist in enumerate(hists):

# (3-1) 히스토그램 그리기

if first_hist:

hist.SetTitle(title) # 캔버스 윗부분에 표시될 제목; "X축; Y축" 형식

hist.Draw("HIST") # 첫 히스토그램

first_hist = False

else:

hist.Draw("HIST SAME") # 겹쳐 그리기

  

# (3-2) 레전드에 항목 추가 (Draw 후에!)

if i == 0:

legend.AddEntry(hist, "mN = 100 (GeV)", "l") # "l" -> 선 스타일

elif i == 1:

legend.AddEntry(hist, "mN = mWR / 2 (GeV)", "l")

elif i == 2:

legend.AddEntry(hist, "mN = mWR - 100 (GeV)", "l")

  

# 4) 레전드 그리기

legend.Draw("SAME") # "SAME" 옵션으로 캔버스에 겹쳐 그림

  

# 5) 캔버스 업데이트

canvas.Update()

print("Number of legend entries:", legend.GetNRows())

## 각 샘플 갯수 다르므로 noramlization

  

# X축 범위 제한

combined_W1000N100.GetXaxis().SetRangeUser(0.1, 5)

combined_W1000N500.GetXaxis().SetRangeUser(0.1, 5)

combined_W1000N900.GetXaxis().SetRangeUser(0.1, 5)

  

combined_W2000N100.GetXaxis().SetRangeUser(0.1, 5)

combined_W2000N1000.GetXaxis().SetRangeUser(0.1, 5)

combined_W2000N1900.GetXaxis().SetRangeUser(0.1, 5)

  

combined_W4000N100.GetXaxis().SetRangeUser(0.1, 5)

combined_W4000N2000.GetXaxis().SetRangeUser(0.1, 5)

combined_W4000N3900.GetXaxis().SetRangeUser(0.1, 5)

  

# WR1000 그룹 히스토그램 그리기

combined_W1000N100.Scale(1/combined_W1000N100.Integral())

combined_W1000N500.Scale(1/combined_W1000N500.Integral())

combined_W1000N900.Scale(1/combined_W1000N900.Integral())

draw_histograms(

canvas_WR1000,

[combined_W1000N100, combined_W1000N500, combined_W1000N900],

"WR1000 Comparison; DeltaR(q,q); A.U."

)

  
  
  
  

combined_W2000N100.Scale(1/combined_W2000N100.Integral())

combined_W2000N1000.Scale(1/combined_W2000N1000.Integral())

combined_W2000N1900.Scale(1/combined_W2000N1900.Integral())

# WR2000 그룹 히스토그램 그리기

draw_histograms(

canvas_WR2000,

[combined_W2000N100, combined_W2000N1000, combined_W2000N1900],

"WR2000 Comparison; DeltaR(q,q); A.U."

)

  
  

# WR4000 그룹 히스토그램 그리기

combined_W4000N100.Scale(1/combined_W4000N100.Integral())

combined_W4000N2000.Scale(1/combined_W4000N2000.Integral())

combined_W4000N3900.Scale(1/combined_W4000N3900.Integral())

draw_histograms(

canvas_WR4000,

[combined_W4000N100, combined_W4000N2000, combined_W4000N3900],

"WR4000 Comparison; DeltaR(q,q); A.U."

)

# 저장 및 출력

# X축 범위 제한

  
  
  
  
  
  
  
  
  

output_file.cd()

combined_W1000N100.Write()

combined_W1000N500.Write()

combined_W1000N900.Write()

  

combined_W2000N100.Write()

combined_W2000N1000.Write()

combined_W2000N1900.Write()

  

combined_W4000N100.Write()

combined_W4000N2000.Write()

combined_W4000N3900.Write()

  

canvas_WR1000.Write("canvas_WR1000")

canvas_WR2000.Write("canvas_WR2000")

canvas_WR4000.Write("canvas_WR4000")

  

canvas_WR1000.SaveAs("canvas_WR1000.png")

canvas_WR2000.SaveAs("canvas_WR2000.png")

canvas_WR4000.SaveAs("canvas_WR4000.png")

  

output_file.Close()

  
  
  

print("Histograms are saved and comparison plots are generated.")
```