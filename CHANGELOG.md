# Changelog

This changelog records release-relevant changes to the firmware and repository. New work remains under `Unreleased` until a release is explicitly prepared. A versioned release section is created only as part of release preparation and must begin with a `### Release summary` prose paragraph; the release workflow uses that paragraph as the opening of the generated GitHub Release notes.

## Unreleased

### Added

- Ambient-bank design specification defining Current, Anchor, Breath and Fog as four distinct forms of long-form continuous modulation, with an explicit duplication audit against Classic, Organic and Generative.
- Dedicated engineering analyses for all four Ambient algorithms, covering mathematical foundations, control contracts, ATmega328P feasibility, verification strategy and explicit musical assessment.
- The initial Ambient working title `Tide` is replaced by `Current` to avoid confusion with Mutable Instruments Tides in the Eurorack context.
- Implemented the Ambient compile-time bank with Current, Anchor, Breath and Fog, shared fixed-point Ambient math and dedicated new/old Nano, native, coverage, sanitizer and timing environments.
- Added four Ambient unit suites covering Current ratio/weight contracts, Anchor mean reversion, Breath cycle topology and Fog cloudlet/occupancy behavior; current `Unreleased` coverage is 47 acceptance criteria and 145 native test cases across 26 suites.
- Added `README-BANK-AMBIENT.md` with bank-specific DIP mapping, controls, implemented mathematical contracts and build/test commands.
- Release tooling now recognizes Ambient as a fourth bank, packages both Nano bootloader variants with per-image provenance, and refuses an Ambient release until the frozen user manual documents Ambient, Current, Anchor, Breath and Fog.
- Generative-bank design specification defining Turing, Markov, Motif and Urn as four distinct forms of discrete musical memory, with a duplication audit against Classic and Organic.
- Dedicated engineering analyses for all four Generative algorithms, including mathematical contracts, AVR cost/test strategy and explicit musical assessment.
- Implemented the Generative compile-time bank with Turing, Markov, Motif and Urn production algorithms, shared pure integer math primitives and dedicated PlatformIO build/test/timing environments for both Nano bootloaders.
- Added four Generative mathematical unit suites covering exact mutation/transition/edit/reinforcement contracts, circular phrase operations, weighted categorical selection, deterministic fixed-seed behavior and DAC-domain invariants.
- Acceptance criteria AC-38 through AC-42 cover Generative bank selection and its four algorithm contracts.
- Added `README-BANK-GENERATIVE.md` with DIP mapping, control semantics, implemented equations, musical roles and Generative build commands.
- Release-preparation manual freezing: the final `docs/manual/drift-user-manual.odt` is now copied byte-for-byte to `docs/manual/releases/X.Y.Z/drift-user-manual.X.Y.Z.odt` before tagging, and existing snapshots are immutable by default.
- Historical manual backfill tooling can recover an exact pre-freeze source from an existing Git tag without moving or rewriting that tag; this provides a deterministic path for the 0.1.0 manual source.
- Optional compile-time **Organic algorithm bank** for the original Drift hardware, selected at build time while leaving the Classic 0.1.0 bank as the default. The four Organic DIP slots are Fractal, Vector, Rain and Attractor.
- Fractal modulation based on three continuous gradient-noise scales at 1x/4x/16x with constant-gain Texture redistribution.
- Vector modulation based on a deterministic two-axis toroidal phase flow with bounded Texture-controlled cross-coupling.
- Rain modulation using Density-controlled stochastic impulses and a fractional-residual leaky envelope; the hardware Attenuation control naturally remains final output Intensity.
- Attractor modulation using a fixed-point Hénon map with Texture-controlled parameter `a` and interpolated travel between map states.
- Dedicated Organic PlatformIO build, native-test, sanitizer, coverage and timing environments for both Nano bootloader variants where applicable.
- Dedicated mathematical unit suites and developer analyses for all four Organic algorithms plus a bank-level architecture/control-contract document.
- Acceptance criteria AC-33 through AC-37 introduced compile-time bank selection and the four Organic mathematical contracts.
- Tagged-release packaging supports Classic, Organic, Generative and Ambient firmware banks, each built for the new and old Arduino Nano bootloader with unambiguous versioned HEX/ELF filenames, generated firmware-artifact mapping and per-image build provenance.
- The release workflow now enforces an explicit bank-aware artifact contract before checksums/upload: every selected bank must provide non-empty new/old-bootloader HEX and ELF images plus matching build-provenance records, and unexpected firmware-bank artifacts fail the release.
- Expanded user manual covering all three compile-time banks and all twelve algorithms, including control semantics, DIP mappings, mathematical foundations and musical interpretation for the Generative bank.
- Nine deterministic true-vector Organic-bank documentation assets plus a generator: four bank-specific DIP-switch diagrams and five explanatory figures for bank overview, Fractal texture, Vector flow, Rain density and the fixed-point Hénon attractor.
- New vector front/back manual covers that identify Classic, Organic and Generative and the current twelve-algorithm scope.

