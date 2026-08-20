# Wobble algorithm engineering analysis

## 1. Purpose and scope

Wobble is the proposed first mode of the working Dubstep/Bass bank. It generates a continuous modulation CV whose **rate follows a deterministic tempo-synchronised phrase**.

The distinction from Drift's Classic LFO is fundamental: Classic LFO is a general free-running waveform source. Wobble should use one deliberately simple carrier and derive its musical identity from switching between beat-relative rate values such as quarter, 3/16, eighth and triplet subdivisions.

There is no Quinn Freedman Wobble mode to preserve.

## 2. Musical and historical basis

Sound On Sound describes the familiar dubstep wobble as LFO modulation of filter cutoff and notes that synchronising the LFO rate to track tempo often works best:

<https://www.soundonsound.com/techniques/dubstep-basics>

Native Instruments provides a concrete production example in which an LFO is sync-enabled and set to **3/16**, yielding what the tutorial calls a classic dubstep wobble:

<https://blog.native-instruments.com/how-to-make-dubstep/>

MusicRadar likewise describes the wobble technique as LFO modulation of a filter and demonstrates tempo-relative/triplet settings:

<https://www.musicradar.com/how-to/lfo-wobble-bass>

The source evidence supports tempo-locked LFO movement and multiple musical divisions. It does not establish one canonical rate sequence. Any Drift phrase is therefore an original project design.

## 3. Mathematical foundations

Let $\phi\in[0,1)$ be carrier phase. Use a fixed unipolar triangle

$$
W(\phi)=1-\left|2\phi-1\right|.
$$

Thus

$$
0\le W\le1,
$$

with $W(0)=0$, $W(1/2)=1$ and a continuous return to zero at phase wrap.

Let the quarter-note frequency be

$$
f_q=\frac{B}{60}.
$$

For a selected rate ratio $r$ in cycles per quarter, carrier frequency is

$$
f_w=r f_q.
$$

The proposed rational rate vocabulary is

$$
R=\left\{\frac12,\frac23,1,\frac43,\frac32,2,3,4\right\}.
$$

The corresponding carrier periods are:

| $r$ cycles/quarter | Musical period |
|---:|---|
| $1/2$ | half note |
| $2/3$ | dotted quarter |
| $1$ | quarter |
| $4/3$ | 3/16 (dotted eighth) |
| $3/2$ | quarter-note triplet duration |
| $2$ | eighth |
| $3$ | eighth-note triplet |
| $4$ | sixteenth |

The phase accumulator remains continuous when $r$ changes. Only its increment changes. This avoids an output discontinuity at every rate switch.

## 4. Proposed rate-phrase model

The phrase is divided into eight eighth-note cells per 4/4 bar. Each cell references one of four phrase symbols according to the project-defined index word

$$
P=(0,1,0,2,1,3,0,2).
$$

Texture selects one of four rate vocabularies:

| Texture region | Symbol 0 | Symbol 1 | Symbol 2 | Symbol 3 |
|---:|---:|---:|---:|---:|
| 0 | $1$ | $1$ | $1$ | $1$ |
| 1 | $1$ | $2$ | $1$ | $2$ |
| 2 | $1$ | $4/3$ | $2$ | $3/2$ |
| 3 | $1$ | $2$ | $3$ | $4$ |

This first contract has useful properties:

- Texture zero is a stable quarter-note wobble;
- region 1 introduces simple quarter/eighth alternation;
- region 2 introduces dotted/triplet displacement;
- region 3 becomes fast and aggressive without exceeding sixteenth-period carrier cycles;
- the phrase remains deterministic and exactly one bar long.

The specific word and vocabulary are **listening-test candidates**, not historical dubstep data.

## 5. Texture mapping

With saturated Texture code $T\in[0,1023]$, region is

$$
c(T)=\left\lfloor\frac{4T}{1024}\right\rfloor,
$$

with the implementation saturating the maximum input to region 3.

Texture changes should be latched at the next bar boundary. A slowly moving Texture CV should not rewrite the rate vocabulary halfway through an existing phrase.

## 6. Clock-source contract

Wobble should use the proposed bank's shared clock behavior:

- no valid Speed-CV clock: internal Speed-knob tempo;
- valid 0..5 V Speed-CV quarter-note clock: external timing;
- two rising edges to acquire;
- loss after more than 2.5 periods: knob fallback;
- re-lock: deterministic new phrase origin.

