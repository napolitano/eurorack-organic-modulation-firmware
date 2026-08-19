/**
 * @file DriftEngine.h
 * Declares algorithm selection and the portable Drift processing core.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_DOMAIN_DRIFT_ENGINE_H
#define FMD_DOMAIN_DRIFT_ENGINE_H

#include <stdint.h>

#include "fmd/domain/BezierAlgorithm.h"
#include "fmd/domain/BrownianAlgorithm.h"
#include "fmd/domain/LfoAlgorithm.h"
#include "fmd/domain/PerlinAlgorithm.h"
#include "fmd/domain/Types.h"

namespace fmd {

/**
 * @brief Own all algorithm state and dispatch each sample to the selected mode.
 *
 * All four algorithm objects are constructed once to avoid dynamic allocation in
 * the domain layer. The rear-switch selection remains fixed for the lifetime of
 * the engine, matching the hardware's power-up-only configuration behaviour.
 */
class DriftEngine {
 public:
  /**
   * @brief Construct all algorithms and select the active one.
   * @param algorithm Algorithm fixed for this engine lifetime.
   * @param randomSeed Deterministic 16-bit seed shared as the initial stochastic seed.
   * @param referenceTables Read-only exponential, inverse-CDF and gamma lookup provider.
   */
  DriftEngine(Algorithm algorithm,
              uint16_t randomSeed,
              const IReferenceTables& referenceTables);

  /**
   * @brief Advance the selected algorithm by one sample.
   * @param controls Coherent knob/CV snapshot.
   * @return 12-bit DAC code 0..4095.
   */
  uint16_t step(const ControlFrame& controls);

  /** @return Algorithm selected when this engine was constructed. */
  Algorithm algorithm() const { return selectedAlgorithm_; }

 private:
  Algorithm selectedAlgorithm_;     ///< Active algorithm for this power cycle.
  PerlinAlgorithm perlinAlgorithm_; ///< Perlin state, retained even when inactive.
  BrownianAlgorithm brownianAlgorithm_; ///< Brownian state, retained even when inactive.
  BezierAlgorithm bezierAlgorithm_; ///< Bézier state, retained even when inactive.
  LfoAlgorithm lfoAlgorithm_;       ///< LFO state, retained even when inactive.
};

/**
 * @brief Decode the two active-low rear configuration inputs.
 * @param configInput1Low true when firmware CONFIG 1 is pulled low.
 * @param configInput2Low true when firmware CONFIG 2 is pulled low.
 * @return Selected algorithm; both inputs high/open select Perlin by default.
 */
Algorithm algorithmFromConfig(bool configInput1Low, bool configInput2Low);

}  // namespace fmd
#endif  // FMD_DOMAIN_DRIFT_ENGINE_H
