#!/usr/bin/env python3
"""Repository-local entry point for the schema-driven ROOT plotter."""

from pathlib import Path
import sys


REPOSITORY = Path(__file__).absolute().parents[1]
sys.path.insert(0, str(REPOSITORY / "python"))

from sknano_plotter.cli import main


if __name__ == "__main__":
    raise SystemExit(main())
