/**
 * @file AmbientAlgorithmMath.cpp
 * Implements pure mathematical primitives used by the optional Ambient algorithm bank.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "fmd/domain/AmbientAlgorithmMath.h"

#include "fmd/domain/Types.h"

#include <limits.h>
#include <stdint.h>

namespace fmd::ambientmath {

uint32_t ambientPhaseIncrement(uint32_t basePhaseIncrement) {
  return static_cast<uint32_t>((basePhaseIncrement + 8U) >> 4U);
}

uint16_t mappedSpeedControl(uint16_t speedKnobAdc, uint16_t speedCvAdc) {
  constexpr uint16_t kMaximumMappedControl = static_cast<uint16_t>((1023U * 12U) / 5U);
  speedKnobAdc = clampAdc(speedKnobAdc);
  speedCvAdc = clampAdc(speedCvAdc);
  uint16_t combined = static_cast<uint16_t>((speedKnobAdc * 12U) / 5U + speedCvAdc);
  if (combined > kMaximumMappedControl) {
    combined = kMaximumMappedControl;
  }
  return combined;
}

uint16_t anchorSpeedBucket(uint16_t speedKnobAdc, uint16_t speedCvAdc) {
  return static_cast<uint16_t>(mappedSpeedControl(speedKnobAdc, speedCvAdc) >> 3U);
}

uint16_t smoothstepQ0F12(uint16_t xQ0F12) {
  if (xQ0F12 > 4096U) {
    xQ0F12 = 4096U;
  }
  const uint32_t xSquaredQ0F12 =
      (static_cast<uint32_t>(xQ0F12) * xQ0F12 + 2048U) >> 12U;
  const uint32_t xCubedQ0F12 =
      (xSquaredQ0F12 * xQ0F12 + 2048U) >> 12U;
  const uint32_t result = 3U * xSquaredQ0F12 - 2U * xCubedQ0F12;
  return static_cast<uint16_t>(result > 4096U ? 4096U : result);
}

}  // namespace fmd::ambientmath

namespace fmd::currentmath {

uint32_t sqrt2Increment(uint32_t baseIncrement) {
  return baseIncrement +
         ((baseIncrement * 106UL + 128UL) >> 8U);
}

uint32_t phiIncrement(uint32_t baseIncrement) {
  return baseIncrement +
         ((baseIncrement * 158UL + 128UL) >> 8U);
}

int16_t softTriangleQ3F12(uint32_t phase) {
  const uint16_t triangleQ0F12 = phase < 0x80000000UL
      ? static_cast<uint16_t>(phase >> 19U)
      : static_cast<uint16_t>((UINT32_MAX - phase) >> 19U);
  const uint16_t softenedQ0F12 = ambientmath::smoothstepQ0F12(triangleQ0F12);
  return static_cast<int16_t>(static_cast<int32_t>(softenedQ0F12) * 2L - 4096L);
}

Weights weights(uint16_t textureControl) {
  textureControl = clampAdc(textureControl);
  const uint16_t primaryTransfer = textureControl == 1023U
      ? 256U
      : static_cast<uint16_t>((static_cast<uint32_t>(textureControl) * 256U + 512U) >> 10U);
  const uint16_t secondaryTransfer = textureControl == 1023U
      ? 128U
      : static_cast<uint16_t>((static_cast<uint32_t>(textureControl) * 128U + 512U) >> 10U);
  const uint16_t primary = static_cast<uint16_t>(768U - primaryTransfer);
  const uint16_t secondary = static_cast<uint16_t>(192U + secondaryTransfer);
  return {primary, secondary, static_cast<uint16_t>(1024U - primary - secondary)};
}

uint16_t mixToDac12(int16_t primaryQ3F12,
                    int16_t secondaryQ3F12,
                    int16_t tertiaryQ3F12,
                    const Weights& selectedWeights) {
  const int32_t weighted =
      static_cast<int32_t>(primaryQ3F12) * selectedWeights.primary +
      static_cast<int32_t>(secondaryQ3F12) * selectedWeights.secondary +
      static_cast<int32_t>(tertiaryQ3F12) * selectedWeights.tertiary;
  const int32_t mixedQ3F12 = weighted / 1024L;
  const int32_t projected = (mixedQ3F12 + 4096L) / 2L;
  if (projected <= 0L) {
    return 0U;
  }
  return static_cast<uint16_t>(projected >= 4095L ? 4095L : projected);
}

}  // namespace fmd::currentmath

namespace fmd::anchormath {

uint16_t spreadQ1F15(uint16_t textureControl) {
  textureControl = clampAdc(textureControl);
  if (textureControl == 1023U) {
    return kMaximumSpreadQ1F15;
  }
  return static_cast<uint16_t>(
      (static_cast<uint32_t>(textureControl) * kMaximumSpreadQ1F15 + 512U) >> 10U);
}

uint32_t reversionAlphaQ0F24(uint32_t ambientIncrement) {
  const uint32_t alpha = static_cast<uint32_t>((ambientIncrement + 128U) >> 8U);
  return alpha == 0U ? 1U : alpha;
}

int16_t revertTowardZero(int16_t stateQ1F15,
                         uint32_t alphaQ0F24,
                         uint32_t& fractionalResidualQ0F24,
                         int8_t& residualDirection) {
  if (stateQ1F15 == 0) {
    fractionalResidualQ0F24 = 0U;
    residualDirection = 0;
    return 0;
  }

  const int8_t direction = stateQ1F15 > 0 ? static_cast<int8_t>(1) : static_cast<int8_t>(-1);
  if (residualDirection != direction) {
    fractionalResidualQ0F24 = 0U;
    residualDirection = direction;
  }

  const uint32_t magnitude = stateQ1F15 == INT16_MIN
      ? 32768UL
      : static_cast<uint32_t>(stateQ1F15 > 0 ? stateQ1F15 : -stateQ1F15);
  const uint32_t weighted = magnitude * alphaQ0F24 + fractionalResidualQ0F24;
  const uint32_t integerMovement = weighted >> 24U;
  fractionalResidualQ0F24 = weighted & 0x00FFFFFFUL;
  const uint32_t remaining = integerMovement >= magnitude ? 0U : magnitude - integerMovement;

  if (remaining == 0U) {
    fractionalResidualQ0F24 = 0U;
    residualDirection = 0;
    return 0;
  }
  if (direction > 0) {
    return static_cast<int16_t>(remaining > 32767U ? 32767U : remaining);
  }
  return remaining >= 32768U ? INT16_MIN : static_cast<int16_t>(-static_cast<int32_t>(remaining));
}

int16_t scaledInnovationQ1F15(int16_t triangularSampleQ1F15,
                             uint16_t innovationGainQ1F15,
                             uint16_t targetSpreadQ1F15) {
  const int32_t speedCompensated =
      (static_cast<int32_t>(triangularSampleQ1F15) * innovationGainQ1F15) >> 15U;
  const int32_t spreadCompensated =
      (speedCompensated * targetSpreadQ1F15) >> 15U;
  if (spreadCompensated < INT16_MIN) {
    return INT16_MIN;
  }
  if (spreadCompensated > INT16_MAX) {
    return INT16_MAX;
  }
  return static_cast<int16_t>(spreadCompensated);
}

int16_t addInnovationSaturating(int16_t stateQ1F15, int16_t innovationQ1F15) {
  const int32_t result = static_cast<int32_t>(stateQ1F15) + innovationQ1F15;
  if (result < INT16_MIN) {
    return INT16_MIN;
  }
  if (result > INT16_MAX) {
    return INT16_MAX;
  }
  return static_cast<int16_t>(result);
}

uint16_t projectToDac12(int16_t stateQ1F15) {
  return static_cast<uint16_t>(
      static_cast<uint32_t>(static_cast<int32_t>(stateQ1F15) + 32768L) >> 4U);
}

}  // namespace fmd::anchormath

namespace fmd::breathmath {

uint16_t variedParameter(uint16_t randomWord,
                         uint16_t textureControl,
                         uint16_t minimumValue,
                         uint16_t nominalValue,
                         uint16_t maximumValue) {
  textureControl = clampAdc(textureControl);
  if (textureControl == 0U || minimumValue >= maximumValue) {
    return nominalValue;
  }

  const int32_t centered = static_cast<int32_t>(randomWord) - 32768L;
  const int32_t textured = textureControl == 1023U
      ? centered
      : (centered * static_cast<int32_t>(textureControl)) >> 10U;

  if (textured >= 0) {
    const uint32_t span = static_cast<uint32_t>(maximumValue - nominalValue);
    const uint32_t delta =
        (static_cast<uint32_t>(textured) * span + 16384U) >> 15U;
    const uint32_t result = static_cast<uint32_t>(nominalValue) + delta;
    return static_cast<uint16_t>(result > maximumValue ? maximumValue : result);
  }

  const uint32_t span = static_cast<uint32_t>(nominalValue - minimumValue);
  const uint32_t magnitude = static_cast<uint32_t>(-textured);
  const uint32_t delta = (magnitude * span + 16384U) >> 15U;
  const uint32_t negativeSpan = static_cast<uint32_t>(nominalValue - minimumValue);
  return delta >= negativeSpan
      ? minimumValue
      : static_cast<uint16_t>(nominalValue - delta);
}

uint16_t rateScaleQ10(uint16_t durationQ10) {
  if (durationQ10 < kDurationMinimumQ10) {
    durationQ10 = kDurationMinimumQ10;
  } else if (durationQ10 > kDurationMaximumQ10) {
    durationQ10 = kDurationMaximumQ10;
  }
  return static_cast<uint16_t>((1048576UL + durationQ10 / 2U) / durationQ10);
}

uint32_t scaledPhaseIncrement(uint32_t ambientIncrement, uint16_t rateScale) {
  if (rateScale >= 1024U) {
    const uint16_t excess = static_cast<uint16_t>(rateScale - 1024U);
    return ambientIncrement +
           ((ambientIncrement * static_cast<uint32_t>(excess) + 512U) >> 10U);
  }
  const uint16_t reduction = static_cast<uint16_t>(1024U - rateScale);
  const uint32_t delta =
      (ambientIncrement * static_cast<uint32_t>(reduction) + 512U) >> 10U;
  return delta >= ambientIncrement ? 0U : ambientIncrement - delta;
}

uint16_t segmentReciprocalQ12(uint16_t segmentLengthQ0F12) {
  if (segmentLengthQ0F12 == 0U) {
    return 0U;
  }
  return static_cast<uint16_t>((16777216UL + segmentLengthQ0F12 / 2U) / segmentLengthQ0F12);
}

uint16_t envelopeQ0F12(uint16_t phaseQ0F12,
                       uint16_t skewQ0F12,
                       uint16_t attackReciprocalQ12,
                       uint16_t releaseReciprocalQ12) {
  if (phaseQ0F12 > 4095U) {
    phaseQ0F12 = 4095U;
  }
  if (skewQ0F12 < kSkewMinimumQ0F12) {
    skewQ0F12 = kSkewMinimumQ0F12;
  } else if (skewQ0F12 > kSkewMaximumQ0F12) {
    skewQ0F12 = kSkewMaximumQ0F12;
  }

  if (phaseQ0F12 < skewQ0F12) {
    uint32_t normalized =
        (static_cast<uint32_t>(phaseQ0F12) * attackReciprocalQ12 + 2048U) >> 12U;
    if (normalized > 4096U) {
      normalized = 4096U;
    }
    return ambientmath::smoothstepQ0F12(static_cast<uint16_t>(normalized));
  }

  const uint16_t releasePhase = static_cast<uint16_t>(phaseQ0F12 - skewQ0F12);
  uint32_t normalized =
      (static_cast<uint32_t>(releasePhase) * releaseReciprocalQ12 + 2048U) >> 12U;
  if (normalized > 4096U) {
    normalized = 4096U;
  }
  return static_cast<uint16_t>(4096U -
      ambientmath::smoothstepQ0F12(static_cast<uint16_t>(normalized)));
}

uint16_t applyAmplitude(uint16_t envelopeQ0F12, uint16_t amplitudeDac12) {
  if (envelopeQ0F12 > 4096U) {
    envelopeQ0F12 = 4096U;
  }
  if (amplitudeDac12 > 4095U) {
    amplitudeDac12 = 4095U;
  }
  return static_cast<uint16_t>(
      (static_cast<uint32_t>(envelopeQ0F12) * amplitudeDac12 + 2048U) >> 12U);
}

}  // namespace fmd::breathmath

namespace fmd::fogmath {

uint16_t kernelQ0F12(uint16_t phaseQ0F12) {
  if (phaseQ0F12 == 0U || phaseQ0F12 >= 4095U) {
    return 0U;
  }
  const uint16_t inverse = static_cast<uint16_t>(4096U - phaseQ0F12);
  const uint32_t productQ0F12 =
      (static_cast<uint32_t>(phaseQ0F12) * inverse) >> 12U;
  const uint32_t result = (productQ0F12 * productQ0F12) >> 8U;
  return static_cast<uint16_t>(result > 4096U ? 4096U : result);
}

uint8_t targetOccupancyEighths(uint16_t textureControl) {
  textureControl = clampAdc(textureControl);
  if (textureControl == 1023U) {
    return kMaximumOccupancyEighths;
  }
  const uint16_t span = static_cast<uint16_t>(
      kMaximumOccupancyEighths - kMinimumOccupancyEighths);
  return static_cast<uint8_t>(kMinimumOccupancyEighths +
      ((static_cast<uint32_t>(textureControl) * span + 512U) >> 10U));
}

uint32_t eventCutoffQ0F32(uint32_t ambientIncrement, uint8_t occupancyEighths) {
  if (occupancyEighths > kMaximumOccupancyEighths) {
    occupancyEighths = kMaximumOccupancyEighths;
  }
  return static_cast<uint32_t>(
      (ambientIncrement * static_cast<uint32_t>(occupancyEighths) + 4U) >> 3U);
}

int16_t amplitudeFromRandom(uint16_t randomWord) {
  const int16_t magnitude = static_cast<int16_t>(512U + (randomWord & 0x01FFU));
  return (randomWord & 0x8000U) != 0U ? static_cast<int16_t>(-magnitude) : magnitude;
}

int16_t voiceContributionAndAdvance(Voice& voice, uint32_t phaseIncrement) {
  if (!voice.active) {
    return 0;
  }

  const uint16_t kernel = kernelQ0F12(static_cast<uint16_t>(voice.phase >> 20U));
  const int16_t contribution = static_cast<int16_t>(
      (static_cast<int32_t>(voice.amplitude) * kernel) >> 12U);
  const uint32_t advanced = static_cast<uint32_t>(voice.phase + phaseIncrement);
  if (advanced < voice.phase) {
    voice.phase = 0U;
    voice.active = false;
  } else {
    voice.phase = advanced;
  }
  return contribution;
}

uint16_t projectToDac12(int32_t signedContribution) {
  const int32_t projected = 2048L + signedContribution;
  if (projected <= 0L) {
    return 0U;
  }
  return static_cast<uint16_t>(projected >= 4095L ? 4095L : projected);
}

}  // namespace fmd::fogmath
