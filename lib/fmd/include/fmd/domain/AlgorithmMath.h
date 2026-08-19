/**
 * @file AlgorithmMath.h
 * Declares pure mathematical primitives used by Drift algorithms and host tests.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_DOMAIN_ALGORITHM_MATH_H
#define FMD_DOMAIN_ALGORITHM_MATH_H

#include <stdint.h>

#include "fmd/ports/ReferenceTables.h"

namespace fmd::perlinmath {

/**
 * @brief Evaluate Perlin's canonical quintic fade function in unsigned Q0.16.
 * @param phaseQ0F16 Segment phase in the complete unsigned Q0.16 domain.
 * @return Monotonic quintic fade weight in unsigned Q0.16.
 */
uint16_t fadeQ0F16(uint16_t phaseQ0F16);

/**
 * @brief Map a random word onto the finite signed gradient set used by Drift.
 * @param randomValue 16-bit pseudo-random input word.
 * @return Signed Q1.15 gradient chosen from the 16-value finite set.
 */
int16_t gradientFromRandom(uint16_t randomValue);

/**
 * @brief Evaluate one one-dimensional gradient-noise segment in signed Q1.15.
 * @param phaseQ0F16 Position inside the current segment in unsigned Q0.16.
 * @param startGradientQ1F15 Gradient at the left lattice point in signed Q1.15.
 * @param endGradientQ1F15 Gradient at the right lattice point in signed Q1.15.
 * @return Interpolated signed Q1.15 gradient-noise value.
 */
int16_t segmentQ1F15(uint16_t phaseQ0F16,
                     int16_t startGradientQ1F15,
                     int16_t endGradientQ1F15);

/**
 * @brief Add a delta to a 32-bit phase accumulator and report unsigned wraparound.
 * @param phaseAccumulator Current phase accumulator.
 * @param phaseIncrement Increment to add.
 * @param rollover Set to true when unsigned addition wraps through zero.
 * @return Advanced phase accumulator with overshoot preserved.
 */
uint32_t advancePhase(uint32_t phaseAccumulator,
                      uint32_t phaseIncrement,
                      bool& rollover);

}  // namespace fmd::perlinmath

namespace fmd::brownianmath {

/// Target values below 1/5 full scale receive a small upward centering bias.
constexpr uint16_t kCenteringMargin = 5U;
/// Centering bias strength relative to the movement-event cutoff.
constexpr uint16_t kCenteringStrength = 64U;
/// Minimum first-order smoothing coefficient, approximately 1/4096 in Q0.16.
constexpr uint16_t kMinAlphaQ0F16 = static_cast<uint16_t>(0xFFFFU / 4096U);
/// Maximum first-order smoothing coefficient, approximately 1/8 in Q0.16.
constexpr uint16_t kMaxAlphaQ0F16 = static_cast<uint16_t>(0xFFFFU / 8U);

/**
 * @brief Map combined Speed to Brownian target-step magnitude.
 * @param speedControl Combined Speed value in the saturated 10-bit control domain.
 * @return Positive target-step magnitude in the internal 16-bit state domain.
 */
uint16_t movementStepSize(uint16_t speedControl);

/**
 * @brief Map combined Speed to the probability threshold for a movement event.
 * @param speedControl Combined Speed value in the saturated 10-bit control domain.
 * @return 16-bit random cutoff below which a movement event occurs.
 */
uint16_t movementEventCutoff(uint16_t speedControl);

/**
 * @brief Derive the conditional cutoff separating downward from upward movement.
 * @param targetValue Current random-walk target in the full 16-bit state domain.
 * @param movementCutoff Movement-event cutoff returned by movementEventCutoff().
 * @return Conditional random cutoff that includes the centre-restoring bias.
 */
uint16_t upwardDirectionCutoff(uint16_t targetValue, uint16_t movementCutoff);

/**
 * @brief Advance the bounded Brownian target by at most one random-walk step.
 * @param targetValue Current target in the full 16-bit state domain.
 * @param randomValue 16-bit pseudo-random word used for event and direction decisions.
 * @param speedControl Combined Speed value in the saturated 10-bit control domain.
 * @return Updated target, saturated to the 16-bit state boundaries.
 */
uint16_t nextTargetValue(uint16_t targetValue,
                         uint16_t randomValue,
                         uint16_t speedControl);

/**
 * @brief Map combined Texture to the Brownian first-order smoothing coefficient.
 * @param textureControl Combined Texture value in the saturated 10-bit control domain.
 * @return Smoothing coefficient spanning kMinAlphaQ0F16..kMaxAlphaQ0F16.
 */
uint16_t smoothingAlphaQ0F16(uint16_t textureControl);

/**
 * @brief Move a smoothed state toward its target while retaining fractional motion.
 * @param targetValue Destination in the internal 16-bit Brownian state domain.
 * @param alphaQ0F16 First-order smoothing coefficient.
 * @param currentValue In/out current smoothed state.
 * @param fractionalResidualQ0F16 In/out sub-code residual carried between samples.
 * @param residualDirection In/out sign of the residual; reset on direction reversal.
 */
void smoothTowardTarget(uint16_t targetValue,
                        uint16_t alphaQ0F16,
                        uint16_t& currentValue,
                        uint16_t& fractionalResidualQ0F16,
                        int8_t& residualDirection);

}  // namespace fmd::brownianmath

