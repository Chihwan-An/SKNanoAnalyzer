#!/usr/bin/env python3

data_directory = "/gv0/Users/achihwan/SKNanoOutput/Reproduce20_002/2023/"

import ROOT
import cmsstyle as CMS
import os
import glob
from array import array
import gc
import argparse
import numpy as np
import pandas as pd

# Prevent ROOT from owning Python objects
ROOT.SetOwnership(ROOT.gROOT, False)

# Colors for plotting
DATA_COLOR = ROOT.kBlack
SIGNAL_COLOR = ROOT.TColor.GetColor("#e42536")  # Red for TTLJ signal

def normalize_by_bin_size(hist):
    """
    히스토그램의 각 빈 내용을 해당 빈의 크기로 나누어 정규화하는 함수
    """
    first_bin_width = hist.GetXaxis().GetBinWidth(1)
    is_discrete = True
    for i in range(1, hist.GetNbinsX() + 1):
        if abs(hist.GetXaxis().GetBinWidth(i) - first_bin_width) > 1e-6:
            is_discrete = False
            break
    
    if is_discrete and abs(first_bin_width - 1.0) < 1e-6:
        # print(f"Skipping normalization for discrete variable: {hist.GetName()} (bin width: {first_bin_width})")
        return hist
    
    normalized_hist = hist.Clone(f"{hist.GetName()}_normalized")
    normalized_hist.SetDirectory(0)
    
    # print(f"Normalizing histogram: {hist.GetName()} (bin width: {first_bin_width})")
    
    for i in range(1, hist.GetNbinsX() + 1):
        content = hist.GetBinContent(i)
        error = hist.GetBinError(i)
        bin_width = hist.GetXaxis().GetBinWidth(i)
        
        if bin_width > 0:
            normalized_content = content / bin_width
            normalized_error = error / bin_width
            normalized_hist.SetBinContent(i, normalized_content)
            normalized_hist.SetBinError(i, normalized_error)
        else:
            normalized_hist.SetBinContent(i, content)
            normalized_hist.SetBinError(i, error)
            
    return normalized_hist

def load_scale_factors(sf_file_path):
    """
    스케일 팩터 CSV 파일을 로딩하는 함수
    """
    if not os.path.exists(sf_file_path):
        print(f"Warning: Scale factor file not found: {sf_file_path}")
        return {}
    
    try:
        df = pd.read_csv(sf_file_path)
        scale_factors = {}
        
        for _, row in df.iterrows():
            hist_name = row['hist_name']
            bin_min = float(row['bin_min'])
            bin_max = float(row['bin_max'])
            scale_factor = float(row['scale_factor'])
            
            if hist_name not in scale_factors:
                scale_factors[hist_name] = []
            
            scale_factors[hist_name].append((bin_min, bin_max, scale_factor))
        
        print(f"Loaded scale factors for {len(scale_factors)} histograms from {sf_file_path}")
        return scale_factors
        
    except Exception as e:
        print(f"Error loading scale factors from {sf_file_path}: {e}")
        return {}

def apply_scale_factors(hist, hist_name, scale_factors):
    """
    히스토그램에 빈별 스케일 팩터를 적용하는 함수
    """
    if not hist or hist_name not in scale_factors:
        return hist
    
    scaled_hist = hist.Clone(f"{hist.GetName()}_scaled")
    scaled_hist.SetDirectory(0)
    ROOT.SetOwnership(scaled_hist, False)
    
    # print(f"Applying scale factors to {hist_name}")
    
    for i in range(1, hist.GetNbinsX() + 1):
        bin_low = hist.GetBinLowEdge(i)
        bin_high = hist.GetBinLowEdge(i + 1)
        
        scale_factor = 1.0
        for bin_min, bin_max, sf in scale_factors[hist_name]:
            if bin_low >= bin_min and bin_high <= bin_max:
                scale_factor = sf
                break
        
        if scale_factor != 1.0:
            original_content = hist.GetBinContent(i)
            original_error = hist.GetBinError(i)
            
            scaled_content = original_content * scale_factor
            scaled_error = original_error * scale_factor
            
            scaled_hist.SetBinContent(i, scaled_content)
            scaled_hist.SetBinError(i, scaled_error)
    
    return scaled_hist

BACKGROUND_COLORS = [
    ROOT.TColor.GetColor("#ffeb3b"), # DY
    ROOT.TColor.GetColor("#f44336"), # TTbar
    ROOT.TColor.GetColor("#4caf50"), # nonprompt (WJets)
    ROOT.TColor.GetColor("#2196f3")  # Others
]

