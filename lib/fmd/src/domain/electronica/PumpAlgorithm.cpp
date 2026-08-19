/**
 * @file PumpAlgorithm.cpp
 * Implements the free-running duck/recovery Pump algorithm.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "fmd/domain/electronica/PumpAlgorithm.h"

#include "fmd/domain/AlgorithmMath.h"
#include "fmd/domain/electronica/ElectronicaAlgorithmMath.h"

namespace fmd {

PumpAlgorithm::PumpAlgorithm(const IReferenceTables& referenceTables)
    : referenceTables_(referenceTables),
      phase_(0U),
      cachedTexture_(0xFFFFU),
      recoveryEndpointQ0F16_(pumpmath::kRecoveryMinimumQ0F16),
      recoveryReciprocalQ28_(pumpmath::recoveryReciprocalQ28(
          pumpmath::kRecoveryMinimumQ0F16)) {}

void PumpAlgorithm::updateRecovery(uint16_t textureControl) {
  textureControl = clampAdc(textureControl);
  if (textureControl == cachedTexture_) {
    return;
  }
  cachedTexture_ = textureControl;
  recoveryEndpointQ0F16_ = pumpmath::recoveryEndpointQ0F16(textureControl);
  recoveryReciprocalQ28_ = pumpmath::recoveryReciprocalQ28(recoveryEndpointQ0F16_);
}

uint16_t PumpAlgorithm::step(const ControlFrame& controls) {
  updateRecovery(sumAdc(controls.textureKnob, controls.textureCv));
  bool rollover = false;
  phase_ = perlinmath::advancePhase(
      phase_,
      electronicamath::quarterNotePhaseIncrement(
          referenceTables_, controls.speedKnob, controls.speedCv),
      rollover);
  (void)rollover;

  return pumpmath::outputDac12(
      static_cast<uint16_t>(phase_ >> 16U),
      recoveryEndpointQ0F16_,
      recoveryReciprocalQ28_);
}

}  // namespace fmd
