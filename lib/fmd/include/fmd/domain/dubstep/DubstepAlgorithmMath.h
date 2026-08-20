/**
 * @file DubstepAlgorithmMath.h
 * Declares fixed-point tempo, phrase and contour primitives for the Dubstep/Bass bank.
 *
 * @details
 * The helpers in this header are intentionally pure. They encode the released
 * 0.3.0 mathematical contract separately from algorithm state machines so unit
 * tests can exhaustively verify tempo mapping, phrase vocabularies, masks,
 * component weights and bounded DAC projection without emulating hardware.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_DOMAIN_DUBSTEP_ALGORITHM_MATH_H
#define FMD_DOMAIN_DUBSTEP_ALGORITHM_MATH_H

#include <stdint.h>

#include "fmd/ports/ReferenceTables.h"

namespace fmd::dubstepmath {

constexpr uint16_t kMinimumTempoBpm = 70U;   ///< Internal Speed minimum in quarter-note BPM.
constexpr uint16_t kCenterTempoBpm = 140U;   ///< Approximate physical midpoint target in quarter-note BPM.
constexpr uint16_t kMaximumTempoBpm = 280U;  ///< Internal Speed maximum in quarter-note BPM.

/**
 * @brief Map Speed to the released logarithmic 70*2^(2u) quarter-note rate.
 * @param referenceTables Read-only exp2 lookup provider used for fixed-point interpolation.
 * @param speedKnobAdc Speed potentiometer in the 10-bit ADC domain; values above 1023 clamp.
 * @return Q0.32 quarter-note phase increment per 2.5 kHz scheduler sample.
 */
uint32_t quarterNotePhaseIncrement(const IReferenceTables& referenceTables,
                                   uint16_t speedKnobAdc);

/**
 * @brief Map saturated Texture to one of four equal 256-code regions.
 * @param textureControl Combined Texture knob/CV code; values above 1023 clamp.
 * @return Region 0..3.
 */
uint8_t textureRegion(uint16_t textureControl);

/**
 * @brief Convert a full 32-bit cycle phase into a unipolar triangle.
 * @param phase Unsigned Q0.32 cycle phase.
 * @return Triangle amplitude in Q0.12, including the exact peak value 4096.
 */
uint16_t triangleQ0F12(uint32_t phase);

/**
 * @brief Convert a Q0.12 unit interval to the physical 12-bit DAC domain.
 * @param valueQ0F12 Normalized amplitude; values at or above 4096 map to full scale.
 * @return DAC code 0..4095 with exact endpoints.
 */
uint16_t q0F12ToDac12(uint16_t valueQ0F12);

}  // namespace fmd::dubstepmath

namespace fmd::wobblemath {

/**
 * @brief Return the project-defined phrase symbol for one eighth-note cell.
 * @param cellIndex Cell index; only the low three bits are significant.
 * @return Phrase symbol 0..3 from the released eight-cell word.
 */
uint8_t phraseSymbol(uint8_t cellIndex);

/**
 * @brief Resolve a phrase symbol through one of the four Texture vocabularies.
 * @param textureRegion Texture region; values above 3 clamp to region 3.
 * @param symbol Phrase symbol; only the low two bits are significant.
 * @return Rate code 0..7 understood by carrierIncrement().
 */
uint8_t rateCode(uint8_t textureRegion, uint8_t symbol);

/**
 * @brief Scale a quarter-note phase increment by an exact rational Wobble rate.
 * @param quarterIncrement Q0.32 phase increment for one cycle per quarter note.
 * @param rateCode Rate code 0..7; values above 7 clamp to code 7.
 * @return Carrier Q0.32 phase increment for ratios 1/2, 2/3, 1, 4/3, 3/2, 2, 3 or 4.
 * @note Codes 0 and 1 are implemented/tested helpers but are not selected by
 *       the released 0.3.0 four-region phrase vocabularies.
 */
uint32_t carrierIncrement(uint32_t quarterIncrement, uint8_t rateCode);

}  // namespace fmd::wobblemath

