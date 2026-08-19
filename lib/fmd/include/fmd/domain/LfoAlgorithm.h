/**
 * @file LfoAlgorithm.h
 * Declares the portable corrected skew-LFO Drift algorithm.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_DOMAIN_LFO_ALGORITHM_H
#define FMD_DOMAIN_LFO_ALGORITHM_H

#include <stdint.h>

#include "fmd/domain/Types.h"
#include "fmd/ports/ReferenceTables.h"

namespace fmd {

/**
 * @brief Deterministic skewable triangle LFO with continuous live shape changes.
 *
 * Texture moves the triangle apex across the cycle, reaching falling and rising
 * sawtooth endpoints. When Texture changes during a cycle, phase is remapped to
 * minimise the instantaneous output discontinuity.
 */
class LfoAlgorithm {
 public:
  /**
   * @brief Construct an LFO using the shared exponential frequency table.
   * @param referenceTables Read-only lookup provider used for Speed/CV mapping.
   */
  explicit LfoAlgorithm(const IReferenceTables& referenceTables);

  /**
   * @brief Advance phase and evaluate one 12-bit waveform sample.
   * @param controls Coherent knob/CV snapshot.
   * @return 12-bit DAC code 0..4095.
   */
  uint16_t step(const ControlFrame& controls);

 private:
  const IReferenceTables& referenceTables_; ///< Exponential table provider.
  uint32_t phaseAccumulator_;               ///< Full 32-bit phase accumulator.
  uint16_t apexPhaseQ0F16_;                 ///< Texture-selected apex position in Q0.16.
  bool textureInitialised_;                 ///< Guards first-sample apex initialisation.
};

}  // namespace fmd
#endif  // FMD_DOMAIN_LFO_ALGORITHM_H
