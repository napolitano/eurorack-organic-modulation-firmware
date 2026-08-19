/**
 * @file VectorAlgorithm.cpp
 * Implements the two-dimensional toroidal Vector algorithm from the optional Organic bank.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "fmd/domain/organic/VectorAlgorithm.h"

#include "fmd/domain/FrequencyMapping.h"
#include "fmd/domain/organic/OrganicAlgorithmMath.h"

#include <stdint.h>

namespace fmd {

VectorAlgorithm::VectorAlgorithm(const IReferenceTables& referenceTables)
    : referenceTables_(referenceTables),
      xPhaseAccumulator_(0U),
      yPhaseAccumulator_(0x40000000UL) {}

uint16_t VectorAlgorithm::step(const ControlFrame& controls) {
  const uint32_t xBaseIncrement = phaseIncrementFromControls(
      referenceTables_, controls.speedKnob, controls.speedCv, 0);
  // A fixed 3/4 rate offset prevents the uncoupled axes from collapsing onto
  // the same trajectory while requiring no division in the hot path.
  const uint32_t yBaseIncrement =
      static_cast<uint32_t>(xBaseIncrement - (xBaseIncrement >> 2U));
  const uint16_t textureControl =
      sumAdc(controls.textureKnob, controls.textureCv);

  const int16_t xWaveBeforeStep = vectormath::triangleSignedQ1F15(
      static_cast<uint16_t>(xPhaseAccumulator_ >> 16U));
  const int16_t yWaveBeforeStep = vectormath::triangleSignedQ1F15(
      static_cast<uint16_t>(yPhaseAccumulator_ >> 16U));

  const uint32_t xIncrement = vectormath::coupledPhaseIncrement(
      xBaseIncrement, yWaveBeforeStep, textureControl, false);
  const uint32_t yIncrement = vectormath::coupledPhaseIncrement(
      yBaseIncrement, xWaveBeforeStep, textureControl, true);

  xPhaseAccumulator_ = static_cast<uint32_t>(xPhaseAccumulator_ + xIncrement);
  yPhaseAccumulator_ = static_cast<uint32_t>(yPhaseAccumulator_ + yIncrement);

  const int16_t xWaveAfterStep = vectormath::triangleSignedQ1F15(
      static_cast<uint16_t>(xPhaseAccumulator_ >> 16U));
  const int16_t yWaveAfterStep = vectormath::triangleSignedQ1F15(
      static_cast<uint16_t>(yPhaseAccumulator_ >> 16U));
  return vectormath::projectToDac12(xWaveAfterStep, yWaveAfterStep);
}

}  // namespace fmd
