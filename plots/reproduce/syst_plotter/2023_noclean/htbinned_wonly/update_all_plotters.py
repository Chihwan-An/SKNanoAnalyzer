#!/usr/bin/env python3
"""
Script to update all EE.py and MM.py plotter files with systematic-only and stat-only error bands
"""

import os
import re
from pathlib import Path

def update_plotter_file(filepath):
    """Update a single plotter file with new error band handling"""

    with open(filepath, 'r') as f:
        content = f.read()

    original_content = content

    # Change 1: Add g_syst_only and g_ratio_syst_only to graph declarations
    pattern1 = r'(g_syst = ROOT\.TGraphAsymmErrors\(n_bins\)\n\s+g_ratio_band = ROOT\.TGraphAsymmErrors\(n_bins\)\n\s+g_ratio_stat = ROOT\.TGraphAsymmErrors\(n_bins\)\n\s+_KEEPER\.extend\(\[g_syst, g_ratio_band, g_ratio_stat\]\))'
    replacement1 = '''g_syst = ROOT.TGraphAsymmErrors(n_bins)
    g_syst_only = ROOT.TGraphAsymmErrors(n_bins)
    g_ratio_band = ROOT.TGraphAsymmErrors(n_bins)
    g_ratio_syst_only = ROOT.TGraphAsymmErrors(n_bins)
    g_ratio_stat = ROOT.TGraphAsymmErrors(n_bins)
    _KEEPER.extend([g_syst, g_syst_only, g_ratio_band, g_ratio_syst_only, g_ratio_stat])'''

    content = re.sub(pattern1, replacement1, content)

    # Change 2: Add g_syst_only and g_ratio_syst_only point setting
    pattern2 = r'(\s+g_syst\.SetPoint\(i-1, x, y\); g_syst\.SetPointError\(i-1, w, w, err_dn, err_up\)\n\s+if y > 0:\n\s+g_ratio_band\.SetPoint\(i-1, x, 1\.0\); g_ratio_band\.SetPointError\(i-1, w, w, err_dn/y, err_up/y\)\n\s+g_ratio_stat\.SetPoint\(i-1, x, 1\.0\); g_ratio_stat\.SetPointError\(i-1, w, w, stat/y, stat/y\))'
    replacement2 = '''        g_syst.SetPoint(i-1, x, y); g_syst.SetPointError(i-1, w, w, err_dn, err_up)
        g_syst_only.SetPoint(i-1, x, y); g_syst_only.SetPointError(i-1, w, w, syst_dn, syst_up)
        if y > 0:
            g_ratio_band.SetPoint(i-1, x, 1.0); g_ratio_band.SetPointError(i-1, w, w, err_dn/y, err_up/y)
            g_ratio_syst_only.SetPoint(i-1, x, 1.0); g_ratio_syst_only.SetPointError(i-1, w, w, syst_dn/y, syst_up/y)
            g_ratio_stat.SetPoint(i-1, x, 1.0); g_ratio_stat.SetPointError(i-1, w, w, stat/y, stat/y)'''

    content = re.sub(pattern2, replacement2, content)

    # Change 3: Update main panel drawing and legend
    pattern3 = r'(stack\.Draw\("HIST SAME"\)\n\s+g_syst\.SetFillColorAlpha\(ROOT\.kBlack, 0\.6\); g_syst\.SetFillStyle\(3013\); g_syst\.Draw\("SAME E2"\)\n\s+\n\s+if h_data and not IS_BLIND:\n\s+h_data\.SetMarkerStyle\(20\); h_data\.SetMarkerSize\(1\.2\); h_data\.SetLineColor\(ROOT\.kBlack\); h_data\.Draw\("PE SAME"\)\n\s+\n\s+leg = ROOT\.TLegend\([^)]+\);[^\n]+\n\s+for g_key, v in reversed\(sorted_for_stack\): leg\.AddEntry\(group_hists\[g_key\], BACKGROUND_GROUPS\[g_key\]\[1\], "f"\)\n\s+if h_data and not IS_BLIND: leg\.AddEntry\(h_data, "Data", "pe"\)\n\s+leg\.AddEntry\(g_syst, "[^"]+", "f"\)\n\s+leg\.AddEntry\(g_ratio_stat, "[^"]+", "f"\)\n\s+leg\.Draw\(\))'
    replacement3 = '''stack.Draw("HIST SAME")
    g_syst.SetFillColorAlpha(ROOT.kBlack, 0.6); g_syst.SetFillStyle(3013); g_syst.Draw("SAME E2")
    g_syst_only.SetFillColorAlpha(ROOT.kBlue, 0.35); g_syst_only.SetFillStyle(3254); g_syst_only.Draw("SAME E2")

    if h_data and not IS_BLIND:
        h_data.SetMarkerStyle(20); h_data.SetMarkerSize(1.2); h_data.SetLineColor(ROOT.kBlack); h_data.Draw("PE SAME")

    leg = ROOT.TLegend(0.55, 0.50, 0.92, 0.88); leg.SetBorderSize(0); leg.SetFillStyle(0); leg.SetTextSize(0.035)
    for g_key, v in reversed(sorted_for_stack): leg.AddEntry(group_hists[g_key], BACKGROUND_GROUPS[g_key][1], "f")
    if h_data and not IS_BLIND: leg.AddEntry(h_data, "Data", "pe")
    leg.AddEntry(g_syst, "Stat #oplus Syst Unc.", "f")
    leg.AddEntry(g_syst_only, "Syst. Unc.", "f")
    leg.AddEntry(g_ratio_stat, "Stat. Unc.", "f")
    leg.Draw()'''

    content = re.sub(pattern3, replacement3, content, flags=re.MULTILINE)

    # Change 4: Update ratio panel drawing
    pattern4 = r'(else:\n\s+g_ratio_band\.SetFillColor\(ROOT\.kGray\+1\); g_ratio_band\.SetFillStyle\(3013\); g_ratio_band\.Draw\("SAME E2"\)\n\s+g_ratio_stat\.SetFillColor\(ROOT\.kGray\+3\); g_ratio_stat\.SetFillStyle\(3005\); g_ratio_stat\.Draw\("SAME E2"\))'
    replacement4 = '''else:
        g_ratio_band.SetFillColor(ROOT.kGray+1); g_ratio_band.SetFillStyle(3013); g_ratio_band.Draw("SAME E2")
        g_ratio_syst_only.SetFillColorAlpha(ROOT.kBlue, 0.35); g_ratio_syst_only.SetFillStyle(3254); g_ratio_syst_only.Draw("SAME E2")
        g_ratio_stat.SetFillColor(ROOT.kOrange); g_ratio_stat.SetFillStyle(3005); g_ratio_stat.Draw("SAME E2")'''

    content = re.sub(pattern4, replacement4, content)

    # Check if any changes were made
    if content != original_content:
        with open(filepath, 'w') as f:
            f.write(content)
        return True
    else:
        return False

def main():
    """Find and update all EE.py and MM.py files"""
    base_dir = Path(__file__).parent

    # Find all EE.py and MM.py files (excluding backup files)
    plotter_files = []
    for pattern in ['**/EE.py', '**/MM.py']:
        plotter_files.extend(base_dir.glob(pattern))

    # Filter out backup files
    plotter_files = [f for f in plotter_files if 'backup' not in f.name.lower()]

    print(f"Found {len(plotter_files)} plotter files to update:")
    for f in plotter_files:
        print(f"  - {f.relative_to(base_dir)}")

    print("\nUpdating files...")
    updated = 0
    for filepath in plotter_files:
        if update_plotter_file(filepath):
            print(f"  ✓ Updated: {filepath.relative_to(base_dir)}")
            updated += 1
        else:
            print(f"  ⚠ No changes: {filepath.relative_to(base_dir)}")

    print(f"\n✓ Updated {updated}/{len(plotter_files)} files successfully")

if __name__ == "__main__":
    main()
