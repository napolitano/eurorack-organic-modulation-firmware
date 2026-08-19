/**
 * @file AcidAlgorithm.cpp
 * Implements deterministic stepped/sliding Acid modulation.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "fmd/domain/electronica/AcidAlgorithm.h"

#include "fmd/domain/AlgorithmMath.h"
#include "fmd/domain/electronica/ElectronicaAlgorithmMath.h"

namespace fmd {

AcidAlgorithm::AcidAlgorithm(const IReferenceTables& referenceTables)
    : referenceTables_(referenceTables),
      phase_(0U),
      stepIndex_(0U),
      previousTargetDac12_(acidmath::baseTargetDac12(15U)),
      currentTargetDac12_(acidmath::baseTargetDac12(0U)) {}

uint16_t AcidAlgorithm::step(const ControlFrame& controls) {
  bool rollover = false;
  phase_ = perlinmath::advancePhase(
      phase_,
      electronicamath::sixteenthNotePhaseIncrement(
          referenceTables_, controls.speedKnob, controls.speedCv),
      rollover);
  if (rollover) {
    previousTargetDac12_ = currentTargetDac12_;
    stepIndex_ = static_cast<uint8_t>((stepIndex_ + 1U) & 0x0FU);
    currentTargetDac12_ = acidmath::baseTargetDac12(stepIndex_);
  }

  const uint16_t phaseQ0F12 = static_cast<uint16_t>(phase_ >> 20U);
  const uint16_t textureQ0F12 = electronicamath::textureQ0F12(
      sumAdc(controls.textureKnob, controls.textureCv));

  uint16_t base = currentTargetDac12_;
  if (acidmath::isSlideStep(stepIndex_)) {
    base = acidmath::slideContourDac12(
        previousTargetDac12_, currentTargetDac12_, phaseQ0F12, textureQ0F12);
  }

  const uint16_t accent = acidmath::isAccentStep(stepIndex_)
      ? acidmath::accentContributionDac12(phaseQ0F12, textureQ0F12)
      : 0U;
  return acidmath::addAccentSaturating(base, accent);
}

}  // namespace fmd
