# Electronica bank architecture and control contract

## 1. Purpose

The implemented **Electronica** bank targets loop-oriented electronic music: house, acid, techno and adjacent styles in which a modulation source is useful not only because it moves, but because its movement has an immediately legible rhythmic role.

The existing banks already cover continuous stochastic motion, nonlinear/organic behavior, discrete generative memory and long-form ambient modulation. Electronica must therefore avoid becoming another random bank or an early version of the planned Percussion bank. Its identity is **continuous or stepped CV shaped by club-music timing concepts** rather than trigger generation.

The four implemented modes are:

- **Pump** — a free-running sidechain-style duck/recovery contour;
- **Acid** — a deterministic 16-step filter-modulation grammar built from levels, accents and slides;
- **Shuffle** — alternating long/short subdivision timing with a continuous decay contour;
- **Polymeter** — two accent cycles sharing one subdivision grid but using different loop lengths.

There is no Quinn Freedman implementation of these modes. The complete bank is project-defined.

## 2. Naming decision: Shuffle rather than Groove

The first brainstorming pass used **Groove**. That name is too broad for an engineering contract: groove can include timing displacement, velocity, random humanization, articulation and many other performance attributes. Ableton Live's official Groove Pool likewise treats timing, random timing fluctuation and velocity as separate groove dimensions: <https://www.ableton.com/en/manual/using-grooves/>.

The proposed Drift mode controls one specific phenomenon: deterministic alternating subdivision timing. **Shuffle** therefore describes the algorithm more accurately and keeps random humanization available for the separate Percussion-bank design.

## 3. Bank slot mapping

| Slot | DIP 1 | DIP 2 | Electronica algorithm |
|---:|---|---|---|
| 0 | OFF | OFF | Pump |
| 1 | ON | OFF | Acid |
| 2 | OFF | ON | Shuffle |
| 3 | ON | ON | Polymeter |

As with every Drift bank, the bank is selected at build/flash time and the rear DIP switches select one of four algorithms in that firmware image at startup.

## 4. Shared Electronica tempo contract

The normal Drift Speed mapping spans a much wider modulation range than a user expects from a tempo-oriented bank. Electronica therefore uses a bank-local tempo mapping rather than pretending the full 1/40 Hz..100 Hz range is a useful BPM control.

Let normalized combined Speed be $u\in[0,1]$. The nominal quarter-note tempo is

$$
B(u)=30\cdot 2^{3u}\ \text{BPM}.
$$

This gives exactly three octaves of tempo:

$$
30\ \text{BPM}\le B\le240\ \text{BPM}.
$$

The quarter-note frequency is

$$
f_q=\frac{B}{60},
$$

and the sixteenth-note grid used by Acid and Polymeter is

$$
f_{16}=4f_q.
$$

At 240 BPM this is only 16 Hz, leaving about 156 samples per sixteenth at Drift's 2.5 kHz processing rate. No Electronica algorithm therefore requires event timing finer than the existing scheduler can represent.

The implementation uses the existing Exp2 reference table over indices 0..48, corresponding to exactly three octaves, with saturated `Speed knob + Speed CV` in the 10-bit control domain. Floating-point arithmetic is not used in the AVR hot path.

### External synchronization limitation

The current Drift hardware has no dedicated clock input. The Electronica bank is therefore **free-running**. A displayed or documented BPM value is nominal relative to Drift's scheduler; it is not transport lock, DIN sync, MIDI clock or Eurorack clock synchronization.

This limitation is especially important for Pump, Shuffle and Polymeter. The algorithms can produce musically useful rhythm on their own, but the firmware must not claim sample-accurate synchronization to another module.

## 5. Shared control contract

| Algorithm | Speed | Texture | Texture CV | Analog Attenuation |
|---|---|---|---|---|
| **Pump** | nominal tempo | recovery length / pump persistence | lengthens recovery | final modulation depth |
| **Acid** | nominal tempo | accent + slide intensity | increases liquidity/emphasis | final modulation depth |
| **Shuffle** | nominal tempo | straight-to-shuffled timing ratio | increases shuffle | final modulation depth |
| **Polymeter** | nominal tempo | secondary meter length: 3/5/7/9 | selects meter region | final modulation depth |