class SignalBackgroundCanvas():
    def __init__(self, combined_data_hist, signal_hists, background_hists, config, draw_tb_lines=True, target_tb_sample=None, normalize_by_bin_size=False, blind_data=False):
        super().__init__()
        
        self.data_hist = combined_data_hist
        self.signal_hists = signal_hists
        self.background_hists = background_hists
        self.config = config
        self.draw_tb_lines = draw_tb_lines
        self.target_tb_sample = target_tb_sample
        self.normalize_by_bin_size = normalize_by_bin_size
        self.blind_data = blind_data  # Blind 플래그 저장
        
        self._objects_to_keep = []
        
        self.tb_hists = {}
        if self.draw_tb_lines:
            self._separate_tb_samples()
        
        self._build_background_stack()
        
        # Style the data histogram (Blind 모드가 아닐 때만)
        if self.data_hist and not self.blind_data:
            self.data_hist.SetMarkerStyle(20)
            self.data_hist.SetMarkerSize(1.0)
            self.data_hist.SetMarkerColor(DATA_COLOR)
            self.data_hist.SetLineColor(DATA_COLOR)
            self.data_hist.SetLineWidth(2)
            self.data_hist.SetStats(0)
            if not self.data_hist.GetSumw2N():
                self.data_hist.Sumw2()
        
        # Create ratio histogram (Blind 모드가 아닐 때만)
        self.ratio = None
        if self.data_hist and self.total_background and not self.blind_data:
            self.ratio = self.data_hist.Clone("data_stack_ratio")
            self.ratio.SetDirectory(0)
            self.ratio.SetStats(0)
            self.ratio.Divide(self.total_background)
            self._objects_to_keep.append(self.ratio)
        
        self._style_tb_histograms()
        self._setup_canvas()
    
    def _separate_tb_samples(self):
        if self.signal_hists:
            for name, hist in self.signal_hists.items():
                if name.startswith("TB"):
                    self.tb_hists[name] = hist
                    print(f"Using TB sample for line drawing: {name}")
        
    def _style_tb_histograms(self):
        for name, hist in self.tb_hists.items():
            if not hist.GetSumw2N():
                hist.Sumw2()
            hist.SetLineColor(ROOT.kRed)
            hist.SetLineWidth(2)
            hist.SetMarkerSize(0)
            hist.SetFillStyle(0)
            hist.SetStats(0)
            self._objects_to_keep.append(hist)
    
    def _build_background_stack(self):
        self.background_stack = ROOT.THStack("bg_stack", "Background + Signal")
        self._objects_to_keep.append(self.background_stack)
        self.total_background = None
        
        groups = {
            "TT+TW": [],
            "WJets": [],
            "DYJets": [],
            "Others": []
        }
        
        all_hists = {}
        if self.background_hists:
            all_hists.update(self.background_hists)
        if self.signal_hists:
            all_hists.update(self.signal_hists)
        
        if not all_hists:
            return
        
        for name, hist in all_hists.items():
            if name.startswith("TB") and self.draw_tb_lines:
                continue
            elif name.startswith("TTLJ") or name.startswith("TTLL") or name.startswith("ST"):
                groups["TT+TW"].append((name, hist))
            elif name.startswith("DYJets"):
                groups["DYJets"].append((name, hist))
            elif name.startswith("WJets"):
                groups["WJets"].append((name, hist))
            else:
                groups["Others"].append((name, hist))
        
        self.grouped_hists = {}
        group_colors = {
            "TT+TW": BACKGROUND_COLORS[1],
            "WJets": BACKGROUND_COLORS[2],
            "DYJets": BACKGROUND_COLORS[0],
            "Others": BACKGROUND_COLORS[3]  
        }
        
        group_integrals = []
        
        for group_name, samples in groups.items():
            if not samples:
                continue
            
            combined_hist = None
            total_integral = 0
            
            for sample_name, hist in samples:
                if not hist.GetSumw2N():
                    hist.Sumw2()
                    
                if combined_hist is None:
                    combined_hist = hist.Clone(f"combined_{group_name}")
                    combined_hist.SetDirectory(0)
                    self._objects_to_keep.append(combined_hist)
                else:
                    combined_hist.Add(hist)
                    
                total_integral += hist.Integral()
            
            if combined_hist is not None:
                combined_hist.SetFillColor(group_colors[group_name])
                combined_hist.SetLineColor(group_colors[group_name])
                combined_hist.SetLineWidth(1)
                combined_hist.SetFillStyle(1001)
                combined_hist.SetStats(0)
                
                self.grouped_hists[group_name] = combined_hist
                group_integrals.append((group_name, total_integral))
        
        group_integrals.sort(key=lambda x: x[1], reverse=True)
        
        for group_name, integral in reversed(group_integrals):
            hist = self.grouped_hists[group_name]
            self.background_stack.Add(hist)
            
            if self.total_background is None:
                self.total_background = hist.Clone("total_background")
                self.total_background.SetDirectory(0)
                self.total_background.SetStats(0)
                self._objects_to_keep.append(self.total_background)
            else:
                self.total_background.Add(hist)
        
        self.all_samples = group_integrals
    
    def _setup_canvas(self):
        ROOT.gStyle.SetOptStat(0)
        ROOT.gStyle.SetOptTitle(0)
        
        if "ymax" in self.config.keys() and self.config["ymax"] is not None:
            ymax = self.config["ymax"]
        else:
            # Blind 모드일 경우 data_max는 0으로 처리
            data_max = self.data_hist.GetMaximum() if (self.data_hist and not self.blind_data) else 0
            stack_max = self.total_background.GetMaximum() if self.total_background else 0
            ymax = max(data_max, stack_max) * 1.5
            if ymax <= 0:
                ymax = 1e8
        
        ymin = self.config.get("ymin", 0.1 if self.config.get("logy", False) else 0.)
        
        if self.config.get('logy', False):
            if "ymin" not in self.config.keys():
                ymin = 1e-1
            if ymax < 1:
                ymax = 1e8
        
        CMS.SetEnergy(13.6)
        CMS.SetLumi("17.794 fb^{-1}")
        CMS.SetExtraText("Preliminary")
        
        canvas_name = f"canvas_{id(self)}"
        self.canv = ROOT.TCanvas(canvas_name, canvas_name, 800, 800)
        ROOT.SetOwnership(self.canv, False)
        self._objects_to_keep.append(self.canv)
        
        pad1_name = f"pad1_{id(self)}"
        pad2_name = f"pad2_{id(self)}"
        
        pad1 = ROOT.TPad(pad1_name, pad1_name, 0, 0.3, 1, 1.0)
        pad1.SetBottomMargin(0.02)
        pad1.SetTopMargin(0.08)
        pad1.SetLeftMargin(0.12)
        pad1.SetRightMargin(0.05)
        ROOT.SetOwnership(pad1, False)
        self._objects_to_keep.append(pad1)
        
        pad2 = ROOT.TPad(pad2_name, pad2_name, 0, 0.0, 1, 0.3)
        pad2.SetTopMargin(0.02)
        pad2.SetBottomMargin(0.35)
        pad2.SetLeftMargin(0.12)
        pad2.SetRightMargin(0.05)
        ROOT.SetOwnership(pad2, False)
        self._objects_to_keep.append(pad2)
        
        if self.config.get('logy', False):
            pad1.SetLogy()
        
        pad1.Draw()
        pad2.Draw()
        
        self.pad1 = pad1
        self.pad2 = pad2
        
        self.xmin = self.config["xRange"][0]
        self.xmax = self.config["xRange"][-1]
        self.ymin = ymin
        self.ymax = ymax
        self.ratio_ymin = self.config["yRange"][0]
        self.ratio_ymax = self.config["yRange"][1]
        
        if self.total_background:
            self.bin_min = self.total_background.FindBin(self.xmin)
            self.bin_max = self.total_background.FindBin(self.xmax)
        elif self.data_hist:
            self.bin_min = self.data_hist.FindBin(self.xmin)
            self.bin_max = self.data_hist.FindBin(self.xmax)
        else:
            dummy = ROOT.TH1F("dummy_for_bins", "", 200, 0, 2000)
            self.bin_min = dummy.FindBin(self.xmin)
            self.bin_max = dummy.FindBin(self.xmax)
            dummy.Delete()
        
        n_entries = 6
        leg_height = 0.05 * (n_entries + 1)
        self.leg = ROOT.TLegend(0.8, 0.9 - leg_height, 0.92, 0.9)
        self.leg.SetTextSize(0.03)
        self.leg.SetBorderSize(0)
        self.leg.SetFillStyle(0)
        ROOT.SetOwnership(self.leg, False)
        self._objects_to_keep.append(self.leg)
    
    def draw(self):
        # =====================================
        # Upper pad - Main plot
        # =====================================
        self.pad1.cd()
        
        first_drawn = False
        if self.background_stack:
            self.background_stack.Draw("HIST")
            first_drawn = True
            
            self.background_stack.SetMinimum(self.ymin)
            self.background_stack.SetMaximum(self.ymax)
            
            stack_hist = self.background_stack.GetHistogram()
            if stack_hist:
                stack_hist.GetXaxis().SetRange(self.bin_min, self.bin_max)
                stack_hist.GetXaxis().SetRangeUser(self.xmin, self.xmax)
                stack_hist.GetYaxis().SetTitle(self.config["yTitle"])
                stack_hist.GetYaxis().SetTitleSize(0.05)
                stack_hist.GetYaxis().SetTitleOffset(1.2)
                stack_hist.GetYaxis().SetLabelSize(0.04)
                stack_hist.GetXaxis().SetLabelSize(0)
                stack_hist.GetXaxis().SetTickLength(0.03)
                ROOT.SetOwnership(stack_hist, False)
                self._objects_to_keep.append(stack_hist)
        
        if self.total_background:
            mc_error_band = self.total_background.Clone(f"mc_error_band_{id(self)}")
            mc_error_band.SetFillColor(ROOT.kBlack)
            mc_error_band.SetFillStyle(3013)
            mc_error_band.SetMarkerSize(0)
            mc_error_band.GetXaxis().SetRange(self.bin_min, self.bin_max)
            mc_error_band.GetXaxis().SetRangeUser(self.xmin, self.xmax)
            ROOT.SetOwnership(mc_error_band, False)
            self._objects_to_keep.append(mc_error_band)
            mc_error_band.Draw("E2 SAME")
        
        # [수정] Blind 모드가 아닐 때만 Data Draw
        if self.data_hist and not self.blind_data:
            self.data_hist.GetXaxis().SetRange(self.bin_min, self.bin_max)
            self.data_hist.GetXaxis().SetRangeUser(self.xmin, self.xmax)
            if not first_drawn:
                self.data_hist.SetMinimum(self.ymin)
                self.data_hist.SetMaximum(self.ymax)
                self.data_hist.GetYaxis().SetTitle(self.config["yTitle"])
                self.data_hist.GetYaxis().SetTitleSize(0.05)
                self.data_hist.GetYaxis().SetTitleOffset(1.2)
                self.data_hist.GetYaxis().SetLabelSize(0.04)
                self.data_hist.GetXaxis().SetLabelSize(0)
                self.data_hist.Draw("E1")
            else:
                self.data_hist.Draw("E1 SAME")
        
        if self.draw_tb_lines:
            for name, hist in self.tb_hists.items():
                hist.GetXaxis().SetRange(self.bin_min, self.bin_max)
                hist.GetXaxis().SetRangeUser(self.xmin, self.xmax)
                hist.Draw("HIST SAME")
        
        if hasattr(self, 'all_samples') and hasattr(self, 'grouped_hists'):
            for group_name, integral in self.all_samples:
                if group_name in self.grouped_hists:
                    legend_name = group_name
                    if group_name == "ST":
                        legend_name = "Single Top"
                    elif group_name == "DYJets":
                        legend_name = "Drell-Yan"
                    self.leg.AddEntry(self.grouped_hists[group_name], legend_name, "F")
        
        if self.draw_tb_lines:
            for name, hist in self.tb_hists.items():
                if name.startswith("TB") and "WR" in name:
                    wr_pos = name.find("WR")
                    legend_name = name[wr_pos:] if wr_pos != -1 else name
                else:
                    legend_name = name
                self.leg.AddEntry(hist, legend_name, "L")
        
        # [수정] Blind 모드가 아닐 때만 Legend에 Data 추가
        if self.data_hist and not self.blind_data:
            self.leg.AddEntry(self.data_hist, "Data", "PE")
        
        self.leg.Draw()
        
        latex = ROOT.TLatex()
        latex.SetNDC()
        latex.SetTextSize(0.045)
        latex.SetTextFont(62)
        latex.DrawLatex(0.12, 0.93, "CMS")
        
        latex.SetTextSize(0.035)
        latex.SetTextFont(52)
        latex.DrawLatex(0.20, 0.93, "Preliminary")
        
        latex.SetTextFont(42)
        latex.DrawLatex(0.73, 0.93, "17.794 fb^{-1} (13.6 TeV)")
        latex.DrawLatex(0.16, 0.85 ,"#mu#mu")
        latex.DrawLatex(0.16, 0.8 ,"Boosted SR")
        ROOT.SetOwnership(latex, False)
        self._objects_to_keep.append(latex)
        
        self.pad1.RedrawAxis()
        
        # =====================================
        # Lower pad - Ratio plot
        # =====================================
        self.pad2.cd()
        
        if self.background_stack and self.background_stack.GetHistogram():
            ratio_dummy = self.background_stack.GetHistogram().Clone(f"ratio_dummy_{id(self)}")
            ratio_dummy.Reset()
        elif self.total_background:
            ratio_dummy = self.total_background.Clone(f"ratio_dummy_{id(self)}")
            ratio_dummy.Reset()
        else:
            ratio_dummy = ROOT.TH1F(f"ratio_dummy_{id(self)}", "", 100, self.xmin, self.xmax)
        
        ratio_dummy.GetXaxis().SetRange(self.bin_min, self.bin_max)
        ratio_dummy.GetXaxis().SetRangeUser(self.xmin, self.xmax)
        
        ratio_dummy.SetMinimum(self.ratio_ymin)
        ratio_dummy.SetMaximum(self.ratio_ymax)
        ratio_dummy.GetXaxis().SetTitle(self.config["xTitle"])
        ratio_dummy.GetYaxis().SetTitle("Data / MC")
        
        ratio_dummy.GetXaxis().SetTitleSize(0.12)
        ratio_dummy.GetXaxis().SetTitleOffset(1.0)
        ratio_dummy.GetXaxis().SetLabelSize(0.10)
        ratio_dummy.GetYaxis().SetTitleSize(0.08)
        ratio_dummy.GetYaxis().SetTitleOffset(0.5)
        ratio_dummy.GetYaxis().SetLabelSize(0.08)
        ratio_dummy.GetYaxis().SetNdivisions(505)
        ratio_dummy.GetXaxis().SetTickLength(0.03)
        ratio_dummy.SetDirectory(0)
        
        ROOT.SetOwnership(ratio_dummy, False)
        self._objects_to_keep.append(ratio_dummy)
        ratio_dummy.Draw()
        
        # [수정] Blind 모드일 경우: 텍스트 출력
        if self.blind_data:
            blind_text = ROOT.TLatex(0.5, 0.7, "Blinded")
            blind_text.SetNDC()
            blind_text.SetTextAlign(22) # Center align
            blind_text.SetTextSize(0.15)
            blind_text.SetTextColor(ROOT.kBlack)
            blind_text.Draw()
            self._objects_to_keep.append(blind_text)
            
        # [수정] Blind 모드가 아닐 경우: 실제 Ratio Plot 그리기
        else:
            if self.total_background:
                mc_ratio_error = self.total_background.Clone(f"mc_ratio_error_{id(self)}")
                mc_ratio_error.Reset()
                mc_ratio_error.GetXaxis().SetRange(self.bin_min, self.bin_max)
                mc_ratio_error.GetXaxis().SetRangeUser(self.xmin, self.xmax)
                
                for i in range(1, mc_ratio_error.GetNbinsX() + 1):
                    mc_content = self.total_background.GetBinContent(i)
                    mc_error = self.total_background.GetBinError(i)
                    if mc_content > 0:
                        ratio_error = mc_error / mc_content
                        mc_ratio_error.SetBinContent(i, 1.0)
                        mc_ratio_error.SetBinError(i, ratio_error)
                    else:
                        mc_ratio_error.SetBinContent(i, 1.0)
                        mc_ratio_error.SetBinError(i, 0.0)
                
                mc_ratio_error.SetFillColor(ROOT.kBlack)
                mc_ratio_error.SetFillStyle(3013)
                mc_ratio_error.SetMarkerSize(0)
                mc_ratio_error.SetDirectory(0)
                ROOT.SetOwnership(mc_ratio_error, False)
                self._objects_to_keep.append(mc_ratio_error)
                mc_ratio_error.Draw("E2 SAME")
            
            line = ROOT.TLine(self.xmin, 1., self.xmax, 1.)
            line.SetLineStyle(2)
            line.SetLineColor(ROOT.kBlack)
            line.SetLineWidth(1)
            ROOT.SetOwnership(line, False)
            self._objects_to_keep.append(line)
            line.Draw()
            
            if self.ratio:
                self.ratio.GetXaxis().SetRange(self.bin_min, self.bin_max)
                self.ratio.GetXaxis().SetRangeUser(self.xmin, self.xmax)
                self.ratio.SetLineColor(DATA_COLOR)
                self.ratio.SetLineWidth(2)
                self.ratio.SetMarkerStyle(20)
                self.ratio.SetMarkerSize(0.8)
                self.ratio.SetMarkerColor(DATA_COLOR)
                self.ratio.Draw("E1 SAME")
            
        self.pad2.RedrawAxis()
        self.canv.Update()
    
    def save_as(self, filename):
        self.canv.SaveAs(filename)
    
    def close(self):
        if hasattr(self, 'canv') and self.canv:
            self.canv.Close()
        self._objects_to_keep.clear()
        gc.collect()
    
    def __del__(self):
        try:
            self.close()
        except:
            pass

