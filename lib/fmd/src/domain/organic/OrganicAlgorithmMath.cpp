/**
 * @file OrganicAlgorithmMath.cpp
 * Implements pure mathematical primitives used by the optional Organic algorithm bank.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "fmd/domain/organic/OrganicAlgorithmMath.h"

#include "fmd/domain/Types.h"

#include <stdint.h>

namespace fmd::fractalmath {

OctaveWeights octaveWeights(uint16_t textureControl) {
  textureControl = clampAdc(textureControl);

  // The endpoint weights sum to 1024. Interpolate the two added-detail
  // weights independently, then derive the macro weight as the exact remainder
  // so quantisation can never change the total gain.
  // The 10-bit ADC domain is almost exactly a /1024 fraction. Using a
  // rounded power-of-two scale avoids an AVR software division in the
  // per-sample path while still reaching both documented endpoint weights.
  const uint16_t mesoWeight = static_cast<uint16_t>(
      (static_cast<uint32_t>(textureControl) * 320U + 512U) >> 10U);
  const uint16_t detailWeight = static_cast<uint16_t>(
      (static_cast<uint32_t>(textureControl) * 192U + 512U) >> 10U);
  const uint16_t macroWeight =
      static_cast<uint16_t>(1024U - mesoWeight - detailWeight);
  return {macroWeight, mesoWeight, detailWeight};
}

int16_t mixOctavesQ1F15(int16_t macroQ1F15,
                        int16_t mesoQ1F15,
                        int16_t detailQ1F15,
                        const OctaveWeights& weights) {
  const int32_t weightedSum =
      static_cast<int32_t>(macroQ1F15) * weights.macro +
      static_cast<int32_t>(mesoQ1F15) * weights.meso +
      static_cast<int32_t>(detailQ1F15) * weights.detail;
  return static_cast<int16_t>(weightedSum / 1024);
}

}  // namespace fmd::fractalmath

namespace fmd::vectormath {

int16_t triangleSignedQ1F15(uint16_t phaseQ0F16) {
  const uint16_t halfPhase = static_cast<uint16_t>(phaseQ0F16 & 0x7FFFU);
  if ((phaseQ0F16 & 0x8000U) == 0U) {
    return static_cast<int16_t>(
        -32768L + static_cast<int32_t>(halfPhase) * 2L);
  }

  return static_cast<int16_t>(
      32767L - static_cast<int32_t>(halfPhase) * 2L);
}

uint32_t coupledPhaseIncrement(uint32_t basePhaseIncrement,
                               int16_t otherAxisWaveQ1F15,
                               uint16_t textureControl,
                               bool invertCoupling) {
  textureControl = clampAdc(textureControl);

  // (wave * texture) >> 11 is at most about +/-16368, i.e. +/-0.5 in Q1.15.
  // Multiplying by (base >> 16) therefore yields approximately +/-25% of
  // basePhaseIncrement while staying inside signed 32-bit arithmetic even at
  // the fastest supported classic speed-map endpoint.
  int32_t scaledCrossAxis =
      (static_cast<int32_t>(otherAxisWaveQ1F15) * textureControl) >> 11U;
  if (invertCoupling) {
    scaledCrossAxis = -scaledCrossAxis;
  }

  const int32_t signedModulation =
      static_cast<int32_t>(basePhaseIncrement >> 16U) * scaledCrossAxis;
  if (signedModulation >= 0) {
    const uint32_t positiveModulation = static_cast<uint32_t>(signedModulation);
    return UINT32_MAX - basePhaseIncrement < positiveModulation
        ? UINT32_MAX
        : basePhaseIncrement + positiveModulation;
  }

  const uint32_t negativeMagnitude =
      static_cast<uint32_t>(-signedModulation);
  return negativeMagnitude >= basePhaseIncrement
      ? 0U
      : basePhaseIncrement - negativeMagnitude;
}

uint16_t projectToDac12(int16_t xWaveQ1F15, int16_t yWaveQ1F15) {
  const int32_t projectedQ1F15 =
      (static_cast<int32_t>(xWaveQ1F15) + static_cast<int32_t>(yWaveQ1F15)) / 2L;
  const uint32_t biased = static_cast<uint32_t>(projectedQ1F15 + 32768L);
  const uint32_t outputCode = biased >> 4U;
  return static_cast<uint16_t>(outputCode > 4095U ? 4095U : outputCode);
}

}  // namespace fmd::vectormath

namespace fmd::rainmath {

uint16_t eventCutoff(uint16_t densityControl) {
  densityControl = clampAdc(densityControl);
  const uint32_t squaredDensity =
      static_cast<uint32_t>(densityControl) * densityControl;
  return static_cast<uint16_t>(squaredDensity >> 6U);
}

uint16_t decayAlphaQ0F16(uint16_t speedControl) {
  speedControl = clampAdc(speedControl);
  // Eight Q0.16 codes per ADC step gives a simple monotonic control law with
  // no division or wide multiply in the 2.5 kHz sample path.
  return static_cast<uint16_t>(
      kMinDecayAlphaQ0F16 + static_cast<uint16_t>(speedControl << 3U));
}

uint16_t impulseAmplitude(uint16_t randomValue) {
  // 1/16..approximately 5/16 of full-scale envelope. Dense rain can therefore
  // accumulate naturally instead of every event immediately saturating output.
  return static_cast<uint16_t>(4096U + (randomValue & 0x3FFFU));
}

void decayEnvelope(uint16_t alphaQ0F16,
                   uint16_t& envelopeValue,
                   uint16_t& decayResidualQ0F16) {
  if (envelopeValue == 0U) {
    decayResidualQ0F16 = 0U;
    return;
  }

  const uint32_t fullPrecisionDecay =
      static_cast<uint32_t>(envelopeValue) * alphaQ0F16 + decayResidualQ0F16;
  uint16_t integerDecay = static_cast<uint16_t>(fullPrecisionDecay >> 16U);
  decayResidualQ0F16 = static_cast<uint16_t>(fullPrecisionDecay & 0xFFFFU);

  if (integerDecay >= envelopeValue) {
    envelopeValue = 0U;
    decayResidualQ0F16 = 0U;
    return;
  }

  envelopeValue = static_cast<uint16_t>(envelopeValue - integerDecay);
}

uint16_t addImpulseSaturating(uint16_t envelopeValue, uint16_t impulseValue) {
  const uint32_t summedValue =
      static_cast<uint32_t>(envelopeValue) + impulseValue;
  return summedValue > 0xFFFFU
      ? 0xFFFFU
      : static_cast<uint16_t>(summedValue);
}

}  // namespace fmd::rainmath

namespace fmd::attractormath {
namespace {

int16_t clampQ2F14ToInt16(int32_t value) {
  if (value < -32768L) {
    return INT16_MIN;
  }
  if (value > 32767L) {
    return INT16_MAX;
  }
  return static_cast<int16_t>(value);
}

}  // namespace

uint16_t parameterAQ2F14(uint16_t textureControl) {
  textureControl = clampAdc(textureControl);
  if (textureControl == 1023U) {
    return kMaxParameterAQ2F14;
  }

  const uint32_t parameterSpan =
      static_cast<uint32_t>(kMaxParameterAQ2F14 - kMinParameterAQ2F14);
  // Approximate division by the 10-bit full-scale value with /1024. The
  // explicit endpoint above retains the exact documented 1.40 setting while
  // avoiding an AVR software division on every sample.
  const uint32_t interpolated =
      (parameterSpan * textureControl + 512U) >> 10U;
  return static_cast<uint16_t>(kMinParameterAQ2F14 + interpolated);
}

HenonState iterateHenon(const HenonState& state, uint16_t parameterAQ2F14) {
  constexpr int32_t kOneQ2F14 = 16384L;

  // Q2.14 * Q2.14 -> Q4.28. Shift once before applying parameter a so both
  // multiplications fit comfortably in signed 32-bit arithmetic on AVR.
  const int32_t xSquaredQ2F14 =
      (static_cast<int32_t>(state.xQ2F14) * state.xQ2F14 + (INT32_C(1) << 13U)) >> 14U;
  const int32_t scaledSquareQ2F14 =
      (xSquaredQ2F14 * static_cast<int32_t>(parameterAQ2F14) + (INT32_C(1) << 13U)) >> 14U;
  const int32_t nextXQ2F14 =
      kOneQ2F14 + static_cast<int32_t>(state.yQ2F14) - scaledSquareQ2F14;
  const int32_t nextYQ2F14 =
      (static_cast<int32_t>(state.xQ2F14) * kParameterBQ2F14) >> 14U;

  return {
      clampQ2F14ToInt16(nextXQ2F14),
      clampQ2F14ToInt16(nextYQ2F14),
  };
}

int16_t interpolateQ2F14(int16_t startQ2F14,
                         int16_t endQ2F14,
                         uint16_t phaseQ0F12) {
  if (phaseQ0F12 > 4095U) {
    phaseQ0F12 = 4095U;
  }
  const int32_t difference =
      static_cast<int32_t>(endQ2F14) - startQ2F14;
  const int32_t interpolatedDelta =
      (difference * static_cast<int32_t>(phaseQ0F12)) >> 12U;
  return clampQ2F14ToInt16(
      static_cast<int32_t>(startQ2F14) + interpolatedDelta);
}

uint16_t coordinateToDac12(int16_t coordinateQ2F14) {
  const int32_t biased = static_cast<int32_t>(coordinateQ2F14) + 32768L;
  if (biased <= 0L) {
    return 0U;
  }
  if (biased >= 65535L) {
    return 4095U;
  }
  return static_cast<uint16_t>(static_cast<uint32_t>(biased) >> 4U);
}

}  // namespace fmd::attractormath
