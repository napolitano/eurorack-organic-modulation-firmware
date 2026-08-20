/**
 * @file WobbleAlgorithm.cpp
 * Implements the tempo-synchronised deterministic rate-phrase modulation algorithm.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "fmd/domain/dubstep/WobbleAlgorithm.h"

#include "fmd/domain/AlgorithmMath.h"
#include "fmd/domain/dubstep/DubstepAlgorithmMath.h"

namespace fmd {

WobbleAlgorithm::WobbleAlgorithm(const IReferenceTables& referenceTables)
    : referenceTables_(referenceTables),
      clockSource_(),
      gridPhase_(0U),
      carrierPhase_(0U),
      cellIndex_(0U),
      latchedTextureRegion_(0U),
      externalQuarterIndex_(0U),
      externalHalfAdvanced_(false),
      initialized_(false) {}

void WobbleAlgorithm::start(uint16_t textureControl) {
  gridPhase_ = 0U;
  carrierPhase_ = 0U;
  cellIndex_ = 0U;
  latchedTextureRegion_ = dubstepmath::textureRegion(textureControl);
  externalQuarterIndex_ = 0U;
  externalHalfAdvanced_ = false;
  initialized_ = true;
}

void WobbleAlgorithm::advanceCell(uint16_t textureControl) {
  cellIndex_ = static_cast<uint8_t>((cellIndex_ + 1U) & 0x07U);
  if (cellIndex_ == 0U) {
    latchedTextureRegion_ = dubstepmath::textureRegion(textureControl);
  }
}

void WobbleAlgorithm::handleExternalQuarter(uint16_t textureControl) {
  externalQuarterIndex_ = static_cast<uint8_t>((externalQuarterIndex_ + 1U) & 0x03U);
  cellIndex_ = static_cast<uint8_t>(externalQuarterIndex_ << 1U);
  gridPhase_ = 0U;
  externalHalfAdvanced_ = false;
  if (cellIndex_ == 0U) {
    latchedTextureRegion_ = dubstepmath::textureRegion(textureControl);
  }
}

uint16_t WobbleAlgorithm::step(const ControlFrame& controls) {
  const uint16_t texture = sumAdc(controls.textureKnob, controls.textureCv);
  const uint32_t internalQuarter = dubstepmath::quarterNotePhaseIncrement(
      referenceTables_, controls.speedKnob);
  const clock::ClockUpdate clock = clockSource_.update(controls.speedCv, internalQuarter);

  if (!initialized_) {
    start(texture);
  }

  if (clock.externalAcquired) {
    start(texture);
  } else if (clock.externalActive) {
    if (clock.quarterBoundary) {
      handleExternalQuarter(texture);
    } else {
      bool rollover = false;
      gridPhase_ = perlinmath::advancePhase(gridPhase_, clock.quarterIncrement * 2UL, rollover);
      if (rollover && !externalHalfAdvanced_) {
        externalHalfAdvanced_ = true;
        advanceCell(texture);
      }
    }
  } else {
    bool rollover = false;
    gridPhase_ = perlinmath::advancePhase(gridPhase_, clock.quarterIncrement * 2UL, rollover);
    if (rollover) {
      advanceCell(texture);
    }
  }

  const uint16_t output = dubstepmath::q0F12ToDac12(
      dubstepmath::triangleQ0F12(carrierPhase_));
  const uint8_t symbol = wobblemath::phraseSymbol(cellIndex_);
  const uint8_t rate = wobblemath::rateCode(latchedTextureRegion_, symbol);
  carrierPhase_ += wobblemath::carrierIncrement(clock.quarterIncrement, rate);
  return output;
}

}  // namespace fmd
