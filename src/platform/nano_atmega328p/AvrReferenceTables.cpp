/**
 * @file AvrReferenceTables.cpp
 * Implements PROGMEM-backed numerical lookup tables for ATmega328P.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "platform/nano_atmega328p/AvrReferenceTables.h"

#include <avr/pgmspace.h>

namespace fmd::platform::nano {
namespace {

/// 256-entry base-2 exponential table, Q16.16, stored in flash.
const uint32_t kExp2TableQ16F16[256] PROGMEM = {
#include "fmd/config/generated/Exp2Table.inc"
};

/// 257-entry symmetric triangular inverse-CDF table, Q1.15, stored in flash.
const int16_t kTriangularIcdfTableQ1F15[257] PROGMEM = {
#include "fmd/config/generated/IcdfTable.inc"
};

/// 256-entry gamma 2.2 LED transfer table stored in flash.
const uint8_t kGammaTable8[256] PROGMEM = {
#include "fmd/config/generated/GammaTable.inc"
};

}  // namespace

uint32_t AvrReferenceTables::exp2Q16_16(uint8_t index) const {
  return pgm_read_dword(&kExp2TableQ16F16[index]);
}

int16_t AvrReferenceTables::triangularIcdfQ1_15(uint16_t index) const {
  return static_cast<int16_t>(pgm_read_word(&kTriangularIcdfTableQ1F15[index]));
}

uint8_t AvrReferenceTables::gamma8(uint8_t index) const {
  return pgm_read_byte(&kGammaTable8[index]);
}

}  // namespace fmd::platform::nano
