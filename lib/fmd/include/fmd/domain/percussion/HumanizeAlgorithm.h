/**
 * @file HumanizeAlgorithm.h
 * Declares the bounded timing/amplitude humanizer.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_DOMAIN_PERCUSSION_HUMANIZE_ALGORITHM_H
#define FMD_DOMAIN_PERCUSSION_HUMANIZE_ALGORITHM_H

#include <stdint.h>

#include "fmd/domain/ParallelLfsr.h"
#include "fmd/domain/Types.h"
#include "fmd/domain/percussion/PercussionAlgorithmMath.h"
#include "fmd/ports/ReferenceTables.h"

namespace fmd {

/**
 * @brief Eighth-note pulse train with bounded non-accumulating timing and amplitude variation.
 *
 * @details
 * Humanize keeps an ideal eighth-note phase as the timing reference and applies
 * each random offset relative to that nominal event, preventing cumulative tempo
 * drift. Texture maps to at most +/-30 scheduler samples (about +/-12 ms at
 * 2.5 kHz) and a bounded DAC-amplitude radius around 3840. Negative offsets are
 * predicted from the current phase increment; positive offsets are delayed only
 * after the nominal boundary. External acquisition emits the new origin on-grid
 * because no earlier period exists from which a negative offset could be predicted.
 */
class HumanizeAlgorithm {
 public:
  /**
   * @brief Construct Humanize before the first nominal eighth-note event.
   * @param referenceTables Long-lived provider for internal tempo mapping.
   * @param randomSeed Deterministic seed used for timing and amplitude deviations.
   */
  HumanizeAlgorithm(const IReferenceTables& referenceTables, uint16_t randomSeed);

  /**
   * @brief Advance nominal timing, jitter scheduling and the active pulse.
   * @param controls Speed knob sets internal tempo; Speed CV is optional 0..5 V
   *        quarter clock; Texture knob/CV set timing and amplitude deviation radii.
   * @return Current humanized pulse amplitude in 12-bit DAC codes, or zero between pulses.
   */
  uint16_t step(const ControlFrame& controls);

 private:
  /** @brief Start a fixed-length pulse at a defensively saturated 12-bit amplitude. */
  void launchPulse(uint16_t amplitude);
  /** @brief Draw and cache the offset/amplitude of the next nominal event. */
  void prepareUpcomingEvent(uint16_t textureControl);
  /** @brief Resolve the event due at a nominal boundary and adopt the next timing increment. */
  void handleNominalBoundary(uint16_t textureControl, uint32_t nextPhaseIncrement);
  /** @brief Establish deterministic external timing at the acquisition edge. */
  void synchronizeExternal(uint16_t textureControl, uint32_t quarterIncrement);
  /** @brief Convert a negative sample offset into the phase threshold that fires it early. */
  void recomputeEarlyThreshold();

  const IReferenceTables& referenceTables_;  ///< Non-owning internal-tempo table provider.
  uint32_t nominalPhase_;                    ///< Ideal eighth-note phase; never displaced by jitter.
  uint32_t latchedPhaseIncrement_;           ///< Eighth-note phase increment used to predict timing offsets.
  uint32_t earlyThreshold_;                  ///< Phase threshold for the next negative-jitter event.
  uint8_t pulseSamplesRemaining_;            ///< Samples remaining in the active fixed-length pulse.
  uint8_t lateDelaySamplesRemaining_;        ///< Complete samples still owed after a positive-offset boundary.
  int8_t upcomingOffsetSamples_;             ///< Next event's signed timing offset in scheduler samples.
  uint16_t upcomingAmplitudeDac12_;           ///< Next event's latched amplitude before it fires.
  uint16_t currentPulseAmplitudeDac12_;       ///< Amplitude held for the currently active pulse.
  ParallelLfsr randomGenerator_;             ///< Deterministic source for timing/amplitude deviations.
  percussionmath::ClockSource clockSource_;  ///< Shared external quarter-clock detector and fallback.
  bool initialized_;                         ///< True after the first nominal event has been prepared.
  bool upcomingEarlyFired_;                  ///< Prevents a negative-offset event from firing twice.
  bool externalMidpointSeen_;                ///< Tracks the interpolated eighth between external quarter edges.
};

}  // namespace fmd
#endif  // FMD_DOMAIN_PERCUSSION_HUMANIZE_ALGORITHM_H
