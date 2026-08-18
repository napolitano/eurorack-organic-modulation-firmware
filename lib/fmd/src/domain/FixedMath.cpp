#include "fmd/domain/FixedMath.h"
#include <stdint.h>
namespace fmd::fixedmath {
int16_t mulI1F15(int16_t a, int16_t b) {
  const int32_t p = static_cast<int32_t>(a) * static_cast<int32_t>(b);
  return static_cast<int16_t>(p >> 15);
}
uint16_t lerpU0F16(uint16_t t, uint16_t a, uint16_t b) {
  const int32_t d = static_cast<int32_t>(b) - static_cast<int32_t>(a);
  const int64_t scaled = static_cast<int64_t>(d) * t;
  const int32_t delta = static_cast<int32_t>(scaled >> 16U);
  return static_cast<uint16_t>(static_cast<int32_t>(a) + delta);
}
int16_t lerpI1F15(int16_t t, int16_t a, int16_t b) {
  const int32_t d = static_cast<int32_t>(b) - static_cast<int32_t>(a);
  const int64_t scaled = static_cast<int64_t>(d) * static_cast<int32_t>(t);
  const int32_t delta = static_cast<int32_t>(scaled >> 15U);
  return static_cast<int16_t>(static_cast<int32_t>(a) + delta);
}
uint16_t lerpU4F12(uint16_t t, uint16_t a, uint16_t b) {
  const int32_t d = static_cast<int32_t>(b) - static_cast<int32_t>(a);
  const int64_t scaled = static_cast<int64_t>(d) * t;
  const int32_t delta = static_cast<int32_t>(scaled >> 12U);
  return static_cast<uint16_t>(static_cast<int32_t>(a) + delta);
}
uint32_t lerpU16F16(uint32_t t, uint32_t a, uint32_t b) {
  if (b >= a) return a + static_cast<uint32_t>((static_cast<uint64_t>(b-a) * t) >> 16U);
  return a - static_cast<uint32_t>((static_cast<uint64_t>(a-b) * t) >> 16U);
}
}  // namespace fmd::fixedmath
