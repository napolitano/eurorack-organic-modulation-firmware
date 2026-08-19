/**
 * @file test_main.cpp
 * Implements mathematical verification for the Generative-bank Turing algorithm.
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

#include "fmd/domain/GenerativeAlgorithmMath.h"
#include "fmd/domain/TuringAlgorithm.h"
#include "MemoryReferenceTables.h"

void test_turing_mutation_mapping_has_exact_zero_and_half_probability_endpoints() {
  TEST_ASSERT_EQUAL_UINT32(0U, fmd::turingmath::mutationCutoff(0U));
  TEST_ASSERT_EQUAL_UINT32(32768U, fmd::turingmath::mutationCutoff(1023U));
  TEST_ASSERT_FALSE(fmd::turingmath::shouldMutate(0U, 0U));
  TEST_ASSERT_TRUE(fmd::turingmath::shouldMutate(1023U, 32767U));
  TEST_ASSERT_FALSE(fmd::turingmath::shouldMutate(1023U, 32768U));

  uint32_t previous = 0U;
  for (uint16_t texture = 0U; texture <= 1023U; ++texture) {
    const uint32_t current = fmd::turingmath::mutationCutoff(texture);
    TEST_ASSERT_GREATER_OR_EQUAL_UINT32(previous, current);
    TEST_ASSERT_LESS_OR_EQUAL_UINT32(32768U, current);
    previous = current;
  }
}

void test_turing_locked_register_returns_exactly_after_sixteen_rotations() {
  constexpr uint16_t initial = 0xA53CU;
  uint16_t state = initial;
  for (uint8_t step = 0U; step < 16U; ++step) {
    state = fmd::turingmath::advanceRegister(state, false);
  }
  TEST_ASSERT_EQUAL_HEX16(initial, state);
}

void test_turing_forced_mutation_changes_only_recycled_feedback_bit() {
  constexpr uint16_t state = 0x8123U;
  const uint16_t normal = fmd::turingmath::advanceRegister(state, false);
  const uint16_t mutated = fmd::turingmath::advanceRegister(state, true);
  TEST_ASSERT_EQUAL_HEX16(0x8000U, static_cast<uint16_t>(normal ^ mutated));
  TEST_ASSERT_EQUAL_UINT16(static_cast<uint16_t>(normal & 0x7FFFU),
                           static_cast<uint16_t>(mutated & 0x7FFFU));
}

void test_turing_algorithm_is_seed_deterministic_and_bounded() {
  MemoryReferenceTables referenceTables;
  fmd::TuringAlgorithm first(referenceTables, 0x4411U);
  fmd::TuringAlgorithm second(referenceTables, 0x4411U);

  bool observedChange = false;
  const fmd::ControlFrame controls{1023U, 1023U, 1023U, 1023U};
  uint16_t previous = first.step(controls);
  TEST_ASSERT_EQUAL_UINT16(previous, second.step(controls));
  for (uint16_t sample = 0U; sample < 4000U; ++sample) {
    const uint16_t a = first.step(controls);
    const uint16_t b = second.step(controls);
    TEST_ASSERT_EQUAL_UINT16(a, b);
    TEST_ASSERT_LESS_OR_EQUAL_UINT16(4095U, a);
    observedChange |= a != previous;
    previous = a;
  }
  TEST_ASSERT_TRUE(observedChange);
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_turing_mutation_mapping_has_exact_zero_and_half_probability_endpoints);
  RUN_TEST(test_turing_locked_register_returns_exactly_after_sixteen_rotations);
  RUN_TEST(test_turing_forced_mutation_changes_only_recycled_feedback_bit);
  RUN_TEST(test_turing_algorithm_is_seed_deterministic_and_bounded);
  return UNITY_END();
}
