#ifndef FMD_PLATFORM_NANO_AVR_LED_OUTPUT_H
#define FMD_PLATFORM_NANO_AVR_LED_OUTPUT_H
#include "fmd/ports/LedOutput.h"
namespace fmd::platform::nano { class AvrLedOutput final:public ILedOutput{public:void begin();void setBrightness(uint8_t duty) override;}; }
#endif
