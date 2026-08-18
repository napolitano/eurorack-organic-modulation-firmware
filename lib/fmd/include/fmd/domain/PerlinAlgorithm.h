/** @file PerlinAlgorithm.h @brief Portable Perlin Drift algorithm. SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef FMD_DOMAIN_PERLIN_ALGORITHM_H
#define FMD_DOMAIN_PERLIN_ALGORITHM_H
#include <stdint.h>
#include "fmd/domain/Types.h"
#include "fmd/domain/ParallelLfsr.h"
#include "fmd/ports/ReferenceTables.h"
namespace fmd {
class PerlinAlgorithm {
 public:
  PerlinAlgorithm(const IReferenceTables& tables, uint16_t seed);
  uint16_t step(const ControlFrame& controls);
 private:
  struct Octave { uint32_t time; int16_t lastGrad; int16_t nextGrad; };
  static int16_t randomGrad(ParallelLfsr& rng);
  int16_t stepOctave(Octave& octave, uint32_t deltaTime);
  const IReferenceTables& tables_;
  Octave base_;
  Octave octave_;
  ParallelLfsr rng_;
};
}  // namespace fmd
#endif
