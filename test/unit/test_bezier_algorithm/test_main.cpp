/**
 * @file test_main.cpp
 * Implements the Bézier mathematical verification native test suite.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <unity.h>

#include <cmath>
#include <cstdint>

#include "fmd/domain/AlgorithmMath.h"
#include "fmd/domain/BezierAlgorithm.h"
#include "fmd/domain/FrequencyMapping.h"
#include "fmd/domain/ParallelLfsr.h"
#include "MemoryReferenceTables.h"

namespace {
double smoothReference(double x) { return 3.0 * x * x - 2.0 * x * x * x; }
double reverseReference(double x) { return 2.0 * x * x * x + 2.0 * x - 3.0 * x * x; }
double triangularIcdfReference(double p) {
  if (p <= 0.0) return -1.0;
  if (p >= 1.0) return 1.0;
  return p < 0.5 ? -1.0 + std::sqrt(2.0 * p) : 1.0 - std::sqrt(2.0 * (1.0 - p));
}
}

void test_bezier_special_curves_have_exact_endpoints_and_midpoint() {
  TEST_ASSERT_EQUAL_UINT16(0U, fmd::beziermath::smoothCurveQ4F12(0U));
  TEST_ASSERT_EQUAL_UINT16(4096U, fmd::beziermath::smoothCurveQ4F12(4096U));
  TEST_ASSERT_EQUAL_UINT16(2048U, fmd::beziermath::smoothCurveQ4F12(2048U));
  TEST_ASSERT_EQUAL_UINT16(0U, fmd::beziermath::reverseCurveQ4F12(0U));
  TEST_ASSERT_EQUAL_UINT16(4096U, fmd::beziermath::reverseCurveQ4F12(4096U));
  TEST_ASSERT_EQUAL_UINT16(2048U, fmd::beziermath::reverseCurveQ4F12(2048U));
}

void test_bezier_special_curves_match_cubic_reference() {
  for (uint16_t raw = 0U; raw <= 4096U; raw = static_cast<uint16_t>(raw + 16U)) {
    const double x = static_cast<double>(raw) / 4096.0;
    const double smooth = static_cast<double>(fmd::beziermath::smoothCurveQ4F12(raw)) / 4096.0;
    const double reverse = static_cast<double>(fmd::beziermath::reverseCurveQ4F12(raw)) / 4096.0;
    TEST_ASSERT_DOUBLE_WITHIN(0.0015, smoothReference(x), smooth);
    TEST_ASSERT_DOUBLE_WITHIN(0.0015, reverseReference(x), reverse);
  }
}

void test_bezier_special_curves_are_monotonic() {
  uint16_t previousSmooth = 0U;
  uint16_t previousReverse = 0U;
  for (uint16_t raw = 0U; raw <= 4096U; ++raw) {
    const uint16_t smooth = fmd::beziermath::smoothCurveQ4F12(raw);
    const uint16_t reverse = fmd::beziermath::reverseCurveQ4F12(raw);
    TEST_ASSERT_GREATER_OR_EQUAL_UINT16(previousSmooth, smooth);
    TEST_ASSERT_GREATER_OR_EQUAL_UINT16(previousReverse, reverse);
    previousSmooth = smooth;
    previousReverse = reverse;
  }
}

void test_bezier_texture_morph_has_exact_curve_endpoints_and_near_linear_center() {
  for (uint16_t phase = 0U; phase <= 4096U; phase = static_cast<uint16_t>(phase + 64U)) {
    TEST_ASSERT_EQUAL_UINT16(fmd::beziermath::reverseCurveQ4F12(phase),
                             fmd::beziermath::morphCurveQ4F12(phase, 0U));
    TEST_ASSERT_EQUAL_UINT16(fmd::beziermath::smoothCurveQ4F12(phase),
                             fmd::beziermath::morphCurveQ4F12(phase, 1023U));
    TEST_ASSERT_UINT16_WITHIN(3U, phase, fmd::beziermath::morphCurveQ4F12(phase, 512U));
  }
}

void test_bezier_texture_morph_is_continuous_across_former_511_512_boundary() {
  for (uint16_t phase = 0U; phase <= 4096U; phase = static_cast<uint16_t>(phase + 32U)) {
    const uint16_t a = fmd::beziermath::morphCurveQ4F12(phase, 511U);
    const uint16_t b = fmd::beziermath::morphCurveQ4F12(phase, 512U);
    TEST_ASSERT_UINT16_WITHIN(3U, a, b);
  }
}

void test_bezier_texture_blend_clamps_out_of_range_input() {
  TEST_ASSERT_EQUAL_UINT16(0xFFFFU, fmd::beziermath::textureBlendQ0F16(0xFFFFU));
}

void test_bezier_257_point_icdf_contains_both_mathematical_endpoints() {
  MemoryReferenceTables tables;
  TEST_ASSERT_EQUAL_INT16(-32767, tables.triangularIcdfQ1_15(0U));
  TEST_ASSERT_EQUAL_INT16(32767, tables.triangularIcdfQ1_15(256U));
  TEST_ASSERT_INT16_WITHIN(1, 0, tables.triangularIcdfQ1_15(128U));
}

void test_bezier_icdf_table_matches_triangular_inverse_cdf() {
  MemoryReferenceTables tables;
  for (uint16_t i = 0U; i <= 256U; ++i) {
    const double expected = triangularIcdfReference(static_cast<double>(i) / 256.0);
    const double actual = static_cast<double>(tables.triangularIcdfQ1_15(i)) / 32767.0;
    TEST_ASSERT_DOUBLE_WITHIN(0.00005, expected, actual);
  }
}

void test_bezier_runtime_icdf_interpolation_is_monotonic_and_has_no_final_plateau() {
  MemoryReferenceTables tables;
  int16_t previous = fmd::beziermath::triangularIcdfQ1F15(0U, tables);
  for (uint32_t u = 1U; u <= 0x7FFFU; ++u) {
    const int16_t current = fmd::beziermath::triangularIcdfQ1F15(static_cast<uint16_t>(u), tables);
    TEST_ASSERT_GREATER_OR_EQUAL_INT16(previous, current);
    previous = current;
  }
  TEST_ASSERT_GREATER_THAN_INT16(fmd::beziermath::triangularIcdfQ1F15(32640U, tables),
                                 fmd::beziermath::triangularIcdfQ1F15(32767U, tables));
}

void test_bezier_phase_wrap_preserves_overshoot() {
  bool rollover = false;
  const uint32_t result = fmd::beziermath::advancePhase(0xFFFFFF00UL, 0x00000200UL, rollover);
  TEST_ASSERT_TRUE(rollover);
  TEST_ASSERT_EQUAL_UINT32(0x00000100UL, result);
}


void test_bezier_speed_variation_scale_has_exact_dead_zone_and_full_scale_endpoints() {
  for (uint16_t knob = 383U; knob <= 639U; ++knob) {
    TEST_ASSERT_EQUAL_UINT16(0U, fmd::beziermath::speedVariationScaleQ1F15(knob, 0U));
  }
  TEST_ASSERT_GREATER_THAN_UINT16(0U, fmd::beziermath::speedVariationScaleQ1F15(382U, 0U));
  TEST_ASSERT_GREATER_THAN_UINT16(0U, fmd::beziermath::speedVariationScaleQ1F15(640U, 0U));
  TEST_ASSERT_EQUAL_UINT16(0x7FFFU, fmd::beziermath::speedVariationScaleQ1F15(0U, 0U));
  TEST_ASSERT_EQUAL_UINT16(0x7FFFU, fmd::beziermath::speedVariationScaleQ1F15(1023U, 0U));
}

void test_bezier_speed_variation_scale_is_symmetric_around_center_until_saturation() {
  constexpr uint16_t center = 511U;
  for (uint16_t distance = 0U; distance <= 511U; ++distance) {
    const uint16_t left = static_cast<uint16_t>(center - distance);
    const uint16_t right = static_cast<uint16_t>(center + distance);
    TEST_ASSERT_EQUAL_UINT16(fmd::beziermath::speedVariationScaleQ1F15(left, 0U),
                             fmd::beziermath::speedVariationScaleQ1F15(right, 0U));
  }
}

void test_bezier_speed_variation_cv_contribution_is_monotonic_and_saturates() {
  uint16_t previous = 0U;
  for (uint16_t cv = 0U; cv <= 1023U; ++cv) {
    const uint16_t current = fmd::beziermath::speedVariationScaleQ1F15(511U, cv);
    TEST_ASSERT_GREATER_OR_EQUAL_UINT16(previous, current);
    previous = current;
  }
  TEST_ASSERT_EQUAL_UINT16(0U, fmd::beziermath::speedVariationScaleQ1F15(511U, 0U));
  TEST_ASSERT_EQUAL_UINT16(0x7FFFU, fmd::beziermath::speedVariationScaleQ1F15(511U, 1023U));
  TEST_ASSERT_EQUAL_UINT16(0x7FFFU, fmd::beziermath::speedVariationScaleQ1F15(0xFFFFU, 0xFFFFU));
}

void test_bezier_icdf_is_antisymmetric_over_complete_uniform_domain() {
  MemoryReferenceTables tables;
  for (uint16_t u = 0U; u <= 0x7FFFU; ++u) {
    const int16_t a = fmd::beziermath::triangularIcdfQ1F15(u, tables);
    const int16_t b = fmd::beziermath::triangularIcdfQ1F15(static_cast<uint16_t>(0x7FFFU - u), tables);
    TEST_ASSERT_INT16_WITHIN(32, 0, static_cast<int16_t>(a + b));
  }
}

void test_bezier_interpolation_hits_exact_segment_endpoints_for_both_directions() {
  constexpr uint16_t textures[] = {0U, 511U, 1023U};
  for (const uint16_t texture : textures) {
    TEST_ASSERT_EQUAL_UINT16(500U, fmd::beziermath::interpolateQ4F12(0U, 500U, 3500U, texture));
    TEST_ASSERT_EQUAL_UINT16(3500U, fmd::beziermath::interpolateQ4F12(4096U, 500U, 3500U, texture));
    TEST_ASSERT_EQUAL_UINT16(3500U, fmd::beziermath::interpolateQ4F12(0U, 3500U, 500U, texture));
    TEST_ASSERT_EQUAL_UINT16(500U, fmd::beziermath::interpolateQ4F12(4096U, 3500U, 500U, texture));
  }
}

void test_bezier_algorithm_matches_independent_no_speed_variation_state_machine_across_rollovers() {
  MemoryReferenceTables tables;
  constexpr uint16_t seed = 0x5A3CU;
  fmd::BezierAlgorithm algorithm(tables, seed);
  fmd::ParallelLfsr rng(seed);
  uint16_t a = 0U;
  uint16_t b = static_cast<uint16_t>(rng.next() >> 4U);
  uint32_t phase = 0U;
  const fmd::ControlFrame controls{1023U, 0U, 1023U, 511U};
  const uint32_t delta = fmd::phaseIncrementFromControls(tables, controls.speedKnob, controls.speedCv, 0);
  uint32_t rollovers = 0U;

  for (uint32_t i = 0U; i < 400U; ++i) {
    const uint32_t previous = phase;
    phase = static_cast<uint32_t>(phase + delta);
    if (phase < previous) {
      ++rollovers;
      a = b;
      b = static_cast<uint16_t>(rng.next() >> 4U);
    }
    const uint16_t expected = fmd::beziermath::interpolateQ4F12(
        static_cast<uint16_t>(phase >> 20U), a, b, controls.textureKnob);
    TEST_ASSERT_EQUAL_UINT16(expected, algorithm.step(controls));
  }
  TEST_ASSERT_GREATER_THAN_UINT32(5U, rollovers);
}


void test_bezier_interpolation_stays_inside_endpoint_interval_over_dense_phase_grid() {
  constexpr uint16_t endpoints[][2] = {{0U, 4095U}, {4095U, 0U}, {731U, 2844U}, {2844U, 731U}};
  constexpr uint16_t textures[] = {0U, 127U, 511U, 512U, 896U, 1023U};
  for (const auto& endpoint : endpoints) {
    const uint16_t low = endpoint[0] < endpoint[1] ? endpoint[0] : endpoint[1];
    const uint16_t high = endpoint[0] > endpoint[1] ? endpoint[0] : endpoint[1];
    for (const uint16_t texture : textures) {
      for (uint16_t phase = 0U; phase <= 4096U; ++phase) {
        const uint16_t value = fmd::beziermath::interpolateQ4F12(phase, endpoint[0], endpoint[1], texture);
        TEST_ASSERT_GREATER_OR_EQUAL_UINT16(low, value);
        TEST_ASSERT_LESS_OR_EQUAL_UINT16(high, value);
      }
    }
  }
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_bezier_special_curves_have_exact_endpoints_and_midpoint);
  RUN_TEST(test_bezier_special_curves_match_cubic_reference);
  RUN_TEST(test_bezier_special_curves_are_monotonic);
  RUN_TEST(test_bezier_texture_morph_has_exact_curve_endpoints_and_near_linear_center);
  RUN_TEST(test_bezier_texture_morph_is_continuous_across_former_511_512_boundary);
  RUN_TEST(test_bezier_texture_blend_clamps_out_of_range_input);
  RUN_TEST(test_bezier_257_point_icdf_contains_both_mathematical_endpoints);
  RUN_TEST(test_bezier_icdf_table_matches_triangular_inverse_cdf);
  RUN_TEST(test_bezier_runtime_icdf_interpolation_is_monotonic_and_has_no_final_plateau);
  RUN_TEST(test_bezier_phase_wrap_preserves_overshoot);
  RUN_TEST(test_bezier_speed_variation_scale_has_exact_dead_zone_and_full_scale_endpoints);
  RUN_TEST(test_bezier_speed_variation_scale_is_symmetric_around_center_until_saturation);
  RUN_TEST(test_bezier_speed_variation_cv_contribution_is_monotonic_and_saturates);
  RUN_TEST(test_bezier_icdf_is_antisymmetric_over_complete_uniform_domain);
  RUN_TEST(test_bezier_interpolation_hits_exact_segment_endpoints_for_both_directions);
  RUN_TEST(test_bezier_algorithm_matches_independent_no_speed_variation_state_machine_across_rollovers);
  RUN_TEST(test_bezier_interpolation_stays_inside_endpoint_interval_over_dense_phase_grid);
  return UNITY_END();
}
