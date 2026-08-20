/**
 * @file test_main.cpp
 * Verifies Dubstep/Bass Growl mathematical and runtime contracts.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <unity.h>

#include "fmd/domain/dubstep/DubstepAlgorithmMath.h"
#include "fmd/domain/dubstep/GrowlAlgorithm.h"
#include "MemoryReferenceTables.h"

void test_growl_texture_zero_reduces_exactly_to_fundamental_triangle() {
  const auto weights = fmd::growlmath::normalizedWeights(0U);
  TEST_ASSERT_EQUAL_UINT16(4096U, weights.fundamental);
  TEST_ASSERT_EQUAL_UINT16(0U, weights.second);
  TEST_ASSERT_EQUAL_UINT16(0U, weights.third);
  const uint32_t phases[] = {0U, UINT32_C(0x20000000), UINT32_C(0x80000000), UINT32_C(0xE0000000)};
  for (uint32_t phase : phases) {
    TEST_ASSERT_EQUAL_UINT16(
        fmd::dubstepmath::q0F12ToDac12(fmd::dubstepmath::triangleQ0F12(phase)),
        fmd::growlmath::outputDac12(phase, weights));
  }
}

void test_growl_normalized_weights_sum_to_unity_and_track_texture() {
  uint16_t previousSecond = 0U;
  uint16_t previousThird = 0U;
  for (uint16_t texture = 0U; texture <= 1023U; ++texture) {
    const auto weights = fmd::growlmath::normalizedWeights(texture);
    TEST_ASSERT_EQUAL_UINT16(4096U,
        static_cast<uint16_t>(weights.fundamental + weights.second + weights.third));
    TEST_ASSERT_GREATER_OR_EQUAL_UINT16(previousSecond, weights.second);
    // Independent fixed-point normalization can move by one LSB when the
    // denominator changes on a quantized Texture step.  Larger reversals
    // would indicate a real shape-mapping regression.
    TEST_ASSERT_GREATER_OR_EQUAL_UINT16(previousThird, static_cast<uint16_t>(weights.third + 1U));
    previousSecond = weights.second;
    previousThird = weights.third;
  }
  const auto maximum = fmd::growlmath::normalizedWeights(1023U);
  TEST_ASSERT_GREATER_OR_EQUAL_UINT16(1000U, maximum.second);
  TEST_ASSERT_GREATER_OR_EQUAL_UINT16(600U, maximum.third);
}

void test_growl_output_is_bounded_over_dense_phase_and_texture_grid() {
  for (uint16_t texture = 0U; texture <= 1023U; texture += 73U) {
    const auto weights = fmd::growlmath::normalizedWeights(texture);
    for (uint32_t phase = 0U; phase < UINT32_MAX - UINT32_C(0x01000000); phase += UINT32_C(0x01000000)) {
      TEST_ASSERT_LESS_OR_EQUAL_UINT16(4095U, fmd::growlmath::outputDac12(phase, weights));
    }
  }
}

void test_growl_higher_texture_changes_compound_shape() {
  const auto low = fmd::growlmath::normalizedWeights(0U);
  const auto high = fmd::growlmath::normalizedWeights(1023U);
  TEST_ASSERT_TRUE(high.second > 0U);
  TEST_ASSERT_TRUE(high.third > 0U);
  TEST_ASSERT_NOT_EQUAL(
      fmd::growlmath::outputDac12(UINT32_C(0x20000000), low),
      fmd::growlmath::outputDac12(UINT32_C(0x20000000), high));
}

void test_growl_algorithm_is_deterministic_and_bounded() {
  MemoryReferenceTables tables;
  fmd::GrowlAlgorithm first(tables);
  fmd::GrowlAlgorithm second(tables);
  for (uint32_t i = 0U; i < 5000U; ++i) {
    const fmd::ControlFrame controls{
        static_cast<uint16_t>((i * 13U) & 1023U),
        static_cast<uint16_t>((i * 31U) & 1023U),
        static_cast<uint16_t>((i * 47U) & 1023U),
        static_cast<uint16_t>((i * 71U) & 1023U)};
    const uint16_t a = first.step(controls);
    const uint16_t b = second.step(controls);
    TEST_ASSERT_EQUAL_UINT16(a, b);
    TEST_ASSERT_LESS_OR_EQUAL_UINT16(4095U, a);
  }
}

void test_growl_external_clock_relock_has_deterministic_origin() {
  MemoryReferenceTables tables;
  fmd::GrowlAlgorithm first(tables);
  fmd::GrowlAlgorithm second(tables);
  fmd::ControlFrame controls{0U, 0U, 0U, 700U};
  for (uint8_t i = 0U; i < 20U; ++i) {
    first.step(controls);
    second.step(controls);
  }
  controls.speedCv = 600U;
  first.step(controls); second.step(controls);
  controls.speedCv = 0U;
  for (uint8_t i = 1U; i < 100U; ++i) { first.step(controls); second.step(controls); }
  controls.speedCv = 600U;
  TEST_ASSERT_EQUAL_UINT16(first.step(controls), second.step(controls));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_growl_texture_zero_reduces_exactly_to_fundamental_triangle);
  RUN_TEST(test_growl_normalized_weights_sum_to_unity_and_track_texture);
  RUN_TEST(test_growl_output_is_bounded_over_dense_phase_and_texture_grid);
  RUN_TEST(test_growl_higher_texture_changes_compound_shape);
  RUN_TEST(test_growl_algorithm_is_deterministic_and_bounded);
  RUN_TEST(test_growl_external_clock_relock_has_deterministic_origin);
  return UNITY_END();
}
