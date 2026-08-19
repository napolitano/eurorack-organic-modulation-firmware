/**
 * @file FirmwareController.cpp
 * Implements the Arduino Nano composition root for the Drift firmware.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "platform/nano_atmega328p/FirmwareController.h"

#include "fmd/application/DriftRuntime.h"
#include "platform/nano_atmega328p/AvrAnalogInputs.h"
#include "platform/nano_atmega328p/AvrLedOutput.h"
#include "platform/nano_atmega328p/AvrMcp4922Dac.h"
#include "platform/nano_atmega328p/AvrReferenceTables.h"
#include "platform/nano_atmega328p/BoardConfig.h"
#include "platform/nano_atmega328p/SampleTimer.h"
#include "platform/nano_atmega328p/SeedGenerator.h"

#include <Arduino.h>
#include <new>

namespace fmd::platform::nano {
namespace {

/// Concrete ADC adapter owned for the complete firmware lifetime.
AvrAnalogInputs analogInputs;
/// Concrete timer-latched MCP4922 adapter owned for the complete firmware lifetime.
AvrMcp4922Dac dacOutput;
/// Concrete front-panel LED adapter owned for the complete firmware lifetime.
AvrLedOutput ledOutput;
/// PROGMEM-backed numerical-table adapter shared by the portable runtime.
AvrReferenceTables referenceTables;

// Construct DriftRuntime in static storage instead of using the heap. The
// concrete algorithm cannot be chosen until the rear switches are sampled at
// runtime, while AVR firmware should avoid long-lived dynamic allocation.
/// Placement-new backing storage for the runtime selected after reading rear switches.
alignas(DriftRuntime) uint8_t runtimeStorage[sizeof(DriftRuntime)];
/// Non-owning pointer to the DriftRuntime instance constructed in runtimeStorage.
DriftRuntime* driftRuntime = nullptr;

}  // namespace

void FirmwareController::begin() {
#ifdef FMD_TIMING_PROBE
  pinMode(board::kTimingProbePin, OUTPUT);
#endif

  pinMode(board::kConfigInput1Pin, INPUT_PULLUP);
  pinMode(board::kConfigInput2Pin, INPUT_PULLUP);

  const bool configInput1Low =
      digitalRead(board::kConfigInput1Pin) == LOW;
  const bool configInput2Low =
      digitalRead(board::kConfigInput2Pin) == LOW;
  const uint16_t randomSeed = generateSeed();

  analogInputs.begin();
  dacOutput.begin();
  ledOutput.begin();

  driftRuntime = new (runtimeStorage) DriftRuntime(
      algorithmFromConfig(configInput1Low, configInput2Low),
      randomSeed,
      analogInputs,
      dacOutput,
      ledOutput,
      referenceTables);

  // Start the deterministic latch timer only after every dependency and the
  // runtime are ready, preventing an intentional startup tick from being
  // recorded as a missed foreground sample.
  SampleTimer::begin();
}

void FirmwareController::run() {
  if (driftRuntime != nullptr) {
    driftRuntime->processNextSampleIfReady();
  }
}

}  // namespace fmd::platform::nano
