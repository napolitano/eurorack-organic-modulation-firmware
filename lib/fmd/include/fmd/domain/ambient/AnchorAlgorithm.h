/**
 * @file AnchorAlgorithm.h
 * Declares mean-reverting stochastic Anchor modulation for the Ambient bank.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_DOMAIN_ANCHOR_ALGORITHM_H
#define FMD_DOMAIN_ANCHOR_ALGORITHM_H

#include <stdint.h>

#include "fmd/domain/ParallelLfsr.h"
#include "fmd/domain/Types.h"
#include "fmd/ports/ReferenceTables.h"

namespace fmd {

/**
 * @brief Bounded Ornstein-Uhlenbeck-inspired modulation around DAC midpoint.
 *
 * @details
 * Anchor is implemented as a fixed-point, mean-reverting AR(1)-style process.
 * Every scheduler sample first moves the signed Q1.15 state toward zero. Texture
 * then controls the target stochastic spread; when the spread is non-zero a
 * triangular innovation is drawn from the deterministic PRNG and compensated for
 * the current Ambient speed using the generated Anchor gain table.
 *
 * The implementation is deliberately described as OU-inspired rather than an
 * exact Gaussian Ornstein-Uhlenbeck process: innovations use the project's
 * triangular inverse-CDF primitive. The internal state saturates to signed Q1.15
 * and is projected around the 12-bit DAC midpoint.
 */
class AnchorAlgorithm {
 public:
  /**
   * @brief Construct Anchor at the exact centre state.
   * @param referenceTables Long-lived table provider for frequency mapping,
   *        triangular inverse-CDF data and Anchor speed compensation.
   * @param randomSeed Non-zero deterministic seed for the parallel LFSR source.
   */
  AnchorAlgorithm(const IReferenceTables& referenceTables, uint16_t randomSeed);

  /**
   * @brief Advance mean reversion and optional innovation by one sample.
   * @param controls Current 10-bit control frame. Speed changes correlation time;
   *        Texture changes stationary spread from zero to the documented maximum.
   * @return State projected to an inclusive 12-bit DAC code 0..4095.
   */
  uint16_t step(const ControlFrame& controls);

 private:
  const IReferenceTables& referenceTables_;  ///< Non-owning reference/frequency table provider.
  int16_t stateQ1F15_;                       ///< Signed mean-reverting state in Q1.15.
  uint32_t residualQ0F24_;                   ///< Fractional reversion movement retained between samples.
  int8_t residualDirection_;                 ///< Sign associated with the retained reversion residual.
  ParallelLfsr randomGenerator_;             ///< Deterministic source for triangular innovations.
};

}  // namespace fmd
#endif  // FMD_DOMAIN_ANCHOR_ALGORITHM_H
