/**
 * @file test_main.cpp
 * Implements mathematical verification for the Generative-bank Markov algorithm.
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
#include "fmd/domain/generative/MarkovAlgorithm.h"
#include "MemoryReferenceTables.h"

void test_markov_structured_kernel_has_exact_documented_partition() {
  constexpr uint8_t current = 3U;
  const uint8_t expected[8] = {3U, 3U, 3U, 3U, 4U, 4U, 2U, 7U};
  for (uint8_t draw = 0U; draw < 8U; ++draw) {
    TEST_ASSERT_EQUAL_UINT8(expected[draw],
                            fmd::markovmath::structuredNextState(current, draw));
  }
}

void test_markov_exploration_has_exact_control_endpoints() {
  for (uint32_t random = 0U; random <= 0xFFFFU; random += 257U) {
    TEST_ASSERT_FALSE(fmd::markovmath::useUniformExploration(
        0U, static_cast<uint16_t>(random)));
    TEST_ASSERT_TRUE(fmd::markovmath::useUniformExploration(
        1023U, static_cast<uint16_t>(random)));
  }
}

void test_markov_stratified_vocabulary_covers_exactly_one_value_per_source_band() {
  for (uint8_t band = 0U; band < 8U; ++band) {
    const uint16_t low = fmd::markovmath::stratifiedVocabularyValue(band, 0U);
    const uint16_t high =
        fmd::markovmath::stratifiedVocabularyValue(band, 0xFFFFU);
    TEST_ASSERT_EQUAL_UINT16(static_cast<uint16_t>(band * 512U), low);
    TEST_ASSERT_EQUAL_UINT16(static_cast<uint16_t>(band * 512U + 511U), high);
  }
}

void test_markov_algorithm_is_seed_deterministic_bounded_and_uses_fixed_vocabulary() {
  MemoryReferenceTables referenceTables;
  fmd::MarkovAlgorithm first(referenceTables, 0x2244U);
  fmd::MarkovAlgorithm second(referenceTables, 0x2244U);

  uint16_t seen[32] = {0U};
  uint8_t seenCount = 0U;
  for (uint16_t sample = 0U; sample < 6000U; ++sample) {
    const fmd::ControlFrame controls{1023U, 1023U, 1023U, 1023U};
    const uint16_t a = first.step(controls);
    const uint16_t b = second.step(controls);
    TEST_ASSERT_EQUAL_UINT16(a, b);
    TEST_ASSERT_LESS_OR_EQUAL_UINT16(4095U, a);

    bool known = false;
    for (uint8_t i = 0U; i < seenCount; ++i) {
      known |= seen[i] == a;
    }
    if (!known && seenCount < 32U) {
      seen[seenCount++] = a;
    }
  }
  TEST_ASSERT_GREATER_OR_EQUAL_UINT8(2U, seenCount);
  TEST_ASSERT_LESS_OR_EQUAL_UINT8(8U, seenCount);
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_markov_structured_kernel_has_exact_documented_partition);
  RUN_TEST(test_markov_exploration_has_exact_control_endpoints);
  RUN_TEST(test_markov_stratified_vocabulary_covers_exactly_one_value_per_source_band);
  RUN_TEST(test_markov_algorithm_is_seed_deterministic_bounded_and_uses_fixed_vocabulary);
  return UNITY_END();
}
