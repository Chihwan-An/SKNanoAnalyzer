#!/usr/bin/env python3
"""
Systematic Uncertainty Breakdown Calculator
Shows individual contribution of each systematic source
"""
import ROOT
import math
import numpy as np
import os
import sys
import argparse

# --- Configuration ---
DEFAULT_DATA_PATH = "/gv0/Users/achihwan/SKNanoOutput/Reproduce20_002_copy/2017/"

SAMPLE_MAP = {
    "DYJets": "DYJets",
    "TTLL": "TT",
    "TTLJ": "Nonprompt",
    "ST": "TT",
    "WJets": "Nonprompt",
    "ST_tch": "Nonprompt",
    "ST_sch": "Nonprompt"
}

EXCLUDE_SAMPLES = ["DYJets_MG", "Skim"]
DATA_FILES_EE = ["SingleElectron"]
DATA_FILES_MM = ["Muon"]

SYST_LIST = ["Pileup", "ElectronID", "ElectronReco", "ElectronTrig",
             "MuonID", "MuonReco", "MuonTrig", "MuonIso", "JER", "JES"]

def get_hist_from_file(file_path, sys_dir, hist_name, custom_bins=None, rebin_factor=1):
    """Load histogram from file"""
    f = ROOT.TFile.Open(file_path)
    if not f or f.IsZombie():
        return None

    h_orig = f.Get(f"{sys_dir}/{hist_name}")
    if not h_orig:
        f.Close()
        return None

    h_name = f"h_{sys_dir}_{os.path.basename(file_path).split('.')[0]}_{ROOT.TUUID().AsString()[:8]}"
    h = h_orig.Clone(h_name)
    h.SetDirectory(0)

    if custom_bins is not None:
        bin_edges = np.array(custom_bins, dtype=float)
        h_new = h.Rebin(len(bin_edges)-1, h.GetName() + "_re", bin_edges)
        h = h_new
        h.SetDirectory(0)
    elif rebin_factor > 1:
        h.Rebin(rebin_factor)

    f.Close()
    return h

def calculate_syst_breakdown(data_path, hist_name, channel="EE", custom_bins=None, rebin_factor=1):
    """Calculate per-systematic breakdown"""
    ROOT.gROOT.SetBatch(True)

    DATA_FILES = DATA_FILES_EE if channel == "EE" else DATA_FILES_MM
    EXCLUDE = EXCLUDE_SAMPLES + (["Muon"] if channel == "EE" else ["SingleElectron"])

    all_files = sorted([f for f in os.listdir(data_path) if f.endswith(".root")])

    # Load central (nominal) MC
    h_total_mc = None
    for fname in all_files:
        if any(ex in fname for ex in EXCLUDE): continue
        if any(df in fname for df in DATA_FILES): continue  # Skip data

        path = os.path.join(data_path, fname)
        h = get_hist_from_file(path, "Central", hist_name, custom_bins, rebin_factor)

        if h:
            if h_total_mc is None:
                h_total_mc = h.Clone("h_total_mc")
                h_total_mc.SetDirectory(0)
            else:
                h_total_mc.Add(h)

    if h_total_mc is None:
        print(f"Error: No MC histograms found for '{hist_name}'")
        return None

    n_bins = h_total_mc.GetNbinsX()

    # Calculate per-systematic contributions
    syst_contributions = {}

    for syst in SYST_LIST:
        h_mc_up, h_mc_dn = None, None

        for fname in all_files:
            if any(ex in fname for ex in EXCLUDE): continue
            if any(df in fname for df in DATA_FILES): continue

            path = os.path.join(data_path, fname)
            u = get_hist_from_file(path, f"{syst}_Up", hist_name, custom_bins, rebin_factor)
            d = get_hist_from_file(path, f"{syst}_Down", hist_name, custom_bins, rebin_factor)

            if u:
                if h_mc_up is None:
                    h_mc_up = u.Clone(f"h_up_{syst}")
                    h_mc_up.SetDirectory(0)
                else:
                    h_mc_up.Add(u)
            if d:
                if h_mc_dn is None:
                    h_mc_dn = d.Clone(f"h_dn_{syst}")
                    h_mc_dn.SetDirectory(0)
                else:
                    h_mc_dn.Add(d)

        if h_mc_up and h_mc_dn:
            up_pct = []
            dn_pct = []

            for i in range(1, n_bins + 1):
                nom = h_total_mc.GetBinContent(i)
                if nom > 0:
                    diff_up = h_mc_up.GetBinContent(i) - nom
                    diff_dn = h_mc_dn.GetBinContent(i) - nom
                    # Symmetric uncertainty: max of up/down deviations
                    max_dev = max(abs(diff_up), abs(diff_dn))
                    up_pct.append((max_dev / nom) * 100)
                    dn_pct.append((max_dev / nom) * 100)
                else:
                    up_pct.append(0)
                    dn_pct.append(0)

            syst_contributions[syst] = {
                'up': up_pct,
                'dn': dn_pct,
                'avg': [(u + d) / 2 for u, d in zip(up_pct, dn_pct)]
            }
        else:
            syst_contributions[syst] = {
                'up': [0] * n_bins,
                'dn': [0] * n_bins,
                'avg': [0] * n_bins
            }

    # Calculate statistical uncertainty
    stat_pct = []
    for i in range(1, n_bins + 1):
        nom = h_total_mc.GetBinContent(i)
        stat = h_total_mc.GetBinError(i)
        if nom > 0:
            stat_pct.append((stat / nom) * 100)
        else:
            stat_pct.append(0)

    return {
        'histogram': h_total_mc,
        'n_bins': n_bins,
        'stat_pct': stat_pct,
        'syst': syst_contributions
    }