def rebin_to_common_binning(hist, xmin=0, xmax=2000, nbins=200, custom_bins=None):
    """Rebin histogram to common binning to avoid merge issues"""
    if not hist:
        return None
    
    if custom_bins is not None:
        bin_edges = np.array(custom_bins, dtype=float)
        if len(bin_edges) >= 2 and bin_edges[-1] > xmax * 2:
            valid_bins = bin_edges[bin_edges <= xmax]
            if len(valid_bins) == 0 or valid_bins[-1] < xmax:
                bin_edges = np.append(valid_bins, xmax)
            else:
                bin_edges = valid_bins
        
        new_hist = ROOT.TH1F(f"{hist.GetName()}_rebinned", hist.GetTitle(), len(bin_edges)-1, bin_edges)
        nbins = len(bin_edges) - 1
    else:
        new_hist = ROOT.TH1F(f"{hist.GetName()}_rebinned", hist.GetTitle(), nbins, xmin, xmax)
    
    new_hist.SetDirectory(0)
    
    for i in range(1, nbins + 1):
        bin_low = new_hist.GetBinLowEdge(i)
        bin_high = new_hist.GetBinLowEdge(i + 1)
        
        content = hist.Integral(hist.FindBin(bin_low), hist.FindBin(bin_high))
        
        error_sq = 0
        for j in range(hist.FindBin(bin_low), hist.FindBin(bin_high) + 1):
            if j >= 1 and j <= hist.GetNbinsX():
                error_sq += hist.GetBinError(j) ** 2
        
        new_hist.SetBinContent(i, content)
        new_hist.SetBinError(i, error_sq ** 0.5)
    
    new_hist.SetLineColor(hist.GetLineColor())
    new_hist.SetLineWidth(hist.GetLineWidth())
    new_hist.SetFillColor(hist.GetFillColor())
    new_hist.SetFillStyle(hist.GetFillStyle())
    new_hist.SetMarkerColor(hist.GetMarkerColor())
    new_hist.SetMarkerStyle(hist.GetMarkerStyle())
    new_hist.SetMarkerSize(hist.GetMarkerSize())
    
    ROOT.SetOwnership(new_hist, False)
    return new_hist

