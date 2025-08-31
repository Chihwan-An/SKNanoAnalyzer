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

  

def beginJob(self, histFile=None, histDirName=None):

	Module.beginJob(self, histFile, histDirName)

  

def build_tree(gen_parts):

"""트리 형태로 계보를 구축.gen_parts: 리스트 - 각 항목은 {'pdgId': int, 'genPartIdxMother': int} 형태의 사전"""

tree = {}

index_to_node = {}

# 노드 초기화

for idx, part in enumerate(gen_parts):

node = {"pdgId": part["pdgId"], "parents": [], "children": [], "eta": part["eta"], "phi": part["phi"]}

index_to_node[idx] = node

tree[idx] = node

  

# 부모-자식 관계 설정

for idx, part in enumerate(gen_parts):

if isHardProcess(part):

mother_indices = part["genPartIdxMother"] if isinstance(part["genPartIdxMother"], list) else [part["genPartIdxMother"]]

## mother가 여러개면 리스트 타입으로 나오고 , 단일이면 정수로 나옴 .

for mother_idx in mother_indices:

if mother_idx >= 0: # 유효한 부모인 경우

index_to_node[mother_idx]["children"].append(tree[idx])

tree[idx]["parents"].append(index_to_node[mother_idx])

  

# 루트 노드 반환 (부모가 없고 자식이 있는 노드)

roots = [node for idx, node in tree.items() if len(node["parents"]) == 0 and len(node["children"]) > 0]

return roots

  

# 트리 출력 (재귀적으로)

def print_tree(node, depth=0):

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

if node == None:

print("")

else:

if node["pdgId"] not in pdg_to_name:

print("{indent}Particle PDG ID: {pdgId} (Unknown)".format(indent=" " * depth, pdgId=node["pdgId"]))

else:

print("{indent}Particle PDG ID: {pdgId}".format(indent=" " * depth, pdgId=pdg_to_name[node["pdgId"]]))

for child in node["children"]:

print_tree(child, depth + 1)

  
  

class ExampleAnalysis(Module):

def __init__(self):

self.writeHistFile = True

  

def beginJob(self, histFile=None, histDirName=None):

Module.beginJob(self, histFile, histDirName)

# Delta R 히스토그램 생성

self.addObject(ROOT.TH1F("deltaR_dist", "DeltaR Distribution between Two Quarks", 50, 0, 5))

  

def analyze(self, event):

genparts = Collection(event, "GenPart")

roots = build_tree(genparts)

  

# 트리 구조에서 루트부터 탐색

for root in roots:

# Heavy neutrino 찾기 (루트가 직접 9900016인지 확인)

if root["pdgId"] == 9900016 :

# 자식 노드 중 쿼크 찾기

quark_nodes = [child for child in root["children"] if abs(child["pdgId"]) in [1, 2, 3, 4, 5, 6]] # u, d, s, c, b, t

  
  

if len(quark_nodes) >= 2:

# 첫 번째와 두 번째 쿼크의 Delta R 계산

quark_a, quark_b = quark_nodes[0], quark_nodes[1]

# Delta R 직접 계산

delta_eta = quark_a["eta"] - quark_b["eta"]

delta_phi = quark_a["phi"] - quark_b["phi"]

# |Delta phi| 값 조정

if delta_phi > 3.14159:

delta_phi -= 2 * 3.14159

elif delta_phi < -3.14159:

delta_phi += 2 * 3.14159

  

delta_r = (delta_eta**2 + delta_phi**2)**0.5

# 히스토그램에 Delta R 추가

self.deltaR_dist.Fill(delta_r)

  

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