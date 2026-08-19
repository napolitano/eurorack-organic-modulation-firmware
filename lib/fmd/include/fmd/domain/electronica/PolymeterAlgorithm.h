/**
 * @file PolymeterAlgorithm.h
 * Declares deterministic 4-against-odd-meter modulation for the Electronica bank.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_DOMAIN_ELECTRONICA_POLYMETER_ALGORITHM_H
#define FMD_DOMAIN_ELECTRONICA_POLYMETER_ALGORITHM_H

#include <stdint.h>

#include "fmd/domain/Types.h"
#include "fmd/ports/ReferenceTables.h"

namespace fmd {

/** @brief Shared-grid polymetric accent process with exact integer meter lengths. */
class PolymeterAlgorithm {
 public:
  explicit PolymeterAlgorithm(const IReferenceTables& referenceTables);
  uint16_t step(const ControlFrame& controls);

 private:
  void launchEnvelope(uint16_t amplitudeDac12);
  void initialize(uint16_t textureControl);
  void advanceStep(uint16_t textureControl);

  const IReferenceTables& referenceTables_;
  uint32_t sixteenthPhase_;
  uint32_t envelopePhase_;
  uint16_t envelopeAmplitudeDac12_;
  uint8_t primaryCountdown_;
  uint8_t secondaryCountdown_;
  uint8_t secondaryLength_;
  bool envelopeActive_;
  bool initialized_;
};

}  // namespace fmd
#endif  // FMD_DOMAIN_ELECTRONICA_POLYMETER_ALGORITHM_H
