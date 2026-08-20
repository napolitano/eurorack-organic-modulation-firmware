/**
 * @file ElectronicaAlgorithmMath.h
 * Declares fixed-point primitives shared by the Electronica algorithm bank.
 *
 * @details
 * The bank uses a documented free-running 30..240 BPM tempo mapping and a
 * 2.5 kHz scheduler. Functions are deterministic, allocation-free and expressed
 * in explicit phase/DAC/Q formats so they can be unit-tested independently of
 * the stateful algorithm classes.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_DOMAIN_ELECTRONICA_ALGORITHM_MATH_H
#define FMD_DOMAIN_ELECTRONICA_ALGORITHM_MATH_H

#include <stdint.h>

#include "fmd/ports/ReferenceTables.h"

namespace fmd::electronicamath {

constexpr uint16_t kMinimumTempoBpm = 30U;   ///< Nominal Speed-minimum tempo.
constexpr uint16_t kMaximumTempoBpm = 240U;  ///< Nominal Speed-maximum tempo.

/**
 * @brief Saturate and combine the 10-bit Speed knob/CV controls.
 * @param speedKnobAdc Speed knob ADC code.
 * @param speedCvAdc Speed-CV ADC code.
 * @return Saturated project-defined Electronica speed-control code.
 */
uint16_t speedControl(uint16_t speedKnobAdc, uint16_t speedCvAdc);

/**
 * @brief Map Electronica Speed to the documented 30*2^(3u) quarter-note phase rate.
 * @param referenceTables Lookup-table provider used by the exponential mapping.
 * @param speedKnobAdc Speed knob ADC code; defensively clamped.
 * @param speedCvAdc Speed-CV ADC code; defensively clamped.
 * @return Q0.32-style unsigned quarter-note phase increment per scheduler sample.
 */
uint32_t quarterNotePhaseIncrement(const IReferenceTables& referenceTables,
                                   uint16_t speedKnobAdc,
                                   uint16_t speedCvAdc);

/**
 * @brief Return the sixteenth-note grid rate, exactly four times quarter rate.
 * @param referenceTables Lookup-table provider for the base tempo mapping.
 * @param speedKnobAdc Speed knob ADC code.
 * @param speedCvAdc Speed-CV ADC code.
 * @return Unsigned phase increment for one sixteenth-note cycle.
 */
uint32_t sixteenthNotePhaseIncrement(const IReferenceTables& referenceTables,
                                     uint16_t speedKnobAdc,
                                     uint16_t speedCvAdc);

/**
 * @brief Return the phase rate of one two-sixteenth Shuffle pair.
 * @param referenceTables Lookup-table provider for the base tempo mapping.
 * @param speedKnobAdc Speed knob ADC code.
 * @param speedCvAdc Speed-CV ADC code.
 * @return Unsigned phase increment for a pair lasting exactly two sixteenths.
 */
uint32_t shufflePairPhaseIncrement(const IReferenceTables& referenceTables,
                                   uint16_t speedKnobAdc,
                                   uint16_t speedCvAdc);

/**
 * @brief Map saturated Texture to Q0.12 including exact endpoints.
 * @param textureControl Combined Texture control; clamped to 0..1023.
 * @return 0..4096 in unsigned Q0.12.
 */
uint16_t textureQ0F12(uint16_t textureControl);

/**
 * @brief Evaluate cubic smoothstep S(x)=3x^2-2x^3.
 * @param xQ0F12 Q0.12 input; clamped to 0..4096.
 * @return Q0.12 result in 0..4096.
 */
uint16_t smoothstepQ0F12(uint16_t xQ0F12);

/**
 * @brief Evaluate complementary decay D(x)=1-S(x).
 * @param xQ0F12 Q0.12 input; clamped by smoothstepQ0F12().
 * @return Q0.12 decay result in 0..4096.
 */
uint16_t decayQ0F12(uint16_t xQ0F12);

/**
 * @brief Scale a 12-bit peak by a Q0.12 envelope with exact endpoints.
 * @param peakDac12 Peak DAC value; caller supplies a 12-bit value.
 * @param envelopeQ0F12 Normalized Q0.12 envelope.
 * @return Scaled DAC-domain result.
 */
