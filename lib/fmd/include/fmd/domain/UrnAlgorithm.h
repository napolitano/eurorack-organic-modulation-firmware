/**
 * @file UrnAlgorithm.h
 * Declares the leaky reinforced-state algorithm from the optional Generative bank.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_DOMAIN_URN_ALGORITHM_H
#define FMD_DOMAIN_URN_ALGORITHM_H

#include <stdint.h>

#include "fmd/domain/GenerativeAlgorithmMath.h"
#include "fmd/domain/ParallelLfsr.h"
#include "fmd/domain/Types.h"
#include "fmd/ports/ReferenceTables.h"

namespace fmd {

/**
 * @brief Eight-state Pólya-inspired process with bounded, decaying preference weights.
 *
 * Speed controls draw rate. Texture controls positive reinforcement. All state
 * weights relax by 31/32 toward a common baseline before each new weighted draw,
 * preventing early random preferences from becoming permanent.
 */
class UrnAlgorithm {
 public:
  /**
   * @brief Construct equal baseline weights and deterministic random state.
   * @param referenceTables Read-only exponential table provider for Speed mapping.
   * @param randomSeed Deterministic seed for weighted state selection.
   */
  UrnAlgorithm(const IReferenceTables& referenceTables, uint16_t randomSeed);

  /**
   * @brief Advance phase and perform one relaxed/reinforced categorical draw on wrap.
   * @param controls Coherent knob/CV snapshot.
   * @return Held fixed-vocabulary 12-bit DAC code.
   */
  uint16_t step(const ControlFrame& controls);

 private:
  const IReferenceTables& referenceTables_;   ///< Exponential frequency table provider.
  uint32_t phaseAccumulator_;                 ///< Internal draw phase.
  uint16_t weights_[urnmath::kStateCount];    ///< Mutable bounded preference weights.
  uint8_t currentState_;                      ///< Most recently selected state.
  uint16_t outputValue_;                      ///< Held fixed-vocabulary output value.
  ParallelLfsr randomGenerator_;              ///< Deterministic weighted-selection RNG.
};

}  // namespace fmd
#endif  // FMD_DOMAIN_URN_ALGORITHM_H
