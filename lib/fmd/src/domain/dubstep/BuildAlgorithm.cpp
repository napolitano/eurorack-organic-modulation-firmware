/**
 * @file BuildAlgorithm.cpp
 * Implements the repeating multi-bar tension-rise and micro-rate escalation algorithm.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "fmd/domain/dubstep/BuildAlgorithm.h"

#include "fmd/domain/dubstep/DubstepAlgorithmMath.h"

namespace fmd {

BuildAlgorithm::BuildAlgorithm(const IReferenceTables& referenceTables)
    : referenceTables_(referenceTables),
      clockSource_(),
      phrasePhase_(0U),
      microPhase_(0U),
      latchedTexture_(0U),
      phraseLengthBars_(8U),
      externalQuarterIndex_(0U),
      initialized_(false) {}

void BuildAlgorithm::start(uint16_t textureControl) {
  latchedTexture_ = clampAdc(textureControl);
  phraseLengthBars_ = buildmath::phraseLengthBars(latchedTexture_);
  phrasePhase_ = 0U;
  microPhase_ = 0U;
  externalQuarterIndex_ = 0U;
  initialized_ = true;
}

void BuildAlgorithm::wrapPhrase(uint16_t textureControl) {
  latchedTexture_ = clampAdc(textureControl);
  phraseLengthBars_ = buildmath::phraseLengthBars(latchedTexture_);
  phrasePhase_ = 0U;
  microPhase_ = 0U;
  externalQuarterIndex_ = 0U;
}

void BuildAlgorithm::handleExternalQuarter(uint16_t textureControl) {
  const uint8_t totalQuarters = static_cast<uint8_t>(phraseLengthBars_ * 4U);
  ++externalQuarterIndex_;
  if (externalQuarterIndex_ >= totalQuarters) {
    wrapPhrase(textureControl);
    return;
  }
  const uint8_t shift = buildmath::phraseQuarterShift(phraseLengthBars_);
  phrasePhase_ = static_cast<uint32_t>(externalQuarterIndex_) << (32U - shift);
  microPhase_ = 0U;
}

uint16_t BuildAlgorithm::step(const ControlFrame& controls) {
  const uint16_t texture = sumAdc(controls.textureKnob, controls.textureCv);
  const uint32_t internalQuarter = dubstepmath::quarterNotePhaseIncrement(
      referenceTables_, controls.speedKnob);
  const clock::ClockUpdate clock = clockSource_.update(controls.speedCv, internalQuarter);

  if (!initialized_) {
    start(texture);
  }

  if (clock.externalAcquired) {
    start(texture);
  } else if (clock.externalActive && clock.quarterBoundary) {
    handleExternalQuarter(texture);
  }

  const uint16_t output = buildmath::outputDac12(phrasePhase_, microPhase_);
  if (!(clock.externalActive && clock.quarterBoundary) && !clock.externalAcquired) {
    const uint8_t shift = buildmath::phraseQuarterShift(phraseLengthBars_);
    const uint32_t phraseIncrement = clock.quarterIncrement >> shift;
    const uint32_t previousPhrase = phrasePhase_;
    phrasePhase_ += phraseIncrement;
    const uint8_t stage = buildmath::microRateStage(previousPhrase);
    microPhase_ += buildmath::microPhaseIncrement(clock.quarterIncrement, stage);
    if (phrasePhase_ < previousPhrase) {
      wrapPhrase(texture);
    }
  }
  return output;
}

}  // namespace fmd
