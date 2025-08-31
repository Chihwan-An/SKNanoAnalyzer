
1.  *using electron and muon*

```python
#!/usr/bin/env python

# -*- coding: utf-8 -*-

from PhysicsTools.NanoAODTools.postprocessing.tools import *

from PhysicsTools.NanoAODTools.postprocessing.framework.eventloop import Module

from PhysicsTools.NanoAODTools.postprocessing.framework.datamodel import Collection,Object

from PhysicsTools.NanoAODTools.postprocessing.framework.postprocessor import PostProcessor

#from PhysicsTools.NanoAODTools.postprocessing.analyser.ID.GenStatus import *

#from PhysicsTools.NanoAODTools.postprocessing.analyser.AnalyserHelper.AnalyserHelper import *

from importlib import import_module

import os

import sys

import ROOT

import argparse

import linecache

ROOT.PyConfig.IgnoreCommandLineOptions = True

  
  

def Select(inputcoll,etamax,ptmin) :

output = []

for obj in inputcoll :

if abs(obj.eta) < etamax and obj.pt > ptmin : output.append(obj)

return output

def FlagIsOn(status,flag_n) :

if status & (1 << (flag_n -1)) : return True

else : return False

  

def isHardProcess(gen) : return FlagIsOn(gen.statusFlags,8)

def Process2(gen) : return FlagIsOn(gen.statusFlags,2)

def Process3(gen) : return FlagIsOn(gen.statusFlags,3)

def Process4(gen) : return FlagIsOn(gen.statusFlags,4)

def Process5(gen) : return FlagIsOn(gen.statusFlags,5)

  

class ExampleAnalysis(Module):

def __init__(self):

self.writeHistFile = True

  

def beginJob(self, histFile=None, histDirName=None):

Module.beginJob(self, histFile, histDirName)

####################before cut #################

# pt

self.addObject(ROOT.TH1F('tau_no_cut_pt_leading','tau_no_cut_pt_leading',400,0.,4000.))

self.addObject(ROOT.TH1F('jetAK4_no_cut_pt_leading','jetAK4_no_cut_pt_leading',400,0.,4000.))

self.addObject(ROOT.TH1F('jetAK4_no_cut_pt_subleading','jetAK4_no_cut_pt_subleading',400,0.,4000.))

self.addObject(ROOT.TH1F('jetAK8_no_cut_pt_leading','jetAK8_no_cut_pt_leading',400,0.,4000.)) ## N8 이랑 동일

self.addObject(ROOT.TH1F('subjetAK8_no_cut_pt_leading','subjetAK8_no_cut_pt_leading',400,0,4000))

self.addObject(ROOT.TH1F('subjetAK8_no_cut_pt_subleading','subjetAK8_no_cut_pt_subleading',400,0,4000))

self.addObject(ROOT.TH1F('WR4_no_cut_pt_el','WR4_no_cut_pt_el',400,0.,4000.))

self.addObject(ROOT.TH1F('WR4_no_cut_pt_mu','WR4_no_cut_pt_mu',400,0.,4000.))

self.addObject(ROOT.TH1F('N4_no_cut_pt_el','N4_no_cut_pt_el',400,0.,4000.))

self.addObject(ROOT.TH1F('N4_no_cut_pt_mu','N4_no_cut_pt_mu',400,0.,4000.))

self.addObject(ROOT.TH1F('WR8_no_cut_pt','WR8_no_cut_pt',400,0.,4000.))

self.addObject(ROOT.TH1F('subWR8_no_cut_pt_el','subWR8_no_cut_pt_el',400,0.,4000.))

self.addObject(ROOT.TH1F('subWR8_no_cut_pt_mu','subWR8_no_cut_pt_mu',400,0.,4000.))

self.addObject(ROOT.TH1F('subN8_no_cut_pt_el','subN8_no_cut_pt_el',400,0.,4000.))

self.addObject(ROOT.TH1F('subN8_no_cut_pt_mu','subN8_no_cut_pt_mu',400,0.,4000.))

  

# eta

self.addObject(ROOT.TH1F('tau_no_cut_eta_leading','tau_no_cut_eta_leading',80,-4.,4.))

self.addObject(ROOT.TH1F('jetAK4_no_cut_eta_leading','jetAK4_no_cut_eta_leading',80,-4.,4.))

self.addObject(ROOT.TH1F('jetAK4_no_cut_eta_subleading','jetAK4_no_cut_eta_subleading',80,-4.,4.))

self.addObject(ROOT.TH1F('jetAK8_no_cut_eta_leading','jetAK8_no_cut_eta_leading',80,-4.,4.))

self.addObject(ROOT.TH1F('subjetAK8_no_cut_eta_leading','subjetAK8_no_cut_eta_leading',80,-4.,4.))

self.addObject(ROOT.TH1F('subjetAK8_no_cut_eta_subleading','subjetAK8_no_cut_eta_subleading',80,-4.,4.))

self.addObject(ROOT.TH1F('WR4_no_cut_eta_el','WR4_no_cut_eta_el',80,-4.,4.))

self.addObject(ROOT.TH1F('WR4_no_cut_eta_mu','WR4_no_cut_eta_mu',80,-4.,4.))

self.addObject(ROOT.TH1F('N4_no_cut_eta_el','N4_no_cut_eta_el',80,-4.,4.))

self.addObject(ROOT.TH1F('N4_no_cut_eta_mu','N4_no_cut_eta_mu',80,-4.,4.))

self.addObject(ROOT.TH1F('WR8_no_cut_eta','WR8_no_cut_eta',80,-4.,4.))

self.addObject(ROOT.TH1F('subWR8_no_cut_eta_el','subWR8_no_cut_eta_el',80,-4.,4.))

self.addObject(ROOT.TH1F('subWR8_no_cut_eta_mu','subWR8_no_cut_eta_mu',80,-4.,4.))

self.addObject(ROOT.TH1F('subN8_no_cut_eta_el','subN8_no_cut_eta_el',80,-4.,4.))

self.addObject(ROOT.TH1F('subN8_no_cut_eta_mu','subN8_no_cut_eta_mu',80,-4.,4.))

#mass

self.addObject(ROOT.TH1F('no_cut_mass_AK4_el','tau_no_cut_mass_AK4_el',750,0.,7500.)) #N4

self.addObject(ROOT.TH1F('no_cut_mass_AK4_mu','tau_no_cut_mass_AK4_mu',750,0.,7500.)) #N4

self.addObject(ROOT.TH1F('tau_no_cut_mass_AK4_el','tau_no_cut_mass_AK4_el',750,0.,7500.)) # WR4

self.addObject(ROOT.TH1F('tau_no_cut_mass_AK4_mu','tau_no_cut_mass_AK4_mu',750,0.,7500.)) # WR4

self.addObject(ROOT.TH1F('tau_no_cut_mass_AK8_tau','tau_no_cut_mass_AK8_tau',750,0.,7500.)) ## WR8

self.addObject(ROOT.TH1F('no_cut_mass_AK8','no_cut_mass_AK8',750,0.,7500.)) ## N8

self.addObject(ROOT.TH1F('tau_no_cut_mass_subAK8_el','tau_no_cut_mass_subAK8_el',750,0.,7500.)) #sub W8

self.addObject(ROOT.TH1F('tau_no_cut_mass_subAK8_mu','tau_no_cut_mass_subAK8_mu',750,0.,7500.)) # sub W8

self.addObject(ROOT.TH1F('no_cut_mass_subAK8_el','no_cut_mass_subAK8_el',750,0.,7500.)) #sub N8

self.addObject(ROOT.TH1F('no_cut_mass_subAK8_mu','no_cut_mass_subAK8_mu',750,0.,7500.)) # sub N8

  

########################After cut######################

  

#pt

self.addObject(ROOT.TH1F('tau_cut_pt_leading','tau_cut_pt_leading',400,0.,4000.))

self.addObject(ROOT.TH1F('sectau_cut_pt_leading','sectau_cut_pt_leading',400,0.,4000.))

self.addObject(ROOT.TH1F('jetAK4_cut_pt_leading','jetAK4_cut_pt_leading',400,0.,4000.))

self.addObject(ROOT.TH1F('jetAK4_cut_pt_subleading','jetAK4_cut_pt_subleading',400,0.,4000.))

self.addObject(ROOT.TH1F('jetAK8_cut_pt_leading','jetAK8_cut_pt_leading',400,0.,4000.))

self.addObject(ROOT.TH1F('subjetAK8_cut_pt_leading','subjetAK8_cut_pt_leading',400,0,4000))

self.addObject(ROOT.TH1F('subjetAK8_cut_pt_subleading','subjetAK8_cut_pt_subleading',400,0,4000))

self.addObject(ROOT.TH1F('WR4_cut_pt_el','WR4_cut_pt_el',400,0.,4000.))

self.addObject(ROOT.TH1F('WR4_cut_pt_mu','WR4_cut_pt_mu',400,0.,4000.))

self.addObject(ROOT.TH1F('N4_cut_pt_el','N4_cut_pt_el',400,0.,4000.))

self.addObject(ROOT.TH1F('N4_cut_pt_mu','N4_cut_pt_mu',400,0.,4000.))

self.addObject(ROOT.TH1F('WR8_cut_pt','WR8_cut_pt',400,0.,4000.))

self.addObject(ROOT.TH1F('subWR8_cut_pt_el','subWR8_cut_pt_el',400,0.,4000.))

self.addObject(ROOT.TH1F('subWR8_cut_pt_mu','subWR8_cut_pt_mu',400,0.,4000.))

self.addObject(ROOT.TH1F('subN8_cut_pt_el','subN8_cut_pt_el',400,0.,4000.))

self.addObject(ROOT.TH1F('subN8_cut_pt_mu','subN8_cut_pt_mu',400,0.,4000.))

  
  
  

# eta

self.addObject(ROOT.TH1F('tau_cut_eta_leading','tau_cut_eta_leading',80,-4.,4.))

self.addObject(ROOT.TH1F('sectau_cut_eta_leading','sectau_cut_eta_leading',80,-4.,4.))

self.addObject(ROOT.TH1F('jetAK4_cut_eta_leading','jetAK4_cut_eta_leading',80,-4.,4.))

self.addObject(ROOT.TH1F('jetAK4_cut_eta_subleading','jetAK4_cut_eta_subleading',80,-4.,4.))

self.addObject(ROOT.TH1F('jetAK8_cut_eta_leading','jetAK8_cut_eta_leading',80,-4.,4.))

self.addObject(ROOT.TH1F('subjetAK8_cut_eta_leading','subjetAK8_cut_eta_leading',80,-4.,4.))

self.addObject(ROOT.TH1F('subjetAK8_cut_eta_subleading','subjetAK8_cut_eta_subleading',80,-4.,4.))

self.addObject(ROOT.TH1F('WR4_cut_eta_el','WR4_cut_eta_el',80,-4.,4.))

self.addObject(ROOT.TH1F('WR4_cut_eta_mu','WR4_cut_eta_mu',80,-4.,4.))

self.addObject(ROOT.TH1F('N4_cut_eta_el','N4_cut_eta_el',80,-4.,4.))

self.addObject(ROOT.TH1F('N4_cut_eta_mu','N4_cut_eta_mu',80,-4.,4.))

self.addObject(ROOT.TH1F('WR8_cut_eta','WR8_cut_eta',80,-4.,4.))

self.addObject(ROOT.TH1F('subWR8_cut_eta_el','subWR8_cut_eta_el',80,-4.,4.))

self.addObject(ROOT.TH1F('subWR8_cut_eta_mu','subWR8_cut_eta_mu',80,-4.,4.))

self.addObject(ROOT.TH1F('subN8_cut_eta_el','subN8_cut_eta_el',80,-4.,4.))

self.addObject(ROOT.TH1F('subN8_cut_eta_mu','subN8_cut_eta_mu',80,-4.,4.))

#mass

self.addObject(ROOT.TH1F('cut_mass_AK4_el','cut_mass_AK4_el',750,0.,7500.)) #N4

self.addObject(ROOT.TH1F('cut_mass_AK4_mu','cut_mass_AK4_mu',750,0.,7500.)) #N4

self.addObject(ROOT.TH1F('tau_cut_mass_AK4_el','tau_cut_mass_AK4_el',750,0.,7500.)) # WR4

self.addObject(ROOT.TH1F('tau_cut_mass_AK4_mu','tau_cut_mass_AK4_mu',750,0.,7500.)) # WR4

self.addObject(ROOT.TH1F('tau_cut_mass_AK8_tau','tau_cut_mass_AK8_tau',750,0.,7500.)) ## WR8

self.addObject(ROOT.TH1F('cut_mass_AK8','tau_cut_mass_AK8',750,0.,7500.)) ## N4

self.addObject(ROOT.TH1F('cut_mass_subAK8_el','cut_mass_subAK8_el',750,0.,7500.)) # subN8

self.addObject(ROOT.TH1F('cut_mass_subAK8_mu','cut_mass_subAK8_mu',750,0.,7500.)) # subN8

self.addObject(ROOT.TH1F('tau_cut_mass_subAK8_el','tau_cut_mass_subAK8_el',750,0.,7500.)) #sub W8

self.addObject(ROOT.TH1F('tau_cut_mass_subAK8_mu','tau_cut_mass_subAK8_mu',750,0.,7500.)) # sub W8

  

def analyze(self, event):

isNoCut = False

  

rawGenParts = Collection(event,"GenPart")

rawGenJetAK4 = Collection(event, "GenJet")

rawGenJetAK8 = Collection(event,"GenJetAK8")

rawGenVisTaus = Collection(event,"GenVisTau")

rawSubGenJetAK8 = Collection(event,"SubGenJetAK8")

muons = [] ; electrons =[];tausort=[];jetAK4sort=[];jetAK8sort=[];jetSubAK8sort=[];leptons=[];

sectaus=[];

  

for rawGenParts in rawGenParts:

if abs(rawGenParts.pdgId == 11):

electrons.append(rawGenParts)

leptons.append(rawGenParts)

if abs(rawGenParts.pdgId == 13):

muons.append(rawGenParts)

leptons.append(rawGenParts)

if abs(rawGenParts.pdgId == 15):

if rawGenParts.genPartIdxMother == 9900016:

sectaus.append(rawGenParts)

## pt sorting

electrons.sort(key=lambda x : x.pt, reverse=True)

muons.sort(key=lambda x : x.pt, reverse=True)

leptons.sort(key=lambda x : x.pt, reverse=True)

sectaus.sort(key=lambda x : x.pt, reverse=True)

  

usingleptons = [];

  

for lepton in leptons :

if Process2(lepton) or Process3(lepton) or Process4(lepton) or Process5(lepton): # tau product

usingleptons.append(lepton)

usingleptons.sort(key=lambda x : x.pt , reverse= True)

  
  

'''''

i=0

for lepton in leptons:

i+=1

if Process2(lepton) or Process3(lepton) or Process4(lepton) or Process5(lepton): # tau product

print (i,"nd Pt uses tau particle")

break

'''

  
  

### no cut pt eta

## pt sorting

for rawGenVisTaus in rawGenVisTaus:

tausort.append(rawGenVisTaus)

for rawGenJetAK4 in rawGenJetAK4:

jetAK4sort.append(rawGenJetAK4)

for rawGenJetAK8 in rawGenJetAK8:

jetAK8sort.append(rawGenJetAK8)

  

for rawSubGenJetAK8 in rawSubGenJetAK8:

jetSubAK8sort.append(rawSubGenJetAK8)

tausort.sort(key=lambda x : x.pt, reverse=True)

jetAK4sort.sort(key=lambda x : x.pt, reverse=True)

jetAK8sort.sort(key=lambda x : x.pt, reverse=True)

jetSubAK8sort.sort(key=lambda x : x.pt, reverse=True)

  

if len(tausort) > 0:

self.tau_no_cut_pt_leading.Fill(tausort[0].pt)

self.tau_no_cut_eta_leading.Fill(tausort[0].eta)

if len(jetAK4sort) > 0:

self.jetAK4_no_cut_pt_leading.Fill(jetAK4sort[0].pt)

self.jetAK4_no_cut_eta_leading.Fill(jetAK4sort[0].eta)

if len(jetAK4sort) > 1:

self.jetAK4_no_cut_pt_subleading.Fill(jetAK4sort[1].pt)

self.jetAK4_no_cut_eta_subleading.Fill(jetAK4sort[1].eta)

if len(jetAK8sort) > 0:

self.jetAK8_no_cut_pt_leading.Fill(jetAK8sort[0].pt)

self.jetAK8_no_cut_eta_leading.Fill(jetAK8sort[0].eta)

if len(jetSubAK8sort) > 0:

self.subjetAK8_no_cut_pt_leading.Fill(jetSubAK8sort[0].pt)

self.subjetAK8_no_cut_eta_leading.Fill(jetSubAK8sort[0].eta)

if len(jetSubAK8sort) > 1:

self.subjetAK8_no_cut_pt_subleading.Fill(jetSubAK8sort[1].pt)

self.subjetAK8_no_cut_eta_subleading.Fill(jetSubAK8sort[1].eta)

WR4_el =[]; N4_el =[];WR4_mu =[];WR4_el=[];

WR8 = []; subWR8_el =[]; subWR8_mu = [];

subN8_el =[]; subN8_mu=[];

if (len(tausort)>0 and len(jetAK4sort)>1) :

if ( len( usingleptons) >0 ):

if abs(usingleptons[0].pdgId) == 11 :

WR4_el = tausort[0].p4() + usingleptons[0].p4() + jetAK4sort[0].p4() + jetAK4sort[1].p4()

N4_el = usingleptons[0].p4() + jetAK4sort[0].p4() + jetAK4sort[1].p4()

self.WR4_no_cut_pt_el.Fill(WR4_el.Pt())

self.N4_no_cut_pt_el.Fill(N4_el.Pt())

self.WR4_no_cut_eta_el.Fill(WR4_el.Eta())

self.N4_no_cut_eta_el.Fill(N4_el.Eta())

if abs(usingleptons[0].pdgId) == 13:

WR4_mu = tausort[0].p4() + usingleptons[0].p4() + jetAK4sort[0].p4() + jetAK4sort[1].p4()

N4_mu = usingleptons[0].p4() + jetAK4sort[0].p4() + jetAK4sort[1].p4()

self.WR4_no_cut_pt_mu.Fill(WR4_mu.Pt())

self.N4_no_cut_pt_mu.Fill(N4_mu.Pt())

self.WR4_no_cut_eta_mu.Fill(WR4_mu.Eta())

self.N4_no_cut_eta_mu.Fill(N4_mu.Eta())

  

if (len(tausort)>0 and len(jetAK8sort)>0) :

WR8 = tausort[0].p4() + jetAK8sort[0].p4()

self.WR8_no_cut_pt.Fill(WR8.Pt())

self.WR8_no_cut_eta.Fill(WR8.Eta())

  

if (len(tausort)>0 and len(jetSubAK8sort)>1):

if ( len( usingleptons) >0 ):

if abs(usingleptons[0].pdgId) == 11 :

subWR8_el = tausort[0].p4() + usingleptons[0].p4() + jetSubAK8sort[0].p4() + jetSubAK8sort[1].p4()

subN8_el = usingleptons[0].p4() + jetSubAK8sort[0].p4() + jetSubAK8sort[1].p4()

self.subWR8_no_cut_pt_el.Fill(subWR8_el.Pt())

self.subN8_no_cut_pt_el.Fill(subN8_el.Pt())

self.subWR8_no_cut_eta_el.Fill(subWR8_el.Eta())

self.subN8_no_cut_eta_el.Fill(subN8_el.Eta())

if abs(usingleptons[0].pdgId) == 13:

subWR8_mu = tausort[0].p4() + usingleptons[0].p4() + jetSubAK8sort[0].p4() + jetSubAK8sort[1].p4()

subN8_mu = usingleptons[0].p4() + jetSubAK8sort[0].p4() + jetSubAK8sort[1].p4()

self.subWR8_no_cut_pt_mu.Fill(subWR8_mu.Pt())

self.subN8_no_cut_pt_mu.Fill(subN8_mu.Pt())

self.subWR8_no_cut_eta_mu.Fill(subWR8_mu.Eta())

self.subN8_no_cut_eta_mu.Fill(subN8_mu.Eta())

  

WR4_el_mass =[]; WR4_mu_mass =[]; N4_el_mass = []; N4_mu_mass =[];

WR8_mass = []; N8_mass = []; subWR8_el_mass = []; subWR8_mu_mass = [];

subN8_el_mass = []; subN8_mu_mass = [];

## mass

#AK4 no cut mass

if len(tausort)>0:

if len(jetAK4sort)>1 :

if ( len( usingleptons) >0 ):

if abs(usingleptons[0].pdgId) == 11 :

WR4_el_mass = tausort[0].p4() + usingleptons[0].p4() + jetAK4sort[0].p4() + jetAK4sort[1].p4()

N4_el_mass = usingleptons[0].p4() + jetAK4sort[0].p4() + jetAK4sort[1].p4()

self.tau_no_cut_mass_AK4_el.Fill(WR4_el_mass.M())

self.no_cut_mass_AK4_el.Fill(N4_el_mass.M())

if abs(usingleptons[0].pdgId) == 13:

WR4_mu_mass = tausort[0].p4() + usingleptons[0].p4() + jetAK4sort[0].p4() + jetAK4sort[1].p4()

N4_mu_mass = usingleptons[0].p4() + jetAK4sort[0].p4() + jetAK4sort[1].p4()

self.tau_no_cut_mass_AK4_mu.Fill(WR4_mu_mass.M())

self.no_cut_mass_AK4_mu.Fill(N4_mu_mass.M())

  

# AK8 no cut mass

if (len(tausort)>0 and len(jetAK8sort)>0) :

WR8_mass = tausort[0].p4() + jetAK8sort[0].p4()

self.tau_no_cut_mass_AK8_tau.Fill(WR8_mass.M())

if len(jetAK8sort)>0:

N8_mass = jetAK8sort[0].p4()

self.no_cut_mass_AK8.Fill(N8_mass.M())

# SubAK8 no cut mass

if len(tausort)>0:

if len(jetSubAK8sort)>1:

if ( len( usingleptons) >0 ):

if Process2(usingleptons[0]) or Process3(usingleptons[0]) or Process4(usingleptons[0]) or Process5(usingleptons[0]): # tau product

if abs(usingleptons[0].pdgId) == 11 :

subWR8_el_mass = tausort[0].p4() + usingleptons[0].p4() + jetSubAK8sort[0].p4() + jetSubAK8sort[1].p4()

subN8_el_mass = usingleptons[0].p4() + jetSubAK8sort[0].p4() + jetSubAK8sort[1].p4()

self.tau_no_cut_mass_subAK8_el.Fill(subWR8_el_mass.M())

self.no_cut_mass_subAK8_el.Fill(subN8_el_mass.M())

if abs(usingleptons[0].pdgId) == 13:

subWR8_mu_mass = tausort[0].p4() + usingleptons[0].p4() + jetSubAK8sort[0].p4() + jetSubAK8sort[1].p4()

subN8_mu_mass = usingleptons[0].p4() + jetSubAK8sort[0].p4() + jetSubAK8sort[1].p4()

self.tau_no_cut_mass_subAK8_mu.Fill(subWR8_mu_mass.M())

self.no_cut_mass_subAK8_mu.Fill(subN8_mu_mass.M())

################################################################

################################################################

  

## obj cut

cut_tau =[]

cut_jetAK4 = []

cut_jetAK8 = []

cut_subAK8 = []

if len(tausort)>0 :

cut_tau = Select(tausort,2.1,190)

if len(jetAK4sort)>0:

cut_jetAK4 = Select(jetAK4sort,2.4,40)

if len(jetAK8sort)>0:

cut_jetAK8 = Select(jetAK8sort,2.4,200)

if len(jetSubAK8sort)>0:

cut_subAK8 = Select(jetSubAK8sort,2.4,40)

## cleaning

cleaned_AK4=[]

cleaned_AK8=[]

cleaned_subAK8=[]

  

if len(cut_tau)>0 and len(cut_jetAK4 )>0:

for taus in cut_tau :

for jet in cut_jetAK4 :

if deltaR(taus,jet) > 0.4:

cleaned_AK4.append(jet)

if len(electrons) >0 :

for el in electrons:

if deltaR(jet,el) < 0.4:

if jet in cleaned_AK4:

cleaned_AK4.remove(jet)

if len(muons) >0 and len(electrons) ==0 :

for mu in muons:

if deltaR(jet,mu) < 0.4 :

if jet in cleaned_AK4:

cleaned_AK4.remove(jet)

  

if len(cut_tau)>0 and len(cut_jetAK8)>0:

for taus in cut_tau :

for jet in cut_jetAK8 :

if deltaR(taus,jet) > 0.4:

cleaned_AK8.append(jet)

  

if len(cut_tau)>0 and len(cut_subAK8 )>0:

for taus in cut_tau :

for jet in cut_subAK8 :

if deltaR(taus,jet) > 0.4:

cleaned_subAK8.append(jet)

if len(electrons) >0 :

for el in electrons:

if deltaR(jet,el) < 0.4:

if jet in cleaned_subAK8:

cleaned_subAK8.remove(jet)

if len(muons) >0 and len(electrons) ==0 :

for mu in muons:

if deltaR(jet,mu) < 0.4 :

if jet in cleaned_subAK8:

cleaned_subAK8.remove(jet)

  

## cleaned & cut

  

cut_tau.sort(key=lambda x : x.pt, reverse=True)

cleaned_AK4.sort(key=lambda x : x.pt, reverse=True)

cleaned_AK8.sort(key=lambda x : x.pt, reverse=True)

cleaned_subAK8.sort(key=lambda x : x.pt, reverse=True)

  

if len(cut_tau) > 0:

self.tau_cut_pt_leading.Fill(cut_tau[0].pt)

self.tau_cut_eta_leading.Fill(cut_tau[0].eta)

  

if len(cleaned_AK4) > 0:

self.jetAK4_cut_pt_leading.Fill(cleaned_AK4[0].pt)

self.jetAK4_cut_eta_leading.Fill(cleaned_AK4[0].eta)

if len(cleaned_AK4) > 1:

self.jetAK4_cut_pt_subleading.Fill(cleaned_AK4[1].pt)

self.jetAK4_cut_eta_subleading.Fill(cleaned_AK4[1].eta)

if len(cleaned_AK8) > 0:

self.jetAK8_cut_pt_leading.Fill(cleaned_AK8[0].pt)

self.jetAK8_cut_eta_leading.Fill(cleaned_AK8[0].eta)

if len(cleaned_subAK8)>0:

self.subjetAK8_cut_pt_leading.Fill(cleaned_subAK8[0].pt)

self.subjetAK8_cut_eta_leading.Fill(cleaned_subAK8[0].eta)

if len(cleaned_subAK8)>1:

self.subjetAK8_cut_pt_subleading.Fill(cleaned_subAK8[1].pt)

self.subjetAK8_cut_eta_subleading.Fill(cleaned_subAK8[1].eta)

  

cut_WR4_el =[]; cut_N4_el =[];cut_WR4_mu =[];cut_WR4_el=[];

cut_WR8 = []; cut_subWR8_el =[]; cut_subWR8_mu = [];

cut_subN8_el =[]; cut_subN8_mu=[];

if (len(cut_tau)>0 and len(cut_jetAK4)>1) :

if ( len( usingleptons) >0 ): # leptons 리스트를 순차적으로 순회

if abs(usingleptons[0].pdgId) == 11 :

cut_WR4_el = cut_tau[0].p4() + usingleptons[0].p4() + cut_jetAK4[0].p4() + cut_jetAK4[1].p4()

cut_N4_el = usingleptons[0].p4() + cut_jetAK4[0].p4() + cut_jetAK4[1].p4()

self.WR4_cut_pt_el.Fill(cut_WR4_el.Pt())

self.N4_cut_pt_el.Fill(cut_N4_el.Pt())

self.WR4_cut_eta_el.Fill(cut_WR4_el.Eta())

self.N4_cut_eta_el.Fill(cut_N4_el.Eta())

if abs(usingleptons[0].pdgId) == 13:

cut_WR4_mu = cut_tau[0].p4() + usingleptons[0].p4() + cut_jetAK4[0].p4() + cut_jetAK4[1].p4()

cut_N4_mu = usingleptons[0].p4() + cut_jetAK4[0].p4() + cut_jetAK4[1].p4()

self.WR4_cut_pt_mu.Fill(cut_WR4_mu.Pt())

self.N4_cut_pt_mu.Fill(cut_N4_mu.Pt())

self.WR4_cut_eta_mu.Fill(cut_WR4_mu.Eta())

self.N4_cut_eta_mu.Fill(cut_N4_mu.Eta())

  

if (len(cut_tau)>0 and len(cleaned_AK8)>0) :

cut_WR8 = cut_tau[0].p4() + cleaned_AK8[0].p4()

self.WR8_cut_pt.Fill(cut_WR8.Pt())

self.WR8_cut_eta.Fill(cut_WR8.Eta())

  

if (len(cut_tau)>0 and len(cleaned_subAK8)>1):

if ( len( usingleptons) >0 ):

if abs(usingleptons[0].pdgId) == 11 :

cut_subWR8_el = cut_tau[0].p4() + usingleptons[0].p4() + cleaned_subAK8[0].p4() + cleaned_subAK8[1].p4()

cut_subN8_el = usingleptons[0].p4() + cleaned_subAK8[0].p4() + cleaned_subAK8[1].p4()

self.subWR8_cut_pt_el.Fill(cut_subWR8_el.Pt())

self.subN8_cut_pt_el.Fill(cut_subN8_el.Pt())

self.subWR8_cut_eta_el.Fill(cut_subWR8_el.Eta())

self.subN8_cut_eta_el.Fill(cut_subN8_el.Eta())

if abs(usingleptons[0].pdgId) == 13:

cut_subWR8_mu = cut_tau[0].p4() + usingleptons[0].p4() + cleaned_subAK8[0].p4() + cleaned_subAK8[1].p4()

cut_subN8_mu = usingleptons[0].p4() + cleaned_subAK8[0].p4() + cleaned_subAK8[1].p4()

self.subWR8_cut_pt_mu.Fill(cut_subWR8_mu.Pt())

self.subN8_cut_pt_mu.Fill(cut_subN8_mu.Pt())

self.subWR8_cut_eta_mu.Fill(cut_subWR8_mu.Eta())

self.subN8_cut_eta_mu.Fill(cut_subN8_mu.Eta())

cut_WR4_el_mass =[]; cut_WR4_mu_mass =[]; cut_N4_el_mass = []; cut_N4_mu_mass =[];

cut_WR8_mass = []; cut_N8_mass = []; cut_subWR8_el_mass = []; cut_subWR8_mu_mass = [];

cut_subN8_el_mass = []; cut_subN8_mu_mass = [];

## mass

#AK4 cut mass

if len(cut_tau)>0:

if len(cleaned_AK4)>1 :

if ( len( usingleptons) >0 ):

if abs(usingleptons[0].pdgId) == 11 :

cut_WR4_el_mass = cut_tau[0].p4() + usingleptons[0].p4() + cleaned_AK4[0].p4() + cleaned_AK4[1].p4()

cut_N4_el_mass = usingleptons[0].p4() + cleaned_AK4[0].p4() + cleaned_AK4[1].p4()

self.tau_cut_mass_AK4_el.Fill(cut_WR4_el_mass.M())

self.cut_mass_AK4_el.Fill(cut_N4_el_mass.M())

if abs(usingleptons[0].pdgId) == 13:

cut_WR4_mu_mass = cut_tau[0].p4() + usingleptons[0].p4() + cleaned_AK4[0].p4() + cleaned_AK4[1].p4()

cut_N4_mu_mass = usingleptons[0].p4() + cleaned_AK4[0].p4() + cleaned_AK4[1].p4()

self.tau_cut_mass_AK4_mu.Fill(cut_WR4_mu_mass.M())

self.cut_mass_AK4_mu.Fill(cut_N4_mu_mass.M())

  

# AK8 cut mass

if (len(cut_tau)>0 and len(cleaned_AK8)>0) :

cut_WR8 = cut_tau[0].p4() + cleaned_AK8[0].p4()

self.tau_cut_mass_AK8_tau.Fill(cut_WR8.M())

if len(cleaned_AK8)>0:

cut_N8 = cleaned_AK8[0].p4()

self.cut_mass_AK8.Fill(cut_N8.M())

# SubAK8 cut mass

if len(cut_tau)>0:

if len(cleaned_subAK8)>1 :

if ( len( usingleptons) >0 ):

if abs(usingleptons[0].pdgId) == 11 :

cut_subWR8_el_mass = cut_tau[0].p4() + usingleptons[0].p4() + cleaned_subAK8[0].p4() + cleaned_subAK8[1].p4()

cut_subN8_el_mass = usingleptons[0].p4() + cleaned_subAK8[0].p4() + cleaned_subAK8[1].p4()

self.tau_cut_mass_subAK8_el.Fill(cut_subWR8_el_mass.M())

self.cut_mass_subAK8_el.Fill(cut_subN8_el_mass.M())

if abs(usingleptons[0].pdgId) == 13:

cut_subWR8_mu_mass = cut_tau[0].p4() + usingleptons[0].p4() + cleaned_subAK8[0].p4() + cleaned_subAK8[1].p4()

cut_subN8_mu_mass = usingleptons[0].p4() + cleaned_subAK8[0].p4() + cleaned_subAK8[1].p4()

self.tau_cut_mass_subAK8_mu.Fill(cut_subWR8_mu_mass.M())

self.cut_mass_subAK8_mu.Fill(cut_subN8_mu_mass.M())

  

  
  

  

  

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

2. *Using only taus*
```python 
#!/usr/bin/env python

