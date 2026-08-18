# LFO algorithm analysis

> [!IMPORTANT]
> **Current unreleased implementation decision.** Texture is applied from the first processing step; endpoint Texture values implement exact rising/falling saws; live skew changes use output-preserving phase remapping; and phase wrap preserves overshoot. The upstream cycle-latched and first-cycle behaviors remain documented below as the reference problem statement, not as the current default behavior.

> [!NOTE]
> This document is an engineering analysis of Quinn Freedman's upstream Drift LFO implementation. The upstream source itself describes the shape-update policy as a stopgap. The analysis therefore separates the upstream reference behavior from the mathematically corrected behavior selected for the current unreleased C++ implementation. Source-sensitive conclusions were reviewed against upstream `main` on 2026-08-18.

## Purpose and scope

LFO mode is nominally the simplest Drift algorithm: a unipolar skewable triangle oscillator. In implementation terms it exposes several useful embedded-design questions:

- phase accumulation and cycle accuracy;
- piecewise-linear waveform mathematics;
- shape/skew modulation without discontinuities;
- endpoint representation in fixed-point arithmetic;
- division cost on an 8-bit AVR;
- startup behavior and cycle-boundary state updates.

This analysis derives the ideal piecewise waveform first, then maps the upstream fixed-point implementation onto it.

## Upstream references reviewed

