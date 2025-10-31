#!/usr/bin/env bash

python plot.py --x-range "0,2000" --rebin 15 --hist-name "DYCR_Resolved_MM_leading_jet_pt" --output "DY_leading_pt_2022" --x-title "p_{T} of jet (GeV)" --logy --y-range "0.7,1.3"
python plot.py --x-range "0,1000" --rebin 8 --hist-name "DYCR_Resolved_MM_pt" --output "DY_Dilepton_pt_2022" --x-title 'p_{T}^{ll}(GeV)' --logy --y-range "0.7,1.3"
python plot.py --x-range "0,500" --rebin 5 --hist-name "DYCR_Resolved_MM_subleading_jet_pt" --output "DY_subleading_pt_2022" --x-title "p_{T} of the subleading jet (GeV)" --logy --y-range "0.7,1.3"
python plot.py --x-range "0,8000" --rebin 10 --hist-name "DYCR_Resolved_MM_mlljj" --output "DY_mlljj_2022" --x-title "mlljj" --logy --y-range "0.7,1.3"