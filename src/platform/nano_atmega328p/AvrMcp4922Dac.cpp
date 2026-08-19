/**
 * @file AvrMcp4922Dac.cpp
 * Implements the timer-latched MCP4922 adapter used by the original Drift hardware.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "platform/nano_atmega328p/AvrMcp4922Dac.h"

#include "platform/nano_atmega328p/BoardConfig.h"

#include <Arduino.h>
#include <SPI.h>
#include <avr/interrupt.h>

namespace fmd::platform::nano {
namespace {

/// MCP4922 channel-A command bits: unbuffered, 1x gain, output enabled.
constexpr uint16_t kChannelAControlBits = 0x3000U;
/// Mask selecting the MCP4922's 12 data bits.
constexpr uint16_t kDacDataMask = 0x0FFFU;
/// SPI bus rate used by the original Nano-class target.
constexpr uint32_t kSpiClockHz = 8000000UL;

}  // namespace

volatile bool AvrMcp4922Dac::frameQueued_ = false;
volatile uint16_t AvrMcp4922Dac::missedLatchCount_ = 0U;

void AvrMcp4922Dac::begin() {
  pinMode(board::kDacChipSelectPin, OUTPUT);
  digitalWrite(board::kDacChipSelectPin, HIGH);

  SPI.begin();
  SPI.beginTransaction(SPISettings(kSpiClockHz, MSBFIRST, SPI_MODE0));
  frameQueued_ = false;
}

bool AvrMcp4922Dac::ready() const {
  const uint8_t previousStatusRegister = SREG;
  cli();
  const bool isReady = !frameQueued_;
  SREG = previousStatusRegister;
  return isReady;
}

void AvrMcp4922Dac::queue12Bit(uint16_t value12) {
  const uint16_t clampedCode = static_cast<uint16_t>(value12 & kDacDataMask);
  const uint16_t commandWord =
      static_cast<uint16_t>(kChannelAControlBits | clampedCode);

  // Keep CS low after shifting the frame. The sample timer raises it at the
  // deterministic 2.5 kHz boundary, which latches the prepared channel-A code.
  digitalWrite(board::kDacChipSelectPin, LOW);
  SPI.transfer(static_cast<uint8_t>(commandWord >> 8U));
  SPI.transfer(static_cast<uint8_t>(commandWord & 0x00FFU));

  const uint8_t previousStatusRegister = SREG;
  cli();
  frameQueued_ = true;
  SREG = previousStatusRegister;
}

void AvrMcp4922Dac::latchFromTimerIsr() {
  if (frameQueued_) {
    // D10 is PB2 on the ATmega328P. Direct port access is intentional here: the
    // ISR must do only the minimum work needed to generate the latch edge.
    PORTB |= _BV(PORTB2);
    frameQueued_ = false;
  } else if (missedLatchCount_ != 0xFFFFU) {
    ++missedLatchCount_;
  }
}

uint16_t AvrMcp4922Dac::missedLatchCount() const {
  const uint8_t previousStatusRegister = SREG;
  cli();
  const uint16_t capturedCount = missedLatchCount_;
  SREG = previousStatusRegister;
  return capturedCount;
}

}  // namespace fmd::platform::nano
