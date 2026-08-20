/**
 * @file test_main.cpp
 * Verifies Percussion Euclid mathematical and integration contracts.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <unity.h>
#include "fmd/domain/percussion/EuclidAlgorithm.h"
#include "fmd/domain/percussion/PercussionAlgorithmMath.h"
#include "MemoryReferenceTables.h"

void test_phrase_length_and_fill_strength_have_exact_regions() {
  TEST_ASSERT_EQUAL_UINT8(16U, fmd::percussionmath::phraseLengthBars(0U));
  TEST_ASSERT_EQUAL_UINT8(16U, fmd::percussionmath::phraseLengthBars(255U));
  TEST_ASSERT_EQUAL_UINT8(12U, fmd::percussionmath::phraseLengthBars(256U));
  TEST_ASSERT_EQUAL_UINT8(8U, fmd::percussionmath::phraseLengthBars(512U));
  TEST_ASSERT_EQUAL_UINT8(4U, fmd::percussionmath::phraseLengthBars(768U));
  TEST_ASSERT_EQUAL_UINT8(0U, fmd::percussionmath::fillStrength(0U));
  TEST_ASSERT_EQUAL_UINT8(4U, fmd::percussionmath::fillStrength(1023U));
}

void test_euclid_hit_count_maps_exactly_to_2_through_13() {
  TEST_ASSERT_EQUAL_UINT8(2U, fmd::euclidmath::hitCount(0U));
  TEST_ASSERT_EQUAL_UINT8(13U, fmd::euclidmath::hitCount(1023U));
  for (uint16_t t = 0U; t < 1023U; ++t) {
    const uint8_t current = fmd::euclidmath::hitCount(t);
    const uint8_t next = fmd::euclidmath::hitCount(static_cast<uint16_t>(t + 1U));
    TEST_ASSERT_TRUE(next >= current);
    TEST_ASSERT_TRUE(next <= static_cast<uint8_t>(current + 1U));
  }
}

void test_euclid_masks_have_requested_population_and_step_zero() {
  for (uint8_t hits = 2U; hits <= 13U; ++hits) {
    const uint16_t mask = fmd::euclidmath::canonicalMask(hits);
    uint8_t count = 0U;
    for (uint8_t step = 0U; step < 16U; ++step) {
      if (fmd::euclidmath::stepHits(mask, step)) ++count;
    }
    TEST_ASSERT_EQUAL_UINT8(hits, count);
    TEST_ASSERT_TRUE(fmd::euclidmath::stepHits(mask, 0U));
  }
}

void test_fill_tail_masks_only_add_final_quarter_candidates() {
  TEST_ASSERT_EQUAL_HEX16(0x0000U, fmd::euclidmath::fillTailMask(0U));
  TEST_ASSERT_EQUAL_HEX16(0x8000U, fmd::euclidmath::fillTailMask(1U));
  TEST_ASSERT_EQUAL_HEX16(0xC000U, fmd::euclidmath::fillTailMask(2U));
  TEST_ASSERT_EQUAL_HEX16(0xE000U, fmd::euclidmath::fillTailMask(3U));
  TEST_ASSERT_EQUAL_HEX16(0xF000U, fmd::euclidmath::fillTailMask(4U));
}

void test_euclid_algorithm_is_deterministic_and_bounded() {
  MemoryReferenceTables tables;
  fmd::EuclidAlgorithm first(tables);
  fmd::EuclidAlgorithm second(tables);
  const fmd::ControlFrame controls{700U, 0U, 600U, 0U};
  for (uint16_t i = 0U; i < 5000U; ++i) {
    const uint16_t a = first.step(controls);
    const uint16_t b = second.step(controls);
    TEST_ASSERT_EQUAL_UINT16(a, b);
    TEST_ASSERT_TRUE(a == 0U || a == 4095U);
  }
}

void test_euclid_acquires_external_clock_on_second_valid_edge() {
  MemoryReferenceTables tables;
  fmd::EuclidAlgorithm algorithm(tables);
  fmd::ControlFrame controls{0U, 0U, 0U, 1023U};

  // Drain the startup pulse while the very slow Speed-pot clock is internal.
  for (uint8_t i = 0U; i < 30U; ++i) {
    algorithm.step(controls);
  }

  controls.speedCv = 600U;
  algorithm.step(controls);  // first edge only arms measurement
  controls.speedCv = 0U;
  for (uint8_t i = 1U; i < 100U; ++i) {
    algorithm.step(controls);
  }
  controls.speedCv = 600U;
  TEST_ASSERT_EQUAL_UINT16(4095U, algorithm.step(controls));
}

void test_percussion_speed_cv_dc_does_not_modulate_internal_tempo() {
  MemoryReferenceTables tables;
  fmd::EuclidAlgorithm lowCv(tables);
  fmd::EuclidAlgorithm highDcCv(tables);
  const fmd::ControlFrame lowControls{0U, 0U, 700U, 1023U};
  const fmd::ControlFrame highControls{1023U, 0U, 700U, 1023U};
  for (uint16_t sample = 0U; sample < 3000U; ++sample) {
    TEST_ASSERT_EQUAL_UINT16(lowCv.step(lowControls), highDcCv.step(highControls));
  }
}


void test_euclid_helpers_clamp_invalid_hit_and_fill_inputs() {
  TEST_ASSERT_EQUAL_HEX16(
      fmd::euclidmath::canonicalMask(2U), fmd::euclidmath::canonicalMask(0U));
  TEST_ASSERT_EQUAL_HEX16(
      fmd::euclidmath::canonicalMask(13U), fmd::euclidmath::canonicalMask(255U));
  TEST_ASSERT_EQUAL_HEX16(
      fmd::euclidmath::fillTailMask(4U), fmd::euclidmath::fillTailMask(255U));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_phrase_length_and_fill_strength_have_exact_regions);
  RUN_TEST(test_euclid_hit_count_maps_exactly_to_2_through_13);
  RUN_TEST(test_euclid_masks_have_requested_population_and_step_zero);
  RUN_TEST(test_fill_tail_masks_only_add_final_quarter_candidates);
  RUN_TEST(test_euclid_algorithm_is_deterministic_and_bounded);
  RUN_TEST(test_euclid_acquires_external_clock_on_second_valid_edge);
  RUN_TEST(test_percussion_speed_cv_dc_does_not_modulate_internal_tempo);
  RUN_TEST(test_euclid_helpers_clamp_invalid_hit_and_fill_inputs);
  return UNITY_END();
}
