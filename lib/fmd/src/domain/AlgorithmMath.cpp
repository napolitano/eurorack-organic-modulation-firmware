/**
 * @file AlgorithmMath.cpp
 * Implements pure, host-testable mathematical primitives for Drift algorithms.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "fmd/domain/AlgorithmMath.h"

#include "fmd/domain/FixedMath.h"

#include <stdint.h>

namespace fmd::perlinmath {
namespace {

/**
 * @brief Reinterpret an unsigned Q0.16 phase as the positive half of signed Q1.15.
 * @param phaseQ0F16 Unsigned phase value in Q0.16.
 * @return Equivalent non-negative signed Q1.15 value.
 */
int16_t q0F16PhaseToQ1F15(uint16_t phaseQ0F16) {
  return static_cast<int16_t>(phaseQ0F16 >> 1U);
}

}  // namespace

int16_t gradientFromRandom(uint16_t randomValue) {
  const uint16_t gradientSelector = static_cast<uint16_t>(randomValue & 15U);
  const uint16_t gradientMagnitude =
      static_cast<uint16_t>(1U + (gradientSelector & 7U));
  const int16_t positiveGradientQ1F15 =
      static_cast<int16_t>(gradientMagnitude << 11U);
  return (gradientSelector & 8U) != 0U
      ? static_cast<int16_t>(-positiveGradientQ1F15)
      : positiveGradientQ1F15;
}

uint16_t fadeQ0F16(uint16_t phaseQ0F16) {
  // The canonical quintic is f(x)=6x^5-15x^4+10x^3. The upstream result has
  // 12 effective fractional bits, so evaluate the exact rational polynomial on
  // x in Q0.12 and round once. Repeated truncated fixed-point multiplication
  // created local one-code reversals; the single-round form is monotone.
  constexpr uint64_t kHalfQ48 = UINT64_C(1) << 47U;
  constexpr uint64_t kQ0F12Scale = 4096U;

  const uint64_t phaseQ0F12 = static_cast<uint64_t>(phaseQ0F16 >> 4U);
  const uint64_t phaseSquared = phaseQ0F12 * phaseQ0F12;
  const uint64_t polynomial =
      6U * phaseSquared -
      15U * kQ0F12Scale * phaseQ0F12 +
      10U * kQ0F12Scale * kQ0F12Scale;
  const uint64_t numerator = phaseSquared * phaseQ0F12 * polynomial;

  uint64_t fadeQ0F12 = (numerator + kHalfQ48) >> 48U;
  if (fadeQ0F12 > 4095U) {
    fadeQ0F12 = 4095U;
  }
  return static_cast<uint16_t>(fadeQ0F12 << 4U);
}

int16_t segmentQ1F15(uint16_t phaseQ0F16,
                     int16_t startGradientQ1F15,
                     int16_t endGradientQ1F15) {
  constexpr int16_t kUnityQ1F15 = 0x7FFF;

  const uint16_t fadeWeightQ0F16 = fadeQ0F16(phaseQ0F16);
  const int16_t phaseQ1F15 = q0F16PhaseToQ1F15(phaseQ0F16);
  const int16_t startRampQ1F15 =
      fixedmath::mulI1F15(startGradientQ1F15, phaseQ1F15);
  const int16_t endRampQ1F15 = fixedmath::mulI1F15(
      endGradientQ1F15,
      static_cast<int16_t>(phaseQ1F15 - kUnityQ1F15));

  return fixedmath::lerpI1F15(q0F16PhaseToQ1F15(fadeWeightQ0F16),
                              startRampQ1F15,
                              endRampQ1F15);
}

uint32_t advancePhase(uint32_t phaseAccumulator,
                      uint32_t phaseIncrement,
                      bool& rollover) {
  const uint32_t advancedPhase =
      static_cast<uint32_t>(phaseAccumulator + phaseIncrement);
  rollover = advancedPhase < phaseAccumulator;
  return advancedPhase;
}

}  // namespace fmd::perlinmath

