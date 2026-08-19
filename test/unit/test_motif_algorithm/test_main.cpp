/**
 * @file test_main.cpp
 * Implements mathematical verification for the Generative-bank Motif algorithm.
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

#include "fmd/domain/generative/GenerativeAlgorithmMath.h"
#include "fmd/domain/generative/MotifAlgorithm.h"
#include "MemoryReferenceTables.h"

namespace {
void assertPhrase(const uint16_t actual[8], const uint16_t expected[8]) {
  for (uint8_t i = 0U; i < 8U; ++i) {
    TEST_ASSERT_EQUAL_UINT16(expected[i], actual[i]);
  }
}
}

void test_motif_edit_probability_has_exact_zero_and_full_endpoints() {
  for (uint32_t random = 0U; random <= 0xFFFFU; random += 257U) {
    TEST_ASSERT_FALSE(fmd::motifmath::shouldEdit(0U, static_cast<uint16_t>(random)));
    TEST_ASSERT_TRUE(fmd::motifmath::shouldEdit(1023U, static_cast<uint16_t>(random)));
  }
}

void test_motif_rotations_are_exact_and_reversible() {
  uint16_t phrase[8] = {0U, 1U, 2U, 3U, 4U, 5U, 6U, 7U};
  const uint16_t left[8] = {1U, 2U, 3U, 4U, 5U, 6U, 7U, 0U};
  const uint16_t original[8] = {0U, 1U, 2U, 3U, 4U, 5U, 6U, 7U};
  fmd::motifmath::rotate(phrase, true);
  assertPhrase(phrase, left);
  fmd::motifmath::rotate(phrase, false);
  assertPhrase(phrase, original);
}

void test_motif_circular_swap_and_three_step_reversal_handle_boundary() {
  uint16_t swapped[8] = {0U, 1U, 2U, 3U, 4U, 5U, 6U, 7U};
  const uint16_t expectedSwap[8] = {7U, 1U, 2U, 3U, 4U, 5U, 6U, 0U};
  fmd::motifmath::adjacentSwap(swapped, 7U);
  assertPhrase(swapped, expectedSwap);

  uint16_t reversed[8] = {0U, 1U, 2U, 3U, 4U, 5U, 6U, 7U};
  const uint16_t expectedReverse[8] = {0U, 7U, 2U, 3U, 4U, 5U, 6U, 1U};
  fmd::motifmath::reverseThree(reversed, 7U);
  assertPhrase(reversed, expectedReverse);
}

void test_motif_replacement_changes_one_position_and_clamps_to_twelve_bits() {
  uint16_t phrase[8] = {10U, 11U, 12U, 13U, 14U, 15U, 16U, 17U};
  fmd::motifmath::replaceOne(phrase, 10U, 0xFABC);
  TEST_ASSERT_EQUAL_UINT16(0x0ABCU, phrase[2]);
  TEST_ASSERT_EQUAL_UINT16(10U, phrase[0]);
  TEST_ASSERT_EQUAL_UINT16(17U, phrase[7]);
}

void test_motif_algorithm_is_seed_deterministic_and_bounded() {
  MemoryReferenceTables referenceTables;
  fmd::MotifAlgorithm first(referenceTables, 0x1255U);
  fmd::MotifAlgorithm second(referenceTables, 0x1255U);
  for (uint16_t sample = 0U; sample < 7000U; ++sample) {
    const fmd::ControlFrame controls{1023U, 1023U, 1023U, 1023U};
    const uint16_t a = first.step(controls);
    const uint16_t b = second.step(controls);
    TEST_ASSERT_EQUAL_UINT16(a, b);
    TEST_ASSERT_LESS_OR_EQUAL_UINT16(4095U, a);
  }
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_motif_edit_probability_has_exact_zero_and_full_endpoints);
  RUN_TEST(test_motif_rotations_are_exact_and_reversible);
  RUN_TEST(test_motif_circular_swap_and_three_step_reversal_handle_boundary);
  RUN_TEST(test_motif_replacement_changes_one_position_and_clamps_to_twelve_bits);
  RUN_TEST(test_motif_algorithm_is_seed_deterministic_and_bounded);
  return UNITY_END();
}
