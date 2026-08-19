/**
 * @file LedOutput.h
 * Defines the platform-independent status-LED output port.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_PORTS_LED_OUTPUT_H
#define FMD_PORTS_LED_OUTPUT_H

#include <stdint.h>

namespace fmd {

/** @brief PWM brightness sink for the front-panel output-level LED. */
class ILedOutput {
 public:
  /** @brief Virtual destructor for safe destruction through the port interface. */
  virtual ~ILedOutput() {}

  /**
   * @brief Set the LED PWM duty cycle.
   * @param duty 8-bit duty value 0..255.
   */
  virtual void setBrightness(uint8_t duty) = 0;
};

}  // namespace fmd
#endif  // FMD_PORTS_LED_OUTPUT_H
