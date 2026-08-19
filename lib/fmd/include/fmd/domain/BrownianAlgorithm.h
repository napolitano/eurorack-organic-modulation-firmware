/**
 * @file BrownianAlgorithm.h
 * Declares the portable corrected Brownian Drift algorithm.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_DOMAIN_BROWNIAN_ALGORITHM_H
#define FMD_DOMAIN_BROWNIAN_ALGORITHM_H

#include <stdint.h>

#include "fmd/domain/ParallelLfsr.h"
#include "fmd/domain/Types.h"

namespace fmd {

/**
 * @brief Bounded random walk with centre bias and fractional first-order smoothing.
 *
 * Speed controls movement probability and target step size. Texture controls the
 * smoothing coefficient used to follow that random-walk target. Internal state
 * remains 16-bit and is reduced to 12 bits only at the output boundary.
 */
class BrownianAlgorithm {
 public:
  /**
   * @brief Construct a Brownian generator with deterministic random seed.
   * @param randomSeed Initial 16-bit seed for movement-event and direction decisions.
   */
  explicit BrownianAlgorithm(uint16_t randomSeed);

  /**
   * @brief Advance the random walk and smoother by one processing sample.
   * @param controls Coherent knob/CV snapshot.
   * @return 12-bit DAC code 0..4095.
   */
  uint16_t step(const ControlFrame& controls);

 private:
  /**
   * @brief Potentially move the bounded random-walk target once.
   * @param speedControl Combined Speed knob/CV value in the saturated 10-bit domain.
   */
  void updateTargetValue(uint16_t speedControl);

  /**
   * @brief Move the visible state toward the target using Texture-derived smoothing.
   * @param textureControl Combined Texture knob/CV value in the saturated 10-bit domain.
   */
  void updateSmoothedValue(uint16_t textureControl);

  uint16_t targetValue_;         ///< Random-walk target in the full 16-bit state domain.
  uint16_t currentValue_;        ///< Smoothed current state in the full 16-bit domain.
  uint16_t smoothingResidual_;   ///< Q0.16 fractional movement not yet large enough for one code.
  int8_t smoothingDirection_;    ///< Sign associated with the retained fractional residual.
  ParallelLfsr randomGenerator_; ///< Deterministic source for event and direction decisions.
};

}  // namespace fmd
#endif  // FMD_DOMAIN_BROWNIAN_ALGORITHM_H
