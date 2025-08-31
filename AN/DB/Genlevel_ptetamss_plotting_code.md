1. *Using electron and muon*
```python
import os

from ROOT import TCanvas, TH1D, TFile, TLegend, TLatex

from ROOT import kRed, kGreen, kAzure, gStyle

  

# (1) mWR -> [mN 리스트], 선 색깔 리스트 등 기존 그대로 사용

d_mp = {1000:[100,500,900], 2000:[100,1000,1900], 4000:[100,2000,3900]}

l_col = [kRed+1, kGreen+2, kAzure+2]

  

# (2) 새로 정의한 사전

d_plots = {

"tau_cut": ["pt","eta"],

"jetAK4_cut": ["pt","eta"],

"jetAK4_cut_pt_subleading":["pT"],

"jetAK4_cut_eta_subleading":["etas"],

"jetAK8_cut":["pt","eta"],

  

"WR4_cut_pt": ["el_mu"],

"WR4_cut_eta": ["el_mu"],

"N4_cut_pt": ["el_mu"],

"N4_cut_eta": ["el_mu"],

"WR8_cut": ["ptpt","etaeta"],

  

"tau_cut_mass_AK4": ["el_mu"],

"tau_cut_mass_AK8_tau": ["q"],

"cut_mass_AK4": ["el_mu"],

}

  

# 예: 히스토그램 파일들이 있는 디렉토리 (사용 상황에 맞게 수정)

dirname = "/data6/Users/snuintern1/nano3/CMSSW_10_6_22/src/PhysicsTools/NanoAODTools/python/postprocessing/analyzer/analyzer/Genlevel_selection/result/"

  

################################################################################

# 메인 루프

################################################################################

for obj in d_plots: # 새 dictionary에서 "오브젝트"를 반복

for var in d_plots[obj]: # 오브젝트별로 [pt,eta], 또는 [mass] 등 반복

for mWR in d_mp: # mWR 후보들 (1000, 2000, 4000)

# 결과 저장 경로: 기존처럼 mWR별 폴더만 만든 예시

outdir = f"../fig/signalsample/WR{mWR}"

os.system(f"mkdir -p {outdir}")

  

# 캔버스, 레전드 설정

c = TCanvas(f"{obj}_{var}_WR{mWR}", "", 1000, 1000)

l = TLegend(0.5, 0.675, 0.85, 0.875)

latex = TLatex()

latex.SetNDC()

l.SetFillStyle(0)

l.SetBorderSize(0)

  

# 두 번 그리기 위해 먼저 최대 bin 높이를 구하기

idx = 0

maxy = []

for mN in d_mp[mWR]:

fpath = f"{dirname}/WR{mWR}N{mN}.root"

f = TFile(f"{dirname}/WR{mWR}N{mN}.root")

# h = plots/ + obj + _ + var

# 예: h = f.Get("plots/tau_cut_leading_pt")

if var == "pt" or var == "eta":

h = f.Get(f"plots/{obj}_{var}_leading")

if var == "pT"or var == "etas" or var == "q":

h = f.Get(f"plots/{obj}")

if var == "el_mu":

h = f.Get(f"plots/{obj}_el")

hmu = f.Get(f"plots/{obj}_mu")

h.Add(hmu)

if var == "ptpt" :

h = f.Get(f"plots/{obj}_pt")

if var== "etaeta":

h = f.Get(f"plots/{obj}_eta")

  

if obj == "tau_cut":

if var == "pt":

h.GetXaxis().SetTitle("p_{T} of #tau")

h.SetTitle("")

h.Rebin(10)

h.GetXaxis().SetRangeUser(0, mWR*0.8)

if var == "eta":

h.GetXaxis().SetTitle("#eta of #tau")

h.SetTitle("")

h.GetXaxis().SetRangeUser(-4, 4)

h.Rebin(5)

if obj == "jetAK4_cut":

#h.SetTitle(f"Genlevel leading AK4 Jet {var}")

if var == "pt":

h.GetXaxis().SetTitle("p_{T} of j_{leading,AK4}")

h.Rebin(10)

h.SetTitle("")

h.GetXaxis().SetRangeUser(0, mWR)

#h.SetTitle(f"Genlevel leading AK4 Jet {var}")

if var == "eta":

h.GetXaxis().SetTitle("#eta of j_{leading,AK4}")

h.Rebin(5)

h.SetTitle("")

h.GetXaxis().SetRangeUser(-4, 4)

if obj == "jetAK4_cut_pt_subleading":

#h.SetTitle("Genlevel subleading AK4 Jet p_{T}")

h.GetXaxis().SetTitle("p_{T} of j_{subleading,AK4}")

h.Rebin(10)

h.SetTitle("")

h.GetXaxis().SetRangeUser(0, mWR*0.6)

if obj == "jetAK4_cut_eta_subleading":

#h.SetTitle("Genlevel subleading AK4 Jet #eta")

h.GetXaxis().SetTitle("#eta of j_{subleading,AK4}")

h.Rebin(5)

h.SetTitle("")

h.GetXaxis().SetRangeUser(-4, 4)

if obj == "jetAK8_cut":

#h.SetTitle(f"Genlevel AK8 Jet {var}")

if var == "pt":

h.GetXaxis().SetTitle("p_{T} of j_{AK8}")

h.Rebin(10)

h.SetTitle("")

h.GetXaxis().SetRangeUser(0, mWR)

#h.SetTitle(f"Genlevel AK8 Jet {var}")

if var == "eta":

h.GetXaxis().SetTitle("#eta of j_{AK8}")

h.Rebin(5)

h.SetTitle("")

h.GetXaxis().SetRangeUser(-4, 4)

if obj == "WR4_cut_pt":

#h.SetTitle("Genlevel W_{R} p_{T} with AK4 jet")

h.GetXaxis().SetTitle("p_{T} of #tau_{pri}#tau_{sec}jj_{AK4}")

h.Rebin(10)

h.SetTitle("")

h.GetXaxis().SetRangeUser(0, mWR)

if obj == "WR4_cut_eta":

#h.SetTitle("Genlevel #eta_{W_{R}} with AK4 jet")

h.GetXaxis().SetTitle("#eta of #tau_{pri}#tau_{sec}jj_{AK4}")

h.Rebin(5)

h.SetTitle("")

if obj == "N4_cut_pt":

#h.SetTitle("Genlevel N p_{T} with AK4 jet")

h.GetXaxis().SetTitle("p_{T} of #tau_{sec}jj_{AK4}")

h.Rebin(10)

h.GetXaxis().SetRangeUser(0, mWR*0.8)

h.SetTitle("")

if obj == "N4_cut_eta":

#h.SetTitle("Genlevel #eta_{N} with AK4 jet")

h.GetXaxis().SetTitle("#eta of #tau_{sec}jj_{AK4}")

h.Rebin(5)

h.SetTitle("")

if obj == "WR8_cut":

if var == "ptpt":

#h.SetTitle("Genlevel W_{R} p_{T} with AK8 jet")

h.GetXaxis().SetTitle("p_{T} of j_{AK8}")

h.Rebin(10)

h.SetTitle("")

h.GetXaxis().SetRangeUser(0, mWR*0.8)

if var == "etaeta":

#h.SetTitle("Genlevel #eta_{W_{R}} with AK8 jet")

h.GetXaxis().SetTitle("#eta of j_{AK8}")

h.Rebin(5)

h.SetTitle("")

if obj == "tau_cut_mass_AK4":

#h.SetTitle("Genlevel M_{W_{R}} with AK4 jet")

h.GetXaxis().SetTitle("m(#tau_{pri}#tau_{sec}jj_{AK4})")

h.Rebin(25)

h.GetXaxis().SetRangeUser(0, mWR*2)

h.SetTitle("")

if obj == "tau_cut_mass_AK8_tau":

#h.SetTitle("Genlevel M_{W_{R}} with AK8 jet")

h.GetXaxis().SetTitle("m(#tau_{pri}j_{AK8})")

h.Rebin(25)

h.GetXaxis().SetRangeUser(0, mWR*1.5)

h.SetTitle("")

if obj == "cut_mass_AK4":

#h.SetTitle("Genlevel M_{N} with AK4 jet")

h.GetXaxis().SetTitle("m(#tau_{sec}jj_{AK4})")

h.Rebin(25)

h.SetTitle("")

h.GetXaxis().SetRangeUser(0, mWR*1.5)

  
  

  

  

# 안전장치: 혹시 hist가 없으면 continue

if not h:

print(f"Cannot find histogram: plots/{obj}_{var} in file {dirname}/WR{mWR}N{mN}.root")

continue

  

# 정규화

h.Scale(1./h.Integral())

  
  

# 최대 bin 저장

maxy.append(h.GetMaximum())

  

####################################################################

# 실제 그리기(두 번째 루프)

####################################################################

idx = 0

for mN in d_mp[mWR]:

fpath = f"{dirname}/WR{mWR}N{mN}.root"

f = TFile(f"{dirname}/WR{mWR}N{mN}.root")

if var == "pt" or var == "eta":

h = f.Get(f"plots/{obj}_{var}_leading")

if var == "pT" or var == "etas" or var == "q":

h = f.Get(f"plots/{obj}")

if var == "el_mu":

h = f.Get(f"plots/{obj}_el")

hmu = f.Get(f"plots/{obj}_mu")

h.Add(hmu)

if var == "ptpt" :

h = f.Get(f"plots/{obj}_pt")

if var== "etaeta":

h = f.Get(f"plots/{obj}_eta")

if var == "AK4_el" or var == "AK4_mu" or var == "AK8_tau":

h = f.Get(f"plots/{obj}{var}")

  

if not h:

continue

h.GetXaxis().SetTitle("Gev")

h.GetXaxis().SetTitleSize(0.05)

##제목 설정

if obj == "tau_cut":

if var == "pt":

h.GetXaxis().SetTitle("p_{T} of #tau")

h.SetTitle("")

h.Rebin(10)

h.GetXaxis().SetRangeUser(0, mWR*0.8)

if var == "eta":

h.GetXaxis().SetTitle("#eta of #tau")

h.SetTitle("")

h.GetXaxis().SetRangeUser(-4, 4)

h.Rebin(5)

if obj == "jetAK4_cut":

#h.SetTitle(f"Genlevel leading AK4 Jet {var}")

if var == "pt":

h.GetXaxis().SetTitle("p_{T} of leading AK4 jet ")

h.Rebin(10)

h.SetTitle("")

h.GetXaxis().SetRangeUser(0, mWR)

#h.SetTitle(f"Genlevel leading AK4 Jet {var}")

if var == "eta":

h.GetXaxis().SetTitle("#eta of leading AK4 jet")

h.Rebin(5)

h.SetTitle("")

h.GetXaxis().SetRangeUser(-4, 4)

if obj == "jetAK4_cut_pt_subleading":

#h.SetTitle("Genlevel subleading AK4 Jet p_{T}")

h.GetXaxis().SetTitle("p_{T} of subleading AK4 jet")

h.Rebin(10)

h.SetTitle("")

h.GetXaxis().SetRangeUser(0, mWR*0.6)

if obj == "jetAK4_cut_eta_subleading":

#h.SetTitle("Genlevel subleading AK4 Jet #eta")

h.GetXaxis().SetTitle("#eta of subleading AK4 jet")

h.Rebin(5)

h.SetTitle("")

h.GetXaxis().SetRangeUser(-4, 4)

if obj == "jetAK8_cut":

#h.SetTitle(f"Genlevel AK8 Jet {var}")

if var == "pt":

h.GetXaxis().SetTitle("p_{T} of AK8 jet")

h.Rebin(10)

h.SetTitle("")

h.GetXaxis().SetRangeUser(0, mWR)

#h.SetTitle(f"Genlevel AK8 Jet {var}")

if var == "eta":

h.GetXaxis().SetTitle("#eta of AK8 jet")

h.Rebin(5)

h.SetTitle("")

h.GetXaxis().SetRangeUser(-4, 4)

if obj == "WR4_cut_pt":

#h.SetTitle("Genlevel W_{R} p_{T} with AK4 jet")

h.GetXaxis().SetTitle("p_{T} of #tau_{pri}#tau_{sec} AK4 jj")

h.Rebin(10)

h.SetTitle("")

h.GetXaxis().SetRangeUser(0, mWR)

if obj == "WR4_cut_eta":

#h.SetTitle("Genlevel #eta_{W_{R}} with AK4 jet")

h.GetXaxis().SetTitle("#eta of #tau_{pri}#tau_{sec} AK4 jj")

h.Rebin(5)

h.SetTitle("")

if obj == "N4_cut_pt":

#h.SetTitle("Genlevel N p_{T} with AK4 jet")

h.GetXaxis().SetTitle("p_{T} of #tau_{sec} AK4 jj")

h.Rebin(10)

h.GetXaxis().SetRangeUser(0, mWR*0.8)

h.SetTitle("")

if obj == "N4_cut_eta":

#h.SetTitle("Genlevel #eta_{N} with AK4 jet")

h.GetXaxis().SetTitle("#eta of #tau_{sec} AK4 jj")

h.Rebin(5)

h.SetTitle("")

if obj == "WR8_cut":

if var == "ptpt":

#h.SetTitle("Genlevel W_{R} p_{T} with AK8 jet")

h.GetXaxis().SetTitle("p_{T} of #tau_{pri} AK8 jet")

h.Rebin(10)

h.SetTitle("")

h.GetXaxis().SetRangeUser(0, mWR*0.8)

if var == "etaeta":

#h.SetTitle("Genlevel #eta_{W_{R}} with AK8 jet")

h.GetXaxis().SetTitle("#eta of #tau_{pri} AK8 jet")

h.Rebin(5)

h.SetTitle("")

if obj == "tau_cut_mass_AK4":

#h.SetTitle("Genlevel M_{W_{R}} with AK4 jet")

h.GetXaxis().SetTitle("m(#tau_{pri}#tau_{sec})")

h.Rebin(25)

h.GetXaxis().SetRangeUser(0, mWR*2)

h.SetTitle("")

if obj == "tau_cut_mass_AK8_tau":

#h.SetTitle("Genlevel M_{W_{R}} with AK8 jet")

h.GetXaxis().SetTitle("m(#tau_{pri} AK8 jet)")

h.Rebin(25)

h.GetXaxis().SetRangeUser(0, mWR*1.5)

h.SetTitle("")

if obj == "cut_mass_AK4":

#h.SetTitle("Genlevel M_{N} with AK4A jet")

h.GetXaxis().SetTitle("m(#tau_{sec} Ak4 jj)")

h.Rebin(25)

h.SetTitle("")

h.GetXaxis().SetRangeUser(0, mWR*1.5)

h.GetXaxis().SetTitleSize(0.03)

h.GetXaxis().SetTitleOffset(1.5)

  

h.SetDirectory(0)

h.SetStats(0)

h.Scale(1./h.Integral())

  

h.SetLineColor(l_col[idx])

h.SetLineWidth(4)

  

  

# y축 최대값

if maxy:

h.GetYaxis().SetRangeUser(0, max(maxy)*1.5)

h.GetYaxis().SetTitle("A.U.")

h.GetYaxis().SetTitleSize(0.05)

  

# 레전드에 추가

l.AddEntry(h, f"m_{{WR}}={mWR} GeV, m_{{N}}={mN} GeV","l")

  

c.cd()

if idx == 0:

h.Draw("hist")

else:

h.Draw("hist same")

idx += 1

  

####################################################################

# CMS 라벨 붙이기

####################################################################

textSize = 0.5*gStyle.GetPadTopMargin()

latex.SetTextFont(61)

latex.SetTextSize(textSize)

latex.DrawLatex(0.2, 0.8, "CMS")

latex.SetTextFont(52)

latex.SetTextSize(0.75*textSize)

latex.DrawLatex(0.2, 0.76, "Simulation")

  

# 레전드, 마진 등

l.Draw()

c.SetLeftMargin(0.15)

c.SetBottomMargin(0.1)

c.Draw()

  

# 이미지 저장

out_png = f"{outdir}/{obj}_{var}.png"

#out_pdf = f"{outdir}/{obj}_{var}.pdf"

c.SaveAs(out_png)

#c.SaveAs(out_pdf)

print(f"Saved plots: {out_png}")

#out_pdf
```

