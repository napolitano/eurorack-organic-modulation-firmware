#include <unity.h>

#include <cstdint>

#include "fmd/domain/AlgorithmMath.h"
#include "fmd/domain/BrownianAlgorithm.h"

void test_brownian_step_size_and_event_probability_follow_documented_linear_laws() {
  for (uint16_t speed = 0U; speed <= 1023U; ++speed) {
    TEST_ASSERT_EQUAL_UINT16(static_cast<uint16_t>((256U + speed) >> 1U),
                             fmd::brownianmath::stepSize(speed));
    TEST_ASSERT_EQUAL_UINT16(static_cast<uint16_t>(speed << 6U),
                             fmd::brownianmath::eventCutoff(speed));
  }
}

void test_brownian_centering_bias_has_exact_conditional_thresholds() {
  const uint16_t cutoff = fmd::brownianmath::eventCutoff(512U);
  TEST_ASSERT_EQUAL_UINT16(static_cast<uint16_t>(cutoff / 2U - cutoff / 64U),
                           fmd::brownianmath::directionCutoff(0U, cutoff));
  TEST_ASSERT_EQUAL_UINT16(static_cast<uint16_t>(cutoff / 2U),
                           fmd::brownianmath::directionCutoff(32768U, cutoff));
  TEST_ASSERT_EQUAL_UINT16(static_cast<uint16_t>(cutoff / 2U + cutoff / 64U),
                           fmd::brownianmath::directionCutoff(65535U, cutoff));
}

void test_brownian_exhaustive_uniform_random_domain_matches_event_and_direction_probabilities() {
  constexpr uint16_t speed = 512U;
  constexpr uint16_t target = 32768U;
  const uint16_t cutoff = fmd::brownianmath::eventCutoff(speed);
  const uint16_t split = fmd::brownianmath::directionCutoff(target, cutoff);
  uint32_t unchanged = 0U;
  uint32_t down = 0U;
  uint32_t up = 0U;

  for (uint32_t r = 0U; r <= 0xFFFFU; ++r) {
    const uint16_t next = fmd::brownianmath::nextTarget(target, static_cast<uint16_t>(r), speed);
    if (next == target) {
      ++unchanged;
    } else if (next < target) {
      ++down;
    } else {
      ++up;
    }
  }

  TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(0x10000UL - cutoff), unchanged);
  TEST_ASSERT_EQUAL_UINT32(split, down);
  TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(cutoff - split), up);
}

void test_brownian_target_update_saturates_at_numeric_boundaries() {
  TEST_ASSERT_EQUAL_UINT16(0U, fmd::brownianmath::nextTarget(1U, 0U, 1023U));
  const uint16_t cutoff = fmd::brownianmath::eventCutoff(1023U);
  const uint16_t split = fmd::brownianmath::directionCutoff(65534U, cutoff);
  TEST_ASSERT_EQUAL_UINT16(65535U,
                           fmd::brownianmath::nextTarget(65534U, split, 1023U));
}

void test_brownian_texture_alpha_spans_declared_range_monotonically() {
  TEST_ASSERT_EQUAL_UINT16(fmd::brownianmath::kMinAlphaQ0F16,
                           fmd::brownianmath::textureAlphaQ0F16(0U));
  TEST_ASSERT_EQUAL_UINT16(fmd::brownianmath::kMaxAlphaQ0F16,
                           fmd::brownianmath::textureAlphaQ0F16(1023U));

  uint16_t previous = 0U;
  for (uint16_t texture = 0U; texture <= 1023U; ++texture) {
    const uint16_t alpha = fmd::brownianmath::textureAlphaQ0F16(texture);
    TEST_ASSERT_GREATER_OR_EQUAL_UINT16(previous, alpha);
    previous = alpha;
  }
}

