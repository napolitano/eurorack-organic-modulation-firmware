/**
 * @file HumanizeAlgorithm.cpp
 * Implements the bounded timing/amplitude humanizer.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "fmd/domain/percussion/HumanizeAlgorithm.h"

#include "fmd/domain/AlgorithmMath.h"
#include "fmd/domain/electronica/ElectronicaAlgorithmMath.h"

namespace fmd {

HumanizeAlgorithm::HumanizeAlgorithm(const IReferenceTables& referenceTables,
                                     uint16_t randomSeed)
    : referenceTables_(referenceTables),
      nominalPhase_(0U),
      latchedPhaseIncrement_(0U),
      earlyThreshold_(0U),
      pulseSamplesRemaining_(0U),
      lateDelaySamplesRemaining_(0U),
      upcomingOffsetSamples_(0),
      upcomingAmplitudeDac12_(3840U),
      currentPulseAmplitudeDac12_(0U),
      randomGenerator_(randomSeed),
      clockSource_(),
      initialized_(false),
      upcomingEarlyFired_(false),
      externalMidpointSeen_(false) {}

void HumanizeAlgorithm::launchPulse(uint16_t amplitude) {
  currentPulseAmplitudeDac12_ = amplitude > 4095U ? 4095U : amplitude;
  pulseSamplesRemaining_ = percussionmath::kPulseSamples;
}

void HumanizeAlgorithm::recomputeEarlyThreshold() {
  if (upcomingOffsetSamples_ < 0) {
    const uint32_t samplesEarly = static_cast<uint32_t>(-upcomingOffsetSamples_);
    const uint64_t advance = static_cast<uint64_t>(latchedPhaseIncrement_) * samplesEarly;
    earlyThreshold_ = advance >= UINT32_C(0xFFFFFFFF)
        ? 0U
        : static_cast<uint32_t>(UINT32_C(0xFFFFFFFF) - advance + 1U);
  } else {
    earlyThreshold_ = UINT32_MAX;
  }
}

void HumanizeAlgorithm::prepareUpcomingEvent(uint16_t textureControl) {
  upcomingOffsetSamples_ = humanizemath::jitterSamples(
      randomGenerator_.next(), textureControl);
  upcomingAmplitudeDac12_ = humanizemath::pulseAmplitudeDac12(
      randomGenerator_.next(), textureControl);
  upcomingEarlyFired_ = false;
  // Positive jitter is armed only when the nominal boundary is reached. This
  // prevents a future late event from firing immediately after it is prepared.
  lateDelaySamplesRemaining_ = 0U;
  recomputeEarlyThreshold();
}

void HumanizeAlgorithm::handleNominalBoundary(uint16_t textureControl,
                                              uint32_t nextPhaseIncrement) {
  if (upcomingOffsetSamples_ < 0) {
    if (!upcomingEarlyFired_) {
      // Defensive fallback for a large tempo jump that moved the predicted
      // early threshold past the scheduler sample before the boundary.
      launchPulse(upcomingAmplitudeDac12_);
    }
  } else if (upcomingOffsetSamples_ == 0) {
    launchPulse(upcomingAmplitudeDac12_);
  } else {
    lateDelaySamplesRemaining_ = static_cast<uint8_t>(upcomingOffsetSamples_);
  }

  latchedPhaseIncrement_ = nextPhaseIncrement;
  if (upcomingOffsetSamples_ <= 0) {
    prepareUpcomingEvent(textureControl);
  }
}

void HumanizeAlgorithm::synchronizeExternal(uint16_t textureControl,
                                            uint32_t quarterIncrement) {
  initialized_ = true;
  nominalPhase_ = 0U;
  latchedPhaseIncrement_ = quarterIncrement * 2UL;
  pulseSamplesRemaining_ = 0U;
  lateDelaySamplesRemaining_ = 0U;
  externalMidpointSeen_ = false;
  // Acquisition has no previous period from which a negative jitter position
  // could have been predicted, so the new phrase origin is emitted on-grid.
  launchPulse(3840U);
  prepareUpcomingEvent(textureControl);
}

uint16_t HumanizeAlgorithm::step(const ControlFrame& controls) {
  const uint16_t texture = sumAdc(controls.textureKnob, controls.textureCv);
  const uint32_t internalQuarterIncrement = electronicamath::quarterNotePhaseIncrement(
      referenceTables_, controls.speedKnob, 0U);
  const percussionmath::ClockUpdate clock =
      clockSource_.update(controls.speedCv, internalQuarterIncrement);

  if (!initialized_) {
    initialized_ = true;
    latchedPhaseIncrement_ = clock.quarterIncrement * 2UL;
    launchPulse(3840U);
    prepareUpcomingEvent(texture);
  }

  if (clock.externalAcquired) {
    synchronizeExternal(texture, clock.quarterIncrement);
  } else {
    // A positive offset is counted only after its nominal boundary. Processing
    // the delay before this sample's boundary logic gives an offset of N exactly
    // N complete scheduler samples of delay.
    if (lateDelaySamplesRemaining_ > 0U) {
      --lateDelaySamplesRemaining_;
      if (lateDelaySamplesRemaining_ == 0U) {
        launchPulse(upcomingAmplitudeDac12_);
        prepareUpcomingEvent(texture);
      }
    }

    if (clock.externalLost) {
      latchedPhaseIncrement_ = clock.quarterIncrement * 2UL;
      recomputeEarlyThreshold();
    }

    if (upcomingOffsetSamples_ < 0 && !upcomingEarlyFired_ &&
        nominalPhase_ >= earlyThreshold_) {
      launchPulse(upcomingAmplitudeDac12_);
      upcomingEarlyFired_ = true;
    }

    if (clock.externalActive) {
      if (clock.quarterBoundary) {
        nominalPhase_ = 0U;
        handleNominalBoundary(texture, clock.quarterIncrement * 2UL);
        externalMidpointSeen_ = false;
      } else {
        bool rollover = false;
        nominalPhase_ = perlinmath::advancePhase(
            nominalPhase_, latchedPhaseIncrement_, rollover);
        if (rollover && !externalMidpointSeen_) {
          handleNominalBoundary(texture, clock.quarterIncrement * 2UL);
          externalMidpointSeen_ = true;
        }
      }
    } else {
      bool rollover = false;
      nominalPhase_ = perlinmath::advancePhase(
          nominalPhase_, latchedPhaseIncrement_, rollover);
      if (rollover) {
        handleNominalBoundary(texture, clock.quarterIncrement * 2UL);
      }
    }
  }

  const uint16_t output = pulseSamplesRemaining_ > 0U
      ? currentPulseAmplitudeDac12_
      : 0U;
  if (pulseSamplesRemaining_ > 0U) {
    --pulseSamplesRemaining_;
  }
  return output;
}

}  // namespace fmd
