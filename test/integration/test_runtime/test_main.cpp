/**
 * @file test_main.cpp
 * Implements the portable runtime integration native test suite.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <unity.h>

#include "fmd/application/DriftRuntime.h"
#include "DriftTestRig.h"

void test_runtime_queues_dac_and_led_together() {
  DriftTestRig rig(fmd::Algorithm::Lfo, 1U);
  const uint16_t outputCode = rig.tick(0U, 0U, 512U, 512U);

  TEST_ASSERT_EQUAL_UINT16(outputCode, rig.runtime.lastOutputCode());
  TEST_ASSERT_EQUAL_UINT8(
      rig.referenceTables.gamma8(static_cast<uint8_t>(outputCode >> 4U)),
      rig.ledOutput.duty);
  TEST_ASSERT_FALSE(rig.dacOutput.isReady);
}

void test_runtime_does_not_advance_while_dac_busy() {
  DriftTestRig rig(fmd::Algorithm::Lfo, 1U);
  rig.tick(0U, 0U, 512U, 512U);
  const uint16_t firstOutputCode = rig.runtime.lastOutputCode();

  TEST_ASSERT_FALSE(rig.runtime.processNextSampleIfReady());
  TEST_ASSERT_EQUAL_UINT16(firstOutputCode, rig.runtime.lastOutputCode());
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_runtime_queues_dac_and_led_together);
  RUN_TEST(test_runtime_does_not_advance_while_dac_busy);
  return UNITY_END();
}
