/**
 * @file test_main.cpp
 * Implements the upstream-behaviour regression native test suite.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <unity.h>

#include <cstdint>

#include "fmd/domain/AlgorithmMath.h"
#include "fmd/domain/LfoAlgorithm.h"
#include "MemoryReferenceTables.h"

void test_brownian_texture_mapping_no_longer_has_1020_regime_switch() {
  const uint16_t a1019 = fmd::brownianmath::smoothingAlphaQ0F16(1019U);
  const uint16_t a1020 = fmd::brownianmath::smoothingAlphaQ0F16(1020U);
  const uint16_t a1021 = fmd::brownianmath::smoothingAlphaQ0F16(1021U);
  TEST_ASSERT_TRUE(a1019 < a1020);
  TEST_ASSERT_TRUE(a1020 <= a1021);
  TEST_ASSERT_UINT16_WITHIN(16U, a1019, a1020);
}

void test_bezier_icdf_final_interval_is_not_flat() {
  MemoryReferenceTables tables;
  const int16_t start = fmd::beziermath::triangularIcdfQ1F15(32640U, tables);
  const int16_t middle = fmd::beziermath::triangularIcdfQ1F15(32704U, tables);
  const int16_t end = fmd::beziermath::triangularIcdfQ1F15(32767U, tables);
  TEST_ASSERT_TRUE(start < middle);
  TEST_ASSERT_TRUE(middle < end);
}

void test_bezier_texture_has_no_hard_curve_family_jump_at_512() {
  for (uint16_t phase = 0U; phase <= 4096U; phase = static_cast<uint16_t>(phase + 32U)) {
    const uint16_t before = fmd::beziermath::morphCurveQ4F12(phase, 511U);
    const uint16_t after = fmd::beziermath::morphCurveQ4F12(phase, 512U);
    TEST_ASSERT_UINT16_WITHIN(3U, before, after);
  }
}

void test_lfo_texture_is_applied_from_first_cycle() {
  MemoryReferenceTables tables;
  fmd::LfoAlgorithm falling(tables);
  fmd::LfoAlgorithm rising(tables);
  const uint16_t firstFalling = falling.step({0U, 0U, 0U, 0U});
  const uint16_t firstRising = rising.step({0U, 0U, 0U, 1023U});
  TEST_ASSERT_GREATER_THAN_UINT16(4000U, firstFalling);
  TEST_ASSERT_LESS_THAN_UINT16(10U, firstRising);
}

void test_lfo_first_cycle_no_longer_contains_peak_narrowing_zero_spike() {
  MemoryReferenceTables tables;
  fmd::LfoAlgorithm lfo(tables);
  const fmd::ControlFrame controls{0U, 0U, 0U, 512U};
  uint16_t previous2 = 0U;
  uint16_t previous1 = 0U;
  bool observed = false;
  for (uint32_t i = 0U; i < 60000U; ++i) {
    const uint16_t current = lfo.step(controls);
    if (previous2 >= 4090U && previous1 == 0U && current >= 4090U) {
      observed = true;
      break;
    }
    previous2 = previous1;
    previous1 = current;
  }
  TEST_ASSERT_FALSE(observed);
}

void test_lfo_shape_change_is_output_continuous_at_slow_rate() {
  MemoryReferenceTables tables;
  fmd::LfoAlgorithm lfo(tables);
  fmd::ControlFrame controls{0U, 0U, 0U, 512U};
  uint16_t before = 0U;
  for (uint16_t i = 0U; i < 1000U; ++i) {
    before = lfo.step(controls);
  }
  controls.textureKnob = 128U;
  const uint16_t after = lfo.step(controls);
  const uint16_t delta = before > after
      ? static_cast<uint16_t>(before - after)
      : static_cast<uint16_t>(after - before);
  TEST_ASSERT_LESS_OR_EQUAL_UINT16(8U, delta);
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_brownian_texture_mapping_no_longer_has_1020_regime_switch);
  RUN_TEST(test_bezier_icdf_final_interval_is_not_flat);
  RUN_TEST(test_bezier_texture_has_no_hard_curve_family_jump_at_512);
  RUN_TEST(test_lfo_texture_is_applied_from_first_cycle);
  RUN_TEST(test_lfo_first_cycle_no_longer_contains_peak_narrowing_zero_spike);
  RUN_TEST(test_lfo_shape_change_is_output_continuous_at_slow_rate);
  return UNITY_END();
}