uint16_t scaleDac12(uint16_t peakDac12, uint16_t envelopeQ0F12);

}  // namespace fmd::electronicamath

namespace fmd::pumpmath {

constexpr uint16_t kRecoveryMinimumQ0F16 = 16384U;  ///< Recovery completes after 1/4 beat.
constexpr uint16_t kRecoveryMaximumQ0F16 = 61440U;  ///< Recovery completes after 15/16 beat.

/**
 * @brief Map Texture to the recovery endpoint within the beat.
 * @param textureControl Combined Texture control; clamped to 0..1023.
 * @return Endpoint in unsigned Q0.16 between 1/4 and 15/16 beat.
 */
uint16_t recoveryEndpointQ0F16(uint16_t textureControl);

/**
 * @brief Cache 2^28/endpoint for division-free per-sample normalization.
 * @param endpointQ0F16 Non-zero recovery endpoint in Q0.16.
 * @return Rounded reciprocal in the implementation's Q28 scaling.
 */
uint16_t recoveryReciprocalQ28(uint16_t endpointQ0F16);

/**
 * @brief Normalize beat phase below the recovery endpoint to Q0.12.
 * @param phaseQ0F16 Current beat phase in Q0.16.
 * @param endpointQ0F16 Latched recovery endpoint in Q0.16.
 * @param reciprocalQ28 Cached reciprocal produced by recoveryReciprocalQ28().
 * @return Normalized recovery progress in 0..4096 Q0.12.
 */
uint16_t recoveryProgressQ0F12(uint16_t phaseQ0F16,
                              uint16_t endpointQ0F16,
                              uint16_t reciprocalQ28);

/**
 * @brief Evaluate the complete full-scale Pump recovery contour.
 * @param phaseQ0F16 Current beat phase in Q0.16.
 * @param endpointQ0F16 Latched recovery endpoint.
 * @param reciprocalQ28 Cached normalization reciprocal.
 * @return 12-bit DAC code: smooth recovery before endpoint, then full scale.
 */
uint16_t outputDac12(uint16_t phaseQ0F16,
                     uint16_t endpointQ0F16,
                     uint16_t reciprocalQ28);

}  // namespace fmd::pumpmath

namespace fmd::acidmath {

/**
 * @brief Evaluate project-defined permutation q_n=(5n+3) mod 16.
 * @param stepIndex Step index; only the low four bits are relevant.
 * @return Permutation code 0..15.
 */
uint8_t permutationCode(uint8_t stepIndex);

/**
 * @brief Project a permuted step to the documented base DAC range.
 * @param stepIndex Step index; wrapped by permutationCode().
 * @return Base target in the 1024..2944 DAC range.
 */
uint16_t baseTargetDac12(uint8_t stepIndex);

/**
 * @brief Test the project-defined accent predicate.
 * @param stepIndex Step index; wrapped to the 16-step phrase.
 * @return true when the current step carries an accent.
 */
bool isAccentStep(uint8_t stepIndex);

/**
 * @brief Test the project-defined slide predicate.
 * @param stepIndex Step index; wrapped to the 16-step phrase.
 * @return true when the current step slides from the previous target.
 */
bool isSlideStep(uint8_t stepIndex);

/**
 * @brief Evaluate Texture-controlled interpolation on a marked slide step.
 * @param previousTargetDac12 Previous base step target.
 * @param currentTargetDac12 Current base step target.
 * @param phaseQ0F12 Current sixteenth phase in Q0.12.
 * @param textureQ0F12 Texture depth in Q0.12.
 * @return Interpolated DAC-domain target.
 */
uint16_t slideContourDac12(uint16_t previousTargetDac12,
                           uint16_t currentTargetDac12,
                           uint16_t phaseQ0F12,
                           uint16_t textureQ0F12);

/**
 * @brief Evaluate the decaying Texture-controlled accent contribution.
 * @param phaseQ0F12 Current sixteenth phase in Q0.12.
 * @param textureQ0F12 Accent depth in Q0.12.
 * @return Accent contribution from zero through the documented maximum 768 DAC codes.
 */
uint16_t accentContributionDac12(uint16_t phaseQ0F12,
                                 uint16_t textureQ0F12);

/**
 * @brief Add base and accent with one final 12-bit saturation.
 * @param baseDac12 Base or slide-interpolated value.
 * @param accentDac12 Non-negative accent contribution.
 * @return Saturated 12-bit sum in 0..4095.
 */
uint16_t addAccentSaturating(uint16_t baseDac12, uint16_t accentDac12);

}  // namespace fmd::acidmath