def print_breakdown_table(result, hist_name):
    """Print formatted breakdown table"""
    if result is None:
        return

    h = result['histogram']
    n_bins = result['n_bins']

    print(f"\n{'='*120}")
    print(f"Systematic Uncertainty Breakdown for: {hist_name}")
    print(f"{'='*120}")

    # Header
    syst_names = list(result['syst'].keys())
    header = f"{'Bin':<4} | {'Range':<18} | {'MC Yield':<10} | {'Stat(%)':<8}"
    for s in syst_names:
        header += f" | {s[:10]:<10}"
    header += " | Total Syst(%)"
    print(header)
    print("-" * 120)

    # Per-bin breakdown
    for i in range(n_bins):
        bin_low = h.GetBinLowEdge(i + 1)
        bin_high = h.GetBinLowEdge(i + 2)
        mc_yield = h.GetBinContent(i + 1)
        stat = result['stat_pct'][i]

        row = f"{i+1:<4} | [{bin_low:>7.0f},{bin_high:>7.0f}] | {mc_yield:>10.1f} | {stat:>7.1f}%"

        total_syst_sq = 0
        for s in syst_names:
            val = result['syst'][s]['avg'][i]
            total_syst_sq += val ** 2
            row += f" | {val:>9.1f}%"

        total_syst = math.sqrt(total_syst_sq)
        row += f" | {total_syst:>12.1f}%"
        print(row)

    print("=" * 120)

    # Summary: Average contribution across all bins
    print(f"\n{'='*80}")
    print("Average Systematic Contribution (across all bins with yield > 0):")
    print(f"{'='*80}")

    valid_bins = [i for i in range(n_bins) if h.GetBinContent(i + 1) > 0]

    avg_stat = np.mean([result['stat_pct'][i] for i in valid_bins])
    print(f"{'Statistical':<20}: {avg_stat:>8.2f}%")

    syst_avg_list = []
    for s in syst_names:
        avg = np.mean([result['syst'][s]['avg'][i] for i in valid_bins])
        syst_avg_list.append((s, avg))
        print(f"{s:<20}: {avg:>8.2f}%")

    # Sort by contribution
    print(f"\n{'='*80}")
    print("Systematics Ranked by Average Contribution:")
    print(f"{'='*80}")
    syst_avg_list.sort(key=lambda x: x[1], reverse=True)
    for rank, (s, avg) in enumerate(syst_avg_list, 1):
        bar = '#' * int(avg / 2)
        print(f"{rank:>2}. {s:<15}: {avg:>8.2f}% |{bar}")

    print(f"{'='*80}\n")

def save_breakdown_tsv(result, output_file, hist_name):
    """Save breakdown to TSV file"""
    if result is None:
        return

    h = result['histogram']
    n_bins = result['n_bins']
    syst_names = list(result['syst'].keys())

    with open(output_file, 'w') as f:
        # Header
        header = ["Bin", "BinLow", "BinHigh", "MC_Yield", "Stat_pct"]
        for s in syst_names:
            header.append(f"{s}_pct")
        header.append("TotalSyst_pct")
        f.write("\t".join(header) + "\n")

        # Data rows
        for i in range(n_bins):
            row = [
                str(i + 1),
                f"{h.GetBinLowEdge(i + 1):.2f}",
                f"{h.GetBinLowEdge(i + 2):.2f}",
                f"{h.GetBinContent(i + 1):.4f}",
                f"{result['stat_pct'][i]:.4f}"
            ]

            total_syst_sq = 0
            for s in syst_names:
                val = result['syst'][s]['avg'][i]
                total_syst_sq += val ** 2
                row.append(f"{val:.4f}")

            total_syst = math.sqrt(total_syst_sq)
            row.append(f"{total_syst:.4f}")
            f.write("\t".join(row) + "\n")

    print(f">> TSV saved: {output_file}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Systematic Uncertainty Breakdown Calculator")
    parser.add_argument("--hist", type=str, required=True, help="Histogram name")
    parser.add_argument("--channel", type=str, default="EE", choices=["EE", "MM"], help="Channel (EE or MM)")
    parser.add_argument("--path", type=str, default=DEFAULT_DATA_PATH, help="Data path")
    parser.add_argument("--bins", type=str, default=None, help="Custom bins (comma-separated)")
    parser.add_argument("--rebin", type=int, default=1, help="Rebin factor")
    parser.add_argument("--output", type=str, default=None, help="Output TSV file")
    args = parser.parse_args()

    custom_bins = [float(x) for x in args.bins.split(",")] if args.bins else None

    result = calculate_syst_breakdown(
        args.path,
        args.hist,
        args.channel,
        custom_bins,
        args.rebin
    )

    print_breakdown_table(result, args.hist)

    if args.output:
        save_breakdown_tsv(result, args.output, args.hist)
    else:
        save_breakdown_tsv(result, f"syst_breakdown_{args.hist}_{args.channel}.tsv", args.hist)
