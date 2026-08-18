/**
 * @file AlgorithmMath.h
 * @brief Pure mathematical primitives used by Drift algorithms and their host tests.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_DOMAIN_ALGORITHM_MATH_H
#define FMD_DOMAIN_ALGORITHM_MATH_H

#include <stdint.h>

#include "fmd/ports/ReferenceTables.h"

namespace fmd::perlinmath {
uint16_t fadeQ0F16(uint16_t phase);
int16_t gradientFromRandom(uint16_t random);
int16_t segmentQ1F15(uint16_t phase, int16_t lastGradient, int16_t nextGradient);
uint32_t advancePhase(uint32_t phase, uint32_t delta, bool& rollover);
}

namespace fmd::brownianmath {
constexpr uint16_t kCenteringMargin = 5U;
constexpr uint16_t kCenteringStrength = 64U;
constexpr uint16_t kMinAlphaQ0F16 = static_cast<uint16_t>(0xFFFFU / 4096U);
constexpr uint16_t kMaxAlphaQ0F16 = static_cast<uint16_t>(0xFFFFU / 8U);

uint16_t stepSize(uint16_t speed);
uint16_t eventCutoff(uint16_t speed);
uint16_t directionCutoff(uint16_t target, uint16_t cutoff);
uint16_t nextTarget(uint16_t target, uint16_t random, uint16_t speed);
uint16_t textureAlphaQ0F16(uint16_t texture);
void smoothToward(uint16_t target,
                  uint16_t alphaQ0F16,
                  uint16_t& current,
                  uint16_t& residualQ0F16,
                  int8_t& residualDirection);
}

namespace fmd::beziermath {
uint16_t speedVariationScaleQ1F15(uint16_t knob, uint16_t cv);
uint16_t smoothCurveQ4F12(uint16_t phaseQ4F12);
uint16_t reverseCurveQ4F12(uint16_t phaseQ4F12);
uint16_t textureBlendQ0F16(uint16_t texture);
uint16_t morphCurveQ4F12(uint16_t phaseQ4F12, uint16_t texture);
uint16_t interpolateQ4F12(uint16_t phaseQ4F12,
                          uint16_t aQ4F12,
                          uint16_t bQ4F12,
                          uint16_t texture);
int16_t triangularIcdfQ1F15(uint16_t uniform15, const IReferenceTables& tables);
uint32_t advancePhase(uint32_t phase, uint32_t delta, bool& rollover);
}

namespace fmd::lfomath {
uint16_t apexFromTexture(uint16_t texture);
uint16_t waveformQ0F16(uint16_t phase, uint16_t apex);
uint16_t waveform12(uint16_t phase, uint16_t apex);
uint16_t remapPhasePreservingOutput(uint16_t phase,
                                    uint16_t oldApex,
                                    uint16_t newApex);
uint32_t advancePhase(uint32_t phase, uint32_t delta, bool& rollover);
}

#endif
