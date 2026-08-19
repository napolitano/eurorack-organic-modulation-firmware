#!/usr/bin/env python3
"""Guard the tagged-release workflow's Ambient-bank publication contract.

This is deliberately a lightweight structural test.  It prevents a future
workflow edit from leaving Ambient implemented/tested in the repository while
silently dropping one of its release stages.

SPDX-License-Identifier: GPL-3.0-or-later
"""
from __future__ import annotations

from pathlib import Path


WORKFLOW = Path(".github/workflows/release.yml")


REQUIRED_FRAGMENTS = {
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
    "bank-aware packaging": (
        '--banks "${RELEASE_BANKS}"',
        "check_release_artifact_set.py",
    ),
}


def main() -> int:
    text = WORKFLOW.read_text(encoding="utf-8")
    missing: list[str] = []
    for stage, fragments in REQUIRED_FRAGMENTS.items():
        absent = [fragment for fragment in fragments if fragment not in text]
        if absent:
            missing.append(f"{stage}: {', '.join(absent)}")

    if missing:
        for item in missing:
            print(f"release workflow bank contract error: {item}")
        return 2

    print("release workflow Ambient-bank contract: passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