# -*- coding: utf-8 -*-

from PhysicsTools.NanoAODTools.postprocessing.tools import *

from PhysicsTools.NanoAODTools.postprocessing.framework.eventloop import Module

from PhysicsTools.NanoAODTools.postprocessing.framework.datamodel import Collection,Object

from PhysicsTools.NanoAODTools.postprocessing.framework.postprocessor import PostProcessor

#from PhysicsTools.NanoAODTools.postprocessing.analyser.ID.GenStatus import *

#from PhysicsTools.NanoAODTools.postprocessing.analyser.AnalyserHelper.AnalyserHelper import *

from importlib import import_module

import os

import sys

import ROOT

import argparse

import linecache

ROOT.PyConfig.IgnoreCommandLineOptions = True

  
  

def Select(inputcoll,etamax,ptmin) :

output = []

for obj in inputcoll :

if abs(obj.eta) < etamax and obj.pt > ptmin : output.append(obj)

return output

def FlagIsOn(status,flag_n) :

if status & (1 << (flag_n -1)) : return True

else : return False

  

def isHardProcess(gen) : return FlagIsOn(gen.statusFlags,8)

def Process2(gen) : return FlagIsOn(gen.statusFlags,2)

def Process3(gen) : return FlagIsOn(gen.statusFlags,3)

