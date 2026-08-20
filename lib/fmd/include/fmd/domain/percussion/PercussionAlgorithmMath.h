/**
 * @file PercussionAlgorithmMath.h
 * Declares fixed-point rhythm primitives shared by the Percussion bank.
 *
 * @details
 * Percussion uses a 16-step bar, a 4/8/12/16-bar phrase layer and the shared
 * ClockSource transport. Helpers in this header are deterministic and separately
 * unit-testable. Probability cutoffs use the half-open 16-bit random domain
 * 0..65535, with 65536 represented in uint32_t as the exact always-hit endpoint.
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

constexpr uint8_t kPulseSamples = 25U;       ///< 10 ms at the 2.5 kHz scheduler rate.
constexpr uint16_t kFullScaleDac12 = 4095U;  ///< Full-scale 12-bit pulse amplitude.

/** @brief Shared clock detector constants/types retained under the Percussion API. */
constexpr uint16_t kClockLowThresholdAdc = clock::kClockLowThresholdAdc;
constexpr uint16_t kClockHighThresholdAdc = clock::kClockHighThresholdAdc;
constexpr uint32_t kClockMinimumPeriodSamples = clock::kClockMinimumPeriodSamples;
constexpr uint32_t kClockMaximumPeriodSamples = clock::kClockMaximumPeriodSamples;
using ClockUpdate = clock::ClockUpdate;
using ClockSource = clock::ClockSource;
using clock::quarterIncrementFromPeriodSamples;
using clock::clockTimeoutSamples;

/**
 * @brief Map Texture macro regions to phrase lengths 16, 12, 8 and 4 bars.
 * @param textureControl Combined Texture control; clamped to 0..1023.
 * @return Phrase length in bars.
 */
uint8_t phraseLengthBars(uint16_t textureControl);

/**
 * @brief Quantize Texture to the five fill-strength levels 0..4.
 * @param textureControl Combined Texture control; clamped to 0..1023.
 * @return Fill strength 0..4.
 */
uint8_t fillStrength(uint16_t textureControl);

/** @brief Bar counter that latches a new phrase length only when a phrase wraps. */
class PhraseState {
 public:
  /** @brief Construct phrase state at bar zero with the least-active 16-bar default. */
  PhraseState();

  /**
   * @brief Reset to bar zero and latch a phrase length from Texture.
   * @param textureControl Combined Texture macro value.
   */
  void start(uint16_t textureControl);

  /**
   * @brief Advance one bar, latching a new phrase length only on phrase wrap.
   * @param textureControl Texture used only if the current phrase completes.
   */
  void advanceBar(uint16_t textureControl);

  /** @return Current zero-based bar index within the latched phrase. */
  uint8_t barIndex() const { return barIndex_; }
  /** @return Current latched phrase length in bars. */
  uint8_t phraseLength() const { return phraseLength_; }
  /** @return true only for the final bar of the current phrase. */
  bool isFillBar() const { return barIndex_ + 1U == phraseLength_; }

 private:
  uint8_t barIndex_;      ///< Zero-based current bar index.
  uint8_t phraseLength_;  ///< Latched phrase length: 4, 8, 12 or 16 bars.
};

/**
 * @brief Convert Texture to a linear 16-bit random cutoff.
 * @param textureControl Combined Texture control; clamped to 0..1023.
 * @return Cutoff 0..65535; Texture maximum maps to 65535 by design.
 */
uint16_t randomCutoffLinear(uint16_t textureControl);

/**
 * @brief Compare a 16-bit random word against an extended half-open cutoff.
 * @param randomWord Uniform random word in 0..65535.
 * @param cutoff Cutoff in 0..65536; 65536 means always true.
 * @return true when randomWord is strictly below cutoff.
 */
bool randomBelow(uint16_t randomWord, uint32_t cutoff);

/**
 * @brief Map a uniform word to an approximately uniform signed integer deviation.
 * @param randomWord Uniform random word in 0..65535.
 * @param radius Inclusive deviation radius.
 * @return Integer in the inclusive interval [-radius,+radius].
 */
int16_t signedDeviation(uint16_t randomWord, uint16_t radius);

}  // namespace fmd::percussionmath

namespace fmd::euclidmath {

/**
 * @brief Map Texture to Euclidean hit count k for E(k,16).
 * @param textureControl Combined Texture control; clamped to 0..1023.
 * @return Hit count 2..13.
 */
uint8_t hitCount(uint16_t textureControl);

/**
 * @brief Return the verified canonical 16-step mask for the requested hit count.
 * @param hitCount Requested k; defensively clamped to 2..13.
 * @return 16-bit mask whose set bits are Euclidean onsets.
 */
uint16_t canonicalMask(uint8_t hitCount);

/**
 * @brief Build the deterministic phrase-fill mask over the final bar steps.
 * @param fillStrength Strength 0..4; values above four are clamped.
 * @return Mask adding the final 0..4 steps without removing base-pattern hits.
 */
uint16_t fillTailMask(uint8_t fillStrength);

/**
 * @brief Test one wrapped 16-step position against a rhythm mask.
 * @param mask 16-step bit mask.
 * @param stepIndex Step index; low four bits select the position.
 * @return true when the selected bit is set.
 */
bool stepHits(uint16_t mask, uint8_t stepIndex);

}  // namespace fmd::euclidmath

