/**
 * @file DubstepAlgorithmMath.cpp
 * Implements fixed-point tempo, phrase and contour primitives for the Dubstep/Bass bank.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "fmd/domain/dubstep/DubstepAlgorithmMath.h"

#include "fmd/domain/FrequencyMapping.h"
#include "fmd/domain/Types.h"
#include "fmd/domain/electronica/ElectronicaAlgorithmMath.h"

namespace fmd::dubstepmath {
namespace {

uint32_t dubstepExp2Q16F16(const IReferenceTables& referenceTables,
                           uint16_t speedControl) {
  speedControl = clampAdc(speedControl);
  // Exp2 table entry i is 2^(i/16).  Two octaves therefore occupy 0..32.
  const uint16_t positionQ8F8 = static_cast<uint16_t>(
      (static_cast<uint32_t>(speedControl) * 8192UL + 511UL) / 1023UL);
  const uint8_t lower = static_cast<uint8_t>(positionQ8F8 >> 8U);
  const uint8_t fraction = static_cast<uint8_t>(positionQ8F8 & 0xFFU);
  if (fraction == 0U) {
    return referenceTables.exp2Q16_16(lower);
  }
  const uint32_t start = referenceTables.exp2Q16_16(lower);
  const uint32_t end = referenceTables.exp2Q16_16(static_cast<uint8_t>(lower + 1U));
  return start + static_cast<uint32_t>(((end - start) * fraction + 128UL) >> 8U);
}

}  // namespace

uint32_t quarterNotePhaseIncrement(const IReferenceTables& referenceTables,
                                   uint16_t speedKnobAdc) {
  const uint32_t factorQ16F16 = dubstepExp2Q16F16(referenceTables, speedKnobAdc);
  // Quarter frequency in Hz is BPM/60. FrequencyMapping accepts decihertz,
  // hence 70 BPM corresponds to 70/6 decihertz.
  const uint32_t decihertzQ16F16 =
      static_cast<uint32_t>((static_cast<uint64_t>(factorQ16F16) * 70ULL + 3ULL) / 6ULL);
  return phaseIncrementFromDecihertzQ16_16(decihertzQ16F16);
}

uint8_t textureRegion(uint16_t textureControl) {
  return static_cast<uint8_t>(clampAdc(textureControl) >> 8U);
}

uint16_t triangleQ0F12(uint32_t phase) {
  const uint16_t phaseQ0F16 = static_cast<uint16_t>(phase >> 16U);
  const uint32_t folded = phaseQ0F16 <= 32768U
      ? phaseQ0F16
      : 65536UL - phaseQ0F16;
  return static_cast<uint16_t>((folded * 4096UL + 16384UL) >> 15U);
}

uint16_t q0F12ToDac12(uint16_t valueQ0F12) {
  if (valueQ0F12 >= 4096U) return 4095U;
  return static_cast<uint16_t>((static_cast<uint32_t>(valueQ0F12) * 4095UL + 2048UL) >> 12U);
}

}  // namespace fmd::dubstepmath

namespace fmd::wobblemath {

uint8_t phraseSymbol(uint8_t cellIndex) {
  constexpr uint8_t kPhrase[8] = {0U, 1U, 0U, 2U, 1U, 3U, 0U, 2U};
  return kPhrase[cellIndex & 0x07U];
}

uint8_t rateCode(uint8_t textureRegion, uint8_t symbol) {
  constexpr uint8_t kVocabulary[4][4] = {
      {2U, 2U, 2U, 2U},  // 1, 1, 1, 1
      {2U, 5U, 2U, 5U},  // 1, 2, 1, 2
      {2U, 3U, 5U, 4U},  // 1, 4/3, 2, 3/2
      {2U, 5U, 6U, 7U},  // 1, 2, 3, 4
  };
  if (textureRegion > 3U) textureRegion = 3U;
  return kVocabulary[textureRegion][symbol & 0x03U];
}

uint32_t carrierIncrement(uint32_t quarterIncrement, uint8_t rate) {
  switch (rate > 7U ? 7U : rate) {
    case 0U: return static_cast<uint32_t>((quarterIncrement + 1UL) >> 1U);                         // 1/2
    case 1U: return static_cast<uint32_t>((2ULL * quarterIncrement + 1ULL) / 3ULL); // 2/3
    case 2U: return quarterIncrement;                                       // 1
    case 3U: return static_cast<uint32_t>((4ULL * quarterIncrement + 1ULL) / 3ULL); // 4/3
    case 4U: return static_cast<uint32_t>((3ULL * quarterIncrement + 1ULL) / 2ULL); // 3/2
    case 5U: return quarterIncrement * 2UL;
    case 6U: return quarterIncrement * 3UL;
    default: return quarterIncrement * 4UL;
  }
}

}  // namespace fmd::wobblemath

namespace fmd::growlmath {

Weights normalizedWeights(uint16_t textureControl) {
  const uint16_t tau = electronicamath::textureQ0F12(textureControl);
  const uint16_t a = static_cast<uint16_t>((static_cast<uint32_t>(tau) * 3UL + 2UL) >> 2U);
  const uint16_t tauSquared = static_cast<uint16_t>(
      (static_cast<uint32_t>(tau) * tau + 2048UL) >> 12U);
  const uint16_t b = static_cast<uint16_t>((tauSquared + 1U) >> 1U);
  const uint16_t denominator = static_cast<uint16_t>(4096U + a + b);

  const uint16_t second = static_cast<uint16_t>(
      (static_cast<uint32_t>(a) * 4096UL + denominator / 2U) / denominator);
  uint16_t third = static_cast<uint16_t>(
      (static_cast<uint32_t>(b) * 4096UL + denominator / 2U) / denominator);
  if (static_cast<uint32_t>(second) + third > 4096UL) {
    third = static_cast<uint16_t>(4096U - second);
  }
  // Keep the normalized fixed-point weights exactly unity.  Assigning the
  // rounding residual to the fundamental preserves the monotonic higher-order
  // component contract across the full 10-bit Texture range.
  const uint16_t fundamental = static_cast<uint16_t>(4096U - second - third);
  return Weights{fundamental, second, third};
}

uint16_t outputDac12(uint32_t phase, const Weights& weights) {
  const uint16_t first = dubstepmath::triangleQ0F12(phase);
  const uint16_t second = dubstepmath::triangleQ0F12(static_cast<uint32_t>(phase * 2UL + UINT32_C(0x40000000)));
  const uint16_t third = dubstepmath::triangleQ0F12(static_cast<uint32_t>(phase * 3UL + UINT32_C(0x20000000)));
  const uint32_t weighted =
      static_cast<uint32_t>(first) * weights.fundamental +
      static_cast<uint32_t>(second) * weights.second +
      static_cast<uint32_t>(third) * weights.third;
  const uint16_t q0F12 = static_cast<uint16_t>((weighted + 2048UL) >> 12U);
  return dubstepmath::q0F12ToDac12(q0F12);
}

}  // namespace fmd::growlmath

namespace fmd::chopmath {

uint8_t addedOnsetCount(uint16_t textureControl) {
  textureControl = fmd::clampAdc(textureControl);
  return static_cast<uint8_t>((static_cast<uint32_t>(textureControl) * 9UL) >> 10U);
}

uint16_t onsetMask(uint8_t addedOnsets) {
  constexpr uint8_t kCandidates[8] = {3U, 11U, 6U, 14U, 2U, 10U, 7U, 15U};
  if (addedOnsets > 8U) addedOnsets = 8U;
  uint16_t mask = 0x0101U;  // anchors 0 and 8
  for (uint8_t index = 0U; index < addedOnsets; ++index) {
    mask = static_cast<uint16_t>(mask | static_cast<uint16_t>(1U << kCandidates[index]));
  }
  return mask;
}

bool stepActive(uint16_t mask, uint8_t stepIndex) {
  return (mask & static_cast<uint16_t>(1U << (stepIndex & 0x0FU))) != 0U;
}

uint16_t articulationDac12(uint32_t stepPhase) {
  if (stepPhase < UINT32_C(0x80000000)) return 4095U;
  const uint32_t remaining = UINT32_MAX - stepPhase;
  return static_cast<uint16_t>((static_cast<uint64_t>(remaining) * 4095ULL +
                                UINT64_C(0x3FFFFFFF)) >> 31U);
}

}  // namespace fmd::chopmath

namespace fmd::buildmath {

uint8_t phraseLengthBars(uint16_t textureControl) {
  switch (dubstepmath::textureRegion(textureControl)) {
    case 0U: return 8U;
    case 1U: return 4U;
    case 2U: return 2U;
    default: return 1U;
  }
}

uint8_t phraseQuarterShift(uint8_t bars) {
  if (bars >= 8U) return 5U;
  if (bars >= 4U) return 4U;
  if (bars >= 2U) return 3U;
  return 2U;
}

uint16_t macroRiseQ0F12(uint32_t phrasePhase) {
  return electronicamath::smoothstepQ0F12(
      static_cast<uint16_t>(phrasePhase >> 20U));
}

uint8_t microRateStage(uint32_t phrasePhase) {
  return static_cast<uint8_t>(phrasePhase >> 30U);
}

uint32_t microPhaseIncrement(uint32_t quarterIncrement, uint8_t stage) {
  if (stage > 3U) stage = 3U;
  return quarterIncrement << stage;
}

uint16_t outputDac12(uint32_t phrasePhase, uint32_t microPhase) {
  const uint16_t macro = macroRiseQ0F12(phrasePhase);
  const uint16_t micro = dubstepmath::triangleQ0F12(microPhase);
  const uint16_t microFactor = static_cast<uint16_t>(1024U +
      ((static_cast<uint32_t>(micro) * 3UL + 2UL) >> 2U));
  const uint16_t combined = static_cast<uint16_t>(
      (static_cast<uint32_t>(macro) * microFactor + 2048UL) >> 12U);
  return dubstepmath::q0F12ToDac12(combined);
}

}  // namespace fmd::buildmath
