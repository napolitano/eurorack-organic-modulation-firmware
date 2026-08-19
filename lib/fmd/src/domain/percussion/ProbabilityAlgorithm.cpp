/**
 * @file ProbabilityAlgorithm.cpp
 * Implements the phrase-aware metric probability pulse generator.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "fmd/domain/percussion/ProbabilityAlgorithm.h"

#include "fmd/domain/AlgorithmMath.h"
#include "fmd/domain/electronica/ElectronicaAlgorithmMath.h"

namespace fmd {

ProbabilityAlgorithm::ProbabilityAlgorithm(const IReferenceTables& referenceTables,
                                           uint16_t randomSeed)
    : referenceTables_(referenceTables),
      phase_(0U),
      stepIndex_(0U),
      pulseSamplesRemaining_(0U),
      latchedTexture_(0U),
      phraseState_(),
      randomGenerator_(randomSeed),
      clockSource_(),
      externalSubdivisions_(0U),
      initialized_(false) {}

void ProbabilityAlgorithm::launchPulse() {
  pulseSamplesRemaining_ = percussionmath::kPulseSamples;
}

void ProbabilityAlgorithm::emitCurrentStep() {
  const auto stepClass = probabilitymath::classifyStep(stepIndex_);
  const uint32_t cutoff = probabilitymath::effectiveCutoff(
      stepClass,
      latchedTexture_,
      phraseState_.isFillBar(),
      stepIndex_ >= 12U,
      percussionmath::fillStrength(latchedTexture_));
  if (stepClass == probabilitymath::StepClass::Primary ||
      percussionmath::randomBelow(randomGenerator_.next(), cutoff)) {
    launchPulse();
  }
}

void ProbabilityAlgorithm::synchronizeExternal(uint16_t textureControl) {
  initialized_ = true;
  phase_ = 0U;
  stepIndex_ = 0U;
  pulseSamplesRemaining_ = 0U;
  latchedTexture_ = textureControl;
  phraseState_.start(textureControl);
  externalSubdivisions_ = 0U;
  emitCurrentStep();
}

void ProbabilityAlgorithm::handleExternalQuarterBoundary(uint16_t textureControl) {
  const uint8_t currentQuarter = static_cast<uint8_t>((stepIndex_ >> 2U) & 0x03U);
  const uint8_t nextQuarter = static_cast<uint8_t>((currentQuarter + 1U) & 0x03U);
  stepIndex_ = static_cast<uint8_t>(nextQuarter << 2U);
  if (stepIndex_ == 0U) {
    phraseState_.advanceBar(textureControl);
    latchedTexture_ = textureControl;
  }
  emitCurrentStep();
}

void ProbabilityAlgorithm::handleBoundary(uint16_t textureControl) {
  if (!initialized_) {
    initialized_ = true;
    latchedTexture_ = textureControl;
    phraseState_.start(textureControl);
    stepIndex_ = 0U;
  } else {
    stepIndex_ = static_cast<uint8_t>((stepIndex_ + 1U) & 0x0FU);
    if (stepIndex_ == 0U) {
      phraseState_.advanceBar(textureControl);
      latchedTexture_ = textureControl;
    }
  }
  emitCurrentStep();
}

uint16_t ProbabilityAlgorithm::step(const ControlFrame& controls) {
  const uint16_t texture = sumAdc(controls.textureKnob, controls.textureCv);
  const uint32_t internalQuarterIncrement = electronicamath::quarterNotePhaseIncrement(
      referenceTables_, controls.speedKnob, 0U);
  const percussionmath::ClockUpdate clock =
      clockSource_.update(controls.speedCv, internalQuarterIncrement);

  if (!initialized_) {
    handleBoundary(texture);
  }

  if (clock.externalAcquired) {
    synchronizeExternal(texture);
  } else if (clock.externalActive) {
    if (clock.quarterBoundary) {
      phase_ = 0U;
      externalSubdivisions_ = 0U;
      handleExternalQuarterBoundary(texture);
    } else {
      bool rollover = false;
      phase_ = perlinmath::advancePhase(
          phase_, clock.quarterIncrement * 4UL, rollover);
      if (rollover && externalSubdivisions_ < 3U) {
        ++externalSubdivisions_;
        handleBoundary(texture);
      }
    }
  } else {
    bool rollover = false;
    phase_ = perlinmath::advancePhase(
        phase_, clock.quarterIncrement * 4UL, rollover);
    if (rollover) {
      handleBoundary(texture);
    }
  }

  const uint16_t output = pulseSamplesRemaining_ > 0U ? 4095U : 0U;
  if (pulseSamplesRemaining_ > 0U) {
    --pulseSamplesRemaining_;
  }
  return output;
}

}  // namespace fmd
