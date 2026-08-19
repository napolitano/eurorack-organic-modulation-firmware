# Breath algorithm engineering analysis

## 1. Purpose and scope

Breath is the implemented Ambient-bank modulation mode that produces recurring smooth swells while allowing each complete cycle to vary slightly in duration, height and asymmetry.

The algorithm is intentionally project-defined. It is not a physiological breathing simulator and should not be documented as one. The word **Breath** describes the musical gesture: rise, peak, release, rest, repeat with controlled imperfection.

There is no Quinn Freedman Breath mode to preserve.

## 2. Mathematical foundations

Each cycle is a smooth unipolar envelope with normalized phase

$$
p\in[0,1).
$$

Let the peak occur at cycle fraction

$$
s\in(0,1).
$$

Using cubic smoothstep

$$
S(x)=3x^2-2x^3,
$$

the normalized envelope is

$$
E(p;s)=
\begin{cases}
S(p/s), & p<s,\\
1-S((p-s)/(1-s)), & p\ge s.
\end{cases}
$$

This guarantees

$$
E(0)=0,
$$

$$
E(s)=1,
$$

and the cycle returns to zero at its endpoint. Because $S'(0)=S'(1)=0$, both baseline and peak are reached with zero slope.

The per-cycle output is

$$
y(p)=A_n E(p;s_n),
$$

where amplitude $A_n$, skew $s_n$ and duration multiplier $d_n$ are held constant for the entire cycle $n$.

## 3. Texture-controlled cycle variation

Texture controls the amount of independent cycle-to-cycle variation, not instantaneous sample noise.

At each cycle rollover, three signed uniform random values are drawn and mapped to bounded parameters. At maximum Texture, the proposed musical bounds are:

$$
0.75\le d_n\le1.25,
$$

$$
0.65\le A_n\le1.00,
$$

$$
0.25\le s_n\le0.50.
$$

At Texture zero these ranges collapse to fixed nominal values. The initial proposed nominal point is:

$$
d=1,
$$

$$
A=0.825,
$$

$$
s=0.375.
$$

These ranges are now frozen in the implementation: duration uses Q10 `768..1280` around `1024`, amplitude uses DAC codes `2662..4095` around `3378`, and peak position uses Q0.12 `1024..2048` around `1536`.

Speed controls nominal cycle rate through the common Ambient /16 macro-time scale. The duration multiplier is applied per cycle without discarding phase overshoot.

## 4. Relationship to prior art and upstream Drift

The use of smooth attack/release envelopes is generic signal-processing practice. Breath's specific stochastic parameter ranges and cycle-update law are project-defined.

There is no upstream Quinn Freedman Breath implementation.

The closest existing Drift modes are LFO and Bézier:

- LFO repeats one waveform indefinitely unless controls change;
- Bézier selects arbitrary random endpoints and interpolates between them;
- Breath always has the same topological gesture: baseline → one peak → baseline, while the **whole gesture parameters** vary only at cycle boundaries.

This invariant return to baseline is the key behavioral distinction.

## 5. Behavioral analysis

At Texture zero, Breath is a stable asymmetric smooth cycle. It is intentionally more envelope-like than Classic LFO's triangle/saw behavior.

As Texture rises, each completed cycle can become a little longer or shorter, louder or softer, and more or less asymmetric. Because parameters are latched for the entire cycle, there are no mid-cycle discontinuities from changing the random target values.

Texture CV can still change the *amount of randomness applied at the next rollover*, but must not retrospectively mutate parameters already latched for the current cycle.

At maximum irregularity the process should remain recognizably cyclical. The parameter ranges are deliberately bounded so Breath never degenerates into arbitrary random Bézier movement.

## 6. Findings and classification

- **Musical design choice:** randomness occurs once per completed cycle, not every sample.
- **Topology requirement:** every cycle begins and ends at baseline and contains exactly one peak.
- **Continuity requirement:** baseline and peak have zero slope in the mathematical envelope.
- **Sound-design choice:** peak occurs in the first half of the cycle, preserving a characteristic faster-rise/slower-release gesture.
- **Implementation requirement:** new random parameters are latched only at rollover.
- **Compatibility status:** no upstream behavior exists to preserve.

## 7. Improvement strategy

The first implementation should keep exactly three varying parameters. Adding rest phases, double peaks, random baseline offsets or multiple envelope stages would weaken the algorithm's identity and should be treated as future variants.

The amplitude, period and skew ranges are now implementation constants. Any later listening-driven retuning must be treated as an explicit behavioral change because it alters the musical contract.

## 8. Computational cost on ATmega328P

Normal samples require:

- one phase advance;
- one branch selecting rise or fall;
- one normalized fixed-point phase calculation;
- one cubic smoothstep evaluation;
- one amplitude scaling.

Only at cycle rollover:

- three RNG draws;
- three bounded parameter mappings;
- one cycle-rate/duration update.

Persistent state is one 32-bit phase accumulator, current amplitude, current skew and current duration/rate adjustment.

This should be a comparatively cheap Ambient mode because stochastic work occurs only once per cycle.

## 9. Optimization opportunities

- Reuse the verified cubic smoothstep integer evaluator.
- Keep skew and amplitude in power-of-two fixed-point domains; the implementation uses Q0.12 and direct 12-bit DAC amplitude respectively.
- Use bounded integer interpolation from Texture to the maximum deviation ranges.
- Generate all three cycle random values only at rollover.
- Preserve phase overshoot exactly rather than resetting phase to zero.
- Cache duration scale and attack/release reciprocals only at rollover so the normal 2.5 kHz path avoids general integer division.

## 10. Verification and test strategy

Required mathematical tests:

- envelope is exactly zero at baseline and exactly at documented peak scale at $p=s$;
- envelope is monotone increasing before $s$ and monotone decreasing after $s$;
- left/right slopes approach zero at baseline and peak under the discrete representation;
- all generated skews remain strictly inside the documented safe range;
- Texture zero produces identical cycle parameters indefinitely;
- maximum Texture never exceeds the documented amplitude, duration or skew ranges;
- parameters change only at cycle rollover;
- phase overshoot is preserved;
- output remains in 0..4095;
- fixed seed/control sequence is deterministic.

A regression test should explicitly prove that changing Texture mid-cycle does not alter the already-latched current cycle parameters.

## 11. Musical assessment

**Musical value: high.**

Breath is less mathematically exotic than Current or Anchor, but it offers a highly legible musical gesture. It is particularly useful where totally free random movement would obscure phrasing.

Strong applications include:

- VCA level swells;
- filter opening/closing;
- reverb send and decay movement;
- evolving spectral brightness;
- slow modulation of drone density or oscillator blend.

Its musical strength is **recognizable recurrence without mechanical repetition**.

The main overlap risk is low Texture, where any repeated smooth envelope begins to resemble an LFO. The cycle-to-cycle parameter mutation is therefore not decorative; it is the defining feature of the mode.

## 12. Engineering assessment

Breath is low-risk computationally and numerically. The implementation freezes the bounded parameter ranges and computes random parameters only on rollover. The host suite verifies nominal Texture-zero parameters, full-Texture bounds, baseline/peak/baseline topology, reciprocal duration endpoints and deterministic bounded output.

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../../assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
