/**
 * @file FixedMath.h
 * Declares small fixed-point arithmetic primitives shared by Drift algorithms.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_DOMAIN_FIXED_MATH_H
#define FMD_DOMAIN_FIXED_MATH_H

#include <stdint.h>

namespace fmd::fixedmath {

/**
 * @brief Multiply two signed Q1.15 values.
 * @param firstQ1F15 First signed Q1.15 factor.
 * @param secondQ1F15 Second signed Q1.15 factor.
 * @return Product rounded by arithmetic truncation back to signed Q1.15.
 */
int16_t mulI1F15(int16_t firstQ1F15, int16_t secondQ1F15);

/**
 * @brief Linearly interpolate unsigned values with a Q0.16 interpolation weight.
 * @param weightQ0F16 Interpolation weight from 0 toward 1 in unsigned Q0.16.
 * @param startValue Value returned at zero weight.
 * @param endValue Value approached at full-scale weight.
 * @return Interpolated unsigned 16-bit value.
 */
uint16_t lerpU0F16(uint16_t weightQ0F16, uint16_t startValue, uint16_t endValue);

/**
 * @brief Linearly interpolate signed Q1.15 endpoints with a signed Q1.15 weight.
 * @param weightQ1F15 Non-negative interpolation weight represented in Q1.15.
 * @param startQ1F15 Signed Q1.15 start endpoint.
 * @param endQ1F15 Signed Q1.15 end endpoint.
 * @return Interpolated signed Q1.15 value.
 */
int16_t lerpI1F15(int16_t weightQ1F15, int16_t startQ1F15, int16_t endQ1F15);

/**
 * @brief Linearly interpolate unsigned endpoints using a Q4.12 interpolation weight.
 * @param weightQ4F12 Interpolation weight where 0 represents 0 and 4096 represents 1.
 * @param startValue Unsigned start endpoint.
 * @param endValue Unsigned end endpoint.
 * @return Interpolated unsigned 16-bit value.
 */
uint16_t lerpU4F12(uint16_t weightQ4F12, uint16_t startValue, uint16_t endValue);

/**
 * @brief Linearly interpolate unsigned Q16.16 endpoints with a Q0.16 weight.
 * @param weightQ0F16 Interpolation weight in unsigned Q0.16.
 * @param startQ16F16 Q16.16 start endpoint.
 * @param endQ16F16 Q16.16 end endpoint.
 * @return Interpolated Q16.16 value.
 */
uint32_t lerpU16F16(uint32_t weightQ0F16, uint32_t startQ16F16, uint32_t endQ16F16);

}  // namespace fmd::fixedmath
#endif  // FMD_DOMAIN_FIXED_MATH_H
