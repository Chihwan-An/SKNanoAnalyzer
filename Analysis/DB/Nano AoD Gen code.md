```python

#!/usr/bin/env python

from PhysicsTools.NanoAODTools.postprocessing.tools import *

from PhysicsTools.NanoAODTools.postprocessing.framework.eventloop import Module

from PhysicsTools.NanoAODTools.postprocessing.framework.datamodel import Collection,Object

from PhysicsTools.NanoAODTools.postprocessing.framework.postprocessor import PostProcessor

from PhysicsTools.NanoAODTools.postprocessing.analyser.ID.GenStatus import *

from PhysicsTools.NanoAODTools.postprocessing.analyser.AnalyserHelper.AnalyserHelper import *

from importlib import import_module

import os

import sys

import ROOT

import argparse

import linecache

ROOT.PyConfig.IgnoreCommandLineOptions = True

  
  
  

class ExampleAnalysis(Module):

def __init__(self):

self.writeHistFile = True

  

def beginJob(self, histFile=None, histDirName=None):

Module.beginJob(self, histFile, histDirName)

  

oblist = ['pt','eta'] ; binning=0 ; minval=0 ; maxval=0 ;

histlist = []

  

for i in range(0,2) :

for ob in oblist :

if ob == 'pt' :

binning = 50 ;minval = 0;maxval = 2500

if ob == 'eta' :

binning = 24 ; minval = -6.0 ; maxval = 6.0

for region in ["resolved","merged"] :

histlist.append(ROOT.TH1F(region+'_Lepton'+str(i)+'_'+ob,region+'_Lepton'+str(i)+'_'+ob,binning,minval,maxval))

histlist.append(ROOT.TH1F(region+'_Jet'+str(i)+'_'+ob,region+'_Jet'+str(i)+'_'+ob,binning,minval,maxval))

histlist.append(ROOT.TH1F(region+'_FatJet'+str(i)+'_'+ob,region+'_FatJet'+str(i)+'_'+ob,binning,minval,maxval))

  
  

for hist in histlist :

self.addObject(hist)

  

self.addObject(ROOT.TH1F('dRll','dRll',60,0.,6.))

self.addObject(ROOT.TH1F('dRJj0','dRJj0',60,0.,6.))

self.addObject(ROOT.TH1F('dRJj1','dRJj1',60,0.,6.))

self.addObject(ROOT.TH1F('dRjj','dRjj',60,0.,6.))

self.addObject(ROOT.TH1F('mjj','mjj',100,0.,1000.))

self.addObject(ROOT.TH1F('Cutflow','Cutflow',6,0,6))

self.addObject(ROOT.TH1F('nAK4_unclean','nAK4_unclean',10,0,10))

self.addObject(ROOT.TH1F('nLep','nLep',10,0,10))

self.addObject(ROOT.TH1F('nTau','nTau',5,0.,5.))

# FinalStates : 0=th+th 1=te+th 2=tmu+th 3=te+tmu 4=te+te 5=tmu+tmu

self.addObject(ROOT.TH1F('FinalStates','FinalStates',6,0.,6.))

  

self.addObject(ROOT.TH1F('resolved_dRll','resolved_dRll',60,0.,6.))

self.addObject(ROOT.TH1F('resolved_WR_mass','resolved_WR_mass',1000,0.,5000.))

self.addObject(ROOT.TH1F('resolved_WR_eta','resolved_WR_eta',24,-6.,6.))

self.addObject(ROOT.TH1F('resolved_WR_pT','resolved_WR_pT',20,0.,1000.))

self.addObject(ROOT.TH1F('resolved_N_mass','resolved_N_mass',1000,0.,5000.))

self.addObject(ROOT.TH1F('resolved_N_eta','resolved_N_eta',120,-6.,6.))

self.addObject(ROOT.TH1F('resolved_N_pT','resolved_N_pT',60,0.,3000.))

self.addObject(ROOT.TH1F('resolved_MET','resolved_MET',1000,0.,5000.))

self.addObject(ROOT.TH1F('resolved_nAK4','resolved_nAK4',10,0,10))

self.addObject(ROOT.TH1F('resolved_nAK4_taucleaned','resolved_nAK4_taucleaned',10,0,10))

self.addObject(ROOT.TH1F('resolved_nAK8','resolved_nAK8',10,0,10))

  

self.addObject(ROOT.TH1F('merged_dRll','merged_dRll',60,0.,6.))

self.addObject(ROOT.TH1F('merged_dRJl_primary','merged_dRll',60,0.,6.))

self.addObject(ROOT.TH1F('merged_dRJl_secondary','merged_dRll',60,0.,6.))

self.addObject(ROOT.TH1F('merged_WR_mass','merged_WR_mass',1000,0.,5000.))

self.addObject(ROOT.TH1F('merged_WR_mass_test','merged_WR_mass_test',1000,0.,5000.))

self.addObject(ROOT.TH1F('merged_WR_eta','merged_WR_eta',24,-6.,6.))

self.addObject(ROOT.TH1F('merged_WR_pT','merged_WR_pT',20,0.,1000.))

self.addObject(ROOT.TH1F('merged_N_mass','merged_N_mass',1000,0.,5000.))

self.addObject(ROOT.TH1F('merged_N_mass_test','merged_N_mass_test',1000,0.,5000.))

self.addObject(ROOT.TH1F('merged_N_eta','merged_N_eta',120,-6.,6.))

self.addObject(ROOT.TH1F('merged_N_pT','merged_N_pT',60,0.,3000.))

self.addObject(ROOT.TH1F('merged_nAK8','merged_nAK8',10,0,10))

  

## LHE_part = patron ( g - qq ) 나오지 않는 경우

## + ps

def analyze(self, event):

  

isNoCut = False

  

rawGenParts = Collection(event,"GenPart") -> 8번

rawGenJets = Collection(event, "GenJet")

rawGenFatJets = Collection(event,"GenJetAK8")

GenMETv = Object(event, "GenMET")

GenMETv_LorentzVector = GetGenMETLorentzVector(GenMETv)

Generator = Object(event,"Generator")

w = Generator.weight

  

rawGenVisTaus = Collection(event,"GenVisTau") ### 뭔 차이 ??

rawGenLeptons = [] ; rawGenMuons = [] ; rawGenElectrons = []; rawGenTaus =[]

  

###########################################

########### 이벤트 가져오기

  

########### rawGenLepton , rawGenTau 생성 ( 13 = muon , 11 = electron , 15 = tau ) staus 에 따라 m, e // tau 나눔 pdgID를 가져온거면 실제 입자를 가져온거겠지?

##################################################################

for rawGenPart in rawGenParts :

if rawGenPart.status == 8 : #and fromHardProcess(rawGenPart)

# if abs(rawGenPart.pdgId) == 13 or abs(rawGenPart.pdgId) == 11 : rawGenLeptons.append(rawGenPart)

# else :

# if abs(rawGenPart.pdgId) == 15 : rawGenTaus.append(rawGenPart)

###################################################################

###################################################################

  

Genlist = []

for p in Genlist :

print ( p.pt )

# # Jets

GenJets = []

GenJets_taucleaned = []

GenFatJets = []

  

# Light Leptons (non Tau origin)

GenLeptons = [] # E+Mu (not from Taus; prompt leptons)

GenMuons = []

GenElectrons = []

# Taus (Leptonic, Hadronic)

GenTaus = [] # All Taus

GenVisTaus = []

#GenTauLeptonic = [] # Leptonic Taus ; GenTauLepEl + GenTauLepMu

#GenTauLeptonicEl = [] # el from ta > el nu nu

#GenTauLeptonicMu = [] # mu from ta > mu nu nu

  

l_Gen = [GenJets, GenJets_taucleaned, GenFatJets,

GenMuons, GenElectrons, GenLeptons,

GenTaus]

  
  

############# rawGenlepton 을 나눠서 e mu로 따로 저장 ( staus 통과한 e mu 따로 저장)

##################################################################

##################################################################

for rawGenLepton in rawGenLeptons :

if abs(rawGenLepton.pdgId) == 13 : rawGenMuons.append(rawGenLepton)

elif abs(rawGenLepton.pdgId) == 11 : rawGenElectrons.append(rawGenLepton)

  

##################################################################

################################################################## status 통과한 Mu E 중에서 M , E 중에서 leptonic tau 같은 거 selection 2개 통과시키기

GenTauLeptonicMu = SelectLeptonicTau(rawGenMuons,6.,0.) # M 중에서 T 기준 1 (eta max, pt min )

GenTauLeptonicEl = SelectLeptonicTau(rawGenElectrons,6.,0.) # E 중에서 T 기준 1

  

AnalysisGenTauLeptonicMu = SelectLeptonicTau(rawGenMuons,2.4,53.) # M중에서 T 기준 2

AnalysisGenTauLeptonicEl = SelectLeptonicTau(rawGenElectrons,2.4,53.) # E중에서 T 기준 2

  

###################################################################

################################################################### status 통과한 Mu E 중에서 M , E 중에서 light leptons 같은 거 selection 2개 통과시키기

GenMuons = SelectLightLeptons(rawGenMuons,6.,0.) # M 중에서 가벼운 렙톤 기준??

AnalysisGenMuons = SelectLightLeptons(rawGenMuons,2.4,53) # M 중에서 가벼운 렙톤 기준 2 ??

GenElectrons = SelectLightLeptons(rawGenElectrons,6.,0.) # E 중에서 가벼운 렙톤 기준 ??

AnalysisGenElectrons = SelectLightLeptons(rawGenElectrons,2.4,53) # E 중에서 가벼운 렙톤 기준 2 ??

GenLeptons = GenMuons + GenElectrons ## M , E 중에서 기준 1 통과

AnalysisGenLeptons = AnalysisGenElectrons + AnalysisGenMuons ## M , E 중에서 기준 2통과

###################################################################

###################################################################rawGenpart -> status 아닌 tau , selection 통과 ,

GenTaus_tmp = Select(rawGenTaus,6.,0.) ## 기준 1 T

GenTaus = SelectWRSignalTaus(GenTaus_tmp,rawGenParts) ## 기준 1 T 통과중 엄마가 34 , 9900012,9900014,9900016 인거 고르기

AnalysisGenTaus_tmp = Select(rawGenTaus,2.4,53) ## 기준 2 T

AnalysisGenTaus = SelectWRSignalTaus(AnalysisGenTaus_tmp,rawGenParts) ## 기준 2 T 통과중 엄마가 34 , 9900012,9900014,9900016 인거 고르기

if isNoCut : AnalysisGenTaus = GenTaus # No cut 처음부터 False 였는데?

###################################################################

###################################################################rawGenVisTaus = Collection(event,"GenVisTau") 에서 온 T

GenVisTaus = Select(rawGenVisTaus,6.,0.) ## 기준 1 T

AnalysisGenVisTaus = Select(rawGenVisTaus,2.4,53) ## 기준 2 T

if isNoCut : AnalysisGenVisTaus = GenVisTaus

  

###################################################################

################################################################### ???

self.nLep.Fill(len(GenLeptons),w) # 기준 1 통과한애중에서 light lepton인거

self.nTau.Fill(len(GenTaus),w) ## 기준 1통과한 T 중 엄마 맞는거

#self.nAK4_unclean.Fill(len(rawGenJets),w)

  
  
  
  
  

## jet 골라내기

###################################################################

################################################################### Jet 안에 lepton 의 dr min 보다 큰 애들만

GenJets_all = JetLeptonCleaning(rawGenJets,rawGenLeptons,0.2) ## rawGenJets 중에서 option 통과한 애들의 R이 0.2보다 큰 경우만 골라내기

  

GenFatJets_alltmp = JetLeptonCleaning(rawGenFatJets,rawGenLeptons,0.2) ## rawfatGenJets 중에서 option 통과한 애들의 R이 0.2보다 큰 경우만 골라내기

GenFatJets_all = JetTauCleaning(GenFatJets_alltmp,AnalysisGenVisTaus,0.1) ## 위에서 골라낸 jet중에서 기준 2 통과한 T의 R이 0.1보다 큰경우만

###################################################################

################################################################### 바로위에서 골라낸 2가지 jet들을 eta pt cut 기준 2가지

GenJets = Select(GenJets_all,6.,0.) ## raw Jet에서 Lepton들어있는경우 R이 0.2보다 크고 , eta가 6보다 작은 경우

GenFatJets = Select(GenFatJets_all,6.,0.) ## 전에는 T의 eta cut이 2.4 , 이제는 jet eta 가 6.0 보다 작은 경우

  

AnalysisGenJets = Select(GenJets_all,2.4,40.) ## raw Jet에서 Lepton들어있는경우 R이 0.2보다 크고 , eta가 2.4보다 작은 경우 , Pt 40보다 큰 경우

AnalysisGenFatJets_uncleaned = Select(GenFatJets_all,2.4,200.)## 전에는 T의 eta cut이 2.4 , 이제는 jet eta 가 2.4 보다 작은 경우 , pt 200 보다 큰 경우

  

AnalysisGenFatJets = AnalysisGenFatJets_uncleaned

if isNoCut : AnalysisGenFatJets = GenFatJets ##????

  

###################################################################

###################################################################

  
  

#print("uncleaned {}".format(len(AnalysisGenFatJets_uncleaned)))

#for J in AnalysisGenFatJets :

# n = 0

# for j in AnalysisGenJets :

# if deltaR(j,J) < 0.01 : n += 1

# if n == 0 : AnalysisGenFatJets.append(J)

#print("cleaned {}".format(len(AnalysisGenFatJets)))

  

#self.nAK4.Fill(len(AnalysisGenJets),w)

#self.nAK8.Fill(len(AnalysisGenFatJets),w)

  

FinalLeptons = AnalysisGenTaus ## 기준 2 통과 T

  

AnalysisGenJets_taucleaned = JetTauCleaning(AnalysisGenJets,AnalysisGenVisTaus,0.1) ## [raw Jet에서 Lepton들어있는경우 R이 0.2보다 크고 , eta가 2.4보다 작은 경우 , Pt 40보다 큰 경우 ]안에

## rawGenVisTaus = Collection(event,"GenVisTau") 에서 온 T 의 R이 0.1보다 큰 경우

AnalysisGenJets_taucleaned.sort(key=lambda x : x.pt , reverse=True) ## pt 큰 순서대로

#self.nAK4_taucleaned.Fill(len(GenJets_taucleaned))

  

###################################################################

###################################################################

  
  
  

l_Gen = [AnalysisGenJets, AnalysisGenJets_taucleaned, AnalysisGenFatJets,

AnalysisGenMuons, AnalysisGenElectrons, AnalysisGenLeptons,

AnalysisGenTaus]

  

################################################################### 지금 까지 만들어 놓은 것들

###################################################################

  

for l in l_Gen :

l.sort(key=lambda x : x.pt , reverse=True)

  

FinalLeptons.sort(key=lambda x : x.pt, reverse=True)

  

## 이거 무슨 기준이지 ??

###################################################################

if len(AnalysisGenJets_taucleaned) > 1 : self.dRjj.Fill(deltaR(AnalysisGenJets_taucleaned[0],AnalysisGenJets_taucleaned[1]))

#if len(GenFatJets) > 0 :

# hists = [self.dRJj0,self.dRJj1]

# for i in range(0,min(len(GenJets_taucleaned),2)) :

# hists[i].Fill(deltaR(GenFatJets[0],GenJets_taucleaned[i]),w)

  

self.Cutflow.Fill(0)

  

###################################################################

################################################################### 기준 2 통과 T 갯수 2개면 통과

if len(FinalLeptons) != 2 : return

self.Cutflow.Fill(1)

  

###################################################################

################################################################### Genvistau의 갯수가 2개, 1개 ( 나머지 tau 같은 el , mu ) 찾기

channel = 0

if len(AnalysisGenVisTaus) == 2 : channel = 0

elif len(AnalysisGenVisTaus) == 1 :

if len(AnalysisGenTauLeptonicEl) == 1 : channel = 1

elif len(AnalysisGenTauLeptonicMu) == 1 : channel = 2

self.FinalStates.Fill(channel,w)

  

###################################################################

################################################################### 최종 제트가 있고 , 기준 2 통과한 진짜 T 2개이고 pt 최저 63보다 큰 경우 델타 에 따라 ㅂ스트 된경우 찾기

  

isResolved = False ; isBoosted = False

  

if len(AnalysisGenFatJets) > 0 :

if len(FinalLeptons) == 2 and FinalLeptons[0].pt > 63 :

if deltaR(FinalLeptons[0],AnalysisGenFatJets[0]) < 0.8 or deltaR(FinalLeptons[1],AnalysisGenFatJets[0]) < 0.8 : isBoosted = True

  
  
  

## 부스트 없는 경우 jet 가 resolve = true 나올 수 있음

if not isBoosted :

if len(FinalLeptons) == 2 and len(AnalysisGenJets_taucleaned) > 1 :

if FinalLeptons[0].pt > 63 and FinalLeptons[1].pt > 53 :

dRlj = []

for lep in FinalLeptons :

for jet in AnalysisGenJets_taucleaned :

dRlj.append(deltaR(lep,jet))

if min(dRlj) > 0.4 : isResolved = True

  

print("{},{}".format(isResolved,isBoosted))

  

WR = ROOT.TLorentzVector()

N = ROOT.TLorentzVector()

N_id = [9900016]

WR_id = [34]

  

isBothFromWR = HasAncestor(WR_id,FinalLeptons[0],rawGenParts) and HasAncestor(WR_id,FinalLeptons[1],rawGenParts)

  

if isResolved == False and isBoosted == True :

#print("boosted")

if not isBothFromWR :

#print("not both from WR")

return

else :

WR = FinalLeptons[0].p4()+FinalLeptons[1].p4()+AnalysisGenFatJets[0].p4()

debug = 0; secondaryidx = 0; primaryidx = 0

for i in range(0,2) :

if HasAncestor(N_id,FinalLeptons[i],rawGenParts) :

debug +=1

secondaryidx = i

elif not HasAncestor(N_id,FinalLeptons[i],rawGenParts) : primaryidx = i

else : return

if debug > 1 :

#print("wrong final state")

return

# AK8 selection in GEN level is really crappy, need to filter out wrong jets too

dRJl_pri = deltaR(FinalLeptons[primaryidx],AnalysisGenFatJets[0])

dRJl_sec = deltaR(FinalLeptons[secondaryidx],AnalysisGenFatJets[0])

isProperBoostedRegion = dRJl_sec < 0.8 and dRJl_pri > 2.5

# the fuck is going on with deltaR ; i'll just fucking kill all of the weird events

if not isProperBoostedRegion : return

N = AnalysisGenFatJets[0].p4()

WR_test = FinalLeptons[primaryidx].p4() + N

  

self.Cutflow.Fill(2)

self.merged_nAK8.Fill(len(AnalysisGenFatJets))

self.merged_dRll.Fill(deltaR(FinalLeptons[0],FinalLeptons[1]))

self.merged_Lepton0_pt.Fill(FinalLeptons[primaryidx].pt)

self.merged_Lepton1_pt.Fill(FinalLeptons[secondaryidx].pt)

self.merged_Lepton0_eta.Fill(FinalLeptons[primaryidx].eta)

self.merged_Lepton1_eta.Fill(FinalLeptons[secondaryidx].eta)

self.merged_WR_mass.Fill(WR.M())

self.merged_WR_mass_test.Fill(WR_test.M())

self.merged_WR_eta.Fill(WR.Eta())

self.merged_WR_pT.Fill(WR.Pt())

self.merged_N_mass.Fill(N.M())

self.merged_N_mass_test.Fill(N.M())

self.merged_N_eta.Fill(N.Eta())

self.merged_N_pT.Fill(N.Pt())

self.merged_FatJet0_pt.Fill(AnalysisGenFatJets[0].pt)

self.merged_FatJet0_eta.Fill(AnalysisGenFatJets[0].eta)

self.merged_dRJl_primary.Fill(deltaR(FinalLeptons[primaryidx],AnalysisGenFatJets[0]))

self.merged_dRJl_secondary.Fill(deltaR(FinalLeptons[secondaryidx],AnalysisGenFatJets[0]))

  
  

elif isResolved == True and isBoosted == False :

#print("resolved")

if not isBothFromWR :

#print("not both from WR")

return

else :

WR = FinalLeptons[0].p4()+FinalLeptons[1].p4()+AnalysisGenJets_taucleaned[0].p4()+AnalysisGenJets_taucleaned[1].p4()

debug = 0; secondaryidx = 0; primaryidx = 0

for i in range(0,2) :

if HasAncestor(N_id,FinalLeptons[i],rawGenParts) :

debug +=1

secondaryidx = i

elif not HasAncestor(N_id,FinalLeptons[i],rawGenParts) : primaryidx = i

if debug > 1 :

#print("[merged] wrong final state")

return

N = FinalLeptons[secondaryidx].p4() + AnalysisGenJets_taucleaned[0].p4()+AnalysisGenJets_taucleaned[1].p4()

  

self.Cutflow.Fill(3)

self.resolved_nAK4.Fill(len(AnalysisGenJets_taucleaned))

self.resolved_dRll.Fill(deltaR(FinalLeptons[0],FinalLeptons[1]))

self.resolved_Lepton0_pt.Fill(FinalLeptons[primaryidx].pt)

self.resolved_Lepton1_pt.Fill(FinalLeptons[secondaryidx].pt)

self.resolved_Lepton0_eta.Fill(FinalLeptons[primaryidx].eta)

self.resolved_Lepton1_eta.Fill(FinalLeptons[secondaryidx].eta)

self.resolved_WR_mass.Fill(WR.M())

self.resolved_WR_eta.Fill(WR.Eta())

self.resolved_WR_pT.Fill(WR.Pt())

self.resolved_N_mass.Fill(N.M())

self.resolved_N_eta.Fill(N.Eta())

self.resolved_N_pT.Fill(N.Pt())

self.resolved_Jet0_pt.Fill(AnalysisGenJets_taucleaned[0].pt)

self.resolved_Jet1_pt.Fill(AnalysisGenJets_taucleaned[1].pt)

self.resolved_Jet0_eta.Fill(AnalysisGenJets_taucleaned[0].eta)

self.resolved_Jet1_eta.Fill(AnalysisGenJets_taucleaned[1].eta)

  

else : return

self.Cutflow.Fill(4)

  

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