def load_histogram(file_path, hist_name, systematic="Central", silent=False):
    """Load histogram from ROOT file"""
    root_file = ROOT.TFile.Open(file_path)
    if not root_file or root_file.IsZombie():
        if not silent:
            print(f"Error: Cannot open file {file_path}")
        return None
    
    directory = root_file.Get(systematic)
    if not directory:
        if not silent:
            print(f"Error: Cannot find directory {systematic} in {file_path}")
        root_file.Close()
        return None
    
    hist = directory.Get(hist_name)
    if not hist:
        if not silent:
            print(f"Error: Cannot find histogram {hist_name} in {file_path}:{systematic}")
        root_file.Close()
        return None
    
    hist_clone = hist.Clone(f"{os.path.basename(file_path)}_{hist_name}")
    hist_clone.SetDirectory(0)
    ROOT.SetOwnership(hist_clone, False)
    root_file.Close()
    
    if ("jetnum" in hist_name.lower() or "num" in hist_name.lower() or 
        hist_clone.GetNbinsX() <= 20):
        return hist_clone
    else:
        custom_bins = getattr(load_histogram, 'custom_bins', None)
        xmax_for_rebin = getattr(load_histogram, 'xmax_for_rebin', 2000)
        rebinned_hist = rebin_to_common_binning(hist_clone, xmin=0, xmax=xmax_for_rebin, nbins=200, custom_bins=custom_bins)
        return rebinned_hist if rebinned_hist else hist_clone

