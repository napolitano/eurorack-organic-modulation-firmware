/**
 * @file PercussionAlgorithmMath.cpp
 * Implements fixed-point rhythm primitives shared by the Percussion bank.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "fmd/domain/percussion/PercussionAlgorithmMath.h"

#include "fmd/domain/Types.h"

namespace fmd::percussionmath {

uint8_t phraseLengthBars(uint16_t textureControl) {
  textureControl = clampAdc(textureControl);
  if (textureControl < 256U) return 16U;
  if (textureControl < 512U) return 12U;
  if (textureControl < 768U) return 8U;
  return 4U;
}

uint8_t fillStrength(uint16_t textureControl) {
  textureControl = clampAdc(textureControl);
  return static_cast<uint8_t>((static_cast<uint32_t>(textureControl) * 5UL) >> 10U);
}

PhraseState::PhraseState() : barIndex_(0U), phraseLength_(16U) {}
void PhraseState::start(uint16_t textureControl) {
  barIndex_ = 0U;
  phraseLength_ = phraseLengthBars(textureControl);
}
void PhraseState::advanceBar(uint16_t textureControl) {
  ++barIndex_;
  if (barIndex_ >= phraseLength_) {
    barIndex_ = 0U;
    phraseLength_ = phraseLengthBars(textureControl);
  }
}

uint16_t randomCutoffLinear(uint16_t textureControl) {
  textureControl = clampAdc(textureControl);
  if (textureControl == 1023U) return 65535U;
  return static_cast<uint16_t>((static_cast<uint32_t>(textureControl) * 65535UL + 511UL) / 1023UL);
}

bool randomBelow(uint16_t randomWord, uint32_t cutoff) {
  return cutoff >= 65536UL || static_cast<uint32_t>(randomWord) < cutoff;
}

int16_t signedDeviation(uint16_t randomWord, uint16_t radius) {
  if (radius == 0U) return 0;
  const uint16_t range = static_cast<uint16_t>(2U * radius + 1U);
  const uint16_t scaled = static_cast<uint16_t>((static_cast<uint32_t>(randomWord) * range) >> 16U);
  return static_cast<int16_t>(scaled) - static_cast<int16_t>(radius);
}

}  // namespace fmd::percussionmath

namespace fmd::euclidmath {
uint8_t hitCount(uint16_t textureControl) {
  textureControl = fmd::clampAdc(textureControl);
  return static_cast<uint8_t>(2U + ((static_cast<uint32_t>(textureControl) * 12UL) >> 10U));
}
uint16_t canonicalMask(uint8_t hits) {
  constexpr uint16_t kMasks[12] = {
      0x0101U, 0x0421U, 0x1111U, 0x1249U, 0x4949U, 0x4A95U,
      0x5555U, 0x6AD5U, 0x6D6DU, 0xB6DBU, 0x7777U, 0xBDEFU};
  if (hits < 2U) hits = 2U;
  if (hits > 13U) hits = 13U;
  return kMasks[hits - 2U];
}
uint16_t fillTailMask(uint8_t strength) {
  if (strength > 4U) strength = 4U;
  if (strength == 0U) return 0U;
  return static_cast<uint16_t>(0xFFFFU << static_cast<uint8_t>(16U - strength));
}
bool stepHits(uint16_t mask, uint8_t stepIndex) {
  return (mask & static_cast<uint16_t>(1U << (stepIndex & 0x0FU))) != 0U;
}
}  // namespace fmd::euclidmath

namespace fmd::repeatmath {
uint32_t repeatCutoff(uint16_t textureControl) {
  textureControl = fmd::clampAdc(textureControl);
  return (static_cast<uint32_t>(textureControl) * 49152UL + 511UL) / 1023UL;
}
uint8_t ratchetCount(uint16_t textureControl) {
  textureControl = fmd::clampAdc(textureControl);
  return static_cast<uint8_t>(2U + ((static_cast<uint32_t>(textureControl) * 3UL) >> 10U));
}
uint32_t subEventThreshold(uint8_t pulseIndex, uint8_t pulseCount) {
  if (pulseCount < 2U || pulseIndex == 0U || pulseIndex >= pulseCount) return 0U;
  return (UINT64_C(1) << 31U) * pulseIndex / pulseCount;
}
uint8_t forcedMinimumCount(uint8_t strength, uint8_t quarterIndex) {
  if (strength == 0U) return 1U;
  quarterIndex &= 0x03U;
  if (strength == 4U && quarterIndex >= 2U) return 4U;
  if (quarterIndex != 3U) return 1U;
  if (strength == 1U) return 2U;
  if (strength == 2U) return 3U;
  return 4U;
}
}  // namespace fmd::repeatmath

namespace fmd::probabilitymath {
StepClass classifyStep(uint8_t stepIndex) {
  const uint8_t step = static_cast<uint8_t>(stepIndex & 0x0FU);
  if ((step & 0x03U) == 0U) return StepClass::Primary;
  if ((step & 0x01U) == 0U) return StepClass::Secondary;
  return StepClass::Ghost;
}
uint32_t secondaryCutoff(uint16_t textureControl) {
  textureControl = fmd::clampAdc(textureControl);
  if (textureControl == 1023U) return 65536UL;
  return (static_cast<uint32_t>(textureControl) * 65536UL) / 1023UL;
}
uint32_t ghostCutoff(uint16_t textureControl) {
  textureControl = fmd::clampAdc(textureControl);
  const uint64_t numerator = static_cast<uint64_t>(textureControl) * textureControl * 32768ULL;
  return static_cast<uint32_t>(numerator / (1023ULL * 1023ULL));
}
uint32_t fillBoostCutoff(uint8_t strength) {
  if (strength > 4U) strength = 4U;
  return static_cast<uint32_t>(strength) * 8192UL;
}
uint32_t effectiveCutoff(StepClass stepClass,
                         uint16_t textureControl,
                         bool fillBar,
                         bool finalQuarter,
                         uint8_t strength) {
  if (stepClass == StepClass::Primary) return 65536UL;
  uint32_t cutoff = stepClass == StepClass::Secondary
      ? secondaryCutoff(textureControl) : ghostCutoff(textureControl);
  if (fillBar) {
    const uint32_t boost = fillBoostCutoff(strength);
    cutoff += boost;
    if (finalQuarter) cutoff += boost;
  }
  return cutoff > 65536UL ? 65536UL : cutoff;
}
}  // namespace fmd::probabilitymath

namespace fmd::humanizemath {
uint8_t jitterRadiusSamples(uint16_t textureControl) {
  textureControl = fmd::clampAdc(textureControl);
  return static_cast<uint8_t>((static_cast<uint32_t>(textureControl) * 30UL) / 1023UL);
}
uint16_t amplitudeRadiusDac12(uint16_t textureControl) {
  textureControl = fmd::clampAdc(textureControl);
  return static_cast<uint16_t>((static_cast<uint32_t>(textureControl) * 255UL) / 1023UL);
}
int8_t jitterSamples(uint16_t randomWord, uint16_t textureControl) {
  return static_cast<int8_t>(fmd::percussionmath::signedDeviation(
      randomWord, jitterRadiusSamples(textureControl)));
}
uint16_t pulseAmplitudeDac12(uint16_t randomWord, uint16_t textureControl) {
  constexpr uint16_t kCenter = 3840U;
  const uint16_t radius = amplitudeRadiusDac12(textureControl);
  int32_t value = static_cast<int32_t>(kCenter) +
      fmd::percussionmath::signedDeviation(randomWord, radius);
  if (value < 0L) value = 0L;
  if (value > 4095L) value = 4095L;
  return static_cast<uint16_t>(value);
}
}  // namespace fmd::humanizemath
