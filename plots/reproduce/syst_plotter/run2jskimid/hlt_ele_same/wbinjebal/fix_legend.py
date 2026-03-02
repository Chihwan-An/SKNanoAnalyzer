#!/usr/bin/env python3
"""Fix legend to only show stat+syst in main panel"""

import re
from pathlib import Path

def fix_legend(filepath):
    """Fix the legend section"""
    with open(filepath, 'r') as f:
        content = f.read()

    original = content

    # Fix legend - remove g_syst_only and g_ratio_stat entries
    pattern = r'(leg = ROOT\.TLegend\(0\.55, 0\.50, 0\.92, 0\.88\);[^\n]+\n\s+for g_key, v in reversed\(sorted_for_stack\): leg\.AddEntry\(group_hists\[g_key\], BACKGROUND_GROUPS\[g_key\]\[1\], "f"\)\n\s+if h_data and not IS_BLIND: leg\.AddEntry\(h_data, "Data", "pe"\)\n\s+leg\.AddEntry\(g_syst, "[^"]*", "f"\))\n\s+leg\.AddEntry\(g_syst_only, "[^"]*", "f"\)\n\s+leg\.AddEntry\(g_ratio_stat, "[^"]*", "f"\)'

    replacement = r'\1'

    content = re.sub(pattern, replacement, content)

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

    print(f"Fixing legends in {len(plotter_files)} files...")
    fixed = 0
    for filepath in plotter_files:
        if fix_legend(filepath):
            print(f"  ✓ Fixed: {filepath.relative_to(base_dir)}")
            fixed += 1
        else:
            print(f"  - No fix needed: {filepath.relative_to(base_dir)}")

    print(f"\n✓ Fixed {fixed}/{len(plotter_files)} files")

if __name__ == "__main__":
    main()
