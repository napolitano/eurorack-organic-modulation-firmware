/**
 * @file AttractorAlgorithm.h
 * Declares the Hénon-map Attractor algorithm from the optional Organic bank.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_DOMAIN_ATTRACTOR_ALGORITHM_H
#define FMD_DOMAIN_ATTRACTOR_ALGORITHM_H

#include <stdint.h>

#include "fmd/domain/OrganicAlgorithmMath.h"
#include "fmd/domain/Types.h"
#include "fmd/ports/ReferenceTables.h"

namespace fmd {

/**
 * @brief Smooth CV traversal of successive points from the two-dimensional Hénon map.
 *
 * Each Speed cycle advances the map by one point. The output linearly travels
 * from the previous x coordinate to the next, avoiding sample-and-hold jumps.
 * Texture varies Hénon's parameter a from 1.20 to 1.40 while b remains 0.30;
 * this intentionally traverses both chaotic regions and periodic windows rather
 * than pretending that the parameter is a monotonic "amount of chaos" control.
 */
class AttractorAlgorithm {
 public:
  /**
   * @brief Construct the attractor traversal at the canonical origin.
   * @param referenceTables Read-only exponential table provider for Speed mapping.
   */
  explicit AttractorAlgorithm(const IReferenceTables& referenceTables);

  /**
   * @brief Advance interpolation phase and, on wrap, iterate the Hénon map once.
   * @param controls Coherent knob/CV snapshot.
   * @return 12-bit DAC code 0..4095.
   */
  uint16_t step(const ControlFrame& controls);

 private:
  const IReferenceTables& referenceTables_; ///< Exponential frequency table provider.
  uint32_t phaseAccumulator_;               ///< Interpolation phase between map points.
  attractormath::HenonState segmentStart_;  ///< Previous Hénon point.
  attractormath::HenonState segmentEnd_;    ///< Next Hénon point.
  bool segmentInitialised_;                 ///< Defers first point generation until Texture is known.
};

}  // namespace fmd
#endif  // FMD_DOMAIN_ATTRACTOR_ALGORITHM_H
