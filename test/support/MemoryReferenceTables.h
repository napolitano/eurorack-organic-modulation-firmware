/**
 * @file MemoryReferenceTables.h
 * Provides generated reference tables in normal host memory for native tests.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_TEST_MEMORY_REFERENCE_TABLES_H
#define FMD_TEST_MEMORY_REFERENCE_TABLES_H

#include "fmd/ports/ReferenceTables.h"

/** @brief Host-memory implementation of the production reference-table port. */
class MemoryReferenceTables final : public fmd::IReferenceTables {
 public:
  uint32_t exp2Q16_16(uint8_t index) const override {
    return exp2TableQ16F16_[index];
  }

  int16_t triangularIcdfQ1_15(uint16_t index) const override {
    return triangularIcdfTableQ1F15_[index];
  }

  uint16_t anchorInnovationGainQ1_15(uint16_t index) const override {
    return anchorInnovationGainQ1F15_[index > 306U ? 306U : index];
  }

  uint8_t gamma8(uint8_t index) const override {
    return gammaTable8_[index];
  }

 private:
  inline static constexpr uint32_t exp2TableQ16F16_[256] = {
#include "fmd/config/generated/Exp2Table.inc"
  };

  inline static constexpr int16_t triangularIcdfTableQ1F15_[257] = {
#include "fmd/config/generated/IcdfTable.inc"
  };

  inline static constexpr uint16_t anchorInnovationGainQ1F15_[307] = {
#include "fmd/config/generated/AnchorGainTable.inc"
  };

  inline static constexpr uint8_t gammaTable8_[256] = {
#include "fmd/config/generated/GammaTable.inc"
  };
};

#endif  // FMD_TEST_MEMORY_REFERENCE_TABLES_H
