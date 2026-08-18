# Requirements traceability

| ID | Acceptance criterion | Primary verification |
|---|---|---|
| AC-01 | Original four-way configuration mapping is preserved. | `test/integration/test_selection` |
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
| AC-25 | Brownian smoothing converges bidirectionally without overshoot, handles residual direction changes and retains long-run centering. | `test/unit/test_brownian_algorithm` |
| AC-26 | Bézier segment-speed variation implements the documented dead zone, symmetry, CV contribution and saturation behavior. | `test/unit/test_bezier_algorithm` |
| AC-27 | Bézier endpoint interpolation and repeated A-to-B segment transitions remain mathematically consistent across rollovers. | `test/unit/test_bezier_algorithm` |
| AC-28 | LFO dense phase remapping, repeated wraps and live non-saw Texture changes retain the defined continuity/state contracts. | `test/unit/test_lfo_algorithm` |
| AC-29 | Extreme LFSR seeds avoid trivial lock-up and long runs remain deterministic with broad short-window diversity. | `test/unit/test_rng` |
| AC-30 | Generated exponential and gamma lookup tables match their mathematical generation contracts. | `test/unit/test_reference_tables` |
| AC-31 | Dynamic end-to-end runtime behavior matches the direct engine and LED/DAC sample coherence is retained. | `test/system/test_signal_path` |

| AC-32 | Portable public boundaries return a safe output for an invalid algorithm enum and clamp out-of-domain ADC values before frequency mapping. | `test/unit/test_algorithms`, `test/unit/test_frequency` |

The machine-readable mapping is `test/requirements-traceability.json` and is validated by CI. Requirement IDs must remain unique and consecutive; referenced test cases must exist as `RUN_TEST()` entries.

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
