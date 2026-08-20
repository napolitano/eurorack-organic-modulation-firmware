/**
 * @file AlgorithmBankConfig.h
 * Defines compile-time selection of the four-algorithm firmware bank.
 *
 * @details
 * Bank selection is fixed when the firmware image is compiled. The two physical
 * rear DIP inputs can represent only four slots, so they select algorithms inside
 * the chosen bank rather than selecting among banks. Release packaging builds one
 * image per bank and supported Nano bootloader.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_CONFIG_ALGORITHM_BANK_CONFIG_H
#define FMD_CONFIG_ALGORITHM_BANK_CONFIG_H

#include <stdint.h>

/** Classic bank containing the four algorithms derived from the original Drift firmware. */
#define FMD_BANK_CLASSIC 0
/** Alternative organic bank containing Fractal, Vector, Rain and Attractor. */
#define FMD_BANK_ORGANIC 1
/** Alternative generative bank containing Turing, Markov, Motif and Urn. */
#define FMD_BANK_GENERATIVE 2
/** Alternative ambient bank containing Current, Anchor, Breath and Fog. */
#define FMD_BANK_AMBIENT 3
/** Alternative Electronica bank containing Pump, Acid, Shuffle and Polymeter. */
#define FMD_BANK_ELECTRONICA 4
/** Alternative Percussion bank containing Euclid, Repeat, Probability and Humanize. */
#define FMD_BANK_PERCUSSION 5
/** Dubstep/Bass bank containing Wobble, Growl, Chop and Build. */
#define FMD_BANK_DUBSTEP 6

#ifndef FMD_ALGORITHM_BANK
/** Default to the original/classic bank when no compiler flag selects another bank. */
#define FMD_ALGORITHM_BANK FMD_BANK_CLASSIC
#endif

#if FMD_ALGORITHM_BANK != FMD_BANK_CLASSIC && \
    FMD_ALGORITHM_BANK != FMD_BANK_ORGANIC && \
    FMD_ALGORITHM_BANK != FMD_BANK_GENERATIVE && \
    FMD_ALGORITHM_BANK != FMD_BANK_AMBIENT && \
    FMD_ALGORITHM_BANK != FMD_BANK_ELECTRONICA && \
    FMD_ALGORITHM_BANK != FMD_BANK_PERCUSSION && \
    FMD_ALGORITHM_BANK != FMD_BANK_DUBSTEP
#error "FMD_ALGORITHM_BANK must be Classic (0), Organic (1), Generative (2), Ambient (3), Electronica (4), Percussion (5), or Dubstep (6)"
#endif

namespace fmd {

/** @brief Compile-time selectable set of four algorithms exposed by the rear DIP switches. */
enum class AlgorithmBank : uint8_t {
  Classic = FMD_BANK_CLASSIC,       ///< Perlin, Brownian, Bezier and LFO.
  Organic = FMD_BANK_ORGANIC,       ///< Fractal, Vector, Rain and Hénon attractor.
  Generative = FMD_BANK_GENERATIVE, ///< Turing, Markov, Motif and Urn.
  Ambient = FMD_BANK_AMBIENT,       ///< Current, Anchor, Breath and Fog.
  Electronica = FMD_BANK_ELECTRONICA, ///< Pump, Acid, Shuffle and Polymeter.
  Percussion = FMD_BANK_PERCUSSION, ///< Euclid, Repeat, Probability and Humanize.
  Dubstep = FMD_BANK_DUBSTEP ///< Wobble, Growl, Chop and Build.
};

/** @brief Bank compiled into this firmware image; constant for the entire binary. */
constexpr AlgorithmBank kSelectedAlgorithmBank =
    static_cast<AlgorithmBank>(FMD_ALGORITHM_BANK);

}  // namespace fmd
#endif  // FMD_CONFIG_ALGORITHM_BANK_CONFIG_H
