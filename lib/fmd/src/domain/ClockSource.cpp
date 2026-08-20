/**
 * @file ClockSource.cpp
 * Implements the shared hysteretic quarter-note clock detector and internal fallback.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "fmd/domain/ClockSource.h"

#include "fmd/domain/Types.h"

namespace fmd::clock {

uint32_t quarterIncrementFromPeriodSamples(uint32_t periodSamples) {
  if (periodSamples < kClockMinimumPeriodSamples) {
    periodSamples = kClockMinimumPeriodSamples;
  }
  if (periodSamples > kClockMaximumPeriodSamples) {
    periodSamples = kClockMaximumPeriodSamples;
  }
  const uint64_t numerator = (UINT64_C(1) << 32U) + (periodSamples / 2U);
  return static_cast<uint32_t>(numerator / periodSamples);
}

uint32_t clockTimeoutSamples(uint32_t periodSamples) {
  if (periodSamples < kClockMinimumPeriodSamples) {
    periodSamples = kClockMinimumPeriodSamples;
  }
  if (periodSamples > kClockMaximumPeriodSamples) {
    periodSamples = kClockMaximumPeriodSamples;
  }
  return static_cast<uint32_t>((static_cast<uint64_t>(periodSamples) * 5ULL + 1ULL) / 2ULL);
}

ClockSource::ClockSource()
    : inputHigh_(false),
      haveReferenceEdge_(false),
      externalActive_(false),
      samplesSinceReferenceEdge_(0U),
      lastPeriodSamples_(0U),
      externalQuarterIncrement_(0U) {}

ClockUpdate ClockSource::update(uint16_t clockInputAdc, uint32_t internalQuarterIncrement) {
  clockInputAdc = clampAdc(clockInputAdc);

  if (haveReferenceEdge_ && samplesSinceReferenceEdge_ < UINT32_MAX) {
    ++samplesSinceReferenceEdge_;
  }

  bool risingEdge = false;
  if (inputHigh_) {
    if (clockInputAdc <= kClockLowThresholdAdc) {
      inputHigh_ = false;
    }
  } else if (clockInputAdc >= kClockHighThresholdAdc) {
    inputHigh_ = true;
    risingEdge = true;
  }

  bool externalLost = false;
  if (externalActive_ && samplesSinceReferenceEdge_ > clockTimeoutSamples(lastPeriodSamples_)) {
    externalActive_ = false;
    haveReferenceEdge_ = false;
    samplesSinceReferenceEdge_ = 0U;
    externalLost = true;
  }

  bool quarterBoundary = false;
  bool externalAcquired = false;
  if (risingEdge) {
    if (haveReferenceEdge_) {
      const uint32_t measuredPeriod = samplesSinceReferenceEdge_;
      if (measuredPeriod >= kClockMinimumPeriodSamples &&
          measuredPeriod <= kClockMaximumPeriodSamples) {
        externalAcquired = !externalActive_;
        externalActive_ = true;
        lastPeriodSamples_ = measuredPeriod;
        externalQuarterIncrement_ = quarterIncrementFromPeriodSamples(measuredPeriod);
        quarterBoundary = true;
        samplesSinceReferenceEdge_ = 0U;
      }
      // A too-fast edge is a glitch and does not replace the previous reference.
      // A too-slow source has already timed out above.
    } else {
      haveReferenceEdge_ = true;
      samplesSinceReferenceEdge_ = 0U;
    }
  }

  return ClockUpdate{
      externalActive_ ? externalQuarterIncrement_ : internalQuarterIncrement,
      externalActive_,
      quarterBoundary,
      externalAcquired,
      externalLost};
}

}  // namespace fmd::clock
