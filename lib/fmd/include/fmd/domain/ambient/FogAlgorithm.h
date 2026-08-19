/**
 * @file FogAlgorithm.h
 * Declares the bounded smooth cloudlet process used by the Ambient bank.
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

#include "fmd/domain/ambient/AmbientAlgorithmMath.h"
#include "fmd/domain/ParallelLfsr.h"
#include "fmd/domain/Types.h"
#include "fmd/ports/ReferenceTables.h"

namespace fmd {

/** @brief Four-voice bipolar filtered-event cloud process. */
class FogAlgorithm {
 public:
  /** @brief Construct an initially empty Fog cloud. */
  FogAlgorithm(const IReferenceTables& referenceTables, uint16_t randomSeed);

  /** @brief Advance cloud voices and stochastic arrivals by one sample. */
  uint16_t step(const ControlFrame& controls);

 private:
  const IReferenceTables& referenceTables_;
  fogmath::Voice voices_[fogmath::kVoiceCount];
  ParallelLfsr randomGenerator_;
};

}  // namespace fmd
#endif  // FMD_DOMAIN_FOG_ALGORITHM_H
