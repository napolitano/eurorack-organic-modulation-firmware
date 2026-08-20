/**
 * @file DriftEngine.cpp
 * Implements compile-time algorithm-bank/target selection and portable Drift sample dispatch.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "fmd/domain/DriftEngine.h"

namespace fmd {

Algorithm algorithmForBankSlot(uint8_t slotIndex) {
#if FMD_ALGORITHM_BANK == FMD_BANK_CLASSIC
  constexpr Algorithm kAlgorithms[4] = {Algorithm::Perlin, Algorithm::Brownian, Algorithm::Bezier, Algorithm::Lfo};
#elif FMD_ALGORITHM_BANK == FMD_BANK_ORGANIC
  constexpr Algorithm kAlgorithms[4] = {Algorithm::Fractal, Algorithm::Vector, Algorithm::Rain, Algorithm::Attractor};
#elif FMD_ALGORITHM_BANK == FMD_BANK_GENERATIVE
  constexpr Algorithm kAlgorithms[4] = {Algorithm::Turing, Algorithm::Markov, Algorithm::Motif, Algorithm::Urn};
#elif FMD_ALGORITHM_BANK == FMD_BANK_AMBIENT
  constexpr Algorithm kAlgorithms[4] = {Algorithm::Current, Algorithm::Anchor, Algorithm::Breath, Algorithm::Fog};
#elif FMD_ALGORITHM_BANK == FMD_BANK_ELECTRONICA
  constexpr Algorithm kAlgorithms[4] = {Algorithm::Pump, Algorithm::Acid, Algorithm::Shuffle, Algorithm::Polymeter};
#elif FMD_ALGORITHM_BANK == FMD_BANK_PERCUSSION
  constexpr Algorithm kAlgorithms[4] = {Algorithm::Euclid, Algorithm::Repeat, Algorithm::Probability, Algorithm::Humanize};
#endif
  return kAlgorithms[slotIndex < 4U ? slotIndex : 0U];
}

Algorithm algorithmFromConfig(bool configInput1Low, bool configInput2Low) {
  if (!configInput1Low && !configInput2Low) {
    return algorithmForBankSlot(0U);
  }
  if (!configInput1Low && configInput2Low) {
    return algorithmForBankSlot(1U);
  }
  if (configInput1Low && !configInput2Low) {
    return algorithmForBankSlot(2U);
  }
  return algorithmForBankSlot(3U);
}

Algorithm startupAlgorithm(bool configInput1Low, bool configInput2Low) {
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO
  return algorithmFromConfig(configInput1Low, configInput2Low);
#else
  (void)configInput1Low;
  (void)configInput2Low;
  return static_cast<Algorithm>(FMD_FORCED_ALGORITHM);
#endif
}

DriftEngine::DriftEngine(Algorithm algorithm,
                         uint16_t randomSeed,
                         const IReferenceTables& referenceTables)
    : selectedAlgorithm_(algorithm)
#if FMD_ALGORITHM_BANK == FMD_BANK_CLASSIC
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_PERLIN
      , perlinAlgorithm_(referenceTables, randomSeed)
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_BROWNIAN
      , brownianAlgorithm_(randomSeed)
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_BEZIER
      , bezierAlgorithm_(referenceTables, randomSeed)
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_LFO
      , lfoAlgorithm_(referenceTables)
#endif
#elif FMD_ALGORITHM_BANK == FMD_BANK_ORGANIC
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_FRACTAL
      , fractalAlgorithm_(referenceTables, randomSeed)
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_VECTOR
      , vectorAlgorithm_(referenceTables)
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_RAIN
      , rainAlgorithm_(randomSeed)
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_ATTRACTOR
      , attractorAlgorithm_(referenceTables)
#endif
#elif FMD_ALGORITHM_BANK == FMD_BANK_GENERATIVE
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_TURING
      , turingAlgorithm_(referenceTables, randomSeed)
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_MARKOV
      , markovAlgorithm_(referenceTables, randomSeed)
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_MOTIF
      , motifAlgorithm_(referenceTables, randomSeed)
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_URN
      , urnAlgorithm_(referenceTables, randomSeed)
#endif
#elif FMD_ALGORITHM_BANK == FMD_BANK_AMBIENT
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_CURRENT
      , currentAlgorithm_(referenceTables)
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_ANCHOR
      , anchorAlgorithm_(referenceTables, randomSeed)
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_BREATH
      , breathAlgorithm_(referenceTables, randomSeed)
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_FOG
      , fogAlgorithm_(referenceTables, randomSeed)
#endif
#elif FMD_ALGORITHM_BANK == FMD_BANK_ELECTRONICA
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_PUMP
      , pumpAlgorithm_(referenceTables)
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_ACID
      , acidAlgorithm_(referenceTables)
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_SHUFFLE
      , shuffleAlgorithm_(referenceTables)
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_POLYMETER
      , polymeterAlgorithm_(referenceTables)
#endif
#elif FMD_ALGORITHM_BANK == FMD_BANK_PERCUSSION
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_EUCLID
      , euclidAlgorithm_(referenceTables)
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_REPEAT
      , repeatAlgorithm_(referenceTables, randomSeed)
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_PROBABILITY
      , probabilityAlgorithm_(referenceTables, randomSeed)
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_HUMANIZE
      , humanizeAlgorithm_(referenceTables, randomSeed)
#endif
#endif
{
  (void)randomSeed;
  (void)referenceTables;
}

uint16_t DriftEngine::step(const ControlFrame& controls) {
  switch (selectedAlgorithm_) {
#if FMD_ALGORITHM_BANK == FMD_BANK_CLASSIC
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_PERLIN
    case Algorithm::Perlin: return perlinAlgorithm_.step(controls);
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_BROWNIAN
    case Algorithm::Brownian: return brownianAlgorithm_.step(controls);
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_BEZIER
    case Algorithm::Bezier: return bezierAlgorithm_.step(controls);
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_LFO
    case Algorithm::Lfo: return lfoAlgorithm_.step(controls);
#endif
#elif FMD_ALGORITHM_BANK == FMD_BANK_ORGANIC
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_FRACTAL
    case Algorithm::Fractal: return fractalAlgorithm_.step(controls);
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_VECTOR
    case Algorithm::Vector: return vectorAlgorithm_.step(controls);
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_RAIN
    case Algorithm::Rain: return rainAlgorithm_.step(controls);
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_ATTRACTOR
    case Algorithm::Attractor: return attractorAlgorithm_.step(controls);
#endif
#elif FMD_ALGORITHM_BANK == FMD_BANK_GENERATIVE
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_TURING
    case Algorithm::Turing: return turingAlgorithm_.step(controls);
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_MARKOV
    case Algorithm::Markov: return markovAlgorithm_.step(controls);
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_MOTIF
    case Algorithm::Motif: return motifAlgorithm_.step(controls);
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_URN
    case Algorithm::Urn: return urnAlgorithm_.step(controls);
#endif
#elif FMD_ALGORITHM_BANK == FMD_BANK_AMBIENT
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_CURRENT
    case Algorithm::Current: return currentAlgorithm_.step(controls);
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_ANCHOR
    case Algorithm::Anchor: return anchorAlgorithm_.step(controls);
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_BREATH
    case Algorithm::Breath: return breathAlgorithm_.step(controls);
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_FOG
    case Algorithm::Fog: return fogAlgorithm_.step(controls);
#endif
#elif FMD_ALGORITHM_BANK == FMD_BANK_ELECTRONICA
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_PUMP
    case Algorithm::Pump: return pumpAlgorithm_.step(controls);
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_ACID
    case Algorithm::Acid: return acidAlgorithm_.step(controls);
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_SHUFFLE
    case Algorithm::Shuffle: return shuffleAlgorithm_.step(controls);
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_POLYMETER
    case Algorithm::Polymeter: return polymeterAlgorithm_.step(controls);
#endif
#elif FMD_ALGORITHM_BANK == FMD_BANK_PERCUSSION
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_EUCLID
    case Algorithm::Euclid: return euclidAlgorithm_.step(controls);
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_REPEAT
    case Algorithm::Repeat: return repeatAlgorithm_.step(controls);
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_PROBABILITY
    case Algorithm::Probability: return probabilityAlgorithm_.step(controls);
#endif
#if FMD_FORCED_ALGORITHM == FMD_ALGORITHM_AUTO || FMD_FORCED_ALGORITHM == FMD_ALGORITHM_HUMANIZE
    case Algorithm::Humanize: return humanizeAlgorithm_.step(controls);
#endif
#endif
    default: break;
  }
  return 0U;
}

}  // namespace fmd