def combine_Muon_Fata(data_dir, hist_name, systematic="Central"):
    """Load and combine Muon C, D and SingleMuon data histograms"""
    print("Loading data from DATA files")
    
    root_files = glob.glob(os.path.join(data_dir, "*.root"))
    
    data_hists = []
    total_events = 0
    
    for file_path in root_files:
        filename = os.path.basename(file_path)
        
        if (filename.startswith("Muon0") or
            filename.startswith("Muon1")):
            
            hist = load_histogram(file_path, hist_name, systematic)
            if hist:
                events = hist.Integral()
                data_hists.append((filename, hist, events))
                print(f"Loaded {filename}: {events:.1f} events")
                total_events += events
    
    if not data_hists:
        print("Warning: No data histograms found (DATA)")
        return None
    
    combined_hist = data_hists[0][1].Clone("Combined_Muon_Fata")
    combined_hist.SetDirectory(0)
    
    for i in range(1, len(data_hists)):
        combined_hist.Add(data_hists[i][1])
    
    ROOT.SetOwnership(combined_hist, False)
    print(f"Combined Data Total (DATA): {total_events:.1f} events")
    print(f"Total data files combined: {len(data_hists)}")
    
    return combined_hist

def load_signal_histograms(data_dir, hist_name, systematic="Central"):
    """Load signal histogram from TTLJ file"""
    signal_hists = {}
    ttlj_path = os.path.join(data_dir, "TTLJ_powheg.root")
    
    if os.path.exists(ttlj_path):
        sig_hist = load_histogram(ttlj_path, hist_name, systematic)
        if sig_hist:
            signal_hists["TTLJ_powheg"] = sig_hist
            print(f"Loaded TTLJ Signal: {sig_hist.Integral():.1f} events")
    
    return signal_hists

