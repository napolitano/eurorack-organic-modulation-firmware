# Perlin noise algorithm analysis

> [!IMPORTANT]
> **Current unreleased implementation decision.** The two-octave Perlin/gradient-noise design and gradient distribution are retained. The fade function is now evaluated from the canonical quintic with a single rounded integer expression over the effective fixed-point phase domain, eliminating local one-code reversals caused by repeated truncation. The shared frequency conversion has also been replaced by an exact-result reciprocal/correction form. Historical compatibility alternatives below are retained as analysis context, not as the current default policy.

> [!NOTE]
> This document is an engineering analysis of Quinn Freedman's upstream Drift firmware, not a criticism of the project and not a substitute for the upstream source. It deliberately separates mathematical properties, directly observed implementation behavior, engineering interpretation, and proposed changes. Source-sensitive conclusions were reviewed against upstream `main` on 2026-08-18 and should be re-verified if upstream changes.

## Purpose and scope

Perlin mode is Drift's default algorithm and the mode that most strongly defines the module's identity. The goal of this document is to establish a technical reference for:

- the mathematical basis of one-dimensional gradient noise;
- the exact form used by the original Drift Rust firmware;
- how `Speed` and `Texture` affect the generated signal;
- fixed-point and phase-accumulator behavior on the ATmega328P;
- computational cost and realistic optimization opportunities;
- compatibility requirements for the C++/PlatformIO implementation;
- test evidence required before an implementation can be called correct.

The analysis intentionally starts from the mathematics and the upstream source. The C++ implementation is not used as proof that the upstream behavior has been understood correctly.

## Upstream references reviewed

Primary implementation references:

