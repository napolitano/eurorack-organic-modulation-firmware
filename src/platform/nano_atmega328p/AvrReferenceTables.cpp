#include "platform/nano_atmega328p/AvrReferenceTables.h"
#include <avr/pgmspace.h>
namespace fmd::platform::nano {
namespace {
const uint32_t kExp[256] PROGMEM = {
#include "fmd/config/generated/Exp2Table.inc"
};
const int16_t kIcdf[257] PROGMEM = {
#include "fmd/config/generated/IcdfTable.inc"
};
const uint8_t kGamma[256] PROGMEM = {
#include "fmd/config/generated/GammaTable.inc"
};
}
uint32_t AvrReferenceTables::exp2Q16_16(uint8_t i) const { return pgm_read_dword(&kExp[i]); }
int16_t AvrReferenceTables::triangularIcdfQ1_15(uint16_t i) const { return static_cast<int16_t>(pgm_read_word(&kIcdf[i])); }
uint8_t AvrReferenceTables::gamma8(uint8_t i) const { return pgm_read_byte(&kGamma[i]); }
}
