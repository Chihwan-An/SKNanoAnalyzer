#to run this script, you need to install uproot.
import ROOT
import correctionlib.convert as convert
import json
import argparse
import os
import itertools
import copy
import sys

def remove_supersets(strings):
    result = []
    for s in strings:
        if any(s != other and s in other for other in strings):
            result.append(s)
    return result if result else strings

def histParser(f, target_binning):
    hist_dicts = {}
    trigs = []
    for keyObj in f.GetListOfKeys():
        key = keyObj.GetName()
        tokens = key.split("_"+target_binning)
        if len(tokens) == 1:
            continue
        trig = tokens[0]
        if not trig in trigs:
            trigs.append(trig)
    trigs = remove_supersets(trigs)
    for trig in trigs:
        hist_dicts[trig] = {}
        hist_dicts[trig]['effData'] = trig+"_"+target_binning+"_efficiencyData"
        hist_dicts[trig]['effData_stat'] = trig+"_"+target_binning+"_efficiencyData_stat"
        hist_dicts[trig]['effData_syst'] = trig+"_"+target_binning+"_efficiencyData_syst"
        hist_dicts[trig]['effMC'] = trig+"_"+target_binning+"_efficiencyMC"
        hist_dicts[trig]['effMC_stat'] = trig+"_"+target_binning+"_efficiencyMC_stat"
        hist_dicts[trig]['effMC_syst'] = trig+"_"+target_binning+"_efficiencyMC_syst"
    return hist_dicts 
        
def makeTempHist(hist_dicts, hist_root, tmp_root_path):
    f = ROOT.TFile(hist_root, "READ")
    f_out = ROOT.TFile(tmp_root_path, "RECREATE")
    for trig, hist_dict in hist_dicts.items():
        
        hist_effData = f.Get(hist_dict['effData'])
        hist_effData_stat = f.Get(hist_dict['effData_stat'])
        hist_effData_syst = f.Get(hist_dict['effData_syst'])
        hist_effMC = f.Get(hist_dict['effMC'])
        hist_effMC_stat = f.Get(hist_dict['effMC_stat'])
        hist_effMC_syst = f.Get(hist_dict['effMC_syst'])
        hist_effDataUp = hist_effData.Clone(hist_dict['effData']+"Up")
        hist_effDataDown = hist_effData.Clone(hist_dict['effData']+"Down")
        hist_effMCUp = hist_effMC.Clone(hist_dict['effMC']+"Up")
        hist_effMCDown = hist_effMC.Clone(hist_dict['effMC']+"Down")
        for binX, binY in itertools.product(range(1,hist_effData.GetNbinsX()+1), range(1,hist_effData.GetNbinsY()+1)):
            effData = hist_effData.GetBinContent(binX,binY)
            effData_stat = hist_effData_stat.GetBinError(binX,binY)
            effData_syst = hist_effData_syst.GetBinError(binX,binY)
            effMC = hist_effMC.GetBinContent(binX,binY)
            effMC_stat = hist_effMC_stat.GetBinError(binX,binY)
            effMC_syst = hist_effMC_syst.GetBinError(binX,binY)
            effDataUp = effData + (effData_stat**2 + effData_syst**2)**0.5
            effDataDown = effData - (effData_stat**2 + effData_syst**2)**0.5
            effMCUp = effMC + (effMC_stat**2 + effMC_syst**2)**0.5
            effMCDown = effMC - (effMC_stat**2 + effMC_syst**2)**0.5
            hist_effDataUp.SetBinContent(binX,binY,effDataUp)
            hist_effDataDown.SetBinContent(binX,binY,effDataDown)
            hist_effMCUp.SetBinContent(binX,binY,effMCUp)
            hist_effMCDown.SetBinContent(binX,binY,effMCDown)
        f_out.cd()
        hist_effDataUp.Write()
        hist_effDataDown.Write()
        hist_effMCUp.Write()
        hist_effMCDown.Write()
        hist_effData.Write()
        hist_effMC.Write()
    f_out.Close()
    f.Close()
      
def _binning_inputs(target_binning):
    if target_binning == 'abseta_pt':
        return ('abseta', 'pt')
    if target_binning == 'eta_pt':
        return ('eta', 'pt')
    return None

