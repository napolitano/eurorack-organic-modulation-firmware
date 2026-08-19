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
class RepeatAlgorithm {
 public:
  RepeatAlgorithm(const IReferenceTables& referenceTables, uint16_t randomSeed);
  uint16_t step(const ControlFrame& controls);
 private:
  void launchPulse();
  void configureCurrentQuarter();
  void synchronizeExternal(uint16_t textureControl);
  void beginQuarter(uint16_t textureControl);
  const IReferenceTables& referenceTables_;
  uint32_t quarterPhase_;
  uint32_t latchedPhaseIncrement_;
  uint8_t quarterIndex_;
  uint8_t pulseSamplesRemaining_;
  uint8_t clusterPulseCount_;
  uint8_t nextSubEventIndex_;
  uint16_t latchedTexture_;
  percussionmath::PhraseState phraseState_;
  ParallelLfsr randomGenerator_;
  percussionmath::ClockSource clockSource_;
  bool initialized_;
};
}  // namespace fmd
#endif