namespace fmd::shufflemath {

constexpr uint16_t kStraightRatioQ0F16 = 32768U;  ///< Second onset at 1/2 pair.
constexpr uint16_t kMaximumRatioQ0F16 = 49152U;   ///< Second onset at 3/4 pair (3:1).

/**
 * @brief Map Texture to the second-onset pair position.
 * @param textureControl Combined Texture control; clamped to 0..1023.
 * @return Q0.16 ratio from 1/2 through 3/4 of the pair.
 */
uint16_t secondOnsetRatioQ0F16(uint16_t textureControl);

/**
 * @brief Convert a Q0.16 onset ratio to a 32-bit pair-phase threshold.
 * @param ratioQ0F16 Second-onset ratio in Q0.16.
 * @return Corresponding unsigned 32-bit phase threshold.
 */
uint32_t secondOnsetThreshold(uint16_t ratioQ0F16);

/**
 * @brief Derive the short decay rate from the current Shuffle pair rate.
 * @param pairPhaseIncrement Unsigned increment for the complete two-sixteenth pair.
 * @return Envelope increment yielding the documented one-eighth-pair decay duration.
 */
uint32_t envelopePhaseIncrement(uint32_t pairPhaseIncrement);

/**
 * @brief Evaluate the full-scale short decay envelope.
 * @param envelopePhase Unsigned 32-bit envelope phase.
 * @return 12-bit full-scale decay value.
 */
uint16_t decayOutputDac12(uint32_t envelopePhase);

}  // namespace fmd::shufflemath

namespace fmd::polymetermath {

constexpr uint16_t kBaseLevelDac12 = 1024U;          ///< Non-start step peak.
constexpr uint16_t kPrimaryAccentDac12 = 1535U;      ///< Additional primary-start level.
constexpr uint16_t kSecondaryAccentDac12 = 1536U;    ///< Additional secondary-start level.

/**
 * @brief Map Texture regions exactly to secondary meter lengths 3, 5, 7 and 9.
 * @param textureControl Combined Texture control; clamped to 0..1023.
 * @return One of the exact meter lengths 3, 5, 7 or 9.
 */
uint8_t secondaryMeterLength(uint16_t textureControl);

/**
 * @brief Compute base/primary/secondary/coincidence peak amplitude.
 * @param primaryStart true when the 4-step anchor starts on this grid step.
 * @param secondaryStart true when the selected odd meter starts on this grid step.
 * @return Exact 12-bit project-defined peak amplitude for the combination.
 */
uint16_t stepAmplitudeDac12(bool primaryStart, bool secondaryStart);

/**
 * @brief Derive a decay rate lasting exactly half of one sixteenth-note step.
 * @param sixteenthPhaseIncrement Unsigned increment for one grid step.
 * @return Unsigned short-envelope phase increment.
 */
uint32_t envelopePhaseIncrement(uint32_t sixteenthPhaseIncrement);

/**
 * @brief Evaluate a short decay at the supplied peak amplitude.
 * @param envelopePhase Unsigned 32-bit decay-envelope phase.
 * @param peakDac12 Peak amplitude for the current meter-start combination.
 * @return 12-bit decay value.
 */
uint16_t decayOutputDac12(uint32_t envelopePhase, uint16_t peakDac12);

/**
 * @brief Compute exact recurrence length for 4 against the selected odd meter.
 * @param secondaryMeterLength Secondary meter length, normally one of 3, 5, 7 or 9.
 * @return Least common multiple in sixteenth-note steps.
 */
uint8_t recurrenceSteps(uint8_t secondaryMeterLength);

}  // namespace fmd::polymetermath

#endif  // FMD_DOMAIN_ELECTRONICA_ALGORITHM_MATH_H
