/**
 * @file test_main.cpp
 * Verifies the mathematical and deterministic contract of Ambient Current.
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

#include "MemoryReferenceTables.h"
#include "fmd/domain/ambient/AmbientAlgorithmMath.h"
#include "fmd/domain/ambient/CurrentAlgorithm.h"

void test_current_weights_have_exact_endpoints_and_constant_sum() {
  const auto low = fmd::currentmath::weights(0U);
  const auto high = fmd::currentmath::weights(1023U);
  TEST_ASSERT_EQUAL_UINT16(768U, low.primary);
  TEST_ASSERT_EQUAL_UINT16(192U, low.secondary);
  TEST_ASSERT_EQUAL_UINT16(64U, low.tertiary);
  TEST_ASSERT_EQUAL_UINT16(512U, high.primary);
  TEST_ASSERT_EQUAL_UINT16(320U, high.secondary);
  TEST_ASSERT_EQUAL_UINT16(192U, high.tertiary);

  for (uint16_t texture = 0U; texture <= 1023U; ++texture) {
    const auto selected = fmd::currentmath::weights(texture);
    TEST_ASSERT_EQUAL_UINT16(
        1024U,
        static_cast<uint16_t>(
            selected.primary + selected.secondary + selected.tertiary));
  }
}

void test_current_ratio_approximations_are_close_to_ideal() {
  constexpr uint32_t base = 1000000UL;
  TEST_ASSERT_DOUBLE_WITHIN(
      0.0002,
      std::sqrt(2.0),
      static_cast<double>(fmd::currentmath::sqrt2Increment(base)) / base);
  TEST_ASSERT_DOUBLE_WITHIN(
      0.001,
      (1.0 + std::sqrt(5.0)) / 2.0,
      static_cast<double>(fmd::currentmath::phiIncrement(base)) / base);
}

void test_current_soft_triangle_has_expected_extrema() {
  TEST_ASSERT_EQUAL_INT16(-4096, fmd::currentmath::softTriangleQ3F12(0U));
  TEST_ASSERT_INT16_WITHIN(
      2, 4096, fmd::currentmath::softTriangleQ3F12(0x80000000UL));
  TEST_ASSERT_EQUAL_INT16(
      -4096, fmd::currentmath::softTriangleQ3F12(0xFFFFFFFFUL));
}

void test_current_algorithm_is_deterministic_and_bounded() {
  MemoryReferenceTables tables;
  fmd::CurrentAlgorithm first(tables);
  fmd::CurrentAlgorithm second(tables);
  const fmd::ControlFrame controls{321U, 654U, 777U, 200U};

  for (uint32_t sample = 0U; sample < 10000U; ++sample) {
    const uint16_t firstValue = first.step(controls);
    const uint16_t secondValue = second.step(controls);
    TEST_ASSERT_EQUAL_UINT16(firstValue, secondValue);
    TEST_ASSERT_LESS_OR_EQUAL_UINT16(4095U, firstValue);
  }
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_current_weights_have_exact_endpoints_and_constant_sum);
  RUN_TEST(test_current_ratio_approximations_are_close_to_ideal);
  RUN_TEST(test_current_soft_triangle_has_expected_extrema);
  RUN_TEST(test_current_algorithm_is_deterministic_and_bounded);
  return UNITY_END();
}
