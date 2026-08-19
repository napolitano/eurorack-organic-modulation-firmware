/**
 * @file main.cpp
 * Provides the minimal Arduino entry points for the Drift firmware.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "platform/nano_atmega328p/FirmwareController.h"

#include <Arduino.h>

namespace {

/// Single firmware composition root used by Arduino's setup()/loop() entry points.
fmd::platform::nano::FirmwareController firmwareController;

}  // namespace

/** @brief Arduino startup entry point. */
void setup() {
  firmwareController.begin();
}

/** @brief Arduino foreground-loop entry point. */
void loop() {
  firmwareController.run();
}
