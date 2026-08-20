/**
 * @file test_main.cpp
 * Implements mathematical verification for the Generative-bank Urn algorithm.
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
#include "fmd/domain/generative/UrnAlgorithm.h"
#include "MemoryReferenceTables.h"

void test_urn_relaxation_moves_weights_monotonically_toward_baseline() {
  TEST_ASSERT_EQUAL_UINT16(fmd::urnmath::kBaselineWeight,
                           fmd::urnmath::relaxWeight(fmd::urnmath::kBaselineWeight));
  for (uint16_t weight = fmd::urnmath::kBaselineWeight + 1U;
       weight <= fmd::urnmath::kMaximumWeight;
       ++weight) {
    const uint16_t relaxed = fmd::urnmath::relaxWeight(weight);
    TEST_ASSERT_GREATER_OR_EQUAL_UINT16(fmd::urnmath::kBaselineWeight, relaxed);
    TEST_ASSERT_LESS_THAN_UINT16(weight, relaxed);
  }
}

void test_urn_reinforcement_mapping_is_monotonic_with_exact_endpoints() {
  TEST_ASSERT_EQUAL_UINT16(0U, fmd::urnmath::reinforcementAmount(0U));
  TEST_ASSERT_EQUAL_UINT16(64U, fmd::urnmath::reinforcementAmount(1023U));
  uint16_t previous = 0U;
  for (uint16_t texture = 0U; texture <= 1023U; ++texture) {
    const uint16_t current = fmd::urnmath::reinforcementAmount(texture);
    TEST_ASSERT_GREATER_OR_EQUAL_UINT16(previous, current);
    previous = current;
  }
  TEST_ASSERT_EQUAL_UINT16(
      fmd::urnmath::kMaximumWeight,
      fmd::urnmath::reinforceSaturating(1000U, 64U));
}

void test_urn_equal_weights_partition_the_complete_random_domain_equally() {
  const uint16_t weights[8] = {32U, 32U, 32U, 32U, 32U, 32U, 32U, 32U};
  uint32_t counts[8] = {0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U};
  for (uint32_t random = 0U; random <= 0xFFFFU; ++random) {
    ++counts[fmd::urnmath::selectWeightedState(
        weights, static_cast<uint16_t>(random))];
  }
  for (uint8_t state = 0U; state < 8U; ++state) {
    TEST_ASSERT_EQUAL_UINT32(8192U, counts[state]);
  }
}

void test_urn_output_vocabulary_is_exact_even_spacing_across_full_dac_range() {
  for (uint8_t state = 0U; state < 8U; ++state) {
    TEST_ASSERT_EQUAL_UINT16(static_cast<uint16_t>(585U * state),
                             fmd::urnmath::outputLevel(state));
  }
  TEST_ASSERT_EQUAL_UINT16(4095U, fmd::urnmath::outputLevel(7U));
}

void test_urn_algorithm_is_seed_deterministic_bounded_and_uses_only_vocabulary_levels() {
  MemoryReferenceTables referenceTables;
  fmd::UrnAlgorithm first(referenceTables, 0x6688U);
  fmd::UrnAlgorithm second(referenceTables, 0x6688U);
  for (uint16_t sample = 0U; sample < 7000U; ++sample) {
    const fmd::ControlFrame controls{1023U, 1023U, 1023U, 900U};
    const uint16_t a = first.step(controls);
    const uint16_t b = second.step(controls);
    TEST_ASSERT_EQUAL_UINT16(a, b);
    TEST_ASSERT_LESS_OR_EQUAL_UINT16(4095U, a);
    TEST_ASSERT_EQUAL_UINT16(0U, static_cast<uint16_t>(a % 585U));
  }
}


void test_urn_weight_helpers_handle_degenerate_and_overrange_inputs() {
  TEST_ASSERT_EQUAL_UINT16(
      fmd::urnmath::relaxWeight(fmd::urnmath::kMaximumWeight),
      fmd::urnmath::relaxWeight(static_cast<uint16_t>(fmd::urnmath::kMaximumWeight + 100U)));
  TEST_ASSERT_EQUAL_UINT16(0U, fmd::urnmath::weightedTarget(0xFFFFU, 0U));
  const uint16_t emptyWeights[fmd::urnmath::kStateCount] = {0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U};
  TEST_ASSERT_EQUAL_UINT8(0U, fmd::urnmath::selectWeightedState(emptyWeights, 0xBEEFU));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_urn_relaxation_moves_weights_monotonically_toward_baseline);
  RUN_TEST(test_urn_reinforcement_mapping_is_monotonic_with_exact_endpoints);
  RUN_TEST(test_urn_equal_weights_partition_the_complete_random_domain_equally);
  RUN_TEST(test_urn_output_vocabulary_is_exact_even_spacing_across_full_dac_range);
  RUN_TEST(test_urn_algorithm_is_seed_deterministic_bounded_and_uses_only_vocabulary_levels);
  RUN_TEST(test_urn_weight_helpers_handle_degenerate_and_overrange_inputs);
  return UNITY_END();
}
