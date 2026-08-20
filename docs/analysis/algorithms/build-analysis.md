# Build algorithm engineering analysis

## 1. Purpose and scope

> **Implementation status — released in 0.3.0:** Implemented under `domain/dubstep/` with the 8/4/2/1-bar Texture mapping, cubic-smoothstep macro rise, quarter/eighth/sixteenth/thirty-second micro stages and deterministic phrase restart on external-clock acquisition. The one-bar endpoint remains explicitly subject to later listening evaluation.


Build is the proposed fourth mode of the working Dubstep/Bass bank. It creates a repeating **multi-bar tension CV** whose macro level rises while a tempo-synchronised micro modulation accelerates toward the phrase boundary.

The mode does not generate audio, a snare roll or a complete arrangement. It provides one CV gesture that can drive a filter, VCA, effect, wavetable or other destination through a recognisable build-to-reset trajectory.

There is no Quinn Freedman Build mode to preserve.

## 2. Musical basis

Build/drop contrast is central to many modern bass-music and EDM arrangements. Native Instruments' dubstep production guide explicitly structures a track around breakdown and drop sections:

<https://blog.native-instruments.com/how-to-make-dubstep/>

Jeremy W. Smith's 2024 research on continuous processes in EDM notes that intense risers are common in bass genres and are often shorter — around two or four bars — than long trance-style rises:

<https://doi.org/10.12801/1947-5403.2024.16.01.06>

MusicRadar describes the established build technique of progressively halving rhythmic values before a drop, e.g. quarter/eighth/sixteenth/thirty-second activity:

<https://www.musicradar.com/how-to/8-classic-recipes-for-a-perfect-drop>

These sources support the **general tension/rate-acceleration concept**. The exact Drift contour is project-defined.

## 3. Phrase model

Let phrase length be $N$ bars. The initial candidate set is

$$
N\in\{8,4,2,1\}.
$$

Texture selects the phrase length in four regions, with increasing Texture producing a shorter and therefore more frequently repeating build.

For normalized phrase phase

$$
u\in[0,1),
$$

define the macro rise using cubic smoothstep:

$$
M(u)=3u^2-2u^3.
$$

Therefore

$$
M(0)=0,
$$

$$
M(1)=1,
$$

and the slope is zero at both endpoints before the intentional phrase reset.

## 4. Micro-rate escalation

Divide each phrase into four equal temporal quarters. The micro modulation period is:

| Stage | Phrase interval | Period |
|---:|---|---|
| 0 | $[0,1/4)$ | quarter note |
| 1 | $[1/4,1/2)$ | eighth note |
| 2 | $[1/2,3/4)$ | sixteenth note |
| 3 | $[3/4,1)$ | thirty-second note |

Let $Q(u)$ be a unipolar triangle carrier at the selected micro rate. The proposed composite output is

$$
Y(u)=M(u)\left(\frac14+\frac34Q(u)\right).
$$

This ensures:

- the overall available modulation range rises over the phrase;
- micro movement remains audible before the macro ramp reaches full scale;
- the final quarter contains the fastest movement and highest macro level;
- the phrase-end reset returns the output to the beginning state for the next cycle.

The reset is musically intentional and represents the transition/release point.

## 5. Texture mapping

A candidate mapping is:

| Texture region | Phrase length |
|---:|---:|
| 0 | 8 bars |
| 1 | 4 bars |
| 2 | 2 bars |
| 3 | 1 bar |

Texture changes should latch only when a phrase completes. Changing Texture in the middle of an eight-bar build must not suddenly reinterpret the current phase as a one-bar build.

The one-bar option is the weakest part of the proposal. Research support is stronger for two- and four-bar bass-music risers. Listening tests should compare `{8,4,2,1}` against alternatives such as `{8,4,2,2}` or `{16,8,4,2}` before implementation is frozen.

## 6. Clock-source contract

Build is phrase-sensitive and therefore benefits strongly from external sync.

Recommended behavior:

- internal mode derives bars from the Speed knob;
- external mode treats each accepted Speed-CV rising edge as a quarter note;
- the second accepted edge on lock establishes beat 1/bar 1/phrase start;
- clock loss falls back to the knob without resetting the phrase counter;
- later re-lock starts a fresh deterministic build phrase.

The 0..5 V current-hardware clock restriction remains mandatory.