def load_background_histograms(data_dir, hist_name, systematic="Central", draw_tb_lines=True):
    """Load background histograms from all ROOT files except data files"""
    background_hists = {}
    
    root_files = glob.glob(os.path.join(data_dir, "*.root"))
    
    for file_path in root_files:
        filename = os.path.basename(file_path)
        sample_name = filename.replace(".root", "")
        
        if ((filename.startswith("Muon") or 
            filename.startswith("EGamma") or
            filename.startswith("TB") or
            filename.startswith("Muon_G.root"))or
            filename.startswith("Muon_") or
            filename.startswith("EGamma_") or
            filename.startswith("MuonEG_") or
            filename.startswith("SingleMuon")or
            filename.startswith("WJets.root")or
            filename.startswith("WZTo")or
            filename.startswith("ZZTo") or
            filename.startswith("DYG") or
            filename.startswith("DYJets_MG") or
            filename.startswith("DYJets10to50_MG") ):
            continue
        
        if sample_name.startswith("TB") and not draw_tb_lines:
            print(f"Skipping TB sample (TB lines disabled): {sample_name}")
            continue
        
        hist = load_histogram(file_path, hist_name, systematic, silent=True)
        if hist:
            background_hists[sample_name] = hist
            print(f"Loaded background {sample_name}: {hist.Integral():.1f} events")
    
    print(f"Total background samples loaded: {len(background_hists)}")
    return background_hists

def get_tb_samples(data_dir):
    """Get list of TB sample names from the data directory"""
    tb_samples = []
    root_files = glob.glob(os.path.join(data_dir, "*.root"))
    
    for file_path in root_files:
        filename = os.path.basename(file_path)
        sample_name = filename.replace(".root", "")
        if sample_name.startswith("TB"):
            tb_samples.append(sample_name)
    
    return sorted(tb_samples)

def format_tb_legend_name(tb_sample_name):
    """Format TB sample name for legend display (show from WR onwards)"""
    if tb_sample_name.startswith("TB") and "WR" in tb_sample_name:
        wr_pos = tb_sample_name.find("WR")
        return tb_sample_name[wr_pos:] if wr_pos != -1 else tb_sample_name
    return tb_sample_name

def load_tb_sample_directly(data_dir, tb_sample_name, hist_name, systematic="Central"):
    """Load a specific TB sample histogram directly"""
    file_path = os.path.join(data_dir, f"{tb_sample_name}.root")
    if os.path.exists(file_path):
        hist = load_histogram(file_path, hist_name, systematic, silent=True)
        if hist:
            print(f"Loaded TB sample directly: {tb_sample_name}: {hist.Integral():.1f} events")
            return hist
    print(f"TB sample file not found: {file_path}")
    return None

