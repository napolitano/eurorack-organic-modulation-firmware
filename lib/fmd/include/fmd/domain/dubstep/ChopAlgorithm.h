/**
 * @file ChopAlgorithm.h
 * Declares the deterministic 16-step sparse/syncopated articulation algorithm.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_DOMAIN_DUBSTEP_CHOP_ALGORITHM_H
#define FMD_DOMAIN_DUBSTEP_CHOP_ALGORITHM_H

#include <stdint.h>

#include "fmd/domain/ClockSource.h"
#include "fmd/domain/Types.h"
#include "fmd/ports/ReferenceTables.h"

namespace fmd {

/** @brief Deterministic bar-latched sparse articulation pattern for sustained audio destinations. */
class ChopAlgorithm {
 public:
  explicit ChopAlgorithm(const IReferenceTables& referenceTables);
  uint16_t step(const ControlFrame& controls);

 private:
  void start(uint16_t textureControl);
  void advanceStep(uint16_t textureControl);
  void handleExternalQuarter(uint16_t textureControl);

  const IReferenceTables& referenceTables_;
  clock::ClockSource clockSource_;
  uint32_t stepPhase_;
  uint8_t stepIndex_;
  uint16_t latchedMask_;
  uint8_t externalQuarterIndex_;
  uint8_t externalSubdivisions_;
  bool initialized_;
};

}  // namespace fmd

#endif  // FMD_DOMAIN_DUBSTEP_CHOP_ALGORITHM_H
