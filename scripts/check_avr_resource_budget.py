#!/usr/bin/env python3
"""Enforce explicit ATmega328P flash and static-SRAM engineering budgets.

PlatformIO checks the absolute MCU limits. This adds deliberate headroom so
feature growth cannot silently consume the last usable bytes.

SPDX-License-Identifier: GPL-3.0-or-later
"""
from __future__ import annotations

import argparse
import shutil
import subprocess
import sys
from pathlib import Path

FLASH_CAPACITY = 30_720
SRAM_CAPACITY = 2_048
FLASH_BUDGET_PERCENT = 85.0
FLASH_BUDGET_BYTES = 26_112
SRAM_BUDGET_PERCENT = 65.0
SRAM_BUDGET_BYTES = 1_331


def within_budget(flash_bytes: int, sram_bytes: int) -> bool:
    """Return whether both measured resources stay inside engineering gates."""
    return flash_bytes <= FLASH_BUDGET_BYTES and sram_bytes <= SRAM_BUDGET_BYTES


def find_avr_size() -> str:
    direct = shutil.which("avr-size")
    if direct:
        return direct
    home = Path.home()
    for candidate in (
        home / ".platformio/packages/toolchain-atmelavr/bin/avr-size",
        home / ".platformio/packages/toolchain-atmelavr/bin/avr-size.exe",
    ):
        if candidate.exists():
            return str(candidate)
    raise FileNotFoundError("avr-size was not found; build an AVR environment first")


def read_sizes(tool: str, elf: Path) -> tuple[int, int, int]:
    result = subprocess.run(
        [tool, "--format=berkeley", str(elf)],
        check=True,
        capture_output=True,
        text=True,
    )
    lines = [line.strip() for line in result.stdout.splitlines() if line.strip()]
    if len(lines) < 2:
        raise RuntimeError(f"unexpected avr-size output:\n{result.stdout}")
    fields = lines[-1].split()
    if len(fields) < 3:
        raise RuntimeError(f"unexpected avr-size data row: {lines[-1]}")
    return int(fields[0]), int(fields[1]), int(fields[2])


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("elf", type=Path)
    args = parser.parse_args()
    if not args.elf.is_file():
        print(f"resource-budget: ELF not found: {args.elf}", file=sys.stderr)
        return 2
    try:
        tool = find_avr_size()
        text, data, bss = read_sizes(tool, args.elf)
    except (FileNotFoundError, RuntimeError, subprocess.CalledProcessError) as exc:
        print(f"resource-budget: {exc}", file=sys.stderr)
        return 2

    flash = text + data
    sram = data + bss
    flash_pct = 100.0 * flash / FLASH_CAPACITY
    sram_pct = 100.0 * sram / SRAM_CAPACITY
    print(
        f"resource-budget: flash {flash}/{FLASH_CAPACITY} bytes "
        f"({flash_pct:.1f}%, target <= {FLASH_BUDGET_PERCENT:.1f}% / {FLASH_BUDGET_BYTES} bytes)"
    )
    print(
        f"resource-budget: static SRAM {sram}/{SRAM_CAPACITY} bytes "
        f"({sram_pct:.1f}%, target <= {SRAM_BUDGET_PERCENT:.1f}% / {SRAM_BUDGET_BYTES} bytes)"
    )
    print(f"resource-budget: sections text={text}, data={data}, bss={bss}")

    failed = not within_budget(flash, sram)
    if flash > FLASH_BUDGET_BYTES:
        print("resource-budget: flash engineering budget exceeded", file=sys.stderr)
    if sram > SRAM_BUDGET_BYTES:
        print("resource-budget: static SRAM engineering budget exceeded", file=sys.stderr)
    return 1 if failed else 0


if __name__ == "__main__":
    raise SystemExit(main())
