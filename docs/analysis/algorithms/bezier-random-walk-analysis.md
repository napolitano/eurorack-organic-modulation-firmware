# Bézier random-segment algorithm analysis

> [!IMPORTANT]
> **Current unreleased implementation decision.** The triangular random offset in log-speed space is retained as the implemented musical model. The ICDF now uses 257 boundary samples for 256 intervals, Texture continuously morphs between the two cubic curve families, cubic evaluation uses monotone single-rounding integer forms, and phase wrap preserves overshoot. Historical compatibility alternatives below remain as rationale, not as the current default behavior.

> [!NOTE]
> This document is an engineering reading of Quinn Freedman's upstream Drift firmware. It separates the cubic easing mathematics, the random endpoint process, the random timing process, and the fixed-point implementation. The upstream mode is called "Bézier" because Bézier-derived easing curves are used between random values; the entire stochastic process is not itself a Bézier curve in the usual geometric-modeling sense. Source-sensitive conclusions were reviewed against upstream `main` on 2026-08-18.

## Purpose and scope

Bézier mode generates a succession of random 12-bit values and interpolates between them with one of two cubic easing functions. Texture also randomizes segment timing.

This analysis covers:

- cubic Bézier/Bernstein foundations;
- the two scalar easing curves used by Drift;
- endpoint and derivative behavior;
- random endpoint generation;
- texture-controlled timing variation;
- the actual inverse-CDF distribution used upstream;
- discrepancies between manual wording and implementation;
- numerical and control-continuity issues;
- CPU cost and test strategy.

## Upstream references reviewed