def Process4(gen) : return FlagIsOn(gen.statusFlags,4)

def Process5(gen) : return FlagIsOn(gen.statusFlags,5)

  

class ExampleAnalysis(Module):

def __init__(self):

self.writeHistFile = True

  

def beginJob(self, histFile=None, histDirName=None):

Module.beginJob(self, histFile, histDirName)

####################before cut #################

# pt

self.addObject(ROOT.TH1F('tau_no_cut_pt_leading','tau_no_cut_pt_leading',400,0.,4000.))

self.addObject(ROOT.TH1F('jetAK4_no_cut_pt_leading','jetAK4_no_cut_pt_leading',400,0.,4000.))

self.addObject(ROOT.TH1F('jetAK4_no_cut_pt_subleading','jetAK4_no_cut_pt_subleading',400,0.,4000.))

self.addObject(ROOT.TH1F('jetAK8_no_cut_pt_leading','jetAK8_no_cut_pt_leading',400,0.,4000.)) ## N8 이랑 동일

self.addObject(ROOT.TH1F('subjetAK8_no_cut_pt_leading','subjetAK8_no_cut_pt_leading',400,0,4000))

self.addObject(ROOT.TH1F('subjetAK8_no_cut_pt_subleading','subjetAK8_no_cut_pt_subleading',400,0,4000))

