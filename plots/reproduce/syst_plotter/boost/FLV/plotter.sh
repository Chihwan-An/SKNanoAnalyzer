#!/usr/bin/env bash
#python EE.py --hist "Obj_PU_pt(ll)_boosted_e_mujet_Flavor_CR" --rebin 50 --ymin 10 --ymax 1e3 --output BoostFlV_e_mujet_dilepton_pt --xmin 0 --xmax 1000 --xlabel "Dilepton p_{T} (GeV)" --rmin 0 --rmax 2.5
#python EE.py --hist "Obj_PU_leading_fatjet_pt_boosted_e_mujet_Flavor_CR"  --ymin 1 --ymax 3e3 --output BoostFlV_e_mujet_leadingfatjet_pt --xmin 0 --xmax 2000 --xlabel "Leading Fat Jet p_{T} (GeV)" --bins "200,400,600,800,1000,2000" --rmin 0 --rmax 2.5
python EE.py --hist "Obj_PU_m(lljj)_boosted_e_mujet_Flavor_CR"  --ymin 1 --ymax 1e3 --output BoostFlV_e_mujet_mlljj --xmin 800 --xmax 8000 --xlabel "m(lJ)" --bins "800,1000,1200,1500,1800,8000" --rmin 0 --rmax 2.5

#python MM.py --hist "Obj_PU_pt(ll)_boosted_mu_ejets_Flavor_CR" --rebin 50 --ymin 10 --ymax 1e3 --output BoostFlV_mu_ejets_dilepton_pt --xmin 0 --xmax 1000 --xlabel "Dilepton p_{T} (GeV)" --rmin 0 --rmax 2.5
#python MM.py --hist "Obj_PU_leading_fatjet_pt_boosted_mu_ejets_Flavor_CR"  --ymin 1 --ymax 3e3 --output BoostFlV_mu_ejets_leadingfatjet_pt --xmin 0 --xmax 2000 --xlabel "Leading Fat Jet p_{T} (GeV)" --bins "200,400,600,800,1000,2000" --rmin 0 --rmax 2.5
python MM.py --hist "Obj_PU_m(lljj)_boosted_mu_ejets_Flavor_CR" --ymin 1 --ymax 1e3 --output BoostFlV_mu_ejets_mlljj --xmin 800 --xmax 8000 --xlabel "mlljj" --bins "800,1000,1200,1500,1800,8000" --rmin 0 --rmax 2.5