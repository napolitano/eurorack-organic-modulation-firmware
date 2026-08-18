/** @file DriftEngine.h @brief Algorithm selection and portable Drift processing core. SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef FMD_DOMAIN_DRIFT_ENGINE_H
#define FMD_DOMAIN_DRIFT_ENGINE_H
#include <stdint.h>
#include "fmd/domain/Types.h"
#include "fmd/domain/PerlinAlgorithm.h"
#include "fmd/domain/BrownianAlgorithm.h"
#include "fmd/domain/BezierAlgorithm.h"
#include "fmd/domain/LfoAlgorithm.h"
namespace fmd {
class DriftEngine {
 public:
  DriftEngine(Algorithm algorithm, uint16_t seed, const IReferenceTables& tables);
  uint16_t step(const ControlFrame& controls);
  Algorithm algorithm() const { return algorithm_; }
 private:
  Algorithm algorithm_;
  PerlinAlgorithm perlin_;
  BrownianAlgorithm brownian_;
  BezierAlgorithm bezier_;
  LfoAlgorithm lfo_;
};
Algorithm algorithmFromConfig(bool config1Low, bool config2Low);
}  // namespace fmd
#endif
