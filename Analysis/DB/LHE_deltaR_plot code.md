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

  
  
  
  

def beginJob(self, histFile=None, histDirName=None):

Module.beginJob(self, histFile, histDirName)

  
  
  

class ExampleAnalysis(Module):

def __init__(self):

self.writeHistFile = True

  

def beginJob(self, histFile=None, histDirName=None):

Module.beginJob(self, histFile, histDirName)

# Delta R 히스토그램 생성

self.addObject(ROOT.TH1F("deltaR_dist", "DeltaR Distribution between Two Quarks", 50, 0, 5))

  

def analyze(self, event):

LHE_parts = Collection(event, "LHEPart")

quarks_ud = []

quarks_sc = []

for idx, part in enumerate(LHE_parts) :

# 쿼크 쌍 찾기 (예: b-쿼크와 반쿼크)

if abs(part["pdgId"]) in [1, 2] : # u, d

quarks_ud.append(part)

if abs(part["pdgId"]) in [3, 4] : # s, c 쿼크

quarks_sc.append(part)

# 가능한 쌍(쿼크-반쿼크)으로 deltaR 계산

if len(quarks_ud) >1:

for i in range(len(quarks_ud)):

for j in range(i + 1, len(quarks_ud)):

delta_R_ud = deltaR(quarks_ud[i], quarks_ud[j])

self.deltaR_dist.Fill(delta_R_ud)

if len(quarks_sc) >1:

for i in range(len(quarks_sc)):

for j in range(i + 1, len(quarks_sc)):

delta_R_sc = deltaR(quarks_sc[i], quarks_sc[j])

self.deltaR_dist.Fill(delta_R_sc)

  
  

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