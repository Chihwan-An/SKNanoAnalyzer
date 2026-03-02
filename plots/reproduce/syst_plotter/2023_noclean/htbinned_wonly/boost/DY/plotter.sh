#!/usr/bin/env bash
python EE.py --hist "Obj_PU_pt(ll)_boosted_DY_CR_EE" --rebin 50 --ymin 10 --ymax 1e5 --output BoostDYEE_ee_pt --xmin 0 --xmax 1000 --xlabel "Dilepton p_{T} (GeV)" 
python EE.py --hist "Obj_PU_leading_fatjet_pt_boosted_DY_CR_EE"  --ymin 3 --ymax 3e3 --output BoostDYEE_ee_leadingfatjet_pt --xmin 0 --xmax 2000 --xlabel "Leading Fat Jet p_{T} (GeV)" --bins "200,400,600,800,1000,2000"
python EE.py --hist "Obj_PU_m(lljj)_boosted_DY_CR_EE"  --ymin 3 --ymax 1e4 --output BoostDYEE_ee_mlljj --xmin 800 --xmax 8000 --xlabel "m(lJ)" --bins "800,1000,1200,1500,1800,8000"
python EE.py --hist "Obj_PU_leading_lep_pt_boosted_DY_CR_EE"  --ymin 3 --ymax 3e3 --output BoostDYEE_ee_leadinglep_pt --xmin 0 --xmax 1000 --xlabel "Leading Lepton p_{T} (GeV)" --bins "0,100,200,300,400,500,1000,2000"
python EE.py --hist "Obj_PU_subleading_lep_pt_boosted_DY_CR_EE"  --ymin 3 --ymax 3e3 --output BoostDYEE_ee_subleadinglep_pt --xmin 0 --xmax 500 --xlabel "Subleading Lepton p_{T} (GeV)" --bins "0,50,100,150,200,250,300,500,1000"
    
python MM.py --hist "Obj_PU_pt(ll)_boosted_DY_CR_MM" --rebin 50 --ymin 10 --ymax 1e5 --output BoostDYMM_mm_pt --xmin 0 --xmax 1000 --xlabel "Dilepton p_{T} (GeV)" 
python MM.py --hist "Obj_PU_leading_fatjet_pt_boosted_DY_CR_MM"  --ymin 3 --ymax 3e3 --output BoostDYMM_mm_leadingfatjet_pt --xmin 0 --xmax 2000 --xlabel "Leading Fat Jet p_{T} (GeV)" --bins "200,400,600,800,1000,2000"
python MM.py --hist "Obj_PU_m(lljj)_boosted_DY_CR_MM" --ymin 3 --ymax 1e4 --output BoostDYMM_mm_mlljj --xmin 800 --xmax 8000 --xlabel "mlljj" --bins "800,1000,1200,1500,1800,8000"
python MM.py --hist "Obj_PU_leading_lep_pt_boosted_DY_CR_MM"  --ymin 3 --ymax 3e3 --output BoostDYMM_mm_leadinglep_pt --xmin 0 --xmax 1000 --xlabel "Leading Lepton p_{T} (GeV)" --bins "0,100,200,300,400,500,1000,2000"
python MM.py --hist "Obj_PU_subleading_lep_pt_boosted_DY_CR_MM"  --ymin 3 --ymax 3e3 --output BoostDYMM_mm_subleadinglep_pt --xmin 0 --xmax 500 --xlabel "Subleading Lepton p_{T} (GeV)" --bins "0,50,100,150,200,250,300,500,1000"    