def makingJson(hist_root, target_binning, tmp_root_path):
    f = ROOT.TFile(hist_root, "READ")
    hist_dicts = histParser(f, target_binning)
    if len(hist_dicts) == 0:
        print(f"No histograms found for binning '{target_binning}' in {hist_root}")
        f.Close()
        return None
    
    trigs = []
    main_json = {"schema_version": 2,
            "description": "This json file contains the efficiency of muon triggers",
            "corrections": []}
    categorys={
        
    }
    
    axis_inputs = _binning_inputs(target_binning)
    if axis_inputs is None:
        print("If you want to use different binning, please modify the code")
        f.Close()
        return None

    axis_x, axis_y = axis_inputs
    categorys['data_or_mc'] = {"type":"string","description":"data or mc","content":["data","mc"]}
    categorys['scale_factors'] = {"type":"string","description":"eff,effup,effdown","content":[]}
    categorys[axis_x] = {"type":"real","description":axis_x}
    categorys[axis_y] = {"type":"real","description":axis_y}

    makeTempHist(hist_dicts, hist_root, tmp_root_path)
    f.Close()

    trigs = hist_dicts.keys()
    
    categorys['scale_factors']['content'] = ['nominal','systup','systdown']
    
    for trig in trigs:
        trig_eff_dict={}
        trig_eff_dict['name'] = trig
        trig_eff_dict['version'] = 0
        trig_eff_dict['inputs'] = []
        trig_eff_dict['output'] = {"name":"weight","type":"real"}
        
        for name, cat in categorys.items():
            trig_eff_dict['inputs'].append(copy.deepcopy(cat))
            trig_eff_dict['inputs'][-1]['name'] = name
            if 'content' in trig_eff_dict['inputs'][-1].keys():
                del trig_eff_dict['inputs'][-1]['content']

        
        eff_dict = {}
        
        eff_dict['effdata'] = convert.from_uproot_THx(f'{tmp_root_path}:{trig}_{target_binning}_efficiencyData')
        eff_dict['effdataup'] = convert.from_uproot_THx(f'{tmp_root_path}:{trig}_{target_binning}_efficiencyDataUp')
        eff_dict['effdatadown'] = convert.from_uproot_THx(f'{tmp_root_path}:{trig}_{target_binning}_efficiencyDataDown')
        eff_dict['effmc'] = convert.from_uproot_THx(f'{tmp_root_path}:{trig}_{target_binning}_efficiencyMC')
        eff_dict['effmcup'] = convert.from_uproot_THx(f'{tmp_root_path}:{trig}_{target_binning}_efficiencyMCUp')
        eff_dict['effmcdown'] = convert.from_uproot_THx(f'{tmp_root_path}:{trig}_{target_binning}_efficiencyMCDown')
        
        trig_eff_dict['data'] = {"nodetype": "category",
                                    "input": "data_or_mc",
                                    "content": [{"key":"data","value":{"nodetype":"category","input":"scale_factors","content":[]}},{"key":"mc","value":{"nodetype":"category","input":"scale_factors","content":[]}}]}

        
        for key, eff in eff_dict.items():
            eff = eff.dict()['data']
            eff['inputs'] = [axis_x, axis_y]
            eff['flow'] = 'clamp'
            if 'effdata' in key:
                this_key = 'nominal'
                if 'up' in key:
                    this_key = 'systup'
                elif 'down' in key:
                    this_key = 'systdown'
                trig_eff_dict['data']['content'][0]['value']['content'].append({"key":this_key,"value":eff})
            else:
                this_key = 'nominal'
                if 'up' in key:
                    this_key = 'systup'
                elif 'down' in key:
                    this_key = 'systdown'
                trig_eff_dict['data']['content'][1]['value']['content'].append({"key":this_key,"value":eff})
                
        
        main_json['corrections'].append(trig_eff_dict)
        
        
        
    return main_json
                    
                
        
        




if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Create JSON file for b-tagging efficiency')
    parser.add_argument('--out_name_str',dest='out_name_str', help='Output JSON file name', default='')
    parser.add_argument('--input_root',dest='input_root', help='Input ROOT file (bypass SKNANO_DATA lookup)', default='')
    parser.add_argument('--out_dir',dest='out_dir', help='Output directory for JSON', default='')
    parser.add_argument('--out_json',dest='out_json', help='Output JSON path (overrides out_dir)', default='')
    parser.add_argument('--target_binning',dest='target_binning', help='Binning name in histogram keys', default='abseta_pt')
    parser.add_argument('--tmp_root',dest='tmp_root', help='Temporary ROOT file path', default='output.root')
    parser.add_argument('--eras',dest='eras', help='Comma-separated eras (legacy mode)', default='')
    args = parser.parse_args()
    out_name_str = args.out_name_str

    if args.input_root:
        hist_root = args.input_root
        target_binning = args.target_binning
        main_json = makingJson(hist_root, target_binning, args.tmp_root)
        if main_json is None:
            sys.exit(1)
        if args.out_json:
            out_path = args.out_json
        else:
            out_dir = args.out_dir if args.out_dir else os.getcwd()
            out_path = os.path.join(out_dir, out_name_str + 'muon_trig.json')
        with open(out_path, 'w') as f:
            json.dump(main_json, f, indent=4)
        sys.exit(0)

    totalEras = ['2022','2022EE']
    totalEras = ['2017','2016preVFP','2016postVFP','2018']
    if args.eras:
        totalEras = [x.strip() for x in args.eras.split(',') if x.strip()]
    for era in totalEras:
        out_dir = args.out_dir if args.out_dir else os.path.join(os.environ['SKNANO_DATA'], era, 'MUO')
        hist_root = os.path.join(out_dir, 'SingleMuonTriggers.root')
        if not os.path.exists(hist_root):
            hist_roots = [x for x in os.listdir(out_dir) if 'SingleMuonTriggers.root' in x]
            if len(hist_roots) == 0:
                continue
            hist_root = os.path.join(out_dir, hist_roots[0])
        main_json = makingJson(hist_root, args.target_binning, args.tmp_root)
        if main_json is None:
            continue
        with open(os.path.join(out_dir, out_name_str+'muon_trig.json'), 'w') as f:
            json.dump(main_json, f, indent=4)

