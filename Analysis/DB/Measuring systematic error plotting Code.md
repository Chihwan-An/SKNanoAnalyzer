```python
from ROOT import *

from ROOT import TFile

import matplotlib.pyplot as plt

import numpy as np

  

file = TFile("WR1000_N300_card_input.root")

bkg_central = file.Get("bkg_SR")

sig_central = file.Get("signal_SR")

fake_central = file.Get("fake_SR")

  

# Signal 히스토그램 나누기

sig_ElectronEn_up = file.Get("signal_SR_ElectronEn_2018Up")

sig_ElectronEn_down = file.Get("signal_SR_ElectronEn_2018Down")

sig_ElectronIDSFUp = file.Get("signal_SR_ElectronIDSFUp")

sig_ElectronIDSFdown = file.Get("signal_SR_ElectronIDSFDown")

sig_ElectronRes_up = file.Get("signal_SR_ElectronRes_2018Up")

sig_ElectronRes_down = file.Get("signal_SR_ElectronRes_2018Down")

sig_JetEn_up = file.Get("signal_SR_JetEn_2018Up")

sig_JetEn_down = file.Get("signal_SR_JetEn_2018Down")

sig_JetRes_up = file.Get("signal_SR_JetRes_2018Up")

sig_JetRes_down = file.Get("signal_SR_JetRes_2018Down")

sig_MuonEn_up = file.Get("signal_SR_MuonEn_2018Up")

sig_MuonEn_down = file.Get("signal_SR_MuonEn_2018Down")

sig_MuonIDSF_up = file.Get("signal_SR_MuonIDSFUp")

sig_MuonIDSF_down = file.Get("signal_SR_MuonIDSFDown")

sig_MuonISOSF_up = file.Get("signal_SR_MuonISOSFUp")

sig_MuonISOSF_down = file.Get("signal_SR_MuonISOSFDown")

sig_PU_up = file.Get("signal_SR_PU_2018Up")

sig_PU_down = file.Get("signal_SR_PU_2018Down")

sig_Prefire_up = file.Get("signal_SR_Prefire_2018Up")

sig_Prefire_down = file.Get("signal_SR_Prefire_2018Down")

sig_TauEn_up = file.Get("signal_SR_TauEn_2018Up")

sig_TauEn_down = file.Get("signal_SR_TauEn_2018Down")

sig_TauIDSFExt_up = file.Get("signal_SR_TauIDSFExtUp")

sig_TauIDSFExt_down = file.Get("signal_SR_TauIDSFExtDown")

sig_TauIDSFStat_up = file.Get("signal_SR_TauIDSFStat_2018Up")

sig_TauIDSFStat_down = file.Get("signal_SR_TauIDSFStat_2018Down")

sig_TauIDSFSyst_up = file.Get("signal_SR_TauIDSFSyst_2018Up")

sig_TauIDSFSyst_down = file.Get("signal_SR_TauIDSFSyst_2018Down")

sig_TauTriggerSF_up = file.Get("signal_SR_TauTriggerSF_2018Up")

sig_TauTriggerSF_down = file.Get("signal_SR_TauTriggerSF_2018Down")

sig_Scale_up = file.Get("signal_SR_ScaleUp")

sig_Scale_down = file.Get("signal_SR_ScaleDown")

  

# Background 히스토그램 나누기

bkg_ElectronEn_up = file.Get("bkg_SR_ElectronEn_2018Up")

bkg_ElectronEn_down = file.Get("bkg_SR_ElectronEn_2018Down")

bkg_ElectronIDSFup = file.Get("bkg_SR_ElectronIDSFUp")

bkg_ElectronIDSFdown = file.Get("bkg_SR_ElectronIDSFDown")

bkg_ElectronRes_up = file.Get("bkg_SR_ElectronRes_2018Up")

bkg_ElectronRes_down = file.Get("bkg_SR_ElectronRes_2018Down")

bkg_JetEn_up = file.Get("bkg_SR_JetEn_2018Up")

bkg_JetEn_down = file.Get("bkg_SR_JetEn_2018Down")

bkg_JetRes_up = file.Get("bkg_SR_JetRes_2018Up")

bkg_JetRes_down = file.Get("bkg_SR_JetRes_2018Down")

bkg_MuonEn_up = file.Get("bkg_SR_MuonEn_2018Up")

bkg_MuonEn_down = file.Get("bkg_SR_MuonEn_2018Down")

bkg_MuonIDSF_up = file.Get("bkg_SR_MuonIDSFUp")

bkg_MuonIDSF_down = file.Get("bkg_SR_MuonIDSFDown")

bkg_MuonISOSF_up = file.Get("bkg_SR_MuonISOSFUp")

bkg_MuonISOSF_down = file.Get("bkg_SR_MuonISOSFDown")

bkg_PU_up = file.Get("bkg_SR_PU_2018Up")

bkg_PU_down = file.Get("bkg_SR_PU_2018Down")

bkg_Prefire_up = file.Get("bkg_SR_Prefire_2018Up")

bkg_Prefire_down = file.Get("bkg_SR_Prefire_2018Down")

bkg_TauEn_up = file.Get("bkg_SR_TauEn_2018Up")

bkg_TauEn_down = file.Get("bkg_SR_TauEn_2018Down")

bkg_TauIDSFExt_up = file.Get("bkg_SR_TauIDSFExtUp")

bkg_TauIDSFExt_down = file.Get("bkg_SR_TauIDSFExtDown")

bkg_TauIDSFStat_up = file.Get("bkg_SR_TauIDSFStat_2018Up")

bkg_TauIDSFStat_down = file.Get("bkg_SR_TauIDSFStat_2018Down")

bkg_TauIDSFSyst_up = file.Get("bkg_SR_TauIDSFSyst_2018Up")

bkg_TauIDSFSyst_down = file.Get("bkg_SR_TauIDSFSyst_2018Down")

bkg_TauTriggerSF_up = file.Get("bkg_SR_TauTriggerSF_2018Up")

bkg_TauTriggerSF_down = file.Get("bkg_SR_TauTriggerSF_2018Down")

  

# fake

fake_up = file.Get("fake_SR_TauFRErr_2018Up")

fake_down = file.Get("fake_SR_TauFRErr_2018Down")

  
  

# Signal 히스토그램 수정

  

sig_ElectronEn_up.Divide(sig_central)

sig_ElectronEn_down.Divide(sig_central)

sig_ElectronIDSFUp.Divide(sig_central)

sig_ElectronIDSFdown.Divide(sig_central)

sig_ElectronRes_up.Divide(sig_central)

sig_ElectronRes_down.Divide(sig_central)

sig_JetEn_up.Divide(sig_central)

sig_JetEn_down .Divide(sig_central)

sig_JetRes_up .Divide(sig_central)

sig_JetRes_down .Divide(sig_central)

sig_MuonEn_up .Divide(sig_central)

sig_MuonEn_down .Divide(sig_central)

sig_MuonIDSF_up .Divide(sig_central)

sig_MuonIDSF_down .Divide(sig_central)

sig_MuonISOSF_up .Divide(sig_central)

sig_MuonISOSF_down .Divide(sig_central)

sig_PU_up .Divide(sig_central)

sig_PU_down .Divide(sig_central)

sig_Prefire_up .Divide(sig_central)

sig_Prefire_down .Divide(sig_central)

sig_TauEn_up .Divide(sig_central)

sig_TauEn_down .Divide(sig_central)

sig_TauIDSFExt_up .Divide(sig_central)

sig_TauIDSFExt_down .Divide(sig_central)

sig_TauIDSFStat_up .Divide(sig_central)

sig_TauIDSFStat_down .Divide(sig_central)

sig_TauIDSFSyst_up .Divide(sig_central)

sig_TauIDSFSyst_down .Divide(sig_central)

sig_TauTriggerSF_up .Divide(sig_central)

sig_TauTriggerSF_down .Divide(sig_central)

sig_Scale_up .Divide(sig_central)

sig_Scale_down .Divide(sig_central)

  

# Background 히스토그램 수정

bkg_ElectronEn_up.Divide(bkg_central)

bkg_ElectronEn_up.Divide(bkg_central)

bkg_ElectronEn_down.Divide(bkg_central)

bkg_ElectronIDSFup.Divide(bkg_central)

bkg_ElectronIDSFdown.Divide(bkg_central)

bkg_ElectronRes_up.Divide(bkg_central)

bkg_ElectronRes_down.Divide(bkg_central)

bkg_JetEn_up.Divide(bkg_central)

bkg_JetEn_down .Divide(bkg_central)

bkg_JetRes_up .Divide(bkg_central)

bkg_JetRes_down .Divide(bkg_central)

bkg_MuonEn_up .Divide(bkg_central)

bkg_MuonEn_down .Divide(bkg_central)

bkg_MuonIDSF_up .Divide(bkg_central)

bkg_MuonIDSF_down .Divide(bkg_central)

bkg_MuonISOSF_up .Divide(bkg_central)

bkg_MuonISOSF_down .Divide(bkg_central)

bkg_PU_up .Divide(bkg_central)

bkg_PU_down .Divide(bkg_central)

bkg_Prefire_up .Divide(bkg_central)

bkg_Prefire_down .Divide(bkg_central)

bkg_TauEn_up .Divide(bkg_central)

bkg_TauEn_down .Divide(bkg_central)

bkg_TauIDSFExt_up .Divide(bkg_central)

bkg_TauIDSFExt_down .Divide(bkg_central)

bkg_TauIDSFStat_up .Divide(bkg_central)

bkg_TauIDSFStat_down .Divide(bkg_central)

bkg_TauIDSFSyst_up .Divide(bkg_central)

bkg_TauIDSFSyst_down .Divide(bkg_central)

bkg_TauTriggerSF_up .Divide(bkg_central)

bkg_TauTriggerSF_down .Divide(bkg_central)

  

#fake

fake_down.Divide(fake_central)

fake_up.Divide(fake_central)

  
  
  

print ("Ok1")

  
  

# plot

  

bins = [1, 2, 3,4] # bin 번호를 Python 리스트로 정의

values_sig_ElectronEn_up = [sig_ElectronEn_up.GetBinContent(i) for i in bins] # 변환 불필요

values_sig_ElectronEn_down = [sig_ElectronEn_down.GetBinContent(i) for i in bins] # 변환 불필요

values_sig_ElectronIDSFUp = [sig_ElectronIDSFUp.GetBinContent(i) for i in bins] # 변환 불필요

values_sig_ElectronIDSFdown = [sig_ElectronIDSFdown.GetBinContent(i) for i in bins] # 변환 불필요

values_sig_ElectronRes_up = [sig_ElectronRes_up.GetBinContent(i) for i in bins] # 변환 불필요

values_sig_ElectronRes_down = [sig_ElectronRes_down.GetBinContent(i) for i in bins] # 변환 불필요

values_sig_JetEn_up = [sig_JetEn_up.GetBinContent(i) for i in bins] # 변환 불필요

values_sig_JetEn_down = [sig_JetEn_down.GetBinContent(i) for i in bins] # 변환 불필요

values_sig_JetRes_up = [sig_JetRes_up.GetBinContent(i) for i in bins] # 변환 불필요

values_sig_JetRes_down = [sig_JetRes_down.GetBinContent(i) for i in bins] # 변환 불필요

values_sig_MuonEn_up = [sig_MuonEn_up.GetBinContent(i) for i in bins] # 변환 불필요

values_sig_MuonEn_down = [sig_MuonEn_down.GetBinContent(i) for i in bins] # 변환 불필요

values_sig_MuonIDSF_up = [sig_MuonIDSF_up.GetBinContent(i) for i in bins]

values_sig_MuonIDSF_down = [sig_MuonIDSF_down.GetBinContent(i) for i in bins]

values_sig_MuonISOSF_up = [sig_MuonISOSF_up.GetBinContent(i) for i in bins]

values_sig_MuonISOSF_down = [sig_MuonISOSF_down.GetBinContent(i) for i in bins]

values_sig_PU_up = [sig_PU_up.GetBinContent(i) for i in bins]

values_sig_PU_down = [sig_PU_down.GetBinContent(i) for i in bins]

values_sig_Prefire_up = [sig_Prefire_up.GetBinContent(i) for i in bins]

values_sig_Prefire_down = [sig_Prefire_down.GetBinContent(i) for i in bins]

values_sig_TauEn_up = [sig_TauEn_up.GetBinContent(i) for i in bins]

values_sig_TauEn_down = [sig_TauEn_down.GetBinContent(i) for i in bins]

values_sig_TauIDSFExt_up = [sig_TauIDSFExt_up.GetBinContent(i) for i in bins]

values_sig_TauIDSFExt_down = [sig_TauIDSFExt_up.GetBinContent(i) for i in bins]

values_sig_TauIDSFStat_up = [sig_TauIDSFExt_down.GetBinContent(i) for i in bins]

values_sig_TauIDSFStat_down = [sig_TauIDSFStat_down.GetBinContent(i) for i in bins]

values_sig_TauIDSFSyst_up = [sig_TauIDSFSyst_up.GetBinContent(i) for i in bins]

values_sig_TauIDSFSyst_down = [sig_TauIDSFSyst_down.GetBinContent(i) for i in bins]

values_sig_TauTriggerSF_up = [sig_TauTriggerSF_up.GetBinContent(i) for i in bins]

values_sig_TauTriggerSF_down = [sig_TauTriggerSF_down.GetBinContent(i) for i in bins]

values_sig_Scale_up = [sig_Scale_up.GetBinContent(i) for i in bins]

values_sig_Scale_down = [sig_Scale_down.GetBinContent(i) for i in bins]

  

values_bkg_ElectronEn_up = [bkg_ElectronEn_up.GetBinContent(i) for i in bins] # 변환 불필요

values_bkg_ElectronEn_down = [bkg_ElectronEn_down.GetBinContent(i) for i in bins] # 변환 불필요

values_bkg_ElectronIDSFUp = [bkg_ElectronIDSFup.GetBinContent(i) for i in bins] # 변환 불필요

values_bkg_ElectronIDSFdown = [bkg_ElectronIDSFdown.GetBinContent(i) for i in bins] # 변환 불필요

values_bkg_ElectronRes_up = [bkg_ElectronRes_up.GetBinContent(i) for i in bins] # 변환 불필요

values_bkg_ElectronRes_down = [bkg_ElectronRes_down.GetBinContent(i) for i in bins] # 변환 불필요

values_bkg_JetEn_up = [bkg_JetEn_up.GetBinContent(i) for i in bins] # 변환 불필요

values_bkg_JetEn_down = [bkg_JetEn_down.GetBinContent(i) for i in bins] # 변환 불필요

values_bkg_JetRes_up = [bkg_JetRes_up.GetBinContent(i) for i in bins] # 변환 불필요

values_bkg_JetRes_down = [bkg_JetRes_down.GetBinContent(i) for i in bins] # 변환 불필요

values_bkg_MuonEn_up = [bkg_MuonEn_up.GetBinContent(i) for i in bins] # 변환 불필요

values_bkg_MuonEn_down = [bkg_MuonEn_down.GetBinContent(i) for i in bins] # 변환 불필요

values_bkg_MuonIDSF_up = [bkg_MuonIDSF_up.GetBinContent(i) for i in bins]

values_bkg_MuonIDSF_down = [bkg_MuonIDSF_down.GetBinContent(i) for i in bins]

values_bkg_MuonISOSF_up = [bkg_MuonISOSF_up.GetBinContent(i) for i in bins]

values_bkg_MuonISOSF_down = [bkg_MuonISOSF_down.GetBinContent(i) for i in bins]

values_bkg_PU_up = [bkg_PU_up.GetBinContent(i) for i in bins]

values_bkg_PU_down = [bkg_PU_down.GetBinContent(i) for i in bins]

values_bkg_Prefire_up = [bkg_Prefire_up.GetBinContent(i) for i in bins]

values_bkg_Prefire_down = [bkg_Prefire_down.GetBinContent(i) for i in bins]

values_bkg_TauEn_up = [bkg_TauEn_up.GetBinContent(i) for i in bins]

values_bkg_TauEn_down = [bkg_TauEn_down.GetBinContent(i) for i in bins]

values_bkg_TauIDSFExt_up = [bkg_TauIDSFExt_up.GetBinContent(i) for i in bins]

values_bkg_TauIDSFExt_down = [bkg_TauIDSFExt_up.GetBinContent(i) for i in bins]

values_bkg_TauIDSFStat_up = [bkg_TauIDSFExt_down.GetBinContent(i) for i in bins]

values_bkg_TauIDSFStat_down = [bkg_TauIDSFStat_down.GetBinContent(i) for i in bins]

values_bkg_TauIDSFSyst_up = [bkg_TauIDSFSyst_up.GetBinContent(i) for i in bins]

values_bkg_TauIDSFSyst_down = [bkg_TauIDSFSyst_down.GetBinContent(i) for i in bins]

values_bkg_TauTriggerSF_up = [bkg_TauTriggerSF_up.GetBinContent(i) for i in bins]

values_bkg_TauTriggerSF_down = [bkg_TauTriggerSF_down.GetBinContent(i) for i in bins]

  

values_fake_up = [fake_up.GetBinContent(i) for i in bins]

values_fake_down = [fake_down.GetBinContent(i) for i in bins]

  

print (sig_ElectronRes_up[1],sig_ElectronRes_up[2],sig_ElectronRes_up[3])

print (sig_ElectronRes_down[1],sig_ElectronRes_down[2],sig_ElectronRes_down[3])

print (sig_TauIDSFExt_up)

  

x = np.linspace(900, 3000, 500) # 900에서 3000까지 500개의 점 생성

def sig_plot_variable(values_up, values_down, label_base):

global x

y = np.zeros_like(x)

# 새로운 그림 생성

plt.figure(figsize=(8, 6))

  

# up 값을 플롯

y[(x >= 900) & (x < 1050)] = values_up[0]

y[(x >= 1050) & (x <= 1250)] = values_up[1]

y[(x >= 1250) & (x <= 1700)] = values_up[2]

y[(x >= 1700) & (x <= 3000)] = values_up[3]

plt.plot(x, y, label=f"{label_base} up", linestyle='--')

  

# down 값을 플롯

y[(x >= 900) & (x < 1050)] = values_down[0]

y[(x >= 1050) & (x <= 1250)] = values_down[1]

y[(x >= 1250) & (x <= 1700)] = values_down[2]

y[(x >= 1700) & (x <= 3000)] = values_down[3]

plt.plot(x, y, label=f"{label_base} down", linestyle='--')

plt.axhline(1, linestyle = '--',color = 'black')

# 플롯 설정

plt.xlabel("X-axis")

plt.ylabel("Var/Norm")

plt.ylim(0.8,1.2)

plt.title(f"{label_base}_sig_Systematic Uncertainty")

plt.legend()

plt.grid(True)

  

# 저장

filename = f"{label_base}_sig_systematic_uncertainty.png"

plt.savefig(filename, dpi=300)

plt.close() # 플롯 닫기

print(f"Saved: {filename}")

  

def bkg_plot_variable(values_up, values_down, label_base):

global x

y = np.zeros_like(x)

# 새로운 그림 생성

plt.figure(figsize=(8, 6))

  

# up 값을 플롯

y[(x >= 900) & (x < 1050)] = values_up[0]

y[(x >= 1050) & (x <= 1250)] = values_up[1]

y[(x >= 1250) & (x <= 1700)] = values_up[2]

y[(x >= 1700) & (x <= 3000)] = values_up[3]

plt.plot(x, y, label=f"{label_base} up", linestyle='--')

# down 값을 플롯

y[(x >= 900) & (x < 1050)] = values_down[0]

y[(x >= 1050) & (x <= 1250)] = values_down[1]

y[(x >= 1250) & (x <= 1700)] = values_down[2]

y[(x >= 1700) & (x <= 3000)] = values_down[3]

plt.plot(x, y, label=f"{label_base} down", linestyle='--')

plt.axhline(1, linestyle = '--',color = 'black')

# 플롯 설정

plt.xlabel("X-axis")

plt.ylabel("Var/Norm")

plt.ylim(0.8,1.2)

plt.title(f"{label_base}_bkg_Systematic Uncertainty")

plt.legend()

plt.grid(True)

  

# 저장

filename = f"{label_base}_bkg_systematic_uncertainty.png"

plt.savefig(filename, dpi=300)

plt.close() # 플롯 닫기

print(f"Saved: {filename}")

  

def fake_plot_variable(values_up, values_down, label_base):

global x

y = np.zeros_like(x)

# 새로운 그림 생성

plt.figure(figsize=(8, 6))

  

# up 값을 플롯

y[(x >= 900) & (x < 1050)] = values_up[0]

y[(x >= 1050) & (x <= 1250)] = values_up[1]

y[(x >= 1250) & (x <= 1700)] = values_up[2]

y[(x >= 1700) & (x <= 3000)] = values_up[3]

plt.plot(x, y, label=f"{label_base} up", linestyle='--')

# down 값을 플롯

y[(x >= 900) & (x < 1050)] = values_down[0]

y[(x >= 1050) & (x <= 1250)] = values_down[1]

y[(x >= 1250) & (x <= 1700)] = values_down[2]

y[(x >= 1700) & (x <= 3000)] = values_down[3]

plt.plot(x, y, label=f"{label_base} down", linestyle='--')

plt.axhline(1, linestyle = '--',color = 'black')

# 플롯 설정

plt.xlabel("X-axis")

plt.ylabel("Var/Norm")

plt.ylim(0.8,1.2)

plt.title(f"{label_base}_fake_Systematic Uncertainty")

plt.legend()

plt.grid(True)

  

# 저장

filename = f"{label_base}_fake_systematic_uncertainty.png"

plt.savefig(filename, dpi=300)

plt.close() # 플롯 닫기

print(f"Saved: {filename}")

# 플롯할 변수 정의

variables_sig = {

"ElectronIDSF": (values_sig_ElectronIDSFUp, values_sig_ElectronIDSFdown),

"ElectronRes": (values_sig_ElectronRes_up, values_sig_ElectronRes_down),

"JetEn": (values_sig_JetEn_up, values_sig_JetEn_down),

"JetRes": (values_sig_JetRes_up, values_sig_JetRes_down),

"MuonEn": (values_sig_MuonEn_up, values_sig_MuonEn_down),

"MuonIDSF": (values_sig_MuonIDSF_up, values_sig_MuonIDSF_down),

"MuonISOSF": (values_sig_MuonISOSF_up, values_sig_MuonISOSF_down),

"PU": (values_sig_PU_up, values_sig_PU_down),

"Prefire": (values_sig_Prefire_up, values_sig_Prefire_down),

"TauEn": (values_sig_TauEn_up, values_sig_TauEn_down),

"TauIDSFExt": (values_sig_TauIDSFExt_up, values_sig_TauIDSFExt_down),

"TauIDSFStat": (values_sig_TauIDSFStat_up, values_sig_TauIDSFStat_down),

"TauIDSFSyst": (values_sig_TauIDSFSyst_up, values_sig_TauIDSFSyst_down),

"TauTriggerSF": (values_sig_TauTriggerSF_up, values_sig_TauTriggerSF_down),

"Scale": (values_sig_Scale_up, values_sig_Scale_down)

}

variables_bkg = {

"ElectronIDSF": (values_bkg_ElectronIDSFUp, values_bkg_ElectronIDSFdown),

"ElectronRes": (values_bkg_ElectronRes_up, values_bkg_ElectronRes_down),

"JetEn": (values_bkg_JetEn_up, values_bkg_JetEn_down),

"JetRes": (values_bkg_JetRes_up, values_bkg_JetRes_down),

"MuonEn": (values_bkg_MuonEn_up, values_bkg_MuonEn_down),

"MuonIDSF": (values_bkg_MuonIDSF_up, values_bkg_MuonIDSF_down),

"MuonISOSF": (values_bkg_MuonISOSF_up, values_bkg_MuonISOSF_down),

"PU": (values_bkg_PU_up, values_bkg_PU_down),

"Prefire": (values_bkg_Prefire_up, values_bkg_Prefire_down),

"TauEn": (values_bkg_TauEn_up, values_bkg_TauEn_down),

"TauIDSFExt": (values_bkg_TauIDSFExt_up, values_bkg_TauIDSFExt_down),

"TauIDSFStat": (values_bkg_TauIDSFStat_up, values_bkg_TauIDSFStat_down),

"TauIDSFSyst": (values_bkg_TauIDSFSyst_up, values_bkg_TauIDSFSyst_down),

"TauTriggerSF": (values_bkg_TauTriggerSF_up, values_bkg_TauTriggerSF_down)

}

  

variables_fake = {

"Fake": (values_fake_up, values_fake_down)

}

for label, (values_up, values_down) in variables_sig.items():

sig_plot_variable(values_up, values_down, label)

  

for label, (values_up, values_down) in variables_bkg.items():

bkg_plot_variable(values_up, values_down, label)

  

for label, (values_up, values_down) in variables_fake.items():

fake_plot_variable(values_up, values_down, label)
```