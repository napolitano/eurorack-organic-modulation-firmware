/**
 * @file CurrentAlgorithm.cpp
 * Implements deterministic long-form Current modulation for the Ambient bank.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "fmd/domain/CurrentAlgorithm.h"

#include "fmd/domain/AmbientAlgorithmMath.h"
#include "fmd/domain/FrequencyMapping.h"

namespace fmd {

CurrentAlgorithm::CurrentAlgorithm(const IReferenceTables& referenceTables)
    : referenceTables_(referenceTables),
      phase0_(0U),
      phase1_(0x55555555UL),
      phase2_(0xAAAAAAAAUL) {}

uint16_t CurrentAlgorithm::step(const ControlFrame& controls) {
  const uint32_t baseIncrement = ambientmath::ambientPhaseIncrement(
      phaseIncrementFromControls(
          referenceTables_, controls.speedKnob, controls.speedCv, 0));

  phase0_ += baseIncrement;
  phase1_ += currentmath::sqrt2Increment(baseIncrement);
  phase2_ += currentmath::phiIncrement(baseIncrement);

  const currentmath::Weights selectedWeights =
      currentmath::weights(sumAdc(controls.textureKnob, controls.textureCv));
  return currentmath::mixToDac12(
      currentmath::softTriangleQ3F12(phase0_),
      currentmath::softTriangleQ3F12(phase1_),
      currentmath::softTriangleQ3F12(phase2_),
      selectedWeights);
}

}  // namespace fmd