2. *Using only tau*

```python
import os

from ROOT import TCanvas, TH1D, TFile, TLegend, TLatex

from ROOT import kRed, kGreen, kAzure, gStyle

  

# (1) mWR -> [mN 리스트], 선 색깔 리스트 등 기존 그대로 사용

d_mp = {1000:[100,500,900], 2000:[100,1000,1900], 4000:[100,2000,3900]}

l_col = [kRed+1, kGreen+2, kAzure+2]

  

# (2) 새로 정의한 사전

d_plots = {

"tau_cut": ["pt","eta"],

"jetAK4_cut": ["pt","eta"],

"jetAK4_cut_pt_subleading":["pT"],

"jetAK4_cut_eta_subleading":["etas"],

"jetAK8_cut":["pt","eta"],

  

"WR4_cut_pt": ["."],

"WR4_cut_eta": ["."],

"N4_cut_pt": ["."],

"N4_cut_eta": ["."],

"WR8_cut": ["ptpt","etaeta"],

  

"tau_cut_mass_AK4": ["."],

"tau_cut_mass_AK8_tau": ["q"],

"cut_mass_AK4": ["."],

}

  

# 예: 히스토그램 파일들이 있는 디렉토리 (사용 상황에 맞게 수정)

dirname = "/data6/Users/snuintern1/nano3/CMSSW_10_6_22/src/PhysicsTools/NanoAODTools/python/postprocessing/analyzer/analyzer/Genlevel_selection/result/"

  

################################################################################

# 메인 루프

################################################################################

for obj in d_plots: # 새 dictionary에서 "오브젝트"를 반복

for var in d_plots[obj]: # 오브젝트별로 [pt,eta], 또는 [mass] 등 반복

for mWR in d_mp: # mWR 후보들 (1000, 2000, 4000)

# 결과 저장 경로: 기존처럼 mWR별 폴더만 만든 예시

outdir = f"../fig/signalsample/WR{mWR}"

os.system(f"mkdir -p {outdir}")

  

# 캔버스, 레전드 설정

c = TCanvas(f"{obj}_{var}_WR{mWR}", "", 1000, 1000)

l = TLegend(0.5, 0.675, 0.85, 0.875)

latex = TLatex()

latex.SetNDC()

l.SetFillStyle(0)

l.SetBorderSize(0)

  

# 두 번 그리기 위해 먼저 최대 bin 높이를 구하기

idx = 0

maxy = []

for mN in d_mp[mWR]:

fpath = f"{dirname}/WR{mWR}N{mN}.root"

f = TFile(f"{dirname}/WR{mWR}N{mN}.root")

# h = plots/ + obj + _ + var

# 예: h = f.Get("plots/tau_cut_leading_pt")

if var == "pt" or var == "eta":

h = f.Get(f"plots/{obj}_{var}_leading")

if var == "pT"or var == "etas" or var == "q":

h = f.Get(f"plots/{obj}")

if var == ".":

h = f.Get(f"plots/{obj}")

if var == "ptpt" :

h = f.Get(f"plots/{obj}_pt")

if var== "etaeta":

h = f.Get(f"plots/{obj}_eta")

  

if obj == "tau_cut":

if var == "pt":

h.GetXaxis().SetTitle("p_{T} of #tau")

h.SetTitle("")

if mWR ==1000:

h.Rebin(5)

else:

h.Rebin(10)

h.GetXaxis().SetRangeUser(0, mWR*0.8)

if var == "eta":

h.GetXaxis().SetTitle("#eta of #tau")

h.SetTitle("")

h.GetXaxis().SetRangeUser(-4, 4)

h.Rebin(5)

if obj == "jetAK4_cut":

#h.SetTitle(f"Genlevel leading AK4 Jet {var}")

if var == "pt":

h.GetXaxis().SetTitle("p_{T} of j_{leading,AK4}")

if mWR ==1000:

h.Rebin(5)

else:

h.Rebin(10)

h.SetTitle("")

h.GetXaxis().SetRangeUser(0, mWR)

#h.SetTitle(f"Genlevel leading AK4 Jet {var}")

if var == "eta":

h.GetXaxis().SetTitle("#eta of j_{leading,AK4}")

h.Rebin(5)

h.SetTitle("")

h.GetXaxis().SetRangeUser(-4, 4)

if obj == "jetAK4_cut_pt_subleading":

#h.SetTitle("Genlevel subleading AK4 Jet p_{T}")

h.GetXaxis().SetTitle("p_{T} of j_{subleading,AK4}")

if mWR == 1000:

h.Rebin(5)

else:

h.Rebin(8)

h.SetTitle("")

h.GetXaxis().SetRangeUser(0, mWR*0.6)

if obj == "jetAK4_cut_eta_subleading":

#h.SetTitle("Genlevel subleading AK4 Jet #eta")

h.GetXaxis().SetTitle("#eta of j_{subleading,AK4}")

h.Rebin(5)

h.SetTitle("")

h.GetXaxis().SetRangeUser(-4, 4)

if obj == "jetAK8_cut":

#h.SetTitle(f"Genlevel AK8 Jet {var}")

if var == "pt":

h.GetXaxis().SetTitle("p_{T} of j_{AK8}")

if mWR ==1000:

h.Rebin(5)

else:

h.Rebin(10)

h.SetTitle("")

h.GetXaxis().SetRangeUser(0, mWR)

#h.SetTitle(f"Genlevel AK8 Jet {var}")

if var == "eta":

h.GetXaxis().SetTitle("#eta of j_{AK8}")

h.Rebin(5)

h.SetTitle("")

h.GetXaxis().SetRangeUser(-4, 4)

if obj == "WR4_cut_pt":

#h.SetTitle("Genlevel W_{R} p_{T} with AK4 jet")

h.GetXaxis().SetTitle("p_{T} of #tau_{pri}#tau_{sec}jj_{AK4}")

if mWR ==1000:

h.Rebin(5)

else:

h.Rebin(10)

h.SetTitle("")

h.GetXaxis().SetRangeUser(0, mWR)

if obj == "WR4_cut_eta":

#h.SetTitle("Genlevel #eta_{W_{R}} with AK4 jet")

h.GetXaxis().SetTitle("#eta of #tau_{pri}#tau_{sec}jj_{AK4}")

h.Rebin(5)

h.SetTitle("")

if obj == "N4_cut_pt":

#h.SetTitle("Genlevel N p_{T} with AK4 jet")

h.GetXaxis().SetTitle("p_{T} of #tau_{sec}jj_{AK4}")

if mWR ==1000:

h.Rebin(5)

else:

h.Rebin(10)

h.GetXaxis().SetRangeUser(0, mWR*0.8)

h.SetTitle("")

if obj == "N4_cut_eta":

#h.SetTitle("Genlevel #eta_{N} with AK4 jet")

h.GetXaxis().SetTitle("#eta of #tau_{sec}jj_{AK4}")

h.Rebin(5)

h.SetTitle("")

if obj == "WR8_cut":

if var == "ptpt":

#h.SetTitle("Genlevel W_{R} p_{T} with AK8 jet")

h.GetXaxis().SetTitle("p_{T} of j_{AK8}")

if mWR ==1000:

h.Rebin(5)

else:

h.Rebin(10)

h.SetTitle("")

h.GetXaxis().SetRangeUser(0, mWR*0.8)

if var == "etaeta":

#h.SetTitle("Genlevel #eta_{W_{R}} with AK8 jet")

h.GetXaxis().SetTitle("#eta of j_{AK8}")

h.Rebin(5)

h.SetTitle("")

if obj == "tau_cut_mass_AK4":

#h.SetTitle("Genlevel M_{W_{R}} with AK4 jet")

h.GetXaxis().SetTitle("m(#tau_{pri}#tau_{sec}jj_{AK4})")

h.Rebin(10)

h.GetXaxis().SetRangeUser(0, mWR*2)

h.SetTitle("")

if obj == "tau_cut_mass_AK8_tau":

#h.SetTitle("Genlevel M_{W_{R}} with AK8 jet")

h.GetXaxis().SetTitle("m(#tau_{pri}j_{AK8})")

h.Rebin(10)

h.GetXaxis().SetRangeUser(0, mWR*1.5)

h.SetTitle("")

if obj == "cut_mass_AK4":

#h.SetTitle("Genlevel M_{N} with AK4 jet")

h.GetXaxis().SetTitle("m(#tau_{sec}jj_{AK4})")

h.Rebin(10)

h.SetTitle("")

h.GetXaxis().SetRangeUser(0, mWR*1.5)

  
  

  

  

# 안전장치: 혹시 hist가 없으면 continue

if not h:

print(f"Cannot find histogram: plots/{obj}_{var} in file {dirname}/WR{mWR}N{mN}.root")

continue

  

# 정규화

h.Scale(1./h.Integral())

  
  

# 최대 bin 저장

maxy.append(h.GetMaximum())

  

####################################################################

# 실제 그리기(두 번째 루프)

####################################################################

idx = 0

for mN in d_mp[mWR]:

fpath = f"{dirname}/WR{mWR}N{mN}.root"

f = TFile(f"{dirname}/WR{mWR}N{mN}.root")

if var == "pt" or var == "eta":

h = f.Get(f"plots/{obj}_{var}_leading")

if var == "pT" or var == "etas" or var == "q":

h = f.Get(f"plots/{obj}")

if var == ".":

h = f.Get(f"plots/{obj}")

if var == "ptpt" :

h = f.Get(f"plots/{obj}_pt")

if var== "etaeta":

h = f.Get(f"plots/{obj}_eta")

if var == "AK4_el" or var == "AK4_mu" or var == "AK8_tau":

h = f.Get(f"plots/{obj}{var}")

  

if not h:

continue

h.GetXaxis().SetTitle("Gev")

h.GetXaxis().SetTitleSize(0.05)

##제목 설정

if obj == "tau_cut":

if var == "pt":

h.GetXaxis().SetTitle("p_{T} of #tau")

h.SetTitle("")

if mWR ==1000:

h.Rebin(5)

else:

h.Rebin(10)

h.GetXaxis().SetRangeUser(0, mWR*0.8)

if var == "eta":

h.GetXaxis().SetTitle("#eta of #tau")

h.SetTitle("")

h.GetXaxis().SetRangeUser(-4, 4)

h.Rebin(5)

if obj == "jetAK4_cut":

#h.SetTitle(f"Genlevel leading AK4 Jet {var}")

if var == "pt":

h.GetXaxis().SetTitle("p_{T} of leading AK4 jet ")

if mWR ==1000:

h.Rebin(5)

else:

h.Rebin(10)

h.SetTitle("")

h.GetXaxis().SetRangeUser(0, mWR)

#h.SetTitle(f"Genlevel leading AK4 Jet {var}")

if var == "eta":

h.GetXaxis().SetTitle("#eta of leading AK4 jet")

h.Rebin(5)

h.SetTitle("")

h.GetXaxis().SetRangeUser(-4, 4)

if obj == "jetAK4_cut_pt_subleading":

#h.SetTitle("Genlevel subleading AK4 Jet p_{T}")

h.GetXaxis().SetTitle("p_{T} of subleading AK4 jet")

if mWR == 1000:

h.Rebin(5)

else:

h.Rebin(8)

h.SetTitle("")

h.GetXaxis().SetRangeUser(0, mWR*0.6)

if obj == "jetAK4_cut_eta_subleading":

#h.SetTitle("Genlevel subleading AK4 Jet #eta")

h.GetXaxis().SetTitle("#eta of subleading AK4 jet")

h.Rebin(5)

h.SetTitle("")

h.GetXaxis().SetRangeUser(-4, 4)

if obj == "jetAK8_cut":

#h.SetTitle(f"Genlevel AK8 Jet {var}")

if var == "pt":

h.GetXaxis().SetTitle("p_{T} of AK8 jet")

if mWR ==1000:

h.Rebin(5)

else:

h.Rebin(10)

h.SetTitle("")

h.GetXaxis().SetRangeUser(0, mWR)

#h.SetTitle(f"Genlevel AK8 Jet {var}")

if var == "eta":

h.GetXaxis().SetTitle("#eta of AK8 jet")

h.Rebin(5)

h.SetTitle("")

h.GetXaxis().SetRangeUser(-4, 4)

if obj == "WR4_cut_pt":

#h.SetTitle("Genlevel W_{R} p_{T} with AK4 jet")

h.GetXaxis().SetTitle("p_{T} of #tau_{pri}#tau_{sec} AK4 jj")

if mWR ==1000:

h.Rebin(5)

else:

h.Rebin(10)

h.SetTitle("")

h.GetXaxis().SetRangeUser(0, mWR)

if obj == "WR4_cut_eta":

#h.SetTitle("Genlevel #eta_{W_{R}} with AK4 jet")

h.GetXaxis().SetTitle("#eta of #tau_{pri}#tau_{sec} AK4 jj")

h.Rebin(5)

h.SetTitle("")

if obj == "N4_cut_pt":

#h.SetTitle("Genlevel N p_{T} with AK4 jet")

h.GetXaxis().SetTitle("p_{T} of #tau_{sec} AK4 jj")

if mWR ==1000:

h.Rebin(5)

else:

h.Rebin(10)

h.GetXaxis().SetRangeUser(0, mWR*0.8)

h.SetTitle("")

if obj == "N4_cut_eta":

#h.SetTitle("Genlevel #eta_{N} with AK4 jet")

h.GetXaxis().SetTitle("#eta of #tau_{sec} AK4 jj")

h.Rebin(5)

h.SetTitle("")

if obj == "WR8_cut":

if var == "ptpt":

#h.SetTitle("Genlevel W_{R} p_{T} with AK8 jet")

h.GetXaxis().SetTitle("p_{T} of #tau_{pri} AK8 jet")

if mWR ==1000:

h.Rebin(5)

else:

h.Rebin(10)

h.SetTitle("")

h.GetXaxis().SetRangeUser(0, mWR*0.8)

if var == "etaeta":

#h.SetTitle("Genlevel #eta_{W_{R}} with AK8 jet")

h.GetXaxis().SetTitle("#eta of #tau_{pri} AK8 jet")

h.Rebin(5)

h.SetTitle("")

if obj == "tau_cut_mass_AK4":

#h.SetTitle("Genlevel M_{W_{R}} with AK4 jet")

h.GetXaxis().SetTitle("m(#tau_{pri}#tau_{sec})")

h.Rebin(10)

h.GetXaxis().SetRangeUser(0, mWR*2)

h.SetTitle("")

if obj == "tau_cut_mass_AK8_tau":

#h.SetTitle("Genlevel M_{W_{R}} with AK8 jet")

h.GetXaxis().SetTitle("m(#tau_{pri} AK8 jet)")

h.Rebin(10)

h.GetXaxis().SetRangeUser(0, mWR*1.5)

h.SetTitle("")

if obj == "cut_mass_AK4":

#h.SetTitle("Genlevel M_{N} with AK4A jet")

h.GetXaxis().SetTitle("m(#tau_{sec} Ak4 jj)")

h.Rebin(10)

h.SetTitle("")

h.GetXaxis().SetRangeUser(0, mWR*1.5)

h.GetXaxis().SetTitleSize(0.03)

h.GetXaxis().SetTitleOffset(1.5)

  

h.SetDirectory(0)

h.SetStats(0)

h.Scale(1./h.Integral())

  

h.SetLineColor(l_col[idx])

h.SetLineWidth(4)

  

  

# y축 최대값

if maxy:

h.GetYaxis().SetRangeUser(0, max(maxy)*1.5)

h.GetYaxis().SetTitle("A.U.")

h.GetYaxis().SetTitleSize(0.05)

  

# 레전드에 추가

l.AddEntry(h, f"m_{{WR}}={mWR} GeV, m_{{N}}={mN} GeV","l")

  

c.cd()

if idx == 0:

h.Draw("hist")

else:

h.Draw("hist same")

idx += 1

  

####################################################################

# CMS 라벨 붙이기

####################################################################

textSize = 0.5*gStyle.GetPadTopMargin()

latex.SetTextFont(61)

latex.SetTextSize(textSize)

latex.DrawLatex(0.2, 0.8, "CMS")

latex.SetTextFont(52)

latex.SetTextSize(0.75*textSize)

latex.DrawLatex(0.2, 0.76, "Simulation")

  

# 레전드, 마진 등

l.Draw()

c.SetLeftMargin(0.15)

c.SetBottomMargin(0.1)

c.Draw()

  

# 이미지 저장

out_png = f"{outdir}/{obj}_{var}.png"

#out_pdf = f"{outdir}/{obj}_{var}.pdf"

c.SaveAs(out_png)

#c.SaveAs(out_pdf)

print(f"Saved plots: {out_png}")

#out_pdf
```