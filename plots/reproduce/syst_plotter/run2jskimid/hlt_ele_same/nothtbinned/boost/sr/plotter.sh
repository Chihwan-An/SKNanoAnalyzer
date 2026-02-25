#!/usr/bin/env bash
# Boost SR

##ee sign inclusive
python EE.py --xmin 0 --xmax 1000 --rebin 50 --hist "Obj_PU_boosted_Dilepton_Pt_ee" --output "Boost_SR_EE_Dilepton_pt_2023" --xlabel "p_{T}^{ll}(GeV)"  --rmin 0.7 --rmax 1.3 --ymax 1e3 --ymin 3 
python EE.py --xmin 0 --xmax 2000  --hist "Obj_PU_leading_fatjet_pt_boosted_ee" --output "Boost_SR_EE_LeadingFatJet_pt_2023" --xlabel 'p_{T}^{J}(GeV)'  --rmin 0.7 --rmax 1.3 --ymax 1e3 --ymin 3 --bins "0,200,400,600,800,1000,2000" 
python EE.py --xmin 800 --xmax 8000  --hist "Obj_PU_boosted_WRMass_ee" --output "Boost_SR_EE_mlljj_2023" --xlabel "m(lJ)"  --rmin 0.7 --rmax 1.3 --ymin 3 --ymax 1e3 --bins "800,1000,1200,1500,1800,8000" 
python EE.py --xmin 0 --xmax 1000 --hist "Obj_PU_boosted_LeadingLepPt_ee" --output "Boost_SR_EE_LeadingLep_pt_2023" --xlabel "Leading Lepton p_{T} (GeV)"  --rmin 0.7 --rmax 1.3 --ymax 1e2 --ymin 1e-3 --bins "0,50,100,150,200,250,300,350,400,450,500,550,600,650,700,750,800,850,900,950,1000"
python EE.py --xmin 0 --xmax 1000 --hist "Obj_PU_boosted_SubLeadingLepPt_ee" --output "Boost_SR_EE_SubLeadingLep_pt_2023" --xlabel "Subleading Lepton p_{T} (GeV)"  --rmin 0.7 --rmax 1.3 --ymax 1e2 --ymin 1e-3 --bins "0,50,100,150,200,250,300,350,400,450,500,550,600,650,700,750,800,850,900,950,1000"
## mumu sign inclusive
python MM.py --xmin 0 --xmax 1000 --rebin 50 --hist "Obj_PU_SR_boosted_Dilepton_Pt_mumu" --output "Boost_SR_MM_Dilepton_pt_2023" --xlabel "p_{T}^{ll}(GeV)"  --rmin 0.7 --rmax 1.3 --ymax 1e3 --ymin 3 
python MM.py --xmin 0 --xmax 2000  --hist "Obj_PU_SR_leading_fatjet_pt_boosted_mumu" --output "Boost_SR_MM_LeadingFatJet_pt_2023" --xlabel 'p_{T}^{J}(GeV)'  --rmin 0.7 --rmax 1.3 --ymax 1e3 --ymin 3 --bins "0,200,400,600,800,1000,2000" 
python MM.py --xmin 800 --xmax 8000  --hist "Obj_PU_SR_boosted_WRMass_mumu" --output "Boost_SR_MM_mlljj_2023" --xlabel "m(lJ)"  --rmin 0.7 --rmax 1.3 --ymin 3 --ymax 1e3 --bins "800,1000,1200,1500,1800,8000" 
python MM.py --xmin 0 --xmax 1000 --hist "Obj_PU_boosted_LeadingLepPt_mumu" --output "Boost_SR_MM_LeadingLep_pt_2023" --xlabel "Leading Lepton p_{T} (GeV)"  --rmin 0.7 --rmax 1.3 --ymax 1e2 --ymin 1e-3 --bins "0,50,100,150,200,250,300,350,400,450,500,550,600,650,700,750,800,850,900,950,1000"
python MM.py --xmin 0 --xmax 1000 --hist "Obj_PU_boosted_SubLeadingLepPt_mumu" --output "Boost_SR_MM_SubLeadingLeap_pt_2023" --xlabel "Subleading Lepton p_{T} (GeV)"  --rmin 0.7 --rmax 1.3 --ymax 1e2 --ymin 1e-3 --bins "0,50,100,150,200,250,300,350,400,450,500,550,600,650,700,750,800,850,900,950,1000"
