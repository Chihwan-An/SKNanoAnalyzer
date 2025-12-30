# SKFlatAnalyzer Tutorial
## Contents
- Introduction
- Environment setup
- Job submission
- Comparing data & mc

## Introduction
In this tutorial, we will check the existence of a massive neutral weak force carrier, the $Z$ boson. 
The lifetime of the $\mathrm{Z}$ boson is very short,

$$ \Delta t \le 1/\Delta m \sim 10^{-26}s $$

which cannot be detected by the CMS detector. 
One way to check the existence is to reconstruct the lorentz invariant mass from its decay products.
Typically, $\mathrm{Z}$ boson decays to a pair of muons. 
If the boson exists, it is possible to detect a large increase of production rate around its own invariant mass,

$$ \sigma(q\bar{q} \rightarrow \mathrm{Z} \rightarrow \mu^+ \mu^-) \sim |{1 \over {s - m_Z^2+im_z\Gamma_Z}}|^2 $$

We call this method a resonant search.

Take a look at the contents of TutorialRun.C before you submit the jobs. It selects the events with two oppositely signed muons with the reconstructed mass not differ than 15 $\mathrm{GeV}$ of the $\mathrm{Z}$ mass.

There are also many processes that can match to the event selection for the search of $\mathrm{Z}$ boson. 
In this tutorial, let's call the Drell-Yan process as the signal process and the other processes as the background processes. Here are some examples of other processes:
- TTbar production
- Diboson production
- Single Top associated with W boson

In the data, it is impossible to make each event to be a certain process if several processes share the same final phase space. 
Therefore, it is also important to estimate how many events are coming from the background processes as well as from the signal process.

## Environment setup
Follow the instruction in https://github.com/CMSSNU/SKFlatAnalyzer
Check whether the test job runs fine. Tips: you can use --reduction option for the test run:
```bash
SKFlat.py -a ExampleRun -i DYJets -n 1 -e 2017 --reduction 100 &
```

*Caution*: When you use *VSCode* you should use cms1 or cms2 server to edit files. /data6 and /data9 can be accesed from cms1 and cms2. Also cms1(alma8 base OS) is preferred as the support of slc7 of VScode dropped.

When if you want to submit jobs to condor, you can use tamsa1 or tamsa2 server by terminal. Also you should compile one more time in tamsa server after you edit files.

And remember that you  *should work at /data6 or /data9* not at /home. SKFLAT will not work due to header file linking problem if you start with wrong directory.


In this tutorial, we will use TutorialRun analyzer. You can check the file named TutorialRun.h and TutorialRun.C in the TutorialRun directory. Let's move them to the SKFlatAnalyzer:
```bash
cd /data6/Users/${USER}/SKFlatAnalyzer
source setup.sh
export TUTORIALRUN="/data6/Users/snu_public/4.SNU-CMS/Tutorials/SKFlatAnalyzer/Zmass" # or other place
cp ${TUTORIALRUN}/TutorialRun/TutorialRun.h $SKFlat_WD/Analyzers/include/
cp ${TUTORIALRUN/TutorialRun/TutorialRun.C $SKFlat_WD/Analyzers/src/
```
and add
```cpp
#pragma link C++ class TutorialRun+;
```
in $SKFlat_WD/Analyzers/include/Analyzers_LinkDef.h

Finally, compile the analyzer:
```bash
cd $SKFlat_WD
make
```

## Job Submission
You can use either TutorialRun/submit.sh or submit directly from the SKFlatAnalyzer directory:
```bash
cd $SKFlat_WD
SKFlat.py -a TutorialRun -i SingleMuon -n 10 -e 2017 &
SKFlat.py -a TutorialRun -l MCList_tutorial.txt -n 10 -e 2017 &
```
It will submit the jobs to condor. To check whether your jobs are running okay, use:
```bash
condor_q $USER	# you can change $USER to your user name, for example condor_q choij
```

After jobs are finished, it will send the outputs to the /data6/Users/${USER}/SKFlatOutput/Run2UltraLegacy_v3/2017/TutorialRun/... 

If any problem occurs, check the log file in /data6/Users/${USER}/SKFlatRunlog/...

> Problem 1.
> Let's see how many events can survive after each step, such as trigger or MET filter.
> The ratio of events before and after each cut is called efficiency of the cut,
> and the table of the number of events for all the cuts is called a cutflow.
> In TutorialRun.C, I made a skeleton of cutflow using FillHist function in ${SKFlat_WD}/Analyzers/src/AnalyzerCore.C
> Complete the cutflow of TutorialRun.C


## Comparing data & mc
You can directly compare the number of observed(from data) and expected(from MCs) events within the Z-mass window. 
> Problem 2.
> Let's check how many events passed the event selection
> both for data and MCs.
> In the limit of large statistics, its error can be approximated as Gaussian,
> i.e. stat error ~ sqrt(events)
> Do data and MCs follow gaussian approximation?
> If not, think about the reason why.

Also, it is a good idea to compare the distribution of data and MCs. 
In this example, we will use pyROOT3 with python >= 3.6 to draw the plots. 
One way to set the environment in the tamsa(tamsa2 is recommended) server is using the LCG environment:
```bash
export TUTORIALBASE="/data6/Users/${USER}/Tutorial"
mkdir -p $TUTORIALBASE
source /cvmfs/sft.cern.ch/lcg/views/LCG_102cuda/x86_64-centos7-gcc8-opt/setup.sh
```
> You can also install root in your local machine. Check more information for installing the root package in [install-root](https://root.cern/install/)

Let's move the output root files from SKFlatAnalyzer to the CMSSW directory. Also, let's get the plotting scripts from $TUTORIALRUN directory.
```bash
cd ${TUTORIALBASE}
cp -r /data6/Users/${USER}/SKFlatOutput/Run2UltraLegacy_v3/TutorialRun/2017/ ${TUTORIALBASE}
cp -r ${TUTORIALRUN}/Plotter ${TUTORIALBASE}
cp ${TUTORIALRUN}/plot.py ${TUTORIALBASE}
```
hadd(horizontal add) the outputs of the TutorialRun.
```bash
mv 2017 TutorialOutput && cd TutorialOutput
cp ${TUTORIALRUN}/hadd.sh .
source hadd.sh
cd ${TUTORIALBASE}
```

Finally, let's draw the plot:
```bash
python plot.py
```
The script will create a plot named DataAndMC\_POGMedium\_ZCand\_mass.png. You can check about 10% discripency between the data and MCs. There could be some other processes responsible for the discrepancy(maybe BSM?), but you should check the difference in efficiencies of IDs, triggers, B-taggings... between the data and MCs, or any mismodelled effect in the simulations(Is it leading order process? Next leading Order? Any jet-matching algorithm applied? PDFs? How large are statistic and systematic errors?) fisrt.

> Problem 3.
> Check the kinematic distributions of decay products, i.e. the leading and the subleading muons.
> After comparing data and MCs, you can check different kinematics of Drell-Yan and other processes.
> Some variables can be used to suppress non-DY backgrounds (for example, ttbar).
> Can you implement additional cuts to suppress background further?
> HINT: Check the distributions of jet & b-jet multiplicity(size) and MET (Missing transverse energy).
> Also check the signal sensitivity (S/sqrt(B)) before and after giving each cut.

TODO: Add tutorials to set weights such as ID/Trigger SFs, PileUp weights, B-tagging SFs...

---
- Author: Jin Choi 
- Contact: choij@cern.ch
- Last Update: October 24, 2023