### Changed

- User manual Algorithm banks introduction now explains the expanded twelve-algorithm potential across three banks, the unavoidable four-selector-state hardware boundary, a compact bank/slot table with DIP symbols, and a concise origin/musical-value table for all twelve algorithms.
- Added a complete Generative bank section to the user manual with bank-specific Turing/Markov/Motif/Urn DIP diagrams, four deterministic explanatory SVGs, mathematical foundations and patch-oriented musical interpretation.
- User manual bank documentation now uses a dedicated Algorithm banks introduction, separate top-level Classic and Organic sections, bank-specific DIP-switch SVGs, and identical heading/subsection styling across all eight algorithms; the former cross-bank configuration comparison is removed.
- Root bank documentation uses explicit `README-BANK-CLASSIC.md`, `README-BANK-ORGANIC.md`, `README-BANK-GENERATIVE.md` and `README-BANK-AMBIENT.md` names, while the main README provides a compact four-bank overview and links to each detailed guide.
- Dependabot CI dependency updates are now consolidated across GitHub Actions and Python into one monthly multi-ecosystem pull request instead of one pull request per ecosystem.
- Tagged-release manual dependency installation now uses only the Ubuntu package source, canonical HTTPS Ubuntu mirrors, bounded APT retry/network timeouts and the minimal `libreoffice-writer` package set.
- The release job now has an explicit 60-minute ceiling and the manual dependency installation step a 10-minute ceiling, preventing a transient package mirror from holding a release indefinitely.
- Release runs for the same tag now share a concurrency group; retriggering a tag cancels an obsolete in-progress run instead of consuming Actions time in parallel.
- `DriftEngine` now owns and dispatches only the compile-time-selected algorithm bank, so inactive-bank algorithm state does not consume SRAM in the resulting firmware image.
- Generic selection, integration, property and system tests are bank-aware and exercise the four slots of the active compile-time bank.
- CI, coverage, sanitizer, AVR resource-budget and timing qualification now cover Classic, Organic, Generative and Ambient builds independently.
- Organic per-sample control mappings avoid general integer division in their hot paths where a bounded power-of-two mapping provides the documented behavior.
- README, testing documentation and algorithm-analysis index now describe the compile-time bank model and the Organic control/DIP mapping.
- README and release-process documentation now explain multi-bank release files, bootloader variants, bank-vs-DIP selection boundary and the versioned release manifest.
- Tagged releases emit separate build-provenance records for every selected bank/bootloader firmware variant instead of describing only one environment per bank.
- Release-note integrity text now derives the published multi-bank firmware set from the tag, alongside the versioned manual, firmware-artifact manifest, provenance files and checksum manifests.
- Tagged releases now publish the frozen versioned manual ODT alongside the PDF, and release/rebuild jobs refuse to use the live development manual for tags created after the freeze mechanism was introduced. Historical pre-freeze tags retain an explicit tag-pinned legacy-source fallback. Generative and Ambient releases are additionally blocked unless the frozen manual source documents the corresponding bank and all four algorithms.

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
