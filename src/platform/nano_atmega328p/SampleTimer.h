/**
 * @file SampleTimer.h
 * Declares the fixed-rate Timer1 scheduler used to latch Drift DAC samples.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_PLATFORM_NANO_SAMPLE_TIMER_H
#define FMD_PLATFORM_NANO_SAMPLE_TIMER_H

namespace fmd::platform::nano {

/** @brief Configures Timer1 CTC mode for the fixed 2.5 kHz DAC latch cadence. */
class SampleTimer {
 public:
  /** @brief Configure and enable Timer1 compare-A interrupts. */
  static void begin();

  /** @brief Minimal compare-A ISR body: timing probe and DAC latch only. */
  static void handleIsr();
};

}  // namespace fmd::platform::nano
#endif  // FMD_PLATFORM_NANO_SAMPLE_TIMER_H
