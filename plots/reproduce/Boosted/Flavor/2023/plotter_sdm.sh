#!/usr/bin/env bash


## SDM

### ## e mu jet
python plot_EE.py --x-range "0,1000" --rebin 50 --hist-name "Boosted_Flavor_CR_e_mujet_Fatjet_SDMass" --output "SDM_e_mujet" --x-title "m_{SD}(GeV)" --logy --y-range "0.7,1.3" --ymin 3 --ymax 1e4

### mu e jet

python plot_EE.py --x-range "0,1000" --rebin 50 --hist-name "Boosted_Flavor_CR_mu_ejets_Fatjet_SDMass" --output "SDM_mu_ejets" --x-title "m_{SD}(GeV)" --logy --y-range "0.7,1.3" --ymin 3 --ymax 1e4