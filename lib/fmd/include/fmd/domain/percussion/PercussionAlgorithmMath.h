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

namespace fmd::percussionmath {

constexpr uint8_t kPulseSamples = 25U;
constexpr uint16_t kFullScaleDac12 = 4095U;

/** ADC threshold corresponding to approximately 1.0 V on a 0..5 V input. */
constexpr uint16_t kClockLowThresholdAdc = 205U;
/** ADC threshold corresponding to approximately 2.0 V on a 0..5 V input. */
constexpr uint16_t kClockHighThresholdAdc = 410U;
/** Reject implausibly fast edge intervals below 12.8 ms at the 2.5 kHz scheduler rate. */
constexpr uint32_t kClockMinimumPeriodSamples = 32UL;
/** Accept external quarter-note periods up to 10 s (6 BPM). */
constexpr uint32_t kClockMaximumPeriodSamples = 25000UL;

/**
 * @brief Result of one Percussion clock-source update.
 *
 * Speed CV is bank-specifically interpreted as a 0..5 V clock input. Before
 * two valid rising edges are observed, and after an external-clock timeout,
 * quarterIncrement is the caller-provided Speed-knob increment.
 */
struct ClockUpdate {
  uint32_t quarterIncrement;  ///< Active quarter-note phase increment.
  bool externalActive;        ///< True while a measured external clock owns timing.
  bool quarterBoundary;       ///< True on an accepted external rising edge.
  bool externalAcquired;      ///< True only on the edge that activates external sync.
  bool externalLost;          ///< True only on the sample that times out to internal timing.
};

/** Convert a measured quarter-note interval to a 32-bit phase increment. */
uint32_t quarterIncrementFromPeriodSamples(uint32_t periodSamples);
/** Return the 2.5-period external-clock loss timeout in scheduler samples. */
uint32_t clockTimeoutSamples(uint32_t periodSamples);

/**
 * @brief Hysteretic Speed-CV clock detector with automatic Speed-knob fallback.
 *
 * Two accepted rising edges are required before external timing becomes active.
 * A valid edge is detected after the input has first fallen to or below
 * kClockLowThresholdAdc and then rises to or above kClockHighThresholdAdc.
 * Each external edge represents one quarter note.
 */
class ClockSource {
 public:
  ClockSource();
  ClockUpdate update(uint16_t speedCvAdc, uint32_t internalQuarterIncrement);
  bool externalActive() const { return externalActive_; }
  uint32_t lastPeriodSamples() const { return lastPeriodSamples_; }

 private:
  bool inputHigh_;
  bool haveReferenceEdge_;
  bool externalActive_;
  uint32_t samplesSinceReferenceEdge_;
  uint32_t lastPeriodSamples_;
  uint32_t externalQuarterIncrement_;
};

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
