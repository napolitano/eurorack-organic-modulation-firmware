/**
 * @file DriftRuntime.h
 * Declares the hardware-independent Drift control-cycle orchestrator.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_APPLICATION_DRIFT_RUNTIME_H
#define FMD_APPLICATION_DRIFT_RUNTIME_H

#include <stdint.h>

#include "fmd/domain/DriftEngine.h"
#include "fmd/ports/AnalogInputs.h"
#include "fmd/ports/DacOutput.h"
#include "fmd/ports/LedOutput.h"
#include "fmd/ports/ReferenceTables.h"

namespace fmd {

/**
 * @brief Coordinates one complete Drift processing cycle.
 *
 * The runtime is the portable application boundary between domain processing
 * and hardware ports. A cycle is executed only when the DAC can accept a new
 * sample. The runtime then snapshots all controls, advances the selected
 * algorithm, derives LED brightness from the same output sample and queues the
 * 12-bit DAC code.
 */
class DriftRuntime {
 public:
  /**
   * @brief Construct the portable Drift runtime.
   * @param algorithm Algorithm selected at startup.
   * @param randomSeed Deterministic seed passed to the stochastic algorithms.
   * @param analogInputs Latched four-channel ADC input port.
   * @param dacOutput Queued 12-bit DAC output port.
   * @param ledOutput PWM LED output port.
   * @param referenceTables Read-only numerical lookup tables.
   */
  DriftRuntime(Algorithm algorithm,
               uint16_t randomSeed,
               IAnalogInputs& analogInputs,
               IDacOutput& dacOutput,
               ILedOutput& ledOutput,
               const IReferenceTables& referenceTables);

  /**
   * @brief Process one sample if the DAC is ready to accept it.
   * @return true when a control frame was processed and queued; false while
   *         the previous DAC frame is still pending.
   */
  bool processNextSampleIfReady();

  /** @return Most recently calculated 12-bit DAC code. */
  uint16_t lastOutputCode() const { return lastOutputCode_; }

 private:
  IAnalogInputs& analogInputs_;                 ///< Source of coherent control snapshots.
  IDacOutput& dacOutput_;                       ///< Destination for queued 12-bit samples.
  ILedOutput& ledOutput_;                       ///< Output-level activity indicator.
  const IReferenceTables& referenceTables_;    ///< Shared read-only numerical tables.
  DriftEngine driftEngine_;                    ///< Selected portable modulation algorithm.
  uint16_t lastOutputCode_;                    ///< Last calculated DAC code, range 0..4095.
};

}  // namespace fmd
#endif  // FMD_APPLICATION_DRIFT_RUNTIME_H
