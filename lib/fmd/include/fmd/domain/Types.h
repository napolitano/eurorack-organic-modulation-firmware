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

#include "fmd/config/AlgorithmBankConfig.h"

namespace fmd {

/**
 * @brief Complete algorithm identity set supported by this source tree.
 *
 * A firmware image exposes exactly four entries from one compile-time-selected
 * AlgorithmBank. Keeping identities distinct allows native tests and developer
 * tooling to describe all banks without changing the hardware DIP interface.
 */
enum class Algorithm : uint8_t {
  Perlin = 0,     ///< Classic: two-octave gradient-noise modulation; default.
  Brownian = 1,   ///< Classic: bounded random walk with first-order smoothing.
  Bezier = 2,     ///< Classic: random destinations joined by morphed cubic curves.
  Lfo = 3,        ///< Classic: deterministic skewable triangle / saw LFO.
  Fractal = 4,    ///< Organic: three-scale procedural gradient-noise fractal.
  Vector = 5,     ///< Organic: cross-coupled two-dimensional toroidal phase flow.
  Rain = 6,       ///< Organic: stochastic impulse / decaying-envelope process.
  Attractor = 7,  ///< Organic: smoothed Hénon-map traversal.
  Turing = 8,     ///< Generative: mutating 16-bit shift-register loop.
  Markov = 9,     ///< Generative: finite-state stochastic transition grammar.
  Motif = 10,     ///< Generative: explicit phrase with structural transformations.
  Urn = 11,       ///< Generative: bounded leaky reinforced-state process.
  Current = 12,   ///< Ambient: deterministic quasi-periodic-inspired long motion.
  Anchor = 13,    ///< Ambient: bounded mean-reverting stochastic modulation.
  Breath = 14,    ///< Ambient: recurrent smooth swells with cycle variation.
  Fog = 15        ///< Ambient: overlapping smooth bipolar stochastic cloudlets.
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