void test_brownian_texture_alpha_matches_full_range_linear_interpolation() {
  constexpr uint16_t points[] = {0U, 1U, 127U, 511U, 512U, 1019U, 1020U, 1023U};
  constexpr uint32_t range = static_cast<uint32_t>(fmd::brownianmath::kMaxAlphaQ0F16)
                           - fmd::brownianmath::kMinAlphaQ0F16;
  for (const uint16_t texture : points) {
    const uint16_t expected = static_cast<uint16_t>(fmd::brownianmath::kMinAlphaQ0F16
        + ((static_cast<uint32_t>(texture) * range + 511U) / 1023U));
    TEST_ASSERT_EQUAL_UINT16(expected, fmd::brownianmath::textureAlphaQ0F16(texture));
  }
}

void test_brownian_texture_alpha_clamps_out_of_range_input() {
  TEST_ASSERT_EQUAL_UINT16(fmd::brownianmath::textureAlphaQ0F16(1023U),
                           fmd::brownianmath::textureAlphaQ0F16(0xFFFFU));
}

void test_brownian_fractional_residual_eliminates_one_code_deadband() {
  uint16_t current = 0U;
  uint16_t residual = 0U;
  int8_t direction = 0;
  const uint16_t alpha = fmd::brownianmath::textureAlphaQ0F16(0U);

  for (uint32_t i = 0U; i < 5000U && current == 0U; ++i) {
    fmd::brownianmath::smoothToward(1U, alpha, current, residual, direction);
  }
  TEST_ASSERT_EQUAL_UINT16(1U, current);
  TEST_ASSERT_EQUAL_UINT16(0U, residual);
  TEST_ASSERT_EQUAL_INT8(0, direction);
}

void test_brownian_smoother_matches_first_order_step_before_fractional_carry() {
  uint16_t current = 1000U;
  uint16_t residual = 0U;
  int8_t direction = 0;
  constexpr uint16_t target = 50000U;
  const uint16_t alpha = fmd::brownianmath::textureAlphaQ0F16(512U);
  const uint16_t delta = static_cast<uint16_t>(target - current);
  const uint16_t expectedMove = static_cast<uint16_t>((static_cast<uint32_t>(alpha) * delta) >> 16U);
  fmd::brownianmath::smoothToward(target, alpha, current, residual, direction);
  TEST_ASSERT_EQUAL_UINT16(static_cast<uint16_t>(1000U + expectedMove), current);
  TEST_ASSERT_EQUAL_INT8(1, direction);
}


void test_brownian_centering_zones_change_only_outside_exact_boundaries() {
  constexpr uint16_t cutoff = 32768U;
  constexpr uint16_t lowBoundary = static_cast<uint16_t>(0xFFFFU / fmd::brownianmath::kCenteringMargin);
  constexpr uint16_t highBoundary = static_cast<uint16_t>(0xFFFFU - lowBoundary);
  const uint16_t neutral = static_cast<uint16_t>(cutoff / 2U);
  const uint16_t lowBiased = static_cast<uint16_t>(neutral - cutoff / fmd::brownianmath::kCenteringStrength);
  const uint16_t highBiased = static_cast<uint16_t>(neutral + cutoff / fmd::brownianmath::kCenteringStrength);

  TEST_ASSERT_EQUAL_UINT16(lowBiased,
                           fmd::brownianmath::directionCutoff(static_cast<uint16_t>(lowBoundary - 1U), cutoff));
  TEST_ASSERT_EQUAL_UINT16(neutral, fmd::brownianmath::directionCutoff(lowBoundary, cutoff));
  TEST_ASSERT_EQUAL_UINT16(neutral, fmd::brownianmath::directionCutoff(highBoundary, cutoff));
  TEST_ASSERT_EQUAL_UINT16(highBiased,
                           fmd::brownianmath::directionCutoff(static_cast<uint16_t>(highBoundary + 1U), cutoff));
}

void test_brownian_smoother_matches_first_order_step_in_descending_direction() {
  uint16_t current = 50000U;
  uint16_t residual = 0U;
  int8_t direction = 0;
  constexpr uint16_t target = 1000U;
  const uint16_t alpha = fmd::brownianmath::textureAlphaQ0F16(512U);
  const uint16_t delta = static_cast<uint16_t>(current - target);
  const uint16_t expectedMove = static_cast<uint16_t>((static_cast<uint32_t>(alpha) * delta) >> 16U);
  fmd::brownianmath::smoothToward(target, alpha, current, residual, direction);
  TEST_ASSERT_EQUAL_UINT16(static_cast<uint16_t>(50000U - expectedMove), current);
  TEST_ASSERT_EQUAL_INT8(-1, direction);
}

