/**
 * @file WobbleAlgorithm.h
 * Declares the tempo-synchronised deterministic rate-phrase modulation algorithm.
 *
 * @details Wobble separates an eighth-note phrase grid from a continuous triangle
 * carrier. Texture is latched once per bar and selects a fixed rate vocabulary;
 * rate changes alter only carrier increment, not carrier phase. Speed CV is
 * interpreted through the shared 0..5 V quarter-note ClockSource.
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

/**
 * @brief One-bar deterministic Wobble rate phrase with continuous triangle carrier phase.
 *
 * The eight phrase cells are eighth notes in 4/4. Internal timing uses the
 * 70..280 BPM Speed mapping; a valid external clock replaces that timing after
 * two edges. External acquisition resets both phrase and carrier origin so a
 * re-locked build is deterministic.
 */
class WobbleAlgorithm {
 public:
  /**
   * @brief Construct Wobble at phrase cell zero and carrier phase zero.
   * @param referenceTables Read-only exponential lookup provider for internal tempo mapping.
   */
  explicit WobbleAlgorithm(const IReferenceTables& referenceTables);

  /**
   * @brief Advance phrase timing, continuous carrier phase and render one sample.
   * @param controls Coherent 10-bit control snapshot. Speed CV is interpreted as optional clock input.
   * @return 12-bit unipolar triangle modulation code 0..4095.
   */
  uint16_t step(const ControlFrame& controls);

 private:
  /** @brief Reset phrase/carrier origin and latch the current Texture region. */
  void start(uint16_t textureControl);
  /** @brief Advance to the next eighth-note cell and latch Texture on bar wrap. */
  void advanceCell(uint16_t textureControl);
  /** @brief Re-anchor the eighth-note grid on an accepted external quarter boundary. */
  void handleExternalQuarter(uint16_t textureControl);

  const IReferenceTables& referenceTables_; ///< Fixed-point exponential table provider for internal tempo.
  clock::ClockSource clockSource_;           ///< Shared hysteretic external-quarter detector and fallback source.
  uint32_t gridPhase_;                       ///< Q0.32 eighth-note cell phase used between cell boundaries.
  uint32_t carrierPhase_;                    ///< Continuous Q0.32 triangle-carrier phase; not reset on rate changes.
  uint8_t cellIndex_;                        ///< Current phrase cell 0..7.
  uint8_t latchedTextureRegion_;             ///< Bar-latched rate-vocabulary selector 0..3.
  uint8_t externalQuarterIndex_;             ///< External quarter position 0..3 used to re-anchor even-numbered cells.
  bool externalHalfAdvanced_;                ///< Prevents a second eighth-cell transition inside one external quarter.
  bool initialized_;                         ///< False until first step establishes deterministic state from controls.
};

}  // namespace fmd

#endif  // FMD_DOMAIN_DUBSTEP_WOBBLE_ALGORITHM_H