namespace fmd::repeatmath {

/**
 * @brief Map Texture to repeat probability, capped at 75 percent.
 * @param textureControl Combined Texture control; clamped to 0..1023.
 * @return Extended 16-bit-domain cutoff 0..49152.
 */
uint32_t repeatCutoff(uint16_t textureControl);

/**
 * @brief Map Texture to the cluster size used after a repeat decision.
 * @param textureControl Combined Texture control; clamped to 0..1023.
 * @return Ratchet pulse count 2..4.
 */
uint8_t ratchetCount(uint16_t textureControl);

/**
 * @brief Return the phase threshold of one ratchet subevent within a quarter.
 * @param pulseIndex Zero-based cluster pulse index; anchor pulse is index zero.
 * @param pulseCount Total cluster pulse count.
 * @return Unsigned quarter-phase threshold; invalid/non-subevent inputs return zero.
 */
uint32_t subEventThreshold(uint8_t pulseIndex, uint8_t pulseCount);

/**
 * @brief Enforce phrase-fill minimum cluster sizes near the end of the fill bar.
 * @param fillStrength Fill strength 0..4.
 * @param quarterIndex Quarter within the bar; low two bits are used.
 * @return Minimum cluster pulse count for this quarter, 1..4.
 */
uint8_t forcedMinimumCount(uint8_t fillStrength, uint8_t quarterIndex);

}  // namespace fmd::repeatmath

namespace fmd::probabilitymath {

/** @brief Metric role assigned to a 16-step grid position. */
enum class StepClass : uint8_t { Primary, Secondary, Ghost };

/**
 * @brief Classify one wrapped 16-step position by metric strength.
 * @param stepIndex Step index; low four bits select the bar position.
 * @return Primary for quarter anchors, Secondary for other even steps, otherwise Ghost.
 */
StepClass classifyStep(uint8_t stepIndex);

/**
 * @brief Map Texture to Secondary hit probability.
 * @param textureControl Combined Texture control; clamped to 0..1023.
 * @return Extended cutoff 0..65536, where 65536 means guaranteed.
 */
uint32_t secondaryCutoff(uint16_t textureControl);

/**
 * @brief Map Texture quadratically to Ghost hit probability.
 * @param textureControl Combined Texture control; clamped to 0..1023.
 * @return Extended cutoff from zero through 32768 (50 percent).
 */
uint32_t ghostCutoff(uint16_t textureControl);

/**
 * @brief Map fill strength to one additive optional-hit probability boost.
 * @param fillStrength Fill strength 0..4; values above four are clamped.
 * @return Additive cutoff in steps of 8192.
 */
uint32_t fillBoostCutoff(uint8_t fillStrength);

/**
 * @brief Combine metric class, Texture and fill context into the final hit cutoff.
 * @param stepClass Metric role of the current step.
 * @param textureControl Combined Texture control.
 * @param fillBar true on the final bar of the current phrase.
 * @param finalQuarter true for steps 12..15, which receive a second fill boost.
 * @param fillStrength Fill strength 0..4.
 * @return Extended cutoff 0..65536; Primary is always 65536.
 */
uint32_t effectiveCutoff(StepClass stepClass,
                         uint16_t textureControl,
                         bool fillBar,
                         bool finalQuarter,
                         uint8_t fillStrength);

}  // namespace fmd::probabilitymath

namespace fmd::humanizemath {

/**
 * @brief Map Texture to maximum absolute timing deviation.
 * @param textureControl Combined Texture control; clamped to 0..1023.
 * @return Radius 0..30 scheduler samples (0..12 ms at 2.5 kHz).
 */
uint8_t jitterRadiusSamples(uint16_t textureControl);

/**
 * @brief Map Texture to maximum pulse-amplitude deviation around 3840.
 * @param textureControl Combined Texture control; clamped to 0..1023.
 * @return Radius 0..255 DAC codes.
 */
uint16_t amplitudeRadiusDac12(uint16_t textureControl);

/**
 * @brief Draw one signed timing offset from the Texture-dependent radius.
 * @param randomWord Uniform 16-bit PRNG word.
 * @param textureControl Combined Texture control.
 * @return Signed offset in scheduler samples, bounded by jitterRadiusSamples().
 */
int8_t jitterSamples(uint16_t randomWord, uint16_t textureControl);

/**
 * @brief Draw one bounded humanized pulse amplitude around the 3840 centre.
 * @param randomWord Uniform 16-bit PRNG word.
 * @param textureControl Combined Texture control.
 * @return Saturated 12-bit pulse amplitude.
 */
uint16_t pulseAmplitudeDac12(uint16_t randomWord, uint16_t textureControl);

}  // namespace fmd::humanizemath

#endif  // FMD_DOMAIN_PERCUSSION_ALGORITHM_MATH_H