Texture remains a saturated 10-bit firmware control. Attenuation remains post-DAC analogue scaling and is not visible to firmware.

## 6. Pump: sidechain-style duck/recovery contour

Sidechain ducking is widely used in dance music to reduce a bass or mix signal when the kick arrives. Ableton's official Compressor documentation explicitly describes this dance-music use of sidechaining/ducking: <https://www.ableton.com/en/live-manual/11/live-audio-effect-reference/#sidechaining-in-dance-music>.

Drift has no sidechain input, so **Pump is not a compressor and does not detect a kick**. It generates only the characteristic control contour.

For normalized beat phase $p\in[0,1)$ and recovery endpoint $e\in[1/4,15/16]$,

$$
R(p;e)=S\left(\operatorname{clamp}\left(\frac{p}{e},0,1\right)\right),
$$

with

$$
S(x)=3x^2-2x^3.
$$

At every quarter-note boundary the output resets to minimum, then rises smoothly to full scale. Texture maps monotonically to $e$. Low Texture recovers quickly and leaves most of the beat open; high Texture stays suppressed for most of the beat.

The reset discontinuity is intentional. Removing it would turn Pump into another smooth cyclic envelope and weaken the characteristic ducking gesture.

## 7. Acid: deterministic 16-step modulation grammar

Roland's own current TB-303 documentation describes 16-step sequencing with **accents**, **slides** and shuffle, and identifies the TB-303 as central to the emergence of acid: <https://www.roland.com/global/products/rc_tb-303/>.

Drift's Acid mode is **not a TB-303 emulator**. It has no oscillator, resonant ladder filter, note sequencer or pitch output. It borrows only the musical grammar of stepped levels, accent emphasis and selected slides to create a cutoff/waveshaper-style modulation source.

The mode uses a deterministic 16-step base contour. To avoid embedding a literal copied musical phrase, the level order is defined mathematically as a permutation:

$$
q_n=(5n+3)\bmod16,
$$

$$
L_n=1024+128q_n,
$$

for step $n\in\{0,\ldots,15\}$. This visits all sixteen level codes once per cycle while keeping the base contour away from both DAC rails.

A project-defined accent mask is generated by

$$
a_n=[n\bmod4=0]\lor[n\bmod7=0],
$$

and a project-defined slide mask by

$$
s_n=[(5n\bmod16)<4].
$$

Texture $\tau$ controls both accent boost and slide strength. On accent steps, a short decaying emphasis term is added. On slide steps, the transition from the previous target toward $L_n$ morphs from an immediate step at $\tau=0$ to a full-step smooth interpolation at $\tau=1$.

The exact masks and level scaling are musical project parameters, not historical TB-303 data, and must receive listening validation before implementation is frozen.

## 8. Shuffle: deterministic alternating subdivision timing

Shuffle operates on pairs of equal straight subdivisions and moves the second onset later. Let $r$ be the fraction of the two-subdivision pair occupied by the first interval:

$$
\frac12\le r\le\frac34.
$$

Texture maps monotonically from $r=1/2$ (straight) to $r=3/4$ (heavy shuffle). The two onset locations in normalized pair phase are therefore

$$
t_0=0,
$$

$$
t_1=r.
$$

Each onset launches the same short, monotone decay contour. Timing changes; amplitude does not. This is deliberate: Ableton's groove model distinguishes timing influence from velocity and random humanization, and Drift should preserve that conceptual separation rather than letting one Texture macro silently alter all three dimensions: <https://www.ableton.com/en/manual/using-grooves/>.

At Texture zero the onset spacing is exactly equal. At higher Texture the long-short alternation becomes increasingly obvious while the total duration of every two-step pair remains constant, so there is no cumulative tempo drift.

## 9. Polymeter: equal subdivision, unequal loop lengths

Polymeter is used here in the strict structural sense: both component cycles share the same subdivision duration, but their cycle lengths differ. They therefore realign only after the least common multiple of their lengths.

The primary cycle is fixed at four sixteenth-note steps. Texture selects the secondary length

$$
m\in\{3,5,7,9\}
$$

in four stable control regions.

On global sixteenth step $n$, define

$$
A_n=[n\bmod4=0],
$$

