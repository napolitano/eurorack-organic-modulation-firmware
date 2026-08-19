/**
 * @file test_main.cpp
 * Implements the portable-domain invariant/property native test suite.
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
#include "MemoryReferenceTables.h"

void test_control_grid_preserves_12bit_output_invariant() {
  MemoryReferenceTables referenceTables;
  const uint16_t boundaryValues[] = {
      0U, 1U, 127U, 511U, 512U, 1019U, 1020U, 1023U,
  };

  for (uint8_t algorithmIndex = 0U; algorithmIndex < 4U; ++algorithmIndex) {
    fmd::DriftEngine engine(
        static_cast<fmd::Algorithm>(algorithmIndex), 0xBEEFU, referenceTables);

    for (uint16_t speedCv : boundaryValues) {
      for (uint16_t textureCv : boundaryValues) {
        for (uint16_t speedKnob : boundaryValues) {
          for (uint16_t textureKnob : boundaryValues) {
            const fmd::ControlFrame controls{
                speedCv, textureCv, speedKnob, textureKnob};
            TEST_ASSERT_LESS_OR_EQUAL_UINT16(4095U, engine.step(controls));
          }
        }
      }
    }
  }
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_control_grid_preserves_12bit_output_invariant);
  return UNITY_END();
}
