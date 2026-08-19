/**
 * @file DriftTestRig.h
 * Provides reusable host-test doubles and an end-to-end Drift runtime fixture.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_TEST_DRIFT_TEST_RIG_H
#define FMD_TEST_DRIFT_TEST_RIG_H

#include <stdint.h>

#include "fmd/application/DriftRuntime.h"
#include "MemoryReferenceTables.h"

/** @brief In-memory four-channel analog-input test double. */
class FakeAnalogInputs final : public fmd::IAnalogInputs {
 public:
  uint16_t channelValues[4] = {0U, 0U, 0U, 0U};

  void beginCycle() override {}

  uint16_t read(uint8_t channelIndex) const override {
    return channelIndex < 4U ? channelValues[channelIndex] : 0U;
  }
};

/** @brief Queued DAC test double that models the runtime's ready/latch handshake. */
class FakeDacOutput final : public fmd::IDacOutput {
 public:
  bool isReady = true;
  uint16_t queuedCode = 0U;

  bool ready() const override { return isReady; }

  void queue12Bit(uint16_t value12) override {
    queuedCode = value12;
    isReady = false;
  }

  void latch() { isReady = true; }
};

/** @brief Captures the most recent LED PWM duty written by the runtime. */
class FakeLedOutput final : public fmd::ILedOutput {
 public:
  uint8_t duty = 0U;

  void setBrightness(uint8_t newDuty) override { duty = newDuty; }
};

/**
 * @brief Wires the real portable DriftRuntime to deterministic host test doubles.
 */
class DriftTestRig {
 public:
  MemoryReferenceTables referenceTables;
  FakeAnalogInputs analogInputs;
  FakeDacOutput dacOutput;
  FakeLedOutput ledOutput;
  fmd::DriftRuntime runtime;

  DriftTestRig(fmd::Algorithm algorithm, uint16_t randomSeed)
      : runtime(algorithm,
                randomSeed,
                analogInputs,
                dacOutput,
                ledOutput,
                referenceTables) {}

  /** @brief Feed one control frame through the real runtime and latch its output. */
  uint16_t tick(uint16_t speedCv,
                uint16_t textureCv,
                uint16_t speedKnob,
                uint16_t textureKnob) {
    analogInputs.channelValues[0] = speedCv;
    analogInputs.channelValues[1] = textureCv;
    analogInputs.channelValues[2] = speedKnob;
    analogInputs.channelValues[3] = textureKnob;
    dacOutput.latch();
    runtime.processNextSampleIfReady();
    return dacOutput.queuedCode;
  }
};

#endif  // FMD_TEST_DRIFT_TEST_RIG_H
