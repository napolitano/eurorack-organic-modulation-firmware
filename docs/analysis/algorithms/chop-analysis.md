# Chop algorithm engineering analysis

## 1. Purpose and scope

> **Implementation status — released in 0.3.0:** Implemented under `domain/dubstep/` with fixed anchors `{0,8}`, the documented ordered candidate set, bar-boundary Texture latching and a half-step hold / half-step linear decay articulation. It reuses the shared 0–5 V quarter-note clock source.


Chop is the third mode of the released Dubstep/Bass bank. It creates a deterministic, tempo-synchronised **sparse/syncopated articulation CV** for chopping sustained basses, drones or effects.

Unlike Percussion Probability, Chop does not randomise whether a hit exists. Unlike Electronica Acid, it does not sequence a set of melodic modulation levels with accents and slides. Its identity is **rhythmic presence versus space** on a 16-step bar.

There is no Quinn Freedman Chop mode to preserve.

## 2. Musical basis

Sound On Sound identifies swing, syncopation, half-step rhythm and deliberate low-frequency space as central dubstep production concerns:

<https://www.soundonsound.com/techniques/dubstep-basics>

A historical study of the genre describes the mid-2000s move toward a half-step groove at around 140 BPM and explicitly links the larger spaces between drum events with room for longer/louder sub-bass lines:

<https://research-information.bris.ac.uk/ws/portalfiles/portal/390601980/Final_Copy_2024_03_08_Mouraviev_IN_PhD.pdf>

More generally, syncopation can be defined as an event on a weak metric position followed by absence at an expected stronger position. That formal relationship is useful for designing a controlled syncopation macro rather than merely sprinkling arbitrary off-grid events:

<https://pmc.ncbi.nlm.nih.gov/articles/PMC10911290/>

The exact Drift onset pattern remains project-defined; there is no single canonical dubstep bass rhythm.

## 3. Mathematical foundations

Use a fixed 16-step bar with step index

$$
s\in\{0,\ldots,15\}.
$$

Two structural anchors are always present:

$$
A=\{0,8\}.
$$

These establish one articulation at the beginning of each half-bar while leaving substantial space.

Define an ordered set of additional onset candidates

$$
C=(3,11,6,14,2,10,7,15).
$$

The first two candidates are one sixteenth before quarter-note boundaries 4 and 12, creating explicit anticipatory syncopation. Later candidates increase density and introduce additional weak-position events.

For saturated Texture code $T\in[0,1023]$, define

$$
k(T)=\left\lfloor\frac{9T}{1024}\right\rfloor,
$$

with saturation at

$$
k\in\{0,\ldots,8\}.
$$

The active onset set is

$$
E(T)=A\cup\{C_0,\ldots,C_{k-1}\}.
$$

Therefore onset count is exactly

$$
|E|=2+k.
$$

It increases monotonically from two to ten events per bar.

## 4. Articulation contour

Chop does not emit the same fixed 10 ms pulse as the Percussion bank. It is intended to articulate sustained audio through a VCA/filter, so the event should occupy a musically meaningful fraction of the sixteenth.

Let local step phase be $x\in[0,1)$. The released unipolar gate-like contour is

$$
C(x)=
\begin{cases}
1,&0\le x<\frac12\\
2-2x,&\frac12\le x<1.
\end{cases}
$$

Thus each active step holds full scale for the first half of the sixteenth and then decays linearly to zero by the next step. Inactive steps output zero.

This avoids a narrow 10 ms trigger and gives each onset a musically substantial articulation window. It **does not** remove the hard 0-to-full-scale transition at an active step boundary, so direct VCA patches can still click; that remains a documented hardware-listening consideration.

No attack ramp is present in 0.3.0. Adding one later would change the released articulation contour and therefore requires an explicit behavior/documentation update rather than being treated as an implementation detail.

## 5. Texture behavior

Texture is intentionally **monotone but stepped**. Increasing Texture never removes an already-active onset; it only adds the next candidate.

This gives the performer a predictable progression:

- minimum: two sparse half-bar anchors;
- low: anticipations before strong quarter boundaries;
- medium: additional off-beat movement;
- high: denser chopped sixteenth articulation while retaining rests.

The mode does not become a full 16th-note gate even at maximum Texture; six steps remain silent.

## 6. Clock-source contract

Chop uses the shared `clock::ClockSource` behavior. Every accepted quarter note defines four sixteenth subdivisions.

On external lock, the second accepted quarter edge establishes step 0 / bar origin. Clock loss falls back to the Speed knob while preserving running bar position; later re-lock establishes a fresh deterministic bar origin.

