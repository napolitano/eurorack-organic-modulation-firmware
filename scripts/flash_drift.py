#!/usr/bin/env python3
"""Build or upload a Drift bank or named algorithm through PlatformIO."""
from __future__ import annotations

import argparse
import os
import shutil
import subprocess
import sys
from pathlib import Path

from drift_targets import algorithm_bank, canonical_algorithm, canonical_bank, platformio_environment


def parser() -> argparse.ArgumentParser:
    result = argparse.ArgumentParser(
        description="Build/flash a complete Drift bank or a DIP-independent named algorithm developer target."
    )
    result.add_argument("--bootloader", choices=("new", "old"), default="new", help="Arduino Nano bootloader variant")
    result.add_argument("--port", help="optional upload port, e.g. COM5 or /dev/ttyUSB0")
    result.add_argument("--build-only", action="store_true", help="compile without uploading")
    result.add_argument("--dry-run", action="store_true", help="print the PlatformIO command without executing it")
    sub = result.add_subparsers(dest="mode", required=True)
    bank = sub.add_parser("bank", help="normal bank firmware; rear DIP switches select one of four algorithms")
    bank.add_argument("name")
    algorithm = sub.add_parser("algorithm", help="developer firmware locked to one algorithm by name; DIP switches ignored")
    algorithm.add_argument("name")
    return result


def build_command(args: argparse.Namespace) -> tuple[list[str], dict[str, str], str, str | None]:
    child_env = dict(os.environ)
    child_env.pop("FMD_FORCE_ALGORITHM", None)

    if args.mode == "bank":
        bank = canonical_bank(args.name)
        algorithm = None
    else:
        algorithm = canonical_algorithm(args.name)
        bank = algorithm_bank(algorithm)
        child_env["FMD_FORCE_ALGORITHM"] = algorithm

    pio_env = platformio_environment(bank, args.bootloader)
    command = ["pio", "run", "-e", pio_env]
    if not args.build_only:
        command += ["-t", "upload"]
        if args.port:
            command += ["--upload-port", args.port]
    return command, child_env, bank, algorithm


def main() -> int:
    args = parser().parse_args()
    try:
        command, child_env, bank, algorithm = build_command(args)
    except ValueError as exc:
        print(f"flash-target error: {exc}", file=sys.stderr)
        return 2

    if algorithm is None:
        print(f"Target: complete {bank.title()} bank; rear DIP switches remain active")
    else:
        print(f"Target: {algorithm.title()} ({bank.title()} bank); rear DIP switches are ignored in this developer build")
    print("Command:", " ".join(command))

    if args.dry_run:
        return 0
    if shutil.which("pio") is None:
        print("flash-target error: PlatformIO CLI 'pio' was not found in PATH", file=sys.stderr)
        return 2
    return subprocess.call(command, cwd=Path(__file__).resolve().parents[1], env=child_env)


if __name__ == "__main__":
    raise SystemExit(main())
