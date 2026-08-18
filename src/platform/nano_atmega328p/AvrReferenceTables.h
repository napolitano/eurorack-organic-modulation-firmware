#ifndef FMD_PLATFORM_NANO_AVR_REFERENCE_TABLES_H
#define FMD_PLATFORM_NANO_AVR_REFERENCE_TABLES_H
#include "fmd/ports/ReferenceTables.h"
namespace fmd::platform::nano {
class AvrReferenceTables final : public IReferenceTables {
 public:
  uint32_t exp2Q16_16(uint8_t i) const override;
  int16_t triangularIcdfQ1_15(uint16_t i) const override;
  uint8_t gamma8(uint8_t i) const override;
};
}
#endif
