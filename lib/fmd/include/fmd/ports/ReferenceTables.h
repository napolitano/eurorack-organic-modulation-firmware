/**
 * @file ReferenceTables.h
 * Defines the platform-independent read-only numerical-table port.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_PORTS_REFERENCE_TABLES_H
#define FMD_PORTS_REFERENCE_TABLES_H

#include <stdint.h>

namespace fmd {

/**
 * @brief Access to generated lookup tables without exposing storage details.
 *
 * Host tests keep tables in normal memory while the AVR adapter stores them in
 * program memory. The domain code therefore depends only on numerical values,
 * not on PROGMEM or platform-specific access primitives.
 */
class IReferenceTables {
 public:
  /** @brief Virtual destructor for safe destruction through the port interface. */
  virtual ~IReferenceTables() {}

  /**
   * @brief Read one base-2 exponential lookup-table entry.
   * @param index Table index 0..255.
   * @return Q16.16 exponential value.
   */
  virtual uint32_t exp2Q16_16(uint8_t index) const = 0;

  /**
   * @brief Read one inverse-CDF entry for the symmetric triangular distribution.
   * @param index Table index 0..256.
   * @return Signed Q1.15 value.
   */
  virtual int16_t triangularIcdfQ1_15(uint16_t index) const = 0;


  /**
   * @brief Read one Anchor triangular-innovation compensation gain.
   * @param index Quantized effective Speed bucket 0..306.
   * @return Q1.15 multiplier that compensates the bounded triangular innovation
   *         for the selected mean-reversion rate.
   */
  virtual uint16_t anchorInnovationGainQ1_15(uint16_t index) const = 0;

  /**
   * @brief Read one gamma-corrected LED brightness entry.
   * @param index Linear brightness index 0..255.
   * @return Gamma-corrected PWM duty 0..255.
   */
  virtual uint8_t gamma8(uint8_t index) const = 0;
};

}  // namespace fmd
#endif  // FMD_PORTS_REFERENCE_TABLES_H
