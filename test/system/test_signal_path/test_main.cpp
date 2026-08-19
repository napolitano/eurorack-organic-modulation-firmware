/**
 * @file test_main.cpp
 * Implements the end-to-end signal-path system native test suite.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <unity.h>
#include "fmd/application/DriftRuntime.h"
#include "fmd/domain/DriftEngine.h"
#include "DriftTestRig.h"
void test_virtual_module_runs_all_four_algorithms() {
  for (uint8_t slotIndex = 0U; slotIndex < 4U; ++slotIndex) {
    DriftTestRig rig(fmd::algorithmForBankSlot(slotIndex), 0x3344U);
    for (uint16_t sampleIndex = 0U; sampleIndex < 100U; ++sampleIndex) {
      const uint16_t outputCode = rig.tick(100U, 200U, 300U, 400U);
      TEST_ASSERT_LESS_OR_EQUAL_UINT16(4095U, outputCode);
    }
  }
}
void test_virtual_module_maps_adc_channels_in_upstream_order() {
  DriftTestRig rig(fmd::algorithmForBankSlot(0U), 1U);
  const uint16_t baselineOutput = rig.tick(0U, 0U, 0U, 0U);
  const uint16_t fasterOutput = rig.tick(1023U, 0U, 1023U, 0U);
  TEST_ASSERT_NOT_EQUAL(baselineOutput, fasterOutput);
}

void test_virtual_module_matches_direct_engine_for_dynamic_control_sequence() {
  for (uint8_t slotIndex = 0U; slotIndex < 4U; ++slotIndex) {
    constexpr uint16_t seed = 0x47A1U;
    const fmd::Algorithm algorithm = fmd::algorithmForBankSlot(slotIndex);
    DriftTestRig rig(algorithm, seed);
    fmd::DriftEngine engine(algorithm, seed, rig.referenceTables);

    for (uint32_t i = 0U; i < 2000U; ++i) {
      const uint16_t speedCv = static_cast<uint16_t>((i * 73U) & 1023U);
      const uint16_t textureCv = static_cast<uint16_t>((i * 151U + 17U) & 1023U);
      const uint16_t speedKnob = static_cast<uint16_t>((1023U - ((i * 37U) & 1023U)) & 1023U);
      const uint16_t textureKnob = static_cast<uint16_t>((i * 211U + 503U) & 1023U);
      const fmd::ControlFrame frame{speedCv, textureCv, speedKnob, textureKnob};
      TEST_ASSERT_EQUAL_UINT16(engine.step(frame),
                               rig.tick(speedCv, textureCv, speedKnob, textureKnob));
    }
  }
}

void test_virtual_module_led_always_represents_same_sample_as_dac_over_dynamic_sequence() {
  DriftTestRig rig(fmd::algorithmForBankSlot(0U), 0x1969U);
  for (uint32_t i = 0U; i < 3000U; ++i) {
    const uint16_t output = rig.tick(static_cast<uint16_t>((i * 29U) & 1023U),
                                     static_cast<uint16_t>((i * 43U) & 1023U),
                                     static_cast<uint16_t>((i * 61U) & 1023U),
                                     static_cast<uint16_t>((i * 97U) & 1023U));
    TEST_ASSERT_EQUAL_UINT8(rig.referenceTables.gamma8(static_cast<uint8_t>(output >> 4U)), rig.ledOutput.duty);
  }
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_virtual_module_runs_all_four_algorithms);
  RUN_TEST(test_virtual_module_maps_adc_channels_in_upstream_order);
  RUN_TEST(test_virtual_module_matches_direct_engine_for_dynamic_control_sequence);
  RUN_TEST(test_virtual_module_led_always_represents_same_sample_as_dac_over_dynamic_sequence);
  return UNITY_END();
}
