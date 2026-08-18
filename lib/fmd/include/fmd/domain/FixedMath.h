/** @file FixedMath.h @brief Raw fixed-point helpers matching upstream formats. SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef FMD_DOMAIN_FIXED_MATH_H
#define FMD_DOMAIN_FIXED_MATH_H
#include <stdint.h>
namespace fmd::fixedmath {
int16_t mulI1F15(int16_t a, int16_t b);
uint16_t lerpU0F16(uint16_t t, uint16_t a, uint16_t b);
int16_t lerpI1F15(int16_t t, int16_t a, int16_t b);
uint16_t lerpU4F12(uint16_t t, uint16_t a, uint16_t b);
uint32_t lerpU16F16(uint32_t t, uint32_t a, uint32_t b);
}  // namespace fmd::fixedmath
#endif
