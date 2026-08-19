/**
 * @file BreathAlgorithm.cpp
 * Implements recurrent cycle-varied Breath modulation for the Ambient bank.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "fmd/domain/ambient/BreathAlgorithm.h"

#include "fmd/domain/ambient/AmbientAlgorithmMath.h"
#include "fmd/domain/AlgorithmMath.h"
#include "fmd/domain/FrequencyMapping.h"

namespace fmd {

BreathAlgorithm::BreathAlgorithm(const IReferenceTables& referenceTables,
                                 uint16_t randomSeed)
    : referenceTables_(referenceTables),
      phase_(0U),
      durationQ10_(breathmath::kDurationNominalQ10),
      rateScaleQ10_(1024U),
      amplitudeDac12_(breathmath::kAmplitudeNominalDac12),
      skewQ12_(breathmath::kSkewNominalQ0F12),
      attackReciprocalQ12_(
          breathmath::segmentReciprocalQ12(breathmath::kSkewNominalQ0F12)),
      releaseReciprocalQ12_(breathmath::segmentReciprocalQ12(
          static_cast<uint16_t>(4096U - breathmath::kSkewNominalQ0F12))),
      randomGenerator_(randomSeed) {}

void BreathAlgorithm::latchCycle(uint16_t textureControl) {
  durationQ10_ = breathmath::variedParameter(
      randomGenerator_.next(),
      textureControl,
      breathmath::kDurationMinimumQ10,
      breathmath::kDurationNominalQ10,
      breathmath::kDurationMaximumQ10);
  amplitudeDac12_ = breathmath::variedParameter(
      randomGenerator_.next(),
      textureControl,
      breathmath::kAmplitudeMinimumDac12,
      breathmath::kAmplitudeNominalDac12,
      breathmath::kAmplitudeMaximumDac12);
  skewQ12_ = breathmath::variedParameter(
      randomGenerator_.next(),
      textureControl,
      breathmath::kSkewMinimumQ0F12,
      breathmath::kSkewNominalQ0F12,
      breathmath::kSkewMaximumQ0F12);

  rateScaleQ10_ = breathmath::rateScaleQ10(durationQ10_);
  attackReciprocalQ12_ = breathmath::segmentReciprocalQ12(skewQ12_);
  releaseReciprocalQ12_ = breathmath::segmentReciprocalQ12(
      static_cast<uint16_t>(4096U - skewQ12_));
}

uint16_t BreathAlgorithm::step(const ControlFrame& controls) {
  const uint32_t ambientIncrement = ambientmath::ambientPhaseIncrement(
      phaseIncrementFromControls(
          referenceTables_, controls.speedKnob, controls.speedCv, 0));

  bool rollover = false;
  phase_ = perlinmath::advancePhase(
      phase_,
      breathmath::scaledPhaseIncrement(ambientIncrement, rateScaleQ10_),
      rollover);
  if (rollover) {
    latchCycle(sumAdc(controls.textureKnob, controls.textureCv));
  }

  const uint16_t envelope = breathmath::envelopeQ0F12(
      static_cast<uint16_t>(phase_ >> 20U),
      skewQ12_,
      attackReciprocalQ12_,
      releaseReciprocalQ12_);
  return breathmath::applyAmplitude(envelope, amplitudeDac12_);
}

}  // namespace fmd
