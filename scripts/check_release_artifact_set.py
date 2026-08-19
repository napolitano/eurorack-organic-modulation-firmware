#!/usr/bin/env python3
"""Validate the exact firmware/provenance set prepared for a Drift release.

The release workflow uses this as a final packaging contract before checksums
and publication.  It is intentionally bank-aware so historical tags can be
rebuilt without inventing firmware banks that did not exist in that tag.

SPDX-License-Identifier: GPL-3.0-or-later
"""
from __future__ import annotations

import argparse
import re
from pathlib import Path

VERSION_RE = re.compile(r"^[0-9]+\.[0-9]+\.[0-9]+$")
VALID_BANKS = ("classic", "organic", "generative", "ambient", "electronica", "percussion")
BOOTLOADERS = ("new", "old")
FIRMWARE_EXTENSIONS = ("hex", "elf")


def parse_banks(raw: str) -> tuple[str, ...]:
    banks = tuple(dict.fromkeys(part.strip().lower() for part in raw.split(",") if part.strip()))
    if not banks:
        raise ValueError("at least one algorithm bank must be selected")
    invalid = [bank for bank in banks if bank not in VALID_BANKS]
    if invalid:
        raise ValueError(f"unknown algorithm bank(s): {', '.join(invalid)}")
    if "classic" not in banks:
        raise ValueError("Classic must remain part of every published Drift release")
    return banks


def expected_firmware_names(version: str, banks: tuple[str, ...]) -> set[str]:
    return {
        f"fm-drift-{bank}-nano-{bootloader}-bootloader.{version}.{extension}"
        for bank in banks
        for bootloader in BOOTLOADERS
        for extension in FIRMWARE_EXTENSIONS
    }


def expected_build_info_names(version: str, banks: tuple[str, ...]) -> set[str]:
    return {
        f"BUILD-INFO-{bank}-nano-{bootloader}.{version}.txt"
        for bank in banks
        for bootloader in BOOTLOADERS
    }


def require_nonempty(output_dir: Path, names: set[str]) -> list[str]:
    errors: list[str] = []
    for name in sorted(names):
        path = output_dir / name
        if not path.is_file():
            errors.append(f"missing release artifact: {name}")
        elif path.stat().st_size == 0:
            errors.append(f"empty release artifact: {name}")
    return errors


def validate_release_artifacts(output_dir: Path, version: str, banks: tuple[str, ...]) -> list[str]:
    expected_firmware = expected_firmware_names(version, banks)
    expected_build_info = expected_build_info_names(version, banks)
    errors = require_nonempty(output_dir, expected_firmware | expected_build_info)

    actual_firmware = {
        path.name
        for path in output_dir.glob(f"fm-drift-*-nano-*-bootloader.{version}.*")
        if path.suffix in {".hex", ".elf"}
    }
    missing = expected_firmware - actual_firmware
    unexpected = actual_firmware - expected_firmware
    for name in sorted(missing):
        if f"missing release artifact: {name}" not in errors:
            errors.append(f"missing release firmware: {name}")
    for name in sorted(unexpected):
        errors.append(f"unexpected release firmware for selected bank set: {name}")

    manifest = output_dir / f"FIRMWARE-ARTIFACTS.{version}.md"
    if not manifest.is_file() or manifest.stat().st_size == 0:
        errors.append(f"missing release artifact: {manifest.name}")
    else:
        text = manifest.read_text(encoding="utf-8")
        for name in sorted(expected_firmware | expected_build_info):
            if name not in text:
                errors.append(f"firmware manifest does not reference: {name}")

    return errors


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--version", required=True)
    parser.add_argument("--banks", required=True)
    parser.add_argument("--output-dir", default="dist")
    args = parser.parse_args()

    if not VERSION_RE.fullmatch(args.version):
        raise SystemExit(f"release-artifact-set error: invalid version {args.version!r}")
    try:
        banks = parse_banks(args.banks)
    except ValueError as exc:
        raise SystemExit(f"release-artifact-set error: {exc}") from exc

    errors = validate_release_artifacts(Path(args.output_dir), args.version, banks)
    if errors:
        for error in errors:
            print(f"release-artifact-set error: {error}")
        return 2

    firmware_count = len(expected_firmware_names(args.version, banks))
    print(
        f"release artifact set: passed ({len(banks)} bank(s), "
        f"{firmware_count} firmware files, {2 * len(banks)} build-info files)"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
