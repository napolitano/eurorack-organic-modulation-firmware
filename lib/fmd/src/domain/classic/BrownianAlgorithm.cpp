/**
 * @file BrownianAlgorithm.cpp
 * Implements the corrected bounded Brownian random-walk Drift algorithm.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "fmd/domain/classic/BrownianAlgorithm.h"

#include "fmd/domain/AlgorithmMath.h"

#include <stdint.h>

namespace fmd {

BrownianAlgorithm::BrownianAlgorithm(uint16_t randomSeed)
    : targetValue_(0U),
      currentValue_(0U),
      smoothingResidual_(0U),
      smoothingDirection_(0),
      randomGenerator_(randomSeed) {}

void BrownianAlgorithm::updateTargetValue(uint16_t speedControl) {
  targetValue_ = brownianmath::nextTargetValue(
      targetValue_, randomGenerator_.next(), speedControl);
}

void BrownianAlgorithm::updateSmoothedValue(uint16_t textureControl) {
  brownianmath::smoothTowardTarget(
      targetValue_,
      brownianmath::smoothingAlphaQ0F16(textureControl),
      currentValue_,
      smoothingResidual_,
      smoothingDirection_);
}

uint16_t BrownianAlgorithm::step(const ControlFrame& controls) {
  const uint16_t combinedSpeed = sumAdc(controls.speedKnob, controls.speedCv);
  const uint16_t combinedTexture = sumAdc(controls.textureKnob, controls.textureCv);

  updateTargetValue(combinedSpeed);
  updateSmoothedValue(combinedTexture);

  // Keep 16-bit state internally so sub-DAC-code motion can accumulate; only
  // the final public output is reduced to the MCP4922's 12-bit domain.
  return static_cast<uint16_t>(currentValue_ >> 4U);
}

}  // namespace fmd
