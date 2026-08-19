/**
 * @file test_main.cpp
 * Verifies the fixed-voice filtered-event contract of Ambient Fog.
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
#include "fmd/domain/AmbientAlgorithmMath.h"
#include "fmd/domain/FogAlgorithm.h"

void test_fog_kernel_has_exact_endpoints_peak_and_symmetry() {
  TEST_ASSERT_EQUAL_UINT16(0U, fmd::fogmath::kernelQ0F12(0U));
  TEST_ASSERT_EQUAL_UINT16(4096U, fmd::fogmath::kernelQ0F12(2048U));
  TEST_ASSERT_EQUAL_UINT16(0U, fmd::fogmath::kernelQ0F12(4095U));
  for (uint16_t phase = 1U; phase < 2048U; phase += 37U) {
    TEST_ASSERT_UINT16_WITHIN(
        5U,
        fmd::fogmath::kernelQ0F12(phase),
        fmd::fogmath::kernelQ0F12(static_cast<uint16_t>(4096U - phase)));
  }
}

void test_fog_occupancy_mapping_is_monotonic_and_below_voice_cap() {
  TEST_ASSERT_EQUAL_UINT8(1U, fmd::fogmath::targetOccupancyEighths(0U));
  TEST_ASSERT_EQUAL_UINT8(24U, fmd::fogmath::targetOccupancyEighths(1023U));
  uint8_t previous = 0U;
  for (uint16_t texture = 0U; texture <= 1023U; ++texture) {
    const uint8_t current = fmd::fogmath::targetOccupancyEighths(texture);
    TEST_ASSERT_GREATER_OR_EQUAL_UINT8(previous, current);
    TEST_ASSERT_LESS_THAN_UINT8(33U, current);
    previous = current;
  }
}

void test_fog_event_cutoff_compensates_duration_linearly() {
  TEST_ASSERT_EQUAL_UINT32(
      100000UL, fmd::fogmath::eventCutoffQ0F32(800000UL, 1U));
  TEST_ASSERT_EQUAL_UINT32(
      2400000UL, fmd::fogmath::eventCutoffQ0F32(800000UL, 24U));
}

void test_fog_amplitude_mapping_is_signed_and_bounded() {
  for (uint32_t random = 0U; random <= 65535U; random += 113U) {
    const int16_t amplitude =
        fmd::fogmath::amplitudeFromRandom(static_cast<uint16_t>(random));
    TEST_ASSERT_TRUE(
        (amplitude >= 512 && amplitude <= 1023) ||
        (amplitude <= -512 && amplitude >= -1023));
  }
}

void test_fog_voice_finishes_and_becomes_free() {
  fmd::fogmath::Voice voice{0xFFFFFF00UL, 700, true};
  (void)fmd::fogmath::voiceContributionAndAdvance(voice, 0x1000UL);
  TEST_ASSERT_FALSE(voice.active);
  TEST_ASSERT_EQUAL_UINT32(0U, voice.phase);
}

void test_fog_algorithm_is_seed_deterministic_and_bounded() {
  MemoryReferenceTables tables;
  fmd::FogAlgorithm first(tables, 0xBEEFU);
  fmd::FogAlgorithm second(tables, 0xBEEFU);
  const fmd::ControlFrame controls{800U, 1023U, 900U, 1023U};
  for (uint32_t sample = 0U; sample < 30000U; ++sample) {
    const uint16_t firstValue = first.step(controls);
    const uint16_t secondValue = second.step(controls);
    TEST_ASSERT_EQUAL_UINT16(firstValue, secondValue);
    TEST_ASSERT_LESS_OR_EQUAL_UINT16(4095U, firstValue);
  }
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_fog_kernel_has_exact_endpoints_peak_and_symmetry);
  RUN_TEST(test_fog_occupancy_mapping_is_monotonic_and_below_voice_cap);
  RUN_TEST(test_fog_event_cutoff_compensates_duration_linearly);
  RUN_TEST(test_fog_amplitude_mapping_is_signed_and_bounded);
  RUN_TEST(test_fog_voice_finishes_and_becomes_free);
  RUN_TEST(test_fog_algorithm_is_seed_deterministic_and_bounded);
  return UNITY_END();
}
