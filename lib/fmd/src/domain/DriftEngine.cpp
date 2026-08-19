/**
 * @file DriftEngine.cpp
 * Implements compile-time algorithm-bank selection and portable Drift sample dispatch.
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
  constexpr Algorithm kAlgorithms[4] = {
      Algorithm::Perlin,
      Algorithm::Brownian,
      Algorithm::Bezier,
      Algorithm::Lfo,
  };
#elif FMD_ALGORITHM_BANK == FMD_BANK_ORGANIC
  constexpr Algorithm kAlgorithms[4] = {
      Algorithm::Fractal,
      Algorithm::Vector,
      Algorithm::Rain,
      Algorithm::Attractor,
  };
#elif FMD_ALGORITHM_BANK == FMD_BANK_GENERATIVE
  constexpr Algorithm kAlgorithms[4] = {
      Algorithm::Turing,
      Algorithm::Markov,
      Algorithm::Motif,
      Algorithm::Urn,
  };
#elif FMD_ALGORITHM_BANK == FMD_BANK_AMBIENT
  constexpr Algorithm kAlgorithms[4] = {
      Algorithm::Current,
      Algorithm::Anchor,
      Algorithm::Breath,
      Algorithm::Fog,
  };
#endif
  return kAlgorithms[slotIndex < 4U ? slotIndex : 0U];
}

Algorithm algorithmFromConfig(bool configInput1Low, bool configInput2Low) {
  // Preserve the original firmware-pin truth table. Physical rear DIP numbering
  // is documented separately because DIP 1/2 order differs from CONFIG 1/2.
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

DriftEngine::DriftEngine(Algorithm algorithm,
                         uint16_t randomSeed,
                         const IReferenceTables& referenceTables)
    : selectedAlgorithm_(algorithm)
#if FMD_ALGORITHM_BANK == FMD_BANK_CLASSIC
      , perlinAlgorithm_(referenceTables, randomSeed)
      , brownianAlgorithm_(randomSeed)
      , bezierAlgorithm_(referenceTables, randomSeed)
      , lfoAlgorithm_(referenceTables)
#elif FMD_ALGORITHM_BANK == FMD_BANK_ORGANIC
      , fractalAlgorithm_(referenceTables, randomSeed)
      , vectorAlgorithm_(referenceTables)
      , rainAlgorithm_(randomSeed)
      , attractorAlgorithm_(referenceTables)
#elif FMD_ALGORITHM_BANK == FMD_BANK_GENERATIVE
      , turingAlgorithm_(referenceTables, randomSeed)
      , markovAlgorithm_(referenceTables, randomSeed)
      , motifAlgorithm_(referenceTables, randomSeed)
      , urnAlgorithm_(referenceTables, randomSeed)
#elif FMD_ALGORITHM_BANK == FMD_BANK_AMBIENT
      , currentAlgorithm_(referenceTables)
      , anchorAlgorithm_(referenceTables, randomSeed)
      , breathAlgorithm_(referenceTables, randomSeed)
      , fogAlgorithm_(referenceTables, randomSeed)
#endif
{}

uint16_t DriftEngine::step(const ControlFrame& controls) {
  switch (selectedAlgorithm_) {
#if FMD_ALGORITHM_BANK == FMD_BANK_CLASSIC
    case Algorithm::Perlin:
      return perlinAlgorithm_.step(controls);
    case Algorithm::Brownian:
      return brownianAlgorithm_.step(controls);
    case Algorithm::Bezier:
      return bezierAlgorithm_.step(controls);
    case Algorithm::Lfo:
      return lfoAlgorithm_.step(controls);
#elif FMD_ALGORITHM_BANK == FMD_BANK_ORGANIC
    case Algorithm::Fractal:
      return fractalAlgorithm_.step(controls);
    case Algorithm::Vector:
      return vectorAlgorithm_.step(controls);
    case Algorithm::Rain:
      return rainAlgorithm_.step(controls);
    case Algorithm::Attractor:
      return attractorAlgorithm_.step(controls);
#elif FMD_ALGORITHM_BANK == FMD_BANK_GENERATIVE
    case Algorithm::Turing:
      return turingAlgorithm_.step(controls);
    case Algorithm::Markov:
      return markovAlgorithm_.step(controls);
    case Algorithm::Motif:
      return motifAlgorithm_.step(controls);
    case Algorithm::Urn:
      return urnAlgorithm_.step(controls);
#elif FMD_ALGORITHM_BANK == FMD_BANK_AMBIENT
    case Algorithm::Current:
      return currentAlgorithm_.step(controls);
    case Algorithm::Anchor:
      return anchorAlgorithm_.step(controls);
    case Algorithm::Breath:
      return breathAlgorithm_.step(controls);
    case Algorithm::Fog:
      return fogAlgorithm_.step(controls);
#endif
    default:
      break;
  }

  // Defensive fallback for invalid enum values or a valid algorithm identity
  // from the bank that was intentionally not compiled into this firmware image.
  return 0U;
}

}  // namespace fmd
