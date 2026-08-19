#!/usr/bin/env python3
"""Guard tagged-release publication contracts for optional Drift banks.

This lightweight structural test prevents implemented banks from silently
falling out of release detection, qualification, firmware builds, manual
coverage checks, provenance generation or bank-aware packaging.

SPDX-License-Identifier: GPL-3.0-or-later
"""
from __future__ import annotations

from pathlib import Path

WORKFLOW = Path(".github/workflows/release.yml")

BANK_FRAGMENTS = {
    "ambient": {
        "bank detection": (
            "grep -q '^\\[env:native_ambient\\]$' platformio.ini",
            "grep -q '^\\[env:nanoatmega328new_ambient\\]$' platformio.ini",
            "grep -q '^\\[env:nanoatmega328_ambient\\]$' platformio.ini",
            'RELEASE_BANKS="${RELEASE_BANKS},ambient"',
        ),
        "native qualification": (
            "pio test -e native_ambient",
            "pio test -e native_ambient_sanitized",
            "pio test -e native_ambient_coverage",
        ),
        "AVR builds": (
            "pio run -e nanoatmega328new_ambient",
            "pio run -e nanoatmega328_ambient",
        ),
        "resource qualification": (
            ".pio/build/nanoatmega328new_ambient/firmware.elf",
            ".pio/build/nanoatmega328_ambient/firmware.elf",
        ),
        "timing qualification": ("pio run -e nanoatmega328new_ambient_timing",),
        "manual coverage": (
            "Require Ambient content in frozen manual",
            'required = ("ambient", "current", "anchor", "breath", "fog")',
        ),
        "build provenance": (
            "BUILD-INFO-ambient-nano-new.${RELEASE_VERSION}.txt",
            "BUILD-INFO-ambient-nano-old.${RELEASE_VERSION}.txt",
            "--environment nanoatmega328new_ambient",
            "--environment nanoatmega328_ambient",
        ),
    },
    "electronica": {
        "bank detection": (
            "grep -q '^\\[env:native_electronica\\]$' platformio.ini",
            "grep -q '^\\[env:nanoatmega328new_electronica\\]$' platformio.ini",
            "grep -q '^\\[env:nanoatmega328_electronica\\]$' platformio.ini",
            'RELEASE_BANKS="${RELEASE_BANKS},electronica"',
        ),
        "native qualification": (
            "pio test -e native_electronica",
            "pio test -e native_electronica_sanitized",
            "pio test -e native_electronica_coverage",
        ),
        "AVR builds": (
            "pio run -e nanoatmega328new_electronica",
            "pio run -e nanoatmega328_electronica",
        ),
        "resource qualification": (
            ".pio/build/nanoatmega328new_electronica/firmware.elf",
            ".pio/build/nanoatmega328_electronica/firmware.elf",
        ),
        "timing qualification": ("pio run -e nanoatmega328new_electronica_timing",),
        "manual coverage": (
            "Require Electronica content in frozen manual",
            'required = ("electronica", "pump", "acid", "shuffle", "polymeter")',
        ),
        "build provenance": (
            "BUILD-INFO-electronica-nano-new.${RELEASE_VERSION}.txt",
            "BUILD-INFO-electronica-nano-old.${RELEASE_VERSION}.txt",
            "--environment nanoatmega328new_electronica",
            "--environment nanoatmega328_electronica",
        ),
    },
}

COMMON_FRAGMENTS = (
    '--banks "${RELEASE_BANKS}"',
    "check_release_artifact_set.py",
)


def main() -> int:
    text = WORKFLOW.read_text(encoding="utf-8")
    missing: list[str] = []
    for bank, stages in BANK_FRAGMENTS.items():
        for stage, fragments in stages.items():
            absent = [fragment for fragment in fragments if fragment not in text]
            if absent:
                missing.append(f"{bank}/{stage}: {', '.join(absent)}")
    absent_common = [fragment for fragment in COMMON_FRAGMENTS if fragment not in text]
    if absent_common:
        missing.append("bank-aware packaging: " + ", ".join(absent_common))

    if missing:
        for item in missing:
            print(f"release workflow bank contract error: {item}")
        return 2

    print("release workflow Ambient/Electronica bank contract: passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
