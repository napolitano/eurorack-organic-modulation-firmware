/**
 * @file FrequencyMapping.h
 * Declares Drift speed/CV mapping and phase-increment conversion.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_DOMAIN_FREQUENCY_MAPPING_H
#define FMD_DOMAIN_FREQUENCY_MAPPING_H

#include <stdint.h>

#include "fmd/ports/ReferenceTables.h"

namespace fmd {

/**
 * @brief Convert a Q16.16 frequency in decihertz to a 32-bit phase increment.
 * @param decihertzQ16F16 Frequency expressed as decihertz in Q16.16 format.
 * @return Per-sample 32-bit phase increment for Drift's 2.5 kHz processing rate.
 *
 * The implementation uses reciprocal multiplication plus a bounded correction
 * so the result is mathematically identical to the rounded integer division
 * without executing a 64-bit division in the AVR hot path.
 */
uint32_t phaseIncrementFromDecihertzQ16_16(uint32_t decihertzQ16F16);

/**
 * @brief Map Speed knob/CV and an algorithmic frequency offset to phase increment.
 * @param referenceTables Exponential lookup-table provider.
 * @param speedKnobAdc Speed potentiometer ADC value; defensively clamped to 0..1023.
 * @param speedCvAdc Speed CV ADC value; defensively clamped to 0..1023.
 * @param rawFrequencyOffset Signed algorithm-specific offset applied before exp2 mapping.
 * @return 32-bit phase increment for one 2.5 kHz processing step.
 */
uint32_t phaseIncrementFromControls(const IReferenceTables& referenceTables,
                                    uint16_t speedKnobAdc,
                                    uint16_t speedCvAdc,
                                    int16_t rawFrequencyOffset);

}  // namespace fmd
#endif  // FMD_DOMAIN_FREQUENCY_MAPPING_H
