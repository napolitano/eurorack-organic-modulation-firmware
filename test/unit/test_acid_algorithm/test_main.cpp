/**
 * @file test_main.cpp
 * Verifies the Electronica Acid mathematical and integration contracts.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <unity.h>

#include "fmd/domain/electronica/AcidAlgorithm.h"
#include "fmd/domain/electronica/ElectronicaAlgorithmMath.h"
#include "MemoryReferenceTables.h"

void test_acid_permutation_visits_all_sixteen_codes_exactly_once() {
  bool seen[16] = {};
  for (uint8_t step = 0U; step < 16U; ++step) {
    const uint8_t code = fmd::acidmath::permutationCode(step);
    TEST_ASSERT_LESS_THAN_UINT8(16U, code);
    TEST_ASSERT_FALSE(seen[code]);
    seen[code] = true;
    const uint16_t target = fmd::acidmath::baseTargetDac12(step);
    TEST_ASSERT_GREATER_OR_EQUAL_UINT16(1024U, target);
    TEST_ASSERT_LESS_OR_EQUAL_UINT16(2944U, target);
  }
}

void test_acid_masks_match_documented_predicates() {
  for (uint8_t step = 0U; step < 16U; ++step) {
    const bool expectedAccent = (step % 4U) == 0U || (step % 7U) == 0U;
    const bool expectedSlide = static_cast<uint8_t>((5U * step) & 0x0FU) < 4U;
    TEST_ASSERT_EQUAL(expectedAccent, fmd::acidmath::isAccentStep(step));
    TEST_ASSERT_EQUAL(expectedSlide, fmd::acidmath::isSlideStep(step));
  }
}

void test_acid_slide_has_exact_texture_and_phase_endpoints() {
  constexpr uint16_t previous = 1024U;
  constexpr uint16_t current = 2944U;
  TEST_ASSERT_EQUAL_UINT16(current,
      fmd::acidmath::slideContourDac12(previous, current, 0U, 0U));
  TEST_ASSERT_EQUAL_UINT16(previous,
      fmd::acidmath::slideContourDac12(previous, current, 0U, 4096U));
  TEST_ASSERT_EQUAL_UINT16(current,
      fmd::acidmath::slideContourDac12(previous, current, 4096U, 4096U));
}

void test_acid_accent_has_exact_zero_full_and_monotone_decay() {
  TEST_ASSERT_EQUAL_UINT16(0U, fmd::acidmath::accentContributionDac12(0U, 0U));
  TEST_ASSERT_EQUAL_UINT16(768U, fmd::acidmath::accentContributionDac12(0U, 4096U));
  TEST_ASSERT_EQUAL_UINT16(0U, fmd::acidmath::accentContributionDac12(4096U, 4096U));
  uint16_t previous = 768U;
  for (uint16_t phase = 0U; phase <= 4096U; phase = static_cast<uint16_t>(phase + 16U)) {
    const uint16_t current = fmd::acidmath::accentContributionDac12(phase, 4096U);
    TEST_ASSERT_LESS_OR_EQUAL_UINT16(previous, current);
    previous = current;
    if (phase == 4096U) {
      break;
    }
  }
}

void test_acid_algorithm_is_deterministic_and_bounded() {
  MemoryReferenceTables tables;
  fmd::AcidAlgorithm first(tables);
  fmd::AcidAlgorithm second(tables);
  const fmd::ControlFrame controls{300U, 400U, 500U, 600U};
  for (uint16_t i = 0U; i < 6000U; ++i) {
    const uint16_t a = first.step(controls);
    const uint16_t b = second.step(controls);
    TEST_ASSERT_EQUAL_UINT16(a, b);
    TEST_ASSERT_LESS_OR_EQUAL_UINT16(4095U, a);
  }
}


void test_acid_math_clamps_external_values_and_saturates_accent_sum() {
  const uint16_t clamped = fmd::acidmath::slideContourDac12(65535U, 65535U, 2048U, 65535U);
  TEST_ASSERT_LESS_OR_EQUAL_UINT16(4095U, clamped);
  TEST_ASSERT_LESS_OR_EQUAL_UINT16(
      768U, fmd::acidmath::accentContributionDac12(0U, 65535U));
  TEST_ASSERT_EQUAL_UINT16(4095U, fmd::acidmath::addAccentSaturating(4000U, 1000U));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_acid_permutation_visits_all_sixteen_codes_exactly_once);
  RUN_TEST(test_acid_masks_match_documented_predicates);
  RUN_TEST(test_acid_slide_has_exact_texture_and_phase_endpoints);
  RUN_TEST(test_acid_accent_has_exact_zero_full_and_monotone_decay);
  RUN_TEST(test_acid_algorithm_is_deterministic_and_bounded);
  RUN_TEST(test_acid_math_clamps_external_values_and_saturates_accent_sum);
  return UNITY_END();
}
