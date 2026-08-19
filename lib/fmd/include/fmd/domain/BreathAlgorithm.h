/**
 * @file BreathAlgorithm.h
 * Declares cycle-varied recurrent Breath modulation for the Ambient bank.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_DOMAIN_BREATH_ALGORITHM_H
#define FMD_DOMAIN_BREATH_ALGORITHM_H

#include <stdint.h>

#include "fmd/domain/ParallelLfsr.h"
#include "fmd/domain/Types.h"
#include "fmd/ports/ReferenceTables.h"

namespace fmd {

/** @brief Smooth baseline-to-peak-to-baseline gesture with rollover-only variation. */
class BreathAlgorithm {
 public:
  /** @brief Construct Breath with the documented nominal first cycle. */
  BreathAlgorithm(const IReferenceTables& referenceTables, uint16_t randomSeed);

  /** @brief Advance the current Breath cycle by one sample. */
  uint16_t step(const ControlFrame& controls);

 private:
  void latchCycle(uint16_t textureControl);

  const IReferenceTables& referenceTables_;
  uint32_t phase_;
  uint16_t durationQ10_;
  uint16_t rateScaleQ10_;
  uint16_t amplitudeDac12_;
  uint16_t skewQ12_;
  uint16_t attackReciprocalQ12_;
  uint16_t releaseReciprocalQ12_;
  ParallelLfsr randomGenerator_;
};

}  // namespace fmd
#endif  // FMD_DOMAIN_BREATH_ALGORITHM_H
