import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
BUILD = ROOT / "build"

if BUILD.exists():
    sys.path.insert(0, str(BUILD))
