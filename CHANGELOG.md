# Changelog

This changelog records release-relevant changes to the firmware and repository. New work remains under `Unreleased` until a release is explicitly prepared. A versioned release section is created only as part of release preparation and must begin with a `### Release summary` prose paragraph; the release workflow uses that paragraph as the opening of the generated GitHub Release notes.

## Unreleased

## 0.2.0 — 2026-08-19

### Release summary

Version 0.2.0 expands Free Modular Drift from the four-algorithm Classic baseline into a six-bank, twenty-four-algorithm modulation platform for the original ATmega328P hardware. Organic, Generative, Ambient, Electronica and Percussion add distinct continuous, memory-based, long-form and rhythm-oriented behaviors; Percussion also supports an optional 0–5 V quarter-note clock on Speed CV with automatic Speed-knob fallback. The release adds bank-organized domain code, mathematical and system verification across all six banks, a substantially expanded user manual, immutable release-manual snapshots, and bank-aware release packaging for both supported Nano bootloaders.

### Added

- **Organic bank:** Fractal, Vector, Rain and Attractor, with fixed-point implementations, bank-specific mathematical tests, engineering analyses, documentation and dedicated native/AVR qualification environments.
- **Generative bank:** Turing, Markov, Motif and Urn, providing mutating-loop, finite-state, phrase-transformation and leaky-reinforcement behaviors with deterministic/statistical verification.
- **Ambient bank:** Current, Anchor, Breath and Fog, covering quasiperiodic long-form motion, mean reversion, recurrent macro-gestures and smooth stochastic cloudlets on an intentionally slower musical time scale.
- **Electronica bank:** Pump, Acid, Shuffle and Polymeter, using an internal 30–240 BPM scheduler for rhythmically legible CV aimed at house, acid, techno and related electronic styles.
- **Percussion bank:** Euclid, Repeat, Probability and Humanize, with a shared 16-step bar model, 4/8/12/16-bar phrase structure and phrase-aware fills where appropriate.
- **Percussion external clock mode:** Speed CV is repurposed in this bank as an optional **0–5 V quarter-note clock input**. Two valid rising edges acquire external timing; clock loss after 2.5 periods automatically falls back to the Speed-knob 30–240 BPM source without resetting the running bar/phrase counters.
- Explicit current-hardware safety warnings in the main README, Percussion guide and user manual: **10 V Eurorack clocks/triggers are unsupported on Speed CV** until the input hardware is revised.
- Dedicated bank-level design documents and per-algorithm engineering analyses for all twenty newly added algorithms, including mathematical contracts, duplication audits, ATmega328P feasibility, verification strategy and musical assessment.
- Dedicated root-level bank guides for Organic, Generative, Ambient, Electronica and Percussion, alongside the Classic guide.
- Expanded native verification to **194 test cases across 35 suites and 59 acceptance criteria**, with bank-specific mathematical, integration, property, system, sanitizer, coverage, AVR resource-budget and timing paths.
- Expanded the maintained user manual to all **six banks and twenty-four algorithms**, including bank-specific DIP diagrams, mathematical foundations, origin/musical-value overviews, Percussion clock guidance and **59 maintained SVG assets**.
- Release-preparation manual freezing under `docs/manual/releases/X.Y.Z/`, with immutable byte-for-byte ODT snapshots and historical tag-based backfill support for releases predating the mechanism.
- Bank-aware release packaging for every bank present in a tag. A current six-bank release publishes both Nano bootloader variants as HEX and ELF, yielding **24 firmware files**, plus **12 per-image BUILD-INFO provenance files**, the frozen ODT/PDF manual pair, firmware manifest and checksum manifests.
- Manual `refresh` and `recreate` maintenance modes for rebuilding an existing release tag without moving or rewriting the Git tag.
- CI/release structural contract tests that fail if a bank silently drops out of native tests, coverage, sanitizers, AVR builds, timing qualification, manual coverage or release packaging.

### Changed