## 7. Reference algorithm

At every processing sample:

1. obtain the current quarter-note period;
2. advance quarter/bar/phrase counters while preserving overshoot;
3. compute normalized phrase phase $u$ in fixed point;
4. select one of four micro-rate stages from the top two bits/quadrants of phrase phase;
5. advance the micro triangle phase at quarter/eighth/sixteenth/thirty-second rate;
6. evaluate macro smoothstep $M(u)$;
7. evaluate micro triangle $Q$;
8. combine them as $Y(u)$;
9. on phrase wrap, reset macro phase and latch the next Texture-selected phrase length.

No RNG is required.

## 8. Duplication boundaries

### Versus Percussion Repeat

Repeat generates discrete trigger clusters around quarter anchors and intensifies them near phrase ends. Build generates a **continuous CV** with a whole-phrase macro rise and stagewise carrier acceleration. It never schedules ratchet events.

### Versus Pump

Pump resets low on every beat and recovers high inside that beat. Build rises across multiple bars and resets only at phrase end.

### Versus Ambient Breath

Breath is a smooth recurrent macro gesture with stochastic cycle variation and no beat grid. Build is fully deterministic, tempo locked and contains explicit rhythmic acceleration.

### Versus Wobble

Wobble's primary information is the repeated rate phrase. Build's primary information is **directional tension over form**; its micro rate sequence is subordinate to the macro ramp.

## 9. Computational cost on ATmega328P

At the proposed 280 BPM maximum, a 32nd note lasts approximately

$$
\frac{60}{280\cdot8}\approx26.8\ \mathrm{ms}.
$$

At 2.5 kHz this is about 67 scheduler samples, so the fastest micro motion remains well resolved.

Per processing sample the algorithm needs:

- phrase/grid counter update;
- one micro phase add;
- one triangle evaluation;
- one fixed-point smoothstep;
- a few multiplies/adds for output composition.

This is materially cheaper than many existing Organic/Ambient calculations.

## 10. Optimization opportunities

- Derive the four stage rates from shifts of the quarter phase increment where possible.
- Use phase quadrant bits to select the micro-rate stage.
- Compute smoothstep in the existing Q-format.
- Latch phrase length only at phrase boundary.
- Avoid general division by carrying a fixed-point normalized phrase accumulator.

## 11. Verification and test strategy

Required tests:

- all phrase lengths map to exact documented Texture regions;
- phrase length changes only at phrase reset;
- macro $M(u)$ is monotone non-decreasing over one phrase;
- $M(0)=0$ and reaches full-scale endpoint within documented fixed-point behavior;
- micro stage boundaries occur at exact 1/4, 1/2 and 3/4 phrase positions;
- stage rates are exactly quarter/eighth/sixteenth/thirty-second relative to the current quarter clock;
- composite output remains 0..4095;
- phrase-end reset is deterministic and produces no accumulated phase error;
- no RNG state is consumed;
- internal and external clocks produce equivalent build timing for equal quarter periods;
- clock fallback does not restart the phrase;
- re-lock restarts phrase origin exactly as documented;
- maximum-tempo 32nd stage remains safely inside scheduler resolution;
- AVR timing remains below 400 microseconds.

Golden vectors should cover full phrases for all four Texture regions. For the eight-bar region this is long but deterministic and should be generated offline rather than hand-authored.

## 12. Musical assessment

**Musical value: high; genre specificity: medium.**

Build can be extremely useful in a modular system because one CV can simultaneously imply “opening up” and “getting faster”. Good destinations include:

- filter cutoff/resonance;
- wavetable/waveshape position;
- VCA level;
- distortion or wavefolder drive;
- reverb/delay feedback or send;
- modulation-depth CV on another oscillator/LFO.

Its weakness is that builds and risers are not uniquely dubstep. The slot must justify itself by how well it complements Wobble/Chop in actual bass-music patches, not by genre labeling alone.

## 13. Engineering assessment

Build is computationally straightforward and mathematically transparent. The design uncertainty lies in **form length and usability without a dedicated reset input**. Because current Drift can only reset the phrase implicitly through start/re-lock, a repeating automatic build may be less useful than the concept suggests on paper.

It should therefore be prototyped, but it is the strongest candidate for replacement if the four-slot bank needs revision after listening tests.

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../../assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
