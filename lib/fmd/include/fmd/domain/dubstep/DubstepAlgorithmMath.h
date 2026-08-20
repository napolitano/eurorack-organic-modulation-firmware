/**
 * @file DubstepAlgorithmMath.h
 * Declares fixed-point tempo, phrase and contour primitives for the Dubstep/Bass bank.
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

constexpr uint16_t kMinimumTempoBpm = 70U;   ///< Internal Speed minimum.
constexpr uint16_t kCenterTempoBpm = 140U;   ///< Approximate physical midpoint target.
constexpr uint16_t kMaximumTempoBpm = 280U;  ///< Internal Speed maximum.

/** Map the Speed knob to the proposed 70*2^(2u) quarter-note phase rate. */
uint32_t quarterNotePhaseIncrement(const IReferenceTables& referenceTables,
                                   uint16_t speedKnobAdc);
/** Saturating four-region Texture selector. */
uint8_t textureRegion(uint16_t textureControl);
/** Convert a 32-bit phase into a bounded unipolar triangle in Q0.12. */
uint16_t triangleQ0F12(uint32_t phase);
/** Convert Q0.12 to the 12-bit DAC domain with exact endpoints. */
uint16_t q0F12ToDac12(uint16_t valueQ0F12);

}  // namespace fmd::dubstepmath

namespace fmd::wobblemath {

/** Eight eighth-note phrase symbols in one 4/4 bar. */
uint8_t phraseSymbol(uint8_t cellIndex);
/** Resolve a phrase symbol and Texture region to one of eight rate codes. */
uint8_t rateCode(uint8_t textureRegion, uint8_t symbol);
/** Scale a quarter-note phase increment by the exact documented rational rate. */
uint32_t carrierIncrement(uint32_t quarterIncrement, uint8_t rateCode);

}  // namespace fmd::wobblemath

namespace fmd::growlmath {

/** Normalized Q0.12 component weights; the three values always sum to 4096. */
struct Weights {
  uint16_t fundamental;
  uint16_t second;
  uint16_t third;
};

/** Compute normalized fixed-point weights for the documented a(tau), b(tau) model. */
Weights normalizedWeights(uint16_t textureControl);
/** Evaluate the three-lobed Growl control gesture for one phase and cached weights. */
uint16_t outputDac12(uint32_t phase, const Weights& weights);

}  // namespace fmd::growlmath

namespace fmd::chopmath {

/** Map Texture to exactly 0..8 additional deterministic onset candidates. */
uint8_t addedOnsetCount(uint16_t textureControl);
/** Return the exact 16-step onset mask for the supplied additional-onset count. */
uint16_t onsetMask(uint8_t addedOnsets);
/** Return whether a wrapped 16-step index is active in a mask. */
bool stepActive(uint16_t mask, uint8_t stepIndex);
/** Evaluate the active-step half-hold/half-decay articulation contour. */
uint16_t articulationDac12(uint32_t stepPhase);

}  // namespace fmd::chopmath

namespace fmd::buildmath {

/** Map Texture regions to repeating 8, 4, 2 or 1 bar builds. */
uint8_t phraseLengthBars(uint16_t textureControl);
/** Return log2(total quarter notes) for the selected power-of-two phrase length. */
uint8_t phraseQuarterShift(uint8_t phraseLengthBars);
/** Evaluate cubic smoothstep for a normalized phrase phase. */
uint16_t macroRiseQ0F12(uint32_t phrasePhase);
/** Select quarter/eighth/sixteenth/thirty-second stage from phrase phase. */
uint8_t microRateStage(uint32_t phrasePhase);
/** Return a micro triangle phase increment at 1x, 2x, 4x or 8x quarter rate. */
uint32_t microPhaseIncrement(uint32_t quarterIncrement, uint8_t stage);
/** Combine the macro rise and micro triangle according to the documented equation. */
uint16_t outputDac12(uint32_t phrasePhase, uint32_t microPhase);

}  // namespace fmd::buildmath

#endif  // FMD_DOMAIN_DUBSTEP_ALGORITHM_MATH_H
