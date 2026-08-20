/**
 * @file test_main.cpp
 * Verifies Dubstep/Bass Chop mathematical and runtime contracts.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <unity.h>

#include "fmd/domain/dubstep/ChopAlgorithm.h"
#include "fmd/domain/dubstep/DubstepAlgorithmMath.h"
#include "MemoryReferenceTables.h"

namespace {
uint8_t popcount16(uint16_t value) {
  uint8_t count = 0U;
  for (uint8_t bit = 0U; bit < 16U; ++bit) {
    if ((value & static_cast<uint16_t>(1U << bit)) != 0U) ++count;
  }
  return count;
}
}

void test_chop_texture_maps_monotonically_to_zero_through_eight_additions() {
  TEST_ASSERT_EQUAL_UINT8(0U, fmd::chopmath::addedOnsetCount(0U));
  TEST_ASSERT_EQUAL_UINT8(8U, fmd::chopmath::addedOnsetCount(1023U));
  for (uint16_t texture = 0U; texture < 1023U; ++texture) {
    TEST_ASSERT_TRUE(fmd::chopmath::addedOnsetCount(texture + 1U) >=
                     fmd::chopmath::addedOnsetCount(texture));
  }
}

void test_chop_masks_preserve_anchors_population_and_strict_supersets() {
  uint16_t previous = fmd::chopmath::onsetMask(0U);
  TEST_ASSERT_EQUAL_HEX16(0x0101U, previous);
  for (uint8_t added = 0U; added <= 8U; ++added) {
    const uint16_t mask = fmd::chopmath::onsetMask(added);
    TEST_ASSERT_TRUE(fmd::chopmath::stepActive(mask, 0U));
    TEST_ASSERT_TRUE(fmd::chopmath::stepActive(mask, 8U));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(2U + added), popcount16(mask));
    if (added > 0U) {
      TEST_ASSERT_EQUAL_HEX16(previous, static_cast<uint16_t>(mask & previous));
      TEST_ASSERT_NOT_EQUAL(previous, mask);
    }
    previous = mask;
  }
}

void test_chop_candidate_order_matches_documented_syncopation_grammar() {
  constexpr uint8_t candidates[8] = {3U, 11U, 6U, 14U, 2U, 10U, 7U, 15U};
  uint16_t previous = fmd::chopmath::onsetMask(0U);
  for (uint8_t added = 1U; added <= 8U; ++added) {
    const uint16_t current = fmd::chopmath::onsetMask(added);
    const uint16_t newBit = static_cast<uint16_t>(current ^ previous);
    TEST_ASSERT_EQUAL_HEX16(static_cast<uint16_t>(1U << candidates[added - 1U]), newBit);
    previous = current;
  }
}

void test_chop_articulation_holds_then_decays_to_zero() {
  TEST_ASSERT_EQUAL_UINT16(4095U, fmd::chopmath::articulationDac12(0U));
  TEST_ASSERT_EQUAL_UINT16(4095U, fmd::chopmath::articulationDac12(UINT32_C(0x7FFFFFFF)));
  TEST_ASSERT_EQUAL_UINT16(4095U, fmd::chopmath::articulationDac12(UINT32_C(0x80000000)));
  const uint16_t threeQuarter = fmd::chopmath::articulationDac12(UINT32_C(0xC0000000));
  TEST_ASSERT_UINT16_WITHIN(2U, 2048U, threeQuarter);
  TEST_ASSERT_EQUAL_UINT16(0U, fmd::chopmath::articulationDac12(UINT32_MAX));
}

void test_chop_algorithm_is_deterministic_and_bounded() {
  MemoryReferenceTables tables;
  fmd::ChopAlgorithm first(tables);
  fmd::ChopAlgorithm second(tables);
  const fmd::ControlFrame controls{0U, 300U, 512U, 600U};
  for (uint16_t i = 0U; i < 6000U; ++i) {
    const uint16_t a = first.step(controls);
    const uint16_t b = second.step(controls);
    TEST_ASSERT_EQUAL_UINT16(a, b);
    TEST_ASSERT_LESS_OR_EQUAL_UINT16(4095U, a);
  }
}

void test_chop_steady_speed_cv_dc_is_not_tempo_modulation() {
  MemoryReferenceTables tables;
  fmd::ChopAlgorithm low(tables);
  fmd::ChopAlgorithm high(tables);
  const fmd::ControlFrame lowControls{0U, 0U, 512U, 1023U};
  const fmd::ControlFrame highControls{1023U, 0U, 512U, 1023U};
  for (uint16_t i = 0U; i < 3000U; ++i) {
    TEST_ASSERT_EQUAL_UINT16(low.step(lowControls), high.step(highControls));
  }
}

void test_chop_helpers_clamp_invalid_inputs() {
  TEST_ASSERT_EQUAL_HEX16(fmd::chopmath::onsetMask(8U), fmd::chopmath::onsetMask(255U));
  const uint16_t mask = fmd::chopmath::onsetMask(0U);
  TEST_ASSERT_EQUAL(fmd::chopmath::stepActive(mask, 0U), fmd::chopmath::stepActive(mask, 16U));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_chop_texture_maps_monotonically_to_zero_through_eight_additions);
  RUN_TEST(test_chop_masks_preserve_anchors_population_and_strict_supersets);
  RUN_TEST(test_chop_candidate_order_matches_documented_syncopation_grammar);
  RUN_TEST(test_chop_articulation_holds_then_decays_to_zero);
  RUN_TEST(test_chop_algorithm_is_deterministic_and_bounded);
  RUN_TEST(test_chop_steady_speed_cv_dc_is_not_tempo_modulation);
  RUN_TEST(test_chop_helpers_clamp_invalid_inputs);
  return UNITY_END();
}
