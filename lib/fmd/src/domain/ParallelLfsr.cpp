#include "fmd/domain/ParallelLfsr.h"
namespace fmd {
namespace {
constexpr uint8_t kTaps1[4] = {16, 14, 13, 11};
constexpr uint8_t kTaps2[4] = {16, 15, 13, 4};
}
ParallelLfsr::ParallelLfsr(uint16_t seed)
    : state1_(seed),
      state2_(static_cast<uint16_t>(~static_cast<uint16_t>(
          static_cast<uint16_t>((seed >> 8U) & 0x00FFU) | static_cast<uint16_t>(seed << 8U)))) {}
uint16_t ParallelLfsr::step(uint16_t state, const uint8_t taps[4]) {
  uint16_t bit = 0;
  for (uint8_t i=0; i<4; ++i) bit ^= static_cast<uint16_t>(state >> static_cast<uint8_t>(16U - taps[i]));
  return static_cast<uint16_t>((state >> 1U) | static_cast<uint16_t>(bit << 15U));
}
uint16_t ParallelLfsr::next() {
  state1_ = step(state1_, kTaps1);
  state2_ = step(state2_, kTaps2);
  return static_cast<uint16_t>(state1_ ^ state2_);
}
}  // namespace fmd
