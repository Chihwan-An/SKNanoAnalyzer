#!/usr/bin/env bash
##SDM

##ee
python plot_EE.py --x-range "0,1000" --rebin 10 --hist-name "DY_CR_Fatjet_SDMass" --output "DY_CR_EE_Fatjet_SDMass_2023" --x-title "m_{SD}(GeV)" --logy --y-range "0.7,1.3" --ymin 3 --ymax 1e4
python plot_EE.py --x-range "0,1000" --rebin 10 --hist-name "Boosted_DY_CR_EE_looselepton_infatjet_Fatjet_SDMass" --output "Boosted_DY_CR_EE_looselepton_infatjet_Fatjet_SDMass_2023" --x-title "m_{SD}(GeV)" --logy --y-range "0.7,1.3" --ymin 3 --ymax 1e4
python plot_EE.py --x-range "0,1000" --rebin 10 --hist-name "Boosted_DY_CR_EE_looselepton_outsidefatjet_Fatjet_SDMass" --output "Boosted_DY_CR_EE_looselepton_outsidefatjet_Fatjet_SDMass_2023" --x-title "m_{SD}(GeV)" --logy --y-range "0.7,1.3" --ymin 3 --ymax 1e4
python plot_EE.py --x-range "0,1000" --rebin 10 --hist-name "Boosted_DY_CR_MM_looselepton_infatjet_Fatjet_SDMass" --output "Boosted_DY_CR_MM_looselepton_infatjet_Fatjet_SDMass_2023" --x-title "m_{SD}(GeV)" --logy --y-range "0.7,1.3" --ymin 3 --ymax 1e4
python plot_EE.py --x-range "0,1000" --rebin 10 --hist-name "Boosted_DY_CR_MM_looselepton_outsidefatjet_Fatjet_SDMass" --output "Boosted_DY_CR_MM_looselepton_outsidefatjet_Fatjet_SDMass_2023" --x-title "m_{SD}(GeV)" --logy --y-range "0.7,1.3" --ymin 3 --ymax 1e4

##mumu

python plot_MM.py --x-range "0,1000" --rebin 10 --hist-name "DY_CR_Fatjet_SDMass" --output "DY_CR_MM_Fatjet_SDMass_2023" --x-title "m_{SD}(GeV)" --logy --y-range "0.7,1.3" --ymin 3 --ymax 1e4
python plot_MM.py --x-range "0,1000" --rebin 10 --hist-name "Boosted_DY_CR_MM_looselepton_infatjet_Fatjet_SDMass" --output "Boosted_DY_CR_MM_looselepton_infatjet_Fatjet_SDMass_2023" --x-title "m_{SD}(GeV)" --logy --y-range "0.7,1.3" --ymin 3 --ymax 1e4
python plot_MM.py --x-range "0,1000" --rebin 10 --hist-name "Boosted_DY_CR_MM_looselepton_outsidefatjet_Fatjet_SDMass" --output "Boosted_DY_CR_MM_looselepton_outsidefatjet_Fatjet_SDMass_2023" --x-title "m_{SD}(GeV)" --logy --y-range "0.7,1.3" --ymin 3 --ymax 1e4
python plot_MM.py --x-range "0,1000" --rebin 10 --hist-name "Boosted_DY_CR_MM_looselepton_infatjet_Fatjet_SDMass" --output "Boosted_DY_CR_MM_looselepton_infatjet_Fatjet_SDMass_2023" --x-title "m_{SD}(GeV)" --logy --y-range "0.7,1.3" --ymin 3 --ymax 1e4
python plot_MM.py --x-range "0,1000" --rebin 10 --hist-name "Boosted_DY_CR_MM_looselepton_outsidefatjet_Fatjet_SDMass" --output "Boosted_DY_CR_MM_looselepton_outsidefatjet_Fatjet_SDMass_2023" --x-title "m_{SD}(GeV)" --logy --y-range "0.7,1.3" --ymin 3 --ymax 1e4