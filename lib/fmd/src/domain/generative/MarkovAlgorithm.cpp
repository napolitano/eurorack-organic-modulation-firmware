/**
 * @file MarkovAlgorithm.cpp
 * Implements the finite-state stochastic Markov algorithm from the Generative bank.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "fmd/domain/generative/MarkovAlgorithm.h"

#include "fmd/domain/AlgorithmMath.h"
#include "fmd/domain/FrequencyMapping.h"

namespace fmd {

MarkovAlgorithm::MarkovAlgorithm(const IReferenceTables& referenceTables,
                                 uint16_t randomSeed)
    : referenceTables_(referenceTables),
      phaseAccumulator_(0U),
      vocabulary_{0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U},
      currentState_(0U),
      outputValue_(0U),
      randomGenerator_(randomSeed) {
  initializeVocabulary();
  outputValue_ = vocabulary_[currentState_];
}

void MarkovAlgorithm::initializeVocabulary() {
  for (uint8_t band = 0U; band < markovmath::kStateCount; ++band) {
    vocabulary_[band] = markovmath::stratifiedVocabularyValue(
        band, randomGenerator_.next());
  }

  // Fisher-Yates permutation destroys any monotonic relation between symbolic
  // state adjacency and physical voltage adjacency. Multiply-high scaling keeps
  // the startup-only integer mapping bounded and division-free.
  for (uint8_t upper = static_cast<uint8_t>(markovmath::kStateCount - 1U);
       upper > 0U;
       --upper) {
    const uint8_t swapIndex = markovmath::scaleRandomToRange(
        randomGenerator_.next(), static_cast<uint8_t>(upper + 1U));
    const uint16_t temporary = vocabulary_[upper];
    vocabulary_[upper] = vocabulary_[swapIndex];
    vocabulary_[swapIndex] = temporary;
  }
}

uint16_t MarkovAlgorithm::step(const ControlFrame& controls) {
  bool phaseRolledOver = false;
  phaseAccumulator_ = perlinmath::advancePhase(
      phaseAccumulator_,
      phaseIncrementFromControls(
          referenceTables_, controls.speedKnob, controls.speedCv, 0),
      phaseRolledOver);

  if (phaseRolledOver) {
    const uint16_t textureControl =
        sumAdc(controls.textureKnob, controls.textureCv);
    const bool explore = markovmath::useUniformExploration(
        textureControl, randomGenerator_.next());
    if (explore) {
      currentState_ = markovmath::uniformState(randomGenerator_.next());
    } else {
      currentState_ = markovmath::structuredNextState(
          currentState_, static_cast<uint8_t>(randomGenerator_.next()));
    }
    outputValue_ = vocabulary_[currentState_];
  }

  return outputValue_;
}

}  // namespace fmd
