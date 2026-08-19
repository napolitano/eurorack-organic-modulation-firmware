/**
 * @file FixedMath.cpp
 * Implements fixed-point arithmetic primitives shared by Drift algorithms.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "fmd/domain/FixedMath.h"

#include <stdint.h>

namespace fmd::fixedmath {

int16_t mulI1F15(int16_t firstQ1F15, int16_t secondQ1F15) {
  const int32_t fullPrecisionProduct =
      static_cast<int32_t>(firstQ1F15) * static_cast<int32_t>(secondQ1F15);
  return static_cast<int16_t>(fullPrecisionProduct >> 15U);
}

uint16_t lerpU0F16(uint16_t weightQ0F16,
                    uint16_t startValue,
                    uint16_t endValue) {
  const int32_t endpointDifference =
      static_cast<int32_t>(endValue) - static_cast<int32_t>(startValue);
  const int64_t weightedDifference =
      static_cast<int64_t>(endpointDifference) * weightQ0F16;
  const int32_t interpolatedDelta = static_cast<int32_t>(weightedDifference >> 16U);
  return static_cast<uint16_t>(static_cast<int32_t>(startValue) + interpolatedDelta);
}

int16_t lerpI1F15(int16_t weightQ1F15,
                   int16_t startQ1F15,
                   int16_t endQ1F15) {
  const int32_t endpointDifference =
      static_cast<int32_t>(endQ1F15) - static_cast<int32_t>(startQ1F15);
  const int64_t weightedDifference =
      static_cast<int64_t>(endpointDifference) * static_cast<int32_t>(weightQ1F15);
  const int32_t interpolatedDelta = static_cast<int32_t>(weightedDifference >> 15U);
  return static_cast<int16_t>(static_cast<int32_t>(startQ1F15) + interpolatedDelta);
}

uint16_t lerpU4F12(uint16_t weightQ4F12,
                    uint16_t startValue,
                    uint16_t endValue) {
  const int32_t endpointDifference =
      static_cast<int32_t>(endValue) - static_cast<int32_t>(startValue);
  const int64_t weightedDifference =
      static_cast<int64_t>(endpointDifference) * weightQ4F12;
  const int32_t interpolatedDelta = static_cast<int32_t>(weightedDifference >> 12U);
  return static_cast<uint16_t>(static_cast<int32_t>(startValue) + interpolatedDelta);
}

uint32_t lerpU16F16(uint32_t weightQ0F16,
                     uint32_t startQ16F16,
                     uint32_t endQ16F16) {
  if (endQ16F16 >= startQ16F16) {
    const uint64_t difference = static_cast<uint64_t>(endQ16F16 - startQ16F16);
    return startQ16F16 +
           static_cast<uint32_t>((difference * weightQ0F16) >> 16U);
  }

  const uint64_t difference = static_cast<uint64_t>(startQ16F16 - endQ16F16);
  return startQ16F16 -
         static_cast<uint32_t>((difference * weightQ0F16) >> 16U);
}

}  // namespace fmd::fixedmath