Raw 10 V Eurorack clocks remain unsupported on the current hardware.

## 7. Reference algorithm

At each processing sample:

1. obtain current quarter duration from the internal tempo or external-clock measurement;
2. advance bar/eighth-cell position while preserving timing overshoot;
3. at bar boundary, latch the current Texture region;
4. map current cell's phrase symbol through the latched rate vocabulary;
5. convert the exact rational rate into a phase increment;
6. advance the Wobble carrier phase without resetting it when the rate changes;
7. evaluate the fixed triangle carrier;
8. scale to 0..4095.

No RNG is required.

## 8. Relationship to prior art and upstream Drift

The general LFO-to-filter wobble technique is established production practice. Drift's originality is not the existence of an LFO but the **specific compact deterministic rate-phrase engine** under the existing two-control hardware.

There is no upstream Wobble implementation. The mode must not claim to emulate a particular artist, preset, Massive/Serum patch or commercial sequencer.

## 9. Duplication boundary

### Versus Classic LFO

Classic LFO exposes general periodic motion. Wobble must keep:

- one fixed carrier family;
- rate quantisation to musical ratios;
- explicit one-bar rate phrases;
- clock-relative behavior.

If Wobble later gains free waveform selection or arbitrary continuous rate, it collapses back toward Classic LFO.

### Versus Electronica Shuffle

Shuffle changes event spacing while preserving pair duration. Wobble changes continuous modulation frequency; it does not shift event onsets.

### Versus Percussion Repeat

Repeat schedules discrete ratchet pulses around quarter anchors. Wobble remains a continuous modulation signal and never produces a pulse cluster schedule.

## 10. Computational cost on ATmega328P

The hot path is inexpensive:

- one quarter/bar scheduler update;
- one small rate-table lookup;
- one fixed-point phase add;
- one triangle evaluation using subtraction/absolute value;
- one 12-bit scaling.

Rational rate conversion should not use general division every sample. Each vocabulary entry should resolve to a precomputed fixed-point multiplier or phase increment when tempo changes.

State is small: carrier phase, phrase/cell counters, latched Texture region and current rate index.

## 11. Optimization opportunities

- Store the four vocabularies in flash.
- Precompute all eight rational phase multipliers.
- Use the current cell's already-resolved multiplier until the next cell boundary.
- Keep carrier phase in the existing unsigned fixed-point phase representation.
- Avoid sine lookup entirely; triangle is intentional.

## 12. Verification and test strategy

Required tests:

- triangle carrier exact values at phase 0, 1/4, 1/2, 3/4 and wrap;
- output remains 0..4095 for all phase values;
- all eight rational rate entries match a high-precision beat-period reference;
- Native-Instruments-style 3/16 ratio is represented exactly by $r=4/3$ within the documented phase-increment quantisation;
- each Texture region maps to the exact documented vocabulary;
- maximum Texture saturates to region 3;
- phrase word is exactly eight cells and repeats after one bar;
- Texture changes mid-bar take effect only at the next bar boundary;
- carrier phase is continuous across every rate change;
- no RNG state is consumed;
- fixed controls remain bit-for-bit deterministic;
- external clock and internal fallback produce equivalent phase evolution for equal quarter periods;
- long-run bar boundaries do not drift;
- AVR timing probe remains below 400 microseconds.

Golden vectors should cover at least four complete bars for every Texture region.

## 13. Musical assessment

**Musical value: very high.**

Wobble is the clearest justification for the proposed bank. It turns Drift from a source of general movement into a source of **musically phrased movement** that can drive:

- filter cutoff/resonance;
- wavetable position;
- wavefolder amount;
- FM index;
- distortion drive;
- VCA level for classic wub-style articulation.

The deterministic phrase design is an advantage for performance: the user can learn what each Texture region does and intentionally return to it.

The main risk is cliché and overlap. The mode must be presented as one bass-music modulation tool, not as a definition of dubstep.

## 14. Engineering assessment

Wobble is low risk mathematically and computationally. The critical design work is musical: rate vocabulary, phrase word and boundary semantics. It should be the **first prototype** of the proposed bank because it can quickly validate both the new tempo mapping and the bank-wide external-clock concept.

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../../assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
