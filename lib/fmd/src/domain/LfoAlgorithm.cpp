/**
 * @file LfoAlgorithm.cpp
 * Implements the corrected skewable triangle / saw Drift LFO.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "fmd/domain/LfoAlgorithm.h"

#include "fmd/domain/AlgorithmMath.h"
#include "fmd/domain/FrequencyMapping.h"

#include <stdint.h>

namespace fmd {

LfoAlgorithm::LfoAlgorithm(const IReferenceTables& referenceTables)
    : referenceTables_(referenceTables),
      phaseAccumulator_(0U),
      apexPhaseQ0F16_(0x7FFFU),
      textureInitialised_(false) {}

uint16_t LfoAlgorithm::step(const ControlFrame& controls) {
  const uint16_t combinedTexture = sumAdc(controls.textureKnob, controls.textureCv);
  const uint16_t requestedApexQ0F16 = lfomath::apexFromTexture(combinedTexture);

  if (!textureInitialised_) {
    apexPhaseQ0F16_ = requestedApexQ0F16;
    textureInitialised_ = true;
  } else if (requestedApexQ0F16 != apexPhaseQ0F16_) {
    const uint16_t currentPhaseQ0F16 =
        static_cast<uint16_t>(phaseAccumulator_ >> 16U);
    const uint16_t remappedPhaseQ0F16 = lfomath::remapPhasePreservingOutput(
        currentPhaseQ0F16, apexPhaseQ0F16_, requestedApexQ0F16);

    // Replace only the visible Q0.16 phase. Retaining the lower fractional
    // accumulator bits avoids throwing away sub-sample phase progress whenever
    // Texture is moved.
    phaseAccumulator_ =
        (static_cast<uint32_t>(remappedPhaseQ0F16) << 16U) |
        (phaseAccumulator_ & 0xFFFFU);
    apexPhaseQ0F16_ = requestedApexQ0F16;
  }

  bool phaseRolledOver = false;
  phaseAccumulator_ = lfomath::advancePhase(
      phaseAccumulator_,
      phaseIncrementFromControls(
          referenceTables_, controls.speedKnob, controls.speedCv, 0),
      phaseRolledOver);
  (void)phaseRolledOver;  // Wrap needs no additional cycle-boundary state update.

  return lfomath::waveform12(
      static_cast<uint16_t>(phaseAccumulator_ >> 16U), apexPhaseQ0F16_);
}

}  // namespace fmd
