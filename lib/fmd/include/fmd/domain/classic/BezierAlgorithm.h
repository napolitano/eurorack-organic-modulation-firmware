/**
 * @file BezierAlgorithm.h
 * Declares the portable corrected Bézier-style Drift algorithm.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_DOMAIN_BEZIER_ALGORITHM_H
#define FMD_DOMAIN_BEZIER_ALGORITHM_H

#include <stdint.h>

#include "fmd/domain/ParallelLfsr.h"
#include "fmd/domain/Types.h"
#include "fmd/ports/ReferenceTables.h"

namespace fmd {

/**
 * @brief Random destinations connected by continuously morphed cubic trajectories.
 *
 * Each wrapped segment chooses a new destination and a triangularly distributed
 * log-frequency offset. The Texture knob continuously morphs the interpolation
 * curve, while Texture CV contributes to the amount of segment-speed variation.
 */
class BezierAlgorithm {
 public:
  /**
   * @brief Construct a seeded Bézier modulation generator.
   * @param referenceTables Exponential and triangular inverse-CDF table provider.
   * @param randomSeed Deterministic 16-bit seed for destination and timing randomness.
   */
  BezierAlgorithm(const IReferenceTables& referenceTables, uint16_t randomSeed);

  /**
   * @brief Advance the current segment by one processing sample.
   * @param controls Coherent knob/CV snapshot.
   * @return 12-bit DAC code 0..4095.
   */
  uint16_t step(const ControlFrame& controls);

 private:
  /**
   * @brief Advance segment phase using Speed controls and the current random offset.
   * @param speedKnobAdc Speed potentiometer ADC code, expected range 0..1023.
   * @param speedCvAdc Speed CV ADC code, expected range 0..1023.
   * @param rollover Set to true when the 32-bit segment phase wraps through zero.
   * @return Advanced 32-bit segment phase with overshoot preserved.
   */
  uint32_t advanceSegmentPhase(uint16_t speedKnobAdc,
                               uint16_t speedCvAdc,
                               bool& rollover);

  /**
   * @brief Draw a new triangularly distributed signed segment-speed offset.
   * @param textureKnobAdc Texture potentiometer ADC code, expected range 0..1023.
   * @param textureCvAdc Texture CV ADC code, expected range 0..1023.
   * @return Signed raw frequency offset consumed by phaseIncrementFromControls().
   */
  int16_t sampleSegmentSpeedOffset(uint16_t textureKnobAdc, uint16_t textureCvAdc);

  /** @return One signed Q1.15 sample from the triangular inverse-CDF table. */
  int16_t sampleTriangularDistribution();

  const IReferenceTables& referenceTables_; ///< Exponential and ICDF table provider.
  uint32_t phaseAccumulator_;               ///< Current 32-bit segment phase.
  int16_t segmentSpeedOffset_;              ///< Random frequency offset held for one segment.
  uint16_t segmentStartValue_;              ///< Current segment start, 12-bit domain.
  uint16_t segmentEndValue_;                ///< Current segment destination, 12-bit domain.
  bool segmentSpeedInitialised_;            ///< Ensures first segment receives an offset immediately.
  ParallelLfsr randomGenerator_;            ///< Destination and timing random source.
};

}  // namespace fmd
#endif  // FMD_DOMAIN_BEZIER_ALGORITHM_H
