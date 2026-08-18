#ifndef FMD_TEST_MEMORY_REFERENCE_TABLES_H
#define FMD_TEST_MEMORY_REFERENCE_TABLES_H

#include "fmd/ports/ReferenceTables.h"

class MemoryReferenceTables final : public fmd::IReferenceTables {
 public:
  uint32_t exp2Q16_16(uint8_t index) const override { return exp_[index]; }
  int16_t triangularIcdfQ1_15(uint16_t index) const override { return icdf_[index]; }
  uint8_t gamma8(uint8_t index) const override { return gamma_[index]; }

 private:
  inline static constexpr uint32_t exp_[256] = {
#include "fmd/config/generated/Exp2Table.inc"
  };

  inline static constexpr int16_t icdf_[257] = {
#include "fmd/config/generated/IcdfTable.inc"
  };

  inline static constexpr uint8_t gamma_[256] = {
#include "fmd/config/generated/GammaTable.inc"
  };
};

#endif
