/** @file Types.h @brief Portable Drift domain types. SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef FMD_DOMAIN_TYPES_H
#define FMD_DOMAIN_TYPES_H
#include <stdint.h>
namespace fmd {
enum class Algorithm : uint8_t { Perlin = 0, Brownian = 1, Bezier = 2, Lfo = 3 };
struct ControlFrame {
  uint16_t speedCv;
  uint16_t textureCv;
  uint16_t speedKnob;
  uint16_t textureKnob;
};
constexpr uint16_t clampAdc(uint16_t value) { return value > 1023U ? 1023U : value; }
constexpr uint16_t sumAdc(uint16_t a, uint16_t b) {
  const uint16_t sum = static_cast<uint16_t>(a + b);
  return sum > 1023U ? 1023U : sum;
}
}  // namespace fmd
#endif
