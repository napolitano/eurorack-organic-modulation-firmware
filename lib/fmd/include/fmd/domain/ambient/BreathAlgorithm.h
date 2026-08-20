/**
 * @file BreathAlgorithm.h
 * Declares cycle-varied recurrent Breath modulation for the Ambient bank.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_DOMAIN_BREATH_ALGORITHM_H
#define FMD_DOMAIN_BREATH_ALGORITHM_H

#include <stdint.h>

#include "fmd/domain/ParallelLfsr.h"
#include "fmd/domain/Types.h"
#include "fmd/ports/ReferenceTables.h"

namespace fmd {

/**
 * @brief Smooth baseline-to-peak-to-baseline gesture with rollover-only variation.
 *
 * @details
 * Breath keeps one coherent gesture for an entire cycle. At rollover, three
 * bounded random parameters are latched: duration, peak amplitude and attack/
 * release skew. Texture controls the amount of variation around their explicit
 * nominal values. Reciprocals for the attack and release segments are computed
 * only at rollover so the per-sample hot path contains no general division.
 */
class BreathAlgorithm {
 public:
  /**
   * @brief Construct Breath with the documented nominal first cycle.
   * @param referenceTables Long-lived table provider used for the shared Drift
   *        frequency mapping.
   * @param randomSeed Deterministic seed used for subsequent cycle variation.
   */
  BreathAlgorithm(const IReferenceTables& referenceTables, uint16_t randomSeed);

  /**
   * @brief Advance the current Breath gesture by one scheduler sample.
   * @param controls Current 10-bit control frame. Speed sets the Ambient base
   *        rate; Texture controls the variation latched at the next rollover.
   * @return Unipolar 12-bit DAC code in the inclusive range 0..4095.
   */
  uint16_t step(const ControlFrame& controls);

 private:
  /**
   * @brief Draw and cache all parameters for the next complete gesture.
   * @param textureControl Saturated 10-bit Texture value controlling variation depth.
   */
  void latchCycle(uint16_t textureControl);

  const IReferenceTables& referenceTables_;  ///< Non-owning frequency/reference-table provider.
  uint32_t phase_;                           ///< Unsigned 32-bit cycle phase accumulator.
  uint16_t durationQ10_;                     ///< Latched cycle-duration multiplier, 1024 = nominal.
  uint16_t rateScaleQ10_;                    ///< Cached reciprocal duration multiplier, 1024 = unity.
  uint16_t amplitudeDac12_;                  ///< Latched 12-bit peak amplitude for this cycle.
  uint16_t skewQ12_;                         ///< Latched peak position in Q0.12 phase units.
  uint16_t attackReciprocalQ12_;             ///< Cached Q12 reciprocal of the attack segment length.
  uint16_t releaseReciprocalQ12_;            ///< Cached Q12 reciprocal of the release segment length.
  ParallelLfsr randomGenerator_;             ///< Deterministic source for rollover parameter variation.
};

}  // namespace fmd
#endif  // FMD_DOMAIN_BREATH_ALGORITHM_H
