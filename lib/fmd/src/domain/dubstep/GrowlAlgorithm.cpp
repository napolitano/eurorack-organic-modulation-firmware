/**
 * @file GrowlAlgorithm.cpp
 * Implements the beat-synchronised multi-lobed timbral-motion CV algorithm.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "fmd/domain/dubstep/GrowlAlgorithm.h"

#include "fmd/domain/dubstep/DubstepAlgorithmMath.h"

namespace fmd {

GrowlAlgorithm::GrowlAlgorithm(const IReferenceTables& referenceTables)
    : referenceTables_(referenceTables),
      clockSource_(),
      phase_(0U),
      cachedTexture_(UINT16_MAX),
      weights_{4096U, 0U, 0U},
      externalQuarterIndex_(0U),
      initialized_(false) {}

void GrowlAlgorithm::updateTexture(uint16_t textureControl) {
  textureControl = clampAdc(textureControl);
  if (textureControl != cachedTexture_) {
    cachedTexture_ = textureControl;
    weights_ = growlmath::normalizedWeights(textureControl);
  }
}

uint16_t GrowlAlgorithm::step(const ControlFrame& controls) {
  const uint16_t texture = sumAdc(controls.textureKnob, controls.textureCv);
  updateTexture(texture);
  const uint32_t internalQuarter = dubstepmath::quarterNotePhaseIncrement(
      referenceTables_, controls.speedKnob);
  const clock::ClockUpdate clock = clockSource_.update(controls.speedCv, internalQuarter);

  if (!initialized_) {
    initialized_ = true;
    phase_ = 0U;
  }

  if (clock.externalAcquired) {
    phase_ = 0U;
    externalQuarterIndex_ = 0U;
  } else if (clock.externalActive && clock.quarterBoundary) {
    externalQuarterIndex_ ^= 1U;
    phase_ = externalQuarterIndex_ == 0U ? 0U : UINT32_C(0x80000000);
  }

  const uint16_t output = growlmath::outputDac12(phase_, weights_);
  if (!(clock.externalActive && clock.quarterBoundary) && !clock.externalAcquired) {
    phase_ += static_cast<uint32_t>((clock.quarterIncrement + 1UL) >> 1U);
  }
  return output;
}

}  // namespace fmd
