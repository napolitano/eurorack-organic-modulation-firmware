#include "fmd/application/DriftRuntime.h"
namespace fmd {
DriftRuntime::DriftRuntime(Algorithm algorithm,uint16_t seed,IAnalogInputs& analog,IDacOutput& dac,ILedOutput& led,const IReferenceTables& tables)
 : analog_(analog),dac_(dac),led_(led),tables_(tables),engine_(algorithm,seed,tables),lastOutput_(0) {}
bool DriftRuntime::processIfReady() {
  if(!dac_.ready()) return false;
  analog_.beginCycle();
  ControlFrame c{analog_.read(0),analog_.read(1),analog_.read(2),analog_.read(3)};
  lastOutput_=engine_.step(c);
  led_.setBrightness(tables_.gamma8(static_cast<uint8_t>(lastOutput_>>4U)));
  dac_.queue12Bit(lastOutput_);
  return true;
}
}  // namespace fmd
