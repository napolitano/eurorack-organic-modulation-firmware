/**
 * @file test_main.cpp
 * Implements the frequency-mapping mathematical verification native test suite.
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

#include "fmd/domain/FrequencyMapping.h"
#include "MemoryReferenceTables.h"

namespace {
uint32_t exactPhaseIncrement(uint32_t rateQ16F16) {
  return static_cast<uint32_t>((static_cast<uint64_t>(rateQ16F16) * 65536ULL + 50000ULL) / 100000ULL);
}
}

void test_phase_increment_conversion_matches_exact_rational_reference() {
  MemoryReferenceTables tables;
  for (uint16_t i = 0U; i < 256U; ++i) {
    const uint32_t rate = tables.exp2Q16_16(static_cast<uint8_t>(i));
    TEST_ASSERT_EQUAL_UINT32(exactPhaseIncrement(rate),
                             fmd::phaseIncrementFromDecihertzQ16_16(rate));
  }
}

void test_phase_increment_reciprocal_correction_covers_known_low_estimate_case() {
  constexpr uint32_t rate = 3509983355UL;
  TEST_ASSERT_EQUAL_UINT32(exactPhaseIncrement(rate),
                           fmd::phaseIncrementFromDecihertzQ16_16(rate));
}

void test_phase_increment_conversion_matches_exact_reference_over_broad_uint32_sample() {
  uint32_t x = 0x12345678UL;
  for (uint32_t i = 0U; i < 100000U; ++i) {
    x = static_cast<uint32_t>(x * UINT32_C(1664525) + UINT32_C(1013904223));
    TEST_ASSERT_EQUAL_UINT32(exactPhaseIncrement(x),
                             fmd::phaseIncrementFromDecihertzQ16_16(x));
  }
  TEST_ASSERT_EQUAL_UINT32(exactPhaseIncrement(0U), fmd::phaseIncrementFromDecihertzQ16_16(0U));
  TEST_ASSERT_EQUAL_UINT32(exactPhaseIncrement(0xFFFFFFFFUL),
                           fmd::phaseIncrementFromDecihertzQ16_16(0xFFFFFFFFUL));
}

void test_delta_time_has_mathematically_correct_minimum_frequency_endpoint() {
  MemoryReferenceTables tables;
  // 1/40 Hz at 2.5 kHz => 2^32 / 100000 samples per cycle.
  TEST_ASSERT_EQUAL_UINT32(42950UL, fmd::phaseIncrementFromControls(tables, 0U, 0U, 0));
}

void test_delta_time_is_monotonic_for_knob() {
  MemoryReferenceTables tables;
  uint32_t previous = 0U;
  for (uint16_t knob = 0U; knob <= 1023U; ++knob) {
    const uint32_t current = fmd::phaseIncrementFromControls(tables, knob, 0U, 0);
    TEST_ASSERT_GREATER_OR_EQUAL_UINT32(previous, current);
    previous = current;
  }
}

void test_delta_time_is_monotonic_for_cv() {
  MemoryReferenceTables tables;
  uint32_t previous = 0U;
  for (uint16_t cv = 0U; cv <= 1023U; ++cv) {
    const uint32_t current = fmd::phaseIncrementFromControls(tables, 0U, cv, 0);
    TEST_ASSERT_GREATER_OR_EQUAL_UINT32(previous, current);
    previous = current;
  }
}

void test_positive_and_negative_offset_move_frequency_in_expected_direction() {
  MemoryReferenceTables tables;
  const uint32_t center = fmd::phaseIncrementFromControls(tables, 512U, 256U, 0);
  const uint32_t positive = fmd::phaseIncrementFromControls(tables, 512U, 256U, 8192);
  const uint32_t negative = fmd::phaseIncrementFromControls(tables, 512U, 256U, -8192);
  TEST_ASSERT_GREATER_THAN_UINT32(center, positive);
  TEST_ASSERT_LESS_THAN_UINT32(center, negative);
}


void test_delta_time_clamps_out_of_range_adc_inputs_to_hardware_domain() {
  MemoryReferenceTables tables;
  TEST_ASSERT_EQUAL_UINT32(fmd::phaseIncrementFromControls(tables, 1023U, 1023U, 0),
                           fmd::phaseIncrementFromControls(tables, 0xFFFFU, 0xFFFFU, 0));
  TEST_ASSERT_EQUAL_UINT32(fmd::phaseIncrementFromControls(tables, 1023U, 0U, 0),
                           fmd::phaseIncrementFromControls(tables, 0xFFFFU, 0U, 0));
  TEST_ASSERT_EQUAL_UINT32(fmd::phaseIncrementFromControls(tables, 0U, 1023U, 0),
                           fmd::phaseIncrementFromControls(tables, 0U, 0xFFFFU, 0));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_phase_increment_conversion_matches_exact_rational_reference);
  RUN_TEST(test_phase_increment_reciprocal_correction_covers_known_low_estimate_case);
  RUN_TEST(test_phase_increment_conversion_matches_exact_reference_over_broad_uint32_sample);
  RUN_TEST(test_delta_time_has_mathematically_correct_minimum_frequency_endpoint);
  RUN_TEST(test_delta_time_is_monotonic_for_knob);
  RUN_TEST(test_delta_time_is_monotonic_for_cv);
  RUN_TEST(test_positive_and_negative_offset_move_frequency_in_expected_direction);
  RUN_TEST(test_delta_time_clamps_out_of_range_adc_inputs_to_hardware_domain);
  return UNITY_END();
}
