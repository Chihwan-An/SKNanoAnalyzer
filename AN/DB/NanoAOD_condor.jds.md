```jds
executable = Try.py
arguments = -f $(myfile) -o out.root
log = WRTauStudy/job.log
output = WRTauStudy/job$(process).out
error = WRTauStudy/job$(process).err
getenv = True
should_transfer_files = yes
when_to_transfer_output = on_exit
transfer_output_remaps = "out.root = $Fn(myfile).root"
queue myfile matching files /gv0/Users/youngwan_public/WRTauNano/WRtoNTautoTauTauJJ_WR1000_N100_TuneCP5_13TeV_madgraph-pythia8/NANOAODSIM/106X_upgrade2018_realistic_v16_L1v1-v2/2530000/*.root
```
