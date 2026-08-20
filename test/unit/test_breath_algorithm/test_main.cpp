/**
 * @file test_main.cpp
 * Verifies the cycle topology and bounded variation contract of Ambient Breath.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <unity.h>

#include "MemoryReferenceTables.h"
#include "fmd/domain/ambient/AmbientAlgorithmMath.h"
#include "fmd/domain/ambient/BreathAlgorithm.h"

void test_breath_zero_texture_parameter_mapping_is_nominal() {
  constexpr uint16_t samples[] = {0U, 1U, 32768U, 65535U};
  for (const uint16_t randomWord : samples) {
    TEST_ASSERT_EQUAL_UINT16(
        fmd::breathmath::kDurationNominalQ10,
        fmd::breathmath::variedParameter(
            randomWord, 0U, 768U, 1024U, 1280U));
    TEST_ASSERT_EQUAL_UINT16(
        fmd::breathmath::kSkewNominalQ0F12,
        fmd::breathmath::variedParameter(
            randomWord, 0U, 1024U, 1536U, 2048U));
  }
}

void test_breath_full_texture_parameters_never_leave_bounds() {
  for (uint32_t random = 0U; random <= 65535U; random += 257U) {
    const uint16_t duration = fmd::breathmath::variedParameter(
        static_cast<uint16_t>(random), 1023U, 768U, 1024U, 1280U);
    const uint16_t amplitude = fmd::breathmath::variedParameter(
        static_cast<uint16_t>(random), 1023U, 2662U, 3378U, 4095U);
    const uint16_t skew = fmd::breathmath::variedParameter(
        static_cast<uint16_t>(random), 1023U, 1024U, 1536U, 2048U);
    TEST_ASSERT_TRUE(duration >= 768U && duration <= 1280U);
    TEST_ASSERT_TRUE(amplitude >= 2662U && amplitude <= 4095U);
    TEST_ASSERT_TRUE(skew >= 1024U && skew <= 2048U);
  }
}

void test_breath_envelope_has_baseline_peak_baseline_topology() {
  constexpr uint16_t skew = fmd::breathmath::kSkewNominalQ0F12;
  const uint16_t attack = fmd::breathmath::segmentReciprocalQ12(skew);
  const uint16_t release =
      fmd::breathmath::segmentReciprocalQ12(4096U - skew);
  TEST_ASSERT_EQUAL_UINT16(
      0U, fmd::breathmath::envelopeQ0F12(0U, skew, attack, release));
  TEST_ASSERT_EQUAL_UINT16(
      4096U, fmd::breathmath::envelopeQ0F12(skew, skew, attack, release));
  TEST_ASSERT_LESS_OR_EQUAL_UINT16(
      2U, fmd::breathmath::envelopeQ0F12(4095U, skew, attack, release));
}

void test_breath_rate_scale_matches_documented_duration_range() {
  TEST_ASSERT_UINT16_WITHIN(1U, 1365U, fmd::breathmath::rateScaleQ10(768U));
  TEST_ASSERT_EQUAL_UINT16(1024U, fmd::breathmath::rateScaleQ10(1024U));
  TEST_ASSERT_UINT16_WITHIN(1U, 819U, fmd::breathmath::rateScaleQ10(1280U));
}

void test_breath_algorithm_is_seed_deterministic_and_bounded() {
  MemoryReferenceTables tables;
  fmd::BreathAlgorithm first(tables, 0xCAFEU);
  fmd::BreathAlgorithm second(tables, 0xCAFEU);
  const fmd::ControlFrame controls{700U, 900U, 900U, 900U};
  for (uint32_t sample = 0U; sample < 20000U; ++sample) {
    const uint16_t firstValue = first.step(controls);
    const uint16_t secondValue = second.step(controls);
    TEST_ASSERT_EQUAL_UINT16(firstValue, secondValue);
    TEST_ASSERT_LESS_OR_EQUAL_UINT16(4095U, firstValue);
  }
}


void test_breath_math_clamps_out_of_contract_inputs() {
  TEST_ASSERT_EQUAL_UINT16(
      fmd::breathmath::rateScaleQ10(fmd::breathmath::kDurationMinimumQ10),
      fmd::breathmath::rateScaleQ10(0U));
  TEST_ASSERT_EQUAL_UINT16(
      fmd::breathmath::rateScaleQ10(fmd::breathmath::kDurationMaximumQ10),
      fmd::breathmath::rateScaleQ10(65535U));
  TEST_ASSERT_EQUAL_UINT16(0U, fmd::breathmath::scaledPhaseIncrement(12345U, 0U));
  TEST_ASSERT_EQUAL_UINT16(0U, fmd::breathmath::segmentReciprocalQ12(0U));

  TEST_ASSERT_LESS_OR_EQUAL_UINT16(
      4096U, fmd::breathmath::envelopeQ0F12(65535U, 0U, 65535U, 65535U));
  TEST_ASSERT_LESS_OR_EQUAL_UINT16(
      4096U, fmd::breathmath::envelopeQ0F12(4095U, 65535U, 65535U, 65535U));
  TEST_ASSERT_EQUAL_UINT16(
      4095U, fmd::breathmath::applyAmplitude(65535U, 65535U));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_breath_zero_texture_parameter_mapping_is_nominal);
  RUN_TEST(test_breath_full_texture_parameters_never_leave_bounds);
  RUN_TEST(test_breath_envelope_has_baseline_peak_baseline_topology);
  RUN_TEST(test_breath_rate_scale_matches_documented_duration_range);
  RUN_TEST(test_breath_algorithm_is_seed_deterministic_and_bounded);
  RUN_TEST(test_breath_math_clamps_out_of_contract_inputs);
  return UNITY_END();
}
