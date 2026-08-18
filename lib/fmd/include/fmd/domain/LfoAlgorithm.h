/** @file LfoAlgorithm.h @brief Portable corrected skew-LFO Drift algorithm. SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef FMD_DOMAIN_LFO_ALGORITHM_H
#define FMD_DOMAIN_LFO_ALGORITHM_H
#include <stdint.h>
#include "fmd/domain/Types.h"
#include "fmd/ports/ReferenceTables.h"
namespace fmd {
class LfoAlgorithm {
 public:
  explicit LfoAlgorithm(const IReferenceTables& tables);
  uint16_t step(const ControlFrame& controls);
 private:
  const IReferenceTables& tables_;
  uint32_t phase_;
  uint16_t apex_;
  bool initialized_;
};
}  // namespace fmd
#endif
