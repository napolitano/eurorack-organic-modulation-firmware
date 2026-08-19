/**
 * @file PerlinAlgorithm.cpp
 * Implements the two-octave Perlin / gradient-noise Drift algorithm.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "fmd/domain/PerlinAlgorithm.h"

#include "fmd/domain/AlgorithmMath.h"
#include "fmd/domain/FixedMath.h"
#include "fmd/domain/FrequencyMapping.h"

#include <stdint.h>

namespace fmd {

PerlinAlgorithm::PerlinAlgorithm(const IReferenceTables& referenceTables,
                                 uint16_t randomSeed)
    : referenceTables_(referenceTables),
      baseOctave_{0U, 0, 0},
      detailOctave_{0U, 0, 0},
      randomGenerator_(randomSeed) {
  baseOctave_.startGradient = nextRandomGradient(randomGenerator_);
  baseOctave_.endGradient = nextRandomGradient(randomGenerator_);
  detailOctave_.startGradient = nextRandomGradient(randomGenerator_);
  detailOctave_.endGradient = nextRandomGradient(randomGenerator_);
}

int16_t PerlinAlgorithm::nextRandomGradient(ParallelLfsr& randomGenerator) {
  return perlinmath::gradientFromRandom(randomGenerator.next());
}

int16_t PerlinAlgorithm::advanceOctave(OctaveState& octaveState,
                                       uint32_t phaseIncrement) {
  bool crossedLatticeBoundary = false;
  octaveState.phaseAccumulator = perlinmath::advancePhase(
      octaveState.phaseAccumulator, phaseIncrement, crossedLatticeBoundary);

  if (crossedLatticeBoundary) {
    // Reuse the former right-hand gradient as the next segment's left-hand
    // gradient. This is the continuity condition that prevents value jumps at
    // integer lattice boundaries.
    octaveState.startGradient = octaveState.endGradient;
    octaveState.endGradient = nextRandomGradient(randomGenerator_);
  }

  const uint16_t segmentPhaseQ0F16 =
      static_cast<uint16_t>(octaveState.phaseAccumulator >> 16U);
  return perlinmath::segmentQ1F15(segmentPhaseQ0F16,
                                  octaveState.startGradient,
                                  octaveState.endGradient);
}

uint16_t PerlinAlgorithm::step(const ControlFrame& controls) {
  const uint32_t basePhaseIncrement = phaseIncrementFromControls(
      referenceTables_, controls.speedKnob, controls.speedCv, 0);
  const int16_t baseNoiseQ1F15 = advanceOctave(baseOctave_, basePhaseIncrement);
  const int16_t detailNoiseQ1F15 = advanceOctave(
      detailOctave_, static_cast<uint32_t>(basePhaseIncrement * 4UL));

  const int16_t textureBlendQ1F15 = static_cast<int16_t>(
      sumAdc(controls.textureKnob, controls.textureCv) << 5U);
  constexpr int16_t kUnityQ1F15 = 0x7FFF;

  // Preserve the original two-octave weighting: the base component dominates,
  // while Texture crossfades additional four-times-faster detail into the mix.
  const int32_t mixedNoise =
      static_cast<int32_t>(baseNoiseQ1F15) * 3 +
      fixedmath::mulI1F15(
          baseNoiseQ1F15,
          static_cast<int16_t>(kUnityQ1F15 - textureBlendQ1F15)) +
      fixedmath::mulI1F15(detailNoiseQ1F15, textureBlendQ1F15);

  int32_t outputCode = mixedNoise / 16 + 2048;
  // These clamps are defensive. The verified mathematical amplitude of the
  // allowed gradient set remains within the 12-bit range, but the guards keep
  // this API safe if an internal invariant is violated by future changes.
  if (outputCode < 0) {
    outputCode = 0;
  }
  if (outputCode > 4095) {
    outputCode = 4095;
  }
  return static_cast<uint16_t>(outputCode);
}

}  // namespace fmd
