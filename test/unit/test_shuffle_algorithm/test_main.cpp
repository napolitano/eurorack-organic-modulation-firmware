/**
 * @file test_main.cpp
 * Verifies the Electronica Shuffle mathematical and integration contracts.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <unity.h>

#include "fmd/domain/electronica/ElectronicaAlgorithmMath.h"
#include "fmd/domain/electronica/ShuffleAlgorithm.h"
#include "MemoryReferenceTables.h"

void test_shuffle_ratio_has_exact_straight_and_three_to_one_endpoints() {
  TEST_ASSERT_EQUAL_UINT16(fmd::shufflemath::kStraightRatioQ0F16,
                           fmd::shufflemath::secondOnsetRatioQ0F16(0U));
  TEST_ASSERT_EQUAL_UINT16(fmd::shufflemath::kMaximumRatioQ0F16,
                           fmd::shufflemath::secondOnsetRatioQ0F16(1023U));
  TEST_ASSERT_EQUAL_UINT32(0x80000000UL,
                           fmd::shufflemath::secondOnsetThreshold(32768U));
  TEST_ASSERT_EQUAL_UINT32(0xC0000000UL,
                           fmd::shufflemath::secondOnsetThreshold(49152U));
}

void test_shuffle_ratio_mapping_is_monotone() {
  uint16_t previous = fmd::shufflemath::secondOnsetRatioQ0F16(0U);
  for (uint16_t texture = 1U; texture <= 1023U; ++texture) {
    const uint16_t current = fmd::shufflemath::secondOnsetRatioQ0F16(texture);
    TEST_ASSERT_GREATER_OR_EQUAL_UINT16(previous, current);
    previous = current;
  }
}

void test_shuffle_intervals_always_sum_to_complete_pair() {
  for (uint16_t texture = 0U; texture <= 1023U; texture = static_cast<uint16_t>(texture + 31U)) {
    const uint16_t first = fmd::shufflemath::secondOnsetRatioQ0F16(texture);
    const uint16_t second = static_cast<uint16_t>(65535U - first + 1U);
    TEST_ASSERT_EQUAL_UINT32(65536UL, static_cast<uint32_t>(first) + second);
  }
}

void test_shuffle_short_decay_has_exact_endpoints() {
  TEST_ASSERT_EQUAL_UINT16(4095U, fmd::shufflemath::decayOutputDac12(0U));
  TEST_ASSERT_EQUAL_UINT16(0U, fmd::shufflemath::decayOutputDac12(0xFFFFFFFFUL));
}

void test_shuffle_algorithm_is_deterministic_and_bounded() {
  MemoryReferenceTables tables;
  fmd::ShuffleAlgorithm first(tables);
  fmd::ShuffleAlgorithm second(tables);
  for (uint16_t i = 0U; i < 8000U; ++i) {
    const fmd::ControlFrame controls{
        0U, static_cast<uint16_t>((i * 13U) & 1023U), 700U,
        static_cast<uint16_t>((i * 7U) & 1023U)};
    const uint16_t a = first.step(controls);
    const uint16_t b = second.step(controls);
    TEST_ASSERT_EQUAL_UINT16(a, b);
    TEST_ASSERT_LESS_OR_EQUAL_UINT16(4095U, a);
  }
}


void test_shuffle_threshold_clamps_ratio_outside_documented_range() {
  TEST_ASSERT_EQUAL_UINT32(
      fmd::shufflemath::secondOnsetThreshold(fmd::shufflemath::kStraightRatioQ0F16),
      fmd::shufflemath::secondOnsetThreshold(0U));
  TEST_ASSERT_EQUAL_UINT32(
      fmd::shufflemath::secondOnsetThreshold(fmd::shufflemath::kMaximumRatioQ0F16),
      fmd::shufflemath::secondOnsetThreshold(65535U));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_shuffle_ratio_has_exact_straight_and_three_to_one_endpoints);
  RUN_TEST(test_shuffle_ratio_mapping_is_monotone);
  RUN_TEST(test_shuffle_intervals_always_sum_to_complete_pair);
  RUN_TEST(test_shuffle_short_decay_has_exact_endpoints);
  RUN_TEST(test_shuffle_algorithm_is_deterministic_and_bounded);
  RUN_TEST(test_shuffle_threshold_clamps_ratio_outside_documented_range);
  return UNITY_END();
}
