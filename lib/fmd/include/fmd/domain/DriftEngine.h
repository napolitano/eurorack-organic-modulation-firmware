/**
 * @file DriftEngine.h
 * Declares compile-time algorithm-bank/target selection and the portable Drift processing core.
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

#include "fmd/config/AlgorithmTargetConfig.h"
#include "fmd/domain/Types.h"
#include "fmd/ports/ReferenceTables.h"

#if FMD_ALGORITHM_BANK == FMD_BANK_CLASSIC
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_BEZIER
#include "fmd/domain/classic/BezierAlgorithm.h"
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_BROWNIAN
#include "fmd/domain/classic/BrownianAlgorithm.h"
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_LFO
#include "fmd/domain/classic/LfoAlgorithm.h"
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_PERLIN
#include "fmd/domain/classic/PerlinAlgorithm.h"
#endif
#elif FMD_ALGORITHM_BANK == FMD_BANK_ORGANIC
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_ATTRACTOR
#include "fmd/domain/organic/AttractorAlgorithm.h"
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_FRACTAL
#include "fmd/domain/organic/FractalAlgorithm.h"
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_RAIN
#include "fmd/domain/organic/RainAlgorithm.h"
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_VECTOR
#include "fmd/domain/organic/VectorAlgorithm.h"
#endif
#elif FMD_ALGORITHM_BANK == FMD_BANK_GENERATIVE
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_MARKOV
#include "fmd/domain/generative/MarkovAlgorithm.h"
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_MOTIF
#include "fmd/domain/generative/MotifAlgorithm.h"
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_TURING
#include "fmd/domain/generative/TuringAlgorithm.h"
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_URN
#include "fmd/domain/generative/UrnAlgorithm.h"
#endif
#elif FMD_ALGORITHM_BANK == FMD_BANK_AMBIENT
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_ANCHOR
#include "fmd/domain/ambient/AnchorAlgorithm.h"
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_BREATH
#include "fmd/domain/ambient/BreathAlgorithm.h"
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_CURRENT
#include "fmd/domain/ambient/CurrentAlgorithm.h"
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_FOG
#include "fmd/domain/ambient/FogAlgorithm.h"
#endif
#elif FMD_ALGORITHM_BANK == FMD_BANK_ELECTRONICA
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_ACID
#include "fmd/domain/electronica/AcidAlgorithm.h"
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_POLYMETER
#include "fmd/domain/electronica/PolymeterAlgorithm.h"
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_PUMP
#include "fmd/domain/electronica/PumpAlgorithm.h"
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_SHUFFLE
#include "fmd/domain/electronica/ShuffleAlgorithm.h"
#endif
#elif FMD_ALGORITHM_BANK == FMD_BANK_PERCUSSION
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_EUCLID
#include "fmd/domain/percussion/EuclidAlgorithm.h"
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_HUMANIZE
#include "fmd/domain/percussion/HumanizeAlgorithm.h"
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_PROBABILITY
#include "fmd/domain/percussion/ProbabilityAlgorithm.h"
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_REPEAT
#include "fmd/domain/percussion/RepeatAlgorithm.h"
#endif
#elif FMD_ALGORITHM_BANK == FMD_BANK_DUBSTEP
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_WOBBLE
#include "fmd/domain/dubstep/WobbleAlgorithm.h"
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_GROWL
#include "fmd/domain/dubstep/GrowlAlgorithm.h"
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_CHOP
#include "fmd/domain/dubstep/ChopAlgorithm.h"
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_BUILD
#include "fmd/domain/dubstep/BuildAlgorithm.h"
#endif
#endif

namespace fmd {

/**
 * @brief Own state for the compile-time-selected bank or named developer target.
 *
 * Normal firmware contains one four-algorithm bank and the rear DIP switches select
 * one algorithm at startup. A developer build with FMD_FORCED_ALGORITHM set keeps
 * only the named algorithm referenced by DriftEngine, allowing linker garbage
 * collection to remove the other three bank algorithms from the final image.
 */