namespace fmd::growlmath {

/** @brief Normalized Q0.12 component weights; all three values sum exactly to 4096. */
struct Weights {
  uint16_t fundamental;  ///< Weight of T(phi).
  uint16_t second;       ///< Weight of T(2*phi + 1/4).
  uint16_t third;        ///< Weight of T(3*phi + 1/8).
};

/**
 * @brief Compute normalized fixed-point weights for the a(tau), b(tau) model.
 * @param textureControl Combined Texture code in the 10-bit ADC domain; clamped to 1023.
 * @return Unity-sum Q0.12 weights. Rounding residual is assigned to the fundamental.
 */
Weights normalizedWeights(uint16_t textureControl);

/**
 * @brief Evaluate the released three-component Growl control gesture.
 * @param phase Unsigned Q0.32 half-note gesture phase.
 * @param weights Cached unity-sum component weights from normalizedWeights().
 * @return Bounded 12-bit DAC code 0..4095.
 */
uint16_t outputDac12(uint32_t phase, const Weights& weights);

}  // namespace fmd::growlmath

namespace fmd::chopmath {

/**
 * @brief Map Texture to the number of additional deterministic onset candidates.
 * @param textureControl Combined Texture code, defensively clamped to 0..1023.
 * @return Additional onset count 0..8; anchors at steps 0 and 8 are not included in this count.
 */
uint8_t addedOnsetCount(uint16_t textureControl);

/**
 * @brief Construct the released 16-step mask for a requested candidate count.
 * @param addedOnsets Number of ordered candidates to add; values above 8 clamp.
 * @return 16-bit mask containing anchors 0/8 plus the requested candidate prefix.
 */
uint16_t onsetMask(uint8_t addedOnsets);

/**
 * @brief Test one wrapped 16-step position in an onset mask.
 * @param mask 16-bit Chop onset mask.
 * @param stepIndex Step number; only the low four bits are significant.
 * @return true when the wrapped step is active.
 */
bool stepActive(uint16_t mask, uint8_t stepIndex);

/**
 * @brief Evaluate the active-step hold/linear-decay articulation contour.
 * @param stepPhase Unsigned Q0.32 phase within one sixteenth step.
 * @return DAC code: 4095 through the first half-step, then a linear decay to zero.
 */
uint16_t articulationDac12(uint32_t stepPhase);

}  // namespace fmd::chopmath

namespace fmd::buildmath {

/**
 * @brief Map Texture regions to repeating 8, 4, 2 or 1 bar builds.
 * @param textureControl Combined Texture code; values above 1023 clamp.
 * @return Phrase length in 4/4 bars.
 */
uint8_t phraseLengthBars(uint16_t textureControl);

/**
 * @brief Return log2(total quarter notes) for a supported phrase length.
 * @param phraseLengthBars Requested bar count; values >=8/4/2 select 8/4/2, otherwise 1.
 * @return Shift 5, 4, 3 or 2 corresponding to 32, 16, 8 or 4 quarter notes.
 */
uint8_t phraseQuarterShift(uint8_t phraseLengthBars);

/**
 * @brief Evaluate cubic smoothstep over the high 12 bits of phrase phase.
 * @param phrasePhase Unsigned Q0.32 normalized phrase phase.
 * @return Macro rise in Q0.12.
 */
uint16_t macroRiseQ0F12(uint32_t phrasePhase);

/**
 * @brief Select the quarter/eighth/sixteenth/thirty-second micro-rate stage.
 * @param phrasePhase Unsigned Q0.32 normalized phrase phase.
 * @return Stage 0..3 from the top two phase bits.
 */
uint8_t microRateStage(uint32_t phrasePhase);

/**
 * @brief Scale quarter timing to the selected 1x/2x/4x/8x micro carrier rate.
 * @param quarterIncrement Q0.32 quarter-note phase increment.
 * @param stage Micro-rate stage; values above 3 clamp to 3.
 * @return Q0.32 micro carrier phase increment.
 */
uint32_t microPhaseIncrement(uint32_t quarterIncrement, uint8_t stage);

/**
 * @brief Combine macro smoothstep and micro triangle into the Build output.
 * @param phrasePhase Unsigned Q0.32 phrase phase used for the macro rise.
 * @param microPhase Unsigned Q0.32 triangle phase at the currently selected micro rate.
 * @return Bounded 12-bit DAC code 0..4095.
 */
uint16_t outputDac12(uint32_t phrasePhase, uint32_t microPhase);

}  // namespace fmd::buildmath

#endif  // FMD_DOMAIN_DUBSTEP_ALGORITHM_MATH_H
