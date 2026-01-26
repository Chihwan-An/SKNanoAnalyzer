#!/usr/bin/env bash

# Flav

## mu ejet

python plot_MM.py --x-range "800,8000" --rebin 10 --hist-name "m(lljj)_boosted_mu_ejets_Flavor_CR" --output "Boost_FLV_CR1_mu_ejet_m(lj)_2023" --x-title "m(lj)" --logy --y-range "0.7,1.3"  --ymin 3 --ymax 3e3 --custom-bins "800,1000,1200,1500,1800,8000" --y-range "0,2.5"

## e mu jet


python plot_EE.py --x-range "800,8000" --rebin 10 --hist-name "m(lljj)_boosted_e_mujet_Flavor_CR" --output "Boost_FLV_CR1_e_mujet_m(lj)_2023" --x-title "m(lj)" --logy --y-range "0.7,1.3"  --ymin 3 --ymax 3e3 --custom-bins "800,1000,1200,1500,1800,8000" --y-range "0,2.5"


## mu ejet

python plot_MM.py --x-range "800,8000" --rebin 10 --hist-name "NO_PU_m(lljj)_boosted_mu_ejets_Flavor_CR" --output "NO_PU_Boost_FLV_CR1_mu_ejet_m(lj)_2023" --x-title "m(lj)" --logy --y-range "0.7,1.3"  --ymin 3 --ymax 3e3 --custom-bins "800,1000,1200,1500,1800,8000" --y-range "0,2.5"

## e mu jet


python plot_EE.py --x-range "800,8000" --rebin 10 --hist-name "NO_PU_m(lljj)_boosted_e_mujet_Flavor_CR" --output "NO_PU_Boost_FLV_CR1_e_mujet_m(lj)_2023" --x-title "m(lj)" --logy --y-range "0.7,1.3"  --ymin 3 --ymax 3e3 --custom-bins "800,1000,1200,1500,1800,8000" --y-range "0,2.5"


## mu ejet

python plot_MM.py --x-range "800,8000" --rebin 10 --hist-name "Obj_PU_m(lljj)_boosted_mu_ejets_Flavor_CR" --output "Obj_PU_Boost_FLV_CR1_mu_ejet_m(lj)_2023" --x-title "m(lj)" --logy --y-range "0.7,1.3"  --ymin 3 --ymax 3e3 --custom-bins "800,1000,1200,1500,1800,8000" --y-range "0,2.5"

## e mu jet


python plot_EE.py --x-range "800,8000" --rebin 10 --hist-name "Obj_PU_m(lljj)_boosted_e_mujet_Flavor_CR" --output "Obj_PU_Boost_FLV_CR1_e_mujet_m(lj)_2023" --x-title "m(lj)" --logy --y-range "0.7,1.3"  --ymin 3 --ymax 3e3 --custom-bins "800,1000,1200,1500,1800,8000" --y-range "0,2.5"


