#!/usr/bin/env python3
"""Guard CI qualification/build coverage for all implemented Drift banks.

The normal CI workflow is the first place where a newly added bank can
silently fall out of the build matrix.  This structural test keeps native
qualification, coverage, sanitizers, AVR bootloader builds and timing probes
aligned with the PlatformIO bank environments.

SPDX-License-Identifier: GPL-3.0-or-later
"""
from __future__ import annotations

from pathlib import Path

WORKFLOW = Path(".github/workflows/ci.yml")

BANKS = {
    "classic": {
        "native": ("pio test -e native -f ${{ matrix.suite }}",),
        "coverage": ("pio test -e native_coverage",),
        "sanitizers": ("pio test -e native_sanitized",),
        "avr": ("nanoatmega328new", "nanoatmega328"),
        "timing": ("nanoatmega328new_timing",),
    },
    "organic": {
        "native": ("pio test -e native_organic -f ${{ matrix.suite }}",),
        "coverage": ("pio test -e native_organic_coverage",),
        "sanitizers": ("pio test -e native_organic_sanitized",),
        "avr": ("nanoatmega328new_organic", "nanoatmega328_organic"),
        "timing": ("nanoatmega328new_organic_timing",),
    },
    "generative": {
        "native": ("pio test -e native_generative -f ${{ matrix.suite }}",),
        "coverage": ("pio test -e native_generative_coverage",),
        "sanitizers": ("pio test -e native_generative_sanitized",),
        "avr": ("nanoatmega328new_generative", "nanoatmega328_generative"),
        "timing": ("nanoatmega328new_generative_timing",),
    },
    "ambient": {
        "native": ("pio test -e native_ambient -f ${{ matrix.suite }}",),
        "coverage": ("pio test -e native_ambient_coverage",),
        "sanitizers": ("pio test -e native_ambient_sanitized",),
        "avr": ("nanoatmega328new_ambient", "nanoatmega328_ambient"),
        "timing": ("nanoatmega328new_ambient_timing",),
    },
    "electronica": {
        "native": ("pio test -e native_electronica -f ${{ matrix.suite }}",),
        "coverage": ("pio test -e native_electronica_coverage",),
        "sanitizers": ("pio test -e native_electronica_sanitized",),
        "avr": ("nanoatmega328new_electronica", "nanoatmega328_electronica"),
        "timing": ("nanoatmega328new_electronica_timing",),
    },
    "percussion": {
        "native": ("pio test -e native_percussion -f ${{ matrix.suite }}",),
        "coverage": ("pio test -e native_percussion_coverage",),
        "sanitizers": ("pio test -e native_percussion_sanitized",),
        "avr": ("nanoatmega328new_percussion", "nanoatmega328_percussion"),
        "timing": ("nanoatmega328new_percussion_timing",),
    },
}



UNFILTERED_HEAVY_COMMANDS = (
    "pio test -e native_coverage",
    "pio test -e native_organic_coverage",
    "pio test -e native_generative_coverage",
    "pio test -e native_ambient_coverage",
    "pio test -e native_electronica_coverage",
    "pio test -e native_percussion_coverage",
    "pio test -e native_sanitized",
    "pio test -e native_organic_sanitized",
    "pio test -e native_generative_sanitized",
    "pio test -e native_ambient_sanitized",
    "pio test -e native_electronica_sanitized",
    "pio test -e native_percussion_sanitized",
)

REQUIRED_FILTERED_SUITES = (
    "-f unit/test_algorithms",
    "-f integration/test_selection",
    "-f integration/test_runtime",
    "-f property/test_invariants",
    "-f system/test_signal_path",
)

PERCUSSION_SUITES = (
    "unit/test_euclid_algorithm",
    "unit/test_repeat_algorithm",
    "unit/test_probability_algorithm",
    "unit/test_humanize_algorithm",
    "unit/test_percussion_clock",
    "integration/test_selection",
    "integration/test_runtime",
    "property/test_invariants",
    "system/test_signal_path",
)

ELECTRONICA_SUITES = (
    "unit/test_pump_algorithm",
    "unit/test_acid_algorithm",
    "unit/test_shuffle_algorithm",
    "unit/test_polymeter_algorithm",
    "integration/test_selection",
    "integration/test_runtime",
    "property/test_invariants",
    "system/test_signal_path",
)


def main() -> int:
    text = WORKFLOW.read_text(encoding="utf-8")
    missing: list[str] = []

    for bank, stages in BANKS.items():
        for stage, fragments in stages.items():
            absent = [fragment for fragment in fragments if fragment not in text]
            if absent:
                missing.append(f"{bank}/{stage}: {', '.join(absent)}")

    for suite in ELECTRONICA_SUITES:
        if suite not in text:
            missing.append(f"electronica/test suite: {suite}")

    if "native-electronica-tests:" not in text:
        missing.append("electronica/native job: native-electronica-tests")
    if "native-electronica-coverage:" not in text:
        missing.append("electronica/coverage job: native-electronica-coverage")
    if "native-electronica-sanitizers:" not in text:
        missing.append("electronica/sanitizer job: native-electronica-sanitizers")

    for suite in PERCUSSION_SUITES:
        if suite not in text:
            missing.append(f"percussion/test suite: {suite}")

    # Coverage and sanitizer jobs must never execute an entire 194-case
    # environment.  PlatformIO filters keep those expensive qualification
    # modes scoped to shared/core + active-bank suites.
    stripped_lines = {line.strip() for line in text.splitlines()}
    for command in UNFILTERED_HEAVY_COMMANDS:
        if command in stripped_lines or f"- run: {command}" in stripped_lines:
            missing.append(f"unfiltered heavy test command: {command}")

    for fragment in REQUIRED_FILTERED_SUITES:
        if fragment not in text:
            missing.append(f"filtered bank qualification fragment: {fragment}")

    if "native-percussion-tests:" not in text:
        missing.append("percussion/native job: native-percussion-tests")
    if "native-percussion-coverage:" not in text:
        missing.append("percussion/coverage job: native-percussion-coverage")
    if "native-percussion-sanitizers:" not in text:
        missing.append("percussion/sanitizer job: native-percussion-sanitizers")

    if missing:
        for item in missing:
            print(f"CI bank contract error: {item}")
        return 2

    print("CI six-bank build/qualification contract: passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