$$
B_n=[n\bmod m=0].
$$

Every sixteenth launches a short envelope with a base amplitude. $A_n$ and $B_n$ add accent weight; coincident cycle starts produce the strongest accent. The composite pattern repeats after

$$
P=\operatorname{lcm}(4,m),
$$

which gives 12, 20, 28 or 36 sixteenth steps for the four Texture regions.

The algorithm is intentionally deterministic. It is not a Euclidean rhythm generator, a probability gate or a polyrhythm in which different subdivision durations are squeezed into the same bar.

## 10. Duplication audit

| Existing/proposed algorithm | Why Electronica does not duplicate it |
|---|---|
| LFO | Pump has an intentional beat-boundary reset; Shuffle changes event spacing; neither is a continuously periodic waveform family |
| Bézier | Electronica timing is grid/tempo-derived rather than random endpoint interpolation |
| Rain | Rain is stochastic continuous-time event generation; Electronica events are deterministic on an internal musical grid |
| Turing | Acid repeats a deterministic contour and never mutates its stored phrase |
| Motif | Acid has no phrase-edit operations; its 16-step grammar is fixed by formula |
| Breath | Breath is a smooth baseline→peak→baseline gesture with cycle variation; Pump is asymmetric reset→recovery ducking |
| Current | Electronica is explicitly subdivision/grid based rather than quasi-periodic long-form movement |
| Fog | Fog is stochastic overlapping bipolar cloudlets, not rhythmic grid modulation |
| planned Percussion / Euclid | Polymeter uses simultaneous periodic accent cycles, not even-distribution hit placement |
| planned Percussion / Chance | Electronica is deterministic; no per-step hit probability |
| planned Percussion / Burst | no ratchet/multi-trigger generation |
| planned Percussion / Humanize | Shuffle is deterministic long-short timing, not random microtiming |

The nearest boundaries that require regression tests are Pump↔Breath/LFO, Acid↔Motif, Shuffle↔Humanize and Polymeter↔Euclid.

## 11. Musical assessment

| Algorithm | Musical value | Strongest musical role | Main limitation |
|---|---|---|---|
| **Pump** | very high | immediate house/techno ducking motion for VCA, filter, send or spectral position | free-running; no actual sidechain trigger |
| **Acid** | high | squelchy stepped/slide modulation for filters, wavefolders and oscillator timbre | fixed 16-step grammar can become recognizable/repetitive |
| **Shuffle** | very high | deterministic swing feel without random timing instability | most useful when its free-running tempo is manually aligned to the patch |
| **Polymeter** | very high | long accent cycles and evolving techno modulation from minimal deterministic state | Texture is intentionally stepped between meter choices |

As a bank, Electronica should feel **rhythmic before it feels random**. Its value comes from immediately patchable movement that suggests arrangement, groove and pulse even when Drift is the only timing source.

## 12. ATmega328P feasibility

All four contracts are small relative to the existing Organic/Ambient worst cases:

- Pump: one beat phase and one smooth recovery evaluation per sample;
- Acid: one sixteenth phase, one 4-bit step index, fixed formulas/masks and at most one smooth interpolation/accent envelope;
- Shuffle: one pair phase plus one active decay contour;
- Polymeter: one global step counter and integer modulo against 4 and one small odd meter.

No heap allocation is required. All state is fixed-size. The proposed tempo range keeps the shortest musical subdivision comfortably above the 2.5 kHz scheduler resolution.

The implementation should still receive AVR timing-probe qualification, especially if integer division/modulo remains in Acid or Polymeter hot paths. Since the meter values are fixed small constants, those operations can likely be replaced with counters and comparisons.

## 13. Implementation decision gate

Before coding, the following must be treated as explicit design decisions rather than silently improvised implementation details:

1. confirm the 30..240 BPM nominal Speed range by listening and patch use;
2. confirm Pump's recovery endpoint range of 1/4..15/16 beat;
3. audition Acid's deterministic level permutation and its project-defined accent/slide masks;
4. confirm Shuffle's maximum 3:1 interval ratio is useful rather than excessive;
5. confirm the Polymeter meter set 3/5/7/9 and stepped Texture behavior;
6. preserve the no-external-sync limitation in user documentation.
<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../../assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
