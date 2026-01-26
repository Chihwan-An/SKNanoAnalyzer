#!/usr/bin/env bash

# Flav



## mu ejet
python plot_MM.py --x-range "0,1000" --rebin 20 --hist-name "Obj_PU_pt(ll)_boosted_mu_ejets_Flavor_CR" --output "Obj_PU_Boost_FLV_CR1_mu_ejet_Dilepton_pt_2023" --x-title "p_{T}^{ll}(GeV)" --logy --y-range "0.7,1.3" --ymax 3e3 --ymin 3 --y-range "0,2.5"
python plot_MM.py --x-range "0,2000" --rebin 8 --hist-name "Obj_PU_leading_fatjet_pt_boosted_mu_ejets_Flavor_CR" --output "Obj_PU_Boost_FLV_CR1_mu_ejet_LeadingFatJet_pt_2023" --x-title 'p_{T}^{J}(GeV)' --logy --y-range "0.7,1.3" --ymax 3e3 --ymin 3 --custom-bins "200,400,600,800,1000,2000,inf" --y-range "0,2.5"
python plot_MM.py --x-range "800,8000" --rebin 10 --hist-name "Obj_PU_m(lljj)_boosted_mu_ejets_Flavor_CR" --output "Obj_PU_Boost_FLV_CR1_mu_ejet_m(lj)_2023" --x-title "m(lj)" --logy --y-range "0.7,1.3"  --ymin 3 --ymax 3e3 --custom-bins "800,1000,1200,1500,1800,8000" --y-range "0,2.5"

## e mu jet

python plot_EE.py --x-range "0,1000" --rebin 20 --hist-name "Obj_PU_pt(ll)_boosted_e_mujet_Flavor_CR" --output "Obj_PU_Boost_FLV_CR1_e_mujet_Dilepton_pt_2023" --x-title "p_{T}^{ll}(GeV)" --logy --y-range "0.7,1.3" --ymax 3e3 --ymin 3 --y-range "0,2.5"
python plot_EE.py --x-range "0,2000" --rebin 8 --hist-name "Obj_PU_leading_fatjet_pt_boosted_e_mujet_Flavor_CR" --output "Obj_PU_Boost_FLV_CR1_e_mujet_LeadingFatJet_pt_2023" --x-title 'p_{T}^{J}(GeV)' --logy --y-range "0.7,1.3" --ymax 3e3 --ymin 3 --custom-bins "200,400,600,800,1000,2000,inf" --y-range "0,2.5"
python plot_EE.py --x-range "800,8000" --rebin 10 --hist-name "Obj_PU_m(lljj)_boosted_e_mujet_Flavor_CR" --output "Obj_PU_Boost_FLV_CR1_e_mujet_m(lj)_2023" --x-title "m(lj)" --logy --y-range "0.7,1.3"  --ymin 3 --ymax 3e3 --custom-bins "800,1000,1200,1500,1800,8000" --y-range "0,2.5"


