/**
 * @file AlgorithmTargetConfig.h
 * Defines optional compile-time locking to one named Drift algorithm.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_CONFIG_ALGORITHM_TARGET_CONFIG_H
#define FMD_CONFIG_ALGORITHM_TARGET_CONFIG_H

#include "fmd/config/AlgorithmBankConfig.h"

#define FMD_ALGORITHM_PERLIN 0
#define FMD_ALGORITHM_BROWNIAN 1
#define FMD_ALGORITHM_BEZIER 2
#define FMD_ALGORITHM_LFO 3
#define FMD_ALGORITHM_FRACTAL 4
#define FMD_ALGORITHM_VECTOR 5
#define FMD_ALGORITHM_RAIN 6
#define FMD_ALGORITHM_ATTRACTOR 7
#define FMD_ALGORITHM_TURING 8
#define FMD_ALGORITHM_MARKOV 9
#define FMD_ALGORITHM_MOTIF 10
#define FMD_ALGORITHM_URN 11
#define FMD_ALGORITHM_CURRENT 12
#define FMD_ALGORITHM_ANCHOR 13
#define FMD_ALGORITHM_BREATH 14
#define FMD_ALGORITHM_FOG 15
#define FMD_ALGORITHM_PUMP 16
#define FMD_ALGORITHM_ACID 17
#define FMD_ALGORITHM_SHUFFLE 18
#define FMD_ALGORITHM_POLYMETER 19
#define FMD_ALGORITHM_EUCLID 20
#define FMD_ALGORITHM_REPEAT 21
#define FMD_ALGORITHM_PROBABILITY 22
#define FMD_ALGORITHM_HUMANIZE 23

/** Sentinel used for normal bank firmware where the rear DIP switches select the algorithm. */
#define FMD_ALGORITHM_AUTO 255

#ifndef FMD_FORCED_ALGORITHM
/** Normal release/user builds use the rear DIP switches. Developer tooling may override this. */
#define FMD_FORCED_ALGORITHM FMD_ALGORITHM_AUTO
#endif

#if FMD_FORCED_ALGORITHM != FMD_ALGORITHM_AUTO
#if FMD_ALGORITHM_BANK == FMD_BANK_CLASSIC
#if FMD_FORCED_ALGORITHM < FMD_ALGORITHM_PERLIN || FMD_FORCED_ALGORITHM > FMD_ALGORITHM_LFO
#error "Forced algorithm does not belong to the Classic bank"
#endif
#elif FMD_ALGORITHM_BANK == FMD_BANK_ORGANIC
#if FMD_FORCED_ALGORITHM < FMD_ALGORITHM_FRACTAL || FMD_FORCED_ALGORITHM > FMD_ALGORITHM_ATTRACTOR
#error "Forced algorithm does not belong to the Organic bank"
#endif
#elif FMD_ALGORITHM_BANK == FMD_BANK_GENERATIVE
#if FMD_FORCED_ALGORITHM < FMD_ALGORITHM_TURING || FMD_FORCED_ALGORITHM > FMD_ALGORITHM_URN
#error "Forced algorithm does not belong to the Generative bank"
#endif
#elif FMD_ALGORITHM_BANK == FMD_BANK_AMBIENT
#if FMD_FORCED_ALGORITHM < FMD_ALGORITHM_CURRENT || FMD_FORCED_ALGORITHM > FMD_ALGORITHM_FOG
#error "Forced algorithm does not belong to the Ambient bank"
#endif
#elif FMD_ALGORITHM_BANK == FMD_BANK_ELECTRONICA
#if FMD_FORCED_ALGORITHM < FMD_ALGORITHM_PUMP || FMD_FORCED_ALGORITHM > FMD_ALGORITHM_POLYMETER
#error "Forced algorithm does not belong to the Electronica bank"
#endif
#elif FMD_ALGORITHM_BANK == FMD_BANK_PERCUSSION
#if FMD_FORCED_ALGORITHM < FMD_ALGORITHM_EUCLID || FMD_FORCED_ALGORITHM > FMD_ALGORITHM_HUMANIZE
#error "Forced algorithm does not belong to the Percussion bank"
#endif
#endif
#endif

#endif  // FMD_CONFIG_ALGORITHM_TARGET_CONFIG_H