namespace fmd::brownianmath {

uint16_t movementStepSize(uint16_t speedControl) {
  return static_cast<uint16_t>((256U + speedControl) >> 1U);
}

uint16_t movementEventCutoff(uint16_t speedControl) {
  return static_cast<uint16_t>(speedControl << 6U);
}

uint16_t upwardDirectionCutoff(uint16_t targetValue,
                               uint16_t movementCutoff) {
  const uint16_t lowerCenteringBoundary =
      static_cast<uint16_t>(0xFFFFU / kCenteringMargin);
  const uint16_t upperCenteringBoundary = static_cast<uint16_t>(
      0xFFFFU - (0xFFFFU / kCenteringMargin));

  if (targetValue < lowerCenteringBoundary) {
    return static_cast<uint16_t>(
        (movementCutoff / 2U) - (movementCutoff / kCenteringStrength));
  }
  if (targetValue > upperCenteringBoundary) {
    return static_cast<uint16_t>(
        (movementCutoff / 2U) + (movementCutoff / kCenteringStrength));
  }
  return static_cast<uint16_t>(movementCutoff / 2U);
}

uint16_t nextTargetValue(uint16_t targetValue,
                         uint16_t randomValue,
                         uint16_t speedControl) {
  const uint16_t movementCutoff = movementEventCutoff(speedControl);
  if (randomValue >= movementCutoff) {
    return targetValue;
  }

  const uint16_t stepAmount = movementStepSize(speedControl);
  if (randomValue >= upwardDirectionCutoff(targetValue, movementCutoff)) {
    const uint32_t increasedTarget =
        static_cast<uint32_t>(targetValue) + static_cast<uint32_t>(stepAmount);
    return increasedTarget > 0xFFFFU
        ? 0xFFFFU
        : static_cast<uint16_t>(increasedTarget);
  }

  return stepAmount > targetValue
      ? 0U
      : static_cast<uint16_t>(targetValue - stepAmount);
}

uint16_t smoothingAlphaQ0F16(uint16_t textureControl) {
  if (textureControl > 1023U) {
    textureControl = 1023U;
  }

  constexpr uint32_t kAlphaRange =
      static_cast<uint32_t>(kMaxAlphaQ0F16) - kMinAlphaQ0F16;
  const uint32_t scaledTexture =
      static_cast<uint32_t>(textureControl) * kAlphaRange;
  return static_cast<uint16_t>(
      static_cast<uint32_t>(kMinAlphaQ0F16) +
      ((scaledTexture + 511U) / 1023U));
}

void smoothTowardTarget(uint16_t targetValue,
                        uint16_t alphaQ0F16,
                        uint16_t& currentValue,
                        uint16_t& fractionalResidualQ0F16,
                        int8_t& residualDirection) {
  if (currentValue == targetValue) {
    fractionalResidualQ0F16 = 0U;
    residualDirection = 0;
    return;
  }

  const int8_t movementDirection = currentValue < targetValue
      ? static_cast<int8_t>(1)
      : static_cast<int8_t>(-1);

  // A residual from the opposite direction would bias the first sample after a
  // target reversal. Reset it so fractional accumulation always belongs to the
  // current direction of travel.
  if (movementDirection != residualDirection) {
    fractionalResidualQ0F16 = 0U;
    residualDirection = movementDirection;
  }

  const uint16_t absoluteDistance = currentValue < targetValue
      ? static_cast<uint16_t>(targetValue - currentValue)
      : static_cast<uint16_t>(currentValue - targetValue);
  const uint32_t weightedDistanceQ0F16 =
      static_cast<uint32_t>(alphaQ0F16) * absoluteDistance +
      static_cast<uint32_t>(fractionalResidualQ0F16);

  const uint16_t integerMovement =
      static_cast<uint16_t>(weightedDistanceQ0F16 >> 16U);
  fractionalResidualQ0F16 =
      static_cast<uint16_t>(weightedDistanceQ0F16 & 0xFFFFU);

  currentValue = movementDirection > 0
      ? static_cast<uint16_t>(currentValue + integerMovement)
      : static_cast<uint16_t>(currentValue - integerMovement);

  if (currentValue == targetValue) {
    fractionalResidualQ0F16 = 0U;
    residualDirection = 0;
  }
}

}  // namespace fmd::brownianmath