def plot_signal_background_comparison(data_dir, hist_name, config, output_name="signal_background_comparison", systematic="Central", draw_tb_lines=True, target_tb_sample=None, normalize_by_bin_size=False, apply_scale_factors_flag=False, sf_file_path=None, blind_data=False):
    """Create Signal+Background vs Data comparison plot"""
    
    print(f"Creating Signal+Background vs Data comparison for histogram: {hist_name}")
    print(f"Data directory: {data_dir}")
    print(f"TB lines drawing: {'Enabled' if draw_tb_lines else 'Disabled'}")
    print(f"Blinded Data (SR Mode): {'YES' if blind_data else 'NO'}")
    
    scale_factors = {}
    if apply_scale_factors_flag and sf_file_path:
        scale_factors = load_scale_factors(sf_file_path)
    
    if blind_data:
        print("Blind mode enabled: Skipping Data loading.")
        combined_data_hist = None
    else:
        combined_data_hist = combine_Muon_Fata(data_dir, hist_name, systematic)
    
    signal_hists = {}
    background_hists = load_background_histograms(data_dir, hist_name, systematic, draw_tb_lines=draw_tb_lines)
    
    if draw_tb_lines and target_tb_sample:
        tb_hist = load_tb_sample_directly(data_dir, target_tb_sample, hist_name, systematic)
        if tb_hist:
            signal_hists[target_tb_sample] = tb_hist
    
    if not combined_data_hist and not signal_hists and not background_hists:
        print("Error: No histograms could be loaded!")
        return None
    
    if "rebin" in config and config["rebin"] > 1:
        if combined_data_hist:
            combined_data_hist.Rebin(config["rebin"])
        for hist in signal_hists.values():
            hist.Rebin(config["rebin"])
        for hist in background_hists.values():
            hist.Rebin(config["rebin"])
    
    if normalize_by_bin_size:
        if combined_data_hist:
            try:
                combined_data_hist = globals()['normalize_by_bin_size'](combined_data_hist)
            except Exception as e:
                print(f"Error normalizing data histogram: {e}")
        
        for name, hist in signal_hists.items():
            try:
                signal_hists[name] = globals()['normalize_by_bin_size'](hist)
            except Exception as e:
                print(f"Error normalizing signal histogram {name}: {e}")
        
        for name, hist in background_hists.items():
            try:
                background_hists[name] = globals()['normalize_by_bin_size'](hist)
            except Exception as e:
                print(f"Error normalizing background histogram {name}: {e}")
    
    if apply_scale_factors_flag and scale_factors:
        for name, hist in signal_hists.items():
            try:
                signal_hists[name] = apply_scale_factors(hist, hist_name, scale_factors)
            except Exception as e:
                print(f"Error applying scale factors to signal histogram {name}: {e}")
        
        for name, hist in background_hists.items():
            try:
                background_hists[name] = apply_scale_factors(hist, hist_name, scale_factors)
            except Exception as e:
                print(f"Error applying scale factors to background histogram {name}: {e}")
    
    canvas = None
    try:
        canvas = SignalBackgroundCanvas(combined_data_hist, signal_hists, background_hists, config, draw_tb_lines=draw_tb_lines, target_tb_sample=target_tb_sample, normalize_by_bin_size=normalize_by_bin_size, blind_data=blind_data)
        canvas.data_dir = data_dir
        canvas.hist_name = hist_name
        canvas.systematic = systematic
        canvas.draw()
        
        canvas.save_as(f"{output_name}_test.png")
        print(f"Plot saved as {output_name}.png and {output_name}.pdf")
        return canvas
        
    except Exception as e:
        print(f"Error creating plot: {e}")
        if canvas:
            canvas.close()
        return None

def parse_arguments():
    """Parse command line arguments"""
    parser = argparse.ArgumentParser(description="TTbar Stacked MC vs Data Comparison Plotter")
    
    parser.add_argument("--data-dir", type=str, default=data_directory,
                      help=f"Data directory path (default: {data_directory})")
    parser.add_argument("--hist-name", type=str, default="Topjetnum",
                      help="Histogram name to plot (default: Topjetnum)")
    parser.add_argument("--output-name", type=str, default="Topjetnum_Central_2022EE",
                      help="Output file name prefix (default: Topjetnum_Central_2022EE)")
    
    parser.add_argument("--no-tb-lines", action="store_true",
                      help="Disable TB sample line drawing (include TB in stack instead)")
    
    parser.add_argument("--normalize-by-bin-size", action="store_true",
                      help="Normalize histogram contents by bin size (divide by bin width)")
    
    parser.add_argument("--apply-scale-factors", action="store_true",
                      help="Apply bin-wise scale factors to MC histograms")
    parser.add_argument("--sf-file", type=str, default="SF.csv",
                      help="Path to scale factor CSV file (default: SF.csv)")
    
    parser.add_argument("--custom-bins", type=str, default=None,
                      help="Custom bin edges as comma-separated values (e.g., '0,120,300')")
    
    parser.add_argument("--x-range", type=str, default="0,5",
                      help="X-axis range as 'min,max' (default: '0,5')")
    parser.add_argument("--y-range", type=str, default="0.5,2.0",
                      help="Ratio plot Y-axis range as 'min,max' (default: '0.5,2.0')")
    parser.add_argument("--x-title", type=str, default="Topjet num ",
                      help="X-axis title (default: 'Topjet num ')")
    parser.add_argument("--y-title", type=str, default="Events / bin",
                      help="Y-axis title (default: 'Events / bin')")
    parser.add_argument("--y-title-normalized", type=str, default="Events / GeV",
                      help="Y-axis title when normalized by bin size (default: 'Events / GeV')")
    parser.add_argument("--logy", action="store_true",
                      help="Use logarithmic Y-axis")
    parser.add_argument("--rebin", type=int, default=1,
                      help="Rebin factor (default: 1)")
    parser.add_argument("--ymax", type=float, default=None,
                      help="Y-axis maximum (auto if not specified)")
    parser.add_argument("--ymin", type=float, default=None,
                      help="Y-axis minimum (auto if not specified)")
    
    parser.add_argument("--sr", action="store_true", 
                      help="Signal Region mode (Blind Data)")
    
    return parser.parse_args()

