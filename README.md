# Free Modular Drift — Alternative C++/PlatformIO Firmware

<p align="center">
  <a href="https://github.com/napolitano/eurorack-organic-modulation-firmware/actions/workflows/ci.yml"><img src="https://github.com/napolitano/eurorack-organic-modulation-firmware/actions/workflows/ci.yml/badge.svg?branch=main" alt="CI"></a>
  <a href="https://github.com/napolitano/eurorack-organic-modulation-firmware/actions/workflows/manual-publication.yml"><img src="https://github.com/napolitano/eurorack-organic-modulation-firmware/actions/workflows/manual-publication.yml/badge.svg?branch=main" alt="Manual publication"></a>
  <a href="https://github.com/napolitano/eurorack-organic-modulation-firmware/releases"><img src="https://img.shields.io/github/v/release/napolitano/eurorack-organic-modulation-firmware?include_prereleases&sort=semver&display_name=tag&label=release" alt="Latest release"></a>
  <a href="LICENSE"><img src="https://img.shields.io/badge/license-GPL--3.0--or--later-blue.svg" alt="GPL-3.0-or-later"></a>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/PlatformIO-C%2B%2B17-orange?logo=platformio" alt="PlatformIO / C++17">
  <img src="https://img.shields.io/badge/target-ATmega328P-00979D?logo=arduino" alt="ATmega328P">
  <img src="https://img.shields.io/badge/native%20coverage-%E2%89%A595%25%20lines%20%7C%20%E2%89%A575%25%20branches-success" alt="Native coverage gate">
  <img src="https://img.shields.io/badge/resource%20guardrails-flash%20%E2%89%A485%25%20%7C%20SRAM%20%E2%89%A465%25-success" alt="AVR resource guardrails">
</p>