void test_brownian_smoother_resets_fractional_residual_when_direction_reverses() {
  uint16_t current = 10000U;
  uint16_t residual = 60000U;
  int8_t direction = 1;
  const uint16_t alpha = fmd::brownianmath::textureAlphaQ0F16(256U);
  const uint16_t delta = 5000U;
  const uint16_t expectedMove = static_cast<uint16_t>((static_cast<uint32_t>(alpha) * delta) >> 16U);

  fmd::brownianmath::smoothToward(5000U, alpha, current, residual, direction);
  TEST_ASSERT_EQUAL_INT8(-1, direction);
  TEST_ASSERT_EQUAL_UINT16(static_cast<uint16_t>(10000U - expectedMove), current);
}

void test_brownian_smoother_converges_to_target_without_overshoot_and_clears_residual() {
  uint16_t current = 60000U;
  uint16_t residual = 0U;
  int8_t direction = 0;
  constexpr uint16_t target = 12345U;
  const uint16_t alpha = fmd::brownianmath::textureAlphaQ0F16(1023U);
  uint16_t previous = current;

  for (uint32_t i = 0U; i < 20000U && current != target; ++i) {
    fmd::brownianmath::smoothToward(target, alpha, current, residual, direction);
    TEST_ASSERT_LESS_OR_EQUAL_UINT16(previous, current);
    TEST_ASSERT_GREATER_OR_EQUAL_UINT16(target, current);
    previous = current;
  }

  TEST_ASSERT_EQUAL_UINT16(target, current);
  TEST_ASSERT_EQUAL_UINT16(0U, residual);
  TEST_ASSERT_EQUAL_INT8(0, direction);
}

void test_brownian_long_run_centering_keeps_walk_away_from_numeric_rails() {
  fmd::BrownianAlgorithm algorithm(0x6D3BU);
  const fmd::ControlFrame controls{1023U, 1023U, 1023U, 1023U};
  uint64_t sum = 0U;
  constexpr uint32_t burnIn = 4000U;
  constexpr uint32_t samples = 20000U;

  for (uint32_t i = 0U; i < burnIn; ++i) {
    (void)algorithm.step(controls);
  }
  for (uint32_t i = 0U; i < samples; ++i) {
    sum += algorithm.step(controls);
  }

  const uint32_t mean = static_cast<uint32_t>(sum / samples);
  TEST_ASSERT_GREATER_THAN_UINT32(900U, mean);
  TEST_ASSERT_LESS_THAN_UINT32(3200U, mean);
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_brownian_step_size_and_event_probability_follow_documented_linear_laws);
  RUN_TEST(test_brownian_centering_bias_has_exact_conditional_thresholds);
  RUN_TEST(test_brownian_exhaustive_uniform_random_domain_matches_event_and_direction_probabilities);
  RUN_TEST(test_brownian_target_update_saturates_at_numeric_boundaries);
  RUN_TEST(test_brownian_texture_alpha_spans_declared_range_monotonically);
  RUN_TEST(test_brownian_texture_alpha_matches_full_range_linear_interpolation);
  RUN_TEST(test_brownian_texture_alpha_clamps_out_of_range_input);
  RUN_TEST(test_brownian_fractional_residual_eliminates_one_code_deadband);
  RUN_TEST(test_brownian_smoother_matches_first_order_step_before_fractional_carry);
  RUN_TEST(test_brownian_centering_zones_change_only_outside_exact_boundaries);
  RUN_TEST(test_brownian_smoother_matches_first_order_step_in_descending_direction);
  RUN_TEST(test_brownian_smoother_resets_fractional_residual_when_direction_reverses);
  RUN_TEST(test_brownian_smoother_converges_to_target_without_overshoot_and_clears_residual);
  RUN_TEST(test_brownian_long_run_centering_keeps_walk_away_from_numeric_rails);
  return UNITY_END();
}
