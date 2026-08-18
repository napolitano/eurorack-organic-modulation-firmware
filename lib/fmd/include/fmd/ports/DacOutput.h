/** @file DacOutput.h @brief Platform-independent queued DAC port. SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef FMD_PORTS_DAC_OUTPUT_H
#define FMD_PORTS_DAC_OUTPUT_H
#include <stdint.h>
namespace fmd {
class IDacOutput {
 public:
  virtual ~IDacOutput() {}
  virtual bool ready() const = 0;
  virtual void queue12Bit(uint16_t value) = 0;
};
}  // namespace fmd
#endif
