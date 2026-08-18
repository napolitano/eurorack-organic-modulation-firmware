/** @file FrequencyMapping.h @brief Drift speed/CV mapping and optimized phase-increment conversion. SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef FMD_DOMAIN_FREQUENCY_MAPPING_H
#define FMD_DOMAIN_FREQUENCY_MAPPING_H
#include <stdint.h>
#include "fmd/ports/ReferenceTables.h"
namespace fmd {
uint32_t phaseIncrementFromDecihertzQ16_16(uint32_t decihertzQ16_16);
uint32_t getDeltaTime(const IReferenceTables& tables, uint16_t knob, uint16_t cv, int16_t offset);
}  // namespace fmd
#endif
