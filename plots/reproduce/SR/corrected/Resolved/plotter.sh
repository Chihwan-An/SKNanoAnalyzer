#!/usr/bin/env bash
# resolve SR

## ee // sign inclusive
python plot_EE.py --x-range "0,1000" --rebin 10 --hist-name "SR_Resolved_EE_ll_pt" --output "Resolved_SR_EE_Dilepton_pt_2023" --x-title "p_{T}^{ll}(GeV)" --logy --y-range "0.7,1.3" --ymax 1e5 --ymin 0.3 --sr
python plot_EE.py --x-range "0,2000" --rebin 25 --hist-name "SR_Resolved_EE_leading_jet_pt" --output "Resolved_SR_EE_Leading_Jet_pt_2023" --x-title 'p_{T}^{J}(GeV)' --logy --y-range "0.7,1.3" --ymax 3e3 --ymin 0.3 --custom-bins "200,400,600,800,1000,2000,inf" --sr
python plot_EE.py --x-range "0,500" --rebin 10 --hist-name "SR_Resolved_EE_subleading_jet_pt" --output "Resolved_SR_EE_Subleading_Jet_pt_2023" --x-title "m(lJ)" --x-title "p_{T} of the subleading jet (GeV)" --logy --y-range "0.7,1.3" --ymax 1e4 --ymin 0.3
python plot_EE.py --x-range "800,8000" --rebin 10 --hist-name "SR_Resolved_EE_mlljj" --output "Resolved_SR_EE_mlljj_2023" --x-title "m(lljj)" --logy --y-range "0.7,1.3" --ymin 0.3 --ymax 2e3 --custom-bins "800,1000,1200,1400,1600,2000,2400,2800,3200,8000" --sr

## mumu // sign inclusive
python plot_MM.py --x-range "0,1000" --rebin 10 --hist-name "SR_Resolved_MM_ll_pt" --output "Resolved_SR_MM_Dilepton_pt_2023" --x-title "p_{T}^{ll}(GeV)" --logy --y-range "0.7,1.3" --ymax 1e5 --ymin 0.3 --sr
python plot_MM.py --x-range "0,2000" --rebin 25 --hist-name "SR_Resolved_MM_leading_jet_pt" --output "Resolved_SR_MM_Leading_Jet_pt_2023" --x-title 'p_{T}^{J}(GeV)' --logy --y-range "0.7,1.3" --ymax 3e3 --ymin 0.3 --custom-bins "200,400,600,800,1000,2000,inf" --sr
python plot_MM.py --x-range "0,500" --rebin 10 --hist-name "SR_Resolved_MM_subleading_jet_pt" --output "Resolved_SR_MM_Subleading_Jet_pt_2023" --x-title "m(lJ)" --x-title "p_{T} of the subleading jet (GeV)" --logy --y-range "0.7,1.3" --ymax 1e4 --ymin 0.3
python plot_MM.py --x-range "800,8000" --rebin 10 --hist-name "SR_Resolved_MM_mlljj" --output "Resolved_SR_MM_mlljj_2023" --x-title "m(lljj)" --logy --y-range "0.7,1.3" --ymin 0.3 --ymax 2e3 --custom-bins "800,1000,1200,1400,1600,2000,2400,2800,3200,8000" --sr    

## ee //same charge
python plot_EE.py --x-range "0,1000" --rebin 10 --hist-name "SR_Resolved_EE_ll_pt_SS" --output "Resolved_SR_EE_Dilepton_SS_pt_2023" --x-title "p_{T}^{ll}(GeV)" --logy --y-range "0.7,1.3" --ymax 1e5 --ymin 0.3 --sr
python plot_EE.py --x-range "0,2000" --rebin 25 --hist-name "SR_Resolved_EE_leading_jet_pt_SS" --output "Resolved_SR_EE_Leading_Jet_SS_pt_2023" --x-title 'p_{T}^{J}(GeV)' --logy --y-range "0.7,1.3" --ymax 3e3 --ymin 0.3 --custom-bins "200,400,600,800,1000,2000,inf" --sr
python plot_EE.py --x-range "0,500" --rebin 10 --hist-name "SR_Resolved_EE_subleading_jet_pt_SS" --output "Resolved_SR_EE_Subleading_Jet_SS_pt_2023" --x-title "m(lJ)" --x-title "p_{T} of the subleading jet (GeV)" --logy --y-range "0.7,1.3" --ymax 1e4 --ymin 0.3
python plot_EE.py --x-range "800,8000" --rebin 10 --hist-name "SR_Resolved_EE_mlljj_SS" --output "Resolved_SR_EE_mlljj_SS_2023" --x-title "m(lljj)" --logy --y-range "0.7,1.3" --ymin 0.3 --ymax 2e3 --custom-bins "800,1000,1200,1400,1600,2000,2400,2800,3200,8000" --sr

