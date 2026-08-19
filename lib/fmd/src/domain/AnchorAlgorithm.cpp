/**
 * @file AnchorAlgorithm.cpp
 * Implements bounded mean-reverting Anchor modulation for the Ambient bank.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "fmd/domain/AnchorAlgorithm.h"

#include "fmd/domain/AmbientAlgorithmMath.h"
#include "fmd/domain/AlgorithmMath.h"
#include "fmd/domain/FrequencyMapping.h"

namespace fmd {

AnchorAlgorithm::AnchorAlgorithm(const IReferenceTables& referenceTables,
                                 uint16_t randomSeed)
    : referenceTables_(referenceTables),
      stateQ1F15_(0),
      residualQ0F24_(0U),
      residualDirection_(0),
      randomGenerator_(randomSeed) {}

uint16_t AnchorAlgorithm::step(const ControlFrame& controls) {
  const uint32_t ambientIncrement = ambientmath::ambientPhaseIncrement(
      phaseIncrementFromControls(
          referenceTables_, controls.speedKnob, controls.speedCv, 0));

  stateQ1F15_ = anchormath::revertTowardZero(
      stateQ1F15_,
      anchormath::reversionAlphaQ0F24(ambientIncrement),
      residualQ0F24_,
      residualDirection_);

  const uint16_t spread =
      anchormath::spreadQ1F15(sumAdc(controls.textureKnob, controls.textureCv));
  if (spread != 0U) {
    const int16_t triangularSample = beziermath::triangularIcdfQ1F15(
        static_cast<uint16_t>(randomGenerator_.next() & 0x7FFFU),
        referenceTables_);
    const uint16_t innovationGain = referenceTables_.anchorInnovationGainQ1_15(
        ambientmath::anchorSpeedBucket(controls.speedKnob, controls.speedCv));
    const int16_t innovation = anchormath::scaledInnovationQ1F15(
        triangularSample, innovationGain, spread);
    stateQ1F15_ =
        anchormath::addInnovationSaturating(stateQ1F15_, innovation);
  }

  return anchormath::projectToDac12(stateQ1F15_);
}

}  // namespace fmd
