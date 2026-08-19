/**
 * @file UrnAlgorithm.cpp
 * Implements the leaky reinforced-state algorithm from the optional Generative bank.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "fmd/domain/generative/UrnAlgorithm.h"

#include "fmd/domain/AlgorithmMath.h"
#include "fmd/domain/FrequencyMapping.h"

namespace fmd {

UrnAlgorithm::UrnAlgorithm(const IReferenceTables& referenceTables,
                           uint16_t randomSeed)
    : referenceTables_(referenceTables),
      phaseAccumulator_(0U),
      weights_{urnmath::kBaselineWeight,
               urnmath::kBaselineWeight,
               urnmath::kBaselineWeight,
               urnmath::kBaselineWeight,
               urnmath::kBaselineWeight,
               urnmath::kBaselineWeight,
               urnmath::kBaselineWeight,
               urnmath::kBaselineWeight},
      currentState_(0U),
      outputValue_(urnmath::outputLevel(0U)),
      randomGenerator_(randomSeed) {}

uint16_t UrnAlgorithm::step(const ControlFrame& controls) {
  bool phaseRolledOver = false;
  phaseAccumulator_ = perlinmath::advancePhase(
      phaseAccumulator_,
      phaseIncrementFromControls(
          referenceTables_, controls.speedKnob, controls.speedCv, 0),
      phaseRolledOver);

  if (phaseRolledOver) {
    for (uint8_t index = 0U; index < urnmath::kStateCount; ++index) {
      weights_[index] = urnmath::relaxWeight(weights_[index]);
    }

    currentState_ = urnmath::selectWeightedState(
        weights_, randomGenerator_.next());
    weights_[currentState_] = urnmath::reinforceSaturating(
        weights_[currentState_],
        urnmath::reinforcementAmount(
            sumAdc(controls.textureKnob, controls.textureCv)));
    outputValue_ = urnmath::outputLevel(currentState_);
  }

  return outputValue_;
}

}  // namespace fmd
