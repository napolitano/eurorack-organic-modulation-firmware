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

/**
 * @brief Quarter-note sidechain-style duck/recovery contour.
 *
 * @details
 * Pump is a modulation contour, not an audio compressor. The 32-bit beat phase
 * runs at the Electronica quarter-note rate. Texture selects the recovery end
 * point from one quarter to fifteen sixteenths of the beat. The corresponding
 * reciprocal is cached whenever Texture changes, keeping the steady-state path
 * free of division.
 */
class PumpAlgorithm {
 public:
  /**
   * @brief Construct Pump at the beginning of a beat with minimum recovery time.
   * @param referenceTables Long-lived provider for the Electronica tempo mapping.
   */
  explicit PumpAlgorithm(const IReferenceTables& referenceTables);

  /**
   * @brief Advance the beat phase and evaluate the current duck/recovery value.
   * @param controls Current 10-bit control frame. Speed+CV map to 30..240 BPM;
   *        Texture+CV select the recovery endpoint.
   * @return Unipolar 12-bit DAC code in the inclusive range 0..4095.
   */
  uint16_t step(const ControlFrame& controls);

 private:
  /**
   * @brief Recompute cached recovery data only when the saturated Texture changes.
   * @param textureControl Combined Texture knob/CV value; clamped internally to 0..1023.
   */
  void updateRecovery(uint16_t textureControl);

  const IReferenceTables& referenceTables_;  ///< Non-owning tempo/reference-table provider.
  uint32_t phase_;                           ///< Unsigned 32-bit quarter-note phase accumulator.
  uint16_t cachedTexture_;                   ///< Last clamped Texture value used to build recovery caches.
  uint16_t recoveryEndpointQ0F16_;           ///< Recovery completion point within the beat in Q0.16.
  uint16_t recoveryReciprocalQ28_;           ///< Cached 2^28/recovery-endpoint normalization reciprocal.
};

}  // namespace fmd
#endif  // FMD_DOMAIN_ELECTRONICA_PUMP_ALGORITHM_H
