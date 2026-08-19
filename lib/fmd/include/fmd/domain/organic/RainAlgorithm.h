/**
 * @file RainAlgorithm.h
 * Declares the stochastic Rain/shot-noise algorithm from the optional Organic bank.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_DOMAIN_RAIN_ALGORITHM_H
#define FMD_DOMAIN_RAIN_ALGORITHM_H

#include <stdint.h>

#include "fmd/domain/ParallelLfsr.h"
#include "fmd/domain/Types.h"

namespace fmd {

/**
 * @brief Discrete-time stochastic impulse process with an aggregate decaying envelope.
 *
 * Texture controls event Density through a quadratic Bernoulli threshold. Speed
 * controls how quickly accumulated drop energy decays. Each event adds a random
 * positive impulse; overlapping tails naturally accumulate and saturate. The
 * front-panel output Attenuation remains the final analog Intensity control.
 */
class RainAlgorithm {
 public:
  /**
   * @brief Construct a deterministic Rain generator.
   * @param randomSeed Initial 16-bit seed for event timing and impulse amplitude.
   */
  explicit RainAlgorithm(uint16_t randomSeed);

  /**
   * @brief Advance the aggregate rain envelope by one processing sample.
   * @param controls Coherent knob/CV snapshot.
   * @return 12-bit DAC code 0..4095.
   */
  uint16_t step(const ControlFrame& controls);

 private:
  uint16_t envelopeValue_;        ///< Aggregate rain energy in full 16-bit domain.
  uint16_t decayResidualQ0F16_;   ///< Fractional leaky-envelope decay remainder.
  ParallelLfsr randomGenerator_;  ///< Event-timing and impulse-amplitude random source.
};

}  // namespace fmd
#endif  // FMD_DOMAIN_RAIN_ALGORITHM_H
