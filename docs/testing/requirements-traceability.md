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
| AC-38 | The Generative compile-time bank maps the four rear DIP slots to Turing, Markov, Motif and Urn while preserving the shared electrical truth table. | `test/integration/test_selection` |
| AC-39 | Turing maps Texture exactly from zero mutation to one-half mutation probability, rotates locked state exactly and remains deterministic in the 12-bit DAC domain. | `test/unit/test_turing_algorithm` |
| AC-40 | Markov implements the exact 4/8, 2/8, 1/8, 1/8 structured transition kernel, exact Texture endpoint behavior and a fixed stratified seed-defined 12-bit vocabulary. | `test/unit/test_markov_algorithm` |
| AC-41 | Motif applies at most one project-defined structural edit at a complete phrase boundary, including correct circular transformations, and remains deterministic and bounded. | `test/unit/test_motif_algorithm` |
| AC-42 | Urn uses bounded 31/32 relaxation, monotonic Texture-controlled reinforcement, correct weighted categorical selection at equal weights and the exact eight-level DAC vocabulary. | `test/unit/test_urn_algorithm` |
| AC-43 | The Ambient compile-time bank maps the four rear DIP slots to Current, Anchor, Breath and Fog while preserving the shared electrical truth table. | `test/integration/test_selection` |
| AC-44 | Current uses three fixed non-harmonic rate approximations, exact constant-sum Texture weights and deterministic bounded 12-bit projection. | `test/unit/test_current_algorithm` |
| AC-45 | Anchor implements explicit bounded mean reversion, zero-spread centre stability and seed-deterministic OU-inspired triangular-innovation motion. | `test/unit/test_anchor_algorithm` |
| AC-46 | Breath preserves a baseline-to-single-peak-to-baseline cycle, keeps all rollover-latched parameters inside documented bounds and remains deterministic for a fixed seed. | `test/unit/test_breath_algorithm` |
| AC-47 | Fog uses the documented compact smooth kernel, a fixed four-voice process, duration-compensated occupancy mapping and deterministic bounded bipolar output. | `test/unit/test_fog_algorithm` |
| AC-48 | The Electronica compile-time bank maps the four rear DIP slots to Pump, Acid, Shuffle and Polymeter while preserving the shared electrical truth table. | `test/integration/test_selection` |
| AC-49 | Pump uses the documented 30..240 BPM Electronica tempo mapping, exact 1/4..15/16 recovery range and monotonic deterministic duck/recovery contour. | `test/unit/test_pump_algorithm` |
| AC-50 | Acid implements the exact project-defined 16-step permutation, accent/slide masks, Texture-zero dry sequence and bounded smooth accent/slide contours without RNG state. | `test/unit/test_acid_algorithm` |
| AC-51 | Shuffle maps Texture monotonically from straight timing to an exact 3:1 long/short pair while preserving total pair duration and deterministic bounded output. | `test/unit/test_shuffle_algorithm` |
| AC-52 | Polymeter selects exact secondary meters 3/5/7/9, preserves the documented accent amplitudes and exact 12/20/28/36-step composite recurrences. | `test/unit/test_polymeter_algorithm` |
| AC-53 | The Percussion compile-time bank maps the four rear DIP slots to Euclid, Repeat, Probability and Humanize while preserving the shared electrical truth table. | `test/integration/test_selection` |
| AC-54 | Euclid maps Texture to E(k,16) hit counts 2..13, uses canonical masks with exact population and applies deterministic phrase-end tail fills without removing base hits. | `test/unit/test_euclid_algorithm` |
| AC-55 | Repeat preserves guaranteed quarter-note anchors, maps Texture to the documented ratchet probability/count and applies exact phrase-fill minimum repeat counts. | `test/unit/test_repeat_algorithm` |
| AC-56 | Probability preserves primary quarter-note hits, uses exact secondary and quadratic ghost thresholds, and applies saturating phrase/final-quarter probability boosts. | `test/unit/test_probability_algorithm` |
| AC-57 | Humanize preserves the fixed eighth-note source while bounding independent grid-relative timing jitter to ±30 samples and pulse amplitude to the documented Texture-controlled range. | `test/unit/test_humanize_algorithm` |

The machine-readable mapping is `test/requirements-traceability.json` and is validated by CI. Requirement IDs must remain unique and consecutive; referenced test cases must exist as `RUN_TEST()` entries.

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
