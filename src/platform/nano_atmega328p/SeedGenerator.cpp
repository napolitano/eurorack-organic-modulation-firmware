/**
 * @file SeedGenerator.cpp
 * Implements startup seed collection for Drift's pseudo-random algorithms.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "platform/nano_atmega328p/SeedGenerator.h"

#include <Arduino.h>

namespace fmd::platform::nano {
namespace {

/**
 * @brief Perform one blocking conversion on an internal AVR ADC multiplexer code.
 * @param muxChannel Low four MUX bits to select temporarily.
 * @return Raw 10-bit ADC conversion result.
 *
 * ADMUX and ADCSRA are restored so startup seed collection does not leak ADC
 * configuration into the interrupt-driven control sampler initialised later.
 */
uint16_t readInternalAdcChannel(uint8_t muxChannel) {
  const uint8_t previousAdmux = ADMUX;
  const uint8_t previousAdcsra = ADCSRA;

  ADMUX = static_cast<uint8_t>(_BV(REFS0) | (muxChannel & 0x0FU));
  ADCSRA = static_cast<uint8_t>(
      _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0) | _BV(ADSC));
  while ((ADCSRA & _BV(ADSC)) != 0U) {
    // Intentional short blocking wait during startup only.
  }

  const uint16_t adcValue = ADC;
  ADMUX = previousAdmux;
  ADCSRA = previousAdcsra;
  return adcValue;
}

}  // namespace

uint16_t generateSeed() {
  const uint16_t analog0Sample = analogRead(A0);
  const uint16_t analog1Sample = analogRead(A1);
  const uint16_t analog2Sample = analogRead(A2);
  const uint16_t analog3Sample = analogRead(A3);
  const uint16_t analog6Sample = analogRead(A6);
  const uint16_t analog7Sample = analogRead(A7);
  const uint16_t internalMux8Sample = readInternalAdcChannel(8U);
  const uint16_t internalMux14Sample = readInternalAdcChannel(14U);

  // Use only low-order ADC bits, where startup noise and analogue uncertainty
  // contribute most. Bit positions intentionally match the upstream seed layout.
  return static_cast<uint16_t>(
      ((analog0Sample & 7U) << 13U) |
      ((analog1Sample & 7U) << 10U) |
      ((analog2Sample & 7U) << 7U) |
      ((analog3Sample & 7U) << 4U) |
      ((analog6Sample & 1U) << 3U) |
      ((analog7Sample & 1U) << 2U) |
      ((internalMux8Sample & 1U) << 1U) |
      (internalMux14Sample & 1U));
}

}  // namespace fmd::platform::nano
