/**
 * @file BezierAlgorithm.cpp
 * Implements the corrected Bézier-style random-destination Drift algorithm.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "fmd/domain/BezierAlgorithm.h"

#include "fmd/domain/AlgorithmMath.h"
#include "fmd/domain/FixedMath.h"
#include "fmd/domain/FrequencyMapping.h"

#include <stdint.h>

namespace fmd {

BezierAlgorithm::BezierAlgorithm(const IReferenceTables& referenceTables,
                                 uint16_t randomSeed)
    : referenceTables_(referenceTables),
      phaseAccumulator_(0U),
      segmentSpeedOffset_(0),
      segmentStartValue_(0U),
      segmentEndValue_(0U),
      segmentSpeedInitialised_(false),
      randomGenerator_(randomSeed) {
  segmentEndValue_ = static_cast<uint16_t>(randomGenerator_.next() >> 4U);
}

uint32_t BezierAlgorithm::advanceSegmentPhase(uint16_t speedKnobAdc,
                                              uint16_t speedCvAdc,
                                              bool& rollover) {
  const uint32_t phaseIncrement = phaseIncrementFromControls(
      referenceTables_, speedKnobAdc, speedCvAdc, segmentSpeedOffset_);
  phaseAccumulator_ = beziermath::advancePhase(
      phaseAccumulator_, phaseIncrement, rollover);
  return phaseAccumulator_;
}

int16_t BezierAlgorithm::sampleTriangularDistribution() {
  const uint16_t uniformSample15 =
      static_cast<uint16_t>(randomGenerator_.next() & 0x7FFFU);
  return beziermath::triangularIcdfQ1F15(uniformSample15, referenceTables_);
}

int16_t BezierAlgorithm::sampleSegmentSpeedOffset(uint16_t textureKnobAdc,
                                                  uint16_t textureCvAdc) {
  const uint16_t variationScaleQ1F15 =
      beziermath::speedVariationScaleQ1F15(textureKnobAdc, textureCvAdc);
  if (variationScaleQ1F15 == 0U) {
    return 0;
  }

  return fixedmath::mulI1F15(sampleTriangularDistribution(),
                             static_cast<int16_t>(variationScaleQ1F15));
}

uint16_t BezierAlgorithm::step(const ControlFrame& controls) {
  if (!segmentSpeedInitialised_) {
    segmentSpeedOffset_ =
        sampleSegmentSpeedOffset(controls.textureKnob, controls.textureCv);
    segmentSpeedInitialised_ = true;
  }

  bool segmentRolledOver = false;
  const uint32_t phaseAccumulator = advanceSegmentPhase(
      controls.speedKnob, controls.speedCv, segmentRolledOver);

  if (segmentRolledOver) {
    // Endpoint B becomes the next segment's endpoint A. This preserves output
    // continuity while a fresh random destination and held timing offset are
    // selected for the new segment.
    segmentStartValue_ = segmentEndValue_;
    segmentEndValue_ = static_cast<uint16_t>(randomGenerator_.next() >> 4U);
    segmentSpeedOffset_ =
        sampleSegmentSpeedOffset(controls.textureKnob, controls.textureCv);
  }

  const uint16_t phaseQ4F12 =
      static_cast<uint16_t>(phaseAccumulator >> 20U);
  return beziermath::interpolateQ4F12(phaseQ4F12,
                                      segmentStartValue_,
                                      segmentEndValue_,
                                      controls.textureKnob);
}

}  // namespace fmd
