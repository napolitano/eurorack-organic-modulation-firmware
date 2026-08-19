/**
 * @file MarkovAlgorithm.h
 * Declares the finite-state stochastic Markov algorithm from the Generative bank.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_DOMAIN_MARKOV_ALGORITHM_H
#define FMD_DOMAIN_MARKOV_ALGORITHM_H

#include <stdint.h>

#include "fmd/domain/generative/GenerativeAlgorithmMath.h"
#include "fmd/domain/ParallelLfsr.h"
#include "fmd/domain/Types.h"
#include "fmd/ports/ReferenceTables.h"

namespace fmd {

/**
 * @brief Eight-state stochastic grammar mapped to a fixed seed-defined voltage vocabulary.
 *
 * Speed controls transition rate. Texture mixes a structured transition kernel
 * with uniform exploration. State labels are deliberately decoupled from
 * voltage ordering by generating and shuffling a stratified vocabulary once at startup.
 */
class MarkovAlgorithm {
 public:
  /**
   * @brief Construct the deterministic vocabulary and initial Markov state.
   * @param referenceTables Read-only exponential table provider for Speed mapping.
   * @param randomSeed Deterministic seed for vocabulary and transitions.
   */
  MarkovAlgorithm(const IReferenceTables& referenceTables, uint16_t randomSeed);

  /**
   * @brief Advance internal phase and perform one state transition on wrap.
   * @param controls Coherent knob/CV snapshot.
   * @return Held 12-bit vocabulary value.
   */
  uint16_t step(const ControlFrame& controls);

 private:
  /** @brief Generate and Fisher-Yates-shuffle the eight-value startup vocabulary. */
  void initializeVocabulary();

  const IReferenceTables& referenceTables_;       ///< Exponential frequency table provider.
  uint32_t phaseAccumulator_;                     ///< Internal transition phase.
  uint16_t vocabulary_[markovmath::kStateCount]; ///< Fixed seed-defined 12-bit output levels.
  uint8_t currentState_;                          ///< Current symbolic state 0..7.
  uint16_t outputValue_;                          ///< Held output vocabulary value.
  ParallelLfsr randomGenerator_;                  ///< Deterministic vocabulary/transition RNG.
};

}  // namespace fmd
#endif  // FMD_DOMAIN_MARKOV_ALGORITHM_H
