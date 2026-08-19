/**
 * @file MotifAlgorithm.h
 * Declares the phrase-transformation algorithm from the optional Generative bank.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_DOMAIN_MOTIF_ALGORITHM_H
#define FMD_DOMAIN_MOTIF_ALGORITHM_H

#include <stdint.h>

#include "fmd/domain/generative/GenerativeAlgorithmMath.h"
#include "fmd/domain/ParallelLfsr.h"
#include "fmd/domain/Types.h"
#include "fmd/ports/ReferenceTables.h"

namespace fmd {

/**
 * @brief Eight-step phrase that evolves through one structural edit per cycle at most.
 *
 * Speed controls phrase step rate. Texture controls the probability of applying
 * one transformation at the complete-cycle boundary. The initial phrase and all
 * stochastic choices are deterministic for a fixed startup seed.
 */
class MotifAlgorithm {
 public:
  /**
   * @brief Construct and seed an explicit eight-value phrase.
   * @param referenceTables Read-only exponential table provider for Speed mapping.
   * @param randomSeed Deterministic seed for phrase values and transformations.
   */
  MotifAlgorithm(const IReferenceTables& referenceTables, uint16_t randomSeed);

  /**
   * @brief Advance the playhead and mutate the phrase only at complete-cycle boundaries.
   * @param controls Coherent knob/CV snapshot.
   * @return Held 12-bit phrase value.
   */
  uint16_t step(const ControlFrame& controls);

 private:
  /** @brief Apply exactly one random structural edit to the stored phrase. */
  void applyRandomEdit();

  const IReferenceTables& referenceTables_;       ///< Exponential frequency table provider.
  uint32_t phaseAccumulator_;                     ///< Internal phrase-step phase.
  uint16_t phrase_[motifmath::kPhraseLength];     ///< Explicit eight-value 12-bit phrase.
  uint8_t playhead_;                              ///< Currently emitted phrase index 0..7.
  uint16_t outputValue_;                          ///< Held current phrase output.
  ParallelLfsr randomGenerator_;                  ///< Deterministic phrase/edit RNG.
};

}  // namespace fmd
#endif  // FMD_DOMAIN_MOTIF_ALGORITHM_H
