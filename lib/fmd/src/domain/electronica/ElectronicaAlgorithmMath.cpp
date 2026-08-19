/**
 * @file ElectronicaAlgorithmMath.cpp
 * Implements fixed-point primitives shared by the Electronica algorithm bank.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "fmd/domain/electronica/ElectronicaAlgorithmMath.h"

#include "fmd/domain/AlgorithmMath.h"
#include "fmd/domain/FrequencyMapping.h"
#include "fmd/domain/Types.h"

#include <stdint.h>

namespace fmd::electronicamath {
namespace {

uint32_t electronicaExp2Q16F16(const IReferenceTables& referenceTables,
                               uint16_t combinedSpeedControl) {
  // Exp2Table index i represents 2^(i/16). Electronica requires 2^(3u),
  // therefore the complete Speed range occupies table positions 0..48.
  const uint16_t positionQ8F8 = static_cast<uint16_t>(
      (static_cast<uint32_t>(combinedSpeedControl) * 12288UL + 511UL) / 1023UL);
  const uint8_t lowerIndex = static_cast<uint8_t>(positionQ8F8 >> 8U);
  const uint8_t fractionQ0F8 = static_cast<uint8_t>(positionQ8F8 & 0xFFU);
  if (fractionQ0F8 == 0U) {
    return referenceTables.exp2Q16_16(lowerIndex);
  }

  const uint8_t upperIndex = static_cast<uint8_t>(lowerIndex + 1U);
  const uint32_t start = referenceTables.exp2Q16_16(lowerIndex);
  const uint32_t end = referenceTables.exp2Q16_16(upperIndex);
  return start + static_cast<uint32_t>(
      ((end - start) * fractionQ0F8 + 128UL) >> 8U);
}

}  // namespace

uint16_t speedControl(uint16_t speedKnobAdc, uint16_t speedCvAdc) {
  return sumAdc(clampAdc(speedKnobAdc), clampAdc(speedCvAdc));
}

uint32_t quarterNotePhaseIncrement(const IReferenceTables& referenceTables,
                                   uint16_t speedKnobAdc,
                                   uint16_t speedCvAdc) {
  const uint32_t octaveFactorQ16F16 = electronicaExp2Q16F16(
      referenceTables, speedControl(speedKnobAdc, speedCvAdc));
  // phaseIncrementFromDecihertzQ16_16() preserves the historical Drift
  // frequency-domain scale where one Q16.16 input unit corresponds to 1/40 Hz.
  // Therefore a 30 BPM quarter note (0.5 Hz) is represented by 20 units.
  // Scaling 20 by the three-octave factor produces 0.5..4 Hz, i.e. exactly
  // 30..240 BPM as documented for Electronica and Percussion.
  return phaseIncrementFromDecihertzQ16_16(octaveFactorQ16F16 * 20UL);
}

uint32_t sixteenthNotePhaseIncrement(const IReferenceTables& referenceTables,
                                     uint16_t speedKnobAdc,
                                     uint16_t speedCvAdc) {
  return quarterNotePhaseIncrement(referenceTables, speedKnobAdc, speedCvAdc) * 4UL;
}

uint32_t shufflePairPhaseIncrement(const IReferenceTables& referenceTables,
                                   uint16_t speedKnobAdc,
                                   uint16_t speedCvAdc) {
  return quarterNotePhaseIncrement(referenceTables, speedKnobAdc, speedCvAdc) * 2UL;
}

uint16_t textureQ0F12(uint16_t textureControl) {
  textureControl = clampAdc(textureControl);
  if (textureControl == 1023U) {
    return 4096U;
  }
  return static_cast<uint16_t>(
      (static_cast<uint32_t>(textureControl) * 4096UL + 511UL) / 1023UL);
}

uint16_t smoothstepQ0F12(uint16_t xQ0F12) {
  return beziermath::smoothCurveQ4F12(xQ0F12 > 4096U ? 4096U : xQ0F12);
}

uint16_t decayQ0F12(uint16_t xQ0F12) {
  return static_cast<uint16_t>(4096U - smoothstepQ0F12(xQ0F12));
}

uint16_t scaleDac12(uint16_t peakDac12, uint16_t envelopeQ0F12) {
  if (peakDac12 > 4095U) {
    peakDac12 = 4095U;
  }
  if (envelopeQ0F12 >= 4096U) {
    return peakDac12;
  }
  return static_cast<uint16_t>(
      (static_cast<uint32_t>(peakDac12) * envelopeQ0F12 + 2048UL) >> 12U);
}

}  // namespace fmd::electronicamath

namespace fmd::pumpmath {

uint16_t recoveryEndpointQ0F16(uint16_t textureControl) {
  textureControl = clampAdc(textureControl);
  if (textureControl == 1023U) {
    return kRecoveryMaximumQ0F16;
  }
  return static_cast<uint16_t>(
      kRecoveryMinimumQ0F16 +
      (static_cast<uint32_t>(textureControl) *
           (kRecoveryMaximumQ0F16 - kRecoveryMinimumQ0F16) +
       511UL) /
          1023UL);
}

uint16_t recoveryReciprocalQ28(uint16_t endpointQ0F16) {
  if (endpointQ0F16 < kRecoveryMinimumQ0F16) {
    endpointQ0F16 = kRecoveryMinimumQ0F16;
  }
  return static_cast<uint16_t>(
      ((UINT32_C(1) << 28U) + (endpointQ0F16 / 2U)) / endpointQ0F16);
}

uint16_t recoveryProgressQ0F12(uint16_t phaseQ0F16,
                              uint16_t endpointQ0F16,
                              uint16_t reciprocalQ28) {
  if (phaseQ0F16 >= endpointQ0F16) {
    return 4096U;
  }
  const uint32_t product = static_cast<uint32_t>(phaseQ0F16) * reciprocalQ28;
  const uint32_t progress = (product + UINT32_C(0x8000)) >> 16U;
  return static_cast<uint16_t>(progress > 4096UL ? 4096UL : progress);
}

uint16_t outputDac12(uint16_t phaseQ0F16,
                     uint16_t endpointQ0F16,
                     uint16_t reciprocalQ28) {
  if (phaseQ0F16 >= endpointQ0F16) {
    return 4095U;
  }
  const uint16_t progress = recoveryProgressQ0F12(
      phaseQ0F16, endpointQ0F16, reciprocalQ28);
  return electronicamath::scaleDac12(
      4095U, electronicamath::smoothstepQ0F12(progress));
}

}  // namespace fmd::pumpmath

namespace fmd::acidmath {

uint8_t permutationCode(uint8_t stepIndex) {
  return static_cast<uint8_t>((5U * (stepIndex & 0x0FU) + 3U) & 0x0FU);
}

uint16_t baseTargetDac12(uint8_t stepIndex) {
  return static_cast<uint16_t>(1024U + 128U * permutationCode(stepIndex));
}

bool isAccentStep(uint8_t stepIndex) {
  constexpr uint16_t kAccentMask = 0x5191U;  // n%4==0 or n%7==0 for n=0..15.
  const uint8_t n = static_cast<uint8_t>(stepIndex & 0x0FU);
  return (kAccentMask & static_cast<uint16_t>(1U << n)) != 0U;
}

bool isSlideStep(uint8_t stepIndex) {
  constexpr uint16_t kSlideMask = 0x2481U;  // (5*n mod 16)<4 for n=0..15.
  const uint8_t n = static_cast<uint8_t>(stepIndex & 0x0FU);
  return (kSlideMask & static_cast<uint16_t>(1U << n)) != 0U;
}

uint16_t slideContourDac12(uint16_t previousTargetDac12,
                           uint16_t currentTargetDac12,
                           uint16_t phaseQ0F12,
                           uint16_t textureQ0F12) {
  if (previousTargetDac12 > 4095U) {
    previousTargetDac12 = 4095U;
  }
  if (currentTargetDac12 > 4095U) {
    currentTargetDac12 = 4095U;
  }
  if (textureQ0F12 > 4096U) {
    textureQ0F12 = 4096U;
  }

  const uint16_t smooth = electronicamath::smoothstepQ0F12(phaseQ0F12);
  const int32_t targetDelta =
      static_cast<int32_t>(currentTargetDac12) - previousTargetDac12;
  const int32_t interpolationDelta =
      (targetDelta * static_cast<int32_t>(smooth) +
       (targetDelta >= 0 ? INT32_C(2048) : INT32_C(-2048))) /
      INT32_C(4096);
  const int32_t interpolated =
      static_cast<int32_t>(previousTargetDac12) + interpolationDelta;
  const int32_t morphDelta =
      interpolated - static_cast<int32_t>(currentTargetDac12);
  const int32_t morphContribution =
      (morphDelta * static_cast<int32_t>(textureQ0F12) +
       (morphDelta >= 0 ? INT32_C(2048) : INT32_C(-2048))) /
      INT32_C(4096);
  int32_t result = static_cast<int32_t>(currentTargetDac12) + morphContribution;
  if (result < 0L) {
    result = 0L;
  } else if (result > 4095L) {
    result = 4095L;
  }
  return static_cast<uint16_t>(result);
}

uint16_t accentContributionDac12(uint16_t phaseQ0F12,
                                 uint16_t textureQ0F12) {
  if (textureQ0F12 > 4096U) {
    textureQ0F12 = 4096U;
  }
  const uint16_t maximumAccent = static_cast<uint16_t>(
      (static_cast<uint32_t>(768U) * textureQ0F12 + 2048UL) >> 12U);
  return electronicamath::scaleDac12(
      maximumAccent, electronicamath::decayQ0F12(phaseQ0F12));
}

uint16_t addAccentSaturating(uint16_t baseDac12, uint16_t accentDac12) {
  const uint32_t sum = static_cast<uint32_t>(baseDac12) + accentDac12;
  return sum > 4095UL ? 4095U : static_cast<uint16_t>(sum);
}

}  // namespace fmd::acidmath

namespace fmd::shufflemath {

uint16_t secondOnsetRatioQ0F16(uint16_t textureControl) {
  textureControl = clampAdc(textureControl);
  if (textureControl == 1023U) {
    return kMaximumRatioQ0F16;
  }
  return static_cast<uint16_t>(
      kStraightRatioQ0F16 +
      (static_cast<uint32_t>(textureControl) *
           (kMaximumRatioQ0F16 - kStraightRatioQ0F16) +
       511UL) /
          1023UL);
}

uint32_t secondOnsetThreshold(uint16_t ratioQ0F16) {
  if (ratioQ0F16 < kStraightRatioQ0F16) {
    ratioQ0F16 = kStraightRatioQ0F16;
  } else if (ratioQ0F16 > kMaximumRatioQ0F16) {
    ratioQ0F16 = kMaximumRatioQ0F16;
  }
  return static_cast<uint32_t>(ratioQ0F16) << 16U;
}

uint32_t envelopePhaseIncrement(uint32_t pairPhaseIncrement) {
  return pairPhaseIncrement * 8UL;
}

uint16_t decayOutputDac12(uint32_t envelopePhase) {
  return electronicamath::scaleDac12(
      4095U,
      electronicamath::decayQ0F12(static_cast<uint16_t>(envelopePhase >> 20U)));
}

}  // namespace fmd::shufflemath

namespace fmd::polymetermath {

uint8_t secondaryMeterLength(uint16_t textureControl) {
  textureControl = clampAdc(textureControl);
  constexpr uint8_t kMeters[4] = {3U, 5U, 7U, 9U};
  return kMeters[textureControl >> 8U];
}

uint16_t stepAmplitudeDac12(bool primaryStart, bool secondaryStart) {
  uint16_t amplitude = kBaseLevelDac12;
  if (primaryStart) {
    amplitude = static_cast<uint16_t>(amplitude + kPrimaryAccentDac12);
  }
  if (secondaryStart) {
    amplitude = static_cast<uint16_t>(amplitude + kSecondaryAccentDac12);
  }
  return amplitude > 4095U ? 4095U : amplitude;
}

uint32_t envelopePhaseIncrement(uint32_t sixteenthPhaseIncrement) {
  return sixteenthPhaseIncrement * 2UL;
}

uint16_t decayOutputDac12(uint32_t envelopePhase, uint16_t peakDac12) {
  return electronicamath::scaleDac12(
      peakDac12,
      electronicamath::decayQ0F12(static_cast<uint16_t>(envelopePhase >> 20U)));
}

uint8_t recurrenceSteps(uint8_t secondaryLength) {
  if (secondaryLength == 0U) {
    return 0U;
  }
  uint8_t a = 4U;
  uint8_t b = secondaryLength;
  while (b != 0U) {
    const uint8_t remainder = static_cast<uint8_t>(a % b);
    a = b;
    b = remainder;
  }
  return static_cast<uint8_t>((4U / a) * secondaryLength);
}

}  // namespace fmd::polymetermath
