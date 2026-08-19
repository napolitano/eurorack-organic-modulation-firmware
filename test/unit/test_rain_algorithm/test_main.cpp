/**
 * @file test_main.cpp
 * Implements mathematical verification for the Organic-bank Rain algorithm.
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
#include "fmd/domain/organic/RainAlgorithm.h"

void test_rain_density_threshold_is_quadratic_monotonic_and_sparse_at_low_settings() {
  TEST_ASSERT_EQUAL_UINT16(0U, fmd::rainmath::eventCutoff(0U));
  TEST_ASSERT_EQUAL_UINT16(64U, fmd::rainmath::eventCutoff(64U));
  TEST_ASSERT_EQUAL_UINT16(1024U, fmd::rainmath::eventCutoff(256U));
  TEST_ASSERT_EQUAL_UINT16(16352U, fmd::rainmath::eventCutoff(1023U));

  uint16_t previous = 0U;
  for (uint16_t density = 0U; density <= 1023U; ++density) {
    const uint16_t current = fmd::rainmath::eventCutoff(density);
    TEST_ASSERT_GREATER_OR_EQUAL_UINT16(previous, current);
    previous = current;
  }
}

void test_rain_decay_speed_mapping_is_monotonic_with_exact_endpoints() {
  TEST_ASSERT_EQUAL_UINT16(
      fmd::rainmath::kMinDecayAlphaQ0F16,
      fmd::rainmath::decayAlphaQ0F16(0U));
  TEST_ASSERT_EQUAL_UINT16(
      fmd::rainmath::kMaxDecayAlphaQ0F16,
      fmd::rainmath::decayAlphaQ0F16(1023U));

  uint16_t previous = 0U;
  for (uint16_t speed = 0U; speed <= 1023U; ++speed) {
    const uint16_t current = fmd::rainmath::decayAlphaQ0F16(speed);
    TEST_ASSERT_GREATER_OR_EQUAL_UINT16(previous, current);
    previous = current;
  }
}

void test_rain_impulses_are_bounded_and_saturating_accumulation_is_exact() {
  for (uint32_t randomValue = 0U; randomValue <= 0xFFFFU; ++randomValue) {
    const uint16_t impulse =
        fmd::rainmath::impulseAmplitude(static_cast<uint16_t>(randomValue));
    TEST_ASSERT_GREATER_OR_EQUAL_UINT16(4096U, impulse);
    TEST_ASSERT_LESS_OR_EQUAL_UINT16(20479U, impulse);
  }

  TEST_ASSERT_EQUAL_UINT16(12345U, fmd::rainmath::addImpulseSaturating(10000U, 2345U));
  TEST_ASSERT_EQUAL_UINT16(0xFFFFU, fmd::rainmath::addImpulseSaturating(65000U, 1000U));
}

void test_rain_fractional_decay_eventually_moves_subcode_tail_to_zero() {
  uint16_t envelope = 1U;
  uint16_t residual = 0U;
  for (uint32_t sampleIndex = 0U; sampleIndex < 20000U && envelope != 0U; ++sampleIndex) {
    fmd::rainmath::decayEnvelope(
        fmd::rainmath::kMinDecayAlphaQ0F16, envelope, residual);
  }
  TEST_ASSERT_EQUAL_UINT16(0U, envelope);
  TEST_ASSERT_EQUAL_UINT16(0U, residual);
}

void test_rain_zero_density_is_silent_and_fixed_seed_sequence_is_deterministic() {
  fmd::RainAlgorithm silent(0x7711U);
  fmd::RainAlgorithm first(0x7711U);
  fmd::RainAlgorithm second(0x7711U);

  bool observedRain = false;
  for (uint32_t sampleIndex = 0U; sampleIndex < 10000U; ++sampleIndex) {
    TEST_ASSERT_EQUAL_UINT16(0U, silent.step({0U, 0U, 512U, 0U}));

    const fmd::ControlFrame denseControls{300U, 700U, 500U, 700U};
    const uint16_t firstOutput = first.step(denseControls);
    const uint16_t secondOutput = second.step(denseControls);
    TEST_ASSERT_EQUAL_UINT16(firstOutput, secondOutput);
    TEST_ASSERT_LESS_OR_EQUAL_UINT16(4095U, firstOutput);
    observedRain |= firstOutput != 0U;
  }
  TEST_ASSERT_TRUE(observedRain);
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_rain_density_threshold_is_quadratic_monotonic_and_sparse_at_low_settings);
  RUN_TEST(test_rain_decay_speed_mapping_is_monotonic_with_exact_endpoints);
  RUN_TEST(test_rain_impulses_are_bounded_and_saturating_accumulation_is_exact);
  RUN_TEST(test_rain_fractional_decay_eventually_moves_subcode_tail_to_zero);
  RUN_TEST(test_rain_zero_density_is_silent_and_fixed_seed_sequence_is_deterministic);
  return UNITY_END();
}
