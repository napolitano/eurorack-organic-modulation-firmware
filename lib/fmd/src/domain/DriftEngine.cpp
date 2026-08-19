/**
 * @file DriftEngine.cpp
 * Implements algorithm selection and portable Drift sample dispatch.
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

Algorithm algorithmFromConfig(bool configInput1Low, bool configInput2Low) {
  if (!configInput1Low && !configInput2Low) {
    return Algorithm::Perlin;
  }
  if (!configInput1Low && configInput2Low) {
    return Algorithm::Brownian;
  }
  if (configInput1Low && !configInput2Low) {
    return Algorithm::Bezier;
  }
  return Algorithm::Lfo;
}

DriftEngine::DriftEngine(Algorithm algorithm,
                         uint16_t randomSeed,
                         const IReferenceTables& referenceTables)
    : selectedAlgorithm_(algorithm),
      perlinAlgorithm_(referenceTables, randomSeed),
      brownianAlgorithm_(randomSeed),
      bezierAlgorithm_(referenceTables, randomSeed),
      lfoAlgorithm_(referenceTables) {}

uint16_t DriftEngine::step(const ControlFrame& controls) {
  switch (selectedAlgorithm_) {
    case Algorithm::Perlin:
      return perlinAlgorithm_.step(controls);
    case Algorithm::Brownian:
      return brownianAlgorithm_.step(controls);
    case Algorithm::Bezier:
      return bezierAlgorithm_.step(controls);
    case Algorithm::Lfo:
      return lfoAlgorithm_.step(controls);
  }

  // Defensive fallback for invalid enum values introduced through unchecked
  // casts or memory corruption. Normal construction cannot reach this branch.
  return 0U;
}

}  // namespace fmd