def main():
    """Main function to create TTbar stacked MC vs Data plots"""
    
    print("TTbar Stacked MC vs Data Comparison Plotter")
    print("=" * 50)
    
    args = parse_arguments()
    
    if not os.path.exists(args.data_dir):
        print(f"Error: Data directory {args.data_dir} does not exist!")
        return
    
    custom_bins = None
    if args.custom_bins:
        try:
            custom_bins = [float(x.strip()) for x in args.custom_bins.split(',')]
            print(f"Using custom bins: {custom_bins}")
            load_histogram.custom_bins = custom_bins
        except ValueError:
            print(f"Error: Invalid custom bins format '{args.custom_bins}'. Use comma-separated numbers.")
            return
    
    try:
        x_range = [float(x.strip()) for x in args.x_range.split(',')]
        y_range = [float(x.strip()) for x in args.y_range.split(',')]
        load_histogram.xmax_for_rebin = x_range[-1]
    except ValueError:
        print(f"Error: Invalid range format. Use 'min,max' format.")
        return
    
    normalize_by_bin_size = args.normalize_by_bin_size
    
    apply_scale_factors_flag = args.apply_scale_factors
    sf_file_path = args.sf_file
    
    if not os.path.isabs(sf_file_path):
        sf_file_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), sf_file_path)
    
    y_title = args.y_title_normalized if normalize_by_bin_size else args.y_title
    plot_config = {
        "xRange": x_range,
        "yRange": y_range,
        "xTitle": args.x_title,
        "yTitle": y_title,
        "logy": args.logy,
        "rebin": args.rebin,
    }
    
    if args.ymax is not None:
        plot_config["ymax"] = args.ymax
    if args.ymin is not None:
        plot_config["ymin"] = args.ymin
    elif args.logy:
        plot_config["ymin"] = 1e-2
    
    draw_tb_lines = not args.no_tb_lines
    blind_data = args.sr
    
    print(f"Configuration:")
    print(f"  Data directory: {args.data_dir}")
    print(f"  Histogram: {args.hist_name}")
    print(f"  Output: {args.output_name}")
    print(f"  TB lines: {'Enabled' if draw_tb_lines else 'Disabled'}")
    print(f"  Normalize by bin size: {'Enabled' if normalize_by_bin_size else 'Disabled'}")
    print(f"  Apply scale factors: {'Enabled' if apply_scale_factors_flag else 'Disabled'}")
    if apply_scale_factors_flag:
        print(f"  Scale factor file: {sf_file_path}")
    print(f"  Custom bins: {custom_bins if custom_bins else 'None'}")
    print(f"  X-range: {x_range}")
    print(f"  Y-range: {y_range}")
    print(f"  Log Y: {args.logy}")
    print(f"  Blind Data (SR): {blind_data}")
    
    try:
        if draw_tb_lines:
            tb_samples = get_tb_samples(args.data_dir)
            
            if tb_samples:
                print(f"Found {len(tb_samples)} TB samples: {tb_samples}")
                
                canvases = []
                for tb_sample in tb_samples:
                    print(f"\n{'='*50}")
                    print(f"Creating plot for TB sample: {tb_sample}")
                    print(f"{'='*50}")
                    
                    tb_output_name = f"{args.output_name}_{tb_sample}"
                    
                    canvas = plot_signal_background_comparison(
                        args.data_dir, 
                        args.hist_name, 
                        plot_config, 
                        tb_output_name,
                        draw_tb_lines=draw_tb_lines,
                        target_tb_sample=tb_sample,
                        normalize_by_bin_size=normalize_by_bin_size,
                        apply_scale_factors_flag=apply_scale_factors_flag,
                        sf_file_path=sf_file_path,
                        blind_data=blind_data
                    )
                    
                    if canvas:
                        canvases.append((tb_sample, canvas))
                        print(f"Plot for {tb_sample} created successfully!")
                    else:
                        print(f"Plot creation failed for {tb_sample}!")
                
                if canvases:
                    print(f"\n{'='*50}")
                    print(f"Successfully created {len(canvases)} plots:")
                    for tb_sample, canvas in canvases:
                        print(f"  - {args.output_name}_{tb_sample}.png/pdf")
                    
                    for tb_sample, canvas in canvases:
                        canvas.close()
                else:
                    print("All plot creations failed!")
            else:
                print("No TB samples found in the data directory.")
                print("Creating plot without TB lines...")
                
                canvas = plot_signal_background_comparison(
                    args.data_dir, 
                    args.hist_name, 
                    plot_config, 
                    args.output_name,
                    draw_tb_lines=False,
                    normalize_by_bin_size=normalize_by_bin_size,
                    apply_scale_factors_flag=apply_scale_factors_flag,
                    sf_file_path=sf_file_path,
                    blind_data=blind_data
                )
                
                if canvas:
                    print("Plot creation successful!")
                    canvas.close()
                else:
                    print("Plot creation failed!")
        else:
            canvas = plot_signal_background_comparison(
                args.data_dir, 
                args.hist_name, 
                plot_config, 
                args.output_name,
                draw_tb_lines=draw_tb_lines,
                normalize_by_bin_size=normalize_by_bin_size,
                apply_scale_factors_flag=apply_scale_factors_flag,
                sf_file_path=sf_file_path,
                blind_data=blind_data
            )
            
            if canvas:
                print("Plot creation successful!")
                canvas.close()
            else:
                print("Plot creation failed!")
            
    except Exception as e:
        print(f"Error in main: {e}")
        import traceback
        traceback.print_exc()

if __name__ == "__main__":
    main()