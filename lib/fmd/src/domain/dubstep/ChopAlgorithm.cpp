/**
 * @file ChopAlgorithm.cpp
 * Implements the deterministic 16-step sparse/syncopated articulation algorithm.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "fmd/domain/dubstep/ChopAlgorithm.h"

#include "fmd/domain/AlgorithmMath.h"
#include "fmd/domain/dubstep/DubstepAlgorithmMath.h"

namespace fmd {

ChopAlgorithm::ChopAlgorithm(const IReferenceTables& referenceTables)
    : referenceTables_(referenceTables),
      clockSource_(),
      stepPhase_(0U),
      stepIndex_(0U),
      latchedMask_(0x0101U),
      externalQuarterIndex_(0U),
      externalSubdivisions_(0U),
      initialized_(false) {}

void ChopAlgorithm::start(uint16_t textureControl) {
  stepPhase_ = 0U;
  stepIndex_ = 0U;
  latchedMask_ = chopmath::onsetMask(chopmath::addedOnsetCount(textureControl));
  externalQuarterIndex_ = 0U;
  externalSubdivisions_ = 0U;
  initialized_ = true;
}

void ChopAlgorithm::advanceStep(uint16_t textureControl) {
  stepIndex_ = static_cast<uint8_t>((stepIndex_ + 1U) & 0x0FU);
  if (stepIndex_ == 0U) {
    latchedMask_ = chopmath::onsetMask(chopmath::addedOnsetCount(textureControl));
  }
}

void ChopAlgorithm::handleExternalQuarter(uint16_t textureControl) {
  externalQuarterIndex_ = static_cast<uint8_t>((externalQuarterIndex_ + 1U) & 0x03U);
  stepIndex_ = static_cast<uint8_t>(externalQuarterIndex_ << 2U);
  stepPhase_ = 0U;
  externalSubdivisions_ = 0U;
  if (stepIndex_ == 0U) {
    latchedMask_ = chopmath::onsetMask(chopmath::addedOnsetCount(textureControl));
  }
}

uint16_t ChopAlgorithm::step(const ControlFrame& controls) {
  const uint16_t texture = sumAdc(controls.textureKnob, controls.textureCv);
  const uint32_t internalQuarter = dubstepmath::quarterNotePhaseIncrement(
      referenceTables_, controls.speedKnob);
  const clock::ClockUpdate clock = clockSource_.update(controls.speedCv, internalQuarter);

  if (!initialized_) {
    start(texture);
  }

  if (clock.externalAcquired) {
    start(texture);
  } else if (clock.externalActive) {
    if (clock.quarterBoundary) {
      handleExternalQuarter(texture);
    } else {
      bool rollover = false;
      stepPhase_ = perlinmath::advancePhase(stepPhase_, clock.quarterIncrement * 4UL, rollover);
      if (rollover && externalSubdivisions_ < 3U) {
        ++externalSubdivisions_;
        advanceStep(texture);
      }
    }
  } else {
    bool rollover = false;
    stepPhase_ = perlinmath::advancePhase(stepPhase_, clock.quarterIncrement * 4UL, rollover);
    if (rollover) {
      advanceStep(texture);
    }
  }

  if (!chopmath::stepActive(latchedMask_, stepIndex_)) {
    return 0U;
  }
  return chopmath::articulationDac12(stepPhase_);
}

}  // namespace fmd
