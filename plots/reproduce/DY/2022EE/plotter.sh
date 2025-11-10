#!/usr/bin/env bash

python plot_MM.py --x-range "0,2000" --rebin 15 --hist-name "DYCR_Resolved_MM_leading_jet_pt" --output "DY_MM_leading_pt_2022EE" --x-title "p_{T} of jet (GeV)" --logy --y-range "0.7,1.8" --ymax 1e4 --ymin 10
python plot_MM.py --x-range "0,1000" --rebin 8 --hist-name "DYCR_Resolved_MM_pt" --output "DY_MM_Dilepton_pt_2022EE" --x-title 'p_{T}^{ll}(GeV)' --logy --y-range "0.7,1.8" --ymax 1e6 --ymin 100
python plot_MM.py --x-range "0,500" --rebin 10 --hist-name "DYCR_Resolved_MM_subleading_jet_pt" --output "DY_MM_subleading_pt_2022EE" --x-title "p_{T} of the subleading jet (GeV)" --logy --y-range "0.7,1.8" --ymax 1e4 --ymin 10
python plot_MM.py --x-range "0,8000" --rebin 10 --hist-name "DYCR_Resolved_MM_mlljj" --output "DY_MM_mlljj_2022EE" --x-title "mlljj" --logy --y-range "0.7,1.8" 

python plot_EE.py --x-range "0,2000" --rebin 15 --hist-name "DYCR_Resolved_EE_leading_jet_pt" --output "DY_EE_leading_pt_2022EE" --x-title "p_{T} of jet (GeV)" --logy --y-range "0.7,1.8" --ymax 1e4 --ymin 10
python plot_EE.py --x-range "0,1000" --rebin 8 --hist-name "DYCR_Resolved_EE_pt" --output "DY_EE_Dilepton_pt_2022EE" --x-title 'p_{T}^{ll}(GeV)' --logy --y-range "0.7,1.8" --ymax 1e6 --ymin 100
python plot_EE.py --x-range "0,500" --rebin 10 --hist-name "DYCR_Resolved_EE_subleading_jet_pt" --output "DY_EE_subleading_pt_2022EE" --x-title "p_{T} of the subleading jet (GeV)" --logy --y-range "0.7,1.8" --ymax 1e4 --ymin 10
python plot_EE.py --x-range "0,8000" --rebin 10 --hist-name "DYCR_Resolved_EE_mlljj" --output "DY_EE_mlljj_2022EE" --x-title "mlljj" --logy --y-range "0.7,1.8"