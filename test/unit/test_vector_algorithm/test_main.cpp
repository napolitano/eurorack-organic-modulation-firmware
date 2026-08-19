/**
 * @file test_main.cpp
 * Implements mathematical verification for the Organic-bank Vector algorithm.
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

#include "fmd/domain/organic/OrganicAlgorithmMath.h"
#include "fmd/domain/organic/VectorAlgorithm.h"
#include "MemoryReferenceTables.h"

void test_vector_triangle_projection_has_expected_turning_points_and_branch_monotonicity() {
  TEST_ASSERT_EQUAL_INT16(INT16_MIN, fmd::vectormath::triangleSignedQ1F15(0U));
  TEST_ASSERT_GREATER_THAN_INT16(32760, fmd::vectormath::triangleSignedQ1F15(32768U));
  TEST_ASSERT_LESS_THAN_INT16(-32760, fmd::vectormath::triangleSignedQ1F15(65535U));

  int16_t previous = fmd::vectormath::triangleSignedQ1F15(0U);
  for (uint32_t phase = 1U; phase <= 32768U; ++phase) {
    const int16_t current =
        fmd::vectormath::triangleSignedQ1F15(static_cast<uint16_t>(phase));
    TEST_ASSERT_GREATER_OR_EQUAL_INT16(previous, current);
    previous = current;
  }

  previous = fmd::vectormath::triangleSignedQ1F15(32768U);
  for (uint32_t phase = 32769U; phase <= 65535U; ++phase) {
    const int16_t current =
        fmd::vectormath::triangleSignedQ1F15(static_cast<uint16_t>(phase));
    TEST_ASSERT_LESS_OR_EQUAL_INT16(previous, current);
    previous = current;
  }
}

void test_vector_zero_texture_is_uncoupled_and_full_texture_remains_forward_bounded() {
  constexpr uint32_t baseIncrement = 800000000UL;
  const int16_t sampleWaves[] = {INT16_MIN, -16384, 0, 16384, INT16_MAX};

  for (int16_t wave : sampleWaves) {
    TEST_ASSERT_EQUAL_UINT32(
        baseIncrement,
        fmd::vectormath::coupledPhaseIncrement(baseIncrement, wave, 0U, false));

    const uint32_t positiveSense =
        fmd::vectormath::coupledPhaseIncrement(baseIncrement, wave, 1023U, false);
    const uint32_t negativeSense =
        fmd::vectormath::coupledPhaseIncrement(baseIncrement, wave, 1023U, true);
    TEST_ASSERT_GREATER_THAN_UINT32(0U, positiveSense);
    TEST_ASSERT_GREATER_THAN_UINT32(0U, negativeSense);
    TEST_ASSERT_UINT32_WITHIN(baseIncrement / 4U + 50000U, baseIncrement, positiveSense);
    TEST_ASSERT_UINT32_WITHIN(baseIncrement / 4U + 50000U, baseIncrement, negativeSense);
  }
}

void test_vector_projection_maps_bipolar_axes_into_12bit_domain() {
  TEST_ASSERT_EQUAL_UINT16(0U, fmd::vectormath::projectToDac12(INT16_MIN, INT16_MIN));
  TEST_ASSERT_UINT16_WITHIN(1U, 2048U, fmd::vectormath::projectToDac12(0, 0));
  TEST_ASSERT_EQUAL_UINT16(4095U, fmd::vectormath::projectToDac12(INT16_MAX, INT16_MAX));

  for (int32_t x = -32768L; x <= 32767L; x += 4093L) {
    for (int32_t y = -32768L; y <= 32767L; y += 4093L) {
      TEST_ASSERT_LESS_OR_EQUAL_UINT16(
          4095U,
          fmd::vectormath::projectToDac12(
              static_cast<int16_t>(x), static_cast<int16_t>(y)));
    }
  }
}

void test_vector_algorithm_is_deterministic_and_texture_changes_flow() {
  MemoryReferenceTables referenceTables;
  fmd::VectorAlgorithm uncoupled(referenceTables);
  fmd::VectorAlgorithm uncoupledCopy(referenceTables);
  fmd::VectorAlgorithm coupled(referenceTables);

  bool textureChangedTrajectory = false;
  for (uint32_t sampleIndex = 0U; sampleIndex < 8000U; ++sampleIndex) {
    const fmd::ControlFrame zeroTexture{127U, 0U, 311U, 0U};
    const fmd::ControlFrame fullTexture{127U, 0U, 311U, 1023U};
    const uint16_t first = uncoupled.step(zeroTexture);
    const uint16_t second = uncoupledCopy.step(zeroTexture);
    const uint16_t third = coupled.step(fullTexture);
    TEST_ASSERT_EQUAL_UINT16(first, second);
    TEST_ASSERT_LESS_OR_EQUAL_UINT16(4095U, first);
    TEST_ASSERT_LESS_OR_EQUAL_UINT16(4095U, third);
    textureChangedTrajectory |= first != third;
  }
  TEST_ASSERT_TRUE(textureChangedTrajectory);
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_vector_triangle_projection_has_expected_turning_points_and_branch_monotonicity);
  RUN_TEST(test_vector_zero_texture_is_uncoupled_and_full_texture_remains_forward_bounded);
  RUN_TEST(test_vector_projection_maps_bipolar_axes_into_12bit_domain);
  RUN_TEST(test_vector_algorithm_is_deterministic_and_texture_changes_flow);
  return UNITY_END();
}