self.addObject(ROOT.TH1F('WR4_no_cut_pt','WR4_no_cut_pt',400,0.,4000.))

self.addObject(ROOT.TH1F('N4_no_cut_pt','N4_no_cut_pt',400,0.,4000.))

self.addObject(ROOT.TH1F('WR8_no_cut_pt','WR8_no_cut_pt',400,0.,4000.))

self.addObject(ROOT.TH1F('subWR8_no_cut_pt','subWR8_no_cut_pt',400,0.,4000.))

self.addObject(ROOT.TH1F('subN8_no_cut_pt','subN8_no_cut_pt',400,0.,4000.))

  

  

# eta

self.addObject(ROOT.TH1F('tau_no_cut_eta_leading','tau_no_cut_eta_leading',80,-4.,4.))

self.addObject(ROOT.TH1F('jetAK4_no_cut_eta_leading','jetAK4_no_cut_eta_leading',80,-4.,4.))

self.addObject(ROOT.TH1F('jetAK4_no_cut_eta_subleading','jetAK4_no_cut_eta_subleading',80,-4.,4.))

self.addObject(ROOT.TH1F('jetAK8_no_cut_eta_leading','jetAK8_no_cut_eta_leading',80,-4.,4.))

self.addObject(ROOT.TH1F('subjetAK8_no_cut_eta_leading','subjetAK8_no_cut_eta_leading',80,-4.,4.))

