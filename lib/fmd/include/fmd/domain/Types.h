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

#include "fmd/config/AlgorithmTargetConfig.h"

namespace fmd {

/**
 * @brief Complete algorithm identity set supported by this source tree.
 *
 * A firmware image exposes exactly four entries from one compile-time-selected
 * AlgorithmBank. Keeping identities distinct allows native tests and developer
 * tooling to describe all banks without changing the hardware DIP interface.
 */
enum class Algorithm : uint8_t {
  Perlin = FMD_ALGORITHM_PERLIN,     ///< Classic: two-octave gradient-noise modulation; default.
  Brownian = FMD_ALGORITHM_BROWNIAN,   ///< Classic: bounded random walk with first-order smoothing.
  Bezier = FMD_ALGORITHM_BEZIER,     ///< Classic: random destinations joined by morphed cubic curves.
  Lfo = FMD_ALGORITHM_LFO,        ///< Classic: deterministic skewable triangle / saw LFO.
  Fractal = FMD_ALGORITHM_FRACTAL,    ///< Organic: three-scale procedural gradient-noise fractal.
  Vector = FMD_ALGORITHM_VECTOR,     ///< Organic: cross-coupled two-dimensional toroidal phase flow.
  Rain = FMD_ALGORITHM_RAIN,       ///< Organic: stochastic impulse / decaying-envelope process.
  Attractor = FMD_ALGORITHM_ATTRACTOR,  ///< Organic: smoothed Hénon-map traversal.
  Turing = FMD_ALGORITHM_TURING,     ///< Generative: mutating 16-bit shift-register loop.
  Markov = FMD_ALGORITHM_MARKOV,     ///< Generative: finite-state stochastic transition grammar.
  Motif = FMD_ALGORITHM_MOTIF,     ///< Generative: explicit phrase with structural transformations.
  Urn = FMD_ALGORITHM_URN,       ///< Generative: bounded leaky reinforced-state process.
  Current = FMD_ALGORITHM_CURRENT,   ///< Ambient: deterministic quasi-periodic-inspired long motion.
  Anchor = FMD_ALGORITHM_ANCHOR,    ///< Ambient: bounded mean-reverting stochastic modulation.
  Breath = FMD_ALGORITHM_BREATH,    ///< Ambient: recurrent smooth swells with cycle variation.
  Fog = FMD_ALGORITHM_FOG,       ///< Ambient: overlapping smooth bipolar stochastic cloudlets.
  Pump = FMD_ALGORITHM_PUMP,      ///< Electronica: free-running duck/recovery contour.
  Acid = FMD_ALGORITHM_ACID,      ///< Electronica: deterministic accented/sliding 16-step contour.
  Shuffle = FMD_ALGORITHM_SHUFFLE,   ///< Electronica: deterministic long/short timing modulation.
  Polymeter = FMD_ALGORITHM_POLYMETER, ///< Electronica: four-against-odd-meter accent process.
  Euclid = FMD_ALGORITHM_EUCLID,     ///< Percussion: phrase-aware 16-step Euclidean pulse rhythm.
  Repeat = FMD_ALGORITHM_REPEAT,     ///< Percussion: quarter-note anchors with ratchet clusters.
  Probability = FMD_ALGORITHM_PROBABILITY,///< Percussion: metrically weighted stochastic pulse rhythm.
  Humanize = FMD_ALGORITHM_HUMANIZE,   ///< Percussion: bounded timing/amplitude variation.
  Wobble = FMD_ALGORITHM_WOBBLE,       ///< Dubstep/Bass: deterministic tempo-synchronised rate phrase.
  Growl = FMD_ALGORITHM_GROWL,         ///< Dubstep/Bass: multi-lobed timbral-motion CV gesture.
  Chop = FMD_ALGORITHM_CHOP,           ///< Dubstep/Bass: sparse deterministic syncopated articulation.
  Build = FMD_ALGORITHM_BUILD          ///< Dubstep/Bass: multi-bar rising and accelerating tension contour.
};

/**
 * @brief Coherent 10-bit control snapshot consumed by one algorithm step.
 *
 * All fields are expected in the AVR ADC domain 0..1023. Public mathematical
 * helpers clamp defensive out-of-range inputs where required by their contract.
 */
struct ControlFrame {
  uint16_t speedCv;      ///< Speed CV ADC code; Percussion and Dubstep/Bass interpret it as 0..5 V clock input.
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