class DriftEngine {
 public:
  /**
   * @brief Construct the selected algorithm state and choose one active mode.
   * @param algorithm Algorithm fixed for this engine lifetime; normally returned by startupAlgorithm().
   * @param randomSeed Deterministic 16-bit seed shared by stochastic algorithms.
   * @param referenceTables Read-only numerical lookup provider.
   */
  DriftEngine(Algorithm algorithm, uint16_t randomSeed, const IReferenceTables& referenceTables);

  /** @brief Advance the selected algorithm by one sample. */
  uint16_t step(const ControlFrame& controls);

  /** @return Algorithm selected when this engine was constructed. */
  Algorithm algorithm() const { return selectedAlgorithm_; }

 private:
  Algorithm selectedAlgorithm_;  ///< Active algorithm for this power cycle.

#if FMD_ALGORITHM_BANK == FMD_BANK_CLASSIC
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_PERLIN
  PerlinAlgorithm perlinAlgorithm_;
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_BROWNIAN
  BrownianAlgorithm brownianAlgorithm_;
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_BEZIER
  BezierAlgorithm bezierAlgorithm_;
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_LFO
  LfoAlgorithm lfoAlgorithm_;
#endif
#elif FMD_ALGORITHM_BANK == FMD_BANK_ORGANIC
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_FRACTAL
  FractalAlgorithm fractalAlgorithm_;
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_VECTOR
  VectorAlgorithm vectorAlgorithm_;
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_RAIN
  RainAlgorithm rainAlgorithm_;
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_ATTRACTOR
  AttractorAlgorithm attractorAlgorithm_;
#endif
#elif FMD_ALGORITHM_BANK == FMD_BANK_GENERATIVE
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_TURING
  TuringAlgorithm turingAlgorithm_;
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_MARKOV
  MarkovAlgorithm markovAlgorithm_;
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_MOTIF
  MotifAlgorithm motifAlgorithm_;
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_URN
  UrnAlgorithm urnAlgorithm_;
#endif
#elif FMD_ALGORITHM_BANK == FMD_BANK_AMBIENT
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_CURRENT
  CurrentAlgorithm currentAlgorithm_;
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_ANCHOR
  AnchorAlgorithm anchorAlgorithm_;
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_BREATH
  BreathAlgorithm breathAlgorithm_;
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_FOG
  FogAlgorithm fogAlgorithm_;
#endif
#elif FMD_ALGORITHM_BANK == FMD_BANK_ELECTRONICA
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_PUMP
  PumpAlgorithm pumpAlgorithm_;
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_ACID
  AcidAlgorithm acidAlgorithm_;
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_SHUFFLE
  ShuffleAlgorithm shuffleAlgorithm_;
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_POLYMETER
  PolymeterAlgorithm polymeterAlgorithm_;
#endif
#elif FMD_ALGORITHM_BANK == FMD_BANK_PERCUSSION
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_EUCLID
  EuclidAlgorithm euclidAlgorithm_;
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_REPEAT
  RepeatAlgorithm repeatAlgorithm_;
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_PROBABILITY
  ProbabilityAlgorithm probabilityAlgorithm_;
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_HUMANIZE
  HumanizeAlgorithm humanizeAlgorithm_;
#endif
#elif FMD_ALGORITHM_BANK == FMD_BANK_DUBSTEP
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_WOBBLE
  WobbleAlgorithm wobbleAlgorithm_;
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_GROWL
  GrowlAlgorithm growlAlgorithm_;
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_CHOP
  ChopAlgorithm chopAlgorithm_;
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_BUILD
  BuildAlgorithm buildAlgorithm_;
#endif
#endif
};

/** @brief Decode the two active-low rear configuration inputs for the compiled bank. */
Algorithm algorithmFromConfig(bool configInput1Low, bool configInput2Low);

/**
 * @brief Select the startup algorithm for this firmware image.
 * @return Forced developer algorithm when configured, otherwise the rear-DIP selection.
 */
Algorithm startupAlgorithm(bool configInput1Low, bool configInput2Low);

/** @brief Return the algorithm occupying a logical DIP slot in the compiled bank. */
Algorithm algorithmForBankSlot(uint8_t slotIndex);

}  // namespace fmd
#endif  // FMD_DOMAIN_DRIFT_ENGINE_H
