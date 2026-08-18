#include "fmd/domain/FrequencyMapping.h"

#include "fmd/domain/FixedMath.h"
#include "fmd/domain/Types.h"

#include <stdint.h>

namespace fmd {
namespace {
uint32_t exp2Lookup(const IReferenceTables& tables, uint16_t raw) {
  const uint8_t low = static_cast<uint8_t>(raw >> 8U);
  const uint8_t high = low == 255U ? 255U : static_cast<uint8_t>(low + 1U);
  const uint32_t remainder = static_cast<uint32_t>(static_cast<uint16_t>(raw << 8U));
  return fixedmath::lerpU16F16(remainder,
                               tables.exp2Q16_16(low),
                               tables.exp2Q16_16(high));
}
}

uint32_t phaseIncrementFromDecihertzQ16_16(uint32_t decihertzQ16_16) {
  // Exact rounded conversion:
  //   delta = round(decihertzQ16_16 * 65536 / 100000).
  // A reciprocal multiply provides a near-exact estimate; a bounded correction
  // against the exact integer numerator removes the possible one-code error
  // without executing an AVR 64-bit division in the 2.5 kHz processing path.
  constexpr uint32_t kScaleQ0F32 = 2814749767UL;
  const uint64_t estimateProduct = static_cast<uint64_t>(decihertzQ16_16) * kScaleQ0F32;
  uint32_t quotient = static_cast<uint32_t>((estimateProduct + (UINT64_C(1) << 31U)) >> 32U);

  const uint64_t roundedNumerator = (static_cast<uint64_t>(decihertzQ16_16) << 16U) + 50000U;
  const uint64_t represented = static_cast<uint64_t>(quotient) * 100000U;
  const uint64_t nextRepresented = represented + 100000U;
  // kScaleQ0F32 is floor(exact reciprocal * 2^32), so the rounded
  // reciprocal estimate cannot exceed the rounded exact quotient for
  // non-negative inputs. It can only be one code low; correct that case.
  if (nextRepresented <= roundedNumerator) {
    ++quotient;
  }
  return quotient;
}

uint32_t getDeltaTime(const IReferenceTables& tables, uint16_t knob, uint16_t cv, int16_t offset) {
  constexpr uint16_t kMaxKnobValue = static_cast<uint16_t>((1023U * 12U) / 5U);
  knob = clampAdc(knob);
  cv = clampAdc(cv);
  const uint16_t knob12v = static_cast<uint16_t>((knob * 12U) / 5U);
  uint16_t sum = static_cast<uint16_t>(knob12v + cv);
  const int16_t scaledOffset = static_cast<int16_t>(offset / 64);

  if (scaledOffset >= 0) {
    const uint16_t add = static_cast<uint16_t>(scaledOffset);
    sum = static_cast<uint16_t>(sum + add);
  } else {
    const uint16_t sub = static_cast<uint16_t>(-scaledOffset);
    sum = sub > sum ? 0U : static_cast<uint16_t>(sum - sub);
  }

  if (sum > kMaxKnobValue) {
    sum = kMaxKnobValue;
  }

  const uint16_t expInput = static_cast<uint16_t>(sum * 20U);
  return phaseIncrementFromDecihertzQ16_16(exp2Lookup(tables, expInput));
}
}  // namespace fmd
