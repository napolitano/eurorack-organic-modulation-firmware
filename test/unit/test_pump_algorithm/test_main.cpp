/**
 * @file test_main.cpp
 * Verifies the Electronica Pump mathematical and integration contracts.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <unity.h>

#include "fmd/domain/electronica/ElectronicaAlgorithmMath.h"
#include "fmd/domain/FrequencyMapping.h"
#include "fmd/domain/electronica/PumpAlgorithm.h"
#include "MemoryReferenceTables.h"

void test_electronica_tempo_mapping_has_exact_endpoints() {
  MemoryReferenceTables tables;
  const uint32_t minimum = fmd::electronicamath::quarterNotePhaseIncrement(tables, 0U, 0U);
  const uint32_t maximum = fmd::electronicamath::quarterNotePhaseIncrement(tables, 1023U, 1023U);
  TEST_ASSERT_EQUAL_UINT32(fmd::phaseIncrementFromDecihertzQ16_16(20UL << 16U), minimum);
  TEST_ASSERT_EQUAL_UINT32(fmd::phaseIncrementFromDecihertzQ16_16(160UL << 16U), maximum);
}

void test_pump_recovery_endpoint_has_exact_texture_endpoints() {
  TEST_ASSERT_EQUAL_UINT16(fmd::pumpmath::kRecoveryMinimumQ0F16,
                           fmd::pumpmath::recoveryEndpointQ0F16(0U));
  TEST_ASSERT_EQUAL_UINT16(fmd::pumpmath::kRecoveryMaximumQ0F16,
                           fmd::pumpmath::recoveryEndpointQ0F16(1023U));
}

void test_pump_output_starts_at_zero_and_reaches_full_scale() {
  const uint16_t textures[] = {0U, 127U, 511U, 1023U};
  for (uint16_t texture : textures) {
    const uint16_t endpoint = fmd::pumpmath::recoveryEndpointQ0F16(texture);
    const uint16_t reciprocal = fmd::pumpmath::recoveryReciprocalQ28(endpoint);
    TEST_ASSERT_EQUAL_UINT16(0U, fmd::pumpmath::outputDac12(0U, endpoint, reciprocal));
    TEST_ASSERT_EQUAL_UINT16(4095U, fmd::pumpmath::outputDac12(endpoint, endpoint, reciprocal));
    TEST_ASSERT_EQUAL_UINT16(4095U, fmd::pumpmath::outputDac12(65535U, endpoint, reciprocal));
  }
}

void test_pump_recovery_is_monotone_for_dense_phase_domain() {
  const uint16_t endpoint = fmd::pumpmath::recoveryEndpointQ0F16(777U);
  const uint16_t reciprocal = fmd::pumpmath::recoveryReciprocalQ28(endpoint);
  uint16_t previous = 0U;
  for (uint32_t phase = 0U; phase <= endpoint; phase += 17U) {
    const uint16_t current = fmd::pumpmath::outputDac12(
        static_cast<uint16_t>(phase), endpoint, reciprocal);
    TEST_ASSERT_GREATER_OR_EQUAL_UINT16(previous, current);
    previous = current;
  }
}

void test_pump_algorithm_is_deterministic_and_bounded() {
  MemoryReferenceTables tables;
  fmd::PumpAlgorithm first(tables);
  fmd::PumpAlgorithm second(tables);
  for (uint16_t i = 0U; i < 4000U; ++i) {
    const fmd::ControlFrame controls{
        static_cast<uint16_t>((i * 17U) & 1023U),
        static_cast<uint16_t>((i * 23U) & 1023U),
        static_cast<uint16_t>((i * 31U) & 1023U),
        static_cast<uint16_t>((i * 47U) & 1023U)};
    const uint16_t a = first.step(controls);
    const uint16_t b = second.step(controls);
    TEST_ASSERT_EQUAL_UINT16(a, b);
    TEST_ASSERT_LESS_OR_EQUAL_UINT16(4095U, a);
  }
}


void test_pump_math_clamps_peak_endpoint_and_completed_recovery() {
  TEST_ASSERT_EQUAL_UINT16(4095U, fmd::electronicamath::scaleDac12(65535U, 4096U));
  TEST_ASSERT_EQUAL_UINT16(
      fmd::pumpmath::recoveryReciprocalQ28(fmd::pumpmath::kRecoveryMinimumQ0F16),
      fmd::pumpmath::recoveryReciprocalQ28(0U));
  const uint16_t endpoint = fmd::pumpmath::kRecoveryMinimumQ0F16;
  TEST_ASSERT_EQUAL_UINT16(
      4096U,
      fmd::pumpmath::recoveryProgressQ0F12(endpoint, endpoint,
          fmd::pumpmath::recoveryReciprocalQ28(endpoint)));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_electronica_tempo_mapping_has_exact_endpoints);
  RUN_TEST(test_pump_recovery_endpoint_has_exact_texture_endpoints);
  RUN_TEST(test_pump_output_starts_at_zero_and_reaches_full_scale);
  RUN_TEST(test_pump_recovery_is_monotone_for_dense_phase_domain);
  RUN_TEST(test_pump_algorithm_is_deterministic_and_bounded);
  RUN_TEST(test_pump_math_clamps_peak_endpoint_and_completed_recovery);
  return UNITY_END();
}
