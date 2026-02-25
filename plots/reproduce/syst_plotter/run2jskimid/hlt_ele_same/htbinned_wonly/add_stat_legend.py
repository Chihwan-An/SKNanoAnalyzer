#!/usr/bin/env python3
"""Add stat-only entry back to legend"""

import re
from pathlib import Path

def fix_file(filepath):
    """Add stat-only to legend"""
    with open(filepath, 'r') as f:
        content = f.read()

    original = content

    # Add stat-only entry to legend
    content = re.sub(
        r'(leg\.AddEntry\(g_syst, "Stat #oplus Syst Unc\.", "f"\))\n(\s+leg\.Draw\(\))',
        r'\1\n    leg.AddEntry(g_ratio_stat, "Stat. Unc.", "f")\n\2',
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

    print(f"Adding stat-only to legend in {len(plotter_files)} files...")
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
