/**
 * @file test_main.cpp
 * Implements mathematical verification for the Organic-bank Fractal algorithm.
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

#include "fmd/domain/organic/FractalAlgorithm.h"
#include "fmd/domain/organic/OrganicAlgorithmMath.h"
#include "MemoryReferenceTables.h"

void test_fractal_octave_weights_have_exact_constant_sum_and_documented_endpoints() {
  const fmd::fractalmath::OctaveWeights minimum = fmd::fractalmath::octaveWeights(0U);
  TEST_ASSERT_EQUAL_UINT16(1024U, minimum.macro);
  TEST_ASSERT_EQUAL_UINT16(0U, minimum.meso);
  TEST_ASSERT_EQUAL_UINT16(0U, minimum.detail);

  const fmd::fractalmath::OctaveWeights maximum = fmd::fractalmath::octaveWeights(1023U);
  TEST_ASSERT_EQUAL_UINT16(512U, maximum.macro);
  TEST_ASSERT_EQUAL_UINT16(320U, maximum.meso);
  TEST_ASSERT_EQUAL_UINT16(192U, maximum.detail);

  uint16_t previousMacro = 1024U;
  uint16_t previousMeso = 0U;
  uint16_t previousDetail = 0U;
  for (uint16_t texture = 0U; texture <= 1023U; ++texture) {
    const fmd::fractalmath::OctaveWeights weights =
        fmd::fractalmath::octaveWeights(texture);
    TEST_ASSERT_EQUAL_UINT16(
        1024U,
        static_cast<uint16_t>(weights.macro + weights.meso + weights.detail));
    TEST_ASSERT_LESS_OR_EQUAL_UINT16(previousMacro, weights.macro);
    TEST_ASSERT_GREATER_OR_EQUAL_UINT16(previousMeso, weights.meso);
    TEST_ASSERT_GREATER_OR_EQUAL_UINT16(previousDetail, weights.detail);
    previousMacro = weights.macro;
    previousMeso = weights.meso;
    previousDetail = weights.detail;
  }
}

void test_fractal_octave_mixer_matches_integer_weighted_reference() {
  constexpr int16_t macro = -12345;
  constexpr int16_t meso = 7777;
  constexpr int16_t detail = 23456;

  for (uint16_t texture = 0U; texture <= 1023U; texture =
           static_cast<uint16_t>(texture + 31U)) {
    const fmd::fractalmath::OctaveWeights weights =
        fmd::fractalmath::octaveWeights(texture);
    const int32_t expected =
        (static_cast<int32_t>(macro) * weights.macro +
         static_cast<int32_t>(meso) * weights.meso +
         static_cast<int32_t>(detail) * weights.detail) /
        1024L;
    TEST_ASSERT_EQUAL_INT16(
        static_cast<int16_t>(expected),
        fmd::fractalmath::mixOctavesQ1F15(macro, meso, detail, weights));
  }
}

void test_fractal_algorithm_is_deterministic_and_stays_in_12bit_domain() {
  MemoryReferenceTables referenceTables;
  fmd::FractalAlgorithm first(referenceTables, 0x4312U);
  fmd::FractalAlgorithm second(referenceTables, 0x4312U);

  for (uint32_t sampleIndex = 0U; sampleIndex < 12000U; ++sampleIndex) {
    const fmd::ControlFrame controls{
        static_cast<uint16_t>((sampleIndex * 31U) & 1023U),
        static_cast<uint16_t>((sampleIndex * 53U + 19U) & 1023U),
        static_cast<uint16_t>((sampleIndex * 17U + 701U) & 1023U),
        static_cast<uint16_t>((sampleIndex * 71U + 3U) & 1023U),
    };
    const uint16_t firstOutput = first.step(controls);
    const uint16_t secondOutput = second.step(controls);
    TEST_ASSERT_EQUAL_UINT16(firstOutput, secondOutput);
    TEST_ASSERT_LESS_OR_EQUAL_UINT16(4095U, firstOutput);
  }
}

void test_fractal_texture_changes_multiscale_trajectory_for_same_seed() {
  MemoryReferenceTables referenceTables;
  fmd::FractalAlgorithm smooth(referenceTables, 0x51A7U);
  fmd::FractalAlgorithm rough(referenceTables, 0x51A7U);
  const fmd::ControlFrame smoothControls{0U, 0U, 320U, 0U};
  const fmd::ControlFrame roughControls{0U, 0U, 320U, 1023U};

  bool observedDifference = false;
  for (uint16_t sampleIndex = 0U; sampleIndex < 2000U; ++sampleIndex) {
    if (smooth.step(smoothControls) != rough.step(roughControls)) {
      observedDifference = true;
      break;
    }
  }
  TEST_ASSERT_TRUE(observedDifference);
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_fractal_octave_weights_have_exact_constant_sum_and_documented_endpoints);
  RUN_TEST(test_fractal_octave_mixer_matches_integer_weighted_reference);
  RUN_TEST(test_fractal_algorithm_is_deterministic_and_stays_in_12bit_domain);
  RUN_TEST(test_fractal_texture_changes_multiscale_trajectory_for_same_seed);
  return UNITY_END();
}
