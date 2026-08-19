# Polymeter algorithm engineering analysis

## 1. Purpose and scope

Polymeter is the implemented Electronica-bank mode for creating long deterministic accent cycles from two meters that share one sixteenth-note subdivision grid but use different loop lengths.

The mode does not generate separate trigger outputs. Instead, every subdivision emits a short CV contour whose amplitude reflects whether the primary meter, secondary meter, both or neither begin on that step. There is no Quinn Freedman Polymeter mode to preserve.

## 2. Mathematical foundations

Let the global step counter be $n\ge0$. The primary meter length is fixed at

$$
a=4.
$$

Texture selects secondary length

$$
b\in\{3,5,7,9\}.
$$

Define indicator functions

$$
A_n=[n\bmod4=0],
$$

$$
B_n=[n\bmod b=0].
$$

The accent level before envelope shaping is

$$
V_n=V_0+V_AA_n+V_BB_n,
$$

with proposed constants

$$
V_0=1024,
$$

$$
V_A=1535,
$$

$$
V_B=1536.
$$

Therefore the strongest coincidence reaches 4095 exactly.

The combined accent sequence repeats after

$$
P=\operatorname{lcm}(4,b).
$$

For the four choices:

| $b$ | $P$ sixteenth steps |
|---:|---:|
| 3 | 12 |
| 5 | 20 |
| 7 | 28 |
| 9 | 36 |

Each step launches a short decay contour scaled by $V_n$. The implemented decay duration is exactly one half of a sixteenth-note step.

## 3. Reference algorithm

At each processing sample:

1. map Speed to Electronica tempo;
2. advance the sixteenth-note phase;
3. on phase wrap, increment global step counters;
4. map Texture into one of four stable secondary-meter regions: 3, 5, 7 or 9;
5. update primary and secondary countdown counters without general modulo in the hot path;
6. choose one of four amplitude cases: base, primary, secondary, coincidence;
7. restart the fixed decay envelope at that amplitude;
8. output the current 12-bit envelope value.

When Texture changes secondary meter, the new meter takes effect on the next primary four-step boundary. The new secondary cycle is restarted on that boundary, making the change deterministic and explicitly aligned to the four-step anchor.

## 4. Relationship to prior art and upstream Drift

In polymeter, multiple metric cycles coexist with differing lengths while sharing a common underlying time unit. A music-theoretical discussion of polymetric structure and perception is Leslie S. Tabak, *Revisiting Polymeter*, Music Theory Online 32.2 (2026): <https://www.mtosmt.org/issues/mto.26.32.2/mto.26.32.2.tabak.html>.

A concise mathematical consequence for two integer cycle lengths is that their joint pattern realigns after the least common multiple.

Drift's implementation is deliberately modest: one shared sixteenth grid, one fixed four-step cycle and one selected odd cycle. It is not a general polymetric sequencer and has no separate voices.

There is no upstream Drift implementation.

## 5. Behavioral analysis

The base pulse on every sixteenth preserves a clear internal grid. The two meter starts act as amplitude accents on that grid:

- primary-only start: medium emphasis;
- secondary-only start: essentially the same medium emphasis;
- coincidence: full-scale emphasis;
- neither: low base pulse.

The use of odd secondary lengths against four creates patterns that continually shift against the four-step reference before realigning.

Texture is intentionally discrete. Pretending that a cycle has a meaningful continuous length of 5.37 steps would destroy the strict polymetric interpretation. The control therefore behaves like a four-position selector spread over the continuous knob/CV range.

## 6. Findings and classification

- **Mathematical requirement:** secondary lengths are integers and the recurrence claim uses exact LCM values.
- **Musical design choice:** primary meter is fixed at four because the bank targets electronic music with a strong four-step reference.
- **Control design choice:** Texture is stepped, not continuously interpolated.
- **Implementation requirement:** meter changes are latched at a stable primary boundary; a small ADC-domain hysteresis should be evaluated if analogue noise causes selector chatter near Texture region boundaries.
- **Optimization requirement:** use small countdown counters rather than AVR modulo on every sample.
- **Duplication boundary:** do not redistribute a number of hits evenly; that would become Euclidean rhythm generation.

## 7. Improvement strategy

The initial meter set 3/5/7/9 is intentionally simple. If listening tests show one region is musically weak, changing the set is a legitimate pre-release design revision.

Future ideas such as rotating phase offsets, different accent weights or three simultaneous meters would materially change the mode and should not be added during optimization.

## 8. Computational cost on ATmega328P

Per 2.5 kHz sample the algorithm mostly advances phase and one decay envelope. Meter logic occurs only at sixteenth boundaries and can use two 8-bit countdown counters.

Persistent state:

- 32-bit sixteenth phase;
- primary countdown 0..3;
- secondary countdown 0..8;
- latched meter selector;
- decay-envelope phase/amplitude.

This is negligible SRAM compared with any Organic/Ambient bank worst case.

## 9. Optimization opportunities

- Replace `% 4` and `% b` with decrement/reset counters.
- Map Texture to meter using its top two bits.
- Precompute the four amplitude cases.
- Use one shared decay curve primitive.
- Apply Texture changes only at primary boundary.

## 10. Verification and test strategy

Required tests:

- Texture regions select exactly 3, 5, 7, 9 including all boundary ADC values;
- any adopted hysteresis has explicit enter/leave thresholds and cannot create an unreachable meter region;
- primary accent repeats every four steps;
- each secondary accent repeats at its selected exact length;
- composite recurrence is exactly 12/20/28/36 steps respectively;
- coincidence steps reach 4095 exactly with proposed weights;
- all amplitude cases remain 0..4095;
- changing Texture mid-primary-cycle does not change the latched meter until the defined boundary;
- one and only one envelope restart occurs per sixteenth;
- fixed controls are deterministic indefinitely;
- no RNG state is consumed;
- countdown implementation matches modulo reference over long runs;
- timing probe remains below 400 µs.

Golden vectors should cover at least two complete composite periods for every secondary meter.

## 11. Musical assessment

**Musical value: very high.**

Polymeter can create long, evolving-feeling techno patterns from almost no state and without randomness. The listener retains a four-step anchor while the second accent cycle walks through it.

Strong patch destinations include:

- filter cutoff or resonance;
- VCA accents on drones/stabs;
- decay time on percussion voices;
- distortion or wavefolder drive;
- effect sends that should peak only on occasional cycle coincidences.

The mode's strongest quality is **structured non-repetition over medium time scales**. It sounds more organised than random probability, yet less obviously looped than a four- or eight-step sequence.

Its limitation is that Texture changes discretely. That is a feature of the mathematical model, but should be made explicit in user documentation.

## 12. Engineering assessment

Polymeter is computationally trivial and mathematically transparent. Its main engineering obligation is to preserve the strict integer-meter meaning: exact counters, exact recurrence, stable meter-change boundaries and no accidental drift. It is a strong Electronica candidate and remains clearly separate from the planned Euclidean Percussion algorithm.
<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../../assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
