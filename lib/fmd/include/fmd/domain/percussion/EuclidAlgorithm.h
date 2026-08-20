/**
 * @file EuclidAlgorithm.h
 * Declares the phrase-aware Euclidean pulse generator.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_DOMAIN_PERCUSSION_EUCLID_ALGORITHM_H
#define FMD_DOMAIN_PERCUSSION_EUCLID_ALGORITHM_H

#include <stdint.h>

#include "fmd/domain/Types.h"
#include "fmd/domain/percussion/PercussionAlgorithmMath.h"
#include "fmd/ports/ReferenceTables.h"

namespace fmd {

/**
 * @brief Sixteen-step Euclidean pulse pattern with phrase-end tail fills.
 *
 * @details
 * Texture is latched at bar boundaries and selects both hit count E(k,16) and
 * phrase/fill intensity. Canonical masks are lookup-based rather than generated
 * in the hot path. In internal mode Speed sets a 30..240 BPM quarter-note clock.
 * In external mode Speed CV is interpreted by ClockSource as a 0..5 V quarter-
 * note clock; accepted external quarter edges re-anchor steps 0/4/8/12 exactly.
 * Output pulses last kPulseSamples scheduler samples at full DAC scale.
 */
class EuclidAlgorithm {
 public:
  /**
   * @brief Construct Euclid before the first step has been emitted.
   * @param referenceTables Long-lived provider for internal tempo mapping.
   */
  explicit EuclidAlgorithm(const IReferenceTables& referenceTables);

  /**
   * @brief Advance clocking, phrase state and the pulse gate by one sample.
   * @param controls Speed knob sets internal tempo; Speed CV is the optional
   *        external clock; Texture knob/CV select the next bar's pattern/fill macro.
   * @return 4095 while a pulse is active, otherwise zero.
   */
  uint16_t step(const ControlFrame& controls);

 private:
  /** @brief Start a fixed-length full-scale pulse. */
  void launchPulse();
  /** @brief Evaluate the current 16-step position against the normal/fill mask. */
  void emitCurrentStep();
  /** @brief Reset phrase/grid state when the second qualifying external edge acquires lock. */
  void synchronizeExternal(uint16_t textureControl);
  /** @brief Re-anchor the 16-step grid on an accepted external quarter boundary. */
  void handleExternalQuarterBoundary(uint16_t textureControl);
  /** @brief Advance one internal or interpolated sixteenth boundary and latch bar Texture as needed. */
  void handleBoundary(uint16_t textureControl);

  const IReferenceTables& referenceTables_;  ///< Non-owning internal-tempo table provider.
  uint32_t phase_;                           ///< Unsigned phase of the current sixteenth subdivision.
  uint8_t stepIndex_;                        ///< Current bar step, always 0..15.
  uint8_t pulseSamplesRemaining_;            ///< Scheduler samples remaining in the active 10 ms pulse.
  uint16_t latchedTexture_;                  ///< Bar-latched combined Texture controlling masks/fills.
  percussionmath::PhraseState phraseState_;  ///< Multi-bar 4/8/12/16 phrase and fill-bar state.
  percussionmath::ClockSource clockSource_;  ///< Shared hysteretic external-clock detector/fallback.
  uint8_t externalSubdivisions_;             ///< Interpolated sixteenths already emitted since external edge.
  bool initialized_;                         ///< True after the first local or external phrase origin is emitted.
};

}  // namespace fmd
#endif  // FMD_DOMAIN_PERCUSSION_EUCLID_ALGORITHM_H
