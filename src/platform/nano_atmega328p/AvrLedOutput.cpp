/**
 * @file AvrLedOutput.cpp
 * Implements the Arduino PWM adapter for Drift's front-panel LED.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "platform/nano_atmega328p/AvrLedOutput.h"

#include "platform/nano_atmega328p/BoardConfig.h"

#include <Arduino.h>

namespace fmd::platform::nano {

void AvrLedOutput::begin() {
  pinMode(board::kLedPwmPin, OUTPUT);
  analogWrite(board::kLedPwmPin, 0U);
}

void AvrLedOutput::setBrightness(uint8_t duty) {
  analogWrite(board::kLedPwmPin, duty);
}

}  // namespace fmd::platform::nano
