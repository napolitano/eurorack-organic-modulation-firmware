/**
 * @file AttractorAlgorithm.cpp
 * Implements the Hénon-map Attractor algorithm from the optional Organic bank.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "fmd/domain/organic/AttractorAlgorithm.h"

#include "fmd/domain/AlgorithmMath.h"
#include "fmd/domain/FrequencyMapping.h"

#include <stdint.h>

namespace fmd {

AttractorAlgorithm::AttractorAlgorithm(const IReferenceTables& referenceTables)
    : referenceTables_(referenceTables),
      phaseAccumulator_(0U),
      segmentStart_{0, 0},
      segmentEnd_{0, 0},
      segmentInitialised_(false) {}

uint16_t AttractorAlgorithm::step(const ControlFrame& controls) {
  const uint16_t textureControl =
      sumAdc(controls.textureKnob, controls.textureCv);
  const uint16_t parameterA = attractormath::parameterAQ2F14(textureControl);

  if (!segmentInitialised_) {
    segmentStart_ = {0, 0};
    segmentEnd_ = attractormath::iterateHenon(segmentStart_, parameterA);
    segmentInitialised_ = true;
  }

  bool crossedMapBoundary = false;
  phaseAccumulator_ = perlinmath::advancePhase(
      phaseAccumulator_,
      phaseIncrementFromControls(
          referenceTables_, controls.speedKnob, controls.speedCv, 0),
      crossedMapBoundary);

  if (crossedMapBoundary) {
    segmentStart_ = segmentEnd_;
    segmentEnd_ = attractormath::iterateHenon(segmentEnd_, parameterA);
  }

  const uint16_t interpolationPhaseQ0F12 =
      static_cast<uint16_t>(phaseAccumulator_ >> 20U);
  const int16_t interpolatedXQ2F14 = attractormath::interpolateQ2F14(
      segmentStart_.xQ2F14,
      segmentEnd_.xQ2F14,
      interpolationPhaseQ0F12);
  return attractormath::coordinateToDac12(interpolatedXQ2F14);
}

}  // namespace fmd
