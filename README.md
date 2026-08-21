# Free Modular Drift — Alternative C++/PlatformIO Firmware

[![CI](https://github.com/napolitano/eurorack-organic-modulation-firmware/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/napolitano/eurorack-organic-modulation-firmware/actions/workflows/ci.yml)
[![Latest release](https://img.shields.io/github/v/release/napolitano/eurorack-organic-modulation-firmware?include_prereleases&sort=semver&display_name=tag&label=release)](https://github.com/napolitano/eurorack-organic-modulation-firmware/releases)
[![GPL-3.0-or-later](https://img.shields.io/badge/license-GPL--3.0--or--later-blue.svg)](LICENSE)
![PlatformIO / C++17](https://img.shields.io/badge/PlatformIO-C%2B%2B17-orange?logo=platformio)
![ATmega328P](https://img.shields.io/badge/target-ATmega328P-00979D?logo=arduino)
![AVR resource guardrails](https://img.shields.io/badge/resource%20guardrails-flash%20%E2%89%A485%25%20%7C%20SRAM%20%E2%89%A465%25-success)

**28 algorithms. Seven banks. The original 4 HP Drift hardware.**

This repository provides an independent C++17/PlatformIO firmware for the **Free Modular Drift** Eurorack modulation source. It keeps the original module recognisably Drift, preserves the four original algorithms in the **Classic** bank, and expands the same Arduino Nano / ATmega328P hardware into six additional musical banks covering organic motion, generative structures, long-form ambient modulation, tempo-shaped electronic modulation, percussion/rhythm generation and tempo-relative bass modulation.

No PCB modification is required. Changing banks is a firmware update; the two rear DIP switches then select one of the four algorithms inside the installed bank.

> [!NOTE]
> **Release status:** release `0.3.0` is the current stable **7-bank / 28-algorithm** release. It adds the Dubstep / Bass bank with Wobble, Growl, Chop and Build while retaining the original 4 HP Arduino Nano / ATmega328P hardware target.

**Start here:** [Install or change a bank](docs/installation/README.md) · [User manual](docs/manual/README.md) · [Choose a bank](#choose-your-bank) · [Latest release](https://github.com/napolitano/eurorack-organic-modulation-firmware/releases) · [Developer/testing guide](README_TESTING.md)

> [!IMPORTANT]
> **When flashing or changing banks, power Drift from USB only.** Switch the Eurorack case off and disconnect the module's Eurorack ribbon cable **before** connecting USB to the Arduino Nano. Do not power the module from USB and the Eurorack PSU at the same time during the update procedure.

> [!NOTE]
> ### With appreciation to Quinn Freedman
> **Drift is Quinn Freedman's instrument.** The original Free Modular hardware, musical concept and Rust firmware are the foundation of this repository. This project is an independent continuation: it keeps the original identity visible, documents every deliberate deviation, and treats the upstream design as a reference rather than something to be quietly overwritten.
>
> - [Free Modular Drift](https://freemodular.org/modules/Drift/)
> - [Quinn Freedman's upstream source](https://github.com/QuinnFreedman/modular/tree/main/modules/Drift)
>
> This repository is unofficial and is not affiliated with Free Modular.

## What Drift becomes with this firmware

Drift is still a compact **0–10 V unipolar modulation source**. The difference is the range of behaviours available from the same front panel. Depending on the installed bank, the module can move smoothly, wander statistically, remember patterns, develop motifs, breathe over minutes, generate tempo-shaped contours or produce phrase-aware rhythmic events.

The banks are deliberately different rather than collections of minor variations on the same noise source:

| Bank | Algorithms | Musical character | Guide |
|---|---|---|---|
| **Classic** | Perlin · Brownian · Bézier · LFO | The original Drift vocabulary: smooth noise, random walk, curved motion and periodic modulation | [Classic](README-BANK-CLASSIC.md) |
| **Organic** | Fractal · Vector · Rain · Attractor | Naturalistic multiscale motion, coupled flow, sparse events and nonlinear structure | [Organic](README-BANK-ORGANIC.md) |
| **Generative** | Turing · Markov · Motif · Urn | Repetition with memory: mutable loops, state transitions, evolving phrases and learned preferences | [Generative](README-BANK-GENERATIVE.md) |
| **Ambient** | Current · Anchor · Breath · Fog | Slow form, mean-reverting drift, recurrent swells and soft modulation clouds | [Ambient](README-BANK-AMBIENT.md) |
| **Electronica** | Pump · Acid · Shuffle · Polymeter | Tempo-shaped CV for house, acid, techno and related electronic styles | [Electronica](README-BANK-ELECTRONICA.md) |
| **Percussion** | Euclid · Repeat · Probability · Humanize | Rhythmic event placement, ratchets, weighted variation, fills and humanised timing | [Percussion](README-BANK-PERCUSSION.md) |
| **Dubstep / Bass** | Wobble · Growl · Chop · Build | Tempo-locked bass motion, syncopated articulation and phrase-scale escalation | [Dubstep / Bass](README-BANK-DUBSTEP.md) |

> [!TIP]
> ### A small problem of abundance
> With **28 algorithms across 7 banks**, the user is left with the rather luxurious problem of deciding which four algorithms to have installed today. One Drift can, by stubborn physical reality, run only one bank at a time. A second Drift is therefore an entirely defensible engineering response. A third begins to look like sensible redundancy. Beyond that, the distinction between *need* and *system architecture* becomes increasingly academic. Strictly speaking, convincing evidence that one can own too many Drifts has yet to emerge.

## Choose your bank

A firmware image contains **one bank**. The rear DIP switches do **not** choose the bank; they choose one of four slots inside the bank that has been flashed.

The four physical selector states are shared by every bank:

| Slot | Rear DIP 1 | Rear DIP 2 |
|---:|---|---|
| **1** | OFF | OFF |
| **2** | ON | OFF |
| **3** | OFF | ON |
| **4** | ON | ON |

After flashing, those four slots mean:

| Bank | Slot 1 · OFF/OFF | Slot 2 · ON/OFF | Slot 3 · OFF/ON | Slot 4 · ON/ON |
|---|---|---|---|---|
| **Classic** | Perlin | Brownian | Bézier | LFO |
| **Organic** | Fractal | Vector | Rain | Attractor |
| **Generative** | Turing | Markov | Motif | Urn |
| **Ambient** | Current | Anchor | Breath | Fog |
| **Electronica** | Pump | Acid | Shuffle | Polymeter |
| **Percussion** | Euclid | Repeat | Probability | Humanize |
| **Dubstep / Bass** | Wobble | Growl | Chop | Build |

The switches are sampled at startup. **ON is the upper physical switch position.** Change the switches only while the module is unpowered, then power-cycle Drift for the new algorithm to take effect.

> [!IMPORTANT]
> This four-algorithm limit is a property of the existing hardware selector. Firmware cannot turn two binary rear switches into six bank selectors. **Flashing chooses the bank; the DIP switches choose the algorithm inside it.**

## First patch in five minutes

<p align="center">
  <img src="docs/manual/assets/drift-front-panel.svg" alt="Free Modular Drift front panel showing Speed, Texture, Attenuation, two CV inputs, output and status LED" width="360">
</p>

| Panel element | What it does |
|---|---|
| **Speed** | Primary time-scale or activity control; exact behaviour depends on the active algorithm |
| **Texture** | Secondary shape, structure, density or character control |
| **Attenuation** | Analogue scaling of the final 0–10 V output; firmware cannot read this knob |
| **Speed CV** | Normally a 0–5 V modulation input; in Percussion and Dubstep / Bass it becomes an optional 0–5 V quarter-note clock input |
| **Texture CV** | 0–5 V modulation of the algorithm-specific Texture parameter |
| **Output** | 0–10 V unipolar modulation/event output |
| **LED** | Visual indication of the current output level |

1. [Install the bank you want](docs/installation/README.md).
2. With the module unpowered, set the rear DIP switches for the algorithm you want.
3. Reinstall Drift and power the rack normally.
4. Patch **OUT** to filter cutoff, wavetable position, FM amount, effect depth, panning, a VCA, or another CV-controlled destination.
5. Start with **Speed** and **Texture** near the middle. Turn **Attenuation** fully clockwise while learning the mode, then reduce it to suit the destination.
6. Open the relevant [bank guide](#algorithm-bank-guides) for the exact control mapping and musical behaviour.

> [!WARNING]
> **Percussion and Dubstep / Bass: Speed CV is a 0–5 V clock input. Do not patch 10 V Eurorack clocks or triggers into Speed CV on the current hardware.** The existing analogue input was designed for 0–5 V operation; firmware cannot add overvoltage protection. Without a valid external clock, Percussion and Dubstep/Bass automatically return to the Speed-knob internal clock.

## Install, update or change the algorithm bank

You do **not** need a development environment to install a prebuilt release. Every tagged release provides a `.hex` file for each bank and for both supported Arduino Nano bootloaders.

The safe end-user workflow is:

1. Decide which **bank** you want.
2. Download the matching `.hex` for the Nano's **new** or **old** bootloader.
3. Switch the Eurorack case **off** and disconnect the module's **Eurorack ribbon cable**.
4. Connect USB to the installed Arduino Nano. USB powers the Nano for the update.
5. Flash the `.hex` with AVRDUDESS on Windows, AVRDUDE, or the documented PlatformIO workflow.
6. Disconnect USB after a successful upload.
7. Set the rear DIP switches for the desired algorithm while the module is unpowered.
8. Reconnect the Eurorack ribbon cable with the case still off, reinstall the module, then power the rack.

> [!CAUTION]
> **Never attach USB for this update procedure while Drift is still connected to the Eurorack power bus.** USB-only flashing is intentional; simultaneous USB and Eurorack PSU power is not part of the supported update procedure.

The complete guide covers bank selection, bootloader choice, Windows 11 / AVRDUDESS, macOS/Linux, troubleshooting and bootloader recovery:

**[→ Firmware installation and bank-switching guide](docs/installation/README.md)**

For Windows 11 there is also a control-by-control AVRDUDESS walkthrough with a numbered screenshot:

**[→ AVRDUDESS step-by-step guide](docs/installation/avrdudess/README.md)**

## Why this alternative firmware exists

The original Drift is attractive precisely because it is small, understandable and musically specific. This project is not trying to turn it into a menu-driven workstation. Instead, it asks how far the original hardware can be taken while keeping the interaction simple and the implementation defensible.

The main goals are:

- preserve the original four algorithms in a clearly identified **Classic** bank;
- add new banks that occupy genuinely different musical territory rather than renaming similar random processes;
- make every algorithm's mathematics and control semantics explicit;
- keep inactive-bank state out of the AVR image through compile-time selection;
- separate portable DSP/domain code from Arduino hardware access;
- verify behaviour with mathematical, unit, integration, property, regression and system tests;
- document hardware limits rather than pretending software can remove them;
- provide prebuilt binaries, reproducible releases and an end-user manual so using the firmware does not require becoming an embedded developer.

Release **0.3.0** extends the six-bank `0.2.0` platform to **seven banks and 28 algorithms** while retaining compatibility with the original Arduino Nano / ATmega328P hardware.

## Community and project stance

This project is intended to be approachable both to people who simply want more musical options from Drift and to developers who want to understand or extend the implementation.

A few principles matter here:

- **Upstream credit stays visible.** Quinn Freedman's hardware and original firmware remain the starting point.
- **Compatibility and changes are distinguished.** The Classic bank preserves the instrument's original vocabulary; verified defects and project-defined extensions are documented rather than silently folded into history.
- **Claims should be testable.** Mathematical behaviour, resource use and release contents have automated contracts where practical.
- **Hardware limitations are stated plainly.** The current Percussion/Dubstep clock warning and the four-selector-state bank limit are examples.
- **User-facing documentation matters.** The README, bank guides, installation guide and maintained manual are treated as part of the product, not as an afterthought.

Bug reports, reproducible hardware observations and focused contributions are welcome through GitHub. Development conventions are documented in [CONTRIBUTING.md](CONTRIBUTING.md), [README_TESTING.md](README_TESTING.md) and the [development documentation](docs/development/).

## Contents

- [What Drift becomes with this firmware](#what-drift-becomes-with-this-firmware)
- [Choose your bank](#choose-your-bank)
- [First patch in five minutes](#first-patch-in-five-minutes)
- [Install, update or change the algorithm bank](#install-update-or-change-the-algorithm-bank)
- [Why this alternative firmware exists](#why-this-alternative-firmware-exists)
- [Community and project stance](#community-and-project-stance)
- [Algorithm-bank guides](#algorithm-bank-guides)
- [User manual](#user-manual)
- [Verification and tests](#verification-and-tests)
- [Engineering architecture](#engineering-architecture)
- [Build from source](#build-from-source)
- [Release artifacts](#release-artifacts)
- [Release history](#release-history)
- [Release process](#release-process)
- [Upstream and licence](#upstream-and-licence)

## Algorithm-bank guides

Each bank has a dedicated guide with visual DIP maps, control semantics, mathematical background, musical use cases, implementation constraints and build/test commands.

| Guide | Focus |
|---|---|
| **[Classic](README-BANK-CLASSIC.md)** | Perlin, Brownian, Bézier and LFO; original Drift behaviour and upstream findings |
| **[Organic](README-BANK-ORGANIC.md)** | Fractal, Vector, Rain and Attractor; multiscale, coupled, event-driven and nonlinear motion |
| **[Generative](README-BANK-GENERATIVE.md)** | Turing, Markov, Motif and Urn; memory, recurrence and evolving structures |
| **[Ambient](README-BANK-AMBIENT.md)** | Current, Anchor, Breath and Fog; slow-form movement and texture |
| **[Electronica](README-BANK-ELECTRONICA.md)** | Pump, Acid, Shuffle and Polymeter; tempo-oriented electronic modulation |
| **[Percussion](README-BANK-PERCUSSION.md)** | Euclid, Repeat, Probability and Humanize; phrase structure, fills, repeats and clocked rhythm |
| **[Dubstep / Bass](README-BANK-DUBSTEP.md)** | Wobble, Growl, Chop and Build; tempo-relative bass motion and phrase-scale modulation |

The algorithm derivations and engineering assessments live under [docs/analysis](docs/analysis/algorithms/README.md).

## User manual

The maintained manual is the complete reference for release `0.3.0`: all seven banks and all twenty-eight algorithms. The frozen `0.2.0` manual remains the historical six-bank / 24-algorithm release record.

- [Manual workspace and publication notes](docs/manual/README.md)
- [Editable ODT source](docs/manual/drift-user-manual.odt)
- [Frozen release sources](docs/manual/releases/README.md)
- [Reusable vector diagrams](docs/manual/assets/README.md)

Tagged releases publish a versioned ODT and PDF generated from the frozen source for that tag. Ordinary pushes and pull requests do not create release-manual artifacts.

## Verification and tests

The repository tests the production implementation rather than maintaining a separate algorithm model only for tests.

Release 0.3.0 contains **236 native test cases across 39 suites** and **64 acceptance criteria**; see [README_TESTING.md](README_TESTING.md) for the qualification matrix and coverage policy. The released platform covers:

- mathematical reference behaviour for all 28 algorithms;
- fixed-point, frequency, RNG and reference-table primitives;
- state-machine and boundary behaviour;
- regression tests for verified upstream findings;
- bank-aware selection, runtime and signal-path integration;
- property/invariant tests across control ranges;
- sanitizer qualification;
- AVR flash/SRAM and timing guardrails;
- bank-owned coverage gates.

The six extended banks each enforce **97% aggregate line / 90% branch coverage**, plus **95% line / 80% branch per production file**. Classic retains its established compatibility/core coverage contract, while the shared `ClockSource` is qualified independently at **97% line / 90% branch** so it is not misattributed to Classic, Percussion or Dubstep/Bass. Coverage is a regression floor, not a substitute for mathematical or requirement-level verification.

AVR release guardrails are deliberately stricter than the ATmega328P hard limits:

- application flash: **≤ 85%** (`26,112 / 30,720` bytes);
- static SRAM: **≤ 65%** (`1,331 / 2,048` bytes).

See [README_TESTING.md](README_TESTING.md) and [requirements traceability](docs/testing/requirements-traceability.md).

## Engineering architecture

```text
Arduino entry points
        |
FirmwareController              src/platform/nano_atmega328p/
        |
   DriftRuntime                 lib/fmd/application/
        |
    DriftEngine                 lib/fmd/domain/
        |
 compile-time bank
 /   |    |    |    |    |    \
classic organic generative ambient electronica percussion dubstep
 \\   |    |    |    |    |    /
 bank-local algorithms          lib/fmd/domain/<bank>/
        |
minimal ports                   lib/fmd/ports/
        |
AVR ADC / DAC / LED / tables    src/platform/nano_atmega328p/
```

The portable core never calls Arduino GPIO/SPI/timing APIs directly. Hardware dependencies terminate at small ports implemented by the Nano/ATmega328P platform layer. Algorithm code is grouped by bank under `domain/classic`, `domain/organic`, `domain/generative`, `domain/ambient`, `domain/electronica`, `domain/percussion` and `domain/dubstep`; genuinely shared fixed-point, frequency, RNG and engine support remains at the domain root.

Production C++ uses complete provenance/licence headers and Doxygen contracts. See the [source-code reference](docs/development/source-code-reference.md) and [code documentation conventions](docs/development/code-documentation.md). The GitHub Wiki is a generated mirror of canonical repository documentation; its publication contract is documented under [docs/wiki](docs/wiki/README.md).

New GitHub issues receive a single documentation-aware first-pass review from the clearly identified **Free Modular Drift Triage-Agent**, implemented as a GitHub Agentic Workflow using GitHub Copilot. It never closes issues: strong documented-feature matches point to concrete user documentation, ambiguous reports get targeted first-aid references, and likely defects are prepared and assigned for maintainer review. The policy, evidence gates, Copilot setup and security boundary are documented in the [issue triage agent reference](docs/development/issue-triage-agent.md).

## Build from source

End users do not need to build the firmware; use the prebuilt release HEX files and the [installation guide](docs/installation/README.md). Developers have two deliberately different PlatformIO workflows: build/flash a **complete bank** with normal rear-DIP selection, or build/flash a **named algorithm target** that ignores the DIP switches for on-device testing.

### Build or flash a complete bank

| Bank | New Nano bootloader | Old Nano bootloader |
|---|---|---|
| Classic | `pio run -e nanoatmega328new` | `pio run -e nanoatmega328` |
| Organic | `pio run -e nanoatmega328new_organic` | `pio run -e nanoatmega328_organic` |
| Generative | `pio run -e nanoatmega328new_generative` | `pio run -e nanoatmega328_generative` |
| Ambient | `pio run -e nanoatmega328new_ambient` | `pio run -e nanoatmega328_ambient` |
| Electronica | `pio run -e nanoatmega328new_electronica` | `pio run -e nanoatmega328_electronica` |
| Percussion | `pio run -e nanoatmega328new_percussion` | `pio run -e nanoatmega328_percussion` |
| Dubstep / Bass | `pio run -e nanoatmega328new_dubstep` | `pio run -e nanoatmega328_dubstep` |

Append `-t upload` to flash the selected bank. The resulting firmware behaves like a normal release image: both rear DIP switches remain active and select one of the bank's four algorithms at startup.

### Build or flash one algorithm by name

For bench/on-device testing, the firmware can instead be compile-time locked to a **named algorithm**. The algorithm name determines its bank automatically and the rear DIP switches are ignored in that developer build. `DriftEngine` references only that selected algorithm, so normal AVR linker garbage collection can discard the other three bank algorithms from the final developer image. No numeric slot or algorithm ID is part of the user-facing command.

The easiest cross-platform interface is the target helper:

```bash
# Flash the complete Ambient bank; DIP switches remain active.
python scripts/flash_drift.py bank ambient

# Flash only the Breath developer target; DIP switches are ignored.
python scripts/flash_drift.py algorithm breath

# Compile without uploading.
python scripts/flash_drift.py --build-only algorithm euclid

# Select the old Nano bootloader and an explicit serial port.
python scripts/flash_drift.py --bootloader old --port COM5 algorithm markov
```

The same mechanism can be invoked directly through PlatformIO. On Linux/macOS:

```bash
FMD_FORCE_ALGORITHM=breath pio run -e nanoatmega328new_ambient -t upload
```

PowerShell:

```powershell
$env:FMD_FORCE_ALGORITHM = "breath"
pio run -e nanoatmega328new_ambient -t upload
Remove-Item Env:FMD_FORCE_ALGORITHM
```

Valid algorithm names are: `perlin`, `brownian`, `bezier`, `lfo`, `fractal`, `vector`, `rain`, `attractor`, `turing`, `markov`, `motif`, `urn`, `current`, `anchor`, `breath`, `fog`, `pump`, `acid`, `shuffle`, `polymeter`, `euclid`, `repeat`, `probability`, `humanize`, `wobble`, `growl`, `chop`, `build`. Names are case-insensitive; `Bézier` is normalised to `bezier`.

> [!IMPORTANT]
> **Named algorithm builds are developer/test images, not release flavours.** A normal bank build and every tagged release leave the override unset, so rear-DIP selection remains functional. The build tooling also rejects an algorithm name when it does not belong to the selected PlatformIO bank environment.

For native tests, sanitizers, coverage, timing targets and the named-target verification path, see [README_TESTING.md](README_TESTING.md).

## Release artifacts

Tagged releases publish every bank present in the tagged source for both supported Arduino Nano bootloaders. End users normally need the `.hex` file only.

For version `X.Y.Z`:

| Bank | New bootloader HEX | Old bootloader HEX |
|---|---|---|
| Classic | `fm-drift-classic-nano-new-bootloader.X.Y.Z.hex` | `fm-drift-classic-nano-old-bootloader.X.Y.Z.hex` |
| Organic | `fm-drift-organic-nano-new-bootloader.X.Y.Z.hex` | `fm-drift-organic-nano-old-bootloader.X.Y.Z.hex` |
| Generative | `fm-drift-generative-nano-new-bootloader.X.Y.Z.hex` | `fm-drift-generative-nano-old-bootloader.X.Y.Z.hex` |
| Ambient | `fm-drift-ambient-nano-new-bootloader.X.Y.Z.hex` | `fm-drift-ambient-nano-old-bootloader.X.Y.Z.hex` |
| Electronica | `fm-drift-electronica-nano-new-bootloader.X.Y.Z.hex` | `fm-drift-electronica-nano-old-bootloader.X.Y.Z.hex` |
| Percussion | `fm-drift-percussion-nano-new-bootloader.X.Y.Z.hex` | `fm-drift-percussion-nano-old-bootloader.X.Y.Z.hex` |
| Dubstep / Bass | `fm-drift-dubstep-nano-new-bootloader.X.Y.Z.hex` | `fm-drift-dubstep-nano-old-bootloader.X.Y.Z.hex` |

Matching `.elf` files are included for debugging/provenance. Release `0.3.0` contains **28 firmware files** (seven banks × two bootloaders × HEX/ELF) and fourteen `BUILD-INFO` files, plus `FIRMWARE-ARTIFACTS.X.Y.Z.md`, the frozen versioned manual ODT/PDF pair and checksum manifests. Release `0.2.0` remains the historical six-bank / 24-firmware-file baseline.

## Release history

| Version | Summary |
|---|---|
| **0.3.0** | Seven banks / 28 algorithms; Dubstep / Bass with Wobble, Growl, Chop and Build; named single-algorithm developer targets; shared rhythm-bank clock source; seven-bank manual and release packaging |
| **0.2.0** | Six banks / 24 algorithms; Organic, Generative, Ambient, Electronica and Percussion; Percussion 0–5 V clock mode; bank-organised domain code; expanded testing/coverage; full multi-bank manual; bank-aware release packaging |
| **0.1.0** | Initial C++17/PlatformIO release with the four Classic algorithms, documented upstream corrections, native/AVR CI, engineering analyses and tagged PDF manual |

The authoritative history is [CHANGELOG.md](CHANGELOG.md).

## Release process

Ordinary development remains under `## Unreleased` in [CHANGELOG.md](CHANGELOG.md). A release is prepared by freezing its manual source, finalising the versioned changelog section and package metadata, then pushing a version tag. The release workflow builds all bank/bootloader images present in that tag, validates the manual and generates deterministic release notes, provenance files and checksums.

Maintainers can also refresh/recreate an existing tag's GitHub Release without moving the tag. See [release process](docs/development/release-process.md).

## Upstream and licence

This repository is an independent alternative firmware for the original Free Modular Drift hardware and is not an official Free Modular repository.

- Original project and hardware documentation: <https://freemodular.org/modules/Drift/>
- Quinn Freedman's upstream source: <https://github.com/QuinnFreedman/modular/tree/main/modules/Drift>

Firmware code is distributed under **GPL-3.0-or-later**. See [LICENSE](LICENSE). The maintained end-user manual has its own **CC BY-NC 4.0** licence; see [docs/manual/LICENSE](docs/manual/LICENSE).

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="docs/assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
