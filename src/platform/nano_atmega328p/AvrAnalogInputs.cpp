/**
 * @file AvrAnalogInputs.cpp
 * Implements the interrupt-driven ATmega328P ADC adapter for Drift controls.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "platform/nano_atmega328p/AvrAnalogInputs.h"

#include "platform/nano_atmega328p/BoardConfig.h"

#include <Arduino.h>
#include <avr/interrupt.h>

namespace fmd::platform::nano {

volatile uint16_t AvrAnalogInputs::latestSamples_[4] = {0U, 0U, 0U, 0U};
volatile uint8_t AvrAnalogInputs::resultChannelIndex_ = 0U;

namespace {

/// ADC channel order must match the portable IAnalogInputs channel contract.
constexpr uint8_t kAdcChannels[4] = {
    board::kSpeedCvAdcChannel,
    board::kTextureCvAdcChannel,
    board::kSpeedKnobAdcChannel,
    board::kTextureKnobAdcChannel,
};
/// Number of continuously sampled Drift controls.
constexpr uint8_t kAdcChannelCount = 4U;

}  // namespace

void AvrAnalogInputs::begin() {
  // Prime the snapshot synchronously so the first foreground processing cycle
  // never observes an all-zero frame merely because the ADC ISR has not yet
  // completed a full four-channel rotation.
  latestSamples_[0] = analogRead(A4);
  latestSamples_[1] = analogRead(A5);
  latestSamples_[2] = analogRead(A6);
  latestSamples_[3] = analogRead(A7);

  resultChannelIndex_ = 0U;
  ADMUX = static_cast<uint8_t>(_BV(REFS0) | kAdcChannels[0]);
  ADCSRB = 0U;
  ADCSRA = static_cast<uint8_t>(
      _BV(ADEN) | _BV(ADSC) | _BV(ADATE) | _BV(ADIE) |
      _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0));

  // In free-running mode the next conversion begins before the ISR executes.
  // Pre-select channel 1 immediately; each ISR then programs one channel ahead
  // so the result index and ADC multiplexer pipeline stay aligned.
  ADMUX = static_cast<uint8_t>(_BV(REFS0) | kAdcChannels[1]);
}

void AvrAnalogInputs::handleIsr() {
  latestSamples_[resultChannelIndex_] = ADC;
  resultChannelIndex_ = static_cast<uint8_t>(
      (resultChannelIndex_ + 1U) % kAdcChannelCount);

  const uint8_t lookAheadChannelIndex = static_cast<uint8_t>(
      (resultChannelIndex_ + 1U) % kAdcChannelCount);
  ADMUX = static_cast<uint8_t>(
      _BV(REFS0) | kAdcChannels[lookAheadChannelIndex]);
}

void AvrAnalogInputs::beginCycle() {
  const uint8_t previousStatusRegister = SREG;
  cli();
  for (uint8_t channelIndex = 0U;
       channelIndex < kAdcChannelCount;
       ++channelIndex) {
    cycleSnapshot_[channelIndex] = latestSamples_[channelIndex];
  }
  SREG = previousStatusRegister;
}

uint16_t AvrAnalogInputs::read(uint8_t channelIndex) const {
  return channelIndex < kAdcChannelCount
      ? cycleSnapshot_[channelIndex]
      : 0U;
}

}  // namespace fmd::platform::nano

/** @brief AVR ADC conversion-complete interrupt bridge. */
ISR(ADC_vect) {
  fmd::platform::nano::AvrAnalogInputs::handleIsr();
}
