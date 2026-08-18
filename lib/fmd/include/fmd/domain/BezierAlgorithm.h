/** @file BezierAlgorithm.h @brief Portable corrected Bezier Drift algorithm. SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef FMD_DOMAIN_BEZIER_ALGORITHM_H
#define FMD_DOMAIN_BEZIER_ALGORITHM_H
#include <stdint.h>
#include "fmd/domain/Types.h"
#include "fmd/domain/ParallelLfsr.h"
#include "fmd/ports/ReferenceTables.h"
namespace fmd {
class BezierAlgorithm {
 public:
  BezierAlgorithm(const IReferenceTables& tables, uint16_t seed);
  uint16_t step(const ControlFrame& controls);
 private:
  uint32_t stepTime(uint16_t knob, uint16_t cv, bool& rollover);
  int16_t getSpeedAdjust(uint16_t knob, uint16_t cv);
  int16_t randomFromDistribution();
  const IReferenceTables& tables_;
  uint32_t time_;
  int16_t speedAdjust_;
  uint16_t valueA_;
  uint16_t valueB_;
  bool speedInitialized_;
  ParallelLfsr rng_;
};
}  // namespace fmd
#endif
