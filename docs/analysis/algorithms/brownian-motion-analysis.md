# Brownian / random-walk algorithm analysis

> [!IMPORTANT]
> **Current unreleased implementation decision.** The bounded random-walk core, Speed-dependent event probability/step size and edge-centering bias are retained. The raw-ADC-as-Q0.16 Texture defect and the 1020 snap are corrected with a normalized smoothing-coefficient map, and fractional smoothing residual is retained so convergence cannot stall solely through truncation. Historical compatibility alternatives below describe the investigated upstream baseline, not the current default behavior.

> [!NOTE]
> This document is an engineering analysis of Quinn Freedman's upstream Drift firmware, not a criticism of the project. The word "Brownian" is retained because it is the user-facing mode name. Mathematically, the implemented process is better described as a bounded discrete random walk with edge bias followed by a first-order smoothing filter. Source-sensitive conclusions were reviewed against upstream `main` on 2026-08-18.

## Purpose and scope

Brownian mode is structurally very different from Drift's other three algorithms. It does **not** use the common volts-per-octave phase accumulator. Instead, `Speed` directly controls the probability and size of random target movements, while `Texture` controls a proportional smoother that follows that target.

This document establishes:

- the mathematical distinction between ideal Brownian motion and Drift's discrete process;
- the exact random-walk probabilities and step sizes in the Rust firmware;
- the edge-centering mechanism;
- the first-order smoothing equation and its fixed-point realization;
- confirmed numerical problems in the texture path;
- computational cost and optimization options;
- statistical and deterministic tests needed to verify the C++ implementation.

## Upstream references reviewed

