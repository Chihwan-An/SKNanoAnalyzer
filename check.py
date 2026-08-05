from importlib import import_module
import os
import sys
import argparse
import linecache
import uproot
import vector
import math
import numpy as np
import matplotlib.pyplot as plt
import awkward as ak
from tqdm import tqdm  # ✅ 진행률 표시
import glob
import json
vector.register_awkward()



sample = "/gv0/Users/youngwan_public/hichihwan/WRtoNTautoTauTauJJ_MWR2000_N200_TuneCP5_13p6TeV_madgraph-pythia8/WRtoNTautoTauTauJJ_MWR2000_N200_TuneCP5_13p6TeV_madgraph-pythia8/chain_RunIII2024Summer24wmLHEGS-RunIII2024Summer24NanoAODv15/20260710_173921/RunIII2024Summer24NanoAODv15_0.root"

file = uproot.open(sample)


events = file["Events"]

runs = file["Runs"]
keys = events.keys()

print(keys)