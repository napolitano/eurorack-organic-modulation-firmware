/** @file ParallelLfsr.h @brief Upstream-compatible paired 16-bit LFSRs. SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef FMD_DOMAIN_PARALLEL_LFSR_H
#define FMD_DOMAIN_PARALLEL_LFSR_H
#include <stdint.h>
namespace fmd {
class ParallelLfsr {
 public:
  explicit ParallelLfsr(uint16_t seed);
  uint16_t next();
 private:
  static uint16_t step(uint16_t state, const uint8_t taps[4]);
  uint16_t state1_;
  uint16_t state2_;
};
}  // namespace fmd
#endif
