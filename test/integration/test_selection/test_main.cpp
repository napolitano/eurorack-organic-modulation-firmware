/**
 * @file test_main.cpp
 * Implements the algorithm-selection integration native test suite.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <unity.h>

#include "fmd/domain/DriftEngine.h"

void test_original_config_pin_mapping() {
  TEST_ASSERT_EQUAL_INT(
      static_cast<int>(fmd::Algorithm::Perlin),
      static_cast<int>(fmd::algorithmFromConfig(false, false)));
  TEST_ASSERT_EQUAL_INT(
      static_cast<int>(fmd::Algorithm::Brownian),
      static_cast<int>(fmd::algorithmFromConfig(false, true)));
  TEST_ASSERT_EQUAL_INT(
      static_cast<int>(fmd::Algorithm::Bezier),
      static_cast<int>(fmd::algorithmFromConfig(true, false)));
  TEST_ASSERT_EQUAL_INT(
      static_cast<int>(fmd::Algorithm::Lfo),
      static_cast<int>(fmd::algorithmFromConfig(true, true)));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_original_config_pin_mapping);
  return UNITY_END();
}
