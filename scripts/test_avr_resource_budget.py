#!/usr/bin/env python3
"""Self-test the ATmega328P engineering resource-budget constants.

SPDX-License-Identifier: GPL-3.0-or-later
"""
from __future__ import annotations

from check_avr_resource_budget import (
    FLASH_BUDGET_BYTES,
    FLASH_BUDGET_PERCENT,
    FLASH_CAPACITY,
    SRAM_BUDGET_BYTES,
    SRAM_BUDGET_PERCENT,
    SRAM_CAPACITY,
    within_budget,
)


def main() -> int:
    assert FLASH_CAPACITY == 30_720
    assert SRAM_CAPACITY == 2_048
    assert FLASH_BUDGET_PERCENT == 85.0
    assert FLASH_BUDGET_BYTES == 26_112
    assert FLASH_BUDGET_BYTES == int(FLASH_CAPACITY * FLASH_BUDGET_PERCENT / 100.0)
    assert SRAM_BUDGET_PERCENT == 65.0
    # 65% of 2048 is 1331.2; integer bytes deliberately round down.
    assert SRAM_BUDGET_BYTES == 1_331
    assert SRAM_BUDGET_BYTES <= SRAM_CAPACITY * SRAM_BUDGET_PERCENT / 100.0
    assert SRAM_BUDGET_BYTES + 1 > SRAM_CAPACITY * SRAM_BUDGET_PERCENT / 100.0
    assert within_budget(FLASH_BUDGET_BYTES, SRAM_BUDGET_BYTES)
    assert not within_budget(FLASH_BUDGET_BYTES + 1, SRAM_BUDGET_BYTES)
    assert not within_budget(FLASH_BUDGET_BYTES, SRAM_BUDGET_BYTES + 1)
    assert not within_budget(FLASH_BUDGET_BYTES + 1, SRAM_BUDGET_BYTES + 1)
    print("avr-resource-budget-tests: passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