self.addObject(ROOT.TH1F('subjetAK8_no_cut_eta_subleading','subjetAK8_no_cut_eta_subleading',80,-4.,4.))

self.addObject(ROOT.TH1F('WR4_no_cut_eta','WR4_no_cut_eta',80,-4.,4.))

self.addObject(ROOT.TH1F('N4_no_cut_eta','N4_no_cut_eta',80,-4.,4.))

self.addObject(ROOT.TH1F('WR8_no_cut_eta','WR8_no_cut_eta',80,-4.,4.))

self.addObject(ROOT.TH1F('subWR8_no_cut_eta','subWR8_no_cut_eta',80,-4.,4.))

self.addObject(ROOT.TH1F('subN8_no_cut_eta','subN8_no_cut_eta',80,-4.,4.))

#mass

self.addObject(ROOT.TH1F('no_cut_mass_AK4','tau_no_cut_mass_AK4',750,0.,7500.)) #N4

self.addObject(ROOT.TH1F('tau_no_cut_mass_AK4','tau_no_cut_mass_AK4',750,0.,7500.)) # WR4

self.addObject(ROOT.TH1F('tau_no_cut_mass_AK8_tau','tau_no_cut_mass_AK8_tau',750,0.,7500.)) ## WR8

self.addObject(ROOT.TH1F('no_cut_mass_AK8','no_cut_mass_AK8',750,0.,7500.)) ## N8

