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

/**
 * @brief Two-onset long/short Shuffle pair with pair-boundary Texture latching.
 *
 * @details
 * One complete pair always spans two sixteenth notes, so changing Shuffle never
 * changes average tempo. The first onset is at pair phase zero and the second is
 * positioned between one half (straight) and three quarters (3:1 long/short) of
 * the pair. Texture is sampled only at pair rollover, preventing a live CV change
 * from creating or deleting an onset mid-pair. Each onset launches the same short
 * decay envelope.
 */
class ShuffleAlgorithm {
 public:
  /**
   * @brief Construct Shuffle at the start of a straight pair.
   * @param referenceTables Long-lived provider for the Electronica tempo mapping.
   */
  explicit ShuffleAlgorithm(const IReferenceTables& referenceTables);

  /**
   * @brief Advance pair timing and the currently active onset envelope.
   * @param controls Current 10-bit control frame. Speed+CV set pair tempo;
   *        Texture+CV select the next pair's second-onset ratio.
   * @return Current full-scale decay envelope as a 12-bit DAC code.
   */
  uint16_t step(const ControlFrame& controls);

 private:
  /** @brief Restart the short decay envelope at full scale. */
  void launchEnvelope();

  /**
   * @brief Latch the second-onset position for a complete pair.
   * @param textureControl Combined Texture knob/CV value mapped to 1/2..3/4 of the pair.
   */
  void latchRatio(uint16_t textureControl);

  const IReferenceTables& referenceTables_;  ///< Non-owning tempo/reference-table provider.
  uint32_t pairPhase_;                       ///< Unsigned phase across exactly two sixteenth notes.
  uint32_t secondOnsetThreshold_;            ///< 32-bit pair-phase threshold latched for the second onset.
  uint32_t envelopePhase_;                   ///< Phase of the short decay launched by either onset.
  bool secondOnsetTriggered_;                ///< Prevents duplicate second onsets within the same pair.
  bool envelopeActive_;                      ///< True until the current short decay completes.
  bool initialized_;                         ///< Distinguishes the first scheduler sample from later pairs.
};

}  // namespace fmd
#endif  // FMD_DOMAIN_ELECTRONICA_SHUFFLE_ALGORITHM_H
