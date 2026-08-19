/**
 * @file FogAlgorithm.cpp
 * Implements the bounded four-voice Fog cloud process for the Ambient bank.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "fmd/domain/FogAlgorithm.h"

#include "fmd/domain/AmbientAlgorithmMath.h"
#include "fmd/domain/FrequencyMapping.h"

namespace fmd {

FogAlgorithm::FogAlgorithm(const IReferenceTables& referenceTables,
                           uint16_t randomSeed)
    : referenceTables_(referenceTables), voices_{}, randomGenerator_(randomSeed) {
  for (uint8_t index = 0U; index < fogmath::kVoiceCount; ++index) {
    voices_[index] = {0U, 0, false};
  }
}

uint16_t FogAlgorithm::step(const ControlFrame& controls) {
  const uint32_t ambientIncrement = ambientmath::ambientPhaseIncrement(
      phaseIncrementFromControls(
          referenceTables_, controls.speedKnob, controls.speedCv, 0));

  int32_t signedContribution = 0L;
  for (uint8_t index = 0U; index < fogmath::kVoiceCount; ++index) {
    signedContribution += fogmath::voiceContributionAndAdvance(
        voices_[index], ambientIncrement);
  }

  const uint32_t random32 =
      (static_cast<uint32_t>(randomGenerator_.next()) << 16U) |
      randomGenerator_.next();
  const uint8_t occupancy = fogmath::targetOccupancyEighths(
      sumAdc(controls.textureKnob, controls.textureCv));
  if (random32 < fogmath::eventCutoffQ0F32(ambientIncrement, occupancy)) {
    for (uint8_t index = 0U; index < fogmath::kVoiceCount; ++index) {
      if (!voices_[index].active) {
        voices_[index] = {
            0U,
            fogmath::amplitudeFromRandom(randomGenerator_.next()),
            true,
        };
        break;
      }
    }
  }

  return fogmath::projectToDac12(signedContribution);
}

}  // namespace fmd
