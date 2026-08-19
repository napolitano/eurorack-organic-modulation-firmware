/**
 * @file AvrMcp4922Dac.h
 * Declares the timer-latched MCP4922 adapter used by the original Drift hardware.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_PLATFORM_NANO_AVR_MCP4922_DAC_H
#define FMD_PLATFORM_NANO_AVR_MCP4922_DAC_H

#include <stdint.h>

#include "fmd/ports/DacOutput.h"

namespace fmd::platform::nano {

/**
 * @brief Split-phase MCP4922 SPI writer with deterministic timer-edge latching.
 *
 * Foreground code pulls chip-select low and shifts a complete channel-A frame.
 * The 2.5 kHz timer ISR raises chip-select, latching that prepared sample at a
 * stable cadence. This mirrors the timing model of the original Drift firmware.
 */
class AvrMcp4922Dac final : public IDacOutput {
 public:
  /** @brief Configure chip-select and SPI for the MCP4922. */
  void begin();

  /** @return true when no prepared frame is waiting for the timer ISR. */
  bool ready() const override;

  /**
   * @brief Shift a channel-A 12-bit frame while leaving chip-select low.
   * @param value12 DAC code; only the lower 12 bits are transmitted.
   */
  void queue12Bit(uint16_t value12) override;

  /** @brief Raise chip-select from the sample-timer ISR and account for missed frames. */
  static void latchFromTimerIsr();

  /** @return Saturating count of timer ticks that found no queued DAC frame. */
  uint16_t missedLatchCount() const;

 private:
  static volatile bool frameQueued_;             ///< true after SPI preparation until timer latch.
  static volatile uint16_t missedLatchCount_;    ///< Saturating missed-deadline diagnostic counter.
};

}  // namespace fmd::platform::nano
#endif  // FMD_PLATFORM_NANO_AVR_MCP4922_DAC_H
