/**
 * @file Types.h
 * Defines portable Drift domain types and ADC-domain helpers.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_DOMAIN_TYPES_H
#define FMD_DOMAIN_TYPES_H

#include <stdint.h>

namespace fmd {

/** @brief Modulation algorithms selectable by the rear configuration switches. */
enum class Algorithm : uint8_t {
  Perlin = 0,    ///< Two-octave gradient-noise modulation; hardware default.
  Brownian = 1,  ///< Bounded random walk with first-order smoothing.
  Bezier = 2,    ///< Random destinations joined by continuously morphed curves.
  Lfo = 3        ///< Deterministic skewable triangle / saw LFO.
};

/**
 * @brief Coherent 10-bit control snapshot consumed by one algorithm step.
 *
 * All fields are expected in the AVR ADC domain 0..1023. Public mathematical
 * helpers clamp defensive out-of-range inputs where required by their contract.
 */
struct ControlFrame {
  uint16_t speedCv;      ///< Speed CV input, ADC code 0..1023.
  uint16_t textureCv;    ///< Texture CV input, ADC code 0..1023.
  uint16_t speedKnob;    ///< Speed potentiometer, ADC code 0..1023.
  uint16_t textureKnob;  ///< Texture potentiometer, ADC code 0..1023.
};

/**
 * @brief Clamp a value to the 10-bit ADC domain.
 * @param value Raw value to clamp.
 * @return value limited to 0..1023.
 */
constexpr uint16_t clampAdc(uint16_t value) {
  return value > 1023U ? 1023U : value;
}

/**
 * @brief Add two ADC-domain controls with saturation at full scale.
 * @param first First 10-bit control value.
 * @param second Second 10-bit control value.
 * @return Saturating sum in the range 0..1023.
 */
constexpr uint16_t sumAdc(uint16_t first, uint16_t second) {
  const uint16_t summedValue = static_cast<uint16_t>(first + second);
  return summedValue > 1023U ? 1023U : summedValue;
}

}  // namespace fmd
#endif  // FMD_DOMAIN_TYPES_H
