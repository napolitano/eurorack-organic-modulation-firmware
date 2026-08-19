/**
 * @file AmbientAlgorithmMath.h
 * Declares pure mathematical primitives used by the optional Ambient algorithm bank.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_DOMAIN_AMBIENT_ALGORITHM_MATH_H
#define FMD_DOMAIN_AMBIENT_ALGORITHM_MATH_H

#include <stdint.h>

namespace fmd::ambientmath {

/** Divide the established Drift phase rate by sixteen for Ambient macro timing. */
uint32_t ambientPhaseIncrement(uint32_t basePhaseIncrement);

/** Reproduce the saturated effective Speed control used by the common frequency map. */
uint16_t mappedSpeedControl(uint16_t speedKnobAdc, uint16_t speedCvAdc);

/** Quantize mapped Speed into the 307-entry Anchor compensation-table domain. */
uint16_t anchorSpeedBucket(uint16_t speedKnobAdc, uint16_t speedCvAdc);

/** Cubic smoothstep S(x)=3x^2-2x^3 in unsigned Q0.12. */
uint16_t smoothstepQ0F12(uint16_t xQ0F12);

}  // namespace fmd::ambientmath

namespace fmd::currentmath {

/** Constant-sum three-current weights, denominator 1024. */
struct Weights {
  uint16_t primary;    ///< Fundamental-current weight.
  uint16_t secondary;  ///< sqrt(2)-rate current weight.
  uint16_t tertiary;   ///< golden-ratio-rate current weight.
};

/** Approximate sqrt(2) rate multiplication using 362/256. */
uint32_t sqrt2Increment(uint32_t baseIncrement);

/** Approximate golden-ratio rate multiplication using 414/256. */
uint32_t phiIncrement(uint32_t baseIncrement);

/** Evaluate a slope-softened bipolar triangle in approximately Q3.12. */
int16_t softTriangleQ3F12(uint32_t phase);

/** Map Texture to the documented constant-sum Current weights. */
Weights weights(uint16_t textureControl);

/** Mix three signed soft currents and project to 12-bit DAC code. */
uint16_t mixToDac12(int16_t primaryQ3F12,
                    int16_t secondaryQ3F12,
                    int16_t tertiaryQ3F12,
                    const Weights& weights);

}  // namespace fmd::currentmath

namespace fmd::anchormath {

/** Maximum target stationary spread, 0.30 in signed Q1.15. */
constexpr uint16_t kMaximumSpreadQ1F15 = 9830U;

/** Map Texture to target stationary spread. */
uint16_t spreadQ1F15(uint16_t textureControl);

/** Convert Ambient phase rate to per-sample mean-reversion alpha in Q0.24. */
uint32_t reversionAlphaQ0F24(uint32_t ambientPhaseIncrement);

/**
 * Move one signed Q1.15 state toward zero while preserving fractional motion.
 * Residual state is reset when the side of zero changes.
 */
int16_t revertTowardZero(int16_t stateQ1F15,
                         uint32_t alphaQ0F24,
                         uint32_t& fractionalResidualQ0F24,
                         int8_t& residualDirection);

/** Scale one triangular Q1.15 innovation by Speed compensation and Texture spread. */
int16_t scaledInnovationQ1F15(int16_t triangularSampleQ1F15,
                             uint16_t innovationGainQ1F15,
                             uint16_t spreadQ1F15);

/** Add innovation with saturation to the signed Q1.15 state domain. */
int16_t addInnovationSaturating(int16_t stateQ1F15, int16_t innovationQ1F15);

/** Project signed Q1.15 state around DAC midpoint. */
uint16_t projectToDac12(int16_t stateQ1F15);

}  // namespace fmd::anchormath

namespace fmd::breathmath {

constexpr uint16_t kDurationMinimumQ10 = 768U;
constexpr uint16_t kDurationNominalQ10 = 1024U;
constexpr uint16_t kDurationMaximumQ10 = 1280U;
constexpr uint16_t kAmplitudeMinimumDac12 = 2662U;
constexpr uint16_t kAmplitudeNominalDac12 = 3378U;
constexpr uint16_t kAmplitudeMaximumDac12 = 4095U;
constexpr uint16_t kSkewMinimumQ0F12 = 1024U;
constexpr uint16_t kSkewNominalQ0F12 = 1536U;
constexpr uint16_t kSkewMaximumQ0F12 = 2048U;

/** Bounded signed-random interpolation around an explicit nominal point. */
uint16_t variedParameter(uint16_t randomWord,
                         uint16_t textureControl,
                         uint16_t minimumValue,
                         uint16_t nominalValue,
                         uint16_t maximumValue);

/** Convert a duration multiplier in Q10 to reciprocal phase-rate multiplier in Q10. */
uint16_t rateScaleQ10(uint16_t durationQ10);

/** Apply reciprocal duration scale to an Ambient phase increment without 64-bit math. */
uint32_t scaledPhaseIncrement(uint32_t ambientIncrement, uint16_t rateScaleQ10);

/** Compute a Q12 reciprocal for one attack/release segment length. */
uint16_t segmentReciprocalQ12(uint16_t segmentLengthQ0F12);

/** Evaluate the complete gesture using rollover-cached branch reciprocals. */
uint16_t envelopeQ0F12(uint16_t phaseQ0F12,
                       uint16_t skewQ0F12,
                       uint16_t attackReciprocalQ12,
                       uint16_t releaseReciprocalQ12);

/** Apply one cycle's amplitude to the normalized envelope. */
uint16_t applyAmplitude(uint16_t envelopeQ0F12, uint16_t amplitudeDac12);

}  // namespace fmd::breathmath

namespace fmd::fogmath {

constexpr uint8_t kVoiceCount = 4U;
constexpr uint8_t kMinimumOccupancyEighths = 1U;   ///< 0.125 expected voices.
constexpr uint8_t kMaximumOccupancyEighths = 24U; ///< 3.0 expected voices.

/** One fixed-memory smooth cloudlet voice. */
struct Voice {
  uint32_t phase;      ///< Unsigned cloudlet phase accumulator.
  int16_t amplitude;   ///< Signed DAC-domain peak amplitude.
  bool active;         ///< true while the cloudlet is alive.
};

/** Quartic compact cloudlet kernel g(u)=16u^2(1-u)^2 in Q0.12. */
uint16_t kernelQ0F12(uint16_t phaseQ0F12);

/** Map Texture to target mean occupancy in eighths of a voice. */
uint8_t targetOccupancyEighths(uint16_t textureControl);

/** Convert duration and target occupancy to a 32-bit Bernoulli cutoff. */
uint32_t eventCutoffQ0F32(uint32_t ambientIncrement, uint8_t occupancyEighths);

/** Map one random word to symmetric signed cloudlet amplitude. */
int16_t amplitudeFromRandom(uint16_t randomWord);

/** Advance one voice; completed voices are deactivated. */
int16_t voiceContributionAndAdvance(Voice& voice, uint32_t phaseIncrement);

/** Project accumulated signed cloud contributions around midpoint with saturation. */
uint16_t projectToDac12(int32_t signedContribution);

}  // namespace fmd::fogmath

#endif  // FMD_DOMAIN_AMBIENT_ALGORITHM_MATH_H
