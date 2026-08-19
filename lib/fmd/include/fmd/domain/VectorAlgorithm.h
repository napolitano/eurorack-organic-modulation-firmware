/**
 * @file VectorAlgorithm.h
 * Declares the two-dimensional toroidal Vector algorithm from the optional Organic bank.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_DOMAIN_VECTOR_ALGORITHM_H
#define FMD_DOMAIN_VECTOR_ALGORITHM_H

#include <stdint.h>

#include "fmd/domain/Types.h"
#include "fmd/ports/ReferenceTables.h"

namespace fmd {

/**
 * @brief Cross-coupled two-dimensional phase flow projected to one CV output.
 *
 * Two phase coordinates move on a torus at related but different base rates.
 * Each axis is perturbed by a bounded bipolar projection of the other axis,
 * forming a simple project-defined vector field. Texture controls cross-coupling
 * strength; Speed controls the common time scale. The output is a continuous
 * scalar projection of both coordinates.
 */
class VectorAlgorithm {
 public:
  /**
   * @brief Construct the vector-flow generator.
   * @param referenceTables Read-only exponential table provider for Speed mapping.
   */
  explicit VectorAlgorithm(const IReferenceTables& referenceTables);

  /**
   * @brief Advance both state axes and render one projected output sample.
   * @param controls Coherent knob/CV snapshot.
   * @return 12-bit DAC code 0..4095.
   */
  uint16_t step(const ControlFrame& controls);

 private:
  const IReferenceTables& referenceTables_; ///< Exponential frequency table provider.
  uint32_t xPhaseAccumulator_;              ///< First toroidal phase coordinate.
  uint32_t yPhaseAccumulator_;              ///< Second toroidal phase coordinate.
};

}  // namespace fmd
#endif  // FMD_DOMAIN_VECTOR_ALGORITHM_H
