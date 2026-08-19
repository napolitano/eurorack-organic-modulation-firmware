/**
 * @file BoardConfig.h
 * Defines the verified pin and timing mapping for the original Free Modular Drift PCB.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_PLATFORM_NANO_BOARD_CONFIG_H
#define FMD_PLATFORM_NANO_BOARD_CONFIG_H

#include <Arduino.h>
#include <stdint.h>

namespace fmd::platform::nano::board {

/// ADC multiplexer channel for the Speed CV input (Arduino A4).
constexpr uint8_t kSpeedCvAdcChannel = 4U;
/// ADC multiplexer channel for the Texture CV input (Arduino A5).
constexpr uint8_t kTextureCvAdcChannel = 5U;
/// ADC-only channel for the Speed potentiometer (ADC6 / Arduino A6).
constexpr uint8_t kSpeedKnobAdcChannel = 6U;
/// ADC-only channel for the Texture potentiometer (ADC7 / Arduino A7).
constexpr uint8_t kTextureKnobAdcChannel = 7U;

/// Active-low firmware CONFIG 1 input on Arduino D5.
constexpr uint8_t kConfigInput1Pin = 5U;
/// Active-low firmware CONFIG 2 input on Arduino D4.
constexpr uint8_t kConfigInput2Pin = 4U;
/// MCP4922 chip-select input on Arduino D10 / AVR PB2.
constexpr uint8_t kDacChipSelectPin = 10U;
/// Front-panel LED PWM output on Arduino D3.
constexpr uint8_t kLedPwmPin = 3U;
/// Optional timing-probe output on Arduino D9 / AVR PB1.
constexpr uint8_t kTimingProbePin = 9U;

/// Fixed DAC latch / processing sample rate in hertz.
constexpr uint32_t kSampleRateHz = 2500UL;

}  // namespace fmd::platform::nano::board
#endif  // FMD_PLATFORM_NANO_BOARD_CONFIG_H
