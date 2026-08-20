/**
 * @file BuildAlgorithm.h
 * Declares the repeating multi-bar tension-rise and micro-rate escalation algorithm.
 *
 * @details Build combines a normalized phrase-scale smoothstep rise with a
 * triangle carrier whose rate advances through quarter/eighth/sixteenth/32nd
 * stages. Texture selects 8/4/2/1-bar phrase length and is latched only at phrase
 * restart. External quarter edges re-anchor phrase position deterministically.
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
  /**
   * @brief Construct an eight-bar default Build at phrase/micro phase zero.
   * @param referenceTables Read-only exponential lookup provider for internal tempo mapping.
   */
  explicit BuildAlgorithm(const IReferenceTables& referenceTables);

  /**
   * @brief Advance macro/micro phase and render one build sample.
   * @param controls Coherent 10-bit control snapshot. Texture selects phrase length; Speed CV is optional clock input.
   * @return 12-bit composite build CV code 0..4095.
   */
  uint16_t step(const ControlFrame& controls);

 private:
  /** @brief Establish phrase origin and latch current Texture-selected phrase length. */
  void start(uint16_t textureControl);
  /** @brief Restart macro/micro phases and latch Texture for the next phrase. */
  void wrapPhrase(uint16_t textureControl);
  /** @brief Re-anchor phrase position to an accepted external quarter boundary. */
  void handleExternalQuarter(uint16_t textureControl);

  const IReferenceTables& referenceTables_; ///< Fixed-point exponential table provider for internal tempo.
  clock::ClockSource clockSource_;           ///< Shared external quarter-note detector and internal fallback.
  uint32_t phrasePhase_;                     ///< Q0.32 normalized macro phrase phase.
  uint32_t microPhase_;                      ///< Q0.32 triangle phase at the current stage rate.
  uint16_t latchedTexture_;                  ///< Phrase-latched saturated Texture code.
  uint8_t phraseLengthBars_;                 ///< Active phrase length: 8, 4, 2 or 1 bar.
  uint8_t externalQuarterIndex_;             ///< Number of accepted external quarters elapsed in the current phrase.
  bool initialized_;                         ///< False until first sample establishes phrase state from controls.
};

}  // namespace fmd

#endif  // FMD_DOMAIN_DUBSTEP_BUILD_ALGORITHM_H
