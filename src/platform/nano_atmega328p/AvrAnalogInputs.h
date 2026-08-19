/**
 * @file AvrAnalogInputs.h
 * Declares the interrupt-driven ATmega328P ADC adapter for Drift controls.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_PLATFORM_NANO_AVR_ANALOG_INPUTS_H
#define FMD_PLATFORM_NANO_AVR_ANALOG_INPUTS_H

#include <stdint.h>

#include "fmd/ports/AnalogInputs.h"

namespace fmd::platform::nano {

/**
 * @brief Continuously samples all four Drift controls using the AVR ADC ISR.
 *
 * Foreground code never reads the volatile ISR state directly. beginCycle()
 * copies all four channels atomically into a stable snapshot used by the
 * portable runtime for the remainder of that processing cycle.
 */
class AvrAnalogInputs final : public IAnalogInputs {
 public:
  /** @brief Prime all channels and start the interrupt-driven conversion sequence. */
  void begin();

  /** @brief Atomically copy the latest background samples into the cycle snapshot. */
  void beginCycle() override;

  /**
   * @brief Read one channel from the latched snapshot.
   * @param channelIndex Portable control index 0..3.
   * @return Latched 10-bit ADC code, or 0 for an invalid channel index.
   */
  uint16_t read(uint8_t channelIndex) const override;

  /** @brief Store the completed ADC result and schedule the following MUX channel. */
  static void handleIsr();

 private:
  static volatile uint16_t latestSamples_[4]; ///< Most recent ISR-produced samples.
  static volatile uint8_t resultChannelIndex_; ///< Channel associated with current ADC result.
  uint16_t cycleSnapshot_[4] = {0U, 0U, 0U, 0U}; ///< Atomic foreground snapshot.
};

}  // namespace fmd::platform::nano
#endif  // FMD_PLATFORM_NANO_AVR_ANALOG_INPUTS_H
