/**
 * @file SeedGenerator.h
 * Declares startup seed collection for Drift's pseudo-random algorithms.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_PLATFORM_NANO_SEED_GENERATOR_H
#define FMD_PLATFORM_NANO_SEED_GENERATOR_H

#include <stdint.h>

namespace fmd::platform::nano {

/**
 * @brief Collect low-order ADC bits into a non-cryptographic 16-bit startup seed.
 * @return Seed for the paired LFSR generators.
 *
 * The routine intentionally mirrors the original Drift approach of combining
 * several external and internal ADC sources. It is suitable for musical
 * variation but must not be treated as a cryptographic entropy source.
 */
uint16_t generateSeed();

}  // namespace fmd::platform::nano
#endif  // FMD_PLATFORM_NANO_SEED_GENERATOR_H
