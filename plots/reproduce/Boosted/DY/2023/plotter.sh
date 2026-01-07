#!/usr/bin/env bash
# DY CR1 

## ee 
python plot_EE.py --x-range "0,1000" --rebin 10 --hist-name "pt(ll)_boosted_DY_CR_EE" --output "Boost_DYCR_EE_Dilepton_pt_2023" --x-title "p_{T}^{ll}(GeV)" --logy --y-range "0.7,1.3" --ymax 1e5 --ymin 30
python plot_EE.py --x-range "0,2000" --rebin 25 --hist-name "leading_fatjet_pt_boosted_DY_CR_EE" --output "Boost_DYCR_EE_LeadingFatJet_pt_2023" --x-title 'p_{T}^{J}(GeV)' --logy --y-range "0.7,1.3" --ymax 3e3 --ymin 3 --custom-bins "200,400,600,800,1000,2000,inf"
python plot_EE.py --x-range "800,8000" --rebin 10 --hist-name "m(lljj)_boosted_DY_CR_EE" --output "Boost_DYCR_EE_mlljj_2023" --x-title "m(lJ)" --logy --y-range "0.7,1.3" --ymin 3 --ymax 1e4 --custom-bins "800,1000,1200,1500,1800,8000"

## mumu 
python plot_MM.py --x-range "0,1000" --rebin 10 --hist-name "pt(ll)_boosted_DY_CR_MM" --output "Boost_DYCR_MM_Dilepton_pt_2023" --x-title "p_{T}^{ll}(GeV)" --logy --y-range "0.7,1.3" --ymax 1e5 --ymin 30
python plot_MM.py --x-range "0,2000" --rebin 25 --hist-name "leading_fatjet_pt_boosted_DY_CR_MM" --output "Boost_DYCR_MM_LeadingFatJet_pt_2023" --x-title 'p_{T}^{J}(GeV)' --logy --y-range "0.7,1.3" --ymax 3e3 --ymin 3 --custom-bins "200,400,600,800,1000,2000,inf"
python plot_MM.py --x-range "800,8000" --rebin 10 --hist-name "m(lljj)_boosted_DY_CR_MM" --output "Boost_DYCR_MM_mlljj_2023" --x-title "mlljj" --logy --y-range "0.7,1.3" --ymin 3 --ymax 1e4 --custom-bins "800,1000,1200,1500,1800,8000"  


# DY CR2

## mumu 
#python plot_MM.py --x-range "0,1000" --rebin 10 --hist-name "pt(ll)_boosted_DY_CR2_MM" --output "Boost_DYCR2_MM_Dilepton_pt_2023" --x-title "p_{T}^{ll}(GeV)" --logy --y-range "0.7,1.3" --ymax 1e5 --ymin 30
#python plot_MM.py --x-range "0,2000" --rebin 25 --hist-name "leading_fatjet_pt_boosted_DY_CR2_MM" --output "Boost_DYCR2_MM_LeadingFatJet_pt_2023" --x-title 'p_{T}^{J}(GeV)' --logy --y-range "0.7,1.3" --ymax 3e3 --ymin 3 --custom-bins "200,400,600,800,1000,2000,inf"
#python plot_MM.py --x-range "800,8000" --rebin 10 --hist-name "m(lljj)_boosted_DY_CR2_MM" --output "Boost_DYCR2_MM_mlljj_2023" --x-title "mlljj" --logy --y-range "0.7,1.3" --ymin 3 --ymax 1e4 --custom-bins "800,1000,1200,1500,1800,8000"

## ee 
#python plot_EE.py --x-range "0,1000" --rebin 10 --hist-name "pt(ll)_boosted_DY_CR2_EE" --output "Boost_DYCR2_EE_Dilepton_pt_2023" --x-title "p_{T}^{ll}(GeV)" --logy --y-range "0.7,1.3" --ymax 1e5 --ymin 30
#python plot_EE.py --x-range "0,2000" --rebin 25 --hist-name "leading_fatjet_pt_boosted_DY_CR2_EE" --output "Boost_DYCR2_EE_LeadingFatJet_pt_2023" --x-title 'p_{T}^{J}(GeV)' --logy --y-range "0.7,1.3" --ymax 3e3 --ymin 3 --custom-bins "200,400,600,800,1000,2000,inf"
#python plot_EE.py --x-range "800,8000" --rebin 10 --hist-name "m(lljj)_boosted_DY_CR2_EE" --output "Boost_DYCR2_EE_mlljj_2023" --x-title "mlljj" --logy --y-range "0.7,1.3"  --ymin 3 --ymax 1e4 --custom-bins "800,1000,1200,1500,1800,8000"
