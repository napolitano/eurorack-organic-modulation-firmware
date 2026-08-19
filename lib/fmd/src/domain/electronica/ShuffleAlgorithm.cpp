/**
 * @file ShuffleAlgorithm.cpp
 * Implements deterministic long/short timing modulation.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "fmd/domain/electronica/ShuffleAlgorithm.h"

#include "fmd/domain/AlgorithmMath.h"
#include "fmd/domain/electronica/ElectronicaAlgorithmMath.h"

namespace fmd {

ShuffleAlgorithm::ShuffleAlgorithm(const IReferenceTables& referenceTables)
    : referenceTables_(referenceTables),
      pairPhase_(0U),
      secondOnsetThreshold_(shufflemath::secondOnsetThreshold(
          shufflemath::kStraightRatioQ0F16)),
      envelopePhase_(0U),
      secondOnsetTriggered_(false),
      envelopeActive_(false),
      initialized_(false) {}

void ShuffleAlgorithm::launchEnvelope() {
  envelopePhase_ = 0U;
  envelopeActive_ = true;
}

void ShuffleAlgorithm::latchRatio(uint16_t textureControl) {
  secondOnsetThreshold_ = shufflemath::secondOnsetThreshold(
      shufflemath::secondOnsetRatioQ0F16(textureControl));
}

uint16_t ShuffleAlgorithm::step(const ControlFrame& controls) {
  const uint32_t pairIncrement = electronicamath::shufflePairPhaseIncrement(
      referenceTables_, controls.speedKnob, controls.speedCv);

  if (!initialized_) {
    latchRatio(sumAdc(controls.textureKnob, controls.textureCv));
    secondOnsetTriggered_ = false;
    launchEnvelope();
    initialized_ = true;
  }

  const uint32_t previousPhase = pairPhase_;
  bool pairRollover = false;
  pairPhase_ = perlinmath::advancePhase(pairPhase_, pairIncrement, pairRollover);

  const bool crossedSecond = !secondOnsetTriggered_ &&
      ((pairRollover && previousPhase < secondOnsetThreshold_) ||
       (!pairRollover && previousPhase < secondOnsetThreshold_ &&
        pairPhase_ >= secondOnsetThreshold_));

  if (crossedSecond) {
    secondOnsetTriggered_ = true;
    launchEnvelope();
  }

  if (pairRollover) {
    latchRatio(sumAdc(controls.textureKnob, controls.textureCv));
    secondOnsetTriggered_ = false;
    launchEnvelope();
  }

  const uint16_t output = envelopeActive_
      ? shufflemath::decayOutputDac12(envelopePhase_)
      : 0U;

  if (envelopeActive_) {
    bool envelopeRollover = false;
    envelopePhase_ = perlinmath::advancePhase(
        envelopePhase_,
        shufflemath::envelopePhaseIncrement(pairIncrement),
        envelopeRollover);
    if (envelopeRollover) {
      envelopeActive_ = false;
      envelopePhase_ = 0U;
    }
  }

  return output;
}

}  // namespace fmd
