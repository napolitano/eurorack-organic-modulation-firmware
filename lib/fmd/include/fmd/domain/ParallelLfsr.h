/**
 * @file ParallelLfsr.h
 * Declares the upstream-compatible paired 16-bit LFSR pseudo-random generator.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_DOMAIN_PARALLEL_LFSR_H
#define FMD_DOMAIN_PARALLEL_LFSR_H

#include <stdint.h>

namespace fmd {

/**
 * @brief Two parallel 16-bit LFSRs combined by XOR.
 *
 * The tap sets and seed transformation intentionally preserve the pseudo-random
 * sequence used by the original Drift firmware. This generator is intended for
 * modulation, not for cryptographic use.
 */
class ParallelLfsr {
 public:
  /**
   * @brief Initialise both LFSR states from one 16-bit seed.
   * @param seed Seed value transformed into the two upstream-compatible register states.
   */
  explicit ParallelLfsr(uint16_t seed);

  /** @return Next 16-bit deterministic pseudo-random value. */
  uint16_t next();

 private:
  /**
   * @brief Advance one 16-bit LFSR by one step.
   * @param state Current register state.
   * @param tapPositions Four one-based polynomial tap positions.
   * @return Advanced register state.
   */
  static uint16_t advanceRegister(uint16_t state, const uint8_t tapPositions[4]);

  uint16_t primaryState_;    ///< First LFSR state.
  uint16_t secondaryState_;  ///< Second LFSR state.
};

}  // namespace fmd
#endif  // FMD_DOMAIN_PARALLEL_LFSR_H
