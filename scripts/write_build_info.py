#!/usr/bin/env python3
"""Write human-readable release build and toolchain metadata."""
from __future__ import annotations

import argparse
import os
import platform
from pathlib import Path
import subprocess
import sys


def capture(command: list[str]) -> str:
    result = subprocess.run(command, check=True, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    return result.stdout.strip()


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", required=True)
    parser.add_argument("--environment", default="nanoatmega328new")
    args = parser.parse_args()

    home = Path.home()
    avr_gxx = home / ".platformio" / "packages" / "toolchain-atmelavr" / "bin" / (
        "avr-g++.exe" if sys.platform.startswith("win") else "avr-g++"
    )
    lines = [
        "FM Drift release build information",
        "==================================",
        f"Host: {platform.platform()}",
        f"Python: {platform.python_version()}",
        f"PlatformIO: {capture(['pio', '--version'])}",
        f"Git commit: {os.environ.get('GITHUB_SHA') or capture(['git', 'rev-parse', 'HEAD'])}",
        f"Git ref/tag: {os.environ.get('GITHUB_REF_NAME', 'local build')}",
        "",
        "Resolved Python packages:",
        capture([sys.executable, "-m", "pip", "freeze"]),
        "",
        f"PlatformIO packages ({args.environment}):",
        capture(["pio", "pkg", "list", "-e", args.environment]),
        "",
        "AVR compiler:",
        capture([str(avr_gxx), "--version"]) if avr_gxx.exists() else "avr-g++ path not found",
        "",
    ]
    Path(args.output).write_text("\n".join(lines), encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
