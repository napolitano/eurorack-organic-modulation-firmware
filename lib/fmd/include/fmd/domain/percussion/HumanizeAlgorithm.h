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
class HumanizeAlgorithm {
 public:
  HumanizeAlgorithm(const IReferenceTables& referenceTables, uint16_t randomSeed);
  uint16_t step(const ControlFrame& controls);
 private:
  void launchPulse(uint16_t amplitude);
  void prepareUpcomingEvent(uint16_t textureControl);
  void handleNominalBoundary(uint16_t textureControl, uint32_t nextPhaseIncrement);
  void synchronizeExternal(uint16_t textureControl, uint32_t quarterIncrement);
  void recomputeEarlyThreshold();
  const IReferenceTables& referenceTables_;
  uint32_t nominalPhase_;
  uint32_t latchedPhaseIncrement_;
  uint32_t earlyThreshold_;
  uint8_t pulseSamplesRemaining_;
  uint8_t lateDelaySamplesRemaining_;
  int8_t upcomingOffsetSamples_;
  uint16_t upcomingAmplitudeDac12_;
  uint16_t currentPulseAmplitudeDac12_;
  ParallelLfsr randomGenerator_;
  percussionmath::ClockSource clockSource_;
  bool initialized_;
  bool upcomingEarlyFired_;
  bool externalMidpointSeen_;
};
}  // namespace fmd
#endif
