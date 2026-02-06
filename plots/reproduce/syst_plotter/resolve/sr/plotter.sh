#!/usr/bin/env bash
# Boost SR

##ee sign inclusive
python EE.py --xmin 0 --xmax 1000 --rebin 50 --hist "Obj_PU_Corr_SR_Resolved_EE_ll_pt" --output "Resolve_SR_EE_Dilepton_pt_2023" --xlabel "p_{T}^{ll}(GeV)"  --rmin 0.7 --rmax 1.3 --ymax 1e3 --ymin 1e-1 
python EE.py --xmin 0 --xmax 2000  --hist "Obj_PU_Corr_SR_Resolved_EE_leading_jet_pt" --output "Resolve_SR_EE_LeadingJet_pt_2023" --xlabel 'p_{T}^{j1}(GeV)'  --rmin 0.7 --rmax 1.3 --ymax 1e3 --ymin 1e-1 --bins "0,200,400,600,800,1000,2000" 
python EE.py --xmin 0 --xmax 2000  --hist "Obj_PU_Corr_SR_Resolved_EE_subleading_jet_pt" --output "Resolve_SR_EE_SubleadingJet_pt_2023" --xlabel 'p_{T}^{j2}(GeV)'  --rmin 0.7 --rmax 1.3 --ymax 1e3 --ymin 1e-1 --bins "0,200,400,600,800,1000,2000" 
python EE.py --xmin 800 --xmax 8000  --hist "Obj_PU_Corr_SR_Resolved_EE_mlljj" --output "Resolve_SR_EE_mlljj_2023" --xlabel "m(lJ)"  --rmin 0.7 --rmax 1.3 --ymin 1e-1 --ymax 1e3 --bins "800,1000,1200,1400,1600,2000,2400,2800,3200,8000" 

## mumu sign inclusive
python MM.py --xmin 0 --xmax 1000 --rebin 50 --hist "Obj_PU_Corr_SR_Resolved_MM_ll_pt" --output "Resolve_SR_MM_Dilepton_pt_2023" --xlabel "p_{T}^{ll}(GeV)"  --rmin 0.7 --rmax 1.3 --ymax 1e3 --ymin 1e-1 
python MM.py --xmin 0 --xmax 2000  --hist "Obj_PU_Corr_SR_Resolved_MM_leading_jet_pt" --output "Resolve_SR_MM_LeadingJet_pt_2023" --xlabel 'p_{T}^{j1}(GeV)'  --rmin 0.7 --rmax 1.3 --ymax 1e3 --ymin 1e-1 --bins "0,200,400,600,800,1000,2000" 
python MM.py --xmin 0 --xmax 2000  --hist "Obj_PU_Corr_SR_Resolved_MM_subleading_jet_pt" --output "Resolve_SR_MM_SubleadingJet_pt_2023" --xlabel 'p_{T}^{j2}(GeV)'  --rmin 0.7 --rmax 1.3 --ymax 1e3 --ymin 1e-1 --bins "0,200,400,600,800,1000,2000" 
python MM.py --xmin 800 --xmax 8000  --hist "Obj_PU_Corr_SR_Resolved_MM_mlljj" --output "Resolve_SR_MM_mlljj_2023" --xlabel "m(lJ)"  --rmin 0.7 --rmax 1.3 --ymin 1e-1 --ymax 1e3 --bins "800,1000,1200,1400,1600,2000,2400,2800,3200,8000" 