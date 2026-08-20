/**
 * @file test_main.cpp
 * Verifies shared Speed-CV clock detection and fallback contracts.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <unity.h>

#include "fmd/domain/percussion/PercussionAlgorithmMath.h"

namespace {

fmd::percussionmath::ClockUpdate acquireClock(
    fmd::percussionmath::ClockSource& source,
    uint32_t periodSamples,
    uint32_t internalIncrement) {
  source.update(600U, internalIncrement);  // first reference edge
  for (uint32_t sample = 1U; sample < periodSamples; ++sample) {
    source.update(0U, internalIncrement);
  }
  return source.update(600U, internalIncrement);  // measured second edge
}

}  // namespace

void test_clock_uses_speed_knob_increment_until_two_valid_edges() {
  fmd::percussionmath::ClockSource source;
  constexpr uint32_t kInternal = 123456UL;

  auto update = source.update(0U, kInternal);
  TEST_ASSERT_FALSE(update.externalActive);
  TEST_ASSERT_EQUAL_UINT32(kInternal, update.quarterIncrement);

  update = source.update(600U, kInternal);
  TEST_ASSERT_FALSE(update.externalActive);
  TEST_ASSERT_FALSE(update.quarterBoundary);
  TEST_ASSERT_EQUAL_UINT32(kInternal, update.quarterIncrement);

  for (uint32_t sample = 1U; sample < 100U; ++sample) {
    update = source.update(0U, kInternal);
  }
  update = source.update(600U, kInternal);

  TEST_ASSERT_TRUE(update.externalActive);
  TEST_ASSERT_TRUE(update.externalAcquired);
  TEST_ASSERT_TRUE(update.quarterBoundary);
  TEST_ASSERT_EQUAL_UINT32(100UL, source.lastPeriodSamples());
  TEST_ASSERT_EQUAL_UINT32(
      fmd::percussionmath::quarterIncrementFromPeriodSamples(100UL),
      update.quarterIncrement);
}

void test_clock_hysteresis_requires_low_release_before_next_rising_edge() {
  fmd::percussionmath::ClockSource source;
  constexpr uint32_t kInternal = 42UL;
  source.update(fmd::percussionmath::kClockHighThresholdAdc, kInternal);

  for (uint16_t i = 0U; i < 120U; ++i) {
    const auto update = source.update(300U, kInternal);
    TEST_ASSERT_FALSE(update.externalActive);
  }
  auto update = source.update(600U, kInternal);
  TEST_ASSERT_FALSE(update.externalActive);

  source.update(fmd::percussionmath::kClockLowThresholdAdc, kInternal);
  update = source.update(600U, kInternal);
  TEST_ASSERT_TRUE(update.externalActive);
  TEST_ASSERT_TRUE(update.externalAcquired);
}

void test_steady_high_speed_cv_never_becomes_external_clock() {
  fmd::percussionmath::ClockSource source;
  constexpr uint32_t kInternal = 98765UL;
  for (uint16_t i = 0U; i < 1000U; ++i) {
    const auto update = source.update(1023U, kInternal);
    TEST_ASSERT_FALSE(update.externalActive);
    TEST_ASSERT_EQUAL_UINT32(kInternal, update.quarterIncrement);
  }
}

void test_external_clock_times_out_after_two_and_a_half_periods() {
  fmd::percussionmath::ClockSource source;
  constexpr uint32_t kInternal = 654321UL;
  auto update = acquireClock(source, 100UL, kInternal);
  TEST_ASSERT_TRUE(update.externalActive);

  const uint32_t timeout = fmd::percussionmath::clockTimeoutSamples(100UL);
  for (uint32_t sample = 0U; sample < timeout; ++sample) {
    update = source.update(0U, kInternal);
    TEST_ASSERT_TRUE(update.externalActive);
  }

  update = source.update(0U, kInternal);
  TEST_ASSERT_FALSE(update.externalActive);
  TEST_ASSERT_TRUE(update.externalLost);
  TEST_ASSERT_EQUAL_UINT32(kInternal, update.quarterIncrement);
}

void test_external_period_conversion_matches_scheduler_phase_contract() {
  TEST_ASSERT_EQUAL_UINT32(6871948UL,
                           fmd::percussionmath::quarterIncrementFromPeriodSamples(625UL));
  TEST_ASSERT_EQUAL_UINT32(250UL, fmd::percussionmath::clockTimeoutSamples(100UL));
}

void test_too_fast_glitch_does_not_replace_reference_edge() {
  fmd::percussionmath::ClockSource source;
  constexpr uint32_t kInternal = 555UL;
  source.update(600U, kInternal);
  for (uint32_t sample = 1U; sample < 10U; ++sample) {
    source.update(0U, kInternal);
  }
  auto update = source.update(600U, kInternal);
  TEST_ASSERT_FALSE(update.externalActive);

  source.update(0U, kInternal);
  for (uint32_t sample = 11U; sample < 100U; ++sample) {
    source.update(0U, kInternal);
  }
  update = source.update(600U, kInternal);
  TEST_ASSERT_TRUE(update.externalActive);
  TEST_ASSERT_EQUAL_UINT32(101UL, source.lastPeriodSamples());
}


void test_clock_period_helpers_clamp_below_and_above_supported_range() {
  TEST_ASSERT_EQUAL_UINT32(
      fmd::percussionmath::quarterIncrementFromPeriodSamples(
          fmd::percussionmath::kClockMinimumPeriodSamples),
      fmd::percussionmath::quarterIncrementFromPeriodSamples(0U));
  TEST_ASSERT_EQUAL_UINT32(
      fmd::percussionmath::quarterIncrementFromPeriodSamples(
          fmd::percussionmath::kClockMaximumPeriodSamples),
      fmd::percussionmath::quarterIncrementFromPeriodSamples(UINT32_MAX));
  TEST_ASSERT_EQUAL_UINT32(
      fmd::percussionmath::clockTimeoutSamples(
          fmd::percussionmath::kClockMinimumPeriodSamples),
      fmd::percussionmath::clockTimeoutSamples(0U));
  TEST_ASSERT_EQUAL_UINT32(
      fmd::percussionmath::clockTimeoutSamples(
          fmd::percussionmath::kClockMaximumPeriodSamples),
      fmd::percussionmath::clockTimeoutSamples(UINT32_MAX));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_clock_uses_speed_knob_increment_until_two_valid_edges);
  RUN_TEST(test_clock_hysteresis_requires_low_release_before_next_rising_edge);
  RUN_TEST(test_steady_high_speed_cv_never_becomes_external_clock);
  RUN_TEST(test_external_clock_times_out_after_two_and_a_half_periods);
  RUN_TEST(test_external_period_conversion_matches_scheduler_phase_contract);
  RUN_TEST(test_too_fast_glitch_does_not_replace_reference_edge);
  RUN_TEST(test_clock_period_helpers_clamp_below_and_above_supported_range);
  return UNITY_END();
}
