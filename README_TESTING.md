# Testing and verification

> [!NOTE]
> Native tests execute the portable production code. They do not emulate ATmega328P instruction timing, the analogue front end, MCP4922 electrical timing or LED optics. Those properties require AVR builds and hardware qualification.

## Test model

The repository uses five complementary test categories:

| Category | Purpose |
|---|---|
| Unit | mathematical primitives, each algorithm, RNG and frequency mapping |
| Integration | algorithm selection and runtime/port interaction |
| Property | broad invariants across control/state domains |
| Regression | verified upstream defects and intentional corrections |
| System | virtual-module signal path through the real runtime and engine |

The system rig does not contain a second firmware implementation. It supplies deterministic input frames to production code and captures DAC/LED outputs. Mathematical unit tests may use independent equations to compute expected results; they do not reimplement the production state machine.

## Algorithm-specific mathematical suites

Each Drift mode has a dedicated suite:

| Suite | Primary proof obligations |
|---|---|
| `unit/test_perlin_algorithm` | canonical quintic fade, gradient mapping, monotonicity, symmetry, lattice boundaries, handoff continuity and phase-wrap state transitions |
| `unit/test_brownian_algorithm` | event/step laws, exact centering boundaries, exhaustive random thresholds, saturation, bidirectional smoothing, residual reversal, convergence and long-run centering |
| `unit/test_bezier_algorithm` | exact cubic endpoints, reference equations, monotonicity, Texture morph, speed-variation law, endpoint-complete triangular ICDF, symmetry and multi-segment rollover state |
| `unit/test_lfo_algorithm` | skew-triangle reference equation, exact saw endpoints, monotonic branches, dense phase-remap proof, repeated wraps, live-skew stress and first-step Texture |

The common frequency mapping has its own exact-rational and monotonicity suite under `unit/test_frequency`; shared fixed-point primitives are independently verified under `unit/test_fixed_math`. `unit/test_reference_tables` independently checks the generated exponential and gamma LUTs against their mathematical generation contracts.

## Correctness versus upstream compatibility

Upstream behavior is evidence, not an unquestioned oracle. Regression tests explicitly record places where the unreleased firmware intentionally differs from a verified upstream defect. Tests for corrected behavior are derived from mathematical/reference contracts documented under `docs/analysis/algorithms/`.

Where upstream behavior is intentional and mathematically coherent, deterministic reference vectors remain valuable for detecting accidental drift. Where an upstream defect has been corrected, the corrected mathematical contract takes precedence.

## Strict host diagnostics

First-party native sources are compiled with:

```text
-Wall -Wextra -Werror -Wconversion -Wsign-conversion -Wshadow -Wpedantic
```

Third-party Unity/PlatformIO sources are not promoted to `-Werror`.

## Coverage policy

The current engineering gate is **95% line / 75% branch coverage** for `lib/fmd/src`. The strict-host GCC/gcov baseline measured on 2026-08-18 after the state/edge-case expansion is **99.45% lines / 82.82% branches**; CI still provides the authoritative PlatformIO/gcovr result. Coverage is a regression signal, not the definition of correctness. Mathematically meaningful branch, boundary, property and requirement coverage take precedence over artificially exercising unreachable defensive code. The floors must not be lowered merely to make a change pass. The only uncovered executable lines in the current strict-host baseline are the two defensive Perlin output-clamp assignments: the verified gradient/noise amplitude contract reaches the 12-bit endpoints but does not exceed them, so forcing those branches would require an invalid internal state rather than a meaningful input case.

## Requirements traceability

`test/requirements-traceability.json` maps acceptance criteria to concrete `RUN_TEST()` cases. The current unreleased baseline contains 32 acceptance criteria and 88 native test cases across 14 suites. CI runs `scripts/check_requirement_traceability.py`; missing suites, stale test names, duplicate IDs or gaps in the AC numbering fail early.

See [docs/testing/requirements-traceability.md](docs/testing/requirements-traceability.md).

## PlatformIO test dependency discovery

PlatformIO's Library Dependency Finder starts from the active test translation unit. Test suites that exercise `DriftRuntime` through `test/support/DriftTestRig.h` therefore include `fmd/application/DriftRuntime.h` directly as an explicit production dependency. This mirrors the Quantizer test pattern and prevents the test from depending on transitive discovery through a non-library support include path.

`test/support` remains header-only. Production behavior must continue to come from `lib/fmd`; the test rig is only an I/O harness and must not reimplement algorithm or runtime behavior.

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="docs/assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