- [`lfo.rs`](https://github.com/QuinnFreedman/modular/blob/main/modules/Drift/Firmware/src/lfo.rs)
- [`shared.rs`](https://github.com/QuinnFreedman/modular/blob/main/modules/Drift/Firmware/src/shared.rs)
- [`main.rs`](https://github.com/QuinnFreedman/modular/blob/main/modules/Drift/Firmware/src/main.rs)
- [Drift manual](https://quinnfreedman.github.io/fm-artifacts/Drift/drift_manual.pdf)

---

## 1. Mathematical foundation

### 1.1 Normalized phase

Let oscillator phase be

\[
p\in[0,1).
\]

Let the apex/skew parameter be

\[
a\in[0,1].
\]

`a` is the phase at which the output reaches its maximum.

### 1.2 Piecewise skewed triangle

For

\[
0<a<1,
\]

a unit-amplitude skewed triangle can be written:

\[
y(p,a)=
\begin{cases}
\frac{p}{a}, & p<a\\
1-\frac{p-a}{1-a}, & p\ge a.
\end{cases}
\]

Equivalent falling form:

\[
y(p,a)=\frac{1-p}{1-a},\qquad p\ge a.
\]

Special cases:

- \(a=0\): reverse saw/ramp down;
- \(a=0.5\): symmetric triangle;
- \(a\to1\): rising saw/ramp with a short reset/fall segment.

The slope is

\[
\frac{dy}{dp}=\frac{1}{a}
\]

on the rising side and

\[
\frac{dy}{dp}=-\frac{1}{1-a}
\]

on the falling side.

This explains why extreme skew values require very large slopes in the short side of the waveform.

### 1.3 Frequency and phase increment

For sample rate \(f_s=2500\) Hz and desired frequency \(f\), an ideal 32-bit wrapping phase accumulator uses

\[
\Delta P = round\left(2^{32}\frac{f}{f_s}\right).
\]

Each sample:

\[
P_{n+1}=P_n+\Delta P\pmod{2^{32}}.
\]

The upstream code obtains an increment through the shared volts-per-octave mapping, but its cycle handling is not a pure wrapping accumulator; that difference is analyzed below.

---

## 2. Upstream implementation

### 2.1 State

The entire LFO state is:

```text
u32 time
u32 apex
```

Constructor values:

```text
time = 0
apex = UINT32_MAX / 2
```

The initial apex is therefore `0x7FFFFFFF`, one raw phase unit below an exact `0x80000000` half-cycle.

### 2.2 Speed mapping

Every sample calls:

```text
get_delta_t(speedKnob, speedCv, 0)
```

so LFO Speed follows the common 1 V/oct-like exponential mapping from roughly 1/40 Hz at the bottom to approximately 100 Hz at the top.

### 2.3 Saturating cycle accumulator

The code performs:

```text
time = saturating_add(time, dt)
rollover = (time == UINT32_MAX)
before_rollover = time
if rollover:
    time = 0
```

Thus the sample that completes a cycle is evaluated with `t == UINT32_MAX`, then internal time becomes zero for the following call.

Any phase overshoot beyond the endpoint is discarded.

### 2.4 Texture-to-apex mapping

Texture is summed and clamped:

\[
c=\min(1023, textureKnob+textureCV).
\]

On a cycle rollover only:

\[
apex=c\cdot2^{22}.
\]

So:

```text
c = 0      -> apex = 0x00000000
c = 512    -> apex = 0x80000000
c = 1023   -> apex = 0xFFC00000
```

The maximum is 1023/1024 of a full cycle, not the exact cycle endpoint.

### 2.5 Piecewise output computation

The source discards the lower 16 phase bits and creates Q16.16 fixed-point values:

```text
completed = t >> 16
out_of    = apex >> 16
ratio     = completed / out_of
```

on the rising side, with the analogous remaining-phase expression on the falling side.

The resulting 16-bit normalized amplitude is shifted right by four for the 12-bit DAC output.

### 2.6 Shape update policy

The source comment is explicit: apex changes only between cycles because directly changing the closed-form skew parameter during a cycle caused output discontinuities, while an attempted numerical implementation altered cycle timing at high frequency because of integer precision.

This is therefore a known implementation trade-off rather than an accidental omission.

---

## 3. Findings

### L-01 — Texture changes can be delayed by almost one full cycle

**Classification:** upstream-recognized interaction limitation.

Apex is updated only on rollover. If the user changes Texture immediately after a rollover, the old shape remains active until the next cycle.

At the minimum LFO rate, one cycle is roughly 40 seconds. The control-feedback delay can therefore also approach 40 seconds.

**Current implementation:** corrected. Texture updates are immediate; phase is remapped to preserve output continuity within the fixed-point error bound.

### L-02 — The first cycle always uses the constructor triangle shape

**Classification:** startup behavior / probable usability defect.

Before the first rollover, Texture has never been applied. The constructor fixes

```text
apex = UINT32_MAX / 2
```

so the first cycle is approximately a symmetric triangle regardless of the physical Texture setting or Texture CV.

At slow Speed this mismatch can persist for tens of seconds after power-up.

A corrected implementation can initialize the active apex from the first control frame without changing the later cycle-boundary policy.

### L-03 — The initial apex can produce a rising-branch 16-bit wrap at the peak

**Classification:** verified fixed-point startup edge case.

The rising branch does:

```rust
(completed / out_of).to_bits() as u16
```

without clamping the result to `u16::MAX` first.

For the constructor apex `0x7FFFFFFF`:

```text
out_of = apex >> 16 = 0x7FFF
```

There exists a phase interval

```text
0x7FFF0000 <= t < 0x7FFFFFFF
```

where

```text
completed = t >> 16 = 0x7FFF = out_of
```

while the code still selects the rising branch because `t < apex`.

The fixed-point division therefore represents exactly 1.0, whose Q16.16 raw value is 65536 (`0x00010000`). Casting that raw value directly to `u16` yields zero.

So, if the discrete phase trajectory lands in that interval during the **first** cycle, the rising waveform can wrap from near full-scale to zero before reaching the apex.

This issue is primarily associated with the constructor apex. After rollover, all apex values are generated by `texture << 22` and are therefore aligned to a much coarser power-of-two grid. For those aligned apexes, `t < apex` prevents the high-16-bit numerator from equaling the high-16-bit denominator in the same way.

The falling branch already clamps its division result before narrowing, which avoids the corresponding wrap there.

### L-04 — Full-right Texture is not mathematically an exact rising saw

**Classification:** endpoint limitation / design trade-off.

Maximum Texture gives

\[
a=1023/1024\approx0.999023.
\]

The falling segment therefore occupies 1/1024 of the period rather than zero.

At fast LFO rates this interval can be shorter than one sample and behaves effectively like an instantaneous reset. At the 40-second minimum rate it is about 39 ms long and can span many 2.5 kHz samples.

Whether this is audible/relevant is a measurement question. If the desired contract is a mathematically exact saw at the endpoint, it should be handled as an explicit special case.

### L-05 — Cycle phase discards overshoot

**Classification:** numerical timing limitation.

Like Bézier, LFO uses saturating addition and resets to zero rather than preserving phase residual. Requested frequencies are already quantized in `get_delta_t()`, and the reset introduces an additional discrete-cycle effect.

At low frequencies the relative error is tiny. At high frequencies the number of samples per cycle is small enough that single-sample differences matter.

### L-06 — LFO performs software-expensive division both in rate mapping and waveform evaluation

**Classification:** performance concern.

Each normal LFO sample includes:

1. the common 64-bit division in `get_delta_t()`;
2. a fixed-point division in either the rising or falling waveform branch.

On an 8-bit AVR both are candidates for multi-instruction software routines. Exact execution time must be obtained from the compiled AVR build and hardware timing measurements, but LFO has an unusually division-heavy hot path for a piecewise-linear waveform.

### L-07 — Continuous skew modulation has an unavoidable contract trade-off

**Classification:** design constraint.

For fixed phase \(p\), changing apex \(a\) changes the closed-form value \(y(p,a)\). Therefore a memoryless implementation cannot arbitrarily change `a` while simultaneously guaranteeing:

- unchanged phase;
- unchanged output;
- unchanged current-cycle timing;
- immediate full application of the new shape.

At least one of those properties must give way. The upstream cycle-latched solution chooses output/phase stability and steady-cycle timing at the cost of control latency.

This is useful context for evaluating any replacement.

---

## 4. Improvement strategies

### 4.1 Low-risk startup corrections

Two changes can be evaluated independently from continuous skew modulation.

#### Apply Texture on first processing step

```text
if shapeNotInitialized:
    apex = mapTextureToApex(textureKnob + textureCv)
    shapeNotInitialized = false
```

This removes the forced first-cycle triangle while retaining cycle-boundary updates afterward.

#### Clamp the rising ratio before narrowing

```text
ratioRaw = divideFixed(completed, outOf)
ratioRaw = min(ratioRaw, 0xFFFF)
value16  = uint16(ratioRaw)
```

This mirrors the defensive treatment already used in the falling branch.

Both corrections are implemented in the current unreleased firmware and have dedicated mathematical/regression coverage.

### 4.2 Explicit endpoint saw modes

A clear endpoint contract can avoid tiny denominators:

```text
if texture == 0:
    output = reverseSaw(phase)
else if texture == 1023:
    output = risingSaw(phase)
else:
    output = skewTriangle(phase, apex)
```

This gives exact documented endpoint waveforms and removes dependence on an almost-zero short segment.

It is a behavior change at the top endpoint and therefore needs dedicated compatibility and corrected-profile tests.

### 4.3 Precompute reciprocal slopes per apex

In the upstream implementation apex changes at most once per cycle, which makes repeated division particularly wasteful there. In the corrected implementation apex may change live, so any reciprocal/slope cache must account for control-rate changes explicitly.

Instead, compute reciprocal/slope constants when the apex changes:

```text
onApexChanged(a):
    riseGain = reciprocal(a)
    fallGain = reciprocal(1 - a)
```

Then each sample uses multiplication:

```text
if phase < a:
    y = phase * riseGain
else:
    y = (1 - phase) * fallGain
```

The reciprocal format must be selected so that maximum DAC error is bounded, ideally by exhaustive host tests over all 1024 apex codes and the complete relevant phase domain.

This moves expensive division from 2,500 times per second to at most once or twice per LFO cycle.

### 4.4 Preserve phase residual

A wrapping accumulator can preserve overshoot:

```text
newPhase, rollover = overflowingAdd(phase, dt)
phase = newPhase
```

The rollover sample can then be evaluated under an explicitly defined endpoint convention rather than by saturating to `UINT32_MAX`.

This is implemented in the current unreleased firmware. Exact upstream sample sequences therefore diverge at wrap boundaries by design.

### 4.5 Phase-remapped continuous skew

One mathematically clean way to change skew without an instantaneous output jump is to **remap phase** so the current output is preserved under the new apex.

For a rising segment with current normalized output \(y\):

\[
p' = y a'.
\]

For a falling segment:

\[
p' = a' + (1-y)(1-a').
\]

Pseudocode:

```text
on texture change:
    y = currentOutputNormalized
    newApex = mapTexture(texture)

    if currentlyRising:
        phase = y * newApex
    else:
        phase = newApex + (1 - y) * (1 - newApex)

    apex = newApex
```

This preserves output continuity at the moment of change, but it **warps phase** and therefore changes the timing of the current cycle. Repeated Texture modulation effectively becomes phase modulation.

That may be musically desirable, but it is a different contract from the original LFO and must be evaluated as such.

### 4.6 Slewed shape parameter

Another option is to keep global phase untouched but slew the apex rather than applying a step change:

```text
apex += clamp(targetApex - apex, -maxStep, +maxStep)
y = skewTriangle(phase, apex)
```

This bounds the discontinuity per sample but cannot make it mathematically zero, and very small apex values can still create large sensitivity.

It is simpler than phase remapping but requires a new shape-slew time constant and careful endpoint handling.

---

## 5. Computational cost

### 5.1 Current hot path

Source-level operation inventory:

| Component | Work |
|---|---|
| Speed mapping | exp2 LUT interpolation, integer scaling, 64-bit division |
| Phase | 32-bit saturating add and compare |
| Shape | branch on phase/apex |
| Waveform | fixed-point division on every sample |
| Output | clamp on falling branch, 16->12-bit shift |

There is very little multiplication-heavy DSP here. The cost is dominated by **division**.

At 2.5 kHz, the current structure can require roughly:

```text
2500 common frequency divisions / second
2500 waveform divisions / second
```

The statement above counts source-level division operations. It does not claim a specific AVR cycle count.

### 5.2 Worst-case timing cases

Timing qualification should cover:

- rising branch;
- falling branch;
- minimum apex;
- maximum apex;
- rollover sample where Texture is remapped;
- any corrected reciprocal-recalculation path;
- continuously changing Speed CV.

### 5.3 Optimization priority

LFO should be one of the highest-value modes for AVR optimization because the mathematical waveform is piecewise linear and therefore does not intrinsically require per-sample division.

The likely priority is:

1. common frequency mapping;
2. per-cycle reciprocal/slopes;
3. only then smaller arithmetic cleanup.

---

## 6. Verification and test strategy

### 6.1 Pure waveform reference tests

A pure helper that accepts phase and apex should be testable independently from Speed mapping.

For each legal apex family:

```text
output at phase 0 is the defined minimum
output rises monotonically before apex
output falls monotonically after apex
output remains within 0..4095
center apex produces symmetric triangle within discrete tolerance
```

### 6.2 Endpoint mode tests

Compatibility profile:

- Texture 0 matches the upstream reverse-saw behavior;
- Texture 512 maps to `0x80000000` only after rollover;
- Texture 1023 maps to `0xFFC00000`;
- maximum Texture retains the short falling segment.

Corrected endpoint profile, if adopted:

- code 0 is exact reverse saw;
- code 1023 is exact rising saw;
- no division by zero or near-zero denominator occurs.

### 6.3 Startup regression tests

Required upstream-characterization tests:

1. first-cycle output is independent of Texture before rollover;
2. constructor apex is `UINT32_MAX/2`;
3. a low-rate phase trajectory that enters `0x7FFF0000..0x7FFFFFFE` reproduces the rising-branch narrowing wrap if the reference arithmetic does so.

The third test should preferably exercise a pure waveform helper/state injection rather than waiting tens of seconds in an integration test.

Corrected profile:

- first sample applies current Texture according to the selected startup contract;
- rising ratio saturates at full-scale instead of narrowing to zero.

### 6.4 Shape-update latency tests

Compatibility test:

```text
change Texture mid-cycle
verify all subsequent samples remain on the old apex until rollover
verify new apex takes effect at rollover
```

Run at multiple Speeds so the test proves state semantics, not wall-clock assumptions.

### 6.5 Period/frequency tests

For a grid of Speed knob/CV values:

- count samples between rollovers;
- derive realized frequency;
- compare against the upstream reference;
- separately compare against the ideal V/oct mathematical target.

This distinguishes compatibility error from inherent upstream quantization.

A corrected direct-phase mapping should have an explicit maximum frequency error requirement.

### 6.6 Continuous-skew tests, if implemented

For phase-remapped skew:

```text
before = output
change apex
remap phase
after = output
assert |after - before| <= 1 DAC LSB
```

Also verify:

- phase remains legal;
- branch selection remains consistent;
- repeated Texture modulation does not overflow;
- steady-state frequency returns to the Speed-defined rate once Texture stops moving;
- maximum transient cycle-length deviation is characterized.

For a slewed-apex design, specify maximum allowed per-sample output step and verify it exhaustively over phase/apex pairs.

### 6.7 Performance qualification

On target hardware:

- toggle a timing pin around LFO processing;
- measure rising/falling/rollover worst-case pulse widths;
- repeat after reciprocal optimization;
- verify the complete runtime stays below the 400 µs DAC service interval with margin;
- confirm the missed-deadline diagnostic remains zero during a sustained stress run.

---

## 7. Engineering assessment

### Preserve for compatibility

- V/oct Speed mapping;
- piecewise skewed-triangle concept;
- summed/clamped Texture control;
- cycle-boundary apex update;
- current endpoint mapping including `1023 << 22`;
- saturating cycle behavior until compatibility vectors are frozen.

### Correct

Strong low-risk candidates:

1. initialize shape from the actual controls rather than forcing the first cycle to a constructor triangle;
2. clamp the rising fixed-point ratio before narrowing to 16 bits;
3. consider exact saw endpoint handling.

### Redesign only with an explicit contract

Continuous skew modulation cannot be treated as a trivial bug fix. A replacement must state which property is primary:

- output continuity;
- phase continuity;
- current-cycle duration;
- immediate control response.

Phase remapping and apex slewing are both viable, but they make different musical trade-offs.

### Optimize

LFO has unusually strong optimization potential because the current hot path performs division for a fundamentally linear waveform. Reciprocal/slope precomputation remains a candidate only if AVR timing measurements show a worthwhile gain and exhaustive waveform-error tests protect the mathematical contract.

### Current conclusion

The upstream LFO is conceptually simple but contains the richest interaction between fixed-point representation and control policy. The cycle-latched Texture behavior is explicitly acknowledged upstream and should not be misrepresented as an accidental coding mistake. Separate from that design trade-off, the forced first-cycle triangle and the constructor-apex narrowing edge are concrete startup problems that can be corrected cleanly. The per-sample division is also a strong performance target once exact compatibility has been demonstrated.

## Implemented state and edge-case verification

The unreleased test suite now verifies phase remapping over a dense phase/apex grid, constant-control accumulation across repeated 32-bit wraps, and repeated live skew changes at slow rate. Exact sawtooth endpoints remain separate cases: a sawtooth contains an intentional cycle discontinuity, so changing live to an exact saw endpoint while the preserved output maps immediately adjacent to that discontinuity cannot in general guarantee a small next-sample delta. The continuity stress test therefore targets continuous skewed-triangle forms, while exact rising/falling saw mathematics and endpoint remapping are verified independently.

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../../assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
