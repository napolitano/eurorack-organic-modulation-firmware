/**
 * @file TuringAlgorithm.cpp
 * Implements the mutating shift-register algorithm from the optional Generative bank.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "fmd/domain/TuringAlgorithm.h"

#include "fmd/domain/AlgorithmMath.h"
#include "fmd/domain/FrequencyMapping.h"
#include "fmd/domain/GenerativeAlgorithmMath.h"

namespace fmd {

TuringAlgorithm::TuringAlgorithm(const IReferenceTables& referenceTables,
                                 uint16_t randomSeed)
    : referenceTables_(referenceTables),
      phaseAccumulator_(0U),
      registerState_(0U),
      outputValue_(0U),
      randomGenerator_(randomSeed) {
  registerState_ = randomGenerator_.next();
  // Avoid the two musically degenerate locked states at Texture zero while
  // keeping initialization entirely deterministic for a given startup seed.
  if (registerState_ == 0x0000U || registerState_ == 0xFFFFU) {
    registerState_ ^= 0xA5C3U;
  }
  outputValue_ = turingmath::projectToDac12(registerState_);
}

uint16_t TuringAlgorithm::step(const ControlFrame& controls) {
  bool phaseRolledOver = false;
  phaseAccumulator_ = perlinmath::advancePhase(
      phaseAccumulator_,
      phaseIncrementFromControls(
          referenceTables_, controls.speedKnob, controls.speedCv, 0),
      phaseRolledOver);

  if (phaseRolledOver) {
    const uint16_t textureControl =
        sumAdc(controls.textureKnob, controls.textureCv);
    const bool mutate = turingmath::shouldMutate(
        textureControl, randomGenerator_.next());
    registerState_ = turingmath::advanceRegister(registerState_, mutate);
    outputValue_ = turingmath::projectToDac12(registerState_);
  }

  return outputValue_;
}

}  // namespace fmd