- [`bezier.rs`](https://github.com/QuinnFreedman/modular/blob/main/modules/Drift/Firmware/src/bezier.rs)
- [`random.rs`](https://github.com/QuinnFreedman/modular/blob/main/modules/Drift/Firmware/src/random.rs)
- [`shared.rs`](https://github.com/QuinnFreedman/modular/blob/main/modules/Drift/Firmware/src/shared.rs)
- [`fm-lib/src/rng.rs`](https://github.com/QuinnFreedman/modular/blob/main/fm-lib/src/rng.rs)
- upstream ICDF generator: [`tools/lut_generator/src/main.rs`](https://github.com/QuinnFreedman/modular/blob/main/modules/Drift/tools/lut_generator/src/main.rs)
- [Drift manual](https://quinnfreedman.github.io/fm-artifacts/Drift/drift_manual.pdf)

Mathematical background:

- cubic Bézier curves expressed in the Bernstein polynomial basis;
- standard cubic smoothstep $3t^2-2t^3$.

---

## 1. Mathematical foundation

### 1.1 Cubic Bézier curve

A cubic Bézier curve with control points $P_0,P_1,P_2,P_3$ is

$$
B(t)=(1-t)^3P_0+3(1-t)^2tP_1+3(1-t)t^2P_2+t^3P_3,
$$

for

$$
t\in[0,1].
$$

Drift does not evaluate a two-dimensional geometric curve. It uses a cubic polynomial as a **scalar interpolation fraction** between two random endpoint values.

If endpoint values are $a$ and $b$, and $E(t)$ is the easing function, output is

$$
y(t)=a+(b-a)E(t).
$$

### 1.2 Smooth easing used by Drift

The upstream `smooth_bezier_curve()` is

$$
E_s(t)=3t^2-2t^3.
$$

This is the cubic smoothstep function. It can be derived from a cubic Bézier scalar curve whose effective inner control values are 0 and 1.

Properties:

$$
E_s(0)=0,\qquad E_s(1)=1,
$$

$$
E_s'(t)=6t(1-t),
$$

so

$$
E_s'(0)=E_s'(1)=0.
$$

Successive segments therefore join with zero slope at each random endpoint, provided the mode remains in the smooth family.

The function also has the useful symmetry

$$
E_s(1-t)=1-E_s(t).
$$

### 1.3 "Reverse" / unsmoothed easing used by Drift

The upstream `unsmooth_bezier_curve()` is

$$
E_u(t)=2t^3-3t^2+2t.
$$

Its derivative is

$$
E_u'(t)=6t^2-6t+2.
$$

This derivative is positive throughout $[0,1]$, with endpoint slopes

$$
E_u'(0)=E_u'(1)=2
$$

and minimum slope 0.5 at the midpoint.

So the function is monotonic and does not overshoot its endpoints. The "spiky" character comes from entering/leaving each random endpoint with a nonzero slope. Adjacent segments normally have unrelated endpoint differences, so their derivatives do not match at the shared random point and a cusp-like accent can occur.

It also satisfies

$$
E_u(1-t)=1-E_u(t).
$$

### 1.4 Difference between the two curve families

The difference is

$$
D(t)=E_u(t)-E_s(t)
     =4t^3-6t^2+2t.
$$

The maximum absolute difference occurs at

$$
t=\frac{3\pm\sqrt{3}}{6}
$$

and has magnitude approximately

$$
|D|_{max}\approx0.19245.
$$

Therefore switching curve family instantaneously during a segment can move the interpolation fraction by almost 19.25% of the current endpoint span.

For a theoretical full-scale 12-bit endpoint difference, that corresponds to roughly 788 DAC codes. The actual jump depends on the random endpoints and current phase.

This becomes relevant because the original firmware chooses the curve family from the **current** Texture knob position every sample.

---

## 2. Random endpoints

At construction:

```text
value_a = 0
value_b = rng.next() >> 4
```

At every segment rollover:

```text
value_a = value_b
value_b = rng.next() >> 4
```

Assuming the PRNG is acceptably uniform, `rng.next() >> 4` gives a nearly uniform 12-bit endpoint in 0..4095.

The exact random stream is part of the compatibility contract. In particular, the timing-randomization path also consumes the same PRNG, so changing the ICDF implementation or call order changes future endpoint values as well as segment durations.

---

## 3. Timing model

### 3.1 Base segment speed

The algorithm calls the shared

```text
get_delta_t(speedKnob, speedCv, speed_adjust)
```

path. Base Speed therefore follows the same approximately 1 V/oct exponential mapping as Perlin and LFO.

### 3.2 Segment-local random speed adjustment

At rollover, a new `speed_adjust` is generated from Texture.

The Texture knob is measured relative to center with a dead zone:

```text
HALF      = 511
DEAD_ZONE = 128
RANGE     = 383
```

Ignoring the one-code asymmetry before clamping, the magnitude is approximately:

$$
m = \max(0, |textureKnob-511|-128).
$$

Texture CV contributes half its raw code:

$$
m' = \min(383, m + textureCV/2).
$$

Thus:

- a centered knob and zero Texture CV give fixed segment timing;
- moving the knob away from center widens the random timing variation;
- positive Texture CV widens timing variation regardless of which side of center the knob is on.

The final random adjustment is a signed Q1.15 random value multiplied by the normalized magnitude.

### 3.3 The random variable is applied in volts-per-octave space

This is a critical semantic detail.

The random value is **not** added directly to the segment duration. It is passed as the signed `offset` argument to `get_delta_t()`, where it is scaled into the same pitch-like control domain used by Speed before the exponential mapping.

Conceptually:

$$
V'=V+kR
$$

then

$$
f \propto 2^{V'}
$$

and segment duration is approximately

$$
T \propto 2^{-V'}.
$$

Therefore even if $R$ were Gaussian, the distribution of **time intervals** would not itself be Gaussian. A Gaussian random variable in log-frequency space produces a log-normal-like multiplicative timing distribution, subject to clamping and discrete sampling.

This distinction is important when interpreting the manual statement that the time between points is sampled from an increasingly wider Gaussian distribution.

---

## 4. Upstream inverse-CDF distribution

### 4.1 Code-level distribution is triangular, not Gaussian

`random.rs` explicitly states that the current inverse-CDF table is triangular and Gaussian is only a possible future option.

The generator implements the inverse CDF of a symmetric triangular distribution on approximately [-1,1]:

For $p<1/2$,

$$
F^{-1}(p)=-1+\sqrt{2p}
$$

and for $p\ge1/2$,

$$
F^{-1}(p)=1-\sqrt{2(1-p)}.
$$

The ideal symmetric triangular distribution has mean 0 and variance

$$
\frac{1}{6}.
$$

### 4.2 Manual discrepancy

The manual says the intervals are sampled from an increasingly wider **Gaussian** distribution. The implementation uses a triangular ICDF.

Moreover, as described above, the random value is applied before an exponential frequency mapping, so the final **interval** distribution is not triangular either.

The most precise description of current behavior is therefore:

> Texture controls the magnitude of a triangular-distributed random offset in the logarithmic/V-oct speed-control domain; that offset produces multiplicative variation in segment duration.

That is less concise than the manual, but it matches the code.

### 4.3 256-entry ICDF endpoint problem

The upstream generator creates exactly 256 entries:

```text
for i in 0..255:
    u = i << 7
    lut[i] = triangle_icdf(u)
```

Runtime indexing is:

```text
idx_low  = u >> 7          // 0..255
idx_high = min(255, idx_low + 1)
```

The 15-bit uniform input has 256 interpolation **intervals** of 128 codes each. Linear interpolation across 256 intervals requires 257 boundary values if the final interval is to interpolate toward the positive endpoint.

With only 256 entries, the last interval has:

```text
idx_low  = 255
idx_high = 255
```

so the final 128 input codes all collapse to the last LUT value instead of continuing toward +1.

The final table point corresponds to

$$
p=\frac{32640}{32767}\approx0.9961
$$

and the ideal triangular inverse CDF there is only about

$$
+0.912.
$$

Consequences:

- the negative endpoint reaches approximately -1;
- the positive tail never reaches the corresponding +1 region;
- the last 1/256 of the uniform input range forms a plateau at the final positive LUT value;
- the sampled speed-adjust distribution is slightly asymmetric.

The overall mean bias is small, but the missing positive tail is a real table-boundary defect and is easy to test exactly.

---

## 5. Upstream curve selection

The actual output branch is:

```rust
if cv[3] < 1024 / 2 {
    reverse_bezier_interpolate(...)
} else {
    bezier_interpolate(...)
}
```

Only the **Texture knob** chooses the curve family. Texture CV does not.

This agrees with the manual's explicit statement that the knob, not CV, determines smooth versus inverse easing.

However, the branch is evaluated every sample. If the knob crosses code 512 mid-segment, the algorithm switches immediately between $E_u(t)$ and $E_s(t)$. Since those functions generally differ at the same phase, the output can jump.

The comments in `bezier.rs` show that the author explicitly considered continuous curve-parameter control and rejected a straightforward closed-form approach because changing the parameter during a segment caused discontinuities. The binary family switch reduces the dimensionality of the problem, but it does not mathematically eliminate the discontinuity when crossing the threshold.

---

## 6. Segment phase behavior

`step_time()` uses saturating addition:

```text
time = saturating_add(time, dt)
rollover = (time == UINT32_MAX)
if rollover:
    time = 0
```

Unlike Perlin's wrapping phase accumulator, any overshoot beyond the segment endpoint is discarded.

At a constant `dt`, the resulting segment duration is quantized to an integer number of 2.5 kHz samples. With a varying Speed CV, the discarded overshoot can also slightly perturb long-term phase relative to a true wrapping accumulator.

The maximum endpoint-time error is less than one processing interval, so this is best classified as a discrete-time/phase-accuracy limitation rather than a severe defect.

---

## 7. Findings

### Z-01 — Manual says Gaussian; implementation uses triangular ICDF

**Classification:** verified documentation discrepancy.

**Compatibility decision:** triangular behavior is the code-level source of truth for the compatibility profile.

Gaussian behavior, if ever added, must be an explicit new option or documented behavior change.

### Z-02 — "Distribution of time intervals" is not the distribution actually sampled

**Classification:** semantic/documentation discrepancy.

The firmware samples a random offset in V/oct speed space and then exponentiates it. The resulting segment durations are multiplicatively distributed and additionally clipped near control-range limits.

Even replacing the triangular ICDF with a Gaussian one would not make raw interval duration Gaussian.

**Design decision required:** decide whether the desired musical invariant is:

- symmetry in octaves/log-frequency, which the current architecture approximates; or
- a specific probability distribution of time intervals.

These are different requirements.

### Z-03 — ICDF LUT is missing the final interpolation endpoint

**Classification:** verified table-boundary defect.

A 256-interval mapping is implemented with only 256 boundary samples. The final 128 random input codes therefore flatten to the final LUT entry around +0.912 rather than approaching +1.

**Current implementation:** corrected. The endpoint-complete 257-sample table is the production contract; the 256-sample plateau remains covered as an upstream regression finding.

### Z-04 — Curve-family crossing can create an instantaneous output jump

**Classification:** control-continuity limitation / implementation trade-off.

Switching between smooth and reverse easing at the same segment phase can change interpolation fraction by as much as approximately 0.19245 of the endpoint span.

The full-scale theoretical jump is roughly 788 DAC codes, though typical random endpoint spans are smaller.

This behavior is not mentioned in the manual. The upstream comments show awareness of the broader discontinuity problem when changing Bézier parameters during a segment.

### Z-05 — Saturating segment phase discards overshoot

**Classification:** numerical timing limitation.

Resetting to zero at the segment endpoint loses residual phase. A wrapping accumulator would preserve long-term average phase more accurately.

The unreleased implementation intentionally makes this correction and verifies overshoot-preserving wrap explicitly; reference-vector differences at affected segment boundaries are therefore expected.

### Z-06 — Random timing is clipped near global Speed limits

**Classification:** expected boundary behavior.

`get_delta_t()` clamps the combined speed-control value to its legal range. Therefore negative random offsets near minimum Speed and positive offsets near maximum Speed are clipped asymmetrically.

The timing distribution necessarily becomes narrower and skewed near the global range boundaries.

This is mathematically expected and should be documented rather than treated as a firmware fault.

---

## 8. Improvement strategies

### 8.1 Correct the triangular ICDF table geometry

The simplest exact interval geometry uses 257 table entries for 256 interpolation intervals:

```text
for i in 0..256:
    p = i / 256
    table[i] = triangular_icdf(p)

low      = u >> 7          // 0..255
fraction = (u & 0x7F) / 128
high     = low + 1         // 1..256
return lerp(table[low], table[high], fraction)
```

This preserves cheap bit-based indexing and adds only one extra `i16` entry: two bytes of flash.

The original 256-entry behavior remains useful only as historical/reference evidence. The unreleased production implementation uses the corrected 257-entry geometry and protects it with explicit distribution and endpoint tests.

### 8.2 Decide the intended timing distribution before changing it

Two coherent designs are possible.

#### Preserve musical symmetry in octaves

Keep the existing architecture:

```text
random = sampleSymmetricDistribution()
logSpeedOffset = width * random
frequency = baseFrequency * 2^(logSpeedOffset)
```

This creates multiplicative time variation and is musically natural for a V/oct control.

#### Specify a duration distribution directly

If the requirement is genuinely "Gaussian-distributed interval duration":

```text
baseDuration = durationFromSpeed(speed)
random = sampleGaussian()
duration = clamp(baseDuration + sigma * random, minimumDuration, maximumDuration)
phaseIncrement = phaseIncrementForDuration(duration)
```

This has very different behavior and should be considered a redesigned mode, not a correction.

### 8.3 Eliminate the curve-family discontinuity

There is no free solution that simultaneously guarantees all of the following during an arbitrary mid-segment shape change:

- unchanged global phase;
- unchanged instantaneous output;
- unchanged remaining segment duration;
- immediate full application of the new curve.

A design must choose its continuity contract.

#### Option A — latch curve family per segment

```text
on segment rollover:
    activeCurve = textureKnob < center ? REVERSE : SMOOTH
```

Pros:

- no mid-segment curve jump;
- trivial and cheap;
- deterministic.

Cons:

- up to one segment of control latency.

#### Option B — crossfade between curve evaluations

Use a short control-rate transition:

```text
oldY = evaluate(oldCurve, phase)
newY = evaluate(newCurve, phase)
mix  = slew(shapeMixTarget)
y     = lerp(oldY, newY, mix)
```

Pros:

- responsive;
- bounded output transition.

Cons:

- requires evaluating both curves during transition;
- no longer exactly a single Bézier easing function;
- adds a new time constant and CPU cost.

#### Option C — incremental/state-space curve

Derive output from an integrated slope state and change the slope law continuously. This can provide stronger continuity guarantees but is more complex and requires careful fixed-point error control. The upstream source already notes precision concerns around this idea.

For Drift, Option A or B is easier to verify rigorously.

### 8.4 Preserve phase overshoot

A corrected segment phase can use overflow rather than saturation:

```text
newTime = time + dt
if newTime overflowed:
    rollover()
time = newTime
```

If multiple rollovers cannot occur for any legal `dt`, one overflow flag is sufficient. This should be proven from the maximum phase increment as part of the test suite.

---

## 9. Computational cost

### 9.1 Normal sample

Source-level hot path:

| Component | Work |
|---|---|
| Shared Speed mapping | exp LUT interpolation + 64-bit division |
| Phase | 32-bit saturating add and rollover check |
| Curve | two fixed-point multiplications for `t²`, `t³`, then arithmetic |
| Endpoint interpolation | one fixed-point lerp |
| Output | 12-bit conversion / endpoint guard |

This is substantially cheaper than Perlin's two full gradient-noise evaluations, but the common 64-bit division remains a major candidate cost.

### 9.2 Rollover sample

Additional work:

- one PRNG sample for the next endpoint;
- one PRNG sample for timing-distribution input;
- two ICDF table reads and interpolation;
- one Q1.15 multiplication for timing width;
- state updates.

Therefore timing qualification must include rollover samples, not only steady interpolation samples.

### 9.3 Optimization priorities

1. optimize the shared frequency mapping first;
2. keep the cubic curve arithmetic unless measurement shows it matters;
3. correct the ICDF table geometry before attempting ICDF micro-optimization;
4. if a curve crossfade is introduced, measure the dual-evaluation path as the new worst case.

---

## 10. Verification and test strategy

### 10.1 Cubic easing tests

For the smooth curve:

```text
E(0) = 0
E(0.5) = 0.5
E(1) = 1
monotonic over the full fixed-point domain
zero endpoint slope within the discrete representation
symmetry: E(1-x) = 1-E(x) within rounding tolerance
```

For the reverse curve:

```text
E(0) = 0
E(0.5) = 0.5
E(1) = 1
monotonic over the full fixed-point domain
positive endpoint slope
same symmetry property
```

An exhaustive 12-bit phase sweep is inexpensive on the host and preferable to a handful of spot checks.

### 10.2 Curve-switch discontinuity test

For representative endpoint spans, evaluate both curve families at every 12-bit phase and find the maximum output difference.

The mathematical reference predicts a maximum interpolation-fraction difference of approximately 0.19245.

This test serves two purposes:

- documents the upstream behavior quantitatively;
- provides a baseline for any future continuity improvement.

### 10.3 ICDF table tests

Compatibility-table tests must verify:

- exact 256 upstream entries once the authoritative blob is imported;
- monotonicity;
- negative endpoint;
- final positive value;
- constant final interpolation interval caused by `idx_high == idx_low == 255`.

Corrected-table tests must verify:

- 257 entries or equivalent endpoint-complete mapping;
- monotonicity;
- approximate antisymmetry;
- no plateau over the final 1/256 of the input domain;
- output approaches both tails according to the chosen discrete contract.

### 10.4 Distribution tests

For long deterministic samples:

- mean near the expected discrete-table mean;
- variance near the expected discrete triangular variance;
- histogram shape;
- positive/negative tail occupancy;
- no unexpected spikes except the known compatibility plateau.

Do not use a generic chi-square threshold without documenting binning and expected discrete probabilities. The table is quantized and interpolated, so the reference distribution is the **implemented discrete distribution**, not an ideal continuous PDF.

### 10.5 Timing-distribution tests

Separate three domains:

1. raw ICDF random variable;
2. `speed_adjust` in control/V-oct space;
3. resulting segment duration in samples.

For each Texture magnitude, record all three. This prevents a misleading test from claiming "triangular interval distribution" when only the upstream random source is triangular.

### 10.6 Segment continuity tests

With a fixed curve family:

- final sample of one segment and rollover endpoint must meet according to the fixed-point contract;
- next segment starts from the same endpoint;
- no DAC code leaves 0..4095;
- endpoint PRNG consumption occurs exactly once per rollover.

### 10.7 Golden vectors

Reference fixtures should cover:

- centered Texture / fixed interval;
- maximum left Texture;
- maximum right Texture;
- nonzero Texture CV at centered knob;
- knob crossing 511/512 during a segment;
- minimum and maximum Speed;
- random-speed clipping near both global Speed boundaries;
- multiple rollovers to expose PRNG-order drift.

---

## 11. Engineering assessment

### Preserve

- random 12-bit endpoints;
- cubic smooth and reverse easing families;
- knob-only selection of curve family;
- Texture magnitude/dead-zone concept for timing variation;
- shared deterministic PRNG order in compatibility mode;
- logarithmic/V-oct timing-offset behavior unless deliberately redesigning the mode.

### Correct

Strong candidates:

1. endpoint-complete triangular ICDF lookup;
2. documentation describing the actual distribution domain;
3. optional curve-switch continuity improvement after a clear response/latency contract is selected;
4. phase-overshoot preservation if timing measurements and compatibility policy support the change.

### Do not silently "fix"

Replacing triangular randomness with Gaussian randomness is not automatically the correct solution. The current code and the manual disagree, and the timing transform means neither label alone describes the final interval distribution. The desired probability model must be specified first.

### Performance

The common `get_delta_t()` division is likely a higher-value optimization target than the cubic polynomial. Rollover samples are the relevant worst-case path because they include endpoint RNG and ICDF work.

### Current conclusion

Bézier mode is mathematically more interesting than its small source file suggests. The cubic easing functions themselves are simple and well behaved. The important findings are in the **control and probability layers**: the manual's Gaussian-interval description does not match the triangular log-speed implementation; the 256-entry ICDF construction misses the final positive interpolation endpoint; and switching curve family mid-segment can create a sizeable output discontinuity. Those behaviors can all be characterized precisely and therefore lend themselves well to compatibility regression tests followed by explicit, testable corrections.

## Implemented state and edge-case verification

The unreleased test suite now verifies the segment-speed variation law directly: center dead zone, knob symmetry, monotonic Texture-CV contribution and saturation. It also checks inverse-CDF antisymmetry within the finite-LUT quantisation bound, dense-grid interpolation boundedness, exact endpoints, and a multi-segment reference state machine across repeated phase rollovers with speed variation disabled.

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../../assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