The current hardware remains limited to **0..5 V clocks only** on Speed CV.

## 7. Reference algorithm

At each processing sample:

1. obtain internal or externally measured quarter duration;
2. derive/advance sixteenth phase while preserving overshoot;
3. increment the 0..15 step counter at each sixteenth boundary;
4. latch Texture-derived $k$ at the next bar boundary;
5. build or select the corresponding 16-bit onset mask;
6. if the current step is inactive, output zero;
7. if active, evaluate the gate/decay contour from local step phase;
8. scale to 0..4095.

No RNG is required.

## 8. Efficient mask representation

Texture has only nine possible onset counts ($k=0..8$). The released implementation constructs the 16-bit mask from the eight-entry candidate list when the bar-latched Texture value changes; it does not store a nine-mask lookup table.

The reference mask for each $k$ is simply

$$
M_k=A\cup\{C_0,\ldots,C_{k-1}\}.
$$

The resulting mask is then reused for per-sample membership tests, so the hot path remains a single bit test while the small reconstruction cost occurs only when a new bar-latched density is applied.

## 9. Duplication boundaries

### Versus Percussion Probability

Probability uses stochastic hit decisions and a metric hierarchy. Chop is deterministic and Texture selects a reproducible articulation mask.

### Versus Euclid

Euclid aims to distribute $k$ events as evenly as possible. Chop intentionally uses uneven, syncopated positions and fixed anchors.

### Versus Acid

Acid's information is primarily **level/accent/slide** on a deterministic 16-step modulation contour. Chop's information is primarily **sound/rest articulation**. It has no pitch-like level sequence and no slide grammar.

### Versus Motif

Chop has no phrase memory, mutations or edit operations. The pattern is a direct function of Texture.

## 10. Findings and design risks

- **Musical strength:** the mode exposes negative space and syncopation rather than another continuous curve.
- **Deterministic contract:** the released implementation consumes no RNG.
- **Boundary contract:** Texture changes latch at bar boundaries to prevent pattern tearing.
- **Pattern risk:** the candidate order is intentionally project-defined and may sound too mechanical or stylistically narrow.
- **Output risk:** a gate-like CV may click when patched directly to some VCAs; edge shaping must be listening-tested.
- **Naming risk:** “Chop” is descriptive but generic. That is preferable to falsely claiming a historically canonical bass phrase.

## 11. Computational cost on ATmega328P

Per sample:

- shared grid phase update;
- 16-bit mask test;
- local contour evaluation only on active steps;
- one scale to DAC code.

State is tiny: sixteenth phase, step index, latched mask and Texture region. No division or RNG is required in the hot path if the local contour uses phase bits directly.

## 12. Verification and test strategy

Required tests:

- $k(T)$ covers exactly 0..8 with documented ADC boundaries;
- maximum Texture saturates to $k=8$;
- anchor steps 0 and 8 are present for every Texture;
- onset count is exactly $2+k$;
- every successive Texture region is a strict superset of the previous mask;
- no mask contains duplicate/out-of-range positions;
- golden masks match the documented candidate order;
- step 3 precedes strong step 4 and step 11 precedes strong step 12 as designed;
- active-step contour is exactly 4095 during the first half and reaches zero at step end within fixed-point quantisation;
- inactive steps are exactly zero;
- Texture changes mid-bar take effect only at the next bar boundary;
- one bar remains exactly 16 sixteenths under internal and external timing;
- no RNG state is consumed;
- internal/external sixteenth timing remains bounded and external quarter edges re-anchor the local grid;
- AVR timing remains below 400 microseconds.

A hardware test should patch Chop directly to both a linear VCA and a filter cutoff and listen specifically for objectionable edge clicks.

## 13. Musical assessment

**Musical value: very high.**

Chop gives the bank a fundamentally different output type from Wobble/Growl. It can turn a sustained oscillator or drone into a bass phrase without requiring a sequencer or envelope generator.

Useful destinations include:

- VCA level;
- filter cutoff;
- LPG control;
- wavefolder enable/depth;
- delay/reverb send chopping;
- sample/switch CV inputs that tolerate continuous gate-like levels.

The sparse low-Texture states are particularly important. The mode should preserve space rather than treating maximum event density as automatically more “dubstep”.

## 14. Engineering assessment

Chop is extremely cheap and highly testable. The candidate order is frozen 0.3.0 behavior and covered by an exact mask-order test. Its remaining uncertainty is musical rather than technical: later listening may justify a deliberately versioned phrase-grammar change, but the current release contract is no longer provisional.

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../../assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
