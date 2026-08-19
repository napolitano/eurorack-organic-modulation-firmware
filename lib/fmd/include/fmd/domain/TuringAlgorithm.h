/**
 * @file TuringAlgorithm.h
 * Declares the mutating shift-register algorithm from the optional Generative bank.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_DOMAIN_TURING_ALGORITHM_H
#define FMD_DOMAIN_TURING_ALGORITHM_H

#include <stdint.h>

#include "fmd/domain/ParallelLfsr.h"
#include "fmd/domain/Types.h"
#include "fmd/ports/ReferenceTables.h"

namespace fmd {

/**
 * @brief Internally clocked 16-bit shift-register loop with Texture-controlled mutation.
 *
 * Speed controls shift rate. Texture controls the probability of inverting the
 * recycled feedback bit from exactly zero to one half, where the incoming bit
 * becomes maximally independent of its previous value. Output is the upper
 * twelve bits of the current register state.
 */
class TuringAlgorithm {
 public:
  /**
   * @brief Construct one deterministic shift-register generator.
   * @param referenceTables Read-only exponential table provider for Speed mapping.
   * @param randomSeed Deterministic seed used for register initialization and mutation.
   */
  TuringAlgorithm(const IReferenceTables& referenceTables, uint16_t randomSeed);

  /**
   * @brief Advance internal phase and shift once on each phase wrap.
   * @param controls Coherent knob/CV snapshot.
   * @return Stepped 12-bit DAC code 0..4095.
   */
  uint16_t step(const ControlFrame& controls);

 private:
  const IReferenceTables& referenceTables_; ///< Exponential frequency table provider.
  uint32_t phaseAccumulator_;               ///< Internal step phase.
  uint16_t registerState_;                  ///< Mutable 16-bit sequence memory.
  uint16_t outputValue_;                    ///< Held 12-bit projection between steps.
  ParallelLfsr randomGenerator_;            ///< Deterministic initialization/mutation RNG.
};

}  // namespace fmd
#endif  // FMD_DOMAIN_TURING_ALGORITHM_H
