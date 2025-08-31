import ROOT
import cmsstyle as CMS
import os
import glob
from array import array
import gc

# Prevent ROOT from owning Python objects
ROOT.SetOwnership(ROOT.gROOT, False)

SIGNAL_COLOR = ROOT.TColor.GetColor("#e42536")  # Red for signal
BACKGROUND_COLORS = [
    ROOT.TColor.GetColor("#5790fc"),  # Blue
    ROOT.TColor.GetColor("#f89c20"),  # Orange  
    ROOT.TColor.GetColor("#964a8b"),  # Purple
    ROOT.TColor.GetColor("#9c9ca1")   # Gray for "Others"
]

class SignalBackgroundCanvas():
    def __init__(self, signal_hists, background_hists, config):
        super().__init__()
        
        self.signal_hists = signal_hists
        self.background_hists = background_hists
        self.config = config
        
        # Keep references to prevent garbage collection
        self._objects_to_keep = []
        
        # Initialize variables (stack will be built after rebinning)
        self.background_stack = None
        self.total_background = None
        
        # Sort backgrounds by integral (highest first)
        bg_integrals = [(name, hist.Integral()) for name, hist in background_hists.items()]
        bg_integrals.sort(key=lambda x: x[1], reverse=True)
        
        # Separate top 2 and others
        self.top_backgrounds = []
        self.other_backgrounds = []
        
        for i, (name, integral) in enumerate(bg_integrals):
            if i < 2:  # Top 2 backgrounds
                self.top_backgrounds.append((name, background_hists[name]))
            else:  # All other backgrounds
                self.other_backgrounds.append((name, background_hists[name]))
        
        # Apply binning FIRST before any processing
        self._handle_binning()
        
        # Create "Others" combined histogram if there are more than 2 backgrounds
        self.others_hist = None
        if len(self.other_backgrounds) > 0:
            for i, (name, hist) in enumerate(self.other_backgrounds):
                if self.others_hist is None:
                    self.others_hist = hist.Clone("Others")
                    self.others_hist.SetDirectory(0)  # Detach from ROOT file system
                    self._objects_to_keep.append(self.others_hist)
                else:
                    self.others_hist.Add(hist)
        
        # Build background stack after rebinning
        self._build_background_stack()
        
        # Process signal histograms
        self.total_signal = None
        for name, hist in self.signal_hists.items():
            hist.Sumw2()
            hist.SetLineColor(SIGNAL_COLOR)
            hist.SetLineWidth(3)
            hist.SetFillStyle(0)  # No fill for signal
            hist.SetStats(0)  # Disable statistics box for this histogram
            
            if self.total_signal is None:
                self.total_signal = hist.Clone("total_signal")
                self.total_signal.SetDirectory(0)
                self.total_signal.SetStats(0)  # Disable stats for cloned histogram
                self._objects_to_keep.append(self.total_signal)
            else:
                self.total_signal.Add(hist)
        
        # Create ratio histogram
        if self.total_background is not None and self.total_signal is not None:
            self.ratio = self.total_signal.Clone("signal_ratio")
            self.ratio.SetDirectory(0)
            self.ratio.SetStats(0)  # Disable statistics box for ratio
            self.ratio.Divide(self.total_background)
            self._objects_to_keep.append(self.ratio)
        
        # Set up canvas
        self._setup_canvas()
        
    def _handle_binning(self):
        """Handle histogram rebinning based on config"""
        if "rebin" in self.config.keys():
            rebin_factor = self.config["rebin"]
            print(f"Attempting to rebin by factor: {rebin_factor}")
            
            # Check if rebinning is possible by examining the first histogram
            test_hist = None
            if self.signal_hists:
                test_hist = list(self.signal_hists.values())[0]
            elif self.top_backgrounds:
                test_hist = self.top_backgrounds[0][1]
            
            if test_hist:
                original_bins = test_hist.GetNbinsX()
                print(f"Total number of bins: {original_bins}")  # 총 빈수 명시적 출력
                print(f"Original bin range: [{test_hist.GetXaxis().GetXmin():.1f}, {test_hist.GetXaxis().GetXmax():.1f}]")
                print(f"Original bin width: {test_hist.GetBinWidth(1):.2f}")
                
                # Check if rebinning is mathematically possible
                if original_bins % rebin_factor != 0:
                    print(f"WARNING: Cannot rebin {original_bins} bins by factor {rebin_factor}")
                    print(f"Rebin factor must be a divisor of {original_bins}")
                    print(f"Possible rebin factors: {[i for i in range(1, original_bins+1) if original_bins % i == 0]}")
                    print("Skipping rebinning...")
                    return
                else:
                    new_bins = original_bins // rebin_factor
                    new_bin_width = test_hist.GetBinWidth(1) * rebin_factor
                    print(f"After rebinning: {new_bins} bins (from {original_bins}), bin width: {new_bin_width:.2f}")
            
            # Rebin signal histograms
            for name, hist in self.signal_hists.items():
                try:
                    hist.Rebin(rebin_factor)
                    print(f"Successfully rebinned signal: {name}")
                except Exception as e:
                    print(f"Error rebinning signal {name}: {e}")
            
            # Rebin top 2 background histograms
            for name, hist in self.top_backgrounds:
                try:
                    hist.Rebin(rebin_factor)
                    print(f"Successfully rebinned background: {name}")
                except Exception as e:
                    print(f"Error rebinning background {name}: {e}")
            
            # Rebin other background histograms
            for name, hist in self.other_backgrounds:
                try:
                    hist.Rebin(rebin_factor)
                    print(f"Successfully rebinned other background: {name}")
                except Exception as e:
                    print(f"Error rebinning other background {name}: {e}")
                
        elif len(self.config["xRange"]) > 2:
            bins = array('d', self.config["xRange"])
            print(f"Using variable binning with {len(bins)-1} bins")
            print(f"Bin edges: {list(bins)}")
            
            # Rebin signal histograms
            for name, hist in self.signal_hists.items():
                try:
                    new_hist = hist.Rebin(len(bins)-1, f"{name}_rebin", bins)
                    new_hist.SetDirectory(0)
                    self.signal_hists[name] = new_hist
                    self._objects_to_keep.append(new_hist)
                    print(f"Successfully variable-rebinned signal: {name}")
                except Exception as e:
                    print(f"Error variable-rebinning signal {name}: {e}")
            
            # Rebin top 2 background histograms
            for i, (name, hist) in enumerate(self.top_backgrounds):
                try:
                    new_hist = hist.Rebin(len(bins)-1, f"{name}_rebin", bins)
                    new_hist.SetDirectory(0)
                    self.top_backgrounds[i] = (name, new_hist)
                    self._objects_to_keep.append(new_hist)
                    print(f"Successfully variable-rebinned background: {name}")
                except Exception as e:
                    print(f"Error variable-rebinning background {name}: {e}")
            
            # Rebin other background histograms
            for i, (name, hist) in enumerate(