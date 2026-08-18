# Changelog

This changelog records release-relevant changes to the firmware and repository. New work remains under `Unreleased` until a release is explicitly prepared. A versioned release section is created only as part of release preparation and must begin with a `### Release summary` prose paragraph; the release workflow uses that paragraph as the opening of the generated GitHub Release notes.

## Unreleased

### Added

- Shared Drift orange-red heart asset plus automated footer validation for user-facing Markdown documentation.
- Reusable, raster-free SVG redraws of the manual's front-panel reference, algorithm diagrams and configuration-switch illustrations under `docs/manual/assets/`, with automated vector-asset validation and selected figures reused in repository documentation.
- Maintained end-user manual source under `docs/manual/`, with a separate CC BY-NC 4.0 licence and publication notes for the required Ubuntu/Ubuntu Light typography.
- Reproducible manual publication tooling and CI that converts the unversioned ODT source into a versioned release PDF and rejects tagged-release artifacts with missing/substituted Ubuntu fonts.
- C++17/PlatformIO firmware architecture with a portable `lib/fmd` core and ATmega328P ports-and-adapters hardware boundary.
- Perlin, Brownian, Bézier and LFO implementations with deterministic paired-LFSR random generation.
- Dedicated mathematical unit-test suites for each of the four algorithms, plus shared fixed-point/frequency/reference-table suites, covering reference equations, boundaries, monotonicity, continuity, distribution behavior, long-run behavior and state-transition properties.
- Native unit, integration, property, regression and system tests with machine-checked requirement traceability; the current unreleased baseline contains 88 test cases and 32 acceptance criteria.
- Native coverage, sanitizer, Nano old/new bootloader, AVR resource-budget and timing-probe CI targets.
- Detailed developer analyses for all four algorithms, including mathematical foundations, upstream implementation review, computational cost, optimization opportunities and verification strategy.
- GitHub issue forms, pull-request template, security policy, contributor guidance, dependency automation and tag-driven release workflow.
- Deterministic changelog-based GitHub Release-note generation with checksum manifests and build provenance metadata.

### Changed

- Repository identity prepared for `napolitano/eurorack-organic-modulation-firmware`, including GitHub status badges and stricter ATmega328P engineering guardrails of 85% application flash and 65% static SRAM.
- Root README reorganized around upstream acknowledgement, a user-oriented explanation of Drift, project rationale, quick-start guide, repository index, neutral upstream-findings table and release-history placeholder for the currently unreleased project.
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

- Native runtime/system suites now include their production dependency explicitly, matching the Quantizer test pattern and ensuring PlatformIO LDF links `lib/fmd` even when `DriftRuntime` is otherwise reached only through `DriftTestRig.h`.
- Native PlatformIO tests now link nested portable sources reliably.
- Shared native reference tables are header-only so every PlatformIO test suite receives the same definitions without relying on out-of-suite support translation units.
- Corrected local fixed-point polynomial evaluations that could introduce one-code reversals in otherwise monotone Perlin/Bézier reference functions.

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="docs/assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
