#!/usr/bin/env python3
"""
Reorder drawing and change colors:
1. Stat only (orange) - drawn first (back)
2. Stat+Syst (dark gray) - drawn second (middle)
3. Data points - drawn last (front)
"""

import re
from pathlib import Path

def fix_file(filepath):
    """Reorder and recolor error bands"""
    with open(filepath, 'r') as f:
        content = f.read()

    original = content

    # Fix main panel (p1): Only draw stat+syst in dark gray
    # Change drawing order and color
    pattern1 = r'stack\.Draw\("HIST SAME"\)\n\s+g_syst\.SetFillColorAlpha\(ROOT\.kBlack, 0\.6\); g_syst\.SetFillStyle\(3013\); g_syst\.Draw\("SAME E2"\)\n\s+if h_data and not IS_BLIND:\n\s+h_data\.SetMarkerStyle\(20\); h_data\.SetMarkerSize\(1\.2\); h_data\.SetLineColor\(ROOT\.kBlack\); h_data\.Draw\("PE SAME"\)'

    replacement1 = '''stack.Draw("HIST SAME")
    g_syst.SetFillColorAlpha(ROOT.kGray+2, 0.8); g_syst.SetFillStyle(3013); g_syst.Draw("SAME E2")

    if h_data and not IS_BLIND:
        h_data.SetMarkerStyle(20); h_data.SetMarkerSize(1.2); h_data.SetLineColor(ROOT.kBlack); h_data.Draw("PE SAME")'''

    content = re.sub(pattern1, replacement1, content)

    # Fix ratio panel (p2): Draw stat-only first (orange), then stat+syst (dark gray)
    pattern2 = r'else:\n\s+g_ratio_band\.SetFillColor\(ROOT\.kGray\+1\); g_ratio_band\.SetFillStyle\(3013\); g_ratio_band\.Draw\("SAME E2"\)\n\s+g_ratio_stat\.SetFillColorAlpha\(ROOT\.kYellow-7, 0\.5\); g_ratio_stat\.SetFillStyle\(3005\); g_ratio_stat\.Draw\("SAME E2"\)'

    replacement2 = '''else:
        g_ratio_stat.SetFillColorAlpha(ROOT.kOrange, 0.6); g_ratio_stat.SetFillStyle(3005); g_ratio_stat.Draw("SAME E2")
        g_ratio_band.SetFillColorAlpha(ROOT.kGray+2, 0.8); g_ratio_band.SetFillStyle(3013); g_ratio_band.Draw("SAME E2")'''

    content = re.sub(pattern2, replacement2, content)

    if content != original:
        with open(filepath, 'w') as f:
            f.write(content)
        return True
    return False

def main():
    base_dir = Path(__file__).parent

    plotter_files = []
    for pattern in ['**/EE.py', '**/MM.py']:
        plotter_files.extend(base_dir.glob(pattern))

    plotter_files = [f for f in plotter_files if 'backup' not in f.name.lower()]

    print(f"Reordering and recoloring in {len(plotter_files)} files...")
    print("\nNew configuration:")
    print("  • Stat only: Orange (kOrange, alpha=0.6) - drawn first (back)")
    print("  • Stat+Syst: Dark gray (kGray+2, alpha=0.8) - drawn second (middle)")
    print("  • Data points: Drawn last (front)")
    print()

    fixed = 0
    for filepath in plotter_files:
        if fix_file(filepath):
            print(f"  ✓ Updated: {filepath.relative_to(base_dir)}")
            fixed += 1
        else:
            print(f"  - No change: {filepath.relative_to(base_dir)}")

    print(f"\n✓ Updated {fixed}/{len(plotter_files)} files")

if __name__ == "__main__":
    main()
