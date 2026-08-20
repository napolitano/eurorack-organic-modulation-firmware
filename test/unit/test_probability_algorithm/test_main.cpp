/**
 * @file test_main.cpp
 * Verifies Percussion Probability mathematical and integration contracts.
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
#include "fmd/domain/percussion/ProbabilityAlgorithm.h"
#include "MemoryReferenceTables.h"

void test_metric_classes_match_documented_masks() {
  for (uint8_t step = 0U; step < 16U; ++step) {
    const auto cls = fmd::probabilitymath::classifyStep(step);
    if ((step % 4U) == 0U) TEST_ASSERT_EQUAL_INT((int)fmd::probabilitymath::StepClass::Primary, (int)cls);
    else if ((step % 2U) == 0U) TEST_ASSERT_EQUAL_INT((int)fmd::probabilitymath::StepClass::Secondary, (int)cls);
    else TEST_ASSERT_EQUAL_INT((int)fmd::probabilitymath::StepClass::Ghost, (int)cls);
  }
}

void test_probability_endpoints_match_contract() {
  TEST_ASSERT_EQUAL_UINT32(0U, fmd::probabilitymath::secondaryCutoff(0U));
  TEST_ASSERT_EQUAL_UINT32(65536U, fmd::probabilitymath::secondaryCutoff(1023U));
  TEST_ASSERT_EQUAL_UINT32(0U, fmd::probabilitymath::ghostCutoff(0U));
  TEST_ASSERT_EQUAL_UINT32(32768U, fmd::probabilitymath::ghostCutoff(1023U));
}

void test_fill_boost_is_exact_eighth_probability_per_level() {
  for (uint8_t f = 0U; f <= 4U; ++f) {
    TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(f) * 8192UL,
                             fmd::probabilitymath::fillBoostCutoff(f));
  }
}

void test_fill_cutoffs_saturate_and_tail_never_decreases() {
  using fmd::probabilitymath::StepClass;
  for (uint16_t t = 0U; t <= 1023U; t += 31U) {
    const uint32_t normal = fmd::probabilitymath::effectiveCutoff(StepClass::Ghost, t, false, false, 4U);
    const uint32_t fill = fmd::probabilitymath::effectiveCutoff(StepClass::Ghost, t, true, false, 4U);
    const uint32_t tail = fmd::probabilitymath::effectiveCutoff(StepClass::Ghost, t, true, true, 4U);
    TEST_ASSERT_GREATER_OR_EQUAL_UINT32(normal, fill);
    TEST_ASSERT_GREATER_OR_EQUAL_UINT32(fill, tail);
    TEST_ASSERT_LESS_OR_EQUAL_UINT32(65536U, tail);
  }
}

void test_probability_algorithm_is_reproducible_and_bounded() {
  MemoryReferenceTables tables;
  fmd::ProbabilityAlgorithm first(tables, 0xBEEFU);
  fmd::ProbabilityAlgorithm second(tables, 0xBEEFU);
  const fmd::ControlFrame controls{700U, 700U, 400U, 0U};
  for (uint16_t i = 0U; i < 6000U; ++i) {
    const uint16_t a = first.step(controls);
    TEST_ASSERT_EQUAL_UINT16(a, second.step(controls));
    TEST_ASSERT_TRUE(a == 0U || a == 4095U);
  }
}


void test_probability_common_cutoff_and_fill_helpers_clamp_endpoints() {
  TEST_ASSERT_EQUAL_UINT16(0U, fmd::percussionmath::randomCutoffLinear(0U));
  TEST_ASSERT_EQUAL_UINT16(65535U, fmd::percussionmath::randomCutoffLinear(1023U));
  TEST_ASSERT_EQUAL_UINT16(65535U, fmd::percussionmath::randomCutoffLinear(65535U));
  TEST_ASSERT_EQUAL_UINT32(
      fmd::probabilitymath::fillBoostCutoff(4U),
      fmd::probabilitymath::fillBoostCutoff(255U));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_metric_classes_match_documented_masks);
  RUN_TEST(test_probability_endpoints_match_contract);
  RUN_TEST(test_fill_boost_is_exact_eighth_probability_per_level);
  RUN_TEST(test_fill_cutoffs_saturate_and_tail_never_decreases);
  RUN_TEST(test_probability_algorithm_is_reproducible_and_bounded);
  RUN_TEST(test_probability_common_cutoff_and_fill_helpers_clamp_endpoints);
  return UNITY_END();
}
