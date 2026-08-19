# Acid algorithm engineering analysis

## 1. Purpose and scope

Acid is the implemented Electronica-bank modulation algorithm for creating deterministic stepped-and-sliding CV contours with accent emphasis. It is intended primarily for filter cutoff, wavefolding, oscillator timbre and effect-parameter modulation.

It is **not** a TB-303 synthesizer or note sequencer. The historical association is musical vocabulary only: 16-step patterns, accents and slides are characteristic TB-303 sequencing concepts documented by Roland. There is no Quinn Freedman Acid mode to preserve.

## 2. Mathematical foundations

### 2.1 Base level permutation

For step index $n\in\{0,\ldots,15\}$, define

$$
q_n=(5n+3)\bmod16.
$$

Because $\gcd(5,16)=1$, multiplication by 5 modulo 16 is a permutation. Therefore every code 0..15 appears exactly once per 16-step cycle.

The base 12-bit-safe target is

$$
L_n=1024+128q_n,
$$

which lies in

$$
1024\le L_n\le2944.
$$

This headroom leaves room for accent emphasis without forcing constant rail clipping.

### 2.2 Accent mask

Define

$$
a_n=[n\bmod4=0]\lor[n\bmod7=0].
$$

At Texture $\tau$, the maximum accent contribution is

$$
A(\tau)=768\tau.
$$

The accent envelope within normalized step phase $p$ uses

$$
D(p)=1-S(p),
$$

with $S(p)=3p^2-2p^3$.

### 2.3 Slide mask

Define

$$
s_n=[(5n\bmod16)<4].
$$

On a slide step, Texture morphs between immediate target and full-step interpolation. Let $L_{n-1}$ be the prior target and

$$
I_n(p)=(1-S(p))L_{n-1}+S(p)L_n.
$$

Then

$$
B_n(p;\tau)=
\begin{cases}
(1-\tau)L_n+\tau I_n(p), & s_n=1,\\
L_n, & s_n=0.
\end{cases}
$$

The final normalized DAC-domain result is the saturated sum of base/slide contour and accent term.

These masks are project-defined musical parameters. They are not transcriptions of any Roland factory pattern.

## 3. Reference algorithm

At each processing sample:

1. map Speed to Electronica tempo;
2. advance a sixteenth-note phase accumulator;
3. on wrap, increment step index modulo 16;
4. compute the mathematically defined base target $L_n$;
5. evaluate accent and slide predicates from $n$;
6. derive Texture macro $\tau$;
7. if the step slides, interpolate from previous target; otherwise hold the current target;
8. if the step is accented, add the decaying accent contour;
9. saturate once to 0..4095.

No random state is used.

## 4. Relationship to prior art and upstream Drift

Roland describes the TB-303 as central to acid and its current official software instrument exposes 16-step patterns with accents and slides: <https://www.roland.com/global/products/rc_tb-303/>. The original hardware manual also documents entering accents and slides: <https://cdn.roland.com/assets/media/pdf/TB-303_OM.pdf>.

This Drift mode borrows only those high-level sequencing ideas. It does not reproduce Roland circuitry, pattern data, pitch encoding, filter behavior, envelope law or software source.

There is no upstream Drift mode to preserve.

## 5. Behavioral analysis

At Texture zero, Acid is a dry deterministic 16-step CV sequence. Increasing Texture has two coupled effects:

- accent steps become more pronounced;
- designated transitions become increasingly legato.

That coupling is intentional: Texture is interpreted as **liquidity/emphasis**, not generic randomness.

The sequence repeats exactly after 16 sixteenth notes. Repetition is part of the design and differentiates Acid from Turing and Motif. The pattern should sound like a stable riff-shaped modulation source rather than a generative sequencer.

## 6. Findings and classification

- **Mathematical requirement:** multiplier 5 must remain coprime with 16 if the one-visit-per-level permutation property is claimed.
- **Musical design choice:** level offset/scaling, accent predicate and slide predicate are project-defined and require audition.
- **Sound-design choice:** Texture jointly increases accent and slide intensity.
- **Implementation requirement:** accent addition must saturate after interpolation, not wrap 12-bit arithmetic.
- **Provenance requirement:** the mode must not imply Roland affiliation or claim to emulate a TB-303.
- **Duplication boundary:** no random phrase mutation is allowed; adding it would overlap Generative Motif/Turing.

## 7. Improvement strategy

The implementation freezes exactly one deterministic grammar. Additional pattern sets, direction modes or randomization would make the mode more sequencer-like and should be separate future revisions.

If the fixed contour proves too recognisable, the first alternative should be a second mathematically generated permutation selected at compile time or via a future bank revision—not silent per-cycle randomization.

## 8. Computational cost on ATmega328P

Most work is small-integer state:

- one sixteenth phase accumulator;
- 4-bit step index;
- modular predicates at step boundaries;
- one smooth interpolation on slide steps;
- one decay contour on accent steps.

The modulo operations need not execute in the 2.5 kHz hot path. Step index is only 0..15, so accent/slide masks can be precomputed as two 16-bit masks after the mathematical predicates are verified.

## 9. Optimization opportunities

- Compile the accent and slide predicates into constant 16-bit masks.
- Generate the 16 base target values at compile time or keep the cheap `(5*n+3)&15` formula.
- Evaluate slide/accent only for the current step.
- Use one shared smoothstep primitive for both slide and accent decay.
- Avoid general division by using normalized phase directly.

## 10. Verification and test strategy

Required tests:

- $q_n$ is a permutation of 0..15;
- every base level lies in 1024..2944;
- accent and slide masks match the stated mathematical predicates for all 16 steps;
- Texture zero produces no accent boost and no interpolation contribution;
- maximum Texture produces full declared accent and full smooth slide on marked steps;
- non-slide steps remain constant apart from accent decay;
- slide interpolation starts at previous target and ends at current target;
- accent envelope is monotone non-increasing;
- final output always remains in 0..4095;
- complete state repeats after exactly 16 steps for static controls;
- fixed controls are deterministic across resets;
- no RNG state is consumed;
- timing probe stays within budget.

Listening regression should specifically test filter cutoff because the usefulness of the chosen masks cannot be proven by unit tests alone.

## 11. Musical assessment

**Musical value: high, potentially very high after audition.**

Acid gives Drift a type of behavior not present in any existing bank: a stable electronic riff contour with both hard steps and controlled legato. It should be particularly effective for:

- resonant filter cutoff;
- wavefolder drive/position;
- wavetable position;
- FM index or oscillator shape;
- delay/reverb parameters that benefit from recurring accents.

Its strength is identity. Its weakness is also identity: a fixed 16-step grammar can become obviously repetitive. That is acceptable if the contour is musically good, but it makes listening validation more important here than for Pump or Shuffle.

## 12. Engineering assessment

The math and resource cost are low-risk. The unresolved risk is musical, not computational: whether the chosen deterministic contour deserves to be frozen as a factory algorithm. The implementation should therefore follow the documented formulas exactly and be evaluated in several acid/house/techno patches before the pattern contract is declared final.
<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../../assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
