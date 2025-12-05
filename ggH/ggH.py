import glob
from coffea.nanoevents import NanoEventsFactory, NanoAODSchema
import awkward as ak
import vector
vector.register_awkward()

NanoAODSchema.warn_missing_crossrefs = False

file = '/gv0/Users/achihwan/SKNano/ggHtogamgam/44BD3ABD-AE09-4C46-BB4E-D16BEE4244EE.root'

events = NanoEventsFactory.from_root(
    {file: "Events"},
    schemaclass=NanoAODSchema,
).events()

photon = events.Photon
photon_pt = events.Photon.pt
hastophotons = ak.num(photon_pt) > 1
photon = photon[hastophotons]
haspropermass = ((photon[:,0] + photon[:,1]).mass > 100) &(photon[:,0].pt > 35 )&(photon[:,1].pt>25) 
photon = photon[haspropermass]
lead_photon = photon[:,0]
sublead_photon = photon[:,1]
dpt_photon = lead_photon.pt - sublead_photon.pt

import matplotlib.pyplot as plt
plt.hist(ak.to_list(dpt_photon), bins=50, range=(0, 250), histtype='stepfilled', alpha=0.7)
plt.hist(ak.to_list(lead_photon.pt), bins=50, range=(0, 250), histtype='step', color='red', label='Leading Photon Pt')
plt.hist(ak.to_list(sublead_photon.pt), bins=50, range=(0, 250), histtype='step', color='blue', label='Subleading Photon Pt')

plt.legend()
plt.yscale('log')
plt.xlabel('Delta Pt (GeV)')
plt.savefig('/data6/Users/achihwan/SKNanoAnalyzer/ggH/dpt_photon_histogram.png')
plt.figure()

plt.hist(ak.to_list((lead_photon+sublead_photon).mass) ,bins=50, range=(0, 500), histtype='stepfilled', alpha=0.7)
plt.hist(ak.to_list((lead_photon + sublead_photon).pt),bins=50, range=(0, 250), histtype='step', color='red', label='Zpt')
plt.legend()
plt.yscale('log')
plt.xlabel('Delta Pt (GeV)')
plt.savefig('/data6/Users/achihwan/SKNanoAnalyzer/ggH/mass_photon_histogram.png')