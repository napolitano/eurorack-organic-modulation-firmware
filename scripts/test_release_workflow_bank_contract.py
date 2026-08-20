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
    "organic": {
        "bank detection": (
            "grep -q '^\\[env:native_organic\\]$' platformio.ini",
            "grep -q '^\\[env:nanoatmega328new_organic\\]$' platformio.ini",
            "grep -q '^\\[env:nanoatmega328_organic\\]$' platformio.ini",
            'RELEASE_BANKS="${RELEASE_BANKS},organic"',
        ),
        "native qualification": (
                    "pio test -e native_organic_sanitized",
            "pio test -e native_organic_coverage",
        ),
        "AVR builds": (
            "pio run -e nanoatmega328new_organic",
            "pio run -e nanoatmega328_organic",
        ),
        "resource qualification": (
            ".pio/build/nanoatmega328new_organic/firmware.elf",
            ".pio/build/nanoatmega328_organic/firmware.elf",
        ),
        "timing qualification": ("pio run -e nanoatmega328new_organic_timing",),
        "build provenance": (
            "BUILD-INFO-organic-nano-new.${RELEASE_VERSION}.txt",
            "BUILD-INFO-organic-nano-old.${RELEASE_VERSION}.txt",
            "--environment nanoatmega328new_organic",
            "--environment nanoatmega328_organic",
        ),
    },
    "generative": {
        "bank detection": (
            "grep -q '^\\[env:native_generative\\]$' platformio.ini",
            "grep -q '^\\[env:nanoatmega328new_generative\\]$' platformio.ini",
            "grep -q '^\\[env:nanoatmega328_generative\\]$' platformio.ini",
            'RELEASE_BANKS="${RELEASE_BANKS},generative"',
        ),
        "native qualification": (
                    "pio test -e native_generative_sanitized",
            "pio test -e native_generative_coverage",
        ),
        "AVR builds": (
            "pio run -e nanoatmega328new_generative",
            "pio run -e nanoatmega328_generative",
        ),
        "resource qualification": (
            ".pio/build/nanoatmega328new_generative/firmware.elf",
            ".pio/build/nanoatmega328_generative/firmware.elf",
        ),
        "timing qualification": ("pio run -e nanoatmega328new_generative_timing",),
        "manual coverage": (
            "Require Generative content in frozen manual",
            'required = ("generative", "turing", "markov", "motif", "urn")',
        ),
        "build provenance": (
            "BUILD-INFO-generative-nano-new.${RELEASE_VERSION}.txt",
            "BUILD-INFO-generative-nano-old.${RELEASE_VERSION}.txt",
            "--environment nanoatmega328new_generative",
            "--environment nanoatmega328_generative",
        ),
    },
    "ambient": {
        "bank detection": (
            "grep -q '^\\[env:native_ambient\\]$' platformio.ini",
            "grep -q '^\\[env:nanoatmega328new_ambient\\]$' platformio.ini",
            "grep -q '^\\[env:nanoatmega328_ambient\\]$' platformio.ini",
            'RELEASE_BANKS="${RELEASE_BANKS},ambient"',
        ),
        "native qualification": (
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
    "percussion": {
        "bank detection": (
            "grep -q '^\\[env:native_percussion\\]$' platformio.ini",
            "grep -q '^\\[env:nanoatmega328new_percussion\\]$' platformio.ini",
            "grep -q '^\\[env:nanoatmega328_percussion\\]$' platformio.ini",
            'RELEASE_BANKS="${RELEASE_BANKS},percussion"',
        ),
        "native qualification": (
                    "pio test -e native_percussion_sanitized",
            "pio test -e native_percussion_coverage",
        ),
        "AVR builds": (
            "pio run -e nanoatmega328new_percussion",
            "pio run -e nanoatmega328_percussion",
        ),
        "resource qualification": (
            ".pio/build/nanoatmega328new_percussion/firmware.elf",
            ".pio/build/nanoatmega328_percussion/firmware.elf",
        ),
        "timing qualification": ("pio run -e nanoatmega328new_percussion_timing",),
        "manual coverage": (
            "Require Percussion content in frozen manual",
            'required = ("percussion", "euclid", "repeat", "probability", "humanize")',
        ),
        "build provenance": (
            "BUILD-INFO-percussion-nano-new.${RELEASE_VERSION}.txt",
            "BUILD-INFO-percussion-nano-old.${RELEASE_VERSION}.txt",
            "--environment nanoatmega328new_percussion",
            "--environment nanoatmega328_percussion",
        ),
    },
}



BANK_WIRING_FILTERS = (
    "-f unit/test_algorithms",
    "-f integration/test_selection",
    "-f integration/test_runtime",
    "-f property/test_invariants",
    "-f system/test_signal_path",
)

UNFILTERED_RELEASE_COMMANDS = (
    "pio test -e native_sanitized",
    "pio test -e native_organic_sanitized",
    "pio test -e native_generative_sanitized",
    "pio test -e native_ambient_sanitized",
    "pio test -e native_electronica_sanitized",
    "pio test -e native_percussion_sanitized",
    "pio test -e native_coverage",
    "pio test -e native_organic_coverage",
    "pio test -e native_generative_coverage",
    "pio test -e native_ambient_coverage",
    "pio test -e native_electronica_coverage",
    "pio test -e native_percussion_coverage",
)

COMMON_FRAGMENTS = (
    '--banks "${RELEASE_BANKS}"',
    "check_release_artifact_set.py",
    "actions/cache@v6.1.0",
    "libreoffice-writer fonts-ubuntu-classic poppler-utils",
    '"Ubuntu-R.ttf"',
    '"Ubuntu-L.ttf"',
    "manual_diagnostics()",
    "trap manual_diagnostics ERR",
)

FORBIDDEN_FRAGMENTS = (
    "actions/cache@v4.2.0",
    "libreoffice-writer fonts-ubuntu poppler-utils",
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
        missing.append("release/publication contract: " + ", ".join(absent_common))

    forbidden = [fragment for fragment in FORBIDDEN_FRAGMENTS if fragment in text]
    if forbidden:
        missing.append("obsolete release/publication dependency: " + ", ".join(forbidden))


    stripped_lines = {line.strip() for line in text.splitlines()}
    for command in UNFILTERED_RELEASE_COMMANDS:
        if command in stripped_lines:
            missing.append(f"unfiltered release test command: {command}")

    if sum(1 for line in stripped_lines if line == "run: pio test -e native" or line == "pio test -e native") != 1:
        missing.append("release/native full-suite run must occur exactly once")

    for fragment in BANK_WIRING_FILTERS:
        if fragment not in text:
            missing.append(f"release/bank wiring filter: {fragment}")


    for environment in (
        "native_organic",
        "native_generative",
        "native_ambient",
        "native_electronica",
        "native_percussion",
    ):
        filtered_block = (
            f"pio test -e {environment}\n"
            "          -f unit/test_algorithms\n"
            "          -f integration/test_selection\n"
            "          -f integration/test_runtime\n"
            "          -f property/test_invariants\n"
            "          -f system/test_signal_path"
        )
        if filtered_block not in text:
            missing.append(f"release/filtered bank wiring block: {environment}")

    if missing:
        for item in missing:
            print(f"release workflow bank contract error: {item}")
        return 2

    print("release workflow six-bank contract: passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
