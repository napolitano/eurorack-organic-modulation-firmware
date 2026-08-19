/**
 * @file ShuffleAlgorithm.h
 * Declares deterministic long/short timing modulation for the Electronica bank.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_DOMAIN_ELECTRONICA_SHUFFLE_ALGORITHM_H
#define FMD_DOMAIN_ELECTRONICA_SHUFFLE_ALGORITHM_H

#include <stdint.h>

#include "fmd/domain/Types.h"
#include "fmd/ports/ReferenceTables.h"

namespace fmd {

/** @brief Free-running two-onset Shuffle pair with latched Texture timing. */
class ShuffleAlgorithm {
 public:
  explicit ShuffleAlgorithm(const IReferenceTables& referenceTables);
  uint16_t step(const ControlFrame& controls);

 private:
  void launchEnvelope();
  void latchRatio(uint16_t textureControl);

  const IReferenceTables& referenceTables_;
  uint32_t pairPhase_;
  uint32_t secondOnsetThreshold_;
  uint32_t envelopePhase_;
  bool secondOnsetTriggered_;
  bool envelopeActive_;
  bool initialized_;
};

}  // namespace fmd
#endif  // FMD_DOMAIN_ELECTRONICA_SHUFFLE_ALGORITHM_H
