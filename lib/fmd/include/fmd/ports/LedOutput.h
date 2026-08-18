/** @file LedOutput.h @brief Platform-independent LED PWM port. SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef FMD_PORTS_LED_OUTPUT_H
#define FMD_PORTS_LED_OUTPUT_H
#include <stdint.h>
namespace fmd {
class ILedOutput {
 public:
  virtual ~ILedOutput() {}
  virtual void setBrightness(uint8_t duty) = 0;
};
}  // namespace fmd
#endif
