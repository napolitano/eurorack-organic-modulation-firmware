/**
 * @file RainAlgorithm.cpp
 * Implements the stochastic Rain/shot-noise algorithm from the optional Organic bank.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "fmd/domain/organic/RainAlgorithm.h"

#include "fmd/domain/organic/OrganicAlgorithmMath.h"

#include <stdint.h>

namespace fmd {

RainAlgorithm::RainAlgorithm(uint16_t randomSeed)
    : envelopeValue_(0U),
      decayResidualQ0F16_(0U),
      randomGenerator_(randomSeed) {}

uint16_t RainAlgorithm::step(const ControlFrame& controls) {
  const uint16_t speedControl = sumAdc(controls.speedKnob, controls.speedCv);
  const uint16_t densityControl =
      sumAdc(controls.textureKnob, controls.textureCv);

  rainmath::decayEnvelope(rainmath::decayAlphaQ0F16(speedControl),
                          envelopeValue_,
                          decayResidualQ0F16_);

  const uint16_t eventRandom = randomGenerator_.next();
  if (eventRandom < rainmath::eventCutoff(densityControl)) {
    envelopeValue_ = rainmath::addImpulseSaturating(
        envelopeValue_,
        rainmath::impulseAmplitude(randomGenerator_.next()));
  }

  return static_cast<uint16_t>(envelopeValue_ >> 4U);
}

}  // namespace fmd
