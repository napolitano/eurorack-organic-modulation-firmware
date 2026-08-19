/**
 * @file PolymeterAlgorithm.cpp
 * Implements deterministic 4-against-odd-meter modulation.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "fmd/domain/electronica/PolymeterAlgorithm.h"

#include "fmd/domain/AlgorithmMath.h"
#include "fmd/domain/electronica/ElectronicaAlgorithmMath.h"

namespace fmd {

PolymeterAlgorithm::PolymeterAlgorithm(const IReferenceTables& referenceTables)
    : referenceTables_(referenceTables),
      sixteenthPhase_(0U),
      envelopePhase_(0U),
      envelopeAmplitudeDac12_(0U),
      primaryCountdown_(0U),
      secondaryCountdown_(0U),
      secondaryLength_(3U),
      envelopeActive_(false),
      initialized_(false) {}

void PolymeterAlgorithm::launchEnvelope(uint16_t amplitudeDac12) {
  envelopePhase_ = 0U;
  envelopeAmplitudeDac12_ = amplitudeDac12;
  envelopeActive_ = true;
}

void PolymeterAlgorithm::initialize(uint16_t textureControl) {
  secondaryLength_ = polymetermath::secondaryMeterLength(textureControl);
  primaryCountdown_ = 3U;
  secondaryCountdown_ = static_cast<uint8_t>(secondaryLength_ - 1U);
  launchEnvelope(polymetermath::stepAmplitudeDac12(true, true));
  initialized_ = true;
}

void PolymeterAlgorithm::advanceStep(uint16_t textureControl) {
  bool primaryStart = false;
  if (primaryCountdown_ == 0U) {
    primaryStart = true;
    primaryCountdown_ = 3U;
  } else {
    --primaryCountdown_;
  }

  bool secondaryStart = false;
  if (primaryStart) {
    const uint8_t requestedLength = polymetermath::secondaryMeterLength(textureControl);
    if (requestedLength != secondaryLength_) {
      secondaryLength_ = requestedLength;
      secondaryCountdown_ = static_cast<uint8_t>(secondaryLength_ - 1U);
      secondaryStart = true;
    }
  }

  if (!secondaryStart) {
    if (secondaryCountdown_ == 0U) {
      secondaryStart = true;
      secondaryCountdown_ = static_cast<uint8_t>(secondaryLength_ - 1U);
    } else {
      --secondaryCountdown_;
    }
  }

  launchEnvelope(polymetermath::stepAmplitudeDac12(primaryStart, secondaryStart));
}

uint16_t PolymeterAlgorithm::step(const ControlFrame& controls) {
  const uint16_t textureControl = sumAdc(controls.textureKnob, controls.textureCv);
  const uint32_t sixteenthIncrement = electronicamath::sixteenthNotePhaseIncrement(
      referenceTables_, controls.speedKnob, controls.speedCv);

  if (!initialized_) {
    initialize(textureControl);
  }

  bool stepRollover = false;
  sixteenthPhase_ = perlinmath::advancePhase(
      sixteenthPhase_, sixteenthIncrement, stepRollover);
  if (stepRollover) {
    advanceStep(textureControl);
  }

  const uint16_t output = envelopeActive_
      ? polymetermath::decayOutputDac12(envelopePhase_, envelopeAmplitudeDac12_)
      : 0U;

  if (envelopeActive_) {
    bool envelopeRollover = false;
    envelopePhase_ = perlinmath::advancePhase(
        envelopePhase_,
        polymetermath::envelopePhaseIncrement(sixteenthIncrement),
        envelopeRollover);
    if (envelopeRollover) {
      envelopeActive_ = false;
      envelopePhase_ = 0U;
    }
  }

  return output;
}

}  // namespace fmd
