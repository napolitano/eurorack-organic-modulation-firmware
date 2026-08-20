/**
 * @file test_main.cpp
 * Implements mathematical verification for the Organic-bank Hénon Attractor algorithm.
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
#include <climits>
#include <cstdlib>

#include "fmd/domain/organic/AttractorAlgorithm.h"
#include "fmd/domain/organic/OrganicAlgorithmMath.h"
#include "MemoryReferenceTables.h"

void test_attractor_texture_maps_monotonically_to_documented_henon_parameter_range() {
  TEST_ASSERT_EQUAL_UINT16(
      fmd::attractormath::kMinParameterAQ2F14,
      fmd::attractormath::parameterAQ2F14(0U));
  TEST_ASSERT_EQUAL_UINT16(
      fmd::attractormath::kMaxParameterAQ2F14,
      fmd::attractormath::parameterAQ2F14(1023U));

  uint16_t previous = 0U;
  for (uint16_t texture = 0U; texture <= 1023U; ++texture) {
    const uint16_t current = fmd::attractormath::parameterAQ2F14(texture);
    TEST_ASSERT_GREATER_OR_EQUAL_UINT16(previous, current);
    previous = current;
  }
}

void test_attractor_henon_iteration_matches_canonical_equation_at_known_points() {
  const uint16_t a = fmd::attractormath::kMaxParameterAQ2F14;
  const fmd::attractormath::HenonState origin{0, 0};
  const fmd::attractormath::HenonState first =
      fmd::attractormath::iterateHenon(origin, a);
  TEST_ASSERT_EQUAL_INT16(16384, first.xQ2F14);
  TEST_ASSERT_EQUAL_INT16(0, first.yQ2F14);

  const fmd::attractormath::HenonState second =
      fmd::attractormath::iterateHenon(first, a);
  // Canonical a=1.4, b=0.3 gives (-0.4, 0.3) from (1, 0).
  TEST_ASSERT_INT16_WITHIN(2, -6554, second.xQ2F14);
  TEST_ASSERT_INT16_WITHIN(2, 4915, second.yQ2F14);
}

void test_attractor_fixed_point_orbits_stay_inside_q2f14_domain_for_complete_texture_range() {
  for (uint16_t texture = 0U; texture <= 1023U; ++texture) {
    const uint16_t parameterA = fmd::attractormath::parameterAQ2F14(texture);
    fmd::attractormath::HenonState state{0, 0};
    for (uint16_t iteration = 0U; iteration < 3000U; ++iteration) {
      state = fmd::attractormath::iterateHenon(state, parameterA);
      TEST_ASSERT_LESS_THAN_INT32(30000L, std::abs(static_cast<int32_t>(state.xQ2F14)));
      TEST_ASSERT_LESS_THAN_INT32(12000L, std::abs(static_cast<int32_t>(state.yQ2F14)));
    }
  }
}

void test_attractor_interpolation_and_coordinate_mapping_are_bounded_and_monotonic() {
  constexpr int16_t start = -12000;
  constexpr int16_t end = 18000;
  int16_t previous = start;
  for (uint16_t phase = 0U; phase <= 4095U; ++phase) {
    const int16_t current = fmd::attractormath::interpolateQ2F14(start, end, phase);
    TEST_ASSERT_GREATER_OR_EQUAL_INT16(previous, current);
    previous = current;
  }
  TEST_ASSERT_EQUAL_INT16(start, fmd::attractormath::interpolateQ2F14(start, end, 0U));
  TEST_ASSERT_INT16_WITHIN(8, end, fmd::attractormath::interpolateQ2F14(start, end, 4095U));

  TEST_ASSERT_EQUAL_UINT16(0U, fmd::attractormath::coordinateToDac12(INT16_MIN));
  TEST_ASSERT_EQUAL_UINT16(2048U, fmd::attractormath::coordinateToDac12(0));
  TEST_ASSERT_EQUAL_UINT16(4095U, fmd::attractormath::coordinateToDac12(INT16_MAX));
}

void test_attractor_algorithm_is_deterministic_and_stays_in_12bit_domain() {
  MemoryReferenceTables referenceTables;
  fmd::AttractorAlgorithm first(referenceTables);
  fmd::AttractorAlgorithm second(referenceTables);

  for (uint32_t sampleIndex = 0U; sampleIndex < 25000U; ++sampleIndex) {
    const fmd::ControlFrame controls{
        static_cast<uint16_t>((sampleIndex * 37U) & 1023U),
        static_cast<uint16_t>((sampleIndex * 43U) & 1023U),
        static_cast<uint16_t>((sampleIndex * 19U + 200U) & 1023U),
        static_cast<uint16_t>((sampleIndex * 61U + 400U) & 1023U),
    };
    const uint16_t firstOutput = first.step(controls);
    const uint16_t secondOutput = second.step(controls);
    TEST_ASSERT_EQUAL_UINT16(firstOutput, secondOutput);
    TEST_ASSERT_LESS_OR_EQUAL_UINT16(4095U, firstOutput);
  }
}


void test_attractor_math_saturates_extreme_states_and_clamps_phase() {
  const auto positive = fmd::attractormath::iterateHenon(
      {0, INT16_MAX}, fmd::attractormath::kMaxParameterAQ2F14);
  TEST_ASSERT_EQUAL_INT16(INT16_MAX, positive.xQ2F14);

  const auto negative = fmd::attractormath::iterateHenon(
      {INT16_MAX, INT16_MIN}, fmd::attractormath::kMaxParameterAQ2F14);
  TEST_ASSERT_EQUAL_INT16(INT16_MIN, negative.xQ2F14);

  TEST_ASSERT_EQUAL_INT16(
      fmd::attractormath::interpolateQ2F14(-1000, 1000, 4095U),
      fmd::attractormath::interpolateQ2F14(-1000, 1000, 65535U));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_attractor_texture_maps_monotonically_to_documented_henon_parameter_range);
  RUN_TEST(test_attractor_henon_iteration_matches_canonical_equation_at_known_points);
  RUN_TEST(test_attractor_fixed_point_orbits_stay_inside_q2f14_domain_for_complete_texture_range);
  RUN_TEST(test_attractor_interpolation_and_coordinate_mapping_are_bounded_and_monotonic);
  RUN_TEST(test_attractor_algorithm_is_deterministic_and_stays_in_12bit_domain);
  RUN_TEST(test_attractor_math_saturates_extreme_states_and_clamps_phase);
  return UNITY_END();
}
