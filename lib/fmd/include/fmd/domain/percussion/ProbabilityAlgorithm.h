/**
 * @file ProbabilityAlgorithm.h
 * Declares the phrase-aware metric probability pulse generator.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_DOMAIN_PERCUSSION_PROBABILITY_ALGORITHM_H
#define FMD_DOMAIN_PERCUSSION_PROBABILITY_ALGORITHM_H

#include <stdint.h>

#include "fmd/domain/ParallelLfsr.h"
#include "fmd/domain/Types.h"
#include "fmd/domain/percussion/PercussionAlgorithmMath.h"
#include "fmd/ports/ReferenceTables.h"

namespace fmd {

/**
 * @brief Metric 16-step probability hierarchy with phrase-aware fill boosts.
 *
 * @details
 * Primary steps 0/4/8/12 are deterministic. Even offbeats are Secondary and use
 * a linear Texture probability; odd steps are Ghost positions and use a quadratic
 * probability capped at one half before fill boosts. Texture is latched per bar.
 * Fill bars increase only optional probabilities, preserving the metric skeleton.
 * Clock semantics and pulse length match Euclid.
 */
class ProbabilityAlgorithm {
 public:
  /**
   * @brief Construct Probability with deterministic PRNG state and no emitted step.
   * @param referenceTables Long-lived provider for internal tempo mapping.
   * @param randomSeed Deterministic seed used for optional-hit decisions.
   */
  ProbabilityAlgorithm(const IReferenceTables& referenceTables, uint16_t randomSeed);

  /**
   * @brief Advance grid, phrase state, optional-hit decision and output pulse.
   * @param controls Speed knob sets internal tempo; Speed CV is optional 0..5 V
   *        quarter clock; Texture knob/CV control optional-hit and fill probabilities.
   * @return 4095 while the fixed-length pulse is active, otherwise zero.
   */
  uint16_t step(const ControlFrame& controls);

 private:
  /** @brief Start a fixed-length full-scale pulse. */
  void launchPulse();
  /** @brief Classify and evaluate the current step using the bar-latched Texture. */
  void emitCurrentStep();
  /** @brief Reset phrase/grid state at deterministic external-clock acquisition. */
  void synchronizeExternal(uint16_t textureControl);
  /** @brief Re-anchor to the next quarter start on an accepted external edge. */
  void handleExternalQuarterBoundary(uint16_t textureControl);
  /** @brief Advance one sixteenth boundary, including bar/phrase rollover. */
  void handleBoundary(uint16_t textureControl);

  const IReferenceTables& referenceTables_;  ///< Non-owning internal-tempo table provider.
  uint32_t phase_;                           ///< Unsigned phase within the current sixteenth subdivision.
  uint8_t stepIndex_;                        ///< Current 16-step bar position, always 0..15.
  uint8_t pulseSamplesRemaining_;            ///< Scheduler samples remaining in an active full-scale pulse.
  uint16_t latchedTexture_;                  ///< Bar-latched probability/fill macro value.
  percussionmath::PhraseState phraseState_;  ///< Multi-bar phrase length and fill-bar state.
  ParallelLfsr randomGenerator_;             ///< Deterministic source for optional-hit decisions.
  percussionmath::ClockSource clockSource_;  ///< Shared external-clock detector and internal fallback.
  uint8_t externalSubdivisions_;             ///< Interpolated sixteenths already emitted after the last edge.
  bool initialized_;                         ///< True after the first phrase/grid origin has been emitted.
};

}  // namespace fmd
#endif  // FMD_DOMAIN_PERCUSSION_PROBABILITY_ALGORITHM_H