self.addObject(ROOT.TH1F('tau_no_cut_mass_subAK8','tau_no_cut_mass_subAK8',750,0.,7500.)) #sub W8

self.addObject(ROOT.TH1F('no_cut_mass_subAK8','no_cut_mass_subAK8',750,0.,7500.)) #sub N8

  

########################After cut######################

  

#pt

self.addObject(ROOT.TH1F('tau_cut_pt_leading','tau_cut_pt_leading',400,0.,4000.))

self.addObject(ROOT.TH1F('sectau_pt_leading','sectau_pt_leading',400,0.,4000.))

self.addObject(ROOT.TH1F('jetAK4_cut_pt_leading','jetAK4_cut_pt_leading',400,0.,4000.))

self.addObject(ROOT.TH1F('jetAK4_cut_pt_subleading','jetAK4_cut_pt_subleading',400,0.,4000.))

self.addObject(ROOT.TH1F('jetAK8_cut_pt_leading','jetAK8_cut_pt_leading',400,0.,4000.))

self.addObject(ROOT.TH1F('subjetAK8_cut_pt_leading','subjetAK8_cut_pt_leading',400,0,4000))

self.addObject(ROOT.TH1F('subjetAK8_cut_pt_subleading','subjetAK8_cut_pt_subleading',400,0,4000))

self.addObject(ROOT.TH1F('WR4_cut_pt','WR4_cut_pt',400,0.,4000.))

self.addObject(ROOT.TH1F('N4_cut_pt','N4_cut_pt',400,0.,4000.))

self.addObject(ROOT.TH1F('WR8_cut_pt','WR8_cut_pt',400,0.,4000.))

self.addObject(ROOT.TH1F('subWR8_cut_pt','subWR8_cut_pt',400,0.,4000.))

self.addObject(ROOT.TH1F('subN8_cut_pt','subN8_cut_pt',400,0.,4000.))

  
  
  
  

# eta

self.addObject(ROOT.TH1F('tau_cut_eta_leading','tau_cut_eta_leading',80,-4.,4.))

self.addObject(ROOT.TH1F('sectau_eta_leading','sectau_eta_leading',80,-4.,4.))

self.addObject(ROOT.TH1F('jetAK4_cut_eta_leading','jetAK4_cut_eta_leading',80,-4.,4.))

self.addObject(ROOT.TH1F('jetAK4_cut_eta_subleading','jetAK4_cut_eta_subleading',80,-4.,4.))

self.addObject(ROOT.TH1F('jetAK8_cut_eta_leading','jetAK8_cut_eta_leading',80,-4.,4.))

self.addObject(ROOT.TH1F('subjetAK8_cut_eta_leading','subjetAK8_cut_eta_leading',80,-4.,4.))

self.addObject(ROOT.TH1F('subjetAK8_cut_eta_subleading','subjetAK8_cut_eta_subleading',80,-4.,4.))

self.addObject(ROOT.TH1F('WR4_cut_eta','WR4_cut_eta',80,-4.,4.))

self.addObject(ROOT.TH1F('N4_cut_eta','N4_cut_eta',80,-4.,4.))

self.addObject(ROOT.TH1F('WR8_cut_eta','WR8_cut_eta',80,-4.,4.))

self.addObject(ROOT.TH1F('subWR8_cut_eta','subWR8_cut_eta',80,-4.,4.))

self.addObject(ROOT.TH1F('subN8_cut_eta','subN8_cut_eta',80,-4.,4.))

#mass

self.addObject(ROOT.TH1F('cut_mass_AK4','cut_mass_AK4',750,0.,7500.)) #N4

self.addObject(ROOT.TH1F('tau_cut_mass_AK4','tau_cut_mass_AK4',750,0.,7500.)) # WR4

self.addObject(ROOT.TH1F('tau_cut_mass_AK8_tau','tau_cut_mass_AK8_tau',750,0.,7500.)) ## WR8

self.addObject(ROOT.TH1F('cut_mass_AK8','tau_cut_mass_AK8',750,0.,7500.)) ## N4

self.addObject(ROOT.TH1F('cut_mass_subAK8','cut_mass_subAK8',750,0.,7500.)) # subN8

self.addObject(ROOT.TH1F('tau_cut_mass_subAK8','tau_cut_mass_subAK8',750,0.,7500.)) #sub W8

  
  

def analyze(self, event):

isNoCut = False

  

rawGenParts = Collection(event,"GenPart")

rawGenJetAK4 = Collection(event, "GenJet")

rawGenJetAK8 = Collection(event,"GenJetAK8")

rawGenVisTaus = Collection(event,"GenVisTau")

rawSubGenJetAK8 = Collection(event,"SubGenJetAK8")

muons = [] ; electrons =[];tausort=[];jetAK4sort=[];jetAK8sort=[];jetSubAK8sort=[];leptons=[];

sectaus=[];N =[];

