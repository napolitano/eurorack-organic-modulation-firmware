/**
 * @file RepeatAlgorithm.h
 * Declares quarter-note ratchet/repeat generation with phrase fills.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_DOMAIN_PERCUSSION_REPEAT_ALGORITHM_H
#define FMD_DOMAIN_PERCUSSION_REPEAT_ALGORITHM_H

#include <stdint.h>

#include "fmd/domain/ParallelLfsr.h"
#include "fmd/domain/Types.h"
#include "fmd/domain/percussion/PercussionAlgorithmMath.h"
#include "fmd/ports/ReferenceTables.h"

namespace fmd {

/**
 * @brief Quarter-anchored ratchets with stochastic repeats and deterministic fill minima.
 *
 * @details
 * Every quarter note always emits an anchor pulse. Texture controls the probability
 * and size of an additional 2..4-pulse cluster. On the final bar of the current
 * phrase, deterministic minimum repeat counts escalate toward the phrase end. The
 * complete cluster is distributed through the first half of the quarter. Texture
 * is latched at bar boundaries, while the quarter increment is latched so all
 * subevents within that quarter share one timing reference.
 */
class RepeatAlgorithm {
 public:
  /**
   * @brief Construct Repeat with deterministic PRNG state before the first quarter.
   * @param referenceTables Long-lived provider for internal tempo mapping.
   * @param randomSeed Deterministic seed used for repeat/no-repeat decisions.
   */
  RepeatAlgorithm(const IReferenceTables& referenceTables, uint16_t randomSeed);

  /**
   * @brief Advance the active quarter, schedule ratchet subevents and emit pulses.
   * @param controls Speed knob sets internal tempo; Speed CV is optional 0..5 V
   *        quarter clock; Texture knob/CV control repeat depth and phrase fills.
   * @return 4095 while a fixed-length pulse is active, otherwise zero.
   */
  uint16_t step(const ControlFrame& controls);

 private:
  /** @brief Start a fixed-length full-scale pulse. */
  void launchPulse();
  /** @brief Draw the current quarter's cluster size and apply any phrase-fill minimum. */
  void configureCurrentQuarter();
  /** @brief Reset timing and phrase state when external clock ownership is acquired. */
  void synchronizeExternal(uint16_t textureControl);
  /** @brief Begin the next quarter, including bar/phrase rollover and Texture latching. */
  void beginQuarter(uint16_t textureControl);

  const IReferenceTables& referenceTables_;  ///< Non-owning internal-tempo table provider.
  uint32_t quarterPhase_;                    ///< Unsigned phase within the current quarter note.
  uint32_t latchedPhaseIncrement_;           ///< Quarter increment held stable for the current cluster.
  uint8_t quarterIndex_;                     ///< Quarter position within the bar, always 0..3.
  uint8_t pulseSamplesRemaining_;            ///< Scheduler samples remaining in the active pulse.
  uint8_t clusterPulseCount_;                ///< Total pulses scheduled for the current quarter, 1..4.
  uint8_t nextSubEventIndex_;                ///< Next ratchet subevent index after the anchor pulse.
  uint16_t latchedTexture_;                  ///< Bar-latched repeat/fill macro value.
  percussionmath::PhraseState phraseState_;  ///< Multi-bar phrase length and final-fill-bar state.
  ParallelLfsr randomGenerator_;             ///< Deterministic source for stochastic repeat activation.
  percussionmath::ClockSource clockSource_;  ///< Shared external-clock detector and internal fallback.
  bool initialized_;                         ///< True after the first quarter has been configured.
};

}  // namespace fmd
#endif  // FMD_DOMAIN_PERCUSSION_REPEAT_ALGORITHM_H
