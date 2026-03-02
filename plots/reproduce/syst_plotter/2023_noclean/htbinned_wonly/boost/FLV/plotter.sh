#!/usr/bin/env bash
python EE.py --hist "Obj_PU_pt(ll)_boosted_e_mujet_Flavor_CR" --rebin 50 --ymin 10 --ymax 1e3 --output BoostFlV_e_mujet_dilepton_pt --xmin 0 --xmax 1000 --xlabel "Dilepton p_{T} (GeV)" --rmin 0 --rmax 2.5
python EE.py --hist "Obj_PU_leading_fatjet_pt_boosted_e_mujet_Flavor_CR"  --ymin 1 --ymax 3e3 --output BoostFlV_e_mujet_leadingfatjet_pt --xmin 0 --xmax 2000 --xlabel "Leading Fat Jet p_{T} (GeV)" --bins "200,400,600,800,1000,2000" --rmin 0 --rmax 2.5
python EE.py --hist "Obj_PU_m(lljj)_boosted_e_mujet_Flavor_CR"  --ymin 1 --ymax 1e3 --output BoostFlV_e_mujet_mlljj --xmin 800 --xmax 8000 --xlabel "m(lJ)" --bins "800,1000,1200,1500,1800,8000" --rmin 0 --rmax 2.5
python EE.py --hist "Obj_PU_Boosted_Flavor_CR_e_mujet_LeadingLepPt"  --ymin 1 --ymax 1e3 --output BoostFlV_e_mujet_leadinglep_pt --xmin 0 --xmax 1000 --xlabel "Leading Lepton p_{T} (GeV)" --bins "0,100,200,300,400,500,1000,2000" --rmin 0 --rmax 2.5
python EE.py --hist "Obj_PU_Boosted_Flavor_CR_e_mujet_SubLeadingLepPt"  --ymin 1 --ymax 1e3 --output BoostFlV_e_mujet_subleadinglep_pt --xmin 0 --xmax 500 --xlabel "Subleading Lepton p_{T} (GeV)" --bins "0,50,100,150,200,250,300,500,1000" --rmin 0 --rmax 2.5

python MM.py --hist "Obj_PU_pt(ll)_boosted_mu_ejets_Flavor_CR" --rebin 50 --ymin 10 --ymax 1e3 --output BoostFlV_mu_ejets_dilepton_pt --xmin 0 --xmax 1000 --xlabel "Dilepton p_{T} (GeV)" --rmin 0 --rmax 2.5
python MM.py --hist "Obj_PU_leading_fatjet_pt_boosted_mu_ejets_Flavor_CR"  --ymin 1 --ymax 3e3 --output BoostFlV_mu_ejets_leadingfatjet_pt --xmin 0 --xmax 2000 --xlabel "Leading Fat Jet p_{T} (GeV)" --bins "200,400,600,800,1000,2000" --rmin 0 --rmax 2.5
python MM.py --hist "Obj_PU_m(lljj)_boosted_mu_ejets_Flavor_CR" --ymin 1 --ymax 1e3 --output BoostFlV_mu_ejets_mlljj --xmin 800 --xmax 8000 --xlabel "mlljj" --bins "800,1000,1200,1500,1800,8000" --rmin 0 --rmax 2.5
python MM.py --hist "Obj_PU_Boosted_Flavor_CR_mu_ejets_LeadingLepPt"  --ymin 1 --ymax 1e3 --output BoostFlV_mu_ejets_leadinglep_pt --xmin 0 --xmax 1000 --xlabel "Leading Lepton p_{T} (GeV)" --bins "0,100,200,300,400,500,1000,2000" --rmin 0 --rmax 2.5  
python MM.py --hist "Obj_PU_Boosted_Flavor_CR_mu_ejets_SubLeadingLepPt"  --ymin 1 --ymax 1e3 --output BoostFlV_mu_ejets_subleadinglep_pt --xmin 0 --xmax 500 --xlabel "Subleading Lepton p_{T} (GeV)" --bins "0,50,100,150,200,250,300,500,1000" --rmin 0 --rmax 2.5
