#!/usr/bin/env python3
"""Remove g_syst_only drawing lines"""

import re
from pathlib import Path

def fix_file(filepath):
    """Remove g_syst_only drawing line"""
    with open(filepath, 'r') as f:
        content = f.read()

    original = content

    # Remove the g_syst_only drawing line
    content = re.sub(
        r'\s+g_syst_only\.SetFillColorAlpha\(ROOT\.kBlue, 0\.35\); g_syst_only\.SetFillStyle\(3254\); g_syst_only\.Draw\("SAME E2"\)\n',
        '',
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

    print(f"Removing g_syst_only draw calls from {len(plotter_files)} files...")
    fixed = 0
    for filepath in plotter_files:
        if fix_file(filepath):
            print(f"  ✓ Fixed: {filepath.relative_to(base_dir)}")
            fixed += 1
        else:
            print(f"  - No fix needed: {filepath.relative_to(base_dir)}")

    print(f"\n✓ Fixed {fixed}/{len(plotter_files)} files")

if __name__ == "__main__":
    main()
