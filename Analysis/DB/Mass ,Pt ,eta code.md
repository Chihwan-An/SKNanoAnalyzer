```python
#!/usr/bin/env python

# -*- coding: utf-8 -*-

from PhysicsTools.NanoAODTools.postprocessing.tools import *

from PhysicsTools.NanoAODTools.postprocessing.framework.eventloop import Module

from PhysicsTools.NanoAODTools.postprocessing.framework.datamodel import Collection,Object

from PhysicsTools.NanoAODTools.postprocessing.framework.postprocessor import PostProcessor

#from PhysicsTools.NanoAODTools.postprocessing.analyzer.ID.GenStatus import *

#from PhysicsTools.NanoAODTools.postprocessing.analyzer.AnalyserHelper.AnalyserHelper import *

from importlib import import_module

import os

import sys

import ROOT

import argparse

import linecache

ROOT.PyConfig.IgnoreCommandLineOptions = True

  
  

# PDG ID와 입자 이름 매핑

def FlagIsOn(status,flag_n) :

if status & (1 << (flag_n -1)) : return True

else : return False

  

def isHardProcess(gen) : return FlagIsOn(gen.statusFlags,8)

  
  

class ExampleAnalysis(Module):

def __init__(self):

self.writeHistFile = True

self.gen_histograms = {}

self.lhe_histograms = {}

self.Genpidlist = [] # 클래스 변수로 정의

self.LHEpidlist = [] # 클래스 변수로 정의

  

def beginJob(self, histFile=None, histDirName=None):

Module.beginJob(self, histFile, histDirName)

  
  

def analyze(self, event):

pdg_to_name = {

2: "Up_quark", -1: "Down_antiquark", 34: "W_prime_boson", 9900016: "Heavy_neutrino",

-15: "Tau_antilepton", 22: "Photon", 1: "Down_quark", -2: "Up_antiquark", 21: "Gluon",

111: "Neutral_pion", -16: "Tau_antineutrino", 321: "Kaon_plus", -11: "Positron",

12: "Electron_neutrino", 15: "Tau_lepton", 4: "Charm_quark", -3: "Strange_antiquark",

421: "D0_meson", 16: "Tau_neutrino", -211: "Negative_pion", -34: "W_prime_antiboson",

3: "Strange_quark", 2101: "ud_diquark", 11: "Electron", -12: "Electron_antineutrino",

413: "D_star_plus_meson", -13: "Muon_plus", 14: "Muon_neutrino", -4: "Charm_antiquark",

-423: "D_star0_antiboson", -421: "Anti_D0_meson", 5: "Bottom_quark", -5: "Bottom_antiquark",

511: "B0_meson", -513: "Anti_B_star0_meson", 431: "Ds_plus_meson", -411: "Anti_D_plus_meson",

-511: "Anti_B0_meson", 423: "D_star0_meson", 513: "B_star0_meson", -413: "Anti_D_star_plus_meson",

-431: "Anti_Ds_plus_meson", 211: "Positive_pion", 433: "Ds_star_plus_meson", 13: "Muon",

-14: "Muon_antineutrino", 411: "D_plus_meson", 4122: "Lambda_c_plus_baryon", 2203: "uu_diquark",

221: "Eta_meson", -433: "Ds_star_minus_meson", 5112: "Sigma_b_minus_baryon",

5122: "Lambda_b0_baryon", 311: "Neutral_kaon", 130: "KL0_meson", -323: "Anti_K_star_minus",

223: "Omega_meson", -4222: "Anti_Sigma_c_plus_plus", -4122: "Anti_Lambda_c_plus",

310: "KS0_meson", -4324: "Anti_Xi_c_star0", -4232: "Anti_Xi_c0", 4132: "Xi_c_plus_baryon",

331: "Eta_prime_meson", 521: "B_plus_meson", 2103: "ud_diquark_S1", 4224: "Sigma_c_star_plus_plus",

1103: "uu_diquark_S0", -321: "Kaon_minus", 3203: "us_diquark", 445: "Chi_c2_2P_meson",

323: "K_star_plus_meson", 443: "J_psi", 213: "Rho_plus_meson", -4114: "Anti_Delta_plus_baryon",

113: "Rho0_meson", -523: "Anti_B_star_minus", -521: "B_minus", 441: "Eta_c_meson",

523: "B_star_plus", 5214: "Sigma_b_star_plus", -311: "Anti_K0_meson", 4212: "Sigma_c0_baryon",

4112: "Sigma_c_minus_baryon", 4214: "Sigma_c_star0", -4212: "Anti_Sigma_c0", 4324: "Xi_c_star_plus",

4232: "Xi_c0", 533: "Bs_star0", 531: "Bs0", -4112: "Anti_Sigma_c_plus", 4114: "Sigma_c_star_minus",

-533: "Anti_Bs_star0", -531: "Anti_Bs0", 4222: "Sigma_c_plus_plus", 3201: "us_diquark_S0",

333: "Phi_meson", -4132: "Anti_Xi_c_plus", -415: "Anti_D_star_minus", 415: "D_star_plus",

5114: "Sigma_b_star_minus", -5122: "Anti_Lambda_b0", -213: "Rho_minus", -5112: "Anti_Sigma_b_minus",

4314: "Xi_c_star0", -4314: "Anti_Xi_c_star0", 4334: "Omega_c_star0", 4332: "Omega_c0",

-425: "Anti_Ds_star_minus", 3103: "us_diquark_S1", 3212: "Sigma0_baryon", -4312: "Anti_Xi_c0",

-4214: "Anti_Sigma_c_star0", -5214: "Anti_Sigma_b_star_plus", -4224: "Anti_Sigma_c_star_plus_plus",

-3212: "Anti_Sigma0", 4203: "ds_diquark_S1", 3101: "us_diquark_S0", -313: "Anti_K_star0",

3303: "ss_diquark_S1", -5232: "Anti_Xi_b0", 5232: "Xi_b0", 313: "K_star0", 2114: "Delta0_baryon",

-4332: "Anti_Omega_c0"}

  

rawGenParts = Collection(event, "GenPart")

for rawGenPart in rawGenParts:

## 입자 종류 판별기

print(self.Genpidlist)

GenpdgId = rawGenPart.pdgId

  

if GenpdgId in pdg_to_name:

gen_particle_name = pdg_to_name[GenpdgId]

else:

gen_particle_name = "Unknown_" + str(GenpdgId)

if GenpdgId not in self.Genpidlist:

self.Genpidlist.append(GenpdgId)

self.gen_histograms['Gen_' + gen_particle_name + '_num'] = ROOT.TH1F('Gen_' + gen_particle_name + '_num', 'Gen_' + gen_particle_name + '_num', 10, -500., 500.)

self.gen_histograms['Gen_' + gen_particle_name + '_pt'] = ROOT.TH1F('Gen_' + gen_particle_name + '_pt', 'Gen_' + gen_particle_name + '_pt', 100, 0., 1000.)

self.gen_histograms['Gen_' + gen_particle_name + '_eta'] = ROOT.TH1F('Gen_' + gen_particle_name + '_eta', 'Gen_' + gen_particle_name + '_eta', 100,-10, 10)

self.gen_histograms['Gen_' + gen_particle_name + '_mass'] = ROOT.TH1F('Gen_' + gen_particle_name + '_mass', 'Gen_' + gen_particle_name + '_mass', 100, 0., 1000.)

  

# 히스토그램을 관리 시스템에 추가

self.addObject(self.gen_histograms['Gen_' + gen_particle_name + '_num'])

self.addObject(self.gen_histograms['Gen_' + gen_particle_name + '_pt'])

self.addObject(self.gen_histograms['Gen_' + gen_particle_name + '_eta'])

self.addObject(self.gen_histograms['Gen_' + gen_particle_name + '_mass'])

for rawGenPart in rawGenParts:

# 특정 조건에 따라 히스토그램 채우기

GenpdgId = rawGenPart.pdgId

  

if GenpdgId in pdg_to_name:

gen_particle_name = pdg_to_name[GenpdgId]

else:

gen_particle_name = "Unknown_" + str(GenpdgId)

  

if isHardProcess(rawGenPart) :

# 딕셔너리를 통해 히스토그램 접근

self.gen_histograms['Gen_' + gen_particle_name + '_num'].Fill(1)

self.gen_histograms['Gen_' + gen_particle_name + '_pt'].Fill(rawGenPart.pt)

self.gen_histograms['Gen_' + gen_particle_name + '_eta'].Fill(rawGenPart.eta)

self.gen_histograms['Gen_' + gen_particle_name + '_mass'].Fill(rawGenPart.mass)

  
  
  
  
  
  

rawLHEPart = Collection(event, "LHEPart")

for rawLHEParts in rawLHEPart:

print(self.LHEpidlist)

LHEpdgId = rawLHEParts.pdgId

  

if LHEpdgId in pdg_to_name:

LHE_particle_name = pdg_to_name[LHEpdgId]

else:

LHE_particle_name = "Unknown_" + str(LHEpdgId)

  

if LHEpdgId not in self.LHEpidlist:

self.LHEpidlist.append(LHEpdgId)

# 동적으로 히스토그램 생성 및 딕셔너리에 저장

self.lhe_histograms['LHE_' + LHE_particle_name + '_num'] = ROOT.TH1F('LHE_' + LHE_particle_name + '_num', 'LHE_' + LHE_particle_name + '_num', 10, -500., 500.)

self.lhe_histograms['LHE_' + LHE_particle_name + '_pt'] = ROOT.TH1F('LHE_' + LHE_particle_name + '_pt', 'LHE_' + LHE_particle_name + '_pt', 100, 0., 1000.)

self.lhe_histograms['LHE_' + LHE_particle_name + '_eta'] = ROOT.TH1F('LHE_' + LHE_particle_name + '_eta', 'LHE_' + LHE_particle_name + '_eta', 100, -10., 10.)

self.lhe_histograms['LHE_' + LHE_particle_name + '_mass'] = ROOT.TH1F('LHE_' + LHE_particle_name + '_mass', 'LHE_' + LHE_particle_name + '_mass', 100, 0., 1000.)

  

# 히스토그램을 관리 시스템에 추가

self.addObject(self.lhe_histograms['LHE_' + LHE_particle_name + '_num'])

self.addObject(self.lhe_histograms['LHE_' + LHE_particle_name + '_pt'])

self.addObject(self.lhe_histograms['LHE_' + LHE_particle_name + '_eta'])

self.addObject(self.lhe_histograms['LHE_' + LHE_particle_name + '_mass'])

  
  
  
  

for rawLHEParts in rawLHEPart:

  

LHEpdgId = rawLHEParts.pdgId

if LHEpdgId in pdg_to_name:

LHE_particle_name = pdg_to_name[LHEpdgId]

else:

LHE_particle_name = "Unknown_" + str(LHEpdgId)

  

if LHEpdgId in pdg_to_name:

self.lhe_histograms['LHE_' + LHE_particle_name + '_num'].Fill(1)

self.lhe_histograms['LHE_' + LHE_particle_name + '_pt'].Fill(rawLHEParts.pt)

self.lhe_histograms['LHE_' + LHE_particle_name + '_eta'].Fill(rawLHEParts.eta)

self.lhe_histograms['LHE_' + LHE_particle_name + '_mass'].Fill(rawLHEParts.mass)

  

# 특정 조건에 따라 히스토그램 채우기

  

#print (LHEpidlist)

return True

  
  
  

parser = argparse.ArgumentParser(description='WRTau NanoGen')

parser.add_argument('-d', dest='directory',default="")

parser.add_argument('-f', dest='singlefile',default="")

parser.add_argument('-o', dest='output',default="histOut.root")

args = parser.parse_args()

  

directory = "" ; outputname = "" ; preselection = "" ; singlefile = ""

  

if args.directory == "" and args.singlefile == "" :

print("Directory(-d) or file(-f) should be given")

quit()

  

outputname = args.output

files=[]

  

if args.directory != "" and args.singlefile == "" :

directory = args.directory

for filename in os.listdir(directory) :

if filename.endswith(".root") :

files.append(os.path.join(directory, filename))

print(os.path.join(directory,filename))

else : continue

  

if args.directory == "" and args.singlefile != "" :

singlefile = args.singlefile

files.append(singlefile)

  
  

p = PostProcessor(".", files, cut=preselection, branchsel=None, modules=[

ExampleAnalysis()], noOut=True, histFileName=outputname, histDirName="plots")

p.run()
```