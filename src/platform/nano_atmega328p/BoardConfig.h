/** @file BoardConfig.h @brief Original Free Modular Drift board mapping. SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef FMD_PLATFORM_NANO_BOARD_CONFIG_H
#define FMD_PLATFORM_NANO_BOARD_CONFIG_H
#include <Arduino.h>
#include <stdint.h>
namespace fmd::platform::nano::board {
constexpr uint8_t kSpeedCvChannel=4;      // A4
constexpr uint8_t kTextureCvChannel=5;    // A5
constexpr uint8_t kSpeedKnobChannel=6;    // ADC6 / A6
constexpr uint8_t kTextureKnobChannel=7;  // ADC7 / A7
constexpr uint8_t kConfig1Pin=5;
constexpr uint8_t kConfig2Pin=4;
constexpr uint8_t kDacCsPin=10;
constexpr uint8_t kLedPin=3;
constexpr uint32_t kSampleRateHz=2500UL;
}
#endif
