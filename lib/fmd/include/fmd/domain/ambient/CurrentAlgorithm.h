/**
 * @file CurrentAlgorithm.h
 * Declares deterministic long-form Current modulation for the Ambient bank.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_DOMAIN_CURRENT_ALGORITHM_H
#define FMD_DOMAIN_CURRENT_ALGORITHM_H

#include <stdint.h>

#include "fmd/domain/Types.h"
#include "fmd/ports/ReferenceTables.h"

namespace fmd {

/** @brief Three-rate deterministic long-form Ambient modulation. */
class CurrentAlgorithm {
 public:
  /** @brief Construct Current with deterministic initial phase offsets. */
  explicit CurrentAlgorithm(const IReferenceTables& referenceTables);

  /** @brief Advance Current by one 2.5 kHz sample. */
  uint16_t step(const ControlFrame& controls);

 private:
  const IReferenceTables& referenceTables_;
  uint32_t phase0_;
  uint32_t phase1_;
  uint32_t phase2_;
};

}  // namespace fmd
#endif  // FMD_DOMAIN_CURRENT_ALGORITHM_H
