/**
 * @file AmbientAlgorithmMath.h
 * Declares fixed-point mathematical primitives shared by the Ambient algorithm bank.
 *
 * @details
 * Functions in this header are deterministic and allocation-free. Units and Q
 * formats are part of the test contract because most helpers sit directly on the
 * 2.5 kHz scheduler hot path. Unless stated otherwise, control inputs are 10-bit
 * ADC-domain values and DAC results are inclusive 12-bit values 0..4095.
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

/**
 * @brief Divide the established Drift phase rate by sixteen for Ambient macro timing.
 * @param basePhaseIncrement Unsigned 32-bit phase increment from the common frequency map.
 * @return Rounded base increment divided by sixteen.
 */
uint32_t ambientPhaseIncrement(uint32_t basePhaseIncrement);

/**
 * @brief Reproduce the saturated effective Speed control used by the common frequency map.
 * @param speedKnobAdc Speed knob ADC code; clamped to 0..1023.
 * @param speedCvAdc Speed-CV ADC code; clamped to 0..1023.
 * @return Combined mapped control in the common frequency-map domain.
 */
uint16_t mappedSpeedControl(uint16_t speedKnobAdc, uint16_t speedCvAdc);

/**
 * @brief Quantize mapped Speed into the 307-entry Anchor compensation-table domain.
 * @param speedKnobAdc Speed knob ADC code; clamped through mappedSpeedControl().
 * @param speedCvAdc Speed-CV ADC code; clamped through mappedSpeedControl().
 * @return Table index in the inclusive range 0..306.
 */
uint16_t anchorSpeedBucket(uint16_t speedKnobAdc, uint16_t speedCvAdc);

/**
 * @brief Evaluate cubic smoothstep S(x)=3x^2-2x^3 in unsigned Q0.12.
 * @param xQ0F12 Input in Q0.12; values above 4096 are clamped to the endpoint.
 * @return Smoothstep result in the inclusive range 0..4096.
 */
uint16_t smoothstepQ0F12(uint16_t xQ0F12);

}  // namespace fmd::ambientmath

namespace fmd::currentmath {

/** @brief Constant-sum Current mixing weights with denominator 1024. */
struct Weights {
  uint16_t primary;    ///< Fundamental-current weight.
  uint16_t secondary;  ///< Approximate sqrt(2)-rate current weight.
  uint16_t tertiary;   ///< Approximate golden-ratio-rate current weight.
};

/**
 * @brief Approximate sqrt(2) rate multiplication with the rational factor 362/256.
 * @param baseIncrement Base unsigned 32-bit phase increment.
 * @return Rounded approximately sqrt(2)-scaled increment.
 */
uint32_t sqrt2Increment(uint32_t baseIncrement);

/**
 * @brief Approximate golden-ratio rate multiplication with the rational factor 414/256.
 * @param baseIncrement Base unsigned 32-bit phase increment.
 * @return Rounded approximately phi-scaled increment.
 */
uint32_t phiIncrement(uint32_t baseIncrement);

/**
 * @brief Evaluate a smooth bipolar triangle from a 32-bit phase accumulator.
 * @param phase Unsigned 32-bit wraparound phase.
 * @return Signed value in approximately -4096..4096, represented in the bank's Q3.12 convention.
 */
int16_t softTriangleQ3F12(uint32_t phase);

/**
 * @brief Map Texture to the documented constant-sum Current weights.
 * @param textureControl Combined Texture control; clamped to 0..1023.
 * @return Three weights that sum exactly to 1024.
 */
Weights weights(uint16_t textureControl);

/**
 * @brief Mix three signed Current components and project around DAC midpoint.
 * @param primaryQ3F12 Fundamental signed component.
 * @param secondaryQ3F12 Approximate sqrt(2)-rate signed component.
 * @param tertiaryQ3F12 Approximate phi-rate signed component.
 * @param weights Constant-sum denominator-1024 mixing weights.
 * @return Saturated 12-bit DAC code in the inclusive range 0..4095.
 */
uint16_t mixToDac12(int16_t primaryQ3F12,
                    int16_t secondaryQ3F12,
                    int16_t tertiaryQ3F12,
                    const Weights& weights);

}  // namespace fmd::currentmath

