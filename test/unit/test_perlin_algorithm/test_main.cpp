#include <unity.h>

#include <cmath>
#include <cstdint>

#include "fmd/domain/AlgorithmMath.h"

namespace {
double fadeReference(double t) {
  return 6.0 * t * t * t * t * t - 15.0 * t * t * t * t + 10.0 * t * t * t;
}

double segmentReference(double x, double g0, double g1) {
  const double a = g0 * x;
  const double b = g1 * (x - 1.0);
  const double u = fadeReference(x);
  return a + u * (b - a);
}
}

void test_perlin_fade_has_exact_fixed_point_endpoints_and_midpoint() {
  TEST_ASSERT_EQUAL_UINT16(0U, fmd::perlinmath::fadeQ0F16(0U));
  TEST_ASSERT_UINT16_WITHIN(16U, 32768U, fmd::perlinmath::fadeQ0F16(32768U));
  TEST_ASSERT_UINT16_WITHIN(32U, 0xFFFFU, fmd::perlinmath::fadeQ0F16(0xFFFFU));
}

void test_perlin_fade_is_monotonic_over_complete_q0f16_domain() {
  uint16_t previous = 0U;
  for (uint32_t raw = 0U; raw <= 0xFFFFU; ++raw) {
    const uint16_t current = fmd::perlinmath::fadeQ0F16(static_cast<uint16_t>(raw));
    TEST_ASSERT_GREATER_OR_EQUAL_UINT16(previous, current);
    previous = current;
  }
}

void test_perlin_fade_matches_quintic_reference() {
  for (uint32_t raw = 0U; raw <= 0xFFFFU; raw += 257U) {
    const double t = static_cast<double>(raw) / 65536.0;
    const double expected = fadeReference(t);
    const double actual = static_cast<double>(fmd::perlinmath::fadeQ0F16(static_cast<uint16_t>(raw))) / 65536.0;
    TEST_ASSERT_DOUBLE_WITHIN(0.0012, expected, actual);
  }
}

void test_perlin_fade_obeys_complement_symmetry_with_fixed_point_tolerance() {
  for (uint32_t raw = 0U; raw <= 0xFFFFU; raw += 521U) {
    const uint16_t a = fmd::perlinmath::fadeQ0F16(static_cast<uint16_t>(raw));
    const uint16_t b = fmd::perlinmath::fadeQ0F16(static_cast<uint16_t>(0xFFFFU - raw));
    const uint32_t sum = static_cast<uint32_t>(a) + static_cast<uint32_t>(b);
    TEST_ASSERT_UINT32_WITHIN(96U, 0xFFFFU, sum);
  }
}

void test_perlin_segment_matches_gradient_noise_reference() {
  constexpr int16_t gradients[] = {-16384, -8192, -2048, 2048, 8192, 16384};
  constexpr uint16_t phases[] = {0U, 4096U, 16384U, 32768U, 49152U, 61440U, 65535U};

  for (const int16_t g0 : gradients) {
    for (const int16_t g1 : gradients) {
      for (const uint16_t phase : phases) {
        const double x = static_cast<double>(phase) / 65536.0;
        const double expected = segmentReference(x,
                                                 static_cast<double>(g0) / 32768.0,
                                                 static_cast<double>(g1) / 32768.0);
        const double actual = static_cast<double>(fmd::perlinmath::segmentQ1F15(phase, g0, g1)) / 32768.0;
        TEST_ASSERT_DOUBLE_WITHIN(0.0015, expected, actual);
      }
    }
  }
}

void test_perlin_segment_is_zero_at_integer_lattice_boundary() {
  constexpr int16_t gradients[] = {-16384, -4096, 4096, 16384};
  for (const int16_t g0 : gradients) {
    for (const int16_t g1 : gradients) {
      TEST_ASSERT_INT16_WITHIN(1, 0, fmd::perlinmath::segmentQ1F15(0U, g0, g1));
    }
  }
}


void test_perlin_gradient_mapping_matches_complete_16_value_gradient_set() {
  for (uint16_t h = 0U; h < 8U; ++h) {
    const int16_t expected = static_cast<int16_t>((h + 1U) << 11U);
    TEST_ASSERT_EQUAL_INT16(expected, fmd::perlinmath::gradientFromRandom(h));
    TEST_ASSERT_EQUAL_INT16(static_cast<int16_t>(-expected),
                            fmd::perlinmath::gradientFromRandom(static_cast<uint16_t>(h | 8U)));
    TEST_ASSERT_EQUAL_INT16(expected,
                            fmd::perlinmath::gradientFromRandom(static_cast<uint16_t>(0xFFF0U | h)));
  }
}

void test_perlin_segment_has_value_and_slope_continuity_across_lattice_handoff() {
  constexpr int16_t outerGradients[] = {-16384, -4096, 4096, 16384};
  constexpr int16_t sharedGradients[] = {-12288, -2048, 2048, 12288};
  constexpr uint16_t epsilon = 16U;

  for (const int16_t g0 : outerGradients) {
    for (const int16_t shared : sharedGradients) {
      for (const int16_t g2 : outerGradients) {
        const int16_t left = fmd::perlinmath::segmentQ1F15(
            static_cast<uint16_t>(0x10000UL - epsilon), g0, shared);
        const int16_t right = fmd::perlinmath::segmentQ1F15(epsilon, shared, g2);
        TEST_ASSERT_INT16_WITHIN(6, 0, static_cast<int16_t>(left + right));
      }
    }
  }
}

void test_perlin_phase_wrap_preserves_overshoot_and_flags_only_on_wrap() {
  bool rollover = true;
  TEST_ASSERT_EQUAL_UINT32(0x12346678UL,
                           fmd::perlinmath::advancePhase(0x12345678UL, 0x1000UL, rollover));
  TEST_ASSERT_FALSE(rollover);

  const uint32_t wrapped = fmd::perlinmath::advancePhase(0xFFFFFF00UL, 0x250UL, rollover);
  TEST_ASSERT_TRUE(rollover);
  TEST_ASSERT_EQUAL_UINT32(0x00000150UL, wrapped);
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_perlin_fade_has_exact_fixed_point_endpoints_and_midpoint);
  RUN_TEST(test_perlin_fade_is_monotonic_over_complete_q0f16_domain);
  RUN_TEST(test_perlin_fade_matches_quintic_reference);
  RUN_TEST(test_perlin_fade_obeys_complement_symmetry_with_fixed_point_tolerance);
  RUN_TEST(test_perlin_segment_matches_gradient_noise_reference);
  RUN_TEST(test_perlin_segment_is_zero_at_integer_lattice_boundary);
  RUN_TEST(test_perlin_gradient_mapping_matches_complete_16_value_gradient_set);
  RUN_TEST(test_perlin_segment_has_value_and_slope_continuity_across_lattice_handoff);
  RUN_TEST(test_perlin_phase_wrap_preserves_overshoot_and_flags_only_on_wrap);
  return UNITY_END();
}
