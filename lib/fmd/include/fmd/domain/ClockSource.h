/**
 * @file ClockSource.h
 * Declares the shared hysteretic quarter-note clock detector and internal fallback.
 *
 * @details
 * Percussion and Dubstep/Bass both reinterpret Speed CV as an optional external
 * 0..5 V quarter-note clock. This component owns only transport detection and
 * period measurement; it does not know about bars, phrases or algorithm state.
 * Consumers receive a phase increment plus one-sample acquisition/loss/boundary
 * events and decide how those timing events affect their own musical state.
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

/**
 * @brief Result of one shared quarter-note clock-source update.
 *
 * Event flags are asserted for exactly one call. `quarterIncrement` always
 * contains the timing source algorithms should use for the current sample:
 * measured external timing while locked, otherwise the caller's internal rate.
 */
struct ClockUpdate {
  uint32_t quarterIncrement;  ///< Active Q0.32 quarter-note phase increment per 2.5 kHz scheduler sample.
  bool externalActive;        ///< True while a measured external clock owns timing.
  bool quarterBoundary;       ///< True on an accepted external rising edge after a valid reference interval.
  bool externalAcquired;      ///< True only on the edge that changes ownership from internal to external timing.
  bool externalLost;          ///< True only on the sample that times out and returns ownership to internal timing.
};

/**
 * @brief Convert a measured quarter-note interval to a 32-bit phase increment.
 * @param periodSamples Measured scheduler samples between accepted rising edges.
 *        Values outside the qualified 32..25000 range are clamped defensively.
 * @return Rounded Q0.32 phase increment that advances one full cycle in
 *         `periodSamples` scheduler samples.
 */
uint32_t quarterIncrementFromPeriodSamples(uint32_t periodSamples);

/**
 * @brief Return the external-clock loss timeout for a measured period.
 * @param periodSamples Last accepted quarter-note period in scheduler samples;
 *        defensively clamped to the qualified period range.
 * @return Timeout equal to 2.5 measured periods, expressed in scheduler samples.
 */
uint32_t clockTimeoutSamples(uint32_t periodSamples);

/**
 * @brief Hysteretic 0..5 V clock detector with automatic internal-tempo fallback.
 *
 * Two accepted rising edges are required before external timing becomes active.
 * A valid edge is detected only after the input has first fallen to or below
 * kClockLowThresholdAdc and then rises to or above kClockHighThresholdAdc.
 * Each accepted external edge represents one quarter note. Edges closer than
 * kClockMinimumPeriodSamples are treated as glitches and do not replace the
 * previous reference. Loss of a locked clock after 2.5 periods returns timing
 * ownership to the caller-supplied internal increment.
 *
 * @note This class implements signal interpretation only. The current Drift
 * hardware is electrically specified for 0..5 V on this repurposed input;
 * raw 10 V Eurorack trigger/clock levels are not made safe by this software.
 */
class ClockSource {
 public:
  /** @brief Construct an unlocked detector in the LOW/input-ready state. */
  ClockSource();

  /**
   * @brief Sample the clock input and update timing ownership.
   * @param clockInputAdc Latest 10-bit Speed-CV ADC code; values above 1023 are clamped.
   * @param internalQuarterIncrement Q0.32 quarter-note phase increment to use whenever no external lock is active.
   * @return Active timing increment and one-sample transport events for this scheduler sample.
   */
  ClockUpdate update(uint16_t clockInputAdc, uint32_t internalQuarterIncrement);

  /** @return true while two-edge-qualified external timing owns the clock. */
  bool externalActive() const { return externalActive_; }

  /** @return Last accepted external quarter-note period in scheduler samples, or zero before first lock. */
  uint32_t lastPeriodSamples() const { return lastPeriodSamples_; }

 private:
  bool inputHigh_;                     ///< Hysteresis state; true after crossing the HIGH threshold until LOW is reached.
  bool haveReferenceEdge_;             ///< True after one usable rising edge has established a period-measurement origin.
  bool externalActive_;                ///< True after a second valid edge establishes external timing ownership.
  uint32_t samplesSinceReferenceEdge_; ///< Saturating scheduler-sample counter since the current reference/accepted edge.
  uint32_t lastPeriodSamples_;          ///< Last accepted external quarter-note interval in scheduler samples.
  uint32_t externalQuarterIncrement_;   ///< Q0.32 phase increment derived from `lastPeriodSamples_`.
};

}  // namespace fmd::clock

#endif  // FMD_DOMAIN_CLOCK_SOURCE_H
