/**
 * @file ChopAlgorithm.h
 * Declares the deterministic 16-step sparse/syncopated articulation algorithm.
 *
 * @details Chop stores a bar-latched 16-bit onset mask and derives four sixteenth
 * subdivisions from each quarter note. Active steps output a half-step hold
 * followed by a linear decay; inactive steps output exactly zero. Texture changes
 * are deferred until bar wrap to avoid tearing a phrase in progress.
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
  /**
   * @brief Construct Chop at bar step zero with only the two structural anchors active.
   * @param referenceTables Read-only exponential lookup provider for internal tempo mapping.
   */
  explicit ChopAlgorithm(const IReferenceTables& referenceTables);

  /**
   * @brief Advance sixteenth timing and render the current articulation state.
   * @param controls Coherent 10-bit control snapshot. Speed CV is optional quarter-note clock input.
   * @return 12-bit articulation code: zero on inactive steps, otherwise the released hold/decay contour.
   */
  uint16_t step(const ControlFrame& controls);

 private:
  /** @brief Reset bar origin and construct a mask from the current Texture code. */
  void start(uint16_t textureControl);
  /** @brief Advance one wrapped 16-step position and rebuild the mask only on bar wrap. */
  void advanceStep(uint16_t textureControl);
  /** @brief Snap to the corresponding quarter boundary after an accepted external edge. */
  void handleExternalQuarter(uint16_t textureControl);

  const IReferenceTables& referenceTables_; ///< Fixed-point exponential table provider for internal tempo.
  clock::ClockSource clockSource_;           ///< Shared external quarter-note detector and internal fallback.
  uint32_t stepPhase_;                       ///< Q0.32 phase within the current sixteenth step.
  uint8_t stepIndex_;                        ///< Current bar position 0..15.
  uint16_t latchedMask_;                     ///< Bar-latched 16-bit onset mask derived from Texture.
  uint8_t externalQuarterIndex_;             ///< External quarter position 0..3 used to re-anchor steps 0/4/8/12.
  uint8_t externalSubdivisions_;             ///< Number of internally generated sixteenths since the last external edge.
  bool initialized_;                         ///< False until the first sample latches Texture and establishes bar origin.
};

}  // namespace fmd

#endif  // FMD_DOMAIN_DUBSTEP_CHOP_ALGORITHM_H