## ee //opposite charge
python plot_EE.py --x-range "0,1000" --rebin 10 --hist-name "SR_Resolved_EE_ll_pt_OS" --output "Resolved_SR_EE_Dilepton_OS_pt_2023" --x-title "p_{T}^{ll}(GeV)" --logy --y-range "0.7,1.3" --ymax 1e5 --ymin 0.3 --sr
python plot_EE.py --x-range "0,2000" --rebin 25 --hist-name "SR_Resolved_EE_leading_jet_pt_OS" --output "Resolved_SR_EE_Leading_Jet_OS_pt_2023" --x-title 'p_{T}^{J}(GeV)' --logy --y-range "0.7,1.3" --ymax 3e3 --ymin 0.3 --custom-bins "200,400,600,800,1000,2000,inf" --sr
python plot_EE.py --x-range "0,500" --rebin 10 --hist-name "SR_Resolved_EE_subleading_jet_pt_OS" --output "Resolved_SR_EE_Subleading_Jet_OS_pt_2023" --x-title "m(lJ)" --x-title "p_{T} of the subleading jet (GeV)" --logy --y-range "0.7,1.3" --ymax 1e4 --ymin 0.3
python plot_EE.py --x-range "800,8000" --rebin 10 --hist-name "SR_Resolved_EE_mlljj_OS" --output "Resolved_SR_EE_mlljj_OS_2023" --x-title "m(lljj)" --logy --y-range "0.7,1.3" --ymin 0.3 --ymax 2e3 --custom-bins "800,1000,1200,1400,1600,2000,2400,2800,3200,8000" --sr



## mumu //same charge
python plot_MM.py --x-range "0,1000" --rebin 10 --hist-name "SR_Resolved_MM_ll_pt_SS" --output "Resolved_SR_MM_Dilepton_SS_pt_2023" --x-title "p_{T}^{ll}(GeV)" --logy --y-range "0.7,1.3" --ymax 1e5 --ymin 0.3 --sr
python plot_MM.py --x-range "0,2000" --rebin 25 --hist-name "SR_Resolved_MM_leading_jet_pt_SS" --output "Resolved_SR_MM_Leading_Jet_SS_pt_2023" --x-title 'p_{T}^{J}(GeV)' --logy --y-range "0.7,1.3" --ymax 3e3 --ymin 0.3 --custom-bins "200,400,600,800,1000,2000,inf" --sr
python plot_MM.py --x-range "0,500" --rebin 10 --hist-name "SR_Resolved_MM_subleading_jet_pt_SS" --output "Resolved_SR_MM_Subleading_Jet_SS_pt_2023" --x-title "m(lJ)" --x-title "p_{T} of the subleading jet (GeV)" --logy --y-range "0.7,1.3" --ymax 1e4 --ymin 0.3
python plot_MM.py --x-range "800,8000" --rebin 10 --hist-name "SR_Resolved_MM_mlljj_SS" --output "Resolved_SR_MM_mlljj_SS_2023" --x-title "m(lljj)" --logy --y-range "0.7,1.3" --ymin 0.3 --ymax 2e3 --custom-bins "800,1000,1200,1400,1600,2000,2400,2800,3200,8000" --sr



## mumu //opposite charge


python plot_MM.py --x-range "0,1000" --rebin 10 --hist-name "SR_Resolved_MM_ll_pt_OS" --output "Resolved_SR_MM_Dilepton_OS_pt_2023" --x-title "p_{T}^{ll}(GeV)" --logy --y-range "0.7,1.3" --ymax 1e5 --ymin 0.3 --sr
python plot_MM.py --x-range "0,2000" --rebin 25 --hist-name "SR_Resolved_MM_leading_jet_pt_OS" --output "Resolved_SR_MM_Leading_Jet_OS_pt_2023" --x-title 'p_{T}^{J}(GeV)' --logy --y-range "0.7,1.3" --ymax 3e3 --ymin 0.3 --custom-bins "200,400,600,800,1000,2000,inf" --sr
python plot_MM.py --x-range "0,500" --rebin 10 --hist-name "SR_Resolved_MM_subleading_jet_pt_OS" --output "Resolved_SR_MM_Subleading_Jet_OS_pt_2023" --x-title "m(lJ)" --x-title "p_{T} of the subleading jet (GeV)" --logy --y-range "0.7,1.3" --ymax 1e4 --ymin 0.3
python plot_MM.py --x-range "800,8000" --rebin 10 --hist-name "SR_Resolved_MM_mlljj_OS" --output "Resolved_SR_MM_mlljj_OS_2023" --x-title "m(lljj)" --logy --y-range "0.7,1.3" --ymin 0.3 --ymax 2e3 --custom-bins "800,1000,1200,1400,1600,2000,2400,2800,3200,8000" --sr
