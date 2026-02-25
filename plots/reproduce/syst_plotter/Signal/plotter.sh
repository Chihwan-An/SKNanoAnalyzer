#!/usr/bin/env bash

python cutflow.py --input /gv0/Users/achihwan/SKNanoRunlog/2026_02_10_143028_Reproduce20_002_copy/2023/WRtoNMutoMuMuJJ_MWR4000_N100/output/hists_0.root --output cutflow_mumu_Boosted_SR
python cutflow.py --input /gv0/Users/achihwan/SKNanoRunlog/2026_02_10_143028_Reproduce20_002_copy/2023/WRtoNMutoMuMuJJ_MWR4000_N2100/output/hists_0.root --output cutflow_mumu_resolved_SR
python cutflow.py --input /gv0/Users/achihwan/SKNanoRunlog/2026_02_10_143028_Reproduce20_002_copy/2023/WRtoNEltoElElJJ_MWR4000_N100/output/hists_0.root --output cutflow_ee_Boosted_SR
python cutflow.py --input /gv0/Users/achihwan/SKNanoRunlog/2026_02_10_143028_Reproduce20_002_copy/2023/WRtoNEltoElElJJ_MWR4000_N2100/output/hists_0.root --output cutflow_ee_resolved_SR
