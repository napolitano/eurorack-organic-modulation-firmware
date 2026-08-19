/**
 * @file MotifAlgorithm.cpp
 * Implements the phrase-transformation algorithm from the optional Generative bank.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "fmd/domain/MotifAlgorithm.h"

#include "fmd/domain/AlgorithmMath.h"
#include "fmd/domain/FrequencyMapping.h"

namespace fmd {

MotifAlgorithm::MotifAlgorithm(const IReferenceTables& referenceTables,
                               uint16_t randomSeed)
    : referenceTables_(referenceTables),
      phaseAccumulator_(0U),
      phrase_{0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U},
      playhead_(0U),
      outputValue_(0U),
      randomGenerator_(randomSeed) {
  for (uint8_t index = 0U; index < motifmath::kPhraseLength; ++index) {
    phrase_[index] = static_cast<uint16_t>(randomGenerator_.next() >> 4U);
  }
  outputValue_ = phrase_[0];
}

void MotifAlgorithm::applyRandomEdit() {
  const motifmath::Operation operation =
      motifmath::operationFromRandom(randomGenerator_.next());
  switch (operation) {
    case motifmath::Operation::Rotate:
      motifmath::rotate(phrase_, (randomGenerator_.next() & 0x0001U) != 0U);
      break;
    case motifmath::Operation::AdjacentSwap:
      motifmath::adjacentSwap(
          phrase_, static_cast<uint8_t>(randomGenerator_.next()));
      break;
    case motifmath::Operation::ReverseThree:
      motifmath::reverseThree(
          phrase_, static_cast<uint8_t>(randomGenerator_.next()));
      break;
    case motifmath::Operation::ReplaceOne:
      motifmath::replaceOne(
          phrase_,
          static_cast<uint8_t>(randomGenerator_.next()),
          static_cast<uint16_t>(randomGenerator_.next() >> 4U));
      break;
  }
}

uint16_t MotifAlgorithm::step(const ControlFrame& controls) {
  bool phaseRolledOver = false;
  phaseAccumulator_ = perlinmath::advancePhase(
      phaseAccumulator_,
      phaseIncrementFromControls(
          referenceTables_, controls.speedKnob, controls.speedCv, 0),
      phaseRolledOver);

  if (phaseRolledOver) {
    const uint8_t nextPlayhead = static_cast<uint8_t>((playhead_ + 1U) & 0x07U);
    if (nextPlayhead == 0U) {
      const uint16_t textureControl =
          sumAdc(controls.textureKnob, controls.textureCv);
      if (motifmath::shouldEdit(textureControl, randomGenerator_.next())) {
        // Apply the edit before the first value of the new phrase cycle is
        // emitted. This keeps every audible cycle internally consistent.
        applyRandomEdit();
      }
    }
    playhead_ = nextPlayhead;
    outputValue_ = phrase_[playhead_];
  }

  return outputValue_;
}

}  // namespace fmd
