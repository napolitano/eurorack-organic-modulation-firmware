/** @file DriftRuntime.h @brief Hardware-independent Drift control-cycle orchestration. SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef FMD_APPLICATION_DRIFT_RUNTIME_H
#define FMD_APPLICATION_DRIFT_RUNTIME_H
#include <stdint.h>
#include "fmd/domain/DriftEngine.h"
#include "fmd/ports/AnalogInputs.h"
#include "fmd/ports/DacOutput.h"
#include "fmd/ports/LedOutput.h"
#include "fmd/ports/ReferenceTables.h"
namespace fmd {
class DriftRuntime {
 public:
  DriftRuntime(Algorithm algorithm, uint16_t seed, IAnalogInputs& analog, IDacOutput& dac,
               ILedOutput& led, const IReferenceTables& tables);
  bool processIfReady();
  uint16_t lastOutput() const { return lastOutput_; }
 private:
  IAnalogInputs& analog_;
  IDacOutput& dac_;
  ILedOutput& led_;
  const IReferenceTables& tables_;
  DriftEngine engine_;
  uint16_t lastOutput_;
};
}  // namespace fmd
#endif