namespace fmd::beziermath {

/**
 * @brief Map Texture knob/CV to the amplitude of Bézier segment-speed variation.
 * @param textureKnobAdc Texture potentiometer ADC code, defensively clamped to 0..1023.
 * @param textureCvAdc Texture CV ADC code, defensively clamped to 0..1023.
 * @return Unsigned Q1.15 scale applied to the random signed speed offset.
 */
uint16_t speedVariationScaleQ1F15(uint16_t textureKnobAdc, uint16_t textureCvAdc);

/**
 * @brief Evaluate the smoothstep curve, y = 3x^2 - 2x^3, in Q4.12.
 * @param phaseQ4F12 Normalised segment phase where 0 is start and 4096 is end.
 * @return Monotonic smoothstep value in the range 0..4096.
 */
uint16_t smoothCurveQ4F12(uint16_t phaseQ4F12);

/**
 * @brief Evaluate the complementary Drift curve, y = 2x^3 - 3x^2 + 2x, in Q4.12.
 * @param phaseQ4F12 Normalised segment phase where 0 is start and 4096 is end.
 * @return Monotonic complementary curve value in the range 0..4096.
 */
uint16_t reverseCurveQ4F12(uint16_t phaseQ4F12);

/**
 * @brief Map the Texture knob linearly from 10-bit ADC to a curve-morph blend weight.
 * @param textureKnobAdc Texture potentiometer ADC code, defensively clamped to 0..1023.
 * @return Unsigned Q0.16 blend weight from reverse curve toward smoothstep.
 */
uint16_t textureBlendQ0F16(uint16_t textureKnobAdc);

/**
 * @brief Morph continuously between reverse and smooth cubic curve families.
 * @param phaseQ4F12 Normalised segment phase where 0 is start and 4096 is end.
 * @param textureKnobAdc Texture potentiometer ADC code controlling the curve family.
 * @return Morphed monotonic curve value in Q4.12.
 */
uint16_t morphCurveQ4F12(uint16_t phaseQ4F12, uint16_t textureKnobAdc);

/**
 * @brief Interpolate between segment endpoints using the Texture-selected curve.
 * @param phaseQ4F12 Normalised segment phase where 0 is start and 4096 is end.
 * @param startValueQ4F12 Segment start value in the 12-bit output domain.
 * @param endValueQ4F12 Segment destination in the 12-bit output domain.
 * @param textureKnobAdc Texture potentiometer ADC code controlling the curve family.
 * @return Interpolated 12-bit-domain value without overshoot.
 */
uint16_t interpolateQ4F12(uint16_t phaseQ4F12,
                          uint16_t startValueQ4F12,
                          uint16_t endValueQ4F12,
                          uint16_t textureKnobAdc);

/**
 * @brief Sample the 257-point triangular inverse-CDF table with linear interpolation.
 * @param uniformSample15 Uniform 15-bit sample in the range 0..32767.
 * @param referenceTables Read-only provider for the 257-point inverse-CDF table.
 * @return Signed Q1.15 sample following the symmetric triangular distribution.
 */
int16_t triangularIcdfQ1F15(uint16_t uniformSample15,
                            const IReferenceTables& referenceTables);

/**
 * @brief Advance Bézier segment phase and preserve overshoot across wraparound.
 * @param phaseAccumulator Current 32-bit segment phase.
 * @param phaseIncrement Increment to add for this sample.
 * @param rollover Set to true when unsigned addition wraps through zero.
 * @return Advanced phase accumulator with overshoot preserved.
 */
uint32_t advancePhase(uint32_t phaseAccumulator,
                      uint32_t phaseIncrement,
                      bool& rollover);

}  // namespace fmd::beziermath

namespace fmd::lfomath {

/**
 * @brief Map combined Texture to the LFO apex phase.
 * @param textureControl Combined Texture value, defensively clamped to 0..1023.
 * @return Apex position in unsigned Q0.16, including exact sawtooth endpoints.
 */
uint16_t apexFromTexture(uint16_t textureControl);

/**
 * @brief Evaluate the skewed triangle / saw waveform in unsigned Q0.16.
 * @param phaseQ0F16 Current cycle phase in unsigned Q0.16.
 * @param apexPhaseQ0F16 Texture-selected apex position in unsigned Q0.16.
 * @return Waveform level in unsigned Q0.16.
 */
uint16_t waveformQ0F16(uint16_t phaseQ0F16, uint16_t apexPhaseQ0F16);

/**
 * @brief Evaluate the skewed triangle / saw waveform as a 12-bit DAC code.
 * @param phaseQ0F16 Current cycle phase in unsigned Q0.16.
 * @param apexPhaseQ0F16 Texture-selected apex position in unsigned Q0.16.
 * @return 12-bit waveform code in the range 0..4095.
 */
uint16_t waveform12(uint16_t phaseQ0F16, uint16_t apexPhaseQ0F16);

/**
 * @brief Remap phase after an apex change while minimising output discontinuity.
 * @param phaseQ0F16 Phase on the waveform using the previous apex.
 * @param previousApexQ0F16 Apex used to interpret the current phase/output.
 * @param requestedApexQ0F16 New Texture-selected apex to apply immediately.
 * @return New phase on the requested waveform that best preserves current output.
 */
uint16_t remapPhasePreservingOutput(uint16_t phaseQ0F16,
                                    uint16_t previousApexQ0F16,
                                    uint16_t requestedApexQ0F16);

/**
 * @brief Advance LFO phase and preserve overshoot across wraparound.
 * @param phaseAccumulator Current 32-bit cycle phase.
 * @param phaseIncrement Increment to add for this sample.
 * @param rollover Set to true when unsigned addition wraps through zero.
 * @return Advanced phase accumulator with overshoot preserved.
 */
uint32_t advancePhase(uint32_t phaseAccumulator,
                      uint32_t phaseIncrement,
                      bool& rollover);

}  // namespace fmd::lfomath

#endif  // FMD_DOMAIN_ALGORITHM_MATH_H
