/** @file AnalogInputs.h @brief Platform-independent four-channel ADC snapshot port. SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef FMD_PORTS_ANALOG_INPUTS_H
#define FMD_PORTS_ANALOG_INPUTS_H
#include <stdint.h>
namespace fmd {
class IAnalogInputs {
 public:
  virtual ~IAnalogInputs() {}
  virtual void beginCycle() = 0;
  virtual uint16_t read(uint8_t channel) const = 0;
};
}  // namespace fmd
#endif