namespace fmd::anchormath {

/** @brief Maximum target stationary spread, 0.30 represented in unsigned Q1.15. */
constexpr uint16_t kMaximumSpreadQ1F15 = 9830U;

/**
 * @brief Map Texture to target stationary spread.
 * @param textureControl Combined Texture control; clamped to 0..1023.
 * @return Unsigned spread in Q1.15, from zero through kMaximumSpreadQ1F15.
 */
uint16_t spreadQ1F15(uint16_t textureControl);

/**
 * @brief Convert Ambient phase rate to per-sample mean-reversion alpha.
 * @param ambientPhaseIncrement Unsigned 32-bit Ambient phase increment.
 * @return Positive Q0.24 alpha; never returns zero so very slow motion still reverts.
 */
uint32_t reversionAlphaQ0F24(uint32_t ambientPhaseIncrement);

/**
 * @brief Move one signed Q1.15 state toward zero while preserving fractional motion.
 * @param stateQ1F15 Current signed Q1.15 state.
 * @param alphaQ0F24 Positive Q0.24 reversion coefficient.
 * @param fractionalResidualQ0F24 In/out residual that carries sub-LSB movement between samples.
 * @param residualDirection In/out sign associated with the residual; reset when the state crosses sides.
 * @return New signed Q1.15 state no farther from zero than the input.
 */
int16_t revertTowardZero(int16_t stateQ1F15,
                         uint32_t alphaQ0F24,
                         uint32_t& fractionalResidualQ0F24,
                         int8_t& residualDirection);

/**
 * @brief Scale one triangular innovation by Speed compensation and Texture spread.
 * @param triangularSampleQ1F15 Signed triangular sample in Q1.15.
 * @param innovationGainQ1F15 Unsigned speed-compensation gain in Q1.15.
 * @param spreadQ1F15 Unsigned target spread in Q1.15.
 * @return Saturated signed innovation in Q1.15.
 */
int16_t scaledInnovationQ1F15(int16_t triangularSampleQ1F15,
                             uint16_t innovationGainQ1F15,
                             uint16_t spreadQ1F15);

/**
 * @brief Add one innovation to the Anchor state with signed-16 saturation.
 * @param stateQ1F15 Current signed Q1.15 state.
 * @param innovationQ1F15 Signed Q1.15 innovation.
 * @return Saturated signed Q1.15 result.
 */
int16_t addInnovationSaturating(int16_t stateQ1F15, int16_t innovationQ1F15);

/**
 * @brief Project signed Q1.15 state around the 12-bit DAC midpoint.
 * @param stateQ1F15 Signed process state.
 * @return Unipolar DAC-domain projection in 0..4095.
 */
uint16_t projectToDac12(int16_t stateQ1F15);

}  // namespace fmd::anchormath

namespace fmd::breathmath {

constexpr uint16_t kDurationMinimumQ10 = 768U;       ///< 0.75x nominal cycle duration.
constexpr uint16_t kDurationNominalQ10 = 1024U;      ///< 1.00x nominal cycle duration.
constexpr uint16_t kDurationMaximumQ10 = 1280U;      ///< 1.25x nominal cycle duration.
constexpr uint16_t kAmplitudeMinimumDac12 = 2662U;   ///< 0.65 of full-scale DAC amplitude.
constexpr uint16_t kAmplitudeNominalDac12 = 3378U;   ///< Nominal Breath peak amplitude.
constexpr uint16_t kAmplitudeMaximumDac12 = 4095U;   ///< Full-scale Breath peak amplitude.
constexpr uint16_t kSkewMinimumQ0F12 = 1024U;        ///< Peak at one quarter of the cycle.
constexpr uint16_t kSkewNominalQ0F12 = 1536U;        ///< Nominal peak at 3/8 of the cycle.
constexpr uint16_t kSkewMaximumQ0F12 = 2048U;        ///< Peak at one half of the cycle.

/**
 * @brief Interpolate a bounded random parameter around an explicit nominal point.
 * @param randomWord Uniform 16-bit PRNG word.
 * @param textureControl Variation depth; clamped to 0..1023.
 * @param minimumValue Lower endpoint.
 * @param nominalValue Value returned at Texture zero.
 * @param maximumValue Upper endpoint.
 * @return Bounded value between minimumValue and maximumValue.
 */
uint16_t variedParameter(uint16_t randomWord,
                         uint16_t textureControl,
                         uint16_t minimumValue,
                         uint16_t nominalValue,
                         uint16_t maximumValue);

/**
 * @brief Convert a duration multiplier to its reciprocal phase-rate multiplier.
 * @param durationQ10 Duration multiplier in Q10; defensively clamped to 768..1280.
 * @return Reciprocal multiplier in Q10, with 1024 representing unity rate.
 */
uint16_t rateScaleQ10(uint16_t durationQ10);

/**
 * @brief Apply reciprocal duration scale to an Ambient phase increment.
 * @param ambientIncrement Unsigned Ambient phase increment.
 * @param rateScaleQ10 Q10 rate multiplier, normally produced by rateScaleQ10().
 * @return Scaled phase increment using 32-bit arithmetic only.
 */
uint32_t scaledPhaseIncrement(uint32_t ambientIncrement, uint16_t rateScaleQ10);

/**
 * @brief Compute a Q12 reciprocal for one attack/release segment length.
 * @param segmentLengthQ0F12 Segment length in Q0.12 phase units.
 * @return Rounded Q12 reciprocal; zero when segment length is zero.
 */
uint16_t segmentReciprocalQ12(uint16_t segmentLengthQ0F12);

/**
 * @brief Evaluate the complete asymmetric smoothstep Breath envelope.
 * @param phaseQ0F12 Current cycle phase in Q0.12; values above 4095 are clamped.
 * @param skewQ0F12 Latched peak position; clamped to the qualified skew range.
 * @param attackReciprocalQ12 Cached reciprocal of the attack segment length.
 * @param releaseReciprocalQ12 Cached reciprocal of the release segment length.
 * @return Normalized envelope in the inclusive Q0.12 range 0..4096.
 */
uint16_t envelopeQ0F12(uint16_t phaseQ0F12,
                       uint16_t skewQ0F12,
                       uint16_t attackReciprocalQ12,
                       uint16_t releaseReciprocalQ12);

/**
 * @brief Apply one cycle's peak amplitude to a normalized envelope.
 * @param envelopeQ0F12 Normalized Q0.12 envelope.
 * @param amplitudeDac12 Peak amplitude in 12-bit DAC units.
 * @return Scaled 12-bit DAC-domain value.
 */
uint16_t applyAmplitude(uint16_t envelopeQ0F12, uint16_t amplitudeDac12);

}  // namespace fmd::breathmath