- [`perlin.rs`](https://github.com/QuinnFreedman/modular/blob/main/modules/Drift/Firmware/src/perlin.rs)
- [`shared.rs`](https://github.com/QuinnFreedman/modular/blob/main/modules/Drift/Firmware/src/shared.rs)
- [`main.rs`](https://github.com/QuinnFreedman/modular/blob/main/modules/Drift/Firmware/src/main.rs)
- [`fm-lib/src/rng.rs`](https://github.com/QuinnFreedman/modular/blob/main/fm-lib/src/rng.rs)
- [Drift manual](https://quinnfreedman.github.io/fm-artifacts/Drift/drift_manual.pdf)

Background references:

- Ken Perlin, *An Image Synthesizer*, SIGGRAPH 1985, DOI `10.1145/325165.325247`.
- Ken Perlin, *Improving Noise*, SIGGRAPH 2002, DOI `10.1145/566570.566636`.

The Drift implementation is a small one-dimensional embedded adaptation. It should not be assumed to be a literal implementation of every detail from either Perlin paper.

---

## 1. Mathematical foundation

### 1.1 Value noise versus gradient noise

A simple smooth random signal can be built by choosing random values at integer lattice points and interpolating between them. Gradient noise takes a different approach: the random quantity stored at each lattice point is a **gradient** (a slope in one dimension), not the output value itself.

For a segment with local coordinate

$$
x \in [0,1],
$$

let the two endpoint gradients be $g_0$ and $g_1$. The two local linear contributions are

$$
n_0(x) = g_0 x
$$

and

$$
n_1(x) = g_1 (x-1).
$$

A smooth interpolation weight $s(x)$ blends the two:

$$
N(x) = (1-s(x))n_0(x) + s(x)n_1(x).
$$

The important consequence is that neighboring segments naturally meet around a common lattice point without independently chosen output values producing obvious corners.

### 1.2 Quintic fade function

The upstream firmware uses the now-standard quintic fade function

$$
s(x) = 6x^5 - 15x^4 + 10x^3.
$$

It satisfies

$$
s(0)=0, \qquad s(1)=1,
$$

and

$$
s'(0)=s'(1)=0,
$$

as well as

$$
s''(0)=s''(1)=0.
$$

This is important because it suppresses visible or audible derivative discontinuities at lattice boundaries. In a modulation source, those derivative properties correspond to a signal that can change direction organically without introducing a deliberate corner at every random segment boundary.

### 1.3 One-dimensional segment behavior

For the ideal mathematical form,

$$
N(0)=0
$$

because $n_0(0)=0$ and $s(0)=0$. Similarly,

$$
N(1)=0.
$$

Thus every ideal lattice boundary is a zero crossing of the un-offset segment function. The random gradients determine the shape between those boundaries.

Drift later scales and offsets the combined noise into the unipolar DAC range, so the physical output does not repeatedly return to 0 V. The zero-boundary property applies to the internal signed noise component before the final offset.

### 1.4 Multiple octaves

A common way to increase the apparent detail of coherent noise is to combine multiple related frequency bands, often called octaves. In a generic form,

$$
N_{sum}(t) = \sum_k a_k N_k(f_k t).
$$

Drift uses exactly two Perlin states:

- a base octave advanced by $\Delta t$;
- a higher octave advanced by $4\Delta t$.

The second state therefore traverses its lattice four times faster.

This is not a generic fractal-Brownian-motion stack with a configurable number of octaves and a conventional persistence parameter. It is a deliberately small two-band construction suitable for a 16 MHz AVR.

### 1.5 Phase accumulator model

Each octave owns a 32-bit unsigned `time` accumulator. For every 2.5 kHz processing step,

$$
T_{n+1} = (T_n + \Delta T) \bmod 2^{32}.
$$

The upper 16 bits are interpreted as the within-segment coordinate:

$$
x \approx \frac{T \gg 16}{2^{16}}.
$$

An unsigned overflow means that the algorithm has crossed a lattice boundary. At that point:

1. the previous `next_grad` becomes `last_grad`;
2. a new random `next_grad` is generated;
3. interpolation continues from the wrapped phase.

Because the overshoot is retained by unsigned wraparound, this is a conventional phase-accumulator technique and gives better long-term rate behavior than resetting the accumulator to zero after a threshold crossing.

---

## 2. Upstream implementation

### 2.1 State

`PerlinModuleState` contains:

```text
base octave:
    u32 time
    I1F15 last_grad
    I1F15 next_grad

higher octave:
    u32 time
    I1F15 last_grad
    I1F15 next_grad

ParallelLfsr rng
```

Four gradients are consumed during construction so both octaves start with independently assigned gradient pairs from the same deterministic PRNG stream.

### 2.2 Gradient generation

The upstream code computes:

```rust
let h = rng.next() & 15;
let grad_int = 1 + (h & 7);
let grad = I1F15::from_bits(((grad_int as u16) << 11) as i16);
```

The magnitude therefore has eight discrete possibilities. In real-number terms the gradient magnitude is approximately

$$
\frac{1}{16},\frac{2}{16},\ldots,\frac{8}{16},
$$

with the remaining random bit selecting the sign.

This differs from the simplest one-dimensional presentation where gradients might be restricted to $\pm 1$. The variable gradient magnitudes are an intentional part of Drift's output character and must be preserved by the compatibility implementation.

### 2.3 Fixed-point representation

The implementation uses:

- `U0F16` for the local phase and fade value;
- `I1F15` for signed gradients and signed segment values.

The conversion helper

```rust
I1F15::from_bits((x.to_bits() >> 1) as i16)
```

preserves the represented real value when converting from unsigned Q0.16 to signed Q1.15: the raw value is divided by two because the destination has one fewer fractional bit.

### 2.4 Fixed-point fade implementation

The source implements the quintic fade without storing coefficients larger than one in `U0F16`:

```rust
const SIX: U0F16 = U0F16::from_bits(6 << 12);
const FIFTEEN: U0F16 = U0F16::from_bits(15 << 12);
const TEN: U0F16 = U0F16::from_bits(10 << 12);
```

These raw constants represent the desired coefficients divided by 16. The result is consequently computed at 1/16 scale, clipped to 12 useful bits, and shifted left by four to restore the Q0.16 range.

This is a compact embedded trick, not a mistaken interpretation of `U0F16`.

The implementation computes:

```text
t³
t⁴
t⁵
(10/16)t³ + (6/16)t⁵ - (15/16)t⁴
clip to 12 bits
restore scale by << 4
```

The source itself notes that a signed reformulation could reduce multiplication/conversion work, but that attempts had introduced artifacts.

### 2.5 Segment interpolation

The upstream segment function is equivalent to:

```text
u  = fade(x)
a  = lastGradient * x
b  = nextGradient * (x - 1)
y  = lerp(u, a, b)
```

with fixed-point quantization at each operation.

The fixed-point implementation, rather than the infinite-precision equation, is the compatibility reference. A mathematically equivalent floating-point implementation is not sufficient evidence of bit-compatible behavior.

### 2.6 Texture blend

The control value is

$$
b = \min(1023,\; textureKnob + textureCV),
$$

then represented approximately as $b/1024$ in Q1.15.

The output is not a simple crossfade from the base octave to the high octave. The source computes:

$$
y = 3B + B(1-b) + Hb,
$$

where $B$ is the base value and $H$ the high-octave value. Rearranged:

$$
y = (4-b)B + bH.
$$

Therefore:

- at minimum texture, the internal signal is approximately $4B$;
- at maximum texture, it is approximately $3B + H$.

The high-frequency octave can therefore replace approximately one quarter of the base weighting rather than replacing the base signal entirely. This is consistent with the manual's description of adding roughness while retaining the large-scale drift.

### 2.7 Output scaling

The signed result is divided by 16 in raw Q1.15 units and biased by 2048:

```rust
let scaled = (value.to_bits() / 16) + (1 << 11);
scaled.clamp(0, 4095)
```

The result is a 12-bit unipolar code centered around mid-scale before the downstream analog output stage and front-panel attenuator.

---

## 3. Speed mapping and shared phase increment

Perlin calls `get_delta_t(speed_knob, speed_cv, 0)` on every sample.

The shared mapping conceptually does the following:

```text
speed knob -> scale as 0..12 V
speed CV   -> add 0..5 V equivalent
sum        -> clamp to the knob's 12 V maximum
volts      -> 2^volts through a 256-entry exp2 LUT
frequency  -> approximately (2^volts) / 40 Hz
frequency  -> samples per segment at 2.5 kHz
samples    -> 32-bit phase increment
```

The base rate at zero is approximately one segment per 40 seconds. Twelve octaves above that is approximately 102.4 Hz mathematically, while the source comments describe the practical maximum as about 100 Hz.

The second Perlin octave is four times faster, so its highest lattice-crossing rate is approximately 400 Hz.

---

## 4. Findings

### P-01 — Two-octave weighting is a deliberate Drift-specific design

**Classification:** sound-design choice.

The expression

$$
(4-b)B+bH
$$

is not a textbook fBm amplitude stack, but it is internally coherent and matches the manual's concept of preserving the smooth large-scale motion while adding a controlled amount of higher-frequency detail.

**Compatibility decision:** preserve exactly.

No correction is proposed merely to make the algorithm look more conventional.

### P-02 — Fade evaluation is multiplication-heavy

**Classification:** performance concern / upstream-recognized optimization candidate.

The source explicitly notes a possible signed reformulation. At source level, one fade evaluation requires construction of `t³`, `t⁴`, `t⁵` plus three coefficient multiplications. The segment then requires two gradient products and interpolation work.

Because Drift evaluates two complete Perlin segments per DAC sample, this arithmetic dominates the algorithm once the common frequency mapping is excluded.

**Impact:** CPU time, not correctness.

**Compatibility decision:** retain the current arithmetic until a replacement is proven equivalent or a deliberate numerical change is accepted.

### P-03 — The common frequency path contains a software-expensive 64-bit division

**Classification:** performance concern.

`shared.rs::divided_by()` promotes a 32-bit numerator to `u64`, shifts it by 16, and performs a 64-bit division. The upstream source itself comments that a more efficient method probably exists.

At 2.5 kHz this path is evaluated 2,500 times per second in Perlin mode.

**Impact:** potentially material on an ATmega328P, but no exact cycle claim should be made without AVR code generation and measurement.

### P-04 — Frequency is quantized through an integer samples-per-cycle stage

**Classification:** numerical limitation.

The shared mapping computes an integer `samples_per_cycle` before deriving the 32-bit phase increment. This necessarily creates increasingly coarse frequency steps as the requested rate approaches the top of the range.

At low Drift rates the relative error is negligible because a segment contains many samples. At high rates the number of samples per segment is small, so integer changes in that count become measurable.

This is not evidence that the instrument is unusable or that the original design is wrong. It is a predictable consequence of the chosen low-cost mapping.

### P-05 — Exact behavior depends on the authoritative `exp2lut.bin`

**Classification:** compatibility dependency.

A mathematically regenerated exponential table can be extremely close while still producing different phase increments at boundary values. Those differences can eventually alter gradient rollover timing and therefore the PRNG consumption order.

That means a small table mismatch can produce a completely different long-run random sequence even if short-term output looks identical.

**Compatibility decision:** byte-level table parity or authoritative golden vectors are required before claiming bit-exact Perlin compatibility.

### P-06 — The high octave remains below Nyquist but deserves timing/spectral verification

**Classification:** verification item, not a defect.

The high octave can cross lattice segments at roughly 400 Hz with a 2.5 kHz output cadence. The fundamental segment rate is below the 1.25 kHz Nyquist limit, but the nonlinear interpolated waveform contains spectral energy above the fundamental.

The correct engineering question is therefore not simply "400 Hz < Nyquist". The implementation should be inspected for objectionable alias products at the top of the Speed range.

No anti-aliasing change should be introduced without measurement because it would alter the sonic character.

---

## 5. Improvement strategies

### 5.1 Lower-cost fade polynomial

A mathematically equivalent Horner-style form is

$$
s(x)=x^3\left(x(6x-15)+10\right).
$$

A future implementation can investigate a signed/wider intermediate representation that reduces multiplication count while maintaining the required error bound.

Illustrative pseudocode:

```text
function fadeOptimized(x):
    x2 = mul(x, x)
    x3 = mul(x2, x)

    inner = mul(SIX, x) - FIFTEEN
    inner = mul(inner, x) + TEN

    return mul(x3, inner)
```

This pseudocode is **not** a drop-in compatibility implementation. The fixed-point widths, rounding rule, saturation behavior, and coefficient scaling must be chosen explicitly.

Acceptance options should be stated before implementation:

1. **bit-exact:** every Q0.16 input produces the upstream output;
2. **DAC-equivalent:** differences are permitted internally but never change the final 12-bit DAC code;
3. **numerically improved:** bounded error is accepted as an intentional behavioral change.

### 5.2 Remove the integer samples-per-cycle intermediate

A later frequency-mapping revision can derive the phase increment directly from the desired frequency:

$$
\Delta T = \frac{2^{32} f}{f_s}.
$$

With

$$
f = \frac{2^V}{40}
$$

and

$$
f_s = 2500\text{ Hz},
$$

this becomes

$$
\Delta T = \frac{2^{32} 2^V}{100000}.
$$

A practical implementation can use a dedicated phase-increment LUT, reciprocal multiplication, or another fixed-point formulation chosen from measured AVR cost.

Pseudocode contract:

```text
speedControl = clamp(scaleKnobTo12V(knob) + cv + offset)
phaseIncrement = phaseIncrementFromControl(speedControl)
```

The important point is that `phaseIncrementFromControl()` should return the final increment directly rather than first quantizing the requested period to an integer sample count.

### 5.3 Optional fade LUT

A small flash-resident fade table plus interpolation may outperform repeated fixed-point multiplication. This should be evaluated rather than assumed.

Trade-offs:

- lower arithmetic cost;
- additional flash consumption;
- additional PROGMEM reads;
- possible interpolation error;
- another compatibility table that must be frozen and tested.

---

## 6. Computational cost

The following is a **source-level operation inventory**, not an AVR cycle count. Exact cost depends on compiler lowering and `fixed` crate implementation details.

### 6.1 Per sample

Approximate hot-path work:

| Component | Work |
|---|---|
| Speed mapping | 2 LUT reads, LUT interpolation, 64-bit division, integer scaling/division |
| Base octave | 32-bit phase add, one segment evaluation |
| High octave | 32-bit phase add, one segment evaluation |
| Each fade | about 7 fixed-point multiplications in the current expression |
| Each segment | 2 gradient multiplications plus fixed-point lerp |
| Texture mix | 2 signed fixed-point multiplications plus integer accumulation |
| Output | shift/divide, bias, clamp |

A rough source-level count is therefore on the order of **twenty-plus fixed-point multiplications per sample**, plus the common 64-bit division. That is sufficient to identify the optimization targets, but it is not a substitute for compiled timing measurements.

### 6.2 Rollover path

When an octave wraps, the additional work is small:

- move `next_grad` to `last_grad`;
- call the parallel LFSR;
- map four random bits to a signed gradient.

The high octave rolls four times as often as the base octave, so worst-case timing qualification must include samples where one or both octave rollovers occur.

### 6.3 Required hardware timing evidence

The 2.5 kHz cadence provides a nominal 400 µs period. The correct qualification is:

1. instrument the beginning/end of the algorithm hot path with a GPIO timing pin;
2. run representative and worst-case control combinations;
3. include simultaneous octave rollover cases;
4. record maximum execution time, not only average time;
5. verify that the complete runtime, including ISR and DAC scheduling overhead, does not miss the 400 µs service deadline.

No cycle-budget number should be promoted to a requirement until it has been measured on the target AVR build.

---

## 7. Verification and test strategy

### 7.1 Mathematical unit tests

Required pure-function tests:

| Area | Evidence |
|---|---|
| Fade endpoints | `fade(0) == 0`; upper endpoint approaches full scale as defined by the fixed format |
| Fade midpoint | verify the expected 0.5 result within the upstream fixed-point rounding contract |
| Fade monotonicity | exhaustive Q0.16 sweep must never decrease |
| Fade symmetry | verify `fade(1-x) ≈ 1-fade(x)` under explicit fixed-point tolerance |
| Segment boundary | segment output is zero at the exact left lattice boundary |
| Segment amplitude | exhaustive phase × legal gradient pairs stays within the proven internal bound |

The amplitude test is particularly valuable because it justifies the absence or presence of internal saturation rather than relying on a comment.

### 7.2 RNG and gradient tests

For fixed seeds:

- compare the first N `ParallelLfsr` values against upstream-known answers;
- compare the first N generated gradients;
- verify the eight magnitude classes and both signs are reachable;
- verify the initialization consumes exactly four gradients in the original order.

### 7.3 Octave state-machine tests

Required cases:

- no rollover;
- base rollover only;
- high-octave rollover only;
- both roll over on the same sample;
- phase overshoot is retained after unsigned wrap;
- exactly one new gradient is consumed per rollover.

These tests matter because changing PRNG consumption order changes all future output.

### 7.4 Texture tests

For a fixed seed and fixed phase history:

- texture 0 must reproduce the base-dominated formula;
- texture 1023 must reproduce the maximum high-octave contribution;
- increasing texture must change only the mix coefficient, not reset algorithm state;
- knob + CV summation must clamp at 1023.

### 7.5 Golden-vector compatibility tests

The strongest compatibility proof is an upstream Rust fixture containing:

```text
seed
N input frames
N expected 12-bit output codes
```

Vectors should include:

- static controls;
- changing Speed CV;
- changing Texture CV;
- octave rollover boundaries;
- minimum and maximum speed;
- minimum and maximum texture;
- at least one long vector sufficient to expose PRNG-order divergence.

A short vector can prove local arithmetic. A long vector is required to prove state-machine and random-stream compatibility.

### 7.6 Property tests

Useful invariants:

```text
0 <= output <= 4095
same seed + same inputs => identical sequence
control summation never exceeds 1023
gradient magnitude is one of the 8 legal values
phase rollover never consumes more than one gradient per octave per sample
```

### 7.7 Spectral/timing characterization

For the highest Speed settings:

- capture long DAC sequences;
- compute spectra on the host;
- compare upstream and C++ output;
- inspect energy near/above Nyquist and any fold-back components;
- separately measure execution time on the physical ATmega328P.

Spectral results are characterization data, not pass/fail defects unless a requirement is explicitly defined.

---

## 8. Engineering assessment

### Preserve

- one-dimensional gradient-noise concept;
- discrete random gradient magnitudes and signs;
- two independent octave states using one deterministic PRNG stream;
- 4× higher-octave phase rate;
- Drift-specific `(4-b)B + bH` texture weighting;
- continuous wrapped phase accumulator semantics;
- knob + CV control summation and 12-bit unipolar output behavior.

### Correct only after evidence

No confirmed algorithmic correctness defect has been identified in the core Perlin interpolation itself at this stage. Any numerical rewrite must first prove equivalence against the upstream fixed-point behavior or be explicitly classified as an intentional change.

### Optimize

Highest-value candidates:

1. common frequency/phase-increment mapping;
2. fade polynomial evaluation;
3. possible fade LUT if flash/time measurements justify it.

### Verify on hardware

- worst-case execution time;
- missed 2.5 kHz deadlines;
- high-speed octave spectral behavior;
- byte/golden parity with the authoritative exponential LUT.

### Current conclusion

The upstream Perlin implementation is compact and conceptually strong. Its main engineering risk is not an obvious mathematical error but the amount of fixed-point work performed around every sample and the sensitivity of long-run deterministic behavior to tiny differences in phase mapping or lookup-table contents. The compatibility implementation should therefore prioritize **reference vectors and state-transition proof** before attempting arithmetic cleanup.

## Implemented state and edge-case verification

The unreleased test suite now additionally exhausts the complete 16-value signed gradient mapping, checks value/slope continuity across a lattice handoff using a shared gradient, and verifies phase-wrap overshoot explicitly. These tests complement the canonical fade and segment-equation checks by covering the state transition that selects a new gradient at rollover.

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../../assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
