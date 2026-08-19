/**
 * @file SampleTimer.cpp
 * Implements the fixed-rate Timer1 scheduler used to latch Drift DAC samples.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "platform/nano_atmega328p/SampleTimer.h"

#include "platform/nano_atmega328p/AvrMcp4922Dac.h"
#include "platform/nano_atmega328p/BoardConfig.h"

#include <Arduino.h>
#include <avr/interrupt.h>

namespace fmd::platform::nano {
namespace {

/// Timer1 clock prescaler selected by CS11|CS10 on ATmega328P.
constexpr uint32_t kTimer1Prescaler = 64UL;
/// Compare value giving F_CPU / (64 * (OCR1A + 1)) = 2500 Hz at 16 MHz.
constexpr uint16_t kTimer1CompareValue = static_cast<uint16_t>(
    (F_CPU / (kTimer1Prescaler * board::kSampleRateHz)) - 1UL);

static_assert(kTimer1CompareValue == 99U,
              "Drift sample timer assumes a 16 MHz ATmega328P clock");

}  // namespace

void SampleTimer::begin() {
  const uint8_t previousStatusRegister = SREG;
  cli();

  TCCR1A = 0U;
  TCCR1B = static_cast<uint8_t>(_BV(WGM12) | _BV(CS11) | _BV(CS10));
  OCR1A = kTimer1CompareValue;
  TCNT1 = 0U;
  TIMSK1 |= _BV(OCIE1A);

  SREG = previousStatusRegister;
}

void SampleTimer::handleIsr() {
#ifdef FMD_TIMING_PROBE
  // D9 is PB1. Writing one to PINB toggles the pin in a single register access,
  // allowing oscilloscope timing without inflating the ISR with digitalWrite().
  PINB = _BV(PINB1);
#endif
  AvrMcp4922Dac::latchFromTimerIsr();
}

}  // namespace fmd::platform::nano

/** @brief AVR Timer1 compare-A interrupt bridge. */
ISR(TIMER1_COMPA_vect) {
  fmd::platform::nano::SampleTimer::handleIsr();
}
