/**
 * @file test_main.cpp
 * Verifies Percussion Humanize mathematical and integration contracts.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <unity.h>
#include "fmd/domain/percussion/HumanizeAlgorithm.h"
#include "fmd/domain/percussion/PercussionAlgorithmMath.h"
#include "MemoryReferenceTables.h"
#include "fmd/domain/AlgorithmMath.h"
#include "fmd/domain/electronica/ElectronicaAlgorithmMath.h"

void test_humanize_jitter_radius_maps_zero_to_thirty_samples() {
  TEST_ASSERT_EQUAL_UINT8(0U, fmd::humanizemath::jitterRadiusSamples(0U));
  TEST_ASSERT_EQUAL_UINT8(30U, fmd::humanizemath::jitterRadiusSamples(1023U));
}

void test_humanize_amplitude_radius_maps_zero_to_255() {
  TEST_ASSERT_EQUAL_UINT16(0U, fmd::humanizemath::amplitudeRadiusDac12(0U));
  TEST_ASSERT_EQUAL_UINT16(255U, fmd::humanizemath::amplitudeRadiusDac12(1023U));
}

void test_humanize_random_mapping_stays_inside_exact_bounds() {
  for (uint32_t word = 0U; word <= 65535UL; word += 257UL) {
    const int8_t jitter = fmd::humanizemath::jitterSamples(static_cast<uint16_t>(word), 1023U);
    TEST_ASSERT_GREATER_OR_EQUAL_INT8(-30, jitter);
    TEST_ASSERT_LESS_OR_EQUAL_INT8(30, jitter);
    const uint16_t amplitude = fmd::humanizemath::pulseAmplitudeDac12(static_cast<uint16_t>(word), 1023U);
    TEST_ASSERT_GREATER_OR_EQUAL_UINT16(3585U, amplitude);
    TEST_ASSERT_LESS_OR_EQUAL_UINT16(4095U, amplitude);
  }
}

void test_humanize_texture_zero_is_neutral() {
  TEST_ASSERT_EQUAL_INT8(0, fmd::humanizemath::jitterSamples(0U, 0U));
  TEST_ASSERT_EQUAL_INT8(0, fmd::humanizemath::jitterSamples(65535U, 0U));
  TEST_ASSERT_EQUAL_UINT16(3840U, fmd::humanizemath::pulseAmplitudeDac12(0U, 0U));
  TEST_ASSERT_EQUAL_UINT16(3840U, fmd::humanizemath::pulseAmplitudeDac12(65535U, 0U));
}

void test_humanize_algorithm_is_reproducible_and_bounded() {
  MemoryReferenceTables tables;
  fmd::HumanizeAlgorithm first(tables, 0xCAFEU);
  fmd::HumanizeAlgorithm second(tables, 0xCAFEU);
  const fmd::ControlFrame controls{800U, 1023U, 300U, 0U};
  for (uint16_t i = 0U; i < 8000U; ++i) {
    const uint16_t a = first.step(controls);
    TEST_ASSERT_EQUAL_UINT16(a, second.step(controls));
    TEST_ASSERT_LESS_OR_EQUAL_UINT16(4095U, a);
  }
}

void test_humanize_positive_jitter_starts_after_nominal_boundary() {
  MemoryReferenceTables tables;
  constexpr uint16_t kSeedWithPositiveFirstJitter = 0x3344U;
  fmd::HumanizeAlgorithm algorithm(tables, kSeedWithPositiveFirstJitter);
  const fmd::ControlFrame controls{0U, 1023U, 1023U, 0U};

  const uint32_t increment =
      fmd::electronicamath::quarterNotePhaseIncrement(tables, 1023U, 0U) * 2UL;
  uint32_t phase = 0U;
  uint32_t callsToBoundary = 0U;
  bool rollover = false;
  do {
    phase = fmd::perlinmath::advancePhase(phase, increment, rollover);
    ++callsToBoundary;
  } while (!rollover);

  // Seed 0x3344 produces +16 samples for the first Texture-max jitter draw.
  const uint32_t expectedSecondOnsetIndex = callsToBoundary + 15U;
  bool previousHigh = false;
  uint32_t observedSecondOnsetIndex = UINT32_MAX;
  for (uint32_t sample = 0U; sample < expectedSecondOnsetIndex + 4U; ++sample) {
    const bool high = algorithm.step(controls) > 0U;
    if (sample > 0U && high && !previousHigh) {
      observedSecondOnsetIndex = sample;
      break;
    }
    previousHigh = high;
  }
  TEST_ASSERT_EQUAL_UINT32(expectedSecondOnsetIndex, observedSecondOnsetIndex);
}


void test_humanize_output_projection_remains_clamped_at_extreme_random_values() {
  TEST_ASSERT_GREATER_OR_EQUAL_UINT16(
      0U, fmd::humanizemath::pulseAmplitudeDac12(0U, 65535U));
  TEST_ASSERT_LESS_OR_EQUAL_UINT16(
      4095U, fmd::humanizemath::pulseAmplitudeDac12(0xFFFFU, 65535U));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_humanize_jitter_radius_maps_zero_to_thirty_samples);
  RUN_TEST(test_humanize_amplitude_radius_maps_zero_to_255);
  RUN_TEST(test_humanize_random_mapping_stays_inside_exact_bounds);
  RUN_TEST(test_humanize_texture_zero_is_neutral);
  RUN_TEST(test_humanize_algorithm_is_reproducible_and_bounded);
  RUN_TEST(test_humanize_positive_jitter_starts_after_nominal_boundary);
  RUN_TEST(test_humanize_output_projection_remains_clamped_at_extreme_random_values);
  return UNITY_END();
}
