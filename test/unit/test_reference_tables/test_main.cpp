/**
 * @file test_main.cpp
 * Implements the generated reference-table verification native test suite.
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

#include "MemoryReferenceTables.h"

void test_exp2_table_matches_documented_exponential_reference_and_is_strictly_monotonic() {
  MemoryReferenceTables tables;
  uint32_t previous = 0U;
  for (uint16_t i = 0U; i < 256U; ++i) {
    const double value = std::pow(2.0, 16.0 * static_cast<double>(i) / 256.0) * 65536.0;
    const uint32_t expected = static_cast<uint32_t>(std::llround(value));
    const uint32_t actual = tables.exp2Q16_16(static_cast<uint8_t>(i));
    TEST_ASSERT_UINT32_WITHIN(1U, expected, actual);
    if (i != 0U) {
      TEST_ASSERT_GREATER_THAN_UINT32(previous, actual);
    }
    previous = actual;
  }
}

void test_exp2_table_doubles_every_sixteen_indices_with_rounding_tolerance() {
  MemoryReferenceTables tables;
  for (uint16_t i = 0U; i < 240U; ++i) {
    const uint32_t a = tables.exp2Q16_16(static_cast<uint8_t>(i));
    const uint32_t b = tables.exp2Q16_16(static_cast<uint8_t>(i + 16U));
    TEST_ASSERT_UINT32_WITHIN(2U, static_cast<uint32_t>(a * 2U), b);
  }
}

void test_gamma_table_matches_gamma_2p2_reference_and_endpoints() {
  MemoryReferenceTables tables;
  TEST_ASSERT_EQUAL_UINT8(0U, tables.gamma8(0U));
  TEST_ASSERT_EQUAL_UINT8(255U, tables.gamma8(255U));
  uint8_t previous = 0U;
  for (uint16_t i = 0U; i < 256U; ++i) {
    const double normalized = static_cast<double>(i) / 255.0;
    const uint8_t expected = i == 0U
        ? 0U
        : static_cast<uint8_t>(std::lround(std::pow(normalized, 2.2) * 255.0));
    const uint8_t actual = tables.gamma8(static_cast<uint8_t>(i));
    TEST_ASSERT_UINT8_WITHIN(1U, expected, actual);
    TEST_ASSERT_GREATER_OR_EQUAL_UINT8(previous, actual);
    previous = actual;
  }
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_exp2_table_matches_documented_exponential_reference_and_is_strictly_monotonic);
  RUN_TEST(test_exp2_table_doubles_every_sixteen_indices_with_rounding_tolerance);
  RUN_TEST(test_gamma_table_matches_gamma_2p2_reference_and_endpoints);
  return UNITY_END();
}
