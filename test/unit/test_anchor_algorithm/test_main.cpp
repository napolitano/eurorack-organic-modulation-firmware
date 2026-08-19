/**
 * @file test_main.cpp
 * Verifies the bounded mean-reversion contract of Ambient Anchor.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <unity.h>

#include "MemoryReferenceTables.h"
#include "fmd/domain/ambient/AmbientAlgorithmMath.h"
#include "fmd/domain/ambient/AnchorAlgorithm.h"

void test_anchor_spread_has_exact_zero_and_full_endpoints() {
  TEST_ASSERT_EQUAL_UINT16(0U, fmd::anchormath::spreadQ1F15(0U));
  TEST_ASSERT_EQUAL_UINT16(
      fmd::anchormath::kMaximumSpreadQ1F15,
      fmd::anchormath::spreadQ1F15(1023U));

  uint16_t previous = 0U;
  for (uint16_t texture = 0U; texture <= 1023U; ++texture) {
    const uint16_t current = fmd::anchormath::spreadQ1F15(texture);
    TEST_ASSERT_GREATER_OR_EQUAL_UINT16(previous, current);
    previous = current;
  }
}

void test_anchor_reversion_moves_both_signs_toward_zero() {
  uint32_t residual = 0U;
  int8_t direction = 0;
  const int16_t positive =
      fmd::anchormath::revertTowardZero(20000, 40000U, residual, direction);
  TEST_ASSERT_LESS_THAN_INT16(20000, positive);
  TEST_ASSERT_GREATER_THAN_INT16(0, positive);

  residual = 0U;
  direction = 0;
  const int16_t negative =
      fmd::anchormath::revertTowardZero(-20000, 40000U, residual, direction);
  TEST_ASSERT_GREATER_THAN_INT16(-20000, negative);
  TEST_ASSERT_LESS_THAN_INT16(0, negative);
}

void test_anchor_fractional_reversion_eventually_moves_one_code() {
  uint32_t residual = 0U;
  int8_t direction = 0;
  int16_t state = 1;
  for (uint32_t sample = 0U; sample < 2000000U && state != 0; ++sample) {
    state = fmd::anchormath::revertTowardZero(
        state, 10U, residual, direction);
  }
  TEST_ASSERT_EQUAL_INT16(0, state);
}

void test_anchor_zero_texture_stays_exactly_at_midpoint() {
  MemoryReferenceTables tables;
  fmd::AnchorAlgorithm algorithm(tables, 0x1234U);
  const fmd::ControlFrame controls{0U, 0U, 0U, 0U};
  for (uint32_t sample = 0U; sample < 10000U; ++sample) {
    TEST_ASSERT_EQUAL_UINT16(2048U, algorithm.step(controls));
  }
}

void test_anchor_is_seed_deterministic_and_bounded() {
  MemoryReferenceTables tables;
  fmd::AnchorAlgorithm first(tables, 0x7777U);
  fmd::AnchorAlgorithm second(tables, 0x7777U);
  const fmd::ControlFrame controls{500U, 600U, 700U, 800U};
  for (uint32_t sample = 0U; sample < 30000U; ++sample) {
    const uint16_t firstValue = first.step(controls);
    const uint16_t secondValue = second.step(controls);
    TEST_ASSERT_EQUAL_UINT16(firstValue, secondValue);
    TEST_ASSERT_LESS_OR_EQUAL_UINT16(4095U, firstValue);
  }
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_anchor_spread_has_exact_zero_and_full_endpoints);
  RUN_TEST(test_anchor_reversion_moves_both_signs_toward_zero);
  RUN_TEST(test_anchor_fractional_reversion_eventually_moves_one_code);
  RUN_TEST(test_anchor_zero_texture_stays_exactly_at_midpoint);
  RUN_TEST(test_anchor_is_seed_deterministic_and_bounded);
  return UNITY_END();
}
