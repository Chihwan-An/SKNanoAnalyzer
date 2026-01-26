#!/usr/bin/env bash
# Boost SR

##ee sign inclusive
python plot_EE.py --x-range "0,1000" --rebin 10 --hist-name "Obj_PU_boosted_Dilepton_Pt_ee" --output "Boost_SR_EE_Dilepton_pt_2023" --x-title "p_{T}^{ll}(GeV)" --logy --y-range "0.7,1.3" --ymax 1e5 --ymin 3 --sr
python plot_EE.py --x-range "0,2000" --rebin 25 --hist-name "Obj_PU_leading_fatjet_pt_boosted_ee" --output "Boost_SR_EE_LeadingFatJet_pt_2023" --x-title 'p_{T}^{J}(GeV)' --logy --y-range "0.7,1.3" --ymax 3e3 --ymin 3 --custom-bins "200,400,600,800,1000,2000,inf" --sr    
python plot_EE.py --x-range "800,8000" --rebin 10 --hist-name "Obj_PU_boosted_WRMass_ee" --output "Boost_SR_EE_mlljj_2023" --x-title "m(lJ)" --logy --y-range "0.7,1.3" --ymin 3 --ymax 1e3 --custom-bins "800,1000,1200,1500,1800,8000" --sr

## mumu sign inclusive
python plot_MM.py --x-range "0,1000" --rebin 10 --hist-name "Obj_PU_SR_boosted_Dilepton_Pt_mumu" --output "Boost_SR_MM_Dilepton_pt_2023" --x-title "p_{T}^{ll}(GeV)" --logy --y-range "0.7,1.3" --ymax 1e5 --ymin 3 --sr
python plot_MM.py --x-range "0,2000" --rebin 25 --hist-name "Obj_PU_SR_leading_fatjet_pt_boosted_mumu" --output "Boost_SR_MM_LeadingFatJet_pt_2023" --x-title 'p_{T}^{J}(GeV)' --logy --y-range "0.7,1.3" --ymax 3e3 --ymin 3 --custom-bins "200,400,600,800,1000,2000,inf" --sr
python plot_MM.py --x-range "800,8000" --rebin 10 --hist-name "Obj_PU_SR_boosted_WRMass_mumu" --output "Boost_SR_MM_mlljj_2023" --x-title "m(lJ)" --logy --y-range "0.7,1.3" --ymin 3 --ymax 1e3 --custom-bins "800,1000,1200,1500,1800,8000" --sr 

## ee 
##same charge
#python plot_EE.py --x-range "0,1000" --rebin 10 --hist-name "SR_boosted_Dilepton_Pt_ee_SS" --output "Boost_SR_EE_Dilepton_SS_pt_2023" --x-title "p_{T}^{ll}(GeV)" --logy --y-range "0.7,1.3" --ymax 1e5 --ymin 3 --sr
#python plot_EE.py --x-range "0,2000" --rebin 25 --hist-name "SR_leading_fatjet_pt_boosted_ee_SS" --output "Boost_SR_EE_LeadingFatJet_SS_pt_2023" --x-title 'p_{T}^{J}(GeV)' --logy --y-range "0.7,1.3" --ymax 3e3 --ymin 3 --custom-bins "200,400,600,800,1000,2000,inf" --sr    
#python plot_EE.py --x-range "800,8000" --rebin 10 --hist-name "SR_boosted_WRMass_ee_SS" --output "Boost_SR_EE_mlljj_SS_2023" --x-title "m(lJ)" --logy --y-range "0.7,1.3" --ymin 3 --ymax 1e3 --custom-bins "800,1000,1200,1500,1800,8000" --sr

## other charge
#python plot_EE.py --x-range "0,1000" --rebin 10 --hist-name "SR_boosted_Dilepton_Pt_ee_OS" --output "Boost_SR_EE_Dilepton_OS_pt_2023" --x-title "p_{T}^{ll}(GeV)" --logy --y-range "0.7,1.3" --ymax 1e5 --ymin 3 --sr
#python plot_EE.py --x-range "0,2000" --rebin 25 --hist-name "SR_leading_fatjet_pt_boosted_ee_OS" --output "Boost_SR_EE_LeadingFatJet_OS_pt_2023" --x-title 'p_{T}^{J}(GeV)' --logy --y-range "0.7,1.3" --ymax 3e3 --ymin 3 --custom-bins "200,400,600,800,1000,2000,inf" --sr
#python plot_EE.py --x-range "800,8000" --rebin 10 --hist-name "SR_boosted_WRMass_ee_OS" --output "Boost_SR_EE_mlljj_OS_2023" --x-title "m(lJ)" --logy --y-range "0.7,1.3" --ymin 3 --ymax 1e3 --custom-bins "800,1000,1200,1500,1800,8000" --sr

## mumu 
##same charge
#python plot_MM.py --x-range "0,1000" --rebin 10 --hist-name "SR_boosted_Dilepton_Pt_mumu_SS" --output "Boost_SR_MM_Dilepton_SS_pt_2023" --x-title "p_{T}^{ll}(GeV)" --logy --y-range "0.7,1.3" --ymax 1e5 --ymin 3 --sr
#python plot_MM.py --x-range "0,2000" --rebin 25 --hist-name "SR_leading_fatjet_pt_boosted_mumu_SS" --output "Boost_SR_MM_LeadingFatJet_SS_pt_2023" --x-title 'p_{T}^{J}(GeV)' --logy --y-range "0.7,1.3" --ymax 3e3 --ymin 3 --custom-bins "200,400,600,800,1000,2000,inf" --sr
#python plot_MM.py --x-range "800,8000" --rebin 10 --hist-name "SR_boosted_WRMass_mumu_SS" --output "Boost_SR_MM_mlljj_SS_2023" --x-title "m(lJ)" --logy --y-range "0.7,1.3" --ymin 3 --ymax 1e3 --custom-bins "800,1000,1200,1500,1800,8000" --sr 


## other charge
#python plot_MM.py --x-range "0,1000" --rebin 10 --hist-name "SR_boosted_Dilepton_Pt_mumu_OS" --output "Boost_SR_MM_Dilepton_OS_pt_2023" --x-title "p_{T}^{ll}(GeV)" --logy --y-range "0.7,1.3" --ymax 1e5 --ymin 3 --sr
#python plot_MM.py --x-range "0,2000" --rebin 25 --hist-name "SR_leading_fatjet_pt_boosted_mumu_OS" --output "Boost_SR_MM_LeadingFatJet_OS_pt_2023" --x-title 'p_{T}^{J}(GeV)' --logy --y-range "0.7,1.3" --ymax 3e3 --ymin 3 --custom-bins "200,400,600,800,1000,2000,inf" --sr
#python plot_MM.py --x-range "800,8000" --rebin 10 --hist-name "SR_boosted_WRMass_mumu_OS" --output "Boost_SR_MM_mlljj_OS_2023" --x-title "m(lJ)" --logy --y-range "0.7,1.3" --ymin 3 --ymax 1e3 --custom-bins "800,1000,1200,1500,1800,8000" --sr


