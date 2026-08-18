#ifndef FMD_PLATFORM_NANO_AVR_ANALOG_INPUTS_H
#define FMD_PLATFORM_NANO_AVR_ANALOG_INPUTS_H
#include <stdint.h>
#include "fmd/ports/AnalogInputs.h"
namespace fmd::platform::nano {
class AvrAnalogInputs final : public IAnalogInputs {
 public: void begin(); void beginCycle() override; uint16_t read(uint8_t channel) const override; static void handleIsr();
 private: static volatile uint16_t latest_[4]; static volatile uint8_t index_; uint16_t snapshot_[4]={0,0,0,0};
};
}
#endif
