/**
 * @file FirmwareController.h
 * Declares the Arduino Nano composition root for the Drift firmware.
 *
 * @author Axel Napolitano
 * @note Original Free Modular Drift concept and Rust firmware by Quinn Freedman.
 * @copyright Copyright (C) 2026 Axel Napolitano
 * @license GPL-3.0-or-later
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef FMD_PLATFORM_NANO_FIRMWARE_CONTROLLER_H
#define FMD_PLATFORM_NANO_FIRMWARE_CONTROLLER_H

namespace fmd::platform::nano {

/**
 * @brief Wires AVR adapters to the portable Drift runtime and owns startup flow.
 *
 * This class is intentionally thin: hardware configuration and dependency
 * composition live here, while modulation behaviour remains in lib/fmd.
 */
class FirmwareController {
 public:
  /** @brief Configure hardware, select algorithm, seed state and start the timer. */
  void begin();

  /** @brief Execute one foreground opportunity to process a new sample. */
  void run();
};

}  // namespace fmd::platform::nano
#endif  // FMD_PLATFORM_NANO_FIRMWARE_CONTROLLER_H
