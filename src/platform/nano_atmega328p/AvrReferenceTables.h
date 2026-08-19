/**
 * @file AvrReferenceTables.h
 * Declares the PROGMEM-backed reference-table adapter for ATmega328P.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_PLATFORM_NANO_AVR_REFERENCE_TABLES_H
#define FMD_PLATFORM_NANO_AVR_REFERENCE_TABLES_H

#include "fmd/ports/ReferenceTables.h"

namespace fmd::platform::nano {

/** @brief Reads generated numerical tables directly from AVR program memory. */
class AvrReferenceTables final : public IReferenceTables {
 public:
  /**
   * @brief Read one Q16.16 base-2 exponential entry from PROGMEM.
   * @param index Table index 0..255.
   * @return Q16.16 exponential value.
   */
  uint32_t exp2Q16_16(uint8_t index) const override;

  /**
   * @brief Read one signed Q1.15 triangular inverse-CDF entry from PROGMEM.
   * @param index Table index 0..256.
   * @return Signed Q1.15 inverse-CDF value.
   */
  int16_t triangularIcdfQ1_15(uint16_t index) const override;


  /** @brief Read one Anchor innovation compensation gain from PROGMEM. */
  uint16_t anchorInnovationGainQ1_15(uint16_t index) const override;

  /**
   * @brief Read one 8-bit gamma-correction entry from PROGMEM.
   * @param index Linear brightness index 0..255.
   * @return Gamma-corrected PWM duty cycle 0..255.
   */
  uint8_t gamma8(uint8_t index) const override;
};

}  // namespace fmd::platform::nano
#endif  // FMD_PLATFORM_NANO_AVR_REFERENCE_TABLES_H
