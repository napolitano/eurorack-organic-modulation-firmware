/**
 * @file FractalAlgorithm.h
 * Declares the three-scale gradient-noise Fractal algorithm from the optional Organic bank.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_DOMAIN_FRACTAL_ALGORITHM_H
#define FMD_DOMAIN_FRACTAL_ALGORITHM_H

#include <stdint.h>

#include "fmd/domain/ParallelLfsr.h"
#include "fmd/domain/Types.h"
#include "fmd/ports/ReferenceTables.h"

namespace fmd {

/**
 * @brief Multi-scale gradient-noise modulation with Texture-controlled roughness.
 *
 * The algorithm sums three continuous one-dimensional gradient-noise octaves at
 * relative rates 1x, 4x and 16x. Texture redistributes a constant total weight
 * from the macro octave toward the two finer scales. This is a procedural
 * fractal-noise construction; it is intentionally not presented as an exact
 * statistical implementation of fractional Brownian motion.
 */
class FractalAlgorithm {
 public:
  /**
   * @brief Construct and seed all three gradient-noise scales.
   * @param referenceTables Read-only exponential table provider for Speed mapping.
   * @param randomSeed Deterministic 16-bit seed for all octave gradients.
   */
  FractalAlgorithm(const IReferenceTables& referenceTables, uint16_t randomSeed);

  /**
   * @brief Advance all three scales and render one output sample.
   * @param controls Coherent knob/CV snapshot.
   * @return 12-bit DAC code 0..4095.
   */
  uint16_t step(const ControlFrame& controls);

 private:
  /** @brief Mutable state of one continuous gradient-noise octave. */
  struct OctaveState {
    uint32_t phaseAccumulator;  ///< Full 32-bit phase accumulator.
    int16_t startGradient;      ///< Left-lattice gradient, signed Q1.15.
    int16_t endGradient;        ///< Right-lattice gradient, signed Q1.15.
  };

  /** @return Next gradient from Drift's verified finite Q1.15 gradient set. */
  int16_t nextRandomGradient();

  /**
   * @brief Advance and evaluate one gradient-noise octave.
   * @param octaveState Mutable octave phase and endpoint gradients.
   * @param phaseIncrement Per-sample 32-bit phase increment.
   * @return Signed Q1.15 noise sample.
   */
  int16_t advanceOctave(OctaveState& octaveState, uint32_t phaseIncrement);

  const IReferenceTables& referenceTables_; ///< Exponential frequency table provider.
  OctaveState macroOctave_;                 ///< Large-scale 1x motion.
  OctaveState mesoOctave_;                  ///< Medium-scale 4x motion.
  OctaveState detailOctave_;                ///< Fine-scale 16x motion.
  ParallelLfsr randomGenerator_;            ///< Shared deterministic gradient source.
};

}  // namespace fmd
#endif  // FMD_DOMAIN_FRACTAL_ALGORITHM_H
