/**
 * @file GrowlAlgorithm.h
 * Declares the beat-synchronised multi-lobed timbral-motion CV algorithm.
 *
 * @details Growl renders a fixed three-triangle compound gesture. Texture changes
 * are converted to normalized Q0.12 weights only when the saturated control code
 * changes, keeping integer normalization division out of the steady-state hot path.
 * The gesture period is one half note and Speed CV uses the shared external clock.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_DOMAIN_DUBSTEP_GROWL_ALGORITHM_H
#define FMD_DOMAIN_DUBSTEP_GROWL_ALGORITHM_H

#include <stdint.h>

#include "fmd/domain/ClockSource.h"
#include "fmd/domain/Types.h"
#include "fmd/domain/dubstep/DubstepAlgorithmMath.h"
#include "fmd/ports/ReferenceTables.h"

namespace fmd {

/**
 * @brief Deterministic compound triangle gesture intended for timbral modulation destinations.
 * @note The name describes intended musical use; this class outputs CV and does not synthesize audio.
 */
class GrowlAlgorithm {
 public:
  /**
   * @brief Construct a half-note Growl gesture at phase zero and Texture zero weights.
   * @param referenceTables Read-only exponential lookup provider for internal tempo mapping.
   */
  explicit GrowlAlgorithm(const IReferenceTables& referenceTables);

  /**
   * @brief Advance the half-note gesture and render one compound CV sample.
   * @param controls Coherent 10-bit control snapshot. Texture is continuous; Speed CV is optional clock input.
   * @return 12-bit DAC code 0..4095.
   */
  uint16_t step(const ControlFrame& controls);

 private:
  /**
   * @brief Recompute normalized component weights only when saturated Texture changes.
   * @param textureControl Combined Texture knob/CV code; clamped to 0..1023.
   */
  void updateTexture(uint16_t textureControl);

  const IReferenceTables& referenceTables_; ///< Fixed-point exponential table provider for internal tempo.
  clock::ClockSource clockSource_;           ///< Shared external quarter-note detector and internal fallback.
  uint32_t phase_;                           ///< Q0.32 half-note gesture phase.
  uint16_t cachedTexture_;                   ///< Last saturated Texture code used to compute `weights_`.
  growlmath::Weights weights_;               ///< Cached unity-sum Q0.12 fundamental/second/third component weights.
  uint8_t externalQuarterIndex_;             ///< Alternates 0/1 so external quarter edges re-anchor phase to 0 or 1/2.
  bool initialized_;                         ///< False until first sample establishes the deterministic phase origin.
};

}  // namespace fmd

#endif  // FMD_DOMAIN_DUBSTEP_GROWL_ALGORITHM_H