for idx, rawGenPartss in enumerate(rawGenParts):

if rawGenPartss.pdgId == 9900016:

N.append(idx) # 객체 대신 인덱스를 저장

for rawGenParts in rawGenParts:

if abs(rawGenParts.pdgId) == 11:

electrons.append(rawGenParts)

leptons.append(rawGenParts)

if abs(rawGenParts.pdgId) == 13:

muons.append(rawGenParts)

leptons.append(rawGenParts)

if abs(rawGenParts.pdgId) == 15:

if rawGenParts.genPartIdxMother in N :

print ("has a N tau")

sectaus.append(rawGenParts)

## pt sorting

electrons.sort(key=lambda x : x.pt, reverse=True)

muons.sort(key=lambda x : x.pt, reverse=True)

leptons.sort(key=lambda x : x.pt, reverse=True)

sectaus.sort(key=lambda x : x.pt, reverse=True)

  

usingleptons = [];

  

for lepton in leptons :

if Process2(lepton) or Process3(lepton) or Process4(lepton) or Process5(lepton): # tau product

usingleptons.append(lepton)

usingleptons.sort(key=lambda x : x.pt , reverse= True)

  
  

'''''

i=0

for lepton in leptons:

i+=1

if Process2(lepton) or Process3(lepton) or Process4(lepton) or Process5(lepton): # tau product

print (i,"nd Pt uses tau particle")

break

'''

  
  

### no cut pt eta

## pt sorting

for rawGenVisTaus in rawGenVisTaus:

tausort.append(rawGenVisTaus)

for rawGenJetAK4 in rawGenJetAK4:

jetAK4sort.append(rawGenJetAK4)

for rawGenJetAK8 in rawGenJetAK8:

jetAK8sort.append(rawGenJetAK8)

  

for rawSubGenJetAK8 in rawSubGenJetAK8:

jetSubAK8sort.append(rawSubGenJetAK8)

tausort.sort(key=lambda x : x.pt, reverse=True)

jetAK4sort.sort(key=lambda x : x.pt, reverse=True)

jetAK8sort.sort(key=lambda x : x.pt, reverse=True)

jetSubAK8sort.sort(key=lambda x : x.pt, reverse=True)

  

if len(tausort) > 0:

self.tau_no_cut_pt_leading.Fill(tausort[0].pt)

self.tau_no_cut_eta_leading.Fill(tausort[0].eta)

if len(sectaus) > 0:

self.sectau_pt_leading.Fill(sectaus[0].pt)

self.sectau_eta_leading.Fill(sectaus[0].eta)

if len(jetAK4sort) > 0:

self.jetAK4_no_cut_pt_leading.Fill(jetAK4sort[0].pt)

self.jetAK4_no_cut_eta_leading.Fill(jetAK4sort[0].eta)

if len(jetAK4sort) > 1:

self.jetAK4_no_cut_pt_subleading.Fill(jetAK4sort[1].pt)

self.jetAK4_no_cut_eta_subleading.Fill(jetAK4sort[1].eta)

if len(jetAK8sort) > 0:

self.jetAK8_no_cut_pt_leading.Fill(jetAK8sort[0].pt)

self.jetAK8_no_cut_eta_leading.Fill(jetAK8sort[0].eta)

if len(jetSubAK8sort) > 0:

self.subjetAK8_no_cut_pt_leading.Fill(jetSubAK8sort[0].pt)

self.subjetAK8_no_cut_eta_leading.Fill(jetSubAK8sort[0].eta)

if len(jetSubAK8sort) > 1:

self.subjetAK8_no_cut_pt_subleading.Fill(jetSubAK8sort[1].pt)

self.subjetAK8_no_cut_eta_subleading.Fill(jetSubAK8sort[1].eta)

WR4 =[]; N4 =[];WR4_mu =[];WR4=[];

WR8 = []; subWR8 =[]; subWR8_mu = [];

subN8 =[]; subN8_mu=[];

if (len(tausort)>0 and len(jetAK4sort)>1) :

if ( len( sectaus) >0 ):

WR4 = tausort[0].p4() + sectaus[0].p4() + jetAK4sort[0].p4() + jetAK4sort[1].p4()

N4 = sectaus[0].p4() + jetAK4sort[0].p4() + jetAK4sort[1].p4()

self.WR4_no_cut_pt.Fill(WR4.Pt())

self.N4_no_cut_pt.Fill(N4.Pt())

self.WR4_no_cut_eta.Fill(WR4.Eta())

self.N4_no_cut_eta.Fill(N4.Eta())

  

if (len(tausort)>0 and len(jetAK8sort)>0) :

WR8 = tausort[0].p4() + jetAK8sort[0].p4()

self.WR8_no_cut_pt.Fill(WR8.Pt())

self.WR8_no_cut_eta.Fill(WR8.Eta())

  

if (len(tausort)>0 and len(jetSubAK8sort)>1):

if ( len( sectaus) >0 ):

subWR8 = tausort[0].p4() + sectaus[0].p4() + jetSubAK8sort[0].p4() + jetSubAK8sort[1].p4()

subN8 = sectaus[0].p4() + jetSubAK8sort[0].p4() + jetSubAK8sort[1].p4()

self.subWR8_no_cut_pt.Fill(subWR8.Pt())

self.subN8_no_cut_pt.Fill(subN8.Pt())

self.subWR8_no_cut_eta.Fill(subWR8.Eta())

self.subN8_no_cut_eta.Fill(subN8.Eta())

  

WR4_mass =[]; WR4_mu_mass =[]; N4_mass = []; N4_mu_mass =[];

WR8_mass = []; N8_mass = []; subWR8_mass = []; subWR8_mu_mass = [];

subN8_mass = []; subN8_mu_mass = [];

## mass

#AK4 no cut mass

if len(tausort)>0:

if len(jetAK4sort)>1 :

if ( len( sectaus) >0 ):

WR4_mass = tausort[0].p4() +sectaus[0].p4() + jetAK4sort[0].p4() + jetAK4sort[1].p4()

N4_mass = sectaus[0].p4() + jetAK4sort[0].p4() + jetAK4sort[1].p4()

self.tau_no_cut_mass_AK4.Fill(WR4_mass.M())

self.no_cut_mass_AK4.Fill(N4_mass.M())

  

# AK8 no cut mass

if (len(tausort)>0 and len(jetAK8sort)>0) :

WR8_mass = tausort[0].p4() + jetAK8sort[0].p4()

self.tau_no_cut_mass_AK8_tau.Fill(WR8_mass.M())

if len(jetAK8sort)>0:

N8_mass = jetAK8sort[0].p4()

self.no_cut_mass_AK8.Fill(N8_mass.M())

# SubAK8 no cut mass

if len(tausort)>0:

if len(jetSubAK8sort)>1:

if ( len( sectaus) >0 ):

subWR8_mass = tausort[0].p4() + sectaus[0].p4() + jetSubAK8sort[0].p4() + jetSubAK8sort[1].p4()

subN8_mass = sectaus[0].p4() + jetSubAK8sort[0].p4() + jetSubAK8sort[1].p4()

self.tau_no_cut_mass_subAK8.Fill(subWR8_mass.M())

self.no_cut_mass_subAK8.Fill(subN8_mass.M())

################################################################

################################################################

  

## obj cut

cut_tau =[]

cut_jetAK4 = []

cut_jetAK8 = []

cut_subAK8 = []

if len(tausort)>0 :

cut_tau = Select(tausort,2.1,190)

