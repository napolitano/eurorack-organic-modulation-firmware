/**
 * @file ParallelLfsr.cpp
 * Implements the upstream-compatible paired 16-bit LFSR generator.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "fmd/domain/ParallelLfsr.h"

namespace fmd {
namespace {

/// One-based taps of the first upstream 16-bit feedback polynomial.
constexpr uint8_t kPrimaryTapPositions[4] = {16U, 14U, 13U, 11U};
/// One-based taps of the second upstream 16-bit feedback polynomial.
constexpr uint8_t kSecondaryTapPositions[4] = {16U, 15U, 13U, 4U};

}  // namespace

ParallelLfsr::ParallelLfsr(uint16_t seed)
    : primaryState_(seed),
      secondaryState_(static_cast<uint16_t>(~static_cast<uint16_t>(
          static_cast<uint16_t>((seed >> 8U) & 0x00FFU) |
          static_cast<uint16_t>(seed << 8U)))) {}

uint16_t ParallelLfsr::advanceRegister(uint16_t state,
                                       const uint8_t tapPositions[4]) {
  uint16_t feedbackBit = 0U;
  for (uint8_t tapIndex = 0U; tapIndex < 4U; ++tapIndex) {
    const uint8_t rightShift = static_cast<uint8_t>(16U - tapPositions[tapIndex]);
    feedbackBit ^= static_cast<uint16_t>(state >> rightShift);
  }

  return static_cast<uint16_t>((state >> 1U) |
                               static_cast<uint16_t>(feedbackBit << 15U));
}

uint16_t ParallelLfsr::next() {
  primaryState_ = advanceRegister(primaryState_, kPrimaryTapPositions);
  secondaryState_ = advanceRegister(secondaryState_, kSecondaryTapPositions);
  return static_cast<uint16_t>(primaryState_ ^ secondaryState_);
}

}  // namespace fmd
