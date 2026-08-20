/**
 * @file FogAlgorithm.h
 * Declares bounded cloudlet-superposition Fog modulation for the Ambient bank.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_DOMAIN_FOG_ALGORITHM_H
#define FMD_DOMAIN_FOG_ALGORITHM_H

#include <stdint.h>

#include "fmd/domain/ParallelLfsr.h"
#include "fmd/domain/Types.h"
#include "fmd/domain/ambient/AmbientAlgorithmMath.h"
#include "fmd/ports/ReferenceTables.h"

namespace fmd {

/**
 * @brief Four-voice stochastic superposition of smooth bipolar cloudlets.
 *
 * @details
 * Fog maintains exactly four fixed-memory voices. Active voices traverse a
 * compact quartic kernel and contribute signed DAC-domain amplitude around the
 * midpoint. Texture controls a target mean occupancy; each scheduler sample may
 * start one inactive voice according to a 32-bit Bernoulli cutoff derived from
 * the current Ambient phase rate. No dynamic allocation is used.
 */
class FogAlgorithm {
 public:
  /**
   * @brief Construct Fog with all cloudlet voices inactive.
   * @param referenceTables Long-lived provider used by the shared frequency map.
   * @param randomSeed Deterministic seed for event timing and bipolar amplitudes.
   */
  FogAlgorithm(const IReferenceTables& referenceTables, uint16_t randomSeed);

  /**
   * @brief Advance active cloudlets and possibly start one new voice.
   * @param controls Current 10-bit control frame. Speed controls cloudlet
   *        duration; Texture controls target voice occupancy/density.
   * @return Saturated 12-bit DAC code centred around midpoint in the absence of voices.
   */
  uint16_t step(const ControlFrame& controls);

 private:
  const IReferenceTables& referenceTables_;  ///< Non-owning frequency/reference-table provider.
  fogmath::Voice voices_[fogmath::kVoiceCount];  ///< Fixed pool of four smooth cloudlet voices.
  ParallelLfsr randomGenerator_;             ///< Deterministic source for events and amplitudes.
};

}  // namespace fmd
#endif  // FMD_DOMAIN_FOG_ALGORITHM_H
