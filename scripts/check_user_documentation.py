#!/usr/bin/env python3
"""Validate the user-facing README and firmware-installation documentation.

SPDX-License-Identifier: GPL-3.0-or-later
"""
from __future__ import annotations

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
README = ROOT / "README.md"
INSTALL = ROOT / "docs" / "installation" / "README.md"
AVR = ROOT / "docs" / "installation" / "avrdudess" / "README.md"
SHOT = ROOT / "docs" / "assets" / "installation" / "avrdudess-2.20-windows-numbered.png"

BANKS = ("Classic", "Organic", "Generative", "Ambient", "Electronica", "Percussion", "Dubstep / Bass")
ALGORITHMS = (
    "Perlin", "Brownian", "Bézier", "LFO",
    "Fractal", "Vector", "Rain", "Attractor",
    "Turing", "Markov", "Motif", "Urn",
    "Current", "Anchor", "Breath", "Fog",
    "Pump", "Acid", "Shuffle", "Polymeter",
    "Euclid", "Repeat", "Probability", "Humanize",
    "Wobble", "Growl", "Chop", "Build",
)


def require(text: str, needle: str, label: str, errors: list[str]) -> None:
    if needle not in text:
        errors.append(f"{label}: missing required text: {needle!r}")


def main() -> int:
    errors: list[str] = []
    for path in (README, INSTALL, AVR, SHOT):
        if not path.is_file():
            errors.append(f"missing user-documentation artifact: {path.relative_to(ROOT)}")

    if errors:
        print("user-documentation validation failed:")
        for error in errors:
            print(f"- {error}")
        return 1

    readme = README.read_text(encoding="utf-8")
    install = INSTALL.read_text(encoding="utf-8")
    avr = AVR.read_text(encoding="utf-8")

    require(readme, "docs/installation/README.md", "README.md", errors)
    require(readme, "28 algorithms", "README.md", errors)
    require(readme, "Seven banks", "README.md", errors)
    require(readme, "Do not power the module from USB and the Eurorack PSU at the same time", "README.md", errors)
    require(readme, "python scripts/flash_drift.py algorithm breath", "README.md named target", errors)
    require(readme, "FMD_FORCE_ALGORITHM=breath", "README.md named target", errors)
    require(readme, "rear DIP switches are ignored", "README.md named target", errors)
    require(install, "DISCONNECT THE EURORACK RIBBON CABLE BEFORE CONNECTING USB", "installation guide", errors)
    require(install, "Flashing selects the bank", "installation guide", errors)
    require(avr, "AVRDUDESS 2.20", "AVRDUDESS guide", errors)
    require(avr, "Erase flash and EEPROM (-e)` must be OFF", "AVRDUDESS guide", errors)
    require(avr, "avrdudess-2.20-windows-numbered.png", "AVRDUDESS guide", errors)

    combined = "\n".join((readme, install, avr))
    for bank in BANKS:
        if bank not in combined:
            errors.append(f"user docs: bank {bank!r} is not documented")
    for algorithm in ALGORITHMS:
        if algorithm not in combined:
            errors.append(f"user docs: algorithm {algorithm!r} is not documented")

    # The old eight-column table rendered poorly in the main README. Keep the
    # user-facing overview split into a four-slot table plus a bank/slot table.
    legacy_headers = (
        "| Rear DIP 1 | Rear DIP 2 | Classic | Organic | Generative | Ambient | Electronica | Percussion |",
        "| Rear DIP 1 | Rear DIP 2 | Classic | Organic | Generative | Ambient | Electronica | Percussion | Dubstep / Bass |",
    )
    if any(header in readme for header in legacy_headers):
        errors.append("README.md: legacy cross-bank DIP truth table must not be reintroduced")

    if SHOT.stat().st_size < 10_000:
        errors.append("AVRDUDESS screenshot appears truncated or invalid")

    if errors:
        print("user-documentation validation failed:")
        for error in errors:
            print(f"- {error}")
        return 1

    print("user-documentation validation passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
