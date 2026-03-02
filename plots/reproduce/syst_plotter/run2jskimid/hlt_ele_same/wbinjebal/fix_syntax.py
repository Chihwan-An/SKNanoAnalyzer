#!/usr/bin/env python3
"""Fix syntax errors from the previous update"""

import re
from pathlib import Path

def fix_file(filepath):
    """Fix syntax error where newline was removed"""
    with open(filepath, 'r') as f:
        content = f.read()

    original = content

    # Fix the merged line: add newline before g_syst.SetPoint
    content = re.sub(
        r'(data_str = f"\{data_val:10\.1f\}" if not IS_BLIND else f"\{\'Blinded\':>10\}")\s+(g_syst\.SetPoint)',
        r'\1\n\n        \2',
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

    print(f"Fixing {len(plotter_files)} files...")
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
