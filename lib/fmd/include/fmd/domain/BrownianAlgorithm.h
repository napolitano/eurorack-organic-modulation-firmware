/** @file BrownianAlgorithm.h @brief Portable corrected Brownian Drift algorithm. SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef FMD_DOMAIN_BROWNIAN_ALGORITHM_H
#define FMD_DOMAIN_BROWNIAN_ALGORITHM_H

#include <stdint.h>
#include "fmd/domain/Types.h"
#include "fmd/domain/ParallelLfsr.h"

namespace fmd {
class BrownianAlgorithm {
 public:
  explicit BrownianAlgorithm(uint16_t seed);
  uint16_t step(const ControlFrame& controls);

 private:
  void stepTargetValue(uint16_t speed);
  void stepSmoothedValue(uint16_t texture);

  uint16_t targetValue_;
  uint16_t currentValue_;
  uint16_t smoothingResidual_;
  int8_t smoothingDirection_;
  ParallelLfsr rng_;
};
}  // namespace fmd
#endif
