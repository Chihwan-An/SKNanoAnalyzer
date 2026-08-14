#!/usr/bin/env python3
import argparse
import fcntl
import json
import os
import re
import tempfile
from contextlib import contextmanager
#This script is used to generate the path information for the sample data
#Sample information is stored in the CommonSampleInfo.json
#This script will generate the path information for the sample data
basePath = os.environ['SKNANO_RUN3_NANOAODPATH'] 
    
def loadCommonSampleInfo(era):
    sampleInfoJson = os.path.join(os.environ['SKNANO_DATA'],era,'Sample','CommonSampleInfo.json')
    sampleInfos = {} 
    try:
        sampleInfos = json.load(open(sampleInfoJson))
    except Exception as e:
        print(f'Error: on {era}')
        print(e)
    return sampleInfos

def atomic_write_json(target_path, payload):
    """Write JSON atomically to avoid partial writes when multiple jobs run."""
    dirpath = os.path.dirname(target_path) or "."
    fd, tmp_path = tempfile.mkstemp(dir=dirpath, prefix=os.path.basename(target_path) + ".tmp.")
    try:
        with os.fdopen(fd, "w") as tmp_file:
            json.dump(payload, tmp_file, indent=4)
        os.replace(tmp_path, target_path)
    finally:
        if os.path.exists(tmp_path):
            os.remove(tmp_path)

@contextmanager
def skim_tree_info_lock(lock_path):
    """Serialize skim metadata updates to avoid concurrent writers."""
    os.makedirs(os.path.dirname(lock_path), exist_ok=True)
    with open(lock_path, "w") as lock_file:
        fcntl.flock(lock_file, fcntl.LOCK_EX)
        try:
            yield
        finally:
            fcntl.flock(lock_file, fcntl.LOCK_UN)

def checkSamplePaths(era):
    """Report how each registry entry resolves against $SKNANO_INPUT_ROOT.

    Sample inputs are derived from era plus PD (MC) or name and period (DATA),
    so there is nothing to write out any more -- what used to be
    --fillSamplePath. What is still worth doing is asking whether the
    derivation finds anything, which is how a half-migrated production or a
    renamed dataset shows up before a submission does.
    """
    from python.sample_paths import resolve_sample_paths

    sampleInfos = loadCommonSampleInfo(era)
    resolved = missing = 0
    for alias, sampleInfo in sorted(sampleInfos.items()):
        periods = [None] if sampleInfo.get('isMC') else sampleInfo.get('periods', [])
        for period in periods:
            label = alias if period is None else f'{alias}_{period}'
            try:
                paths = resolve_sample_paths(
                    dict(sampleInfo, name=alias), era, period)
            except Exception as error:
                print(f'\033[91m  {label}: {error}\033[0m')
                missing += 1
                continue
            print(f'  {label}: {len(paths)} files')
            resolved += 1
    print(f'{era}: {resolved} resolved, {missing} unresolved')


def updateXsec(era):
    sampleInfos = loadCommonSampleInfo(era)
    for alias, sampleInfo in sampleInfos.items():
        if sampleInfo['isMC'] and 'xsec_formula' in sampleInfo:
            xsec = eval(sampleInfo['xsec_formula'])
            sampleInfo['xsec'] = xsec
    sampleInfoJson = os.path.join(os.environ['SKNANO_DATA'],era,'Sample','CommonSampleInfo.json')
    with open(sampleInfoJson,'w') as f:
        json.dump(sampleInfos,f,indent=4)
        
def updateMcInfo(era):
    import ROOT
    sampleInfos = loadCommonSampleInfo(era)
    for alias, sampleInfo in sampleInfos.items():
        if sampleInfo['isMC']:
            try:
                f = ROOT.TFile.Open(os.path.join(os.environ['SKNANO_OUTPUT'],'GetEffLumi',era,alias+'.root'))
            except:
                print(f'File {alias}.root not found')
                print('############################\n')
                continue
            h_sumW = f.Get('sumW')
            h_sumSign = f.Get('sumSign')
            nmc = h_sumW.GetEntries()
            sumW = h_sumW.GetBinContent(1)
            sumSign = h_sumSign.GetBinContent(1)
            sampleInfo['nmc'] = nmc
            sampleInfo['sumW'] = sumW
            sampleInfo['sumsign'] = sumSign
            print('Will update the MC information for',alias,'from')
            print(f'nmc:{sampleInfo["nmc"]}, sumW:{sampleInfo["sumW"]}, sumSign:{sampleInfo["sumsign"]} to')
            print(f'nmc:{nmc}, sumW:{sumW}, sumSign:{sumSign}')
            print('############################\n')
        else:
            nevt = []
            for period in sampleInfo['periods']:
                try:
                    f = ROOT.TFile.Open(os.path.join(os.environ['SKNANO_OUTPUT'],'GetEffLumi',era,alias+f'_{period}.root'))
                except:
                    print(f'File {alias}_{period}.root not found')
                    print('############################\n')
                    continue
                h = f.Get('NEvents')
                NEvents = h.GetBinContent(1)
                nevt.append(NEvents)
            print('Will update the DATA information for',alias,'from')
            print(f'nmc:{sampleInfo["NEvents"]} to')
            print(f'nmc:{nevt}')
            print('############################\n')
            sampleInfo['NEvents'] = nevt
    sampleInfoJson = os.path.join(os.environ['SKNANO_DATA'],era,'Sample','CommonSampleInfo.json')
    with open(sampleInfoJson,'w') as f:
        json.dump(sampleInfos,f,indent=4)

