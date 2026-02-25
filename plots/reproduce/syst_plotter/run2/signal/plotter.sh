#!/usr/bin/env bash

python cutflow.py --input /gv0/Users/achihwan/SKNanoRunlog/2026_02_19_150936_Reproduce20_002_copy/2017/4000100mm/output/hists_0.root --output cutflow_mumu_Boosted_SR
python cutflow.py --input /gv0/Users/achihwan/SKNanoRunlog/2026_02_19_150936_Reproduce20_002_copy/2017/40002100mm/output/hists_0.root --output cutflow_mumu_resolved_SR
python cutflow.py --input /gv0/Users/achihwan/SKNanoRunlog/2026_02_19_150936_Reproduce20_002_copy/2017/4000100ee/output/hists_0.root --output cutflow_ee_Boosted_SR
python cutflow.py --input /gv0/Users/achihwan/SKNanoRunlog/2026_02_19_150936_Reproduce20_002_copy/2017/40002100ee/output/hists_0.root --output cutflow_ee_resolved_SR
