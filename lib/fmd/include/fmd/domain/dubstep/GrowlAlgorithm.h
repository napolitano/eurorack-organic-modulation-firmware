/**
 * @file GrowlAlgorithm.h
 * Declares the beat-synchronised multi-lobed timbral-motion CV algorithm.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_DOMAIN_DUBSTEP_GROWL_ALGORITHM_H
#define FMD_DOMAIN_DUBSTEP_GROWL_ALGORITHM_H

#include <stdint.h>

#include "fmd/domain/ClockSource.h"
#include "fmd/domain/Types.h"
#include "fmd/domain/dubstep/DubstepAlgorithmMath.h"
#include "fmd/ports/ReferenceTables.h"

namespace fmd {

/** @brief Deterministic compound triangle gesture intended for timbral modulation destinations. */
class GrowlAlgorithm {
 public:
  explicit GrowlAlgorithm(const IReferenceTables& referenceTables);
  uint16_t step(const ControlFrame& controls);

 private:
  void updateTexture(uint16_t textureControl);

  const IReferenceTables& referenceTables_;
  clock::ClockSource clockSource_;
  uint32_t phase_;
  uint16_t cachedTexture_;
  growlmath::Weights weights_;
  uint8_t externalQuarterIndex_;
  bool initialized_;
};

}  // namespace fmd

#endif  // FMD_DOMAIN_DUBSTEP_GROWL_ALGORITHM_H
