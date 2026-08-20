/**
 * @file WobbleAlgorithm.h
 * Declares the tempo-synchronised deterministic rate-phrase modulation algorithm.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_DOMAIN_DUBSTEP_WOBBLE_ALGORITHM_H
#define FMD_DOMAIN_DUBSTEP_WOBBLE_ALGORITHM_H

#include <stdint.h>

#include "fmd/domain/ClockSource.h"
#include "fmd/domain/Types.h"
#include "fmd/ports/ReferenceTables.h"

namespace fmd {

/** @brief One-bar deterministic Wobble rate phrase with continuous triangle carrier phase. */
class WobbleAlgorithm {
 public:
  explicit WobbleAlgorithm(const IReferenceTables& referenceTables);
  uint16_t step(const ControlFrame& controls);

 private:
  void start(uint16_t textureControl);
  void advanceCell(uint16_t textureControl);
  void handleExternalQuarter(uint16_t textureControl);

  const IReferenceTables& referenceTables_;
  clock::ClockSource clockSource_;
  uint32_t gridPhase_;
  uint32_t carrierPhase_;
  uint8_t cellIndex_;
  uint8_t latchedTextureRegion_;
  uint8_t externalQuarterIndex_;
  bool externalHalfAdvanced_;
  bool initialized_;
};

}  // namespace fmd

#endif  // FMD_DOMAIN_DUBSTEP_WOBBLE_ALGORITHM_H