namespace fmd::beziermath {

uint16_t speedVariationScaleQ1F15(uint16_t textureKnobAdc,
                                  uint16_t textureCvAdc) {
  /// Maximum code of Drift's 10-bit ADC domain.
  constexpr uint16_t kAdcMaximum = 1023U;
  /// Centre of the Texture knob domain; integer representation is 511.
  constexpr uint16_t kAdcMidpoint = kAdcMaximum / 2U;
  /// Symmetric no-variation zone around the centre position.
  constexpr uint16_t kKnobDeadZone = 128U;
  /// Usable magnitude outside the dead zone before saturation.
  constexpr uint16_t kVariationRange = kAdcMidpoint - kKnobDeadZone;

  if (textureKnobAdc > kAdcMaximum) {
    textureKnobAdc = kAdcMaximum;
  }
  if (textureCvAdc > kAdcMaximum) {
    textureCvAdc = kAdcMaximum;
  }

  uint16_t knobDistanceFromCentre = textureKnobAdc > kAdcMidpoint
      ? static_cast<uint16_t>(textureKnobAdc - kAdcMidpoint)
      : static_cast<uint16_t>(kAdcMidpoint - textureKnobAdc);
  knobDistanceFromCentre = knobDistanceFromCentre > kKnobDeadZone
      ? static_cast<uint16_t>(knobDistanceFromCentre - kKnobDeadZone)
      : 0U;

  uint16_t variationMagnitude = static_cast<uint16_t>(
      knobDistanceFromCentre + textureCvAdc / 2U);
  if (variationMagnitude > kVariationRange) {
    variationMagnitude = kVariationRange;
  }

  return static_cast<uint16_t>(
      (static_cast<uint32_t>(variationMagnitude) * 0x7FFFU) /
      kVariationRange);
}

uint16_t smoothCurveQ4F12(uint16_t phaseQ4F12) {
  // y = 3x^2 - 2x^3. Evaluate the exact integer rational form and round once:
  //   y_q12 = round(x_q12^2 * (3*S - 2*x_q12) / S^2), S=4096.
  // A single rounding step preserves mathematical monotonicity.
  constexpr uint64_t kHalfQ24 = UINT64_C(1) << 23U;
  constexpr uint64_t kMaximumPhaseQ4F12 = 4096U;
  const uint64_t phase = phaseQ4F12 > kMaximumPhaseQ4F12
      ? kMaximumPhaseQ4F12
      : phaseQ4F12;
  const uint64_t numerator =
      phase * phase * (UINT64_C(12288) - 2U * phase);
  return static_cast<uint16_t>((numerator + kHalfQ24) >> 24U);
}

uint16_t reverseCurveQ4F12(uint16_t phaseQ4F12) {
  // y = 2x^3 - 3x^2 + 2x. As above, calculate the exact rational polynomial
  // and round once instead of repeatedly truncating Q4.12 intermediates.
  constexpr uint64_t kHalfQ24 = UINT64_C(1) << 23U;
  constexpr uint64_t kMaximumPhaseQ4F12 = 4096U;
  const uint64_t phase = phaseQ4F12 > kMaximumPhaseQ4F12
      ? kMaximumPhaseQ4F12
      : phaseQ4F12;
  const uint64_t phaseSquared = phase * phase;
  const uint64_t numerator =
      2U * phaseSquared * phase -
      UINT64_C(12288) * phaseSquared +
      UINT64_C(33554432) * phase;
  return static_cast<uint16_t>((numerator + kHalfQ24) >> 24U);
}

uint16_t textureBlendQ0F16(uint16_t textureKnobAdc) {
  if (textureKnobAdc > 1023U) {
    textureKnobAdc = 1023U;
  }
  return static_cast<uint16_t>(
      (static_cast<uint32_t>(textureKnobAdc) * 0xFFFFU + 511U) / 1023U);
}

uint16_t morphCurveQ4F12(uint16_t phaseQ4F12, uint16_t textureKnobAdc) {
  const uint16_t reverseCurve = reverseCurveQ4F12(phaseQ4F12);
  const uint16_t smoothCurve = smoothCurveQ4F12(phaseQ4F12);

  if (textureKnobAdc == 0U) {
    return reverseCurve;
  }
  if (textureKnobAdc >= 1023U) {
    return smoothCurve;
  }
  return fixedmath::lerpU0F16(textureBlendQ0F16(textureKnobAdc),
                              reverseCurve,
                              smoothCurve);
}

uint16_t interpolateQ4F12(uint16_t phaseQ4F12,
                          uint16_t startValueQ4F12,
                          uint16_t endValueQ4F12,
                          uint16_t textureKnobAdc) {
  const uint16_t curveWeightQ4F12 =
      morphCurveQ4F12(phaseQ4F12, textureKnobAdc);
  return fixedmath::lerpU4F12(
      curveWeightQ4F12, startValueQ4F12, endValueQ4F12);
}

int16_t triangularIcdfQ1F15(uint16_t uniformSample15,
                            const IReferenceTables& referenceTables) {
  uniformSample15 = static_cast<uint16_t>(uniformSample15 & 0x7FFFU);

  // 32768 possible random codes are divided into 256 intervals. A 257-entry
  // table is required so interval 255 can interpolate to the positive endpoint
  // instead of flattening at the penultimate sample.
  const uint16_t lowerIndex = static_cast<uint16_t>(uniformSample15 >> 7U);
  const uint16_t upperIndex = static_cast<uint16_t>(lowerIndex + 1U);
  const int16_t interpolationWeightQ1F15 = static_cast<int16_t>(
      (static_cast<uint16_t>(uniformSample15 << 8U)) & 0x7FFFU);

  return fixedmath::lerpI1F15(
      interpolationWeightQ1F15,
      referenceTables.triangularIcdfQ1_15(lowerIndex),
      referenceTables.triangularIcdfQ1_15(upperIndex));
}

uint32_t advancePhase(uint32_t phaseAccumulator,
                      uint32_t phaseIncrement,
                      bool& rollover) {
  const uint32_t advancedPhase =
      static_cast<uint32_t>(phaseAccumulator + phaseIncrement);
  rollover = advancedPhase < phaseAccumulator;
  return advancedPhase;
}

}  // namespace fmd::beziermath

