/**
 * @file ClockSource.h
 * Declares the shared hysteretic quarter-note clock detector and internal fallback.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_DOMAIN_CLOCK_SOURCE_H
#define FMD_DOMAIN_CLOCK_SOURCE_H

#include <stdint.h>

namespace fmd::clock {

/** ADC threshold corresponding to approximately 1.0 V on a 0..5 V input. */
constexpr uint16_t kClockLowThresholdAdc = 205U;
/** ADC threshold corresponding to approximately 2.0 V on a 0..5 V input. */
constexpr uint16_t kClockHighThresholdAdc = 410U;
/** Reject implausibly fast edge intervals below 12.8 ms at the 2.5 kHz scheduler rate. */
constexpr uint32_t kClockMinimumPeriodSamples = 32UL;
/** Accept external quarter-note periods up to 10 s (6 BPM). */
constexpr uint32_t kClockMaximumPeriodSamples = 25000UL;

/** @brief Result of one shared quarter-note clock-source update. */
struct ClockUpdate {
  uint32_t quarterIncrement;  ///< Active quarter-note phase increment.
  bool externalActive;        ///< True while a measured external clock owns timing.
  bool quarterBoundary;       ///< True on an accepted external rising edge.
  bool externalAcquired;      ///< True only on the edge that activates external sync.
  bool externalLost;          ///< True only on the sample that times out to internal timing.
};

/** Convert a measured quarter-note interval to a 32-bit phase increment. */
uint32_t quarterIncrementFromPeriodSamples(uint32_t periodSamples);
/** Return the 2.5-period external-clock loss timeout in scheduler samples. */
uint32_t clockTimeoutSamples(uint32_t periodSamples);

/**
 * @brief Hysteretic 0..5 V clock detector with automatic internal-tempo fallback.
 *
 * Two accepted rising edges are required before external timing becomes active.
 * A valid edge is detected after the input has first fallen to or below
 * kClockLowThresholdAdc and then rises to or above kClockHighThresholdAdc.
 * Each accepted external edge represents one quarter note.
 */
class ClockSource {
 public:
  ClockSource();
  ClockUpdate update(uint16_t clockInputAdc, uint32_t internalQuarterIncrement);
  bool externalActive() const { return externalActive_; }
  uint32_t lastPeriodSamples() const { return lastPeriodSamples_; }

 private:
  bool inputHigh_;
  bool haveReferenceEdge_;
  bool externalActive_;
  uint32_t samplesSinceReferenceEdge_;
  uint32_t lastPeriodSamples_;
  uint32_t externalQuarterIncrement_;
};

}  // namespace fmd::clock

#endif  // FMD_DOMAIN_CLOCK_SOURCE_H
