/**
 * @file CurrentAlgorithm.h
 * Declares deterministic long-form Current modulation for the Ambient bank.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_DOMAIN_CURRENT_ALGORITHM_H
#define FMD_DOMAIN_CURRENT_ALGORITHM_H

#include <stdint.h>

#include "fmd/domain/Types.h"
#include "fmd/ports/ReferenceTables.h"

namespace fmd {

/**
 * @brief Three-rate deterministic long-form Ambient modulation.
 *
 * @details
 * Current advances three 32-bit phase accumulators at the base Ambient rate,
 * approximately sqrt(2) times that rate, and approximately phi times that rate.
 * Each phase is converted to a slope-softened bipolar triangle and mixed with
 * Texture-controlled constant-sum weights. The irrational ratios are represented
 * by fixed rational approximations, so the result is quasiperiodic in character
 * rather than mathematically aperiodic.
 *
 * Speed and Speed CV use the normal Drift frequency mapping divided by sixteen.
 * Texture and Texture CV alter only the relative current weights; they do not
 * change the three phase relationships. Output is always a 12-bit DAC code.
 */
class CurrentAlgorithm {
 public:
  /**
   * @brief Construct Current with deterministic one-third-cycle phase offsets.
   * @param referenceTables Long-lived lookup-table provider used by the shared
   *        Drift frequency mapping. The referenced object must outlive this algorithm.
   */
  explicit CurrentAlgorithm(const IReferenceTables& referenceTables);

  /**
   * @brief Advance all three currents by one scheduler sample.
   * @param controls Current 10-bit knob/CV frame. Speed controls the common
   *        Ambient time scale; Texture controls the constant-sum mix.
   * @return Unipolar 12-bit DAC code in the inclusive range 0..4095.
   */
  uint16_t step(const ControlFrame& controls);

 private:
  const IReferenceTables& referenceTables_;  ///< Non-owning frequency/reference-table provider.
  uint32_t phase0_;  ///< Fundamental 32-bit phase accumulator.
  uint32_t phase1_;  ///< Approximate sqrt(2)-rate phase accumulator.
  uint32_t phase2_;  ///< Approximate golden-ratio-rate phase accumulator.
};

}  // namespace fmd
#endif  // FMD_DOMAIN_CURRENT_ALGORITHM_H
