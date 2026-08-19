/**
 * @file DriftRuntime.cpp
 * Implements hardware-independent Drift control-cycle orchestration.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "fmd/application/DriftRuntime.h"

namespace fmd {

DriftRuntime::DriftRuntime(Algorithm algorithm,
                           uint16_t randomSeed,
                           IAnalogInputs& analogInputs,
                           IDacOutput& dacOutput,
                           ILedOutput& ledOutput,
                           const IReferenceTables& referenceTables)
    : analogInputs_(analogInputs),
      dacOutput_(dacOutput),
      ledOutput_(ledOutput),
      referenceTables_(referenceTables),
      driftEngine_(algorithm, randomSeed, referenceTables),
      lastOutputCode_(0U) {}

bool DriftRuntime::processNextSampleIfReady() {
  if (!dacOutput_.ready()) {
    return false;
  }

  analogInputs_.beginCycle();
  const ControlFrame controls{
      analogInputs_.read(0U),
      analogInputs_.read(1U),
      analogInputs_.read(2U),
      analogInputs_.read(3U),
  };

  lastOutputCode_ = driftEngine_.step(controls);

  // LED and DAC are deliberately derived from the same processed sample. The
  // upper eight DAC bits address the gamma table, preserving the original
  // output-level indication while keeping LED work outside the timer ISR.
  const uint8_t linearLedLevel = static_cast<uint8_t>(lastOutputCode_ >> 4U);
  ledOutput_.setBrightness(referenceTables_.gamma8(linearLedLevel));
  dacOutput_.queue12Bit(lastOutputCode_);
  return true;
}

}  // namespace fmd
