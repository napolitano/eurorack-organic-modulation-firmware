/**
 * @file test_main.cpp
 * Implements compile-time algorithm-bank selection integration tests.
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

void test_selected_bank_config_pin_mapping() {
#if FMD_ALGORITHM_BANK == FMD_BANK_CLASSIC
  constexpr fmd::Algorithm expected[4] = {
      fmd::Algorithm::Perlin,
      fmd::Algorithm::Brownian,
      fmd::Algorithm::Bezier,
      fmd::Algorithm::Lfo,
  };
#elif FMD_ALGORITHM_BANK == FMD_BANK_ORGANIC
  constexpr fmd::Algorithm expected[4] = {
      fmd::Algorithm::Fractal,
      fmd::Algorithm::Vector,
      fmd::Algorithm::Rain,
      fmd::Algorithm::Attractor,
  };
#elif FMD_ALGORITHM_BANK == FMD_BANK_GENERATIVE
  constexpr fmd::Algorithm expected[4] = {
      fmd::Algorithm::Turing,
      fmd::Algorithm::Markov,
      fmd::Algorithm::Motif,
      fmd::Algorithm::Urn,
  };
#elif FMD_ALGORITHM_BANK == FMD_BANK_AMBIENT
  constexpr fmd::Algorithm expected[4] = {
      fmd::Algorithm::Current,
      fmd::Algorithm::Anchor,
      fmd::Algorithm::Breath,
      fmd::Algorithm::Fog,
  };
#elif FMD_ALGORITHM_BANK == FMD_BANK_ELECTRONICA
  constexpr fmd::Algorithm expected[4] = {
      fmd::Algorithm::Pump, fmd::Algorithm::Acid, fmd::Algorithm::Shuffle, fmd::Algorithm::Polymeter,
  };
#else
  constexpr fmd::Algorithm expected[4] = {
      fmd::Algorithm::Euclid, fmd::Algorithm::Repeat, fmd::Algorithm::Probability, fmd::Algorithm::Humanize,
  };
#endif

  TEST_ASSERT_EQUAL_INT(static_cast<int>(expected[0]),
                        static_cast<int>(fmd::algorithmFromConfig(false, false)));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(expected[1]),
                        static_cast<int>(fmd::algorithmFromConfig(false, true)));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(expected[2]),
                        static_cast<int>(fmd::algorithmFromConfig(true, false)));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(expected[3]),
                        static_cast<int>(fmd::algorithmFromConfig(true, true)));

  for (uint8_t slotIndex = 0U; slotIndex < 4U; ++slotIndex) {
    TEST_ASSERT_EQUAL_INT(static_cast<int>(expected[slotIndex]),
                          static_cast<int>(fmd::algorithmForBankSlot(slotIndex)));
  }
}

void test_invalid_bank_slot_falls_back_to_default_slot() {
  TEST_ASSERT_EQUAL_INT(
      static_cast<int>(fmd::algorithmForBankSlot(0U)),
      static_cast<int>(fmd::algorithmForBankSlot(0xFFU)));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_selected_bank_config_pin_mapping);
  RUN_TEST(test_invalid_bank_slot_falls_back_to_default_slot);
  return UNITY_END();
}
