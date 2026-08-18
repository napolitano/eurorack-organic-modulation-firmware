/** @file main.cpp @brief Minimal Arduino entry points. SPDX-License-Identifier: GPL-3.0-or-later */
#include <Arduino.h>
#include "platform/nano_atmega328p/FirmwareController.h"
namespace { fmd::platform::nano::FirmwareController firmware; }
void setup(){ firmware.begin(); }
void loop(){ firmware.run(); }
