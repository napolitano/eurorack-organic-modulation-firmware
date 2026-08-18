# Notes on the original Drift Rust firmware

> [!NOTE]
> This is an engineering reading of Quinn Freedman's upstream firmware and shared `fm-lib`, not a criticism of the project. Implementation-sensitive conclusions must be verified against the cited upstream revision.

## Files reviewed

- `modules/Drift/Firmware/src/main.rs`
- `drift.rs`
- `perlin.rs`
- `brownian.rs`
- `bezier.rs`
- `lfo.rs`
- `random.rs`
- `shared.rs`
- `fm-lib/src/rng.rs`

Detailed per-algorithm analyses:

- [Perlin noise](algorithms/perlin-noise-analysis.md)
- [Brownian / random walk](algorithms/brownian-motion-analysis.md)
- [Bézier random segments](algorithms/bezier-random-walk-analysis.md)
- [LFO](algorithms/lfo-analysis.md)

## What is good about it

The original firmware is unusually deliberate for a small ATmega328P module. It uses fixed-point arithmetic, a fixed 2.5 kHz DAC cadence, asynchronous ADC sampling, deterministic PRNG state after boot seeding, and four algorithms that are separated into small state objects. Perlin uses two octaves and a texture blend; the second octave advances four times faster. Bézier and LFO use saturating time counters so segment/cycle completion has explicit semantics.

## Behaviour to characterize before changing

### Brownian texture mapping

For texture values below 1020, the raw 10-bit value is used as the raw Q0.16 interpolation parameter. This makes the interpolation parameter much smaller than a normalized 0..1 mapping would be. At 1020..1023 the implementation instead copies the target value directly.

### LFO skew update

The apex/skew parameter is updated only on rollover. The source itself calls this a stopgap and explains the continuity/period problems encountered with continuous updates.

### Bézier random distribution

`random.rs` explicitly says the current inverse-CDF LUT is triangular and could become Gaussian in the future. This should be treated as the code-level source of truth when resolving conflicting prose documentation.


### Bézier ICDF endpoint geometry

The published LUT generator creates 256 triangular-ICDF samples at `u = i << 7`, while runtime lookup divides the 15-bit input domain into 256 intervals. The final interval clamps both indices to entry 255, leaving a plateau over the last 128 input codes and preventing the positive tail from approaching +1. The detailed Bézier analysis derives the boundary condition and a 257-entry correction strategy.

### Bézier curve-family switching

The smooth/reverse curve family is selected from the live Texture knob on every sample. Crossing code 511/512 during a segment can therefore change the interpolation fraction discontinuously. This is separate from the source comment about continuous Bézier-parameter modulation and should be treated as a control-continuity limitation rather than silently changed.

### LFO first-cycle apex

The LFO constructor uses `UINT32_MAX / 2` and does not apply Texture until the first rollover. The first cycle is therefore approximately triangular regardless of Texture. The unaligned constructor apex also exposes a rising-branch fixed-point narrowing edge that can produce a one-sample zero at the first-cycle peak for reachable low-rate trajectories.

### Brownian integer-smoothing deadband

The Brownian smoother stores only a 16-bit current value and discards fractional movement. With very small smoothing coefficients, `alpha * delta` can truncate to zero while current and target are still materially different. This is independent of the compressed Texture mapping and requires a fractional-state or residual strategy if corrected.

### Frequency calculation cost

`shared.rs` uses a 64-bit division in `divided_by()` and includes a note that a more efficient implementation may exist. This is an optimization candidate, not evidence of incorrect behaviour.

## Compatibility implementation rule

The C++ implementation preserves intentional musical behaviour but does not retain a verified numerical defect merely for bit compatibility. Every correction requires: a precise upstream characterization, a mathematical or engineering requirement for the corrected behavior, regression/reference coverage, and a README/CHANGELOG entry.

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
