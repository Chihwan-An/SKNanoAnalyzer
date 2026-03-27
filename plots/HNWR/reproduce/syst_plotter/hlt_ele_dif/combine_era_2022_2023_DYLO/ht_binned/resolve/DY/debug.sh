#python EE.py --signal-scale 1.0 --hist "Obj_PU_Corr_DYCR_Resolved_EE_mlljj" --ymin 3 --ymax 1e6 --output ResolveDYEE_mlljj --xmin 800 --xmax 8000 --xlabel "m(lljj) (GeV)" --bins "800,1000,1200,1400,1600,2000,2400,2800,3200,8000" --rmin 0.5 --rmax 2.0
#python MM.py --signal-scale 1.0 --hist "Obj_PU_Corr_DYCR_Resolved_MM_mlljj" --ymin 3 --ymax 1e6 --output ResolveDYMM_mlljj --xmin 800 --xmax 8000 --xlabel "m(lljj) (GeV)" --bins "800,1000,1200,1400,1600,2000,2400,2800,3200,8000" --rmin 0.5 --rmax 2.0

#python EE.py --signal-scale 1.0 --hist "Obj_PU_Corr_DYCR_Resolved_EE_mlljj" --ymin 3 --ymax 1e6 --output ResolveDYEE_mlljj --xmin 800 --xmax 4000 --xlabel "m(lljj) (GeV)" --rebin 10 --rmin 0.5 --rmax 2.0
#python MM.py --signal-scale 1.0 --hist "Obj_PU_Corr_DYCR_Resolved_MM_mlljj" --ymin 3 --ymax 1e6 --output ResolveDYMM_mlljj --xmin 800 --xmax 4000 --xlabel "m(lljj) (GeV)" --rebin 10 --rmin 0.5 --rmax 2.0

python MM.py --hist "Obj_PU_Corr_DYCR_Resolved_MM_SS_mlljj" --ymin 1 --ymax 1e4 --output ResolveDYMM_SS_mlljj --xmin 800 --xmax 4000 --xlabel "m(lljj) (GeV)" --rebin 20 --rmin 0 --rmax 2.0
python EE.py --hist "Obj_PU_Corr_DYCR_Resolved_EE_OS_pt" --ymin 1 --ymax 1e6 --output ResolveDYEE_OS_dilepton_pt --xmin 0 --xmax 1000 --xlabel "Dilepton p_{T} (GeV)" --rmin 0 --rmax 2.0 --rebin 5
python MM.py --hist "Obj_PU_Corr_DYCR_Resolved_MM_SS_pt" --ymin 1 --ymax 1e4 --output ResolveDYMM_SS_dilepton_pt --xmin 0 --xmax 1000 --xlabel "Dilepton p_{T} (GeV)" --rmin 0 --rmax 2.0 --rebin 5
python MM.py --hist "Obj_PU_Corr_DYCR_Resolved_MM_OS_pt" --ymin 1 --ymax 1e6 --output ResolveDYMM_OS_dilepton_pt --xmin 0 --xmax 1000 --xlabel "Dilepton p_{T} (GeV)" --rmin 0 --rmax 2.0 --rebin 5