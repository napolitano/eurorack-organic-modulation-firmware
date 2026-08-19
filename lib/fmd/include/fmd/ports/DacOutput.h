/**
 * @file DacOutput.h
 * Defines the platform-independent queued DAC-output port.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_PORTS_DAC_OUTPUT_H
#define FMD_PORTS_DAC_OUTPUT_H

#include <stdint.h>

namespace fmd {

/**
 * @brief Asynchronous 12-bit DAC output used by the portable runtime.
 *
 * The AVR implementation prepares an SPI frame in foreground code and latches
 * it on the next fixed-rate timer interrupt. This port exposes only the timing
 * semantics required by the portable application layer.
 */
class IDacOutput {
 public:
  /** @brief Virtual destructor for safe destruction through the port interface. */
  virtual ~IDacOutput() {}

  /** @return true when a new output code may be queued. */
  virtual bool ready() const = 0;

  /**
   * @brief Queue a 12-bit DAC code for the next hardware latch event.
   * @param value12 Output code in the range 0..4095; implementations defensively
   *                ignore bits above bit 11.
   */
  virtual void queue12Bit(uint16_t value12) = 0;
};

}  // namespace fmd
#endif  // FMD_PORTS_DAC_OUTPUT_H
