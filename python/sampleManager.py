#!/usr/bin/env python3
import argparse
import fcntl
import json
import os
import tempfile
from contextlib import contextmanager
from multiprocessing import Pool
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

# def fillSamplePath(era):
#     sampleInfos = loadCommonSampleInfo(era)
#     for alias,sampleInfo in sampleInfos.items():
        # if sampleInfo['isMC']:
        #     path = os.path.join(basePath,era,'MC',sampleInfo['PD'])
        #     filePaths = []
        #     #Folder structure is not fixed yet, so let's do the recursive search until the .root file appears, and save all absolute paths
        #     for root, dirs, files in os.walk(path):
        #         for file in files:
        #             if file.endswith('.root'):
        #                 filePaths.append(os.path.join(root,file))
        #     #sort filePaths by tree*.root
        #     #filePaths = sorted(filePaths,key=lambda x: int(x.split('tree_')[-1].split('.root')[0])) 
        #     #now save the path information to another json file
        #     newjsondict = {}
        #     newjsondict['name'] = alias
        #     for key in sampleInfo:
        #         newjsondict[key] = sampleInfo[key]
        #     fileJsonPath = os.path.join(os.environ['SKNANO_DATA'],era,'Sample','ForSNU',alias+'.json')
        #     newjsondict['path'] = filePaths
        #     with open(fileJsonPath,'w') as f:
        #         json.dump(newjsondict,f,indent=4)
        # else:
        #     for period in sampleInfo['periods']:
        #         path = os.path.join(basePath,era,'DATA',alias,f"Period{period}")
        #         filePaths = []
        #         #Folder structure is not fixed yet, so let's do the recursive search until the .root file appears, and save all absolute paths
        #         for root, dirs, files in os.walk(path):
        #             for file in files:
        #                 if file.endswith('.root'):
        #                     filePaths.append(os.path.join(root,file))
        #         #sort filePaths by tree*.root
        #         #filePaths = sorted(filePaths,key=lambda x: int(x.split('tree_')[-1].split('.root')[0]))
        #         #now save the path information to another json file
        #         newjsondict = {}
#         newjsondict['name'] = alias
#         for key in sampleInfo:
#             if key == 'periods':
#                 continue
#             newjsondict[key] = sampleInfo[key]
#         fileJsonPath = os.path.join(os.environ['SKNANO_DATA'],era,'Sample','ForSNU',alias+f'_{period}.json')
#         newjsondict['path'] = filePaths
#         with open(fileJsonPath,'w') as f:
#             json.dump(newjsondict,f,indent=4)

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

# Top-level function to call either process_mc_sample or process_data_sample
def process_sample(fn, alias, sampleInfo, era, basePath):
    fn(alias, sampleInfo, era, basePath)

def process_mc_sample(alias, sampleInfo, era, basePath):
    path = os.path.join(basePath, era, 'MC', sampleInfo['PD'])
    filePaths = []
    for root, dirs, files in os.walk(path):
        for file in files:
            if file.endswith('.root'):
                filePaths.append(os.path.join(root, file))
    #filePaths = sorted(filePaths, key=lambda x: int(x.split('tree_')[-1].split('.root')[0]))

    newjsondict = {'name': alias}
    newjsondict.update(sampleInfo)
    fileJsonPath = os.path.join(os.environ['SKNANO_DATA'], era, 'Sample', 'ForSNU', alias + '.json')
    newjsondict['path'] = filePaths
    with open(fileJsonPath, 'w') as f:
        json.dump(newjsondict, f, indent=4)

def process_data_sample(alias, sampleInfo, era, basePath):
    for period in sampleInfo['periods']:
        path = os.path.join(basePath, era, 'DATA', alias, f"Period{period}")
        filePaths = []
        for root, dirs, files in os.walk(path):
            for file in files:
                if file.endswith('.root'):
                    filePaths.append(os.path.join(root, file))
        #filePaths = sorted(filePaths, key=lambda x: int(x.split('tree_')[-1].split('.root')[0]))

        newjsondict = {'name': alias}
        for key in sampleInfo:
            if key == 'periods':
                continue
            newjsondict[key] = sampleInfo[key]
        fileJsonPath = os.path.join(os.environ['SKNANO_DATA'], era, 'Sample', 'ForSNU', alias + f'_{period}.json')
        newjsondict['path'] = filePaths
        with open(fileJsonPath, 'w') as f:
            json.dump(newjsondict, f, indent=4)

def fillSamplePath(era):
    sampleInfos = loadCommonSampleInfo(era)
    
    # Create a list of tasks for multiprocessing
    tasks = []
    for alias, sampleInfo in sampleInfos.items():
        if sampleInfo['isMC']:
            tasks.append((process_mc_sample, alias, sampleInfo, era, basePath))
        else:
            tasks.append((process_data_sample, alias, sampleInfo, era, basePath))

    # Use multiprocessing to parallelize
    with Pool(processes=16) as pool:
        pool.starmap(process_sample, tasks)



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
    if skimTreeOrigPD.split("_")[-1].isupper() and len(skimTreeOrigPD.split("_")[-1]) == 1:
        #if sample is ends with _one capital letter, it is data
        #hope there will be no exception(please)
        isMC = False
        period = skimTreeOrigPD.split("_")[-1]
        skimTreeOrigPD = skimTreeOrigPD[:-2]
        
    sampleInfos = loadCommonSampleInfo(era)
    skimJsonFolderPath = os.path.join(os.environ['SKNANO_DATA'],era,'Sample','Skim')
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
    parser.add_argument('--fillSamplePath', action='store_true',help='Fill the path information')
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
            if args.fillSamplePath:
                fillSamplePath(era)
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
        if args.fillSamplePath:
            fillSamplePath(era)
        if args.updateXsec:
            updateXsec(era)
        if args.updateMcInfo:
            updateMcInfo(era)
        if args.makeSkimTreeInfo:
            makeSkimTreeInfo(era,args.skimTreeFolder,args.skimTreeSuffix, args.skimTreeOrigPD)
                
