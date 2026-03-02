#!/usr/bin/env python3
"""Make stat-only error band lighter with transparency"""

import re
from pathlib import Path

def fix_file(filepath):
    """Make stat-only band lighter"""
    with open(filepath, 'r') as f:
        content = f.read()

    original = content

    # Change stat-only to lighter yellow with transparency and hatched pattern
    content = re.sub(
        r'g_ratio_stat\.SetFillColor\(ROOT\.kYellow\); g_ratio_stat\.SetFillStyle\(1001\); g_ratio_stat\.Draw\("SAME E2"\)',
        'g_ratio_stat.SetFillColorAlpha(ROOT.kYellow-7, 0.5); g_ratio_stat.SetFillStyle(3005); g_ratio_stat.Draw("SAME E2")',
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

    print(f"Making stat-only band lighter in {len(plotter_files)} files...")
    fixed = 0
    for filepath in plotter_files:
        if fix_file(filepath):
            print(f"  ✓ Updated: {filepath.relative_to(base_dir)}")
            fixed += 1
        else:
            print(f"  - No change needed: {filepath.relative_to(base_dir)}")

    print(f"\n✓ Updated {fixed}/{len(plotter_files)} files")
    print("Stat-only band is now: kYellow-7 with 50% transparency and hatched pattern")

if __name__ == "__main__":
    main()