- Refactored the portable domain tree into bank-owned subdirectories under `domain/classic`, `domain/organic`, `domain/generative`, `domain/ambient`, `domain/electronica` and `domain/percussion`; shared engine, fixed-point, frequency and RNG support remains at the domain root.
- `DriftEngine` now owns and dispatches only the compile-time-selected bank, so inactive-bank algorithm state does not consume SRAM in a firmware image.
- Generic selection, integration, property and system tests are bank-aware and exercise the four DIP slots of the active compile-time bank.
- The user manual now treats algorithm banks as the primary navigation model, with separate bank sections, consistent per-algorithm structure and a compact six-bank overview; it also documents the unavoidable four-selector-state hardware limit and, with due seriousness, the resulting case for owning more than one Drift.
- Release dependency installation was hardened with canonical Ubuntu mirrors, bounded APT retries/timeouts, minimal LibreOffice packages, an explicit job timeout and per-tag workflow concurrency.
- Dependabot version updates are consolidated into a monthly multi-ecosystem update to reduce automated pull-request noise.
- Release notes, artifact manifests, provenance records and integrity checks now derive the actual bank set from the tagged source, preserving compatibility with historical Classic-only and intermediate tags.

### Fixed

- Corrected the shared Electronica/Percussion tempo mapping so the documented endpoints are actually **30–240 BPM** rather than the previous factor-of-four-under-scaled 7.5–60 BPM runtime behavior.
- Corrected Humanize positive-jitter scheduling so delayed events are offset from their nominal grid boundary rather than from event preparation, preserving the intended no-drift timing contract.
- Corrected coverage/release metadata and workflow guards after the domain-by-bank refactor so nested bank sources remain included in the intended qualification paths.

## 0.1.0 — 2026-08-19

### Release summary

Version 0.1.0 establishes the first C++17/PlatformIO firmware release for Free Modular Drift. It preserves Drift's four modulation concepts while correcting documented numerical and state-handling issues, adds mathematical and regression verification, comprehensive source-level Doxygen documentation, AVR resource and timing guardrails, reproducible tag-driven release tooling, and a versioned PDF user manual generated only for tagged releases.

### Added

- Comprehensive Quantizer-style source documentation across production and native-test support code, including standard file headers, Doxygen API contracts, fixed-point/range annotations and targeted inline rationale for non-obvious numerical and AVR-specific paths.
- Local `Doxyfile`, documented code-documentation conventions and an automated repository guardrail for source headers, generated-table provenance and readability.
- Shared Drift orange-red heart asset plus automated footer validation for user-facing Markdown documentation.
- Reusable, raster-free SVG redraws of the manual's front-panel reference, algorithm diagrams and configuration-switch illustrations under `docs/manual/assets/`, with automated vector-asset validation and selected figures reused in repository documentation.
- Maintained end-user manual source under `docs/manual/`, with a separate CC BY-NC 4.0 licence and publication notes for the required Ubuntu/Ubuntu Light typography.
- Reproducible manual publication tooling integrated into the tag-driven release workflow, converting the unversioned ODT source into a versioned release PDF and rejecting release artifacts with missing/substituted Ubuntu fonts.
- C++17/PlatformIO firmware architecture with a portable `lib/fmd` core and ATmega328P ports-and-adapters hardware boundary.
- Perlin, Brownian, Bézier and LFO implementations with deterministic paired-LFSR random generation.
- Dedicated mathematical unit-test suites for each of the four algorithms, plus shared fixed-point/frequency/reference-table suites, covering reference equations, boundaries, monotonicity, continuity, distribution behavior, long-run behavior and state-transition properties.
- Native unit, integration, property, regression and system tests with machine-checked requirement traceability; the 0.1.0 baseline contains 88 test cases and 32 acceptance criteria.
- Native coverage, sanitizer, Nano old/new bootloader, AVR resource-budget and timing-probe CI targets.
- Detailed developer analyses for all four algorithms, including mathematical foundations, upstream implementation review, computational cost, optimization opportunities and verification strategy.
- GitHub issue forms, pull-request template, security policy, contributor guidance, dependency automation and tag-driven release workflow.
- Deterministic changelog-based GitHub Release-note generation with checksum manifests and build provenance metadata.

### Changed

- Internal identifiers were renamed where needed to express domain intent more clearly, including phase accumulators, algorithm state, hardware adapters and the control-to-phase mapping API; these are readability changes rather than intended behavior changes.
- Generated lookup-table fragments now carry deterministic generator provenance and a mathematical description.
- The reference-table native suite is explicitly included in the per-suite CI matrix so every native suite is exercised individually.
- Root README was updated to remove the redundant introductory front-panel image while retaining the annotated Quick Start reference and to describe the complete 0.1.0 release contents.
- Mathematical expressions in the algorithm engineering analyses now use GitHub-native MathJax delimiters for correctly rendered inline and display equations; CI rejects legacy delimiters and unbalanced display-math blocks.
- Dependabot version updates are grouped and reduced to a monthly cadence with at most one open version-update PR per ecosystem to avoid dependency-PR bursts; security updates remain governed separately by GitHub.
- Root README quick-start documentation expanded with front-panel, DIP-switch and per-algorithm SVG guidance, including the physical rear-switch numbering and user-facing control behavior.
- Repository identity prepared for `napolitano/eurorack-organic-modulation-firmware`, including GitHub status badges and stricter ATmega328P engineering guardrails of 85% application flash and 65% static SRAM.
- Root README reorganized around upstream acknowledgement, a user-oriented explanation of Drift, project rationale, quick-start guide, repository index, neutral upstream-findings table and concise release history.
- Manual PDF validation no longer treats the ODT cached `meta:page-count` statistic as a release contract; release validation relies on page geometry, source-content checks and strict embedded Ubuntu/Ubuntu Light verification instead.
- End-user manual content was reviewed against the corrected firmware: generic Speed/output wording now distinguishes Brownian and LFO behavior, Bezier timing is documented as triangular log-speed variation rather than Gaussian intervals, continuous curve morphing is described accurately, original-hardware power figures are restored, and every algorithm section now ends with mathematical foundations and a musical interpretation.
- Perlin gradient selection and phase advancement and Bézier speed-variation scaling are exposed as pure production math contracts so state/edge behavior can be verified without test-only implementations.
- Frequency mapping clamps public ADC inputs to the documented 10-bit hardware domain before scaling, preventing out-of-contract values from wrapping intermediate arithmetic.
- The redundant Bézier post-interpolation 12-bit clamp was removed after dense-grid verification established that interpolation remains inside its 12-bit endpoint interval by construction.
- Brownian Texture now maps the complete 0..1023 control domain to a documented smoothing range instead of interpreting the raw ADC code as Q0.16.
- Brownian smoothing retains fractional movement so convergence cannot stall solely because a sub-code step was truncated.
- Bézier curve evaluation uses single-rounding integer polynomial forms that preserve the mathematical monotonicity of the cubic curves.
- Bézier Texture continuously morphs between the two cubic response families instead of switching abruptly at the former 511/512 boundary.
- Bézier triangular inverse-CDF interpolation uses 257 boundary entries for 256 intervals, restoring the positive endpoint and removing the final plateau.
- Bézier and LFO phase accumulators preserve wrap overshoot rather than discarding residual phase at cycle boundaries.
- LFO Texture is applied on the first processing step and live skew changes remap phase to preserve output continuity as closely as the discrete representation permits.
- LFO endpoint settings implement exact rising and falling sawtooth waveforms.
- Perlin fade evaluation uses an exact, monotone integer form of the canonical quintic over the effective fixed-point phase domain.
- Frequency-to-phase conversion replaces the hot-path 64-bit division with reciprocal multiplication plus bounded exact correction while retaining the mathematically rounded result.
- The project policy now treats the upstream firmware as the behavioral/design reference while correcting verified defects instead of preserving them as default behavior.

### Fixed

- Fixed the Arduino composition-root linkage by including `Arduino.h` in `src/main.cpp`, so `setup()` and `loop()` use the linkage expected by the AVR Arduino core.
- Corrected the manual PDF font validator so `pdffonts` names containing spaces (for example `Ubuntu Light`) are parsed as a single font name instead of being truncated at the first space.
- Release publication now uses Ubuntu 24.04's current `fonts-ubuntu` package and prints resolved/embedded font diagnostics before validating the manual artifact.
- Native runtime/system suites now include their production dependency explicitly, matching the Quantizer test pattern and ensuring PlatformIO LDF links `lib/fmd` even when `DriftRuntime` is otherwise reached only through `DriftTestRig.h`.
- Native PlatformIO tests now link nested portable sources reliably.
- Shared native reference tables are header-only so every PlatformIO test suite receives the same definitions without relying on out-of-suite support translation units.
- Corrected local fixed-point polynomial evaluations that could introduce one-code reversals in otherwise monotone Perlin/Bézier reference functions.

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="docs/assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
