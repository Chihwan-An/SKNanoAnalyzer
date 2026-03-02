#!/usr/bin/env python3
"""
Update all plotter files to:
- Remove systematic-only error bands
- Keep stat+syst in both panels
- Keep stat-only in ratio panel only
- Use yellow color for stat-only for better contrast
"""

import re
from pathlib import Path

def update_file(filepath):
    """Update a single plotter file"""
    with open(filepath, 'r') as f:
        content = f.read()

    original = content

    # Change 1: Remove g_syst_only and g_ratio_syst_only from declarations
    content = re.sub(
        r'g_syst = ROOT\.TGraphAsymmErrors\(n_bins\)\n\s+g_syst_only = ROOT\.TGraphAsymmErrors\(n_bins\)\n\s+g_ratio_band = ROOT\.TGraphAsymmErrors\(n_bins\)\n\s+g_ratio_syst_only = ROOT\.TGraphAsymmErrors\(n_bins\)\n\s+g_ratio_stat = ROOT\.TGraphAsymmErrors\(n_bins\)\n\s+_KEEPER\.extend\(\[g_syst, g_syst_only, g_ratio_band, g_ratio_syst_only, g_ratio_stat\]\)',
        '''g_syst = ROOT.TGraphAsymmErrors(n_bins)
    g_ratio_band = ROOT.TGraphAsymmErrors(n_bins)
    g_ratio_stat = ROOT.TGraphAsymmErrors(n_bins)
    _KEEPER.extend([g_syst, g_ratio_band, g_ratio_stat])''',
        content
    )

    # Change 2: Remove g_syst_only and g_ratio_syst_only SetPoint calls
    content = re.sub(
        r'(\s+g_syst\.SetPoint\(i-1, x, y\); g_syst\.SetPointError\(i-1, w, w, err_dn, err_up\))\n\s+g_syst_only\.SetPoint\(i-1, x, y\); g_syst_only\.SetPointError\(i-1, w, w, syst_dn, syst_up\)\n(\s+if y > 0:\n\s+g_ratio_band\.SetPoint\(i-1, x, 1\.0\); g_ratio_band\.SetPointError\(i-1, w, w, err_dn/y, err_up/y\))\n\s+g_ratio_syst_only\.SetPoint\(i-1, x, 1\.0\); g_ratio_syst_only\.SetPointError\(i-1, w, w, syst_dn/y, syst_up/y\)\n(\s+g_ratio_stat\.SetPoint\(i-1, x, 1\.0\); g_ratio_stat\.SetPointError\(i-1, w, w, stat/y, stat/y\))',
        r'\1\n\2\n\3',
        content
    )

    # Change 3: Update main panel - remove g_syst_only drawing and update legend
    content = re.sub(
        r'stack\.Draw\("HIST SAME"\)\n\s+g_syst\.SetFillColorAlpha\(ROOT\.kBlack, 0\.6\); g_syst\.SetFillStyle\(3013\); g_syst\.Draw\("SAME E2"\)\n\s+g_syst_only\.SetFillColorAlpha\(ROOT\.kBlue, 0\.35\); g_syst_only\.SetFillStyle\(3254\); g_syst_only\.Draw\("SAME E2"\)\n\s+\n\s+if h_data and not IS_BLIND:\n\s+h_data\.SetMarkerStyle\(20\); h_data\.SetMarkerSize\(1\.2\); h_data\.SetLineColor\(ROOT\.kBlack\); h_data\.Draw\("PE SAME"\)\n\s+\n\s+leg = ROOT\.TLegend\([^)]+\);[^\n]+\n\s+for g_key, v in reversed\(sorted_for_stack\): leg\.AddEntry\(group_hists\[g_key\], BACKGROUND_GROUPS\[g_key\]\[1\], "f"\)\n\s+if h_data and not IS_BLIND: leg\.AddEntry\(h_data, "Data", "pe"\)\n\s+leg\.AddEntry\(g_syst, "[^"]*", "f"\)\n\s+leg\.AddEntry\(g_syst_only, "[^"]*", "f"\)\n\s+leg\.AddEntry\(g_ratio_stat, "[^"]*", "f"\)\n\s+leg\.Draw\(\)',
        '''stack.Draw("HIST SAME")
    g_syst.SetFillColorAlpha(ROOT.kBlack, 0.6); g_syst.SetFillStyle(3013); g_syst.Draw("SAME E2")

    if h_data and not IS_BLIND:
        h_data.SetMarkerStyle(20); h_data.SetMarkerSize(1.2); h_data.SetLineColor(ROOT.kBlack); h_data.Draw("PE SAME")

    leg = ROOT.TLegend(0.55, 0.55, 0.92, 0.88); leg.SetBorderSize(0); leg.SetFillStyle(0); leg.SetTextSize(0.035)
    for g_key, v in reversed(sorted_for_stack): leg.AddEntry(group_hists[g_key], BACKGROUND_GROUPS[g_key][1], "f")
    if h_data and not IS_BLIND: leg.AddEntry(h_data, "Data", "pe")
    leg.AddEntry(g_syst, "Stat #oplus Syst Unc.", "f")
    leg.Draw()''',
        content
    )

    # Change 4: Update ratio panel - remove g_ratio_syst_only, use yellow for stat-only
    content = re.sub(
        r'else:\n\s+g_ratio_band\.SetFillColor\(ROOT\.kGray\+1\); g_ratio_band\.SetFillStyle\(3013\); g_ratio_band\.Draw\("SAME E2"\)\n\s+g_ratio_syst_only\.SetFillColorAlpha\(ROOT\.kBlue, 0\.35\); g_ratio_syst_only\.SetFillStyle\(3254\); g_ratio_syst_only\.Draw\("SAME E2"\)\n\s+g_ratio_stat\.SetFillColor\(ROOT\.kOrange\); g_ratio_stat\.SetFillStyle\(3005\); g_ratio_stat\.Draw\("SAME E2"\)',
        '''else:
        g_ratio_band.SetFillColor(ROOT.kGray+1); g_ratio_band.SetFillStyle(3013); g_ratio_band.Draw("SAME E2")
        g_ratio_stat.SetFillColor(ROOT.kYellow); g_ratio_stat.SetFillStyle(1001); g_ratio_stat.Draw("SAME E2")''',
        content
    )

    if content != original:
        with open(filepath, 'w') as f:
            f.write(content)
        return True
    return False

def main():
    base_dir = Path(__file__).parent

    # Find all EE.py and MM.py files
    plotter_files = []
    for pattern in ['**/EE.py', '**/MM.py']:
        plotter_files.extend(base_dir.glob(pattern))

    plotter_files = [f for f in plotter_files if 'backup' not in f.name.lower()]

    print(f"Updating {len(plotter_files)} plotter files...")
    updated = 0
    for filepath in plotter_files:
        if update_file(filepath):
            print(f"  ✓ Updated: {filepath.relative_to(base_dir)}")
            updated += 1
        else:
            print(f"  - No changes: {filepath.relative_to(base_dir)}")

    print(f"\n✓ Updated {updated}/{len(plotter_files)} files")
    print("\nChanges applied:")
    print("  • Removed systematic-only error bands")
    print("  • p1 (main panel): Shows only stat+syst (black/gray)")
    print("  • p2 (ratio panel): Shows stat+syst (gray) and stat-only (yellow)")

if __name__ == "__main__":
    main()
