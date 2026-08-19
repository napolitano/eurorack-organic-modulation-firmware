/**
 * @file test_main.cpp
 * Implements the cross-algorithm smoke native test suite.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <unity.h>

#include "fmd/domain/DriftEngine.h"
#include "MemoryReferenceTables.h"

void test_each_algorithm_stays_in_dac_range() {
  MemoryReferenceTables referenceTables;
  const fmd::ControlFrame maximumControls{1023U, 1023U, 1023U, 1023U};

  for (uint8_t slotIndex = 0U; slotIndex < 4U; ++slotIndex) {
    fmd::DriftEngine engine(
        fmd::algorithmForBankSlot(slotIndex), 0x4A51U, referenceTables);
    for (uint16_t sampleIndex = 0U; sampleIndex < 2000U; ++sampleIndex) {
      TEST_ASSERT_LESS_OR_EQUAL_UINT16(4095U, engine.step(maximumControls));
    }
  }
}

void test_algorithms_are_deterministic_for_fixed_seed() {
  MemoryReferenceTables referenceTables;
  const fmd::ControlFrame controls{222U, 333U, 444U, 555U};

  for (uint8_t slotIndex = 0U; slotIndex < 4U; ++slotIndex) {
    const fmd::Algorithm algorithm = fmd::algorithmForBankSlot(slotIndex);
    fmd::DriftEngine firstEngine(algorithm, 0x1234U, referenceTables);
    fmd::DriftEngine secondEngine(algorithm, 0x1234U, referenceTables);

    for (uint16_t sampleIndex = 0U; sampleIndex < 300U; ++sampleIndex) {
      TEST_ASSERT_EQUAL_UINT16(firstEngine.step(controls), secondEngine.step(controls));
    }
  }
}

void test_invalid_algorithm_enum_returns_safe_zero_output() {
  MemoryReferenceTables referenceTables;
  fmd::DriftEngine engine(
      static_cast<fmd::Algorithm>(0xFFU), 0x1234U, referenceTables);

  TEST_ASSERT_EQUAL_UINT16(0U, engine.step({0U, 0U, 0U, 0U}));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_each_algorithm_stays_in_dac_range);
  RUN_TEST(test_algorithms_are_deterministic_for_fixed_seed);
  RUN_TEST(test_invalid_algorithm_enum_returns_safe_zero_output);
  return UNITY_END();
}
