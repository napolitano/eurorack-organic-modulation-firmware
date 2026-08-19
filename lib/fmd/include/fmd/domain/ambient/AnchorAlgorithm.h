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

/** @brief Bounded OU-inspired AR(1) modulation around the DAC midpoint. */
class AnchorAlgorithm {
 public:
  /** @brief Construct Anchor at its exact centre state. */
  AnchorAlgorithm(const IReferenceTables& referenceTables, uint16_t randomSeed);

  /** @brief Advance the mean-reverting process by one sample. */
  uint16_t step(const ControlFrame& controls);

 private:
  const IReferenceTables& referenceTables_;
  int16_t stateQ1F15_;
  uint32_t residualQ0F24_;
  int8_t residualDirection_;
  ParallelLfsr randomGenerator_;
};

}  // namespace fmd
#endif  // FMD_DOMAIN_ANCHOR_ALGORITHM_H