if len(jetAK4sort)>0:

cut_jetAK4 = Select(jetAK4sort,2.4,40)

if len(jetAK8sort)>0:

cut_jetAK8 = Select(jetAK8sort,2.4,200)

if len(jetSubAK8sort)>0:

cut_subAK8 = Select(jetSubAK8sort,2.4,40)

## cleaning

cleaned_AK4=[]

cleaned_AK8=[]

cleaned_subAK8=[]

  

if len(cut_tau)>0 and len(cut_jetAK4 )>0:

for taus in cut_tau :

for jet in cut_jetAK4 :

if deltaR(taus,jet) > 0.4:

cleaned_AK4.append(jet)

if len(electrons) >0 :

for el in electrons:

if deltaR(jet,el) < 0.4:

if jet in cleaned_AK4:

cleaned_AK4.remove(jet)

if len(muons) >0 and len(electrons) ==0 :

for mu in muons:

if deltaR(jet,mu) < 0.4 :

if jet in cleaned_AK4:

cleaned_AK4.remove(jet)

  

if len(cut_tau)>0 and len(cut_jetAK8)>0:

for taus in cut_tau :

for jet in cut_jetAK8 :

if deltaR(taus,jet) > 0.4:

cleaned_AK8.append(jet)

  

if len(cut_tau)>0 and len(cut_subAK8 )>0:

for taus in cut_tau :

for jet in cut_subAK8 :

if deltaR(taus,jet) > 0.4:

cleaned_subAK8.append(jet)

if len(electrons) >0 :

for el in electrons:

if deltaR(jet,el) < 0.4:

if jet in cleaned_subAK8:

cleaned_subAK8.remove(jet)

if len(muons) >0 and len(electrons) ==0 :

for mu in muons:

if deltaR(jet,mu) < 0.4 :

if jet in cleaned_subAK8:

cleaned_subAK8.remove(jet)

  

## cleaned & cut

  

cut_tau.sort(key=lambda x : x.pt, reverse=True)

cleaned_AK4.sort(key=lambda x : x.pt, reverse=True)

cleaned_AK8.sort(key=lambda x : x.pt, reverse=True)

cleaned_subAK8.sort(key=lambda x : x.pt, reverse=True)

  

if len(cut_tau) > 0:

self.tau_cut_pt_leading.Fill(cut_tau[0].pt)

self.tau_cut_eta_leading.Fill(cut_tau[0].eta)

  

if len(cleaned_AK4) > 0:

self.jetAK4_cut_pt_leading.Fill(cleaned_AK4[0].pt)

self.jetAK4_cut_eta_leading.Fill(cleaned_AK4[0].eta)

if len(cleaned_AK4) > 1:

self.jetAK4_cut_pt_subleading.Fill(cleaned_AK4[1].pt)

self.jetAK4_cut_eta_subleading.Fill(cleaned_AK4[1].eta)

if len(cleaned_AK8) > 0:

self.jetAK8_cut_pt_leading.Fill(cleaned_AK8[0].pt)

self.jetAK8_cut_eta_leading.Fill(cleaned_AK8[0].eta)

if len(cleaned_subAK8)>0:

self.subjetAK8_cut_pt_leading.Fill(cleaned_subAK8[0].pt)

self.subjetAK8_cut_eta_leading.Fill(cleaned_subAK8[0].eta)

if len(cleaned_subAK8)>1:

self.subjetAK8_cut_pt_subleading.Fill(cleaned_subAK8[1].pt)

self.subjetAK8_cut_eta_subleading.Fill(cleaned_subAK8[1].eta)

  

cut_WR4 =[]; cut_N4 =[];cut_WR4_mu =[];cut_WR4=[];

cut_WR8 = []; cut_subWR8 =[]; cut_subWR8_mu = [];

cut_subN8 =[]; cut_subN8_mu=[];

if (len(cut_tau)>0 and len(cut_jetAK4)>1) :

if ( len(sectaus) >0 ): # leptons 리스트를 순차적으로 순회

cut_WR4 = cut_tau[0].p4() + sectaus[0].p4() + cut_jetAK4[0].p4() + cut_jetAK4[1].p4()

cut_N4 = sectaus[0].p4() + cut_jetAK4[0].p4() + cut_jetAK4[1].p4()

self.WR4_cut_pt.Fill(cut_WR4.Pt())

self.N4_cut_pt.Fill(cut_N4.Pt())

self.WR4_cut_eta.Fill(cut_WR4.Eta())

self.N4_cut_eta.Fill(cut_N4.Eta())

  

if (len(cut_tau)>0 and len(cleaned_AK8)>0) :

cut_WR8 = cut_tau[0].p4() + cleaned_AK8[0].p4()

self.WR8_cut_pt.Fill(cut_WR8.Pt())

self.WR8_cut_eta.Fill(cut_WR8.Eta())

  

if (len(cut_tau)>0 and len(cleaned_subAK8)>1):

if ( len( sectaus) >0 ):

cut_subWR8 = cut_tau[0].p4() + sectaus[0].p4() + cleaned_subAK8[0].p4() + cleaned_subAK8[1].p4()

cut_subN8 = sectaus[0].p4() + cleaned_subAK8[0].p4() + cleaned_subAK8[1].p4()

self.subWR8_cut_pt.Fill(cut_subWR8.Pt())

self.subN8_cut_pt.Fill(cut_subN8.Pt())

self.subWR8_cut_eta.Fill(cut_subWR8.Eta())

self.subN8_cut_eta.Fill(cut_subN8.Eta())

cut_WR4_mass =[]; cut_WR4_mu_mass =[]; cut_N4_mass = []; cut_N4_mu_mass =[];

cut_WR8_mass = []; cut_N8_mass = []; cut_subWR8_mass = []; cut_subWR8_mu_mass = [];

cut_subN8_mass = []; cut_subN8_mu_mass = [];

## mass

#AK4 cut mass

if len(cut_tau)>0:

if len(cleaned_AK4)>1 :

if ( len(sectaus) >0 ):

cut_WR4_mass = cut_tau[0].p4() + sectaus[0].p4() + cleaned_AK4[0].p4() + cleaned_AK4[1].p4()

cut_N4_mass = sectaus[0].p4() + cleaned_AK4[0].p4() + cleaned_AK4[1].p4()

self.tau_cut_mass_AK4.Fill(cut_WR4_mass.M())

self.cut_mass_AK4.Fill(cut_N4_mass.M())

  

# AK8 cut mass

if (len(cut_tau)>0 and len(cleaned_AK8)>0) :

cut_WR8 = cut_tau[0].p4() + cleaned_AK8[0].p4()

self.tau_cut_mass_AK8_tau.Fill(cut_WR8.M())

if len(cleaned_AK8)>0:

cut_N8 = cleaned_AK8[0].p4()

self.cut_mass_AK8.Fill(cut_N8.M())

# SubAK8 cut mass

if len(cut_tau)>0:

if len(cleaned_subAK8)>1 :

if ( len( sectaus) >0 ):

cut_subWR8_mass = cut_tau[0].p4() + sectaus[0].p4() + cleaned_subAK8[0].p4() + cleaned_subAK8[1].p4()

cut_subN8_mass = sectaus[0].p4() + cleaned_subAK8[0].p4() + cleaned_subAK8[1].p4()

self.tau_cut_mass_subAK8.Fill(cut_subWR8_mass.M())

self.cut_mass_subAK8.Fill(cut_subN8_mass.M())

  

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