> [!NOTE]
> ### With appreciation to Quinn Freedman
> **Drift is Quinn Freedman's instrument.** The original Free Modular hardware, musical concept and Rust firmware are the foundation of this repository. This alternative firmware exists because that work was published openly and is interesting enough to study carefully, preserve and develop further. The goal here is not to erase the upstream implementation, but to keep its identity intact while making the firmware easier to build, test, document and maintain.
>
> - [Free Modular Drift](https://freemodular.org/modules/Drift/)
> - [Quinn Freedman's upstream source](https://github.com/QuinnFreedman/modular/tree/main/modules/Drift)

<p align="center">
  <img src="docs/manual/assets/drift-front-panel.svg" alt="Drift front panel controls" width="240">
</p>

## What is Drift?

Drift is a **4 HP Eurorack modulation source** that produces evolving 0–10 V control voltages. Its defining idea is controlled movement: instead of choosing between a conventional repeating LFO and completely uncorrelated random values, Drift offers several ways to generate motion that has **continuity, memory, shape or controlled unpredictability**.

The module has two main controls — **Speed** and **Texture** — and CV inputs for both. Four algorithms give those controls different musical meanings:

| Algorithm | Character | What makes it useful musically |
|---|---|---|
| **Perlin** | Smooth, organic gradient noise | Slowly evolving modulation without obvious repetition; useful for timbre, filter, spatial and macro movement |
| **Brownian** | Bounded random walk with memory | Wandering modulation that tends to continue from where it already is instead of jumping to unrelated values |
| **Bézier** | Random destinations connected by shaped transitions | Deliberate-looking rises and falls with controllable curvature and segment timing variation |
| **LFO** | Skewable triangle through rising/falling saw | Deterministic periodic modulation when repeatability is more useful than randomness |

> [!TIP]
> With both configuration switches left open, Drift starts in **Perlin mode**, matching the original hardware default.

## Why this firmware?

The upstream firmware already contains a thoughtful fixed-point implementation of the Drift concept. This project rebuilds it as a **C++17/PlatformIO best-practice firmware** for the original Arduino Nano / ATmega328P hardware, with a different emphasis: behavior should be understandable, mathematically defensible and demonstrably correct.

The project therefore aims to:

- preserve the recognisable Drift instrument and the four original algorithm concepts;
- separate portable signal-processing code from Arduino/AVR hardware access;
- verify each algorithm against its mathematical definition, not merely against historical output bytes;
- retain upstream behavior where it represents intentional musical design;
- correct verified numerical, continuity or state-handling problems transparently;
- document upstream findings neutrally, including why a change was or was not made;
- qualify resource use and real-time behavior on the ATmega328P;
- provide reproducible builds, automated tests, release notes and a versioned PDF user manual.

> [!IMPORTANT]
> **Status: unreleased development firmware.** No release version has been assigned. Hardware timing qualification and final release validation are still required before the first tagged release.

## Quick start

1. **Flash the firmware** to the Arduino Nano used by the original Drift hardware.
2. **Select the algorithm before power-up.** The two configuration inputs are sampled at startup only. Leaving both switches open selects Perlin.
3. **Patch `OUT`** to the parameter you want to modulate.
4. Use **Speed** to set the time scale/activity of the selected algorithm.
5. Use **Texture** to change its character — for example octave content, smoothing, curve shape/timing variation or LFO skew.
6. Patch modulation into **Speed CV** and/or **Texture CV** when you want Drift itself to be modulated.

### Algorithm selection

| CONFIG 1 | CONFIG 2 | Algorithm | Default? |
|---|---|---|---|
| HIGH / open | HIGH / open | **Perlin** | **Yes** |
| HIGH / open | LOW / closed | Brownian | No |
| LOW / closed | HIGH / open | Bézier | No |
| LOW / closed | LOW / closed | LFO | No |

<details>
<summary><strong>What do Speed and Texture mean in each mode?</strong></summary>

| Mode | Speed | Texture |
|---|---|---|
| **Perlin** | Sets the time scale of the noise | Blends in the faster octave, increasing fine movement |
| **Brownian** | Controls stochastic activity through movement probability and step size | Controls how tightly the output follows the underlying random-walk target |
| **Bézier** | Sets the base segment rate | Knob morphs transition shape and contributes to timing variation; Texture CV widens timing variation |
| **LFO** | Sets periodic frequency | Moves the waveform apex from falling saw through triangle to rising saw |

</details>

For installation, operating details, diagrams and mathematical background, see the maintained [user manual](docs/manual/README.md).

## Contents

- [What is Drift?](#what-is-drift)
- [Why this firmware?](#why-this-firmware)
- [Quick start](#quick-start)
- [Contents](#contents)
- [Algorithms and configuration](#algorithms-and-configuration)
- [Original-firmware findings](#original-firmware-findings)
- [Release history](#release-history)
- [Engineering architecture](#engineering-architecture)
- [Verification and tests](#verification-and-tests)
- [Algorithm engineering analyses](#algorithm-engineering-analyses)
- [Build](#build)
- [User manual](#user-manual)
- [Release process](#release-process)
- [Upstream and licence](#upstream-and-licence)

## Algorithms and configuration

The original two configuration inputs are sampled once at startup. The logical mapping follows the upstream Rust booleans, where a pulled-low pin becomes `true`.

The portable `ControlFrame` also preserves the original hardware-visible ADC order:

1. Speed CV — A4
2. Texture CV — A5
3. Speed knob — ADC6/A6
4. Texture knob — ADC7/A7

The output remains a 12-bit MCP4922 Channel-A value; Channel B is unused. The existing single LED follows the output through an 8-bit gamma table.

<table>
<tr>
<td width="50%"><img src="docs/manual/assets/perlin-medium-texture.svg" alt="Medium texture Perlin noise"></td>
<td width="50%"><img src="docs/manual/assets/brownian-texture-comparison.svg" alt="Brownian texture comparison"></td>
</tr>
<tr>
<td width="50%"><img src="docs/manual/assets/bezier-smooth.svg" alt="Smooth Bézier easing"></td>
<td width="50%"><img src="docs/manual/assets/lfo-skew-texture.svg" alt="Skewed triangle LFO"></td>
</tr>
</table>

## Original-firmware findings

The upstream implementation remains the design reference. The table below summarizes the findings that materially affected this firmware. The wording is intentionally descriptive rather than judgmental; detailed derivations and evidence are in the linked developer analyses.

| Area | Upstream observation | Current treatment |
|---|---|---|
| **Brownian — Texture scaling** | Most of the 10-bit Texture range is interpreted directly as raw Q0.16, followed by a separate direct-tracking regime near the top of the control range. | Normalize the complete 0…1023 control range to a documented smoothing range and remove the abrupt regime change. |
| **Brownian — convergence** | Fractional smoothing movement is discarded, so sufficiently small non-zero corrections can quantize to zero before the target is reached. | Retain fractional residual so mathematically non-zero movement continues to accumulate. |
| **Bézier — timing distribution** | The implementation samples a triangular distribution in logarithmic speed space, although older documentation described it as Gaussian. | Keep the triangular model as the implemented musical design and document it accurately. |
| **Bézier — inverse CDF endpoint** | 256 stored values are used for 256 interpolation intervals, causing the upper interpolation index to clamp at the final entry. | Use 257 boundary samples for 256 intervals so both mathematical endpoints are represented. |
| **Bézier — curve selection** | Crossing the Texture midpoint switches directly between two cubic easing families while a segment is running. | Morph continuously between the cubic responses instead of changing family at a single ADC code. |
| **LFO — live Texture changes** | Skew/Texture is latched at cycle rollover, so changes can be delayed by almost a full cycle at slow rates. | Apply Texture immediately and remap phase to preserve the current output as closely as fixed-point resolution permits. |
| **LFO — startup state** | The constructor uses a fixed initial apex and exposes a first-cycle fixed-point edge around the peak. | Initialize and process the requested shape from the first step, removing the startup-only discontinuity. |
| **Perlin — fade evaluation** | Repeated truncating fixed-point polynomial operations can introduce local one-code reversals in an otherwise monotone quintic fade. | Evaluate the canonical quintic with a single rounded integer expression over the effective phase domain. |
| **Frequency mapping — cost** | Frequency-to-phase conversion uses a 64-bit division in a recurring hot path. This is primarily a performance concern rather than a functional defect. | Use reciprocal multiplication plus a bounded exact correction; tests compare the result with the exact rational reference. |

> [!NOTE]
> A correction is accepted only where the intended mathematics or state behavior can be stated and tested. Differences that are part of Drift's musical character are retained even when another implementation might be possible.

See the [original firmware analysis](docs/analysis/original-firmware-analysis.md) and the four [algorithm analyses](docs/analysis/algorithms/README.md) for the full evidence chain.

## Release history

> [!NOTE]
> **There is no released version yet.** Development is intentionally tracked only under `Unreleased` in the [changelog](CHANGELOG.md). This section will become a concise version history after the first tagged release; the changelog remains the authoritative detailed record.

## Engineering architecture

```text
Arduino entry points
        |
FirmwareController              src/platform/nano_atmega328p/
        |
   DriftRuntime                 lib/fmd/application/
        |
    DriftEngine                 lib/fmd/domain/
   /    |    |    \
Perlin Brownian Bezier LFO
        |
minimal ports                   lib/fmd/ports/
        |
AVR ADC / DAC / LED / tables    src/platform/nano_atmega328p/
```

The portable core never calls `analogRead()`, `digitalWrite()`, `SPI.transfer()`, AVR registers or Arduino timing functions. Hardware dependencies terminate at small ports implemented by the Nano/ATmega328P platform layer.

### Engineering goals

- keep `src/main.cpp` as a minimal composition entry point;
- keep portable production code independent of Arduino/AVR APIs;
- execute the real production core in host-side tests rather than duplicating the algorithms in a test model;
- keep mathematical helper functions production-facing so they can be verified directly;
- maintain strict compiler warnings, sanitizer runs, coverage and requirement-traceability gates;
- measure AVR timing and resource costs before accepting performance-sensitive optimisations.

## Verification and tests

The test strategy distinguishes **mathematical correctness**, **state-machine behavior**, **regression protection**, **system behavior** and **hardware qualification**.

Current native coverage includes:

- dedicated mathematical suites for Perlin, Brownian, Bézier and LFO;
- shared fixed-point, frequency, RNG and reference-table tests;
- edge-case and long-run state-transition tests;
- property/invariant tests across the control domain;
- regression tests for corrected upstream findings;
- integration and end-to-end runtime tests using the real production core;
- machine-checked acceptance-criteria traceability;
- sanitizer and coverage environments.

The current unreleased host baseline contains **88 native test cases**, **32 acceptance criteria**, approximately **99.45% line coverage** and **82.82% branch coverage** for the portable production code. Coverage is treated as a regression floor, not as a substitute for requirement or mathematical verification.

AVR builds also carry explicit engineering headroom: **Flash must stay at or below 85% (26,112 / 30,720 bytes)** and **static SRAM at or below 65% (1,331 / 2,048 bytes)**. These are repository guardrails, deliberately stricter than the ATmega328P hard limits.

See [README_TESTING.md](README_TESTING.md) and [requirements traceability](docs/testing/requirements-traceability.md).

## Algorithm engineering analyses

Each algorithm has a dedicated developer analysis that starts from the mathematics and then examines the upstream Rust implementation, actual behavior, computational cost, findings, proposed improvements and verification evidence:

- [Perlin noise](docs/analysis/algorithms/perlin-noise-analysis.md)
- [Brownian / bounded random walk](docs/analysis/algorithms/brownian-motion-analysis.md)
- [Bézier random segments](docs/analysis/algorithms/bezier-random-walk-analysis.md)
- [LFO](docs/analysis/algorithms/lfo-analysis.md)

The common classification policy is documented in [docs/analysis/algorithms/README.md](docs/analysis/algorithms/README.md).

## Build

Firmware builds:

```bash
pio run -e nanoatmega328new
pio run -e nanoatmega328
```

Host verification:

```bash
pio test -e native
pio test -e native_sanitized
pio test -e native_coverage
python scripts/check_requirement_traceability.py
python scripts/check_markdown_footer.py
```

Timing qualification uses the dedicated `nanoatmega328new_timing` environment.

## User manual

The maintained end-user source is [docs/manual/drift-user-manual.odt](docs/manual/drift-user-manual.odt). It remains deliberately unversioned while the firmware is unreleased.

The manual repository area contains:

- [publication and typography notes](docs/manual/README.md);
- [CC BY-NC 4.0 manual licence](docs/manual/LICENSE);
- [reusable vector diagrams](docs/manual/assets/README.md).

A tagged release `vX.Y.Z` produces `drift-user-manual.X.Y.Z.pdf`; the release workflow verifies document structure and embedded Ubuntu/Ubuntu Light fonts before publishing it.

## Release process

All work remains under `## Unreleased` in [CHANGELOG.md](CHANGELOG.md) until a release is explicitly prepared. A version number is introduced only during release preparation.

Tagged releases are built by `.github/workflows/release.yml`. Release notes are generated deterministically from the matching changelog section rather than from generic commit-message aggregation. Firmware binaries, the versioned manual PDF, build provenance and checksum manifests are published together.

See [docs/development/release-process.md](docs/development/release-process.md).

## Upstream and licence

This repository is an independent alternative firmware for the original Free Modular Drift hardware and is not an official Free Modular repository.

- Original project and hardware documentation: <https://freemodular.org/modules/Drift/>
- Quinn Freedman's upstream source: <https://github.com/QuinnFreedman/modular/tree/main/modules/Drift>

Firmware code in this repository is distributed under **GPL-3.0-or-later**. See [LICENSE](LICENSE). The maintained end-user manual has its own **CC BY-NC 4.0** licence; see [docs/manual/LICENSE](docs/manual/LICENSE).

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="docs/assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
