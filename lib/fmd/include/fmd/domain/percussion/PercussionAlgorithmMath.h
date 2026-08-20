/**
 * @file PercussionAlgorithmMath.h
 * Declares fixed-point rhythm primitives shared by the Percussion bank.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_DOMAIN_PERCUSSION_ALGORITHM_MATH_H
#define FMD_DOMAIN_PERCUSSION_ALGORITHM_MATH_H

#include <stdint.h>

#include "fmd/domain/ClockSource.h"

namespace fmd::percussionmath {

constexpr uint8_t kPulseSamples = 25U;
constexpr uint16_t kFullScaleDac12 = 4095U;

/** Shared clock detector constants and types retained under the Percussion API. */
constexpr uint16_t kClockLowThresholdAdc = clock::kClockLowThresholdAdc;
constexpr uint16_t kClockHighThresholdAdc = clock::kClockHighThresholdAdc;
constexpr uint32_t kClockMinimumPeriodSamples = clock::kClockMinimumPeriodSamples;
constexpr uint32_t kClockMaximumPeriodSamples = clock::kClockMaximumPeriodSamples;
using ClockUpdate = clock::ClockUpdate;
using ClockSource = clock::ClockSource;
using clock::quarterIncrementFromPeriodSamples;
using clock::clockTimeoutSamples;

uint8_t phraseLengthBars(uint16_t textureControl);
uint8_t fillStrength(uint16_t textureControl);

class PhraseState {
 public:
  PhraseState();
  void start(uint16_t textureControl);
  void advanceBar(uint16_t textureControl);
  uint8_t barIndex() const { return barIndex_; }
  uint8_t phraseLength() const { return phraseLength_; }
  bool isFillBar() const { return barIndex_ + 1U == phraseLength_; }
 private:
  uint8_t barIndex_;
  uint8_t phraseLength_;
};

uint16_t randomCutoffLinear(uint16_t textureControl);
bool randomBelow(uint16_t randomWord, uint32_t cutoff);
int16_t signedDeviation(uint16_t randomWord, uint16_t radius);

}  // namespace fmd::percussionmath

namespace fmd::euclidmath {
uint8_t hitCount(uint16_t textureControl);
uint16_t canonicalMask(uint8_t hitCount);
uint16_t fillTailMask(uint8_t fillStrength);
bool stepHits(uint16_t mask, uint8_t stepIndex);
}  // namespace fmd::euclidmath

namespace fmd::repeatmath {
uint32_t repeatCutoff(uint16_t textureControl);
uint8_t ratchetCount(uint16_t textureControl);
uint32_t subEventThreshold(uint8_t pulseIndex, uint8_t pulseCount);
uint8_t forcedMinimumCount(uint8_t fillStrength, uint8_t quarterIndex);
}  // namespace fmd::repeatmath

namespace fmd::probabilitymath {
enum class StepClass : uint8_t { Primary, Secondary, Ghost };
StepClass classifyStep(uint8_t stepIndex);
uint32_t secondaryCutoff(uint16_t textureControl);
uint32_t ghostCutoff(uint16_t textureControl);
uint32_t fillBoostCutoff(uint8_t fillStrength);
uint32_t effectiveCutoff(StepClass stepClass,
                         uint16_t textureControl,
                         bool fillBar,
                         bool finalQuarter,
                         uint8_t fillStrength);
}  // namespace fmd::probabilitymath

namespace fmd::humanizemath {
uint8_t jitterRadiusSamples(uint16_t textureControl);
uint16_t amplitudeRadiusDac12(uint16_t textureControl);
int8_t jitterSamples(uint16_t randomWord, uint16_t textureControl);
uint16_t pulseAmplitudeDac12(uint16_t randomWord, uint16_t textureControl);
}  // namespace fmd::humanizemath

#endif