namespace fmd::lfomath {

uint16_t apexFromTexture(uint16_t textureControl) {
  if (textureControl > 1023U) {
    textureControl = 1023U;
  }
  return static_cast<uint16_t>(
      (static_cast<uint32_t>(textureControl) * 0xFFFFU + 511U) / 1023U);
}

uint16_t waveformQ0F16(uint16_t phaseQ0F16, uint16_t apexPhaseQ0F16) {
  if (apexPhaseQ0F16 == 0U) {
    return static_cast<uint16_t>(0xFFFFU - phaseQ0F16);
  }
  if (apexPhaseQ0F16 == 0xFFFFU) {
    return phaseQ0F16;
  }

  if (phaseQ0F16 <= apexPhaseQ0F16) {
    const uint32_t risingNumerator =
        static_cast<uint32_t>(phaseQ0F16) * 0xFFFFU;
    return static_cast<uint16_t>(risingNumerator / apexPhaseQ0F16);
  }

  const uint16_t remainingPhase =
      static_cast<uint16_t>(0xFFFFU - phaseQ0F16);
  const uint16_t fallingPhaseLength =
      static_cast<uint16_t>(0xFFFFU - apexPhaseQ0F16);
  const uint32_t fallingNumerator =
      static_cast<uint32_t>(remainingPhase) * 0xFFFFU;
  return static_cast<uint16_t>(fallingNumerator / fallingPhaseLength);
}

uint16_t waveform12(uint16_t phaseQ0F16, uint16_t apexPhaseQ0F16) {
  return static_cast<uint16_t>(
      waveformQ0F16(phaseQ0F16, apexPhaseQ0F16) >> 4U);
}

uint16_t remapPhasePreservingOutput(uint16_t phaseQ0F16,
                                    uint16_t previousApexQ0F16,
                                    uint16_t requestedApexQ0F16) {
  const uint16_t currentOutputQ0F16 =
      waveformQ0F16(phaseQ0F16, previousApexQ0F16);
  const bool previouslyOnRisingBranch =
      previousApexQ0F16 == 0xFFFFU ||
      (previousApexQ0F16 != 0U && phaseQ0F16 <= previousApexQ0F16);

  // Evaluate both inverse branches of the requested waveform. Near sawtooth
  // endpoints one branch can be only one phase code long and cannot represent
  // arbitrary output levels. Selecting the candidate with the smaller forward
  // error gives the most continuous possible live Texture change.
  const uint32_t risingProduct =
      static_cast<uint32_t>(currentOutputQ0F16) * requestedApexQ0F16;
  const uint16_t risingCandidatePhase = static_cast<uint16_t>(
      (risingProduct + 0x7FFFU) / 0xFFFFU);

  const uint16_t invertedOutputQ0F16 =
      static_cast<uint16_t>(0xFFFFU - currentOutputQ0F16);
  const uint16_t fallingPhaseLength =
      static_cast<uint16_t>(0xFFFFU - requestedApexQ0F16);
  const uint32_t fallingProduct =
      static_cast<uint32_t>(invertedOutputQ0F16) * fallingPhaseLength;
  const uint16_t fallingTailPhase = static_cast<uint16_t>(
      (fallingProduct + 0x7FFFU) / 0xFFFFU);
  const uint16_t fallingCandidatePhase = static_cast<uint16_t>(
      static_cast<uint32_t>(requestedApexQ0F16) + fallingTailPhase);

  const uint16_t risingCandidateOutput =
      waveformQ0F16(risingCandidatePhase, requestedApexQ0F16);
  const uint16_t fallingCandidateOutput =
      waveformQ0F16(fallingCandidatePhase, requestedApexQ0F16);
  const uint16_t risingError = risingCandidateOutput > currentOutputQ0F16
      ? static_cast<uint16_t>(risingCandidateOutput - currentOutputQ0F16)
      : static_cast<uint16_t>(currentOutputQ0F16 - risingCandidateOutput);
  const uint16_t fallingError = fallingCandidateOutput > currentOutputQ0F16
      ? static_cast<uint16_t>(fallingCandidateOutput - currentOutputQ0F16)
      : static_cast<uint16_t>(currentOutputQ0F16 - fallingCandidateOutput);

  if (risingError < fallingError) {
    return risingCandidatePhase;
  }
  if (fallingError < risingError) {
    return fallingCandidatePhase;
  }
  return previouslyOnRisingBranch
      ? risingCandidatePhase
      : fallingCandidatePhase;
}

uint32_t advancePhase(uint32_t phaseAccumulator,
                      uint32_t phaseIncrement,
                      bool& rollover) {
  const uint32_t advancedPhase =
      static_cast<uint32_t>(phaseAccumulator + phaseIncrement);
  rollover = advancedPhase < phaseAccumulator;
  return advancedPhase;
}

}  // namespace fmd::lfomath
