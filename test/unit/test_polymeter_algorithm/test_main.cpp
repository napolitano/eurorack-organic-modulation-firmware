/**
 * @file test_main.cpp
 * Verifies the Electronica Polymeter mathematical and integration contracts.
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
#include "fmd/domain/electronica/PolymeterAlgorithm.h"
#include "MemoryReferenceTables.h"

void test_polymeter_texture_regions_select_exact_meter_lengths() {
  TEST_ASSERT_EQUAL_UINT8(3U, fmd::polymetermath::secondaryMeterLength(0U));
  TEST_ASSERT_EQUAL_UINT8(3U, fmd::polymetermath::secondaryMeterLength(255U));
  TEST_ASSERT_EQUAL_UINT8(5U, fmd::polymetermath::secondaryMeterLength(256U));
  TEST_ASSERT_EQUAL_UINT8(5U, fmd::polymetermath::secondaryMeterLength(511U));
  TEST_ASSERT_EQUAL_UINT8(7U, fmd::polymetermath::secondaryMeterLength(512U));
  TEST_ASSERT_EQUAL_UINT8(7U, fmd::polymetermath::secondaryMeterLength(767U));
  TEST_ASSERT_EQUAL_UINT8(9U, fmd::polymetermath::secondaryMeterLength(768U));
  TEST_ASSERT_EQUAL_UINT8(9U, fmd::polymetermath::secondaryMeterLength(1023U));
}

void test_polymeter_amplitude_cases_are_exact() {
  TEST_ASSERT_EQUAL_UINT16(1024U, fmd::polymetermath::stepAmplitudeDac12(false, false));
  TEST_ASSERT_EQUAL_UINT16(2559U, fmd::polymetermath::stepAmplitudeDac12(true, false));
  TEST_ASSERT_EQUAL_UINT16(2560U, fmd::polymetermath::stepAmplitudeDac12(false, true));
  TEST_ASSERT_EQUAL_UINT16(4095U, fmd::polymetermath::stepAmplitudeDac12(true, true));
}

void test_polymeter_recurrence_lengths_are_exact_lcms() {
  TEST_ASSERT_EQUAL_UINT8(12U, fmd::polymetermath::recurrenceSteps(3U));
  TEST_ASSERT_EQUAL_UINT8(20U, fmd::polymetermath::recurrenceSteps(5U));
  TEST_ASSERT_EQUAL_UINT8(28U, fmd::polymetermath::recurrenceSteps(7U));
  TEST_ASSERT_EQUAL_UINT8(36U, fmd::polymetermath::recurrenceSteps(9U));
}

void test_polymeter_decay_preserves_peak_and_reaches_zero() {
  TEST_ASSERT_EQUAL_UINT16(2559U,
      fmd::polymetermath::decayOutputDac12(0U, 2559U));
  TEST_ASSERT_EQUAL_UINT16(0U,
      fmd::polymetermath::decayOutputDac12(0xFFFFFFFFUL, 4095U));
}

void test_polymeter_algorithm_is_deterministic_and_bounded() {
  MemoryReferenceTables tables;
  fmd::PolymeterAlgorithm first(tables);
  fmd::PolymeterAlgorithm second(tables);
  for (uint16_t i = 0U; i < 10000U; ++i) {
    const fmd::ControlFrame controls{
        static_cast<uint16_t>((i * 3U) & 1023U),
        0U,
        650U,
        static_cast<uint16_t>((i / 500U) * 256U)};
    const uint16_t a = first.step(controls);
    const uint16_t b = second.step(controls);
    TEST_ASSERT_EQUAL_UINT16(a, b);
    TEST_ASSERT_LESS_OR_EQUAL_UINT16(4095U, a);
  }
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_polymeter_texture_regions_select_exact_meter_lengths);
  RUN_TEST(test_polymeter_amplitude_cases_are_exact);
  RUN_TEST(test_polymeter_recurrence_lengths_are_exact_lcms);
  RUN_TEST(test_polymeter_decay_preserves_peak_and_reaches_zero);
  RUN_TEST(test_polymeter_algorithm_is_deterministic_and_bounded);
  return UNITY_END();
}
