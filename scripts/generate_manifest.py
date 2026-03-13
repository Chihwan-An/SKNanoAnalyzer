#!/usr/bin/env python3
import os
import json
import sys
from datetime import datetime, timezone

SUPPORTED_EXTS = {'.png', '.jpg', '.jpeg', '.svg', '.gif', '.pdf'}
SKIP_NAMES = {'all.sh', 'clean.sh', 'plot.sh', 'plotall.sh', 'organize_files.py'}

def scan(base_path):
    files = []
    for root, dirs, filenames in os.walk(base_path):
        dirs[:] = sorted(d for d in dirs if not d.startswith('.'))
        for name in sorted(filenames):
            if name in SKIP_NAMES:
                continue
            ext = os.path.splitext(name)[1].lower()
            if ext not in SUPPORTED_EXTS:
                continue
            rel = os.path.relpath(os.path.join(root, name), base_path)
            files.append({
                "path": rel.replace(os.sep, '/'),
                "name": name,
                "ext": ext.lstrip('.')
            })
    return files

def main():
    plots_dir = sys.argv[1] if len(sys.argv) > 1 else "plots"
    if not os.path.isdir(plots_dir):
        print(f"Error: directory '{plots_dir}' not found", file=sys.stderr)
        sys.exit(1)
    files = scan(plots_dir)
    manifest = {
        "generated": datetime.now(timezone.utc).strftime('%Y-%m-%dT%H:%M:%SZ'),
        "count": len(files),
        "files": files
    }
    print(json.dumps(manifest, indent=2))

if __name__ == "__main__":
    main()