- [`brownian.rs`](https://github.com/QuinnFreedman/modular/blob/main/modules/Drift/Firmware/src/brownian.rs)
- [`main.rs`](https://github.com/QuinnFreedman/modular/blob/main/modules/Drift/Firmware/src/main.rs)
- [`fm-lib/src/rng.rs`](https://github.com/QuinnFreedman/modular/blob/main/fm-lib/src/rng.rs)
- [Drift manual](https://quinnfreedman.github.io/fm-artifacts/Drift/drift_manual.pdf)
- upstream helper used to illustrate the concept: [`make_brownian_graph.py`](https://github.com/QuinnFreedman/modular/blob/main/modules/Drift/tools/make_brownian_graph.py)

---

## 1. Mathematical foundation

### 1.1 Brownian motion in the strict mathematical sense

A standard Wiener/Brownian process \(W(t)\) is a continuous-time stochastic process with continuous paths and independent, stationary Gaussian increments. For \(t>s\),

\[
W(t)-W(s) \sim \mathcal{N}(0,t-s).
\]

A symmetric discrete random walk can converge to Brownian motion under an appropriate scaling limit, but a finite embedded random walk with bounds, fixed step sizes, and filtering is not itself a Wiener process.

This distinction matters for engineering analysis, but not as a criticism of the module name. "Brownian" and "random walk" are common musical descriptions for correlated random modulation.

### 1.2 Drift's target process

Let \(X_n\) be the 16-bit internal `target_value` at sample \(n\). The target either stays in place or attempts a fixed-sized move.

For the combined Speed control

\[
c = \min(1023, speedKnob + speedCV),
\]

the upstream code defines

\[
step(c) = \left\lfloor\frac{256+c}{2}\right\rfloor
\]

and

\[
cutoff(c)=64c.
\]

With a nominally uniform 16-bit PRNG value \(R\), movement occurs when

\[
R < 64c.
\]

Ignoring the tiny PRNG-distribution details, the move probability is therefore approximately

\[
P(move) = \frac{64c}{65536} = \frac{c}{1024}.
\]

So:

- `c = 0` gives zero move probability;
- `c = 512` gives approximately 50% move probability;
- `c = 1023` gives approximately 99.9% move probability.

### 1.3 Speed changes both probability and step magnitude

The manual describes Speed primarily as controlling how likely the particle is to move. The implementation also changes the movement magnitude:

\[
step(0)=128
\]

and

\[
step(1023)=639.
\]

Therefore increasing Speed increases both:

1. how often movement is attempted;
2. how far each successful movement travels.

For an idealized unbiased walk, variance growth per sample is proportional to

\[
P(move)\cdot step(c)^2.
\]

Because both factors increase with `c`, the effective diffusion strength grows much faster than it would if Speed changed only movement probability.

This is important musical behavior and should be treated as part of the compatibility contract unless there is an explicit design decision to decouple the two effects.

### 1.4 Edge-centering bias

The target is bounded to the unsigned 16-bit range using saturating addition/subtraction. To reduce the tendency to remain at an edge, the firmware introduces a small directional bias in the outer fifths of the range.

Away from the edges, the split is 50/50.

Near the low edge, the threshold is

\[
cutoff_2 = cutoff\left(\frac{1}{2}-\frac{1}{64}\right)=cutoff\frac{31}{64}.
\]

Conditional on a move occurring, the probability of moving upward is approximately

\[
\frac{33}{64}=51.5625\%,
\]

and downward approximately

\[
\frac{31}{64}=48.4375\%.
\]

Near the high edge the bias is reversed.

This is a weak mean-reverting mechanism only near the boundaries, not a general Ornstein-Uhlenbeck-style restoring force toward center.

### 1.5 First-order smoothing

Let \(Y_n\) be `current_value` and \(X_n\) the target. A conventional first-order proportional smoother is

\[
Y_{n+1}=Y_n+\alpha(X_n-Y_n),
\]

with

\[
0\le\alpha\le1.
\]

Smaller \(\alpha\) means more smoothing; larger \(\alpha\) follows the target more rapidly.

In the upstream implementation the direction is handled separately and the magnitude is computed from the absolute difference, but the real-number model is equivalent to the expression above.

---

## 2. Upstream implementation

### 2.1 State

The mode owns only:

```text
u16 target_value
u16 current_value
ParallelLfsr rng
```

It is therefore the smallest and computationally cheapest of the four Drift algorithms.

Both `target_value` and `current_value` start at zero. Brownian mode therefore starts from the bottom of its internal range rather than from a random value or mid-scale.

### 2.2 Target update

Every 2.5 kHz processing call:

1. the Speed knob and Speed CV are summed and clamped to 1023;
2. one PRNG value is generated;
3. that same random value decides whether a move happens and, if it does, which direction;
4. the target moves by the Speed-dependent step size with saturation at 0/65535.

Using the same uniform value for event and direction selection is not inherently incorrect. Conditional on `R < cutoff`, `R` remains distributed over the selected interval, so the threshold split provides the intended directional proportions.

### 2.3 Texture smoother

For texture below 1020, upstream constructs:

```rust
let cv_fixed = FixedU16::<U16>::from_bits(cv);
```

`FixedU16<U16>` is Q0.16. Therefore the raw 10-bit ADC code is interpreted directly as a Q0.16 number:

\[
t = \frac{cv}{65536}.
\]

The intended interpolation endpoints are approximately:

\[
\alpha_{min}=\frac{15}{65536}\approx0.0002289
\]

and

\[
\alpha_{max}=\frac{8191}{65536}\approx0.12498.
\]

The code then computes

\[
\alpha=cvFixed\cdot(\alpha_{max}-\alpha_{min})+\alpha_{min}.
\]

Because `cvFixed` never approaches 1, almost none of the nominal interpolation interval is used.

For example, before fixed-point rounding:

| Texture code | Approx. upstream alpha | Approx. 1/alpha samples | Approx. time at 2.5 kHz |
|---:|---:|---:|---:|
| 0 | 0.0002289 | 4369 | 1.75 s |
| 512 | 0.00119 | 840 | 0.34 s |
| 1019 | 0.00217 | 462 | 0.185 s |

Then the code has a separate branch:

```rust
if cv >= 1020 {
    self.current_value = self.target_value;
}
```

So the response jumps from a roughly 185 ms first-order time scale immediately to an exact target copy at the final four ADC codes.

The table above uses the common \(1/\alpha\) approximation as an intuitive time-scale indicator. It is not an exact discrete-time 1/e time constant.

### 2.4 Integer smoothing and lost fractional movement

The implementation computes the movement in the same 16-bit fixed domain:

```text
delta = |target - current|
move  = alpha * delta
current +=/-= move
```

The product truncates to an integer raw step. If

\[
\alpha\cdot |X-Y| < 1\text{ raw LSB},
\]

then `move == 0` and the smoother stops converging.

At minimum texture, `alpha` has raw value 15. Therefore any internal difference smaller than approximately

\[
\frac{65536}{15}\approx4369
\]

can produce zero movement.

Because the DAC output is `current_value >> 4`, 4369 internal codes correspond to roughly 273 12-bit DAC codes. The exact stalled error depends on the discrete trajectory and rounding, but the existence of a substantial fixed-point deadband is a direct consequence of the arithmetic.

At texture 1019 the raw alpha is roughly 142, reducing the zero-movement threshold to roughly 462 internal codes, still around 29 DAC codes.

At 1020 the separate snap-to-target branch bypasses the problem entirely.

---

## 3. Findings

### B-01 — The mode is a bounded discrete random walk, not mathematical Brownian motion

**Classification:** terminology/model clarification.

The implemented process has:

- discrete 2.5 kHz time;
- Bernoulli move events;
- fixed step magnitude for a given Speed setting;
- hard saturation at boundaries;
- weak edge bias;
- a separate low-pass follower.

It does not have Gaussian increments or unbounded continuous paths.

**Compatibility decision:** no change. The user-facing name remains Brownian mode.

The technical documentation should use "bounded random walk" when mathematical precision matters.

### B-02 — Speed also changes step size

**Classification:** documentation discrepancy / sound-design choice.

The manual says Speed controls how likely the particle is to move. The code additionally changes step size from 128 to 639 internal counts.

This causes effective diffusion strength to grow significantly faster with Speed than probability alone would imply.

**Compatibility decision:** preserve.

Any future decoupling would be an intentional redesign of the mode, not a bug fix.

### B-03 — Texture interpolation uses the raw 10-bit code as Q0.16

**Classification:** verified numerical defect unless contrary design evidence appears.

The interpolation factor should normally span approximately 0 to 1 across the control range. The source instead spans only 0 to approximately 0.01555 before a separate direct-copy branch at 1020.

This explains the compressed smoothing-control response and abrupt final transition.

**Current implementation:** corrected. Texture is normalized over the full ADC control range and the historical 1020 regime switch is retained only as a regression reference.

### B-04 — The 16-bit smoother can stall before reaching its target

**Classification:** verified fixed-point limitation.

The smoother stores no fractional residual. When the proportional movement rounds to zero, convergence stops.

The effect is particularly large at low Texture because the minimum alpha is extremely small.

This behavior is independent of B-03. Even after normalizing the Texture control correctly, a very small minimum alpha can still create a deadband unless fractional state is retained.

### B-05 — The 1020 threshold creates a discontinuous control law

**Classification:** verified discontinuity.

Texture 1019 uses the slow fixed-point follower. Texture 1020 copies the target immediately. These are qualitatively different systems separated by one ADC code.

ADC noise around the threshold can therefore alternate between smoothing and hard tracking unless the analog input is sufficiently stable.

### B-06 — Initial state is pinned to the lower boundary

**Classification:** startup behavior / design choice.

Both target and current start at zero. The module must random-walk away from 0 after startup, and attempted negative steps initially saturate.

This is deterministic and easy to reproduce. It may be musically acceptable, but it differs from initializing around mid-scale or from a random initial target.

No compatibility change is proposed.

---

## 4. Improvement strategies

### 4.1 Normalize Texture before interpolation

The low-risk mathematical correction is to map 0..1023 across the full interpolation interval.

Conceptually:

```text
texture = clamp(textureKnob + textureCv, 0, 1023)
t = texture / 1023
alpha = lerp(alphaMin, alphaMax, t)
```

Integer-friendly pseudocode:

```text
alphaRaw = MIN_ALPHA_RAW
         + round(texture * (MAX_ALPHA_RAW - MIN_ALPHA_RAW) / 1023)
```

This removes the compressed range and makes the dedicated `>= 1020` snap branch unnecessary unless an explicit "fully raw" endpoint is desired.

If a raw endpoint is desirable, it should be a documented endpoint rule such as:

```text
if texture == 1023:
    current = target
else:
    use normalized smoother
```

rather than consuming four ADC codes with a hidden discontinuity.

### 4.2 Preserve fractional smoothing state

A more robust smoother should retain sub-LSB motion. Two practical options are:

#### Option A — wider fixed-point current state

Store `current` in a 32-bit fixed-point format:

```text
currentQ16_16

delta = targetQ16_16 - currentQ16_16
currentQ16_16 += alpha * delta
output = currentQ16_16 >> fractionalBitsFor12BitOutput
```

This is conceptually clean and removes the large 16-bit deadband.

#### Option B — residual/error accumulator

Keep the 16-bit state but retain discarded fractional movement:

```text
product = alphaRaw * abs(target - current) + residual
move = product >> 16
residual = product & 0xFFFF
apply move toward target
```

This can be cheaper in state size but needs careful treatment when the error changes sign.

Option A is easier to reason about and test; Option B may be cheaper depending on AVR code generation.

### 4.3 Preserve or explicitly redesign Speed coupling

If the original sonic behavior is desired:

```text
moveProbability = speed / 1024
stepMagnitude   = (256 + speed) / 2
```

should remain unchanged.

If a later mode wants a more textbook random-walk control, probability and step magnitude can be separated. That must be a new behavior/profile because it materially changes the spectrum and long-term variance.

### 4.4 Optional startup policy

Possible later startup choices:

- preserve zero for exact compatibility;
- initialize target/current to mid-scale;
- initialize both from one PRNG sample.

The last two eliminate the startup attraction to one boundary but alter deterministic startup output. They should not be introduced silently.

---

## 5. Computational cost

Brownian mode is significantly cheaper than Perlin, Bézier, or LFO because it never calls `get_delta_t()` and performs no software 64-bit division in the algorithm path.

### 5.1 Per sample

Source-level hot path:

| Component | Work |
|---|---|
| RNG | one `ParallelLfsr::next()` |
| Move decision | compare against cutoff |
| Target update | occasional saturating add/subtract |
| Edge bias | a few integer divisions by constants / shifts depending compiler lowering |
| Texture alpha | one fixed-point lerp multiplication |
| Smoothing | absolute difference plus one fixed-point multiplication |
| Output | right shift by 4 |

This makes Brownian a useful baseline when measuring overall runtime overhead: if Brownian misses deadlines, the problem is likely in shared runtime/hardware handling rather than algorithm arithmetic.

### 5.2 Optimization priorities

There is little value in micro-optimizing the target walk before fixing the numerical semantics. The main engineering improvement is **precision**, not raw speed.

If profiling later shows a need:

- precompute simple control-law terms only if ADC/control update semantics remain unchanged;
- use shifts for powers-of-two constants where the compiler does not already do so;
- avoid adding expensive generic abstractions around the PRNG or smoother.

---

## 6. Verification and test strategy

Brownian correctness requires both deterministic tests and statistical characterization.

### 6.1 Deterministic unit tests

Required exact tests:

| Area | Test |
|---|---|
| Speed zero | target never moves when combined Speed is 0 |
| Step size | verify 128 at 0 and 639 at 1023, plus representative interior values |
| Cutoff | verify `64 * speed` over full input range without overflow |
| Saturation | target never wraps below 0 or above 65535 |
| Edge bias thresholds | exact `31/64`, `32/64`, `33/64` cutoff behavior |
| Output range | output always 0..4095 |
| Determinism | fixed seed + fixed controls gives exact repeatable sequence |

### 6.2 Regression tests for upstream texture behavior

The compatibility suite should pin at least:

```text
texture = 0
texture = 1
texture = 512
texture = 1019
texture = 1020
texture = 1023
```

Required evidence:

- 1019 follows the proportional smoother;
- 1020 copies target directly;
- 1020 and 1019 can produce observably different output with the same initial state/seed;
- corrected mode, when introduced, does not accidentally inherit the discontinuity.

### 6.3 Deadband/convergence test

A pure smoother test seam should allow explicit `current`, `target`, and alpha values.

Upstream-characterization case:

```text
set current and target to a difference below the minimum-alpha movement threshold
step repeatedly
verify current stops changing while target is still different
```

Corrected implementation requirement:

```text
for every target != current:
    repeated smoothing eventually reaches the target to within the declared output tolerance
```

If the design intentionally allows asymptotic convergence without exact equality, the tolerance must be stated in DAC LSBs.

### 6.4 Statistical move-probability tests

Using long deterministic PRNG runs and/or many seeds, estimate movement frequency for representative `speed` values.

Expected relationship:

\[
P(move)\approx speed/1024.
\]

The test should use statistical tolerance rather than exact counts unless the entire finite PRNG period is deliberately enumerated.

Suggested points:

```text
64, 256, 512, 768, 1023
```

### 6.5 Edge-bias characterization

Initialize the target in:

- low fifth;
- middle region;
- high fifth.

Measure successful upward/downward moves over long deterministic runs. The direction ratios should converge near:

```text
low edge:   33/64 up, 31/64 down
middle:     32/64 up, 32/64 down
high edge:  31/64 up, 33/64 down
```

This verifies the intended anti-sticking bias independently from saturation effects.

### 6.6 Long-run signal statistics

For compatibility and future changes, capture:

- mean output;
- variance;
- histogram;
- first several autocorrelation lags;
- average absolute first difference;
- edge occupancy.

These are not substitutes for exact tests. They detect accidental changes to the **character** of a random process that exact short vectors may miss.

### 6.7 Corrected-texture response tests

A corrected implementation should prove:

- alpha is monotonic in Texture;
- alpha spans the declared minimum/maximum range;
- no hidden discontinuity occurs between adjacent ADC codes;
- lower Texture always produces equal or stronger smoothing under a fixed target trajectory;
- full Texture behavior matches the documented endpoint contract.

---

## 7. Engineering assessment

### Preserve

- bounded random-walk concept;
- deterministic `ParallelLfsr` sequence for compatibility;
- Speed-dependent move probability;
- original Speed-dependent step size unless intentionally redesigning the mode;
- weak edge-centering bias;
- 16-bit target range and 12-bit output mapping.

### Correct

Implemented corrections in the current unreleased firmware:

1. normalize Texture across the intended interpolation range;
2. remove the large integer-smoothing deadband by retaining fractional state;
3. replace the opaque 1020..1023 snap region with an explicit documented endpoint policy if hard tracking is still desired.

### Document explicitly

- the mode is a bounded discrete random walk, not a Wiener process;
- Speed changes both move probability and move magnitude;
- startup begins at the lower boundary in compatibility mode.

### Performance

No major optimization work is justified before the numerical corrections. Brownian mode is already cheap enough that correctness and statistical character dominate the engineering priorities.

### Current conclusion

Brownian contains the clearest confirmed numerical problem in the original Drift algorithms: the 10-bit Texture value is interpreted directly as Q0.16, compressing the smoother-control range into roughly the first 1.6% of the intended interpolation domain before an abrupt snap-to-target threshold. A second, independent issue is the loss of fractional smoothing movement, which can leave the follower materially short of its target. Both can be fixed cleanly while preserving the random-walk core, but compatibility tests must retain the original behavior as a reference profile.

## Implemented state and edge-case verification

The unreleased test suite now checks the exact boundaries of the centering zones, both smoothing directions, fractional-residual reset after a direction reversal, convergence without overshoot, residual clearing at the target and a deterministic long-run centering run. This extends the exhaustive event/direction probability proof from single steps to stateful behavior.

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../../assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
