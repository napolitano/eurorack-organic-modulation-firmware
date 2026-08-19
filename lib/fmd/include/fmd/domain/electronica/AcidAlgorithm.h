/**
 * @file AcidAlgorithm.h
 * Declares deterministic stepped/sliding Acid modulation for the Electronica bank.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_DOMAIN_ELECTRONICA_ACID_ALGORITHM_H
#define FMD_DOMAIN_ELECTRONICA_ACID_ALGORITHM_H

#include <stdint.h>

#include "fmd/domain/Types.h"
#include "fmd/ports/ReferenceTables.h"

namespace fmd {

/** @brief Deterministic 16-step CV grammar with project-defined accents and slides. */
class AcidAlgorithm {
 public:
  explicit AcidAlgorithm(const IReferenceTables& referenceTables);
  uint16_t step(const ControlFrame& controls);

 private:
  const IReferenceTables& referenceTables_;
  uint32_t phase_;
  uint8_t stepIndex_;
  uint16_t previousTargetDac12_;
  uint16_t currentTargetDac12_;
};

}  // namespace fmd
#endif  // FMD_DOMAIN_ELECTRONICA_ACID_ALGORITHM_H
