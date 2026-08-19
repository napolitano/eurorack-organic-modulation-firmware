# Requirements traceability

| ID | Acceptance criterion | Primary verification |
|---|---|---|
| AC-01 | The rear configuration inputs preserve the four-way electrical truth table for the compile-time-selected algorithm bank. | `test/integration/test_selection` |
| AC-02 | All four algorithms remain within the 12-bit DAC output domain. | `test/unit/test_algorithms` |
| AC-03 | Fixed seeds produce deterministic algorithm sequences. | `test/unit/test_algorithms` |
| AC-04 | The paired LFSR reproduces the upstream recurrence. | `test/unit/test_rng` |
| AC-05 | Frequency mapping reaches the mathematically defined low-frequency endpoint. | `test/unit/test_frequency` |
| AC-06 | Frequency mapping is monotonic for both Speed knob and CV and its optimized conversion matches the exact rounded rational result. | `test/unit/test_frequency` |
| AC-07 | Brownian Texture uses a continuous normalized smoothing law without the upstream 1020 regime switch. | `test/regression/test_upstream_behaviour`, `test/unit/test_brownian_algorithm` |
| AC-08 | LFO live Texture changes preserve output continuity within the defined fixed-point tolerance. | `test/regression/test_upstream_behaviour`, `test/unit/test_lfo_algorithm` |
| AC-09 | Runtime advances only when a DAC write slot is available. | `test/integration/test_runtime` |
| AC-10 | Runtime updates DAC and LED from the same computed output sample. | `test/integration/test_runtime` |
| AC-11 | Representative full signal paths execute for every algorithm. | `test/system/test_signal_path` |
| AC-12 | A broad control-grid property test enforces the 12-bit output invariant. | `test/property/test_invariants` |
| AC-13 | LFO applies Texture from the first processing step and no longer exhibits the upstream first-cycle peak narrowing zero spike. | `test/regression/test_upstream_behaviour` |
| AC-14 | Perlin fade evaluation matches the canonical quintic and remains monotonic across its complete effective phase domain. | `test/unit/test_perlin_algorithm` |
| AC-15 | Perlin segment evaluation matches the independent one-dimensional gradient-noise reference and has correct lattice boundaries. | `test/unit/test_perlin_algorithm` |
| AC-16 | Brownian random-walk event, direction and step laws match their documented discrete stochastic model. | `test/unit/test_brownian_algorithm` |
| AC-17 | Brownian smoothing retains fractional motion and therefore avoids truncation-only deadband. | `test/unit/test_brownian_algorithm` |
| AC-18 | Both Bézier easing functions match their cubic reference equations and are monotonic. | `test/unit/test_bezier_algorithm` |
| AC-19 | Bézier Texture morph is continuous and the triangular inverse-CDF spans both mathematical endpoints without a final plateau. | `test/unit/test_bezier_algorithm` |
| AC-20 | Bézier phase accumulation preserves wrap overshoot. | `test/unit/test_bezier_algorithm` |
| AC-21 | LFO waveform evaluation matches the mathematical piecewise-linear reference, is monotonic on each branch and provides exact saw endpoints. | `test/unit/test_lfo_algorithm` |
| AC-22 | LFO phase accumulation preserves wrap overshoot. | `test/unit/test_lfo_algorithm` |
| AC-23 | Shared fixed-point interpolation and multiplication primitives match their integer Q-format contracts in both interpolation directions. | `test/unit/test_fixed_math` |
| AC-24 | Perlin gradient selection, lattice handoff continuity and phase-wrap state transitions follow the documented gradient-noise model. | `test/unit/test_perlin_algorithm` |
| AC-25 | Brownian smoothing converges bidirectionally without overshoot, resets fractional state on direction changes and retains the intended long-run centering behavior. | `test/unit/test_brownian_algorithm` |
| AC-26 | Bézier segment-speed variation has the documented center dead zone, symmetric knob response, monotonic CV contribution and bounded full-scale behavior. | `test/unit/test_bezier_algorithm` |
| AC-27 | Bézier endpoint interpolation and multi-segment A-to-B state transitions remain mathematically consistent across repeated phase rollovers. | `test/unit/test_bezier_algorithm` |
| AC-28 | LFO remapping remains output-preserving over a dense phase grid, constant-control phase accumulation survives repeated wraps and live non-saw skew changes stay continuous. | `test/unit/test_lfo_algorithm` |
| AC-29 | The paired LFSR remains deterministic and avoids trivial lock-up for extreme seeds while retaining broad short-window output diversity. | `test/unit/test_rng` |
| AC-30 | Generated exp2 and gamma reference tables match their mathematical generation contracts, endpoints and monotonicity properties. | `test/unit/test_reference_tables` |
| AC-31 | Dynamic end-to-end runtime output matches the direct engine for all algorithms and LED state remains derived from the exact queued DAC sample. | `test/system/test_signal_path` |
| AC-32 | Portable public boundaries fail safely for invalid algorithm selection and clamp out-of-domain ADC values before frequency mapping. | `test/unit/test_algorithms`, `test/unit/test_frequency` |
| AC-33 | Compile-time bank selection exposes exactly four deterministic DIP slots and falls back safely for an invalid slot index. | `test/integration/test_selection` |
| AC-34 | Fractal Texture redistributes a constant total weight across three gradient-noise scales and the algorithm remains deterministic and inside the DAC domain. | `test/unit/test_fractal_algorithm` |
| AC-35 | Vector uses continuous bipolar toroidal projections, bounded cross-coupling and a deterministic 12-bit scalar projection. | `test/unit/test_vector_algorithm` |
| AC-36 | Rain Density and Speed follow documented monotonic event/decay laws, fractional decay reaches zero without a truncation deadband, and fixed seeds are deterministic. | `test/unit/test_rain_algorithm` |
| AC-37 | Attractor implements the documented fixed-point Hénon map over the complete Texture parameter range, remains bounded and emits deterministic 12-bit interpolated output. | `test/unit/test_attractor_algorithm` |

The machine-readable mapping is `test/requirements-traceability.json` and is validated by CI. Requirement IDs must remain unique and consecutive; referenced test cases must exist as `RUN_TEST()` entries.

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
