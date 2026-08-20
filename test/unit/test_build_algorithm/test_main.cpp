/**
 * @file test_main.cpp
 * Verifies Dubstep/Bass Build mathematical and runtime contracts.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <unity.h>

#include "fmd/domain/dubstep/BuildAlgorithm.h"
#include "fmd/domain/dubstep/DubstepAlgorithmMath.h"
#include "MemoryReferenceTables.h"

void test_build_texture_regions_map_to_8_4_2_1_bars() {
  TEST_ASSERT_EQUAL_UINT8(8U, fmd::buildmath::phraseLengthBars(0U));
  TEST_ASSERT_EQUAL_UINT8(8U, fmd::buildmath::phraseLengthBars(255U));
  TEST_ASSERT_EQUAL_UINT8(4U, fmd::buildmath::phraseLengthBars(256U));
  TEST_ASSERT_EQUAL_UINT8(2U, fmd::buildmath::phraseLengthBars(512U));
  TEST_ASSERT_EQUAL_UINT8(1U, fmd::buildmath::phraseLengthBars(768U));
  TEST_ASSERT_EQUAL_UINT8(1U, fmd::buildmath::phraseLengthBars(1023U));
  TEST_ASSERT_EQUAL_UINT8(5U, fmd::buildmath::phraseQuarterShift(8U));
  TEST_ASSERT_EQUAL_UINT8(4U, fmd::buildmath::phraseQuarterShift(4U));
  TEST_ASSERT_EQUAL_UINT8(3U, fmd::buildmath::phraseQuarterShift(2U));
  TEST_ASSERT_EQUAL_UINT8(2U, fmd::buildmath::phraseQuarterShift(1U));
}

void test_build_macro_rise_is_monotone() {
  uint16_t previous = 0U;
  for (uint32_t phase = 0U; phase < UINT32_MAX - UINT32_C(0x00100000); phase += UINT32_C(0x00100000)) {
    const uint16_t current = fmd::buildmath::macroRiseQ0F12(phase);
    TEST_ASSERT_GREATER_OR_EQUAL_UINT16(previous, current);
    previous = current;
  }
}

void test_build_micro_stages_have_exact_quarter_boundaries_and_rates() {
  TEST_ASSERT_EQUAL_UINT8(0U, fmd::buildmath::microRateStage(0U));
  TEST_ASSERT_EQUAL_UINT8(1U, fmd::buildmath::microRateStage(UINT32_C(0x40000000)));
  TEST_ASSERT_EQUAL_UINT8(2U, fmd::buildmath::microRateStage(UINT32_C(0x80000000)));
  TEST_ASSERT_EQUAL_UINT8(3U, fmd::buildmath::microRateStage(UINT32_C(0xC0000000)));
  constexpr uint32_t q = 1234567UL;
  TEST_ASSERT_EQUAL_UINT32(q, fmd::buildmath::microPhaseIncrement(q, 0U));
  TEST_ASSERT_EQUAL_UINT32(q * 2UL, fmd::buildmath::microPhaseIncrement(q, 1U));
  TEST_ASSERT_EQUAL_UINT32(q * 4UL, fmd::buildmath::microPhaseIncrement(q, 2U));
  TEST_ASSERT_EQUAL_UINT32(q * 8UL, fmd::buildmath::microPhaseIncrement(q, 3U));
  TEST_ASSERT_EQUAL_UINT32(q * 8UL, fmd::buildmath::microPhaseIncrement(q, 255U));
}

void test_build_output_starts_at_zero_and_remains_bounded() {
  TEST_ASSERT_EQUAL_UINT16(0U, fmd::buildmath::outputDac12(0U, 0U));
  for (uint32_t phrase = 0U; phrase < UINT32_MAX - UINT32_C(0x02000000); phrase += UINT32_C(0x02000000)) {
    for (uint32_t micro = 0U; micro < UINT32_MAX - UINT32_C(0x10000000); micro += UINT32_C(0x10000000)) {
      TEST_ASSERT_LESS_OR_EQUAL_UINT16(4095U, fmd::buildmath::outputDac12(phrase, micro));
    }
  }
}

void test_build_algorithm_is_deterministic_and_bounded() {
  MemoryReferenceTables tables;
  fmd::BuildAlgorithm first(tables);
  fmd::BuildAlgorithm second(tables);
  const fmd::ControlFrame controls{0U, 200U, 1023U, 700U};
  for (uint32_t i = 0U; i < 12000U; ++i) {
    const uint16_t a = first.step(controls);
    const uint16_t b = second.step(controls);
    TEST_ASSERT_EQUAL_UINT16(a, b);
    TEST_ASSERT_LESS_OR_EQUAL_UINT16(4095U, a);
  }
}

void test_build_external_acquisition_restarts_phrase_at_zero() {
  MemoryReferenceTables tables;
  fmd::BuildAlgorithm algorithm(tables);
  fmd::ControlFrame controls{0U, 0U, 1023U, 1023U};
  for (uint16_t i = 0U; i < 500U; ++i) algorithm.step(controls);
  controls.speedCv = 600U;
  algorithm.step(controls);
  controls.speedCv = 0U;
  for (uint8_t i = 1U; i < 100U; ++i) algorithm.step(controls);
  controls.speedCv = 600U;
  TEST_ASSERT_EQUAL_UINT16(0U, algorithm.step(controls));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_build_texture_regions_map_to_8_4_2_1_bars);
  RUN_TEST(test_build_macro_rise_is_monotone);
  RUN_TEST(test_build_micro_stages_have_exact_quarter_boundaries_and_rates);
  RUN_TEST(test_build_output_starts_at_zero_and_remains_bounded);
  RUN_TEST(test_build_algorithm_is_deterministic_and_bounded);
  RUN_TEST(test_build_external_acquisition_restarts_phrase_at_zero);
  return UNITY_END();
}
