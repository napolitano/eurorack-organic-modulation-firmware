#ifndef FMD_PLATFORM_NANO_AVR_MCP4922_DAC_H
#define FMD_PLATFORM_NANO_AVR_MCP4922_DAC_H
#include <stdint.h>
#include "fmd/ports/DacOutput.h"
namespace fmd::platform::nano {
class AvrMcp4922Dac final:public IDacOutput{
 public:void begin();bool ready() const override;void queue12Bit(uint16_t value) override;static void latchFromTimerIsr();uint16_t missedLatchCount() const;
 private:static volatile bool queued_;static volatile uint16_t missed_;};
}
#endif
