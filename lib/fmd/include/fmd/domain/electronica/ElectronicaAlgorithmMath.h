/**
 * @file ElectronicaAlgorithmMath.h
 * Declares pure fixed-point primitives shared by the Electronica algorithm bank.
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

/** Minimum nominal Electronica tempo in BPM. */
constexpr uint16_t kMinimumTempoBpm = 30U;
/** Maximum nominal Electronica tempo in BPM. */
constexpr uint16_t kMaximumTempoBpm = 240U;

/** Saturating 10-bit Speed knob + CV control used by the Electronica bank. */
uint16_t speedControl(uint16_t speedKnobAdc, uint16_t speedCvAdc);

/** Map Electronica Speed to the documented 30*2^(3u) quarter-note phase rate. */
uint32_t quarterNotePhaseIncrement(const IReferenceTables& referenceTables,
                                   uint16_t speedKnobAdc,
                                   uint16_t speedCvAdc);

/** Return the sixteenth-note grid rate, exactly four times the quarter-note rate. */
uint32_t sixteenthNotePhaseIncrement(const IReferenceTables& referenceTables,
                                     uint16_t speedKnobAdc,
                                     uint16_t speedCvAdc);

/** Return the phase rate of one two-sixteenth Shuffle pair. */
uint32_t shufflePairPhaseIncrement(const IReferenceTables& referenceTables,
                                   uint16_t speedKnobAdc,
                                   uint16_t speedCvAdc);

/** Map saturated Texture to Q0.12, including exact 0 and 4096 endpoints. */
uint16_t textureQ0F12(uint16_t textureControl);

/** Evaluate cubic smoothstep S(x)=3x^2-2x^3 in Q0.12. */
uint16_t smoothstepQ0F12(uint16_t xQ0F12);

/** Evaluate complementary decay D(x)=1-S(x) in Q0.12. */
uint16_t decayQ0F12(uint16_t xQ0F12);

/** Scale a 12-bit peak value by a Q0.12 envelope, preserving exact endpoints. */
uint16_t scaleDac12(uint16_t peakDac12, uint16_t envelopeQ0F12);

}  // namespace fmd::electronicamath

namespace fmd::pumpmath {

constexpr uint16_t kRecoveryMinimumQ0F16 = 16384U;  ///< 1/4 beat.
constexpr uint16_t kRecoveryMaximumQ0F16 = 61440U;  ///< 15/16 beat.

/** Map Texture to the recovery endpoint in Q0.16. */
uint16_t recoveryEndpointQ0F16(uint16_t textureControl);

/** Cache 2^28 / endpoint for division-free per-sample phase normalization. */
uint16_t recoveryReciprocalQ28(uint16_t endpointQ0F16);

/** Normalize a beat phase below the recovery endpoint to Q0.12. */
uint16_t recoveryProgressQ0F12(uint16_t phaseQ0F16,
                              uint16_t endpointQ0F16,
                              uint16_t reciprocalQ28);

/** Evaluate the complete Pump output for one beat phase. */
uint16_t outputDac12(uint16_t phaseQ0F16,
                     uint16_t endpointQ0F16,
                     uint16_t reciprocalQ28);

}  // namespace fmd::pumpmath

namespace fmd::acidmath {

/** Deterministic project-defined level permutation q_n=(5n+3) mod 16. */
uint8_t permutationCode(uint8_t stepIndex);

/** Project the permutation code to the documented 1024..2944 DAC range. */
uint16_t baseTargetDac12(uint8_t stepIndex);

/** Return true when the project-defined accent predicate is active. */
bool isAccentStep(uint8_t stepIndex);

/** Return true when the project-defined slide predicate is active. */
bool isSlideStep(uint8_t stepIndex);

/** Evaluate Texture-controlled slide interpolation for a marked step. */
uint16_t slideContourDac12(uint16_t previousTargetDac12,
                           uint16_t currentTargetDac12,
                           uint16_t phaseQ0F12,
                           uint16_t textureQ0F12);

/** Evaluate the decaying Texture-controlled accent contribution, maximum 768. */
uint16_t accentContributionDac12(uint16_t phaseQ0F12,
                                 uint16_t textureQ0F12);

/** Add base and accent with one final 12-bit saturation. */
uint16_t addAccentSaturating(uint16_t baseDac12, uint16_t accentDac12);

}  // namespace fmd::acidmath

namespace fmd::shufflemath {

constexpr uint16_t kStraightRatioQ0F16 = 32768U;  ///< 1/2 pair.
constexpr uint16_t kMaximumRatioQ0F16 = 49152U;   ///< 3/4 pair.

/** Map Texture to the second-onset pair position, 1/2..3/4 in Q0.16. */
uint16_t secondOnsetRatioQ0F16(uint16_t textureControl);

/** Convert the Q0.16 onset ratio to the 32-bit pair-phase threshold. */
uint32_t secondOnsetThreshold(uint16_t ratioQ0F16);

/** Envelope rate for a decay lasting exactly one eighth of the Shuffle pair. */
uint32_t envelopePhaseIncrement(uint32_t pairPhaseIncrement);

/** Evaluate the full-scale short decay from an envelope phase accumulator. */
uint16_t decayOutputDac12(uint32_t envelopePhase);

}  // namespace fmd::shufflemath

namespace fmd::polymetermath {

constexpr uint16_t kBaseLevelDac12 = 1024U;
constexpr uint16_t kPrimaryAccentDac12 = 1535U;
constexpr uint16_t kSecondaryAccentDac12 = 1536U;

/** Map Texture regions exactly to secondary meter lengths 3, 5, 7 and 9. */
uint8_t secondaryMeterLength(uint16_t textureControl);

/** Compute one of the exact base/primary/secondary/coincidence amplitudes. */
uint16_t stepAmplitudeDac12(bool primaryStart, bool secondaryStart);

/** Envelope rate for a decay lasting exactly half of one sixteenth-note step. */
uint32_t envelopePhaseIncrement(uint32_t sixteenthPhaseIncrement);

/** Evaluate a short decay at the supplied peak amplitude. */
uint16_t decayOutputDac12(uint32_t envelopePhase, uint16_t peakDac12);

/** Exact least-common-multiple helper for the bank's small integer meters. */
uint8_t recurrenceSteps(uint8_t secondaryMeterLength);

}  // namespace fmd::polymetermath

#endif  // FMD_DOMAIN_ELECTRONICA_ALGORITHM_MATH_H