def makeSkimTreeInfo(era,skimTreeFolder,skimTreeSuffix,skimTreeOrigPD):
    from copy import deepcopy
    isMC = True
    period = None
    data_match = re.match(r"^(?P<pd>.+)_(?P<period>[A-Z](?:_v\d+)?)$", skimTreeOrigPD)
    if data_match:
        isMC = False
        period = data_match.group("period")
        skimTreeOrigPD = data_match.group("pd")
        
    sampleInfos = loadCommonSampleInfo(era)
    # Skim metadata belongs to the analysis that produced the skim, not to the
    # backend. Write it into the module directory, which is where SKNano.py
    # discovers it from.
    module_root = os.environ.get('SKNANO_SKIM_METADATA_DIR')
    if not module_root:
        raise SystemExit(
            'SKNANO_SKIM_METADATA_DIR is not set; point it at the '
            '<module>/<Analysis>/data/Skim directory that owns these skims')
    skimJsonFolderPath = os.path.join(module_root, era)
    os.makedirs(skimJsonFolderPath, exist_ok=True)
    skimTreeSummaryJsonPath = os.path.join(skimJsonFolderPath,'skimTreeInfo.json')
    lock_path = os.path.join(skimJsonFolderPath, '.skimTreeInfo.lock')
    summary_key = f'Skim_{skimTreeSuffix}_{skimTreeOrigPD}'

    #just for redundancy
    if isMC != sampleInfos[skimTreeOrigPD]['isMC']:
        print('Error: The sample type is not matched')
        return
    
    if isMC:
        path = os.path.join(skimTreeFolder, summary_key)
        skimPathInfoJson = os.path.join(skimJsonFolderPath, f'{summary_key}.json')
    else:
        path = os.path.join(skimTreeFolder, summary_key, f'Period{period}')
        skimPathInfoJson = os.path.join(skimJsonFolderPath, f'{summary_key}_{period}.json')

    filePaths = []
    print(path)
    for root, dirs, files in os.walk(path):
        for file in files:
            if file.endswith('.root'):
                filePaths.append(os.path.join(root,file))

    with skim_tree_info_lock(lock_path):
        if os.path.exists(skimTreeSummaryJsonPath):
            skimTreeSummary = json.load(open(skimTreeSummaryJsonPath))
        else:
            skimTreeSummary = {}

        if summary_key in skimTreeSummary:
            skimTreeSummary[summary_key]['suffix'] = skimTreeSuffix
            skimTreeSummary[summary_key]['PD'] = skimTreeOrigPD
            skimTreeSummary[summary_key]['isMC'] = int(isMC)
            if not isMC:
                existing_periods = skimTreeSummary[summary_key].setdefault('periods', [])
                if period not in existing_periods:
                    existing_periods.append(period)
        else:
            skimTreeSummary[summary_key] = {'suffix':skimTreeSuffix,'PD':skimTreeOrigPD,'isMC':int(isMC)}
            if not isMC:
                skimTreeSummary[summary_key]['periods'] = [period]

        skimTreePath = deepcopy(skimTreeSummary[summary_key])
        skimTreePath['name']= summary_key
        skimTreePath.pop('periods',None)
        skimTreePath['path'] = filePaths
        atomic_write_json(skimPathInfoJson, skimTreePath)
        atomic_write_json(skimTreeSummaryJsonPath, skimTreeSummary)
    
if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--checkSamplePaths', action='store_true',help='Report how each sample resolves under $SKNANO_INPUT_ROOT')
    parser.add_argument('--updateXsec',action='store_true',help='update the Xsec to json from Xsec formula')
    parser.add_argument('--updateMcInfo',action='store_true',help='update the MC information to json from result of GetEffLumi(SumW, nmc)')
    ###below arguments are used for SKFlat.py for automatic update of the skim information
    parser.add_argument('--makeSkimTreeInfo',action='store_true',help='Make the SkimTreeInfo')
    parser.add_argument('--skimTreeFolder',dest='skimTreeFolder',default='',help='Folder where the skim tree is stored')
    parser.add_argument('--skimTreeSuffix',dest='skimTreeSuffix',default='',help='Suffix of the skim tree')
    parser.add_argument('--skimTreeOrigPD', dest='skimTreeOrigPD',default='',help='Original PD of the skim tree')
    parser.add_argument('--era',dest='era',default='',help='Era of the sample')
    args = parser.parse_args()
    run3eras = ['2022','2022EE', '2023', '2023BPix']
    run2eras = ['2016preVFP','2016postVFP','2017','2018']
    eras = run3eras + run2eras
    eras = ['2024']
    if args.era == '':
        for era in eras:
            if era in run3eras:
                basePath = os.environ['SKNANO_RUN3_NANOAODPATH']
            elif era in run2eras:
                basePath = os.environ['SKNANO_RUN2_NANOAODPATH']
            if args.checkSamplePaths:
                checkSamplePaths(era)
            if args.updateXsec:
                updateXsec(era)
            if args.updateMcInfo:
                updateMcInfo(era)
            


    else:
        era = args.era
        if era in run3eras:
            basePath = os.environ['SKNANO_RUN3_NANOAODPATH']
        elif era in run2eras:
            basePath = os.environ['SKNANO_RUN2_NANOAODPATH']
        if args.checkSamplePaths:
            checkSamplePaths(era)
        if args.updateXsec:
            updateXsec(era)
        if args.updateMcInfo:
            updateMcInfo(era)
        if args.makeSkimTreeInfo:
            makeSkimTreeInfo(era,args.skimTreeFolder,args.skimTreeSuffix, args.skimTreeOrigPD)
                
