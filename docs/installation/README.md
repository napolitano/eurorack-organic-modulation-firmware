# Installing, Updating or Changing the Drift Firmware Bank

[← Main README](../../README.md) · [User manual](../manual/README.md) · [Windows 11 / AVRDUDESS guide](avrdudess/README.md)

This guide explains how to install a **prebuilt tagged release** on the original Free Modular Drift hardware without changing the PCB. Drift uses an Arduino Nano / ATmega328P and can be flashed through the Nano's USB connector while the Nano remains installed on the module.

Changing the firmware is also how you change the **algorithm bank**. The rear DIP switches select only one of four algorithms inside the bank that is currently flashed.

> [!IMPORTANT]
> This is the end-user installation path. Developers building or uploading directly from source should use the [build section in the main README](../../README.md#build-from-source) and the repository's PlatformIO configuration.

## Contents

- [Safety first](#safety-first)
- [What you need](#what-you-need)
- [Step 1 — choose the algorithm bank](#step-1--choose-the-algorithm-bank)
- [Step 2 — choose the Nano bootloader variant](#step-2--choose-the-nano-bootloader-variant)
- [Step 3 — remove and isolate the module](#step-3--remove-and-isolate-the-module)
- [Step 4 — connect the Nano by USB](#step-4--connect-the-nano-by-usb)
- [Step 5 — flash the HEX file](#step-5--flash-the-hex-file)
- [Step 6 — select the algorithm with the rear DIP switches](#step-6--select-the-algorithm-with-the-rear-dip-switches)
- [Step 7 — reinstall and verify](#step-7--reinstall-and-verify)
- [macOS and Linux](#macos-and-linux)
- [Troubleshooting](#troubleshooting)
- [Bootloader recovery](#bootloader-recovery)
- [References](#references)

## Safety first

> [!CAUTION]
> **POWER THE EURORACK CASE OFF BEFORE REMOVING DRIFT.** Do not connect USB while the module is still connected to the Eurorack power bus.

> [!CAUTION]
> **DISCONNECT THE EURORACK RIBBON CABLE BEFORE CONNECTING USB.** This is mandatory for this update procedure. Do not power Drift from the Eurorack PSU and the Arduino Nano USB connection at the same time.

> [!CAUTION]
> **DO NOT TOUCH THE EXPOSED PCB, ARDUINO NANO, HEADERS OR COMPONENTS WHILE USB POWER IS PRESENT.** Handle the removed module by the front-panel edges and keep tools, screws, rails and other conductive objects away from the electronics.

Use normal ESD precautions and work on a clean, dry, nonconductive surface. If the Eurorack case has a detachable external power adapter or mains lead, disconnect it before removing or reinstalling the module.

The Nano may remain installed on the Drift PCB. During flashing, the Nano is powered through USB; the Eurorack ribbon remains disconnected until USB has been removed.

## What you need

- the Drift module;
- a **data-capable USB cable** that fits the installed Nano;
- the correct Drift `.hex` file from the [GitHub release](https://github.com/napolitano/eurorack-organic-modulation-firmware/releases);
- on Windows, [AVRDUDESS](https://github.com/ZakKemble/AVRDUDESS/releases) is the documented graphical path;
- a clean, dry, nonconductive place to put the removed module.

> [!TIP]
> A charging-only USB cable can power the Nano but cannot upload firmware. If no serial port appears, try a known data cable before changing software settings.

## Step 1 — choose the algorithm bank

Choose the bank for the kind of modulation you want. One firmware image contains one bank and therefore four algorithms.

| Bank | Algorithms | Best starting point for |
|---|---|---|
| **Classic** | Perlin · Brownian · Bézier · LFO | Original Drift behaviour and general-purpose modulation |
| **Organic** | Fractal · Vector · Rain · Attractor | Naturalistic, multiscale and nonlinear movement |
| **Generative** | Turing · Markov · Motif · Urn | Evolving loops, recurrent states and generative structures |
| **Ambient** | Current · Anchor · Breath · Fog | Slow movement, swells, texture and long-form modulation |
| **Electronica** | Pump · Acid · Shuffle · Polymeter | Tempo-shaped CV for house, acid, techno and related styles |
| **Percussion** | Euclid · Repeat · Probability · Humanize | Rhythmic pulses, repeats, fills, probability and humanised timing |

For version `X.Y.Z`, the release HEX filename begins with the bank name:

```text
fm-drift-classic-...
fm-drift-organic-...
fm-drift-generative-...
fm-drift-ambient-...
fm-drift-electronica-...
fm-drift-percussion-...
```

> [!IMPORTANT]
> **Flashing selects the bank.** The rear DIP switches cannot move between banks. They select one of the four algorithms in the firmware image that is currently installed.

## Step 2 — choose the Nano bootloader variant

Every current bank is released in two Arduino Nano upload variants:

| Nano bootloader | Filename ending | Baud rate |
|---|---|---:|
| **New bootloader / Optiboot** | `nano-new-bootloader.X.Y.Z.hex` | `115200` |
| **Old Nano bootloader** | `nano-old-bootloader.X.Y.Z.hex` | `57600` |

For example, the Organic bank in release `0.2.0` is available as:

```text
fm-drift-organic-nano-new-bootloader.0.2.0.hex
fm-drift-organic-nano-old-bootloader.0.2.0.hex
```

Both target the same ATmega328P / 16 MHz hardware; the difference is the Nano bootloader/upload timing. Arduino documents the processor/bootloader distinction for the classic Nano. If you do not know which bootloader is installed, start with the new-bootloader image at `115200`. If the uploader cannot establish communication, retry using the old-bootloader image at `57600`.

## Step 3 — remove and isolate the module

1. Switch the Eurorack case **OFF**.
2. If applicable, disconnect the case's external power adapter or mains lead.
3. Wait until the module and case LEDs are dark.
4. Remove the module mounting screws.
5. Pull Drift forward by the **front-panel edges**.
6. Disconnect the Eurorack ribbon cable by its connector housing.
7. Put the module on a clean, dry, nonconductive surface where the PCB cannot touch metal.

> [!CAUTION]
> **THE EURORACK POWER RIBBON MUST BE DISCONNECTED BEFORE USB IS ATTACHED.** Do not leave it connected to the case bus during flashing.

## Step 4 — connect the Nano by USB

With the Eurorack ribbon cable disconnected, connect the USB cable to the installed Arduino Nano and then to the computer.

The Nano can be powered from USB for the update. Do not reconnect the Eurorack ribbon cable while USB remains attached.

## Step 5 — flash the HEX file

### Windows 11

Use the dedicated graphical walkthrough:

**[→ Updating Drift with AVRDUDESS on Windows 11](avrdudess/README.md)**

It includes the numbered AVRDUDESS screenshot, bank/bootloader filename selection, exact programmer/MCU/baud settings and the safe power sequence.

### AVRDUDE command line

For users who already have AVRDUDE installed, the equivalent upload form is:

```bash
avrdude -c arduino -P <SERIAL_PORT> -b <BAUD> -p atmega328p -D -U flash:w:<FIRMWARE.hex>:i
```

Use:

- `115200` with the new-bootloader HEX;
- `57600` with the old-bootloader HEX.

Replace `<SERIAL_PORT>` with the actual device, such as `COM7`, `/dev/cu.usbserial-*` or `/dev/ttyUSB0`.

Do not use `-F` to force past a signature or communication problem. Fix the port, cable, driver, MCU or bootloader selection instead.

## Step 6 — select the algorithm with the rear DIP switches

After the upload has completed successfully:

1. disconnect the USB cable;
2. leave the Eurorack case **off** and the power ribbon **disconnected**;
3. set the rear DIP switches for the algorithm you want.

The four DIP slots are:

| Rear DIP 1 | Rear DIP 2 | Slot |
|---|---|---:|
| OFF | OFF | 1 |
| ON | OFF | 2 |
| OFF | ON | 3 |
| ON | ON | 4 |

The meaning of those slots depends on the flashed bank:

| Bank | Slot 1 | Slot 2 | Slot 3 | Slot 4 |
|---|---|---|---|---|
| Classic | Perlin | Brownian | Bézier | LFO |
| Organic | Fractal | Vector | Rain | Attractor |
| Generative | Turing | Markov | Motif | Urn |
| Ambient | Current | Anchor | Breath | Fog |
| Electronica | Pump | Acid | Shuffle | Polymeter |
| Percussion | Euclid | Repeat | Probability | Humanize |

**ON is the upper physical switch position.** The switches are sampled at startup, so changing them while Drift is already running has no effect until the next power cycle.

## Step 7 — reinstall and verify

After USB has been disconnected and the rear DIP switches are set:

1. make sure the Eurorack case is still **OFF**;
2. reconnect the Eurorack ribbon cable in the correct orientation;
3. reinstall Drift without trapping or straining the cable;
4. tighten the panel screws normally;
5. power the Eurorack case on;
6. verify that Drift responds normally to Speed/Texture and that the selected bank/algorithm behaves as described in its bank guide.

> [!CAUTION]
> **NEVER RECONNECT THE EURORACK POWER BUS WHILE USB IS STILL ATTACHED FOR THIS UPDATE PROCEDURE.** Disconnect USB first; reconnect the rack power cable only while the case remains off.

## macOS and Linux

The same bank and bootloader choice applies. Use AVRDUDE directly or the repository's PlatformIO workflow. AVRDUDESS is primarily documented here as the Windows graphical path.

Useful release parameters remain:

- MCU: `ATmega328P`;
- programmer/protocol: Arduino bootloader / STK500 v1;
- new bootloader: `115200`;
- old bootloader: `57600`.

## Troubleshooting

### No serial port appears

- Try a known data-capable USB cable.
- Try another USB port.
- Close Arduino IDE serial monitors, PlatformIO monitors, terminal programs or other applications that may already own the port.
- Some Nano-compatible boards use a CH340/CH341 USB-to-serial bridge and may require the corresponding driver.

### AVRDUDE cannot communicate with the Nano

- Confirm `ATmega328P` as the MCU.
- Confirm the `arduino` programmer/protocol.
- Confirm the correct serial port.
- Try the other supported bootloader/baud combination.
- Disconnect and reconnect USB, then retry.

### The firmware uploaded, but I got the wrong algorithms

The upload chooses the **bank** and the DIP switches choose the **algorithm**. Check both independently:

1. confirm the flashed HEX filename contains the intended bank name;
2. power Drift off;
3. verify the two rear DIP positions against the table above;
4. power Drift back on.

### Upload starts but fails

Do not use Force (`-F`) to override a device-signature or communication error. Correct the actual port, bootloader, cable, driver or MCU selection.

## Bootloader recovery

A damaged or missing Arduino Nano bootloader is a separate recovery operation and is **not** part of a normal Drift firmware update.

Arduino documents how to burn the bootloader on a classic Nano using another AVR-based Arduino as an ISP programmer:

<https://support.arduino.cc/hc/en-us/articles/4841602539164-Burn-the-bootloader-on-UNO-Mega-and-classic-Nano-using-another-Arduino>

Use bootloader recovery only when normal serial uploading cannot be restored by correcting the cable, serial port, USB driver, MCU or bootloader-speed choice.

## References

- Drift releases: <https://github.com/napolitano/eurorack-organic-modulation-firmware/releases>
- AVRDUDESS: <https://github.com/ZakKemble/AVRDUDESS>
- AVRDUDE documentation: <https://avrdudes.github.io/avrdude/>
- Arduino: select the correct processor for the classic Nano: <https://support.arduino.cc/hc/en-us/articles/4401874304274-Select-the-right-processor-for-Arduino-Nano>
- Arduino: classic Nano bootloader recovery: <https://support.arduino.cc/hc/en-us/articles/4841602539164-Burn-the-bootloader-on-UNO-Mega-and-classic-Nano-using-another-Arduino>
- WCH CH340/CH341 Windows driver: <https://www.wch-ic.com/downloads/CH341SER_ZIP.html>

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
