/**
 * @file DriftEngine.h
 * Declares compile-time algorithm-bank selection and the portable Drift processing core.
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

#include "fmd/config/AlgorithmBankConfig.h"
#include "fmd/domain/Types.h"
#include "fmd/ports/ReferenceTables.h"

#if FMD_ALGORITHM_BANK == FMD_BANK_CLASSIC
#include "fmd/domain/BezierAlgorithm.h"
#include "fmd/domain/BrownianAlgorithm.h"
#include "fmd/domain/LfoAlgorithm.h"
#include "fmd/domain/PerlinAlgorithm.h"
#elif FMD_ALGORITHM_BANK == FMD_BANK_ORGANIC
#include "fmd/domain/AttractorAlgorithm.h"
#include "fmd/domain/FractalAlgorithm.h"
#include "fmd/domain/RainAlgorithm.h"
#include "fmd/domain/VectorAlgorithm.h"
#endif

namespace fmd {

/**
 * @brief Own all state for the compile-time-selected algorithm bank and dispatch samples.
 *
 * A firmware image contains one four-algorithm bank only. The rear DIP switches
 * select one of those four algorithms at startup. Conditional member layout is
 * deliberate: inactive-bank algorithm objects are not retained in SRAM merely
 * to support a compile-time option that cannot be reached in the running image.
 */
class DriftEngine {
 public:
  /**
   * @brief Construct the selected-bank algorithms and choose one active mode.
   * @param algorithm Algorithm fixed for this engine lifetime; normally returned
   *                  by algorithmFromConfig().
   * @param randomSeed Deterministic 16-bit seed shared by stochastic algorithms.
   * @param referenceTables Read-only exponential, inverse-CDF and gamma lookup provider.
   */
  DriftEngine(Algorithm algorithm,
              uint16_t randomSeed,
              const IReferenceTables& referenceTables);

  /**
   * @brief Advance the selected algorithm by one sample.
   * @param controls Coherent knob/CV snapshot.
   * @return 12-bit DAC code 0..4095, or zero for an algorithm absent from this bank.
   */
  uint16_t step(const ControlFrame& controls);

  /** @return Algorithm selected when this engine was constructed. */
  Algorithm algorithm() const { return selectedAlgorithm_; }

 private:
  Algorithm selectedAlgorithm_;  ///< Active algorithm for this power cycle.

#if FMD_ALGORITHM_BANK == FMD_BANK_CLASSIC
  PerlinAlgorithm perlinAlgorithm_;       ///< Classic-bank Perlin state.
  BrownianAlgorithm brownianAlgorithm_;   ///< Classic-bank Brownian state.
  BezierAlgorithm bezierAlgorithm_;       ///< Classic-bank Bézier state.
  LfoAlgorithm lfoAlgorithm_;             ///< Classic-bank LFO state.
#elif FMD_ALGORITHM_BANK == FMD_BANK_ORGANIC
  FractalAlgorithm fractalAlgorithm_;     ///< Organic-bank multi-scale noise state.
  VectorAlgorithm vectorAlgorithm_;       ///< Organic-bank 2D flow state.
  RainAlgorithm rainAlgorithm_;           ///< Organic-bank shot-noise state.
  AttractorAlgorithm attractorAlgorithm_; ///< Organic-bank Hénon traversal state.
#endif
};

/**
 * @brief Decode the two active-low rear configuration inputs for the compiled bank.
 * @param configInput1Low true when firmware CONFIG 1 is pulled low.
 * @param configInput2Low true when firmware CONFIG 2 is pulled low.
 * @return Algorithm occupying that DIP slot in the selected AlgorithmBank.
 *
 * The electrical four-way mapping remains unchanged. Only the semantic contents
 * of each slot differ between Classic and Organic firmware images.
 */
Algorithm algorithmFromConfig(bool configInput1Low, bool configInput2Low);

/**
 * @brief Return the algorithm occupying a logical DIP slot in the compiled bank.
 * @param slotIndex Slot 0..3 in OFF/OFF, OFF/ON, ON/OFF, ON/ON firmware-pin order.
 * @return Selected-bank algorithm, or the bank's first slot for an invalid index.
 */
Algorithm algorithmForBankSlot(uint8_t slotIndex);

}  // namespace fmd
#endif  // FMD_DOMAIN_DRIFT_ENGINE_H
