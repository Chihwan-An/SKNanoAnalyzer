#!/usr/bin/env python3
"""Fix drawing order and colors in all files"""

import re
from pathlib import Path

def fix_file(filepath):
    """Fix drawing order: stat first (orange), then stat+syst (dark gray)"""
    with open(filepath, 'r') as f:
        content = f.read()

    original = content

    # Fix ratio panel - various possible patterns
    # Pattern 1: Current wrong order
    content = re.sub(
        r'g_ratio_band\.SetFillColor(?:Alpha)?\([^)]+\);[^\n]+g_ratio_band\.Draw\([^)]+\)\n\s+g_ratio_stat\.SetFillColor(?:Alpha)?\([^)]+\);[^\n]+g_ratio_stat\.Draw\([^)]+\)',
        'g_ratio_stat.SetFillColorAlpha(ROOT.kOrange, 0.6); g_ratio_stat.SetFillStyle(3005); g_ratio_stat.Draw("SAME E2")\n        g_ratio_band.SetFillColorAlpha(ROOT.kGray+2, 0.8); g_ratio_band.SetFillStyle(3013); g_ratio_band.Draw("SAME E2")',
        content
    )

    # Fix main panel - make sure stat+syst is dark gray
    content = re.sub(
        r'g_syst\.SetFillColor(?:Alpha)?\(ROOT\.k(?:Black|Gray\+1)[^)]*\); g_syst\.SetFillStyle\(3013\); g_syst\.Draw\("SAME E2"\)',
        'g_syst.SetFillColorAlpha(ROOT.kGray+2, 0.8); g_syst.SetFillStyle(3013); g_syst.Draw("SAME E2")',
        content
    )

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

    print(f"Fixing drawing order in {len(plotter_files)} files...")
    fixed = 0
    for filepath in plotter_files:
        if fix_file(filepath):
            print(f"  ✓ Fixed: {filepath.relative_to(base_dir)}")
            fixed += 1
        else:
            print(f"  - Already correct: {filepath.relative_to(base_dir)}")

    print(f"\n✓ Fixed {fixed}/{len(plotter_files)} files")
    print("\nDrawing order:")
    print("  1. Stat only (orange, alpha=0.6) - drawn first")
    print("  2. Stat+Syst (dark gray, alpha=0.8) - drawn second")
    print("  3. Data points - drawn last")

if __name__ == "__main__":
    main()
