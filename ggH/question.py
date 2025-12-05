from coffea.nanoevents import NanoEventsFactory, NanoAODSchema
import awkward as ak
import vector
vector.register_awkward()
NanoAODSchema.warn_missing_crossrefs = False

file = '/gv0/DATA/SKNano/Run3NanoAODv13p1/2022/DYto2L-4Jets_MLL-50_TuneCP5_13p6TeV_madgraphMLM-pythia8/250617_125929/0000/NANOAOD_38.root'

events = NanoEventsFactory.from_root(
    {file: "Events"},
    schemaclass=NanoAODSchema,
).events()

# 데이터 실제로 로드
muon = events.Muon
muon_pt = events.Muon.pt
over2nmuons = ak.num(muon_pt) > 1
muon_pt = muon_pt[over2nmuons]
#print(ak.to_list(muon_pt)) 

z = muon[over2nmuons][:,0] + muon[over2nmuons][:,1]
properz = (z.mass > 60) & (z.mass < 120)
#print(ak.to_list(z.mass)) 


lead_propermuon = muon_pt[properz][:,0]
sublead_propermuon = muon_pt[properz][:,1]
delta_pt = lead_propermuon - sublead_propermuon

# save with plot png file

import matplotlib.pyplot as plt
plt.hist(ak.to_list(delta_pt), bins=50, range=(0, 250), histtype='stepfilled', alpha=0.7)
plt.hist(ak.to_list(lead_propermuon), bins=50, range=(0, 250), histtype='step', color='red', label='Leading Muon Pt')
plt.hist(ak.to_list(sublead_propermuon), bins=50, range=(0, 250), histtype='step', color='blue', label='Subleading Muon Pt')
plt.legend()
plt.yscale('log')
plt.xlabel('Delta Pt (GeV)')
plt.ylabel('Number of Events')
plt.title('Histogram of Delta Pt between Leading and Subleading Muons')
plt.grid(True)
plt.savefig('/data6/Users/achihwan/SKNanoAnalyzer/ggH/delta_pt_histogram.png')
plt.figure()

plt.hist(ak.to_list(z.pt),  bins=50, range=(0, 250), histtype='step', color='red', label='Z Pt')
plt.legend()
plt.yscale('log')
plt.xlabel('Delta Pt (GeV)')
plt.ylabel('Number of Events')
plt.grid(True)
plt.savefig('/data6/Users/achihwan/SKNanoAnalyzer/ggH/Zpt.png')

#print(ak.to_list(lead_propermuon))
#print(ak.to_list(sublead_propermuon)) 

