/**
 * @file test_main.cpp
 * Verifies Dubstep/Bass Wobble mathematical and runtime contracts.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <unity.h>

#include "fmd/domain/FrequencyMapping.h"
#include "fmd/domain/dubstep/DubstepAlgorithmMath.h"
#include "fmd/domain/dubstep/WobbleAlgorithm.h"
#include "MemoryReferenceTables.h"

void test_dubstep_tempo_mapping_has_70_140_280_shape() {
  MemoryReferenceTables tables;
  const uint32_t minimum = fmd::dubstepmath::quarterNotePhaseIncrement(tables, 0U);
  const uint32_t center = fmd::dubstepmath::quarterNotePhaseIncrement(tables, 512U);
  const uint32_t maximum = fmd::dubstepmath::quarterNotePhaseIncrement(tables, 1023U);
  const uint32_t expectedMinimum = fmd::phaseIncrementFromDecihertzQ16_16(
      static_cast<uint32_t>((70ULL << 16U) / 6ULL));
  const uint32_t expectedMaximum = fmd::phaseIncrementFromDecihertzQ16_16(
      static_cast<uint32_t>((280ULL << 16U) / 6ULL));
  TEST_ASSERT_UINT32_WITHIN(2U, expectedMinimum, minimum);
  TEST_ASSERT_UINT32_WITHIN(2U, expectedMaximum, maximum);
  TEST_ASSERT_UINT32_WITHIN(center / 100U, minimum * 2UL, center);
}

void test_wobble_triangle_has_exact_cardinal_points() {
  TEST_ASSERT_EQUAL_UINT16(0U, fmd::dubstepmath::triangleQ0F12(0U));
  TEST_ASSERT_EQUAL_UINT16(2048U, fmd::dubstepmath::triangleQ0F12(UINT32_C(0x40000000)));
  TEST_ASSERT_EQUAL_UINT16(4096U, fmd::dubstepmath::triangleQ0F12(UINT32_C(0x80000000)));
  TEST_ASSERT_EQUAL_UINT16(2048U, fmd::dubstepmath::triangleQ0F12(UINT32_C(0xC0000000)));
}

void test_wobble_phrase_and_texture_vocabularies_are_exact() {
  constexpr uint8_t expectedPhrase[8] = {0U, 1U, 0U, 2U, 1U, 3U, 0U, 2U};
  constexpr uint8_t expectedRates[4][4] = {
      {2U, 2U, 2U, 2U}, {2U, 5U, 2U, 5U},
      {2U, 3U, 5U, 4U}, {2U, 5U, 6U, 7U}};
  for (uint8_t cell = 0U; cell < 8U; ++cell) {
    TEST_ASSERT_EQUAL_UINT8(expectedPhrase[cell], fmd::wobblemath::phraseSymbol(cell));
  }
  for (uint8_t region = 0U; region < 4U; ++region) {
    for (uint8_t symbol = 0U; symbol < 4U; ++symbol) {
      TEST_ASSERT_EQUAL_UINT8(expectedRates[region][symbol],
                              fmd::wobblemath::rateCode(region, symbol));
    }
  }
  TEST_ASSERT_EQUAL_UINT8(7U, fmd::wobblemath::rateCode(255U, 3U));
}

void test_wobble_rate_codes_match_documented_rational_ratios() {
  constexpr uint32_t q = 1200000UL;
  TEST_ASSERT_EQUAL_UINT32(600000UL, fmd::wobblemath::carrierIncrement(q, 0U));
  TEST_ASSERT_EQUAL_UINT32(800000UL, fmd::wobblemath::carrierIncrement(q, 1U));
  TEST_ASSERT_EQUAL_UINT32(1200000UL, fmd::wobblemath::carrierIncrement(q, 2U));
  TEST_ASSERT_EQUAL_UINT32(1600000UL, fmd::wobblemath::carrierIncrement(q, 3U));
  TEST_ASSERT_EQUAL_UINT32(1800000UL, fmd::wobblemath::carrierIncrement(q, 4U));
  TEST_ASSERT_EQUAL_UINT32(2400000UL, fmd::wobblemath::carrierIncrement(q, 5U));
  TEST_ASSERT_EQUAL_UINT32(3600000UL, fmd::wobblemath::carrierIncrement(q, 6U));
  TEST_ASSERT_EQUAL_UINT32(4800000UL, fmd::wobblemath::carrierIncrement(q, 7U));
}

void test_wobble_algorithm_is_deterministic_and_bounded() {
  MemoryReferenceTables tables;
  fmd::WobbleAlgorithm first(tables);
  fmd::WobbleAlgorithm second(tables);
  for (uint32_t i = 0U; i < 6000U; ++i) {
    const fmd::ControlFrame controls{
        static_cast<uint16_t>((i * 17U) & 1023U),
        static_cast<uint16_t>((i * 29U) & 1023U),
        static_cast<uint16_t>((i * 43U) & 1023U),
        static_cast<uint16_t>((i * 61U) & 1023U)};
    const uint16_t a = first.step(controls);
    const uint16_t b = second.step(controls);
    TEST_ASSERT_EQUAL_UINT16(a, b);
    TEST_ASSERT_LESS_OR_EQUAL_UINT16(4095U, a);
  }
}

void test_wobble_steady_speed_cv_dc_does_not_change_internal_tempo() {
  MemoryReferenceTables tables;
  fmd::WobbleAlgorithm low(tables);
  fmd::WobbleAlgorithm high(tables);
  const fmd::ControlFrame lowControls{0U, 0U, 512U, 700U};
  const fmd::ControlFrame highControls{1023U, 0U, 512U, 700U};
  for (uint16_t i = 0U; i < 3000U; ++i) {
    TEST_ASSERT_EQUAL_UINT16(low.step(lowControls), high.step(highControls));
  }
}

void test_wobble_external_acquisition_establishes_zero_phase_origin() {
  MemoryReferenceTables tables;
  fmd::WobbleAlgorithm algorithm(tables);
  fmd::ControlFrame controls{0U, 0U, 0U, 0U};
  for (uint8_t i = 0U; i < 20U; ++i) algorithm.step(controls);
  controls.speedCv = 600U;
  algorithm.step(controls);
  controls.speedCv = 0U;
  for (uint8_t i = 1U; i < 100U; ++i) algorithm.step(controls);
  controls.speedCv = 600U;
  TEST_ASSERT_EQUAL_UINT16(0U, algorithm.step(controls));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_dubstep_tempo_mapping_has_70_140_280_shape);
  RUN_TEST(test_wobble_triangle_has_exact_cardinal_points);
  RUN_TEST(test_wobble_phrase_and_texture_vocabularies_are_exact);
  RUN_TEST(test_wobble_rate_codes_match_documented_rational_ratios);
  RUN_TEST(test_wobble_algorithm_is_deterministic_and_bounded);
  RUN_TEST(test_wobble_steady_speed_cv_dc_does_not_change_internal_tempo);
  RUN_TEST(test_wobble_external_acquisition_establishes_zero_phase_origin);
  return UNITY_END();
}
