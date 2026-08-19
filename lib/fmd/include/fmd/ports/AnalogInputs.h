/**
 * @file AnalogInputs.h
 * Defines the platform-independent four-channel analog-input port.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_PORTS_ANALOG_INPUTS_H
#define FMD_PORTS_ANALOG_INPUTS_H

#include <stdint.h>

namespace fmd {

/**
 * @brief Access to the four controls sampled by the ADC.
 *
 * Channel indices are part of the portable runtime contract:
 * 0 = Speed CV, 1 = Texture CV, 2 = Speed knob, 3 = Texture knob.
 * Implementations must provide a coherent snapshot for each processing cycle.
 */
class IAnalogInputs {
 public:
  /** @brief Virtual destructor for safe destruction through the port interface. */
  virtual ~IAnalogInputs() {}

  /** @brief Latch the latest four ADC samples into the current-cycle snapshot. */
  virtual void beginCycle() = 0;

  /**
   * @brief Read a value from the latched current-cycle snapshot.
   * @param channelIndex Channel index 0..3 as documented above.
   * @return 10-bit ADC value 0..1023; invalid channels return 0 on AVR.
   */
  virtual uint16_t read(uint8_t channelIndex) const = 0;
};

}  // namespace fmd
#endif  // FMD_PORTS_ANALOG_INPUTS_H
