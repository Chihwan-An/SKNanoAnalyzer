import ROOT
from Plotter import ComparisonCanvas
ROOT.gROOT.SetBatch(True)

# set global variables
histkey = "Central/DileptonMass"
DATASTREAM = ["Muon"]
MCs = ["TTLL_powheg", "WW_pythia", "WZ_pythia","ZZ_pythia", "DYJets","DYJets10to50","ST_sch_top_Lep","ST_tW_top_Semilep","ST_tW_antitop_Semilep","ST_tW_top_Lep","ST_tW_antitop_Lep"]


# set config
config = {
    "xTitle": "M(#mu^{+}#mu^{-} [GeV]",
    "yTitle": "Events",
    "xRange": [70, 110],
    "yRange": [0.5, 1.5],
    # "rebin": 5,
    "logy": False,
    "era": "2023"
}

# get histograms
BKGs = {}
COLORs = {}

# get histograms
def get_hist(sample, histkey):
    if sample == "data":
        fkey = f"/gv0/Users/$USER/SKNanoOutput/DY/2023/{DATASTREAM}.root"
    else:
        fkey = f"/gv0/Users/$USER/SKNanoOutput/DY/2023/{sample}.root"
    #print(fkey)
    #print(histkey)
    f = ROOT.TFile.Open(fkey)
    h = f.Get(histkey); h.SetDirectory(0)
    f.Close()
    return h
data = get_hist("data", histkey)
for mc in MCs:
    h = get_hist(mc, histkey)
    BKGs[mc] = h.Clone(mc)

COLORs["data"] = ROOT.kBlack
COLORs["TTLL"] = ROOT.kBlue
COLORs["VV"] = ROOT.kGreen
COLORs["ST"] = ROOT.kViolet
COLORs["DY"] = ROOT.kGray

c = ComparisonCanvas(config=config)
c.drawBackgrounds(BKGs, COLORs)
c.drawData(data)
c.drawRatio()
c.drawLegend()
c.finalize()
c.savefig("result.png")