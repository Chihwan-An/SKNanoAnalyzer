#!/usr/bin/env bash

python plot_MM.py --x-range "0,2000" --rebin 20 --hist-name "CentralFlavorCR_Resolved_leading_jet_pt" --output "DY_MM_leading_pt_2022EE" --x-title "p_{T} of jet (GeV)" --logy --y-range "0.7,1.8" --ymax 1e4 --ymin 3
python plot_MM.py --x-range "0,1000" --rebin 8 --hist-name "CentralFlavorCR_Resolved_ll_pt" --output "DY_MM_Dilepton_pt_2022EE" --x-title 'p_{T}^{ll}(GeV)' --logy --y-range "0.7,1.8" --ymax 1e6 --ymin 50
python plot_MM.py --x-range "0,500" --rebin 10 --hist-name "CentralFlavorCR_Resolved_subleading_jet_pt" --output "DY_MM_subleading_pt_2022EE" --x-title "p_{T} of the subleading jet (GeV)" --logy --y-range "0.7,1.8" --ymax 1e4 --ymin 3
python plot_MM.py --x-range "800,8000" --rebin 10 --hist-name "CentralFlavorCR_Resolved_mlljj" --output "DY_MM_mlljj_2022EE" --x-title "mlljj" --logy --y-range "0.0,2.5" --custom-bins '800,1000,1200,1400,1600,2000,2400,2800,3200,8000' --ymax 3e3  --ymin 2

