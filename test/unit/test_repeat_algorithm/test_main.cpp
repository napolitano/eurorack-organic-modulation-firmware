/**
 * @file test_main.cpp
 * Verifies Percussion Repeat mathematical and integration contracts.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <unity.h>
#include "fmd/domain/percussion/PercussionAlgorithmMath.h"
#include "fmd/domain/percussion/RepeatAlgorithm.h"
#include "MemoryReferenceTables.h"

void test_repeat_probability_and_count_endpoints() {
  TEST_ASSERT_EQUAL_UINT32(0U, fmd::repeatmath::repeatCutoff(0U));
  TEST_ASSERT_EQUAL_UINT32(49152U, fmd::repeatmath::repeatCutoff(1023U));
  TEST_ASSERT_EQUAL_UINT8(2U, fmd::repeatmath::ratchetCount(0U));
  TEST_ASSERT_EQUAL_UINT8(4U, fmd::repeatmath::ratchetCount(1023U));
}

void test_ratchet_thresholds_are_evenly_inside_first_half_beat() {
  TEST_ASSERT_EQUAL_UINT32(0x20000000UL, fmd::repeatmath::subEventThreshold(1U, 4U));
  TEST_ASSERT_EQUAL_UINT32(0x40000000UL, fmd::repeatmath::subEventThreshold(2U, 4U));
  TEST_ASSERT_EQUAL_UINT32(0x60000000UL, fmd::repeatmath::subEventThreshold(3U, 4U));
}

void test_fill_forcing_matches_phrase_contract() {
  TEST_ASSERT_EQUAL_UINT8(2U, fmd::repeatmath::forcedMinimumCount(1U, 3U));
  TEST_ASSERT_EQUAL_UINT8(3U, fmd::repeatmath::forcedMinimumCount(2U, 3U));
  TEST_ASSERT_EQUAL_UINT8(4U, fmd::repeatmath::forcedMinimumCount(3U, 3U));
  TEST_ASSERT_EQUAL_UINT8(4U, fmd::repeatmath::forcedMinimumCount(4U, 2U));
  TEST_ASSERT_EQUAL_UINT8(4U, fmd::repeatmath::forcedMinimumCount(4U, 3U));
  TEST_ASSERT_EQUAL_UINT8(1U, fmd::repeatmath::forcedMinimumCount(3U, 2U));
}

void test_repeat_random_cutoff_is_monotone() {
  uint32_t previous = 0U;
  for (uint16_t t = 0U; t <= 1023U; ++t) {
    const uint32_t current = fmd::repeatmath::repeatCutoff(t);
    TEST_ASSERT_GREATER_OR_EQUAL_UINT32(previous, current);
    previous = current;
  }
}

void test_repeat_algorithm_is_reproducible_and_bounded() {
  MemoryReferenceTables tables;
  fmd::RepeatAlgorithm first(tables, 0x1234U);
  fmd::RepeatAlgorithm second(tables, 0x1234U);
  const fmd::ControlFrame controls{511U, 800U, 511U, 0U};
  for (uint16_t i = 0U; i < 6000U; ++i) {
    const uint16_t a = first.step(controls);
    TEST_ASSERT_EQUAL_UINT16(a, second.step(controls));
    TEST_ASSERT_TRUE(a == 0U || a == 4095U);
  }
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_repeat_probability_and_count_endpoints);
  RUN_TEST(test_ratchet_thresholds_are_evenly_inside_first_half_beat);
  RUN_TEST(test_fill_forcing_matches_phrase_contract);
  RUN_TEST(test_repeat_random_cutoff_is_monotone);
  RUN_TEST(test_repeat_algorithm_is_reproducible_and_bounded);
  return UNITY_END();
}
