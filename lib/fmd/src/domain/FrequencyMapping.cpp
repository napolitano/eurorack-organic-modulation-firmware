/**
 * @file FrequencyMapping.cpp
 * Implements Drift's exponential Speed/CV mapping and phase-increment conversion.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "fmd/domain/FrequencyMapping.h"

#include "fmd/domain/FixedMath.h"
#include "fmd/domain/Types.h"

#include <stdint.h>

namespace fmd {
namespace {

/**
 * @brief Interpolate the 256-entry base-2 exponential table.
 * @param referenceTables Table provider.
 * @param lookupPosition Unsigned 16-bit table position; high byte selects entry.
 * @return Interpolated exponential value in Q16.16.
 */
uint32_t interpolateExp2Q16F16(const IReferenceTables& referenceTables,
                               uint16_t lookupPosition) {
  const uint8_t lowerIndex = static_cast<uint8_t>(lookupPosition >> 8U);
  const uint8_t upperIndex = lowerIndex == 255U
      ? 255U
      : static_cast<uint8_t>(lowerIndex + 1U);
  const uint32_t interpolationWeightQ0F16 =
      static_cast<uint32_t>(static_cast<uint16_t>(lookupPosition << 8U));

  return fixedmath::lerpU16F16(interpolationWeightQ0F16,
                               referenceTables.exp2Q16_16(lowerIndex),
                               referenceTables.exp2Q16_16(upperIndex));
}

}  // namespace

uint32_t phaseIncrementFromDecihertzQ16_16(uint32_t decihertzQ16F16) {
  // Exact rounded conversion:
  //   phaseIncrement = round(decihertzQ16F16 * 65536 / 100000).
  //
  // The reciprocal is intentionally floored. Therefore the first estimate can
  // only equal the exact rounded quotient or be one code low. A single bounded
  // correction removes that possible error without AVR 64-bit division.
  constexpr uint32_t kReciprocalScaleQ0F32 = 2814749767UL;
  constexpr uint32_t kDecihertzDivisor = 100000UL;
  constexpr uint32_t kRoundingHalfDivisor = kDecihertzDivisor / 2U;

  const uint64_t reciprocalProduct =
      static_cast<uint64_t>(decihertzQ16F16) * kReciprocalScaleQ0F32;
  uint32_t phaseIncrement = static_cast<uint32_t>(
      (reciprocalProduct + (UINT64_C(1) << 31U)) >> 32U);

  const uint64_t roundedNumerator =
      (static_cast<uint64_t>(decihertzQ16F16) << 16U) + kRoundingHalfDivisor;
  const uint64_t representedNumerator =
      static_cast<uint64_t>(phaseIncrement) * kDecihertzDivisor;
  const uint64_t nextRepresentedNumerator = representedNumerator + kDecihertzDivisor;

  if (nextRepresentedNumerator <= roundedNumerator) {
    ++phaseIncrement;
  }
  return phaseIncrement;
}

uint32_t phaseIncrementFromControls(const IReferenceTables& referenceTables,
                                    uint16_t speedKnobAdc,
                                    uint16_t speedCvAdc,
                                    int16_t rawFrequencyOffset) {
  /// Maximum combined control value used by the original 0..12 V knob mapping.
  constexpr uint16_t kMaximumMappedControl = static_cast<uint16_t>((1023U * 12U) / 5U);

  speedKnobAdc = clampAdc(speedKnobAdc);
  speedCvAdc = clampAdc(speedCvAdc);

  const uint16_t speedKnobMappedTo12V =
      static_cast<uint16_t>((speedKnobAdc * 12U) / 5U);
  uint16_t combinedControl =
      static_cast<uint16_t>(speedKnobMappedTo12V + speedCvAdc);

  // Bézier supplies a signed random offset in the same raw domain used by the
  // upstream firmware. Dividing by 64 converts it to the exponential-map input
  // domain before saturation.
  const int16_t scaledFrequencyOffset =
      static_cast<int16_t>(rawFrequencyOffset / 64);

  if (scaledFrequencyOffset >= 0) {
    combinedControl = static_cast<uint16_t>(
        combinedControl + static_cast<uint16_t>(scaledFrequencyOffset));
  } else {
    const uint16_t subtraction = static_cast<uint16_t>(-scaledFrequencyOffset);
    combinedControl = subtraction > combinedControl
        ? 0U
        : static_cast<uint16_t>(combinedControl - subtraction);
  }

  if (combinedControl > kMaximumMappedControl) {
    combinedControl = kMaximumMappedControl;
  }

  const uint16_t exponentialLookupPosition =
      static_cast<uint16_t>(combinedControl * 20U);
  const uint32_t frequencyDecihertzQ16F16 =
      interpolateExp2Q16F16(referenceTables, exponentialLookupPosition);
  return phaseIncrementFromDecihertzQ16_16(frequencyDecihertzQ16F16);
}

}  // namespace fmd
