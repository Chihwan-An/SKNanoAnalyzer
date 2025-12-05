#!/usr/bin/env bash

python plot_MM.py --x-range "0,2000" --rebin 20 --hist-name "DYCR_Resolved_MM_leading_jet_pt" --output "DYCR_MM_leading_pt_2022EE" --x-title "p_{T} of jet (GeV)" --logy --y-range "0.7,1.3" --ymax 1e4 --ymin 3
python plot_MM.py --x-range "0,1000" --rebin 8 --hist-name "DYCR_Resolved_MM_pt" --output "DYCR_MM_Dilepton_pt_2022EE" --x-title 'p_{T}^{ll}(GeV)' --logy --y-range "0.7,1.3" --ymax 1e6 --ymin 50
python plot_MM.py --x-range "0,500" --rebin 10 --hist-name "DYCR_Resolved_MM_subleading_jet_pt" --output "DYCR_MM_subleading_pt_2022EE" --x-title "p_{T} of the subleading jet (GeV)" --logy --y-range "0.7 ,1.3" --ymax 1e4 --ymin 3
python plot_MM.py --x-range "0,8000" --rebin 10 --hist-name "DYCR_Resolved_MM_mlljj" --output "DYCR_MM_mlljj_2022EE" --x-title "mlljj" --logy --y-range "0.7,1.3"   

python plot_EE.py --x-range "0,2000" --rebin 20 --hist-name "DYCR_Resolved_EE_leading_jet_pt" --output "DYCR_EE_leading_pt_2022EE" --x-title "p_{T} of jet (GeV)" --logy --y-range "0.7,1.3" --ymax 1e4 --ymin 3
python plot_EE.py --x-range "0,1000" --rebin 8 --hist-name "DYCR_Resolved_EE_pt" --output "DYCR_EE_Dilepton_pt_2022EE" --x-title 'p_{T}^{ll}(GeV)' --logy --y-range "0.7,1.3" --ymax 1e6 --ymin 50
python plot_EE.py --x-range "0,500" --rebin 10 --hist-name "DYCR_Resolved_EE_subleading_jet_pt" --output "DYCR_EE_subleading_pt_2022EE" --x-title "p_{T} of the subleading jet (GeV)" --logy --y-range "0.7,1.3" --ymax 1e4 --ymin 3
python plot_EE.py --x-range "0,8000" --rebin 10 --hist-name "DYCR_Resolved_EE_mlljj" --output "DYCR_EE_mlljj_2022EE" --x-title "mlljj" --logy --y-range "0.7,1.3"   


python plot_MM.py --x-range "0,2000" --rebin 20 --hist-name "DYCR1_Resolved_MM_leading_jet_pt" --output "DYCR1_MM_leading_pt_2022EE" --x-title "p_{T} of jet (GeV)" --logy --y-range "0.7,1.3" --ymax 1e4 --ymin 3
python plot_MM.py --x-range "0,1000" --rebin 8 --hist-name "DYCR1_Resolved_MM_ll_pt" --output "DYCR1_MM_Dilepton_pt_2022EE" --x-title 'p_{T}^{ll}(GeV)' --logy --y-range "0.7,1.3" --ymax 1e6 --ymin 50
python plot_MM.py --x-range "0,500" --rebin 10 --hist-name "DYCR1_Resolved_MM_subleading_jet_pt" --output "DYCR1_MM_subleading_pt_2022EE" --x-title "p_{T} of the subleading jet (GeV)" --logy --y-range "0.7,1.3" --ymax 1e4 --ymin 3
python plot_MM.py --x-range "0,8000" --rebin 10 --hist-name "DYCR1_Resolved_MM_mlljj" --output "DYCR1_MM_mlljj_2022EE" --x-title "mlljj" --logy --y-range "0.7,1.3" 

python plot_MM.py --x-range "0,2000" --rebin 20 --hist-name "DYCR2_Resolved_MM_leading_jet_pt" --output "DYCR2_MM_leading_pt_2022EE" --x-title "p_{T} of jet (GeV)" --logy --y-range "0.7,1.3" --ymax 1e4 --ymin 3
python plot_MM.py --x-range "0,1000" --rebin 8 --hist-name "DYCR2_Resolved_MM_ll_pt" --output "DYCR2_MM_Dilepton_pt_2022EE" --x-title 'p_{T}^{ll}(GeV)' --logy --y-range "0.7,1.3" --ymax 1e6 --ymin 50
python plot_MM.py --x-range "0,500" --rebin 10 --hist-name "DYCR2_Resolved_MM_subleading_jet_pt" --output "DYCR2_MM_subleading_pt_2022EE" --x-title "p_{T} of the subleading jet (GeV)" --logy --y-range "0.7,1.3" --ymax 1e4 --ymin 3
python plot_MM.py --x-range "0,8000" --rebin 10 --hist-name "DYCR2_Resolved_MM_mlljj" --output "DYCR2_MM_mlljj_2022EE" --x-title "mlljj" --logy --y-range "0.7,1.3"

python plot_EE.py --x-range "0,2000" --rebin 20 --hist-name "DYCR1_Resolved_EE_leading_jet_pt" --output "DYCR1_EE_leading_pt_2022EE" --x-title "p_{T} of jet (GeV)" --logy --y-range "0.7,1.3" --ymax 1e4 --ymin 3
python plot_EE.py --x-range "0,1000" --rebin 8 --hist-name "DYCR1_Resolved_EE_ll_pt" --output "DYCR1_EE_Dilepton_pt_2022EE" --x-title 'p_{T}^{ll}(GeV)' --logy --y-range "0.7,1.3" --ymax 1e6 --ymin 50
python plot_EE.py --x-range "0,500" --rebin 10 --hist-name "DYCR1_Resolved_EE_subleading_jet_pt" --output "DYCR1_EE_subleading_pt_2022EE" --x-title "p_{T} of the subleading jet (GeV)" --logy --y-range "0.7,1.3" --ymax 1e4 --ymin 3
python plot_EE.py --x-range "0,8000" --rebin 10 --hist-name "DYCR1_Resolved_EE_mlljj" --output "DYCR1_EE_mlljj_2022EE" --x-title "mlljj" --logy --y-range "0.7,1.3"

python plot_EE.py --x-range "0,2000" --rebin 20 --hist-name "DYCR2_Resolved_EE_leading_jet_pt" --output "DYCR2_EE_leading_pt_2022EE" --x-title "p_{T} of jet (GeV)" --logy --y-range "0.7,1.3" --ymax 1e4 --ymin 3
python plot_EE.py --x-range "0,1000" --rebin 8 --hist-name "DYCR2_Resolved_EE_ll_pt" --output "DYCR2_EE_Dilepton_pt_2022EE" --x-title 'p_{T}^{ll}(GeV)' --logy --y-range "0.7,1.3" --ymax 1e6 --ymin 50
python plot_EE.py --x-range "0,500" --rebin 10 --hist-name "DYCR2_Resolved_EE_subleading_jet_pt" --output "DYCR2_EE_subleading_pt_2022EE" --x-title "p_{T} of the subleading jet (GeV)" --logy --y-range "0.7,1.3" --ymax 1e4 --ymin 3
python plot_EE.py --x-range "0,8000" --rebin 10 --hist-name "DYCR2_Resolved_EE_mlljj" --output "DYCR2_EE_mlljj_2022EE" --x-title "mlljj" --logy --y-range "0.7,1.3"

