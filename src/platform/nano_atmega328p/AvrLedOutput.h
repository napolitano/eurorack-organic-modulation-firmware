/**
 * @file AvrLedOutput.h
 * Declares the Arduino PWM adapter for Drift's front-panel LED.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_PLATFORM_NANO_AVR_LED_OUTPUT_H
#define FMD_PLATFORM_NANO_AVR_LED_OUTPUT_H

#include "fmd/ports/LedOutput.h"

namespace fmd::platform::nano {

/** @brief Hardware PWM implementation of the portable LED output port. */
class AvrLedOutput final : public ILedOutput {
 public:
  /** @brief Configure the front-panel LED pin for PWM output and start dark. */
  void begin();

  /**
   * @brief Write an 8-bit PWM duty cycle to the front-panel LED.
   * @param duty PWM duty cycle in the range 0..255.
   */
  void setBrightness(uint8_t duty) override;
};

}  // namespace fmd::platform::nano
#endif  // FMD_PLATFORM_NANO_AVR_LED_OUTPUT_H
