/** @file ReferenceTables.h @brief Read-only reference-table port. SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef FMD_PORTS_REFERENCE_TABLES_H
#define FMD_PORTS_REFERENCE_TABLES_H
#include <stdint.h>
namespace fmd {
class IReferenceTables {
 public:
  virtual ~IReferenceTables() {}
  virtual uint32_t exp2Q16_16(uint8_t index) const = 0;
  virtual int16_t triangularIcdfQ1_15(uint16_t index) const = 0;
  virtual uint8_t gamma8(uint8_t index) const = 0;
};
}  // namespace fmd
#endif
