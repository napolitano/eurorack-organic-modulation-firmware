/**
 * @file PerlinAlgorithm.h
 * Declares the portable two-octave Perlin / gradient-noise Drift algorithm.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_DOMAIN_PERLIN_ALGORITHM_H
#define FMD_DOMAIN_PERLIN_ALGORITHM_H

#include <stdint.h>

#include "fmd/domain/ParallelLfsr.h"
#include "fmd/domain/Types.h"
#include "fmd/ports/ReferenceTables.h"

namespace fmd {

/**
 * @brief Smooth stochastic modulation based on two one-dimensional noise octaves.
 *
 * The base octave follows the mapped Speed control. A second octave advances at
 * four times that phase rate. Texture crossfades additional high-frequency
 * detail into the output while preserving continuous segment boundaries.
 */
class PerlinAlgorithm {
 public:
  /**
   * @brief Construct and seed both gradient-noise octaves.
   * @param referenceTables Read-only exponential lookup provider for Speed/CV mapping.
   * @param randomSeed Initial 16-bit seed for deterministic gradient generation.
   */
  PerlinAlgorithm(const IReferenceTables& referenceTables, uint16_t randomSeed);

  /**
   * @brief Advance the algorithm by one processing sample.
   * @param controls Coherent knob/CV snapshot.
   * @return 12-bit DAC code 0..4095.
   */
  uint16_t step(const ControlFrame& controls);

 private:
  /** @brief Mutable state of one continuous one-dimensional noise octave. */
  struct OctaveState {
    uint32_t phaseAccumulator;  ///< Full 32-bit phase; upper 16 bits select segment position.
    int16_t startGradient;      ///< Gradient at the left lattice point, Q1.15.
    int16_t endGradient;        ///< Gradient at the right lattice point, Q1.15.
  };

  /**
   * @brief Draw the next random gradient from Drift's finite gradient set.
   * @param randomGenerator Deterministic paired LFSR advanced exactly once.
   * @return Signed Q1.15 gradient selected from the 16-value gradient set.
   */
  static int16_t nextRandomGradient(ParallelLfsr& randomGenerator);

  /**
   * @brief Advance one octave, rotate gradients on wrap and evaluate the active segment.
   * @param octaveState Mutable phase and endpoint gradients for the octave.
   * @param phaseIncrement Per-sample 32-bit phase increment.
   * @return Signed Q1.15 gradient-noise value for the advanced octave state.
   */
  int16_t advanceOctave(OctaveState& octaveState, uint32_t phaseIncrement);

  const IReferenceTables& referenceTables_;  ///< Exponential frequency table provider.
  OctaveState baseOctave_;                   ///< Fundamental gradient-noise octave.
  OctaveState detailOctave_;                 ///< Four-times-faster detail octave.
  ParallelLfsr randomGenerator_;             ///< Deterministic gradient source.
};

}  // namespace fmd
#endif  // FMD_DOMAIN_PERLIN_ALGORITHM_H