namespace fmd::fogmath {

constexpr uint8_t kVoiceCount = 4U;                  ///< Fixed cloudlet voice pool size.
constexpr uint8_t kMinimumOccupancyEighths = 1U;    ///< 0.125 expected active voices.
constexpr uint8_t kMaximumOccupancyEighths = 24U;   ///< 3.0 expected active voices.

/** @brief Fixed-memory state of one smooth Fog cloudlet. */
struct Voice {
  uint32_t phase;     ///< Unsigned 32-bit cloudlet phase accumulator.
  int16_t amplitude;  ///< Signed DAC-domain peak amplitude.
  bool active;        ///< True while the cloudlet is alive.
};

/**
 * @brief Evaluate quartic compact kernel g(u)=16u^2(1-u)^2.
 * @param phaseQ0F12 Cloudlet phase in Q0.12.
 * @return Kernel amplitude in the inclusive Q0.12 range 0..4096.
 */
uint16_t kernelQ0F12(uint16_t phaseQ0F12);

/**
 * @brief Map Texture to target mean active-voice occupancy.
 * @param textureControl Combined Texture control; clamped to 0..1023.
 * @return Expected occupancy expressed in eighths of a voice, from 1 through 24.
 */
uint8_t targetOccupancyEighths(uint16_t textureControl);

/**
 * @brief Convert cloudlet duration and target occupancy to a Bernoulli cutoff.
 * @param ambientIncrement Unsigned phase increment defining cloudlet lifetime.
 * @param occupancyEighths Target mean occupancy in eighths of a voice.
 * @return Unsigned 32-bit cutoff compared directly against a uniform PRNG word.
 */
uint32_t eventCutoffQ0F32(uint32_t ambientIncrement, uint8_t occupancyEighths);

/**
 * @brief Map one random word to a symmetric signed cloudlet peak amplitude.
 * @param randomWord Uniform 16-bit PRNG word.
 * @return Signed DAC-domain amplitude with project-defined minimum magnitude.
 */
int16_t amplitudeFromRandom(uint16_t randomWord);

/**
 * @brief Accumulate one voice's contribution and advance/deactivate it in place.
 * @param voice Mutable fixed-memory voice state.
 * @param phaseIncrement Cloudlet phase increment for this scheduler sample.
 * @return Signed DAC-domain contribution for the pre-advance phase.
 */
int16_t voiceContributionAndAdvance(Voice& voice, uint32_t phaseIncrement);

/**
 * @brief Project summed bipolar cloudlets around DAC midpoint with saturation.
 * @param signedContribution Sum of all active signed DAC-domain voice contributions.
 * @return Saturated 12-bit DAC code in the inclusive range 0..4095.
 */
uint16_t projectToDac12(int32_t signedContribution);

}  // namespace fmd::fogmath

#endif  // FMD_DOMAIN_AMBIENT_ALGORITHM_MATH_H
