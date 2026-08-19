/**
 * @file PumpAlgorithm.h
 * Declares the free-running duck/recovery Pump algorithm for the Electronica bank.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_DOMAIN_ELECTRONICA_PUMP_ALGORITHM_H
#define FMD_DOMAIN_ELECTRONICA_PUMP_ALGORITHM_H

#include <stdint.h>

#include "fmd/domain/Types.h"
#include "fmd/ports/ReferenceTables.h"

namespace fmd {

/** @brief Quarter-note sidechain-style duck/recovery contour. */
class PumpAlgorithm {
 public:
  explicit PumpAlgorithm(const IReferenceTables& referenceTables);
  uint16_t step(const ControlFrame& controls);

 private:
  void updateRecovery(uint16_t textureControl);

  const IReferenceTables& referenceTables_;
  uint32_t phase_;
  uint16_t cachedTexture_;
  uint16_t recoveryEndpointQ0F16_;
  uint16_t recoveryReciprocalQ28_;
};

}  // namespace fmd
#endif  // FMD_DOMAIN_ELECTRONICA_PUMP_ALGORITHM_H
