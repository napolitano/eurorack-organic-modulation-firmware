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

/**
 * @brief Shared-sixteenth-grid 4-against-3/5/7/9 polymetric accent process.
 *
 * @details
 * The primary cycle repeats every four sixteenth-note steps. Texture selects a
 * secondary cycle of 3, 5, 7 or 9 steps. Both cycles share the same step duration;
 * only their meter lengths differ. Coincident starts receive the strongest pulse,
 * while isolated primary and secondary starts receive distinct accent levels.
 * Texture changes are accepted only on a primary-cycle start so meter changes do
 * not tear the current four-step anchor.
 */
class PolymeterAlgorithm {
 public:
  /**
   * @brief Construct Polymeter ready to initialize at a 4-against-3 coincidence.
   * @param referenceTables Long-lived provider for the Electronica tempo mapping.
   */
  explicit PolymeterAlgorithm(const IReferenceTables& referenceTables);

  /**
   * @brief Advance the shared grid and active short-decay accent envelope.
   * @param controls Current 10-bit control frame. Speed+CV set sixteenth-note
   *        duration; Texture+CV request the secondary meter length.
   * @return Current accent envelope as an inclusive 12-bit DAC code.
   */
  uint16_t step(const ControlFrame& controls);

 private:
  /**
   * @brief Start a new short decay at the requested accent amplitude.
   * @param amplitudeDac12 Peak amplitude in DAC codes; helper callers provide 0..4095.
   */
  void launchEnvelope(uint16_t amplitudeDac12);

  /**
   * @brief Establish the first primary/secondary coincidence and latch Texture.
   * @param textureControl Combined Texture knob/CV selecting 3, 5, 7 or 9 steps.
   */
  void initialize(uint16_t textureControl);

  /**
   * @brief Advance both meter countdowns by one shared sixteenth-note step.
   * @param textureControl Current combined Texture; a new secondary length is
   *        adopted only when the primary cycle begins.
   */
  void advanceStep(uint16_t textureControl);

  const IReferenceTables& referenceTables_;  ///< Non-owning tempo/reference-table provider.
  uint32_t sixteenthPhase_;                  ///< Unsigned phase within the shared grid step.
  uint32_t envelopePhase_;                   ///< Phase of the currently active short decay.
  uint16_t envelopeAmplitudeDac12_;          ///< Peak DAC amplitude associated with that decay.
  uint8_t primaryCountdown_;                 ///< Steps remaining before the next 4-step primary start.
  uint8_t secondaryCountdown_;               ///< Steps remaining before the selected odd-meter start.
  uint8_t secondaryLength_;                  ///< Latched secondary meter length: 3, 5, 7 or 9.
  bool envelopeActive_;                      ///< True until the current decay reaches its endpoint.
  bool initialized_;                         ///< True after the initial meter coincidence is emitted.
};

}  // namespace fmd
#endif  // FMD_DOMAIN_ELECTRONICA_POLYMETER_ALGORITHM_H
