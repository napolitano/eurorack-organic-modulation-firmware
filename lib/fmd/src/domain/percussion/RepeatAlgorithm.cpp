/**
 * @file RepeatAlgorithm.cpp
 * Implements quarter-note ratchet/repeat generation with phrase fills.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "fmd/domain/percussion/RepeatAlgorithm.h"

#include "fmd/domain/AlgorithmMath.h"
#include "fmd/domain/electronica/ElectronicaAlgorithmMath.h"

namespace fmd {

RepeatAlgorithm::RepeatAlgorithm(const IReferenceTables& referenceTables,
                                 uint16_t randomSeed)
    : referenceTables_(referenceTables),
      quarterPhase_(0U),
      latchedPhaseIncrement_(0U),
      quarterIndex_(0U),
      pulseSamplesRemaining_(0U),
      clusterPulseCount_(1U),
      nextSubEventIndex_(1U),
      latchedTexture_(0U),
      phraseState_(),
      randomGenerator_(randomSeed),
      clockSource_(),
      initialized_(false) {}

void RepeatAlgorithm::launchPulse() {
  pulseSamplesRemaining_ = percussionmath::kPulseSamples;
}

void RepeatAlgorithm::configureCurrentQuarter() {
  uint8_t count = 1U;
  if (percussionmath::randomBelow(
          randomGenerator_.next(), repeatmath::repeatCutoff(latchedTexture_))) {
    count = repeatmath::ratchetCount(latchedTexture_);
  }
  if (phraseState_.isFillBar()) {
    const uint8_t forced = repeatmath::forcedMinimumCount(
        percussionmath::fillStrength(latchedTexture_), quarterIndex_);
    if (forced > count) {
      count = forced;
    }
  }
  clusterPulseCount_ = count;
  nextSubEventIndex_ = 1U;
  launchPulse();
}

void RepeatAlgorithm::synchronizeExternal(uint16_t textureControl) {
  initialized_ = true;
  quarterPhase_ = 0U;
  quarterIndex_ = 0U;
  pulseSamplesRemaining_ = 0U;
  latchedTexture_ = textureControl;
  phraseState_.start(textureControl);
  configureCurrentQuarter();
}

void RepeatAlgorithm::beginQuarter(uint16_t textureControl) {
  if (!initialized_) {
    initialized_ = true;
    quarterIndex_ = 0U;
    latchedTexture_ = textureControl;
    phraseState_.start(textureControl);
  } else {
    quarterIndex_ = static_cast<uint8_t>((quarterIndex_ + 1U) & 0x03U);
    if (quarterIndex_ == 0U) {
      phraseState_.advanceBar(textureControl);
      latchedTexture_ = textureControl;
    }
  }
  configureCurrentQuarter();
}

uint16_t RepeatAlgorithm::step(const ControlFrame& controls) {
  const uint16_t texture = sumAdc(controls.textureKnob, controls.textureCv);
  const uint32_t internalQuarterIncrement = electronicamath::quarterNotePhaseIncrement(
      referenceTables_, controls.speedKnob, 0U);
  const percussionmath::ClockUpdate clock =
      clockSource_.update(controls.speedCv, internalQuarterIncrement);

  if (!initialized_) {
    beginQuarter(texture);
    latchedPhaseIncrement_ = clock.quarterIncrement;
  }

  if (clock.externalAcquired) {
    synchronizeExternal(texture);
    latchedPhaseIncrement_ = clock.quarterIncrement;
  } else if (clock.externalActive) {
    if (clock.quarterBoundary) {
      quarterPhase_ = 0U;
      beginQuarter(texture);
      latchedPhaseIncrement_ = clock.quarterIncrement;
    } else {
      bool ignoredRollover = false;
      quarterPhase_ = perlinmath::advancePhase(
          quarterPhase_, latchedPhaseIncrement_, ignoredRollover);
      (void)ignoredRollover;
    }
  } else {
    if (clock.externalLost) {
      // Fallback is immediate: the current quarter continues from its current
      // phase using the present Speed-pot tempo rather than the stale clock.
      latchedPhaseIncrement_ = clock.quarterIncrement;
    }
    bool rollover = false;
    quarterPhase_ = perlinmath::advancePhase(
        quarterPhase_, latchedPhaseIncrement_, rollover);
    if (rollover) {
      beginQuarter(texture);
      latchedPhaseIncrement_ = clock.quarterIncrement;
    }
  }

  while (nextSubEventIndex_ < clusterPulseCount_ &&
         quarterPhase_ >=
             repeatmath::subEventThreshold(nextSubEventIndex_, clusterPulseCount_)) {
    launchPulse();
    ++nextSubEventIndex_;
  }

  const uint16_t output = pulseSamplesRemaining_ > 0U ? 4095U : 0U;
  if (pulseSamplesRemaining_ > 0U) {
    --pulseSamplesRemaining_;
  }
  return output;
}

}  // namespace fmd
