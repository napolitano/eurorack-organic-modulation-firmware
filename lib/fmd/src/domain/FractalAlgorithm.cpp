/**
 * @file FractalAlgorithm.cpp
 * Implements the three-scale gradient-noise Fractal algorithm from the optional Organic bank.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "fmd/domain/FractalAlgorithm.h"

#include "fmd/domain/AlgorithmMath.h"
#include "fmd/domain/FrequencyMapping.h"
#include "fmd/domain/OrganicAlgorithmMath.h"

#include <stdint.h>

namespace fmd {

FractalAlgorithm::FractalAlgorithm(const IReferenceTables& referenceTables,
                                   uint16_t randomSeed)
    : referenceTables_(referenceTables),
      macroOctave_{0U, 0, 0},
      mesoOctave_{0U, 0, 0},
      detailOctave_{0U, 0, 0},
      randomGenerator_(randomSeed) {
  macroOctave_.startGradient = nextRandomGradient();
  macroOctave_.endGradient = nextRandomGradient();
  mesoOctave_.startGradient = nextRandomGradient();
  mesoOctave_.endGradient = nextRandomGradient();
  detailOctave_.startGradient = nextRandomGradient();
  detailOctave_.endGradient = nextRandomGradient();
}

int16_t FractalAlgorithm::nextRandomGradient() {
  return perlinmath::gradientFromRandom(randomGenerator_.next());
}

int16_t FractalAlgorithm::advanceOctave(OctaveState& octaveState,
                                        uint32_t phaseIncrement) {
  bool crossedLatticeBoundary = false;
  octaveState.phaseAccumulator = perlinmath::advancePhase(
      octaveState.phaseAccumulator, phaseIncrement, crossedLatticeBoundary);

  if (crossedLatticeBoundary) {
    octaveState.startGradient = octaveState.endGradient;
    octaveState.endGradient = nextRandomGradient();
  }

  const uint16_t segmentPhaseQ0F16 =
      static_cast<uint16_t>(octaveState.phaseAccumulator >> 16U);
  return perlinmath::segmentQ1F15(segmentPhaseQ0F16,
                                  octaveState.startGradient,
                                  octaveState.endGradient);
}

uint16_t FractalAlgorithm::step(const ControlFrame& controls) {
  const uint32_t macroPhaseIncrement = phaseIncrementFromControls(
      referenceTables_, controls.speedKnob, controls.speedCv, 0);
  const int16_t macroNoiseQ1F15 =
      advanceOctave(macroOctave_, macroPhaseIncrement);
  const int16_t mesoNoiseQ1F15 =
      advanceOctave(mesoOctave_, static_cast<uint32_t>(macroPhaseIncrement * 4UL));
  const int16_t detailNoiseQ1F15 =
      advanceOctave(detailOctave_, static_cast<uint32_t>(macroPhaseIncrement * 16UL));

  const uint16_t textureControl =
      sumAdc(controls.textureKnob, controls.textureCv);
  const fractalmath::OctaveWeights weights =
      fractalmath::octaveWeights(textureControl);
  const int16_t mixedNoiseQ1F15 = fractalmath::mixOctavesQ1F15(
      macroNoiseQ1F15, mesoNoiseQ1F15, detailNoiseQ1F15, weights);

  const int32_t biasedOutput = static_cast<int32_t>(mixedNoiseQ1F15) + 32768L;
  const uint32_t outputCode = static_cast<uint32_t>(biasedOutput) >> 4U;
  return static_cast<uint16_t>(outputCode > 4095U ? 4095U : outputCode);
}

}  // namespace fmd
