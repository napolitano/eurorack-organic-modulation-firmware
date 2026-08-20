/**
 * @file BuildAlgorithm.h
 * Declares the repeating multi-bar tension-rise and micro-rate escalation algorithm.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_DOMAIN_DUBSTEP_BUILD_ALGORITHM_H
#define FMD_DOMAIN_DUBSTEP_BUILD_ALGORITHM_H

#include <stdint.h>

#include "fmd/domain/ClockSource.h"
#include "fmd/domain/Types.h"
#include "fmd/ports/ReferenceTables.h"

namespace fmd {

/** @brief Deterministic phrase-scale build contour with accelerating triangle modulation. */
class BuildAlgorithm {
 public:
  explicit BuildAlgorithm(const IReferenceTables& referenceTables);
  uint16_t step(const ControlFrame& controls);

 private:
  void start(uint16_t textureControl);
  void wrapPhrase(uint16_t textureControl);
  void handleExternalQuarter(uint16_t textureControl);

  const IReferenceTables& referenceTables_;
  clock::ClockSource clockSource_;
  uint32_t phrasePhase_;
  uint32_t microPhase_;
  uint16_t latchedTexture_;
  uint8_t phraseLengthBars_;
  uint8_t externalQuarterIndex_;
  bool initialized_;
};

}  // namespace fmd

#endif  // FMD_DOMAIN_DUBSTEP_BUILD_ALGORITHM_H
