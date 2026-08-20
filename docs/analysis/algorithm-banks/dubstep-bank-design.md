# Dubstep / bass-music bank architecture and control contract

## 1. Purpose and scope

> **Implementation status — released in 0.3.0:** The bank is implemented as compile-time bank `FMD_BANK_DUBSTEP` with slots Wobble / Growl / Chop / Build. Release 0.3.0 freezes the 70–280 BPM mapping, reuses the shared Percussion-derived `ClockSource`, uses the 8/4/2/1-bar Build mapping described below, and publishes the user-facing bank name **Dubstep / Bass**; the code identifier remains `dubstep`.


This document evaluates a proposed seventh Drift algorithm bank with the working name **Dubstep**. The bank is intended to cover a part of electronic modulation that the existing six banks do not address directly: **tempo-synchronised bass-motion phrases, syncopated gating and build/drop-scale modulation**.

The working four-mode set is:

- **Wobble** — a tempo-synchronised modulation oscillator whose rate changes according to a deterministic musical phrase;
- **Growl** — a beat-synchronised, multi-lobed timbral-motion contour intended for wavetable, formant, filter, FM or waveshaping destinations;
- **Chop** — a deterministic sparse/syncopated gate-CV phrase for rhythmically chopping a sustained bass or modulation destination;
- **Build** — a repeating multi-bar tension contour that combines a macro rise with progressively faster tempo-locked modulation before a phrase reset.

There is no Quinn Freedman implementation of these modes. The complete bank would be project-defined.

The bank name is deliberately **not frozen by this analysis**. “Dubstep” is immediately understandable, but risks reducing a broad genre to the later wobble/growl/drop vocabulary. Sound On Sound explicitly cautions that dubstep is more than an automated wobble bass, while historical work describes the genre's earlier half-step, sub-bass and spatial emphasis. A public-facing name such as **Bass**, **Bass Music** or **Dubstep / Bass** may therefore achieve better genre accuracy and community acceptance without changing the algorithms themselves:

- <https://www.soundonsound.com/techniques/dubstep-basics>
- <https://research-information.bris.ac.uk/ws/portalfiles/portal/390601980/Final_Copy_2024_03_08_Mouraviev_IN_PhD.pdf>

## 2. Genre evidence relevant to the firmware contract

Ableton's Learning Music material gives a typical Dubstep tempo range of **135..145 BPM**:

<https://learningmusic.ableton.com/make-beats/tempo-and-genre.html>

Sound On Sound describes the genre as typically around 140 BPM, with heavy sub-bass, swing/syncopation and a prominent half-step feel. The same article describes the classic wobble mechanism as an LFO modulating filter cutoff and notes that synchronising LFO speed to track tempo often works best:

<https://www.soundonsound.com/techniques/dubstep-basics>

Native Instruments gives a concrete modern production example using a tempo-synchronised **3/16** LFO ratio and calls the result a classic dubstep wobble. The same tutorial uses a two-bar bass groove with a small second-pass timing variation:

<https://blog.native-instruments.com/how-to-make-dubstep/>

Jeremy W. Smith's research on continuous processes in EDM identifies wobble bass as a short characteristic continuous process in dubstep and also notes that intense risers are common in bass genres, often over shorter two- or four-bar spans:

<https://doi.org/10.12801/1947-5403.2024.16.01.06>

These references support a bank centred on tempo-locked modulation and phrase-scale motion. They do **not** support treating one specific wobble pattern, growl shape or build phrase as canonical. Those details must remain documented project design choices.

## 3. Proposed bank slot mapping

| Slot | DIP 1 | DIP 2 | Proposed algorithm |
|---:|---|---|---|
| 0 | OFF | OFF | Wobble |
| 1 | ON | OFF | Growl |
| 2 | OFF | ON | Chop |
| 3 | ON | ON | Build |

As with the existing banks, the normal firmware image would contain all four modes and the rear DIP switches would select one at startup. The named-algorithm developer target mechanism introduced in 0.3.0 provides direct on-device builds for `wobble`, `growl`, `chop` and `build` without changing the user-facing bank contract.

## 4. Proposed shared tempo contract

A Dubstep-specific internal tempo mapping should make the genre's centre of gravity easy to reach rather than reusing the very broad 30..240 BPM Electronica/Percussion range uncritically.

A useful candidate is

$$
B(u)=70\cdot2^{2u}\ \mathrm{BPM},\qquad u\in[0,1].
$$

This gives

$$
70\le B\le280\ \mathrm{BPM}
$$

with the logarithmic midpoint exactly at

$$
B\left(\frac12\right)=140\ \mathrm{BPM}.
$$

This is attractive for three reasons:

1. 140 BPM sits at the physical centre of the Speed control;
2. the two-octave span covers half-time and double-time interpretations around that centre;
3. the narrower span provides better manual resolution around the typical 135..145 BPM region than the current three-octave Electronica mapping.

This mapping is **a design recommendation, not yet an implementation contract**. Listening/on-device tests should compare it with reusing the established 30..240 BPM mapping before firmware is frozen.

## 5. External clock recommendation

The proposed bank benefits enough from transport-relative timing that it should reuse the already qualified Percussion clock-source model:

- Speed CV is repurposed as an optional **0..5 V quarter-note clock input**;
- Speed knob supplies internal tempo when there is no valid external lock;
- two valid rising edges acquire timing;
- clock loss after more than 2.5 measured periods falls back to the knob without resetting running phrase state;
- a later re-lock establishes a fresh deterministic phrase origin.

This reuse avoids inventing a second clock detector and gives Wobble, Chop and Build deterministic musical boundaries.

> **Current-hardware electrical limitation:** the original Speed CV input is not a protected general-purpose Eurorack trigger input. Any proposed Dubstep/Bass clock feature must retain the existing **0..5 V only** restriction. Raw 10 V Eurorack clocks/triggers remain unsupported until the analogue input stage is revised.

## 6. Shared control contract

| Algorithm | Speed | Texture | Texture CV | Analog Attenuation |
|---|---|---|---|---|
| **Wobble** | internal tempo / external quarter clock | rate-phrase complexity | increases/decreases phrase vocabulary | final modulation depth |
| **Growl** | internal tempo / external quarter clock | higher-order contour complexity | morphs timbral-motion shape | final modulation depth |
| **Chop** | internal tempo / external quarter clock | onset density + syncopation | moves from sparse to busier chops | final gate/CV level |
| **Build** | internal tempo / external quarter clock | phrase length / escalation intensity | shorter, more intense builds | final modulation depth |

Texture remains a saturated 10-bit firmware macro. Attenuation remains analogue after the DAC and therefore should not be consumed as an internal algorithm parameter.

## 7. Wobble concept

Wobble should **not** become “another LFO shape selector”. Its differentiator is a tempo-locked **rate phrase**.

A fixed unipolar triangle carrier is sufficient:

$$
W(\phi)=1-\left|2\phi-1\right|,\qquad \phi\in[0,1).
$$

The phase remains continuous while its rate changes only at musically defined phrase boundaries. Candidate rate ratios, expressed as cycles per quarter note, are

$$
R=\left\{\frac12,\frac23,1,\frac43,\frac32,2,3,4\right\}.
$$

These correspond to familiar tempo-relative periods including half note, dotted quarter, quarter, 3/16, quarter-note triplet, eighth, eighth-note triplet and sixteenth. The Native Instruments 3/16 example provides direct evidence that non-power-of-two ratios belong in the useful vocabulary.

Texture should select increasingly complex **project-defined rate phrases**, not random rates. The phrase set must be original rather than copied from a specific commercial track.

## 8. Growl concept

“Growl” is the most semantically risky candidate because Drift outputs CV, not audio. A growl bass in production normally depends on timbral synthesis and processing — wavetable position, FM, formant filtering, distortion and related movement — none of which Drift can create by itself. A user patch can, however, use a sufficiently structured CV contour to drive those destinations.

The proposed scalar approximation is a weighted sum of phase-related triangle components:

$$
T(x)=1-\left|2\,\mathrm{frac}(x)-1\right|,
$$

$$
G(\phi,\tau)=
\frac{
T(\phi)+a(\tau)T(2\phi+\tfrac14)+b(\tau)T(3\phi+\tfrac18)
}{1+a(\tau)+b(\tau)},
$$

with

$$
a(\tau)=\frac34\tau,
$$

$$
b(\tau)=\frac12\tau^2.
$$

Because all terms lie in $[0,1]$ and the expression is a weighted average,

$$
0\le G\le1
$$

without clipping. Low Texture is a simple one-lobe movement; increasing Texture introduces additional lobes and asymmetry suitable for timbral destinations.

The public name may need to become **Formant**, **Snarl** or **Talk** if listening tests show that “Growl” overpromises what a single CV output can cause.

## 9. Chop concept

Chop targets the sparse, syncopated rhythmic space left around a half-time bass framework. It is intentionally deterministic so it does not duplicate Percussion Probability or Generative Markov/Motif.

The initial model uses a 16-step bar. Two structural anchors remain fixed:

$$
A=\{0,8\}.
$$

Texture progressively adds candidate onset positions from an ordered project-defined set

$$
C=(3,11,6,14,2,10,7,15).
$$

For saturated Texture code $T\in[0,1023]$, let

$$
k(T)=\left\lfloor\frac{9T}{1024}\right\rfloor,
$$

so

$$
k\in\{0,\ldots,8\}.
$$

The active onset set is

$$
E(T)=A\cup\{C_0,\ldots,C_{k-1}\}.
$$

Positions 3 and 11 deliberately anticipate strong quarter-note boundaries by one sixteenth; additional positions increase density without converting the mode into an even subdivision clock.

Each onset should launch a short gate-like contour whose duration is a fixed fraction of a sixteenth rather than a 10 ms Percussion trigger. That distinction lets Chop act as VCA/filter articulation on sustained material rather than merely as another trigger sequencer.

The exact candidate order is a musical project parameter and must be listening-tested. Its purpose is to define an original deterministic grammar, not to claim a canonical dubstep bassline.

## 10. Build concept

Build operates at the largest time scale in the bank. It combines a macro rise with a micro modulation rate that doubles toward the phrase end.

Let normalized phrase phase be

$$
u\in[0,1).
$$

A candidate macro contour is cubic smoothstep

$$
M(u)=3u^2-2u^3.
$$

Divide the phrase into four equal stages. The micro modulation uses the following subdivisions:

| Phrase quarter | Micro period |
|---:|---|
| 1 | quarter note |
| 2 | eighth note |
| 3 | sixteenth note |
| 4 | thirty-second note |

This mirrors the established dance-music build technique of progressively halving note values before a drop, without generating an actual snare roll. MusicRadar describes quarter/eighth/sixteenth/thirty-second acceleration as a common build/drop device:

<https://www.musicradar.com/how-to/8-classic-recipes-for-a-perfect-drop>

If $Q(u)$ is a unipolar triangle at the current stage rate, a useful composite CV is

$$
Y(u)=M(u)\left(\frac14+\frac34Q(u)\right).
$$

The macro rise therefore opens the destination over the whole phrase while the micro motion becomes progressively faster. At phrase end the output resets for the next cycle; that reset is intentional and can act as the release/drop marker.

Texture may select phrase length from

$$
N\in\{8,4,2,1\}\ \text{bars}
$$

with higher Texture producing shorter, more urgent cycles. The one-bar endpoint is useful for aggressive performance but is no longer an arrangement-scale build; listening tests should determine whether the minimum should instead remain two bars.

## 11. Duplication audit

| Existing algorithm | Proposed-bank boundary |
|---|---|
| Classic LFO | Wobble has one fixed carrier shape and derives its identity from deterministic tempo-quantised **rate phrases**; Growl is a fixed multi-component beat contour rather than a waveform family |
| Bézier / Organic Vector | no random endpoints or vector-field motion; all four modes are explicitly grid/phrase relative |
| Generative Markov / Motif | no evolving stored phrase, state grammar or mutation memory |
| Ambient Breath | Growl is short, beat-relative and multi-lobed rather than a slow stochastic recurrent swell |
| Electronica Pump | Build rises across bars; Pump ducks and recovers on every beat |
| Electronica Acid | Chop controls rhythmic articulation/rest structure rather than 16-step levels, accents and slides |
| Electronica Shuffle | no long/short timing deformation |
| Electronica Polymeter | no unequal integer cycle lengths |
| Percussion Euclid | Chop does not evenly distribute a requested hit count |
| Percussion Repeat | Build accelerates a continuous modulation carrier; it does not schedule ratchet trigger clusters around anchors |
| Percussion Probability | Chop is deterministic and consumes no RNG |
| Percussion Humanize | no microtiming or pulse-amplitude randomisation |

The main overlap risks are Wobble vs Classic LFO and Build vs Percussion Repeat. These boundaries must remain explicit in implementation and documentation.

## 12. ATmega328P feasibility

All four proposed modes fit the current processing model comfortably in principle:

- no FFT, audio-rate synthesis or dynamic allocation;
- Wobble needs one phase accumulator, phrase position and rate-table lookup;
- Growl needs three triangle evaluations and fixed-point weighted sums;
- Chop needs one 16-step counter, an onset mask and a gate/envelope phase;
- Build needs phrase counters, one stage-rate selector, one micro phase and one smoothstep macro phase.

At the proposed maximum internal tempo of 280 BPM, a 32nd-note period is approximately

$$
\frac{60}{280\cdot8}\approx26.8\ \mathrm{ms},
$$

which is about 67 processing samples at 2.5 kHz. This remains comfortably resolvable by the existing scheduler.

The exact timing budget must still be measured on AVR. The analysis only establishes that no proposed mathematical primitive obviously exceeds the architecture's capabilities.

## 13. Verification strategy

Before implementation is accepted, the bank requires:

### Shared timing

- exact internal tempo endpoints and midpoint;
- monotonic Speed mapping;
- 140 BPM exactly or to the documented fixed-point quantisation at midpoint if the 70..280 mapping is adopted;
- external-clock acquisition/loss/re-lock behavior identical to the established contract;
- no phrase drift from discarded phase overshoot;
- 0..5 V clock safety wording present in every user-facing location that exposes the feature.

### Wobble

- every rate token maps to the intended rational beat period;
- phase remains continuous at rate changes;
- phrase repeats exactly at its documented length;
- Texture changes only at safe phrase/rate boundaries;
- no RNG consumption;
- output always 0..4095.

### Growl

- exact $[0,1]$ / 0..4095 boundedness over dense phase/Texture sweeps;
- continuity at component phase wraps;
- increasing Texture increases higher-component contribution monotonically;
- deterministic recurrence and no RNG;
- fixed-point output matches a floating reference within an explicit tolerance.

### Chop

- anchor positions 0 and 8 are never removed;
- Texture adds candidate onsets monotonically and never duplicates an onset;
- each Texture region has an exact golden mask;
- onset/gate timing stays on the sixteenth grid;
- no stochastic decisions;
- gate width cannot overlap the next onset at maximum tempo unless overlap is explicitly part of the contract.

### Build

- phrase lengths and stage transitions occur at exact bar boundaries;
- micro rate follows the quarter/eighth/sixteenth/thirty-second sequence exactly;
- macro contour is monotone non-decreasing inside one phrase;
- phrase-end reset is exact and does not shift the next phrase origin;
- maximum-tempo 32nd modulation remains within scheduler timing limits.

## 14. Musical and product assessment

The proposed bank fills a genuine gap. Existing Drift modes can certainly be patched into bass music, but none currently treats **bass-modulation phrasing itself** as the primary domain.

Provisional value assessment:

| Algorithm | Musical value | Design confidence | Main risk |
|---|---|---|---|
| **Wobble** | very high | high | must remain meaningfully different from Classic LFO |
| **Growl** | high if patched to an appropriate timbral destination | medium | name may overpromise; scalar CV cannot create a growl alone |
| **Chop** | very high | high | project-defined phrase grammar requires listening validation |
| **Build** | high | medium-high | generic EDM technique; needs enough bass-music character to justify the slot |

The strongest candidates are Wobble and Chop. Growl is technically feasible but needs semantic/listening validation. Build is musically useful, but its genre specificity is weaker than Wobble/Chop and should be compared against at least one alternative fourth mode before implementation is frozen.

## 15. Recommendation

Proceed to prototype only after the following analysis decisions are accepted:

1. keep the four working concepts **Wobble / Growl / Chop / Build** for initial hardware listening;
2. treat **Dubstep** as a working bank name, not yet a final public label;
3. reuse the qualified Percussion 0..5 V external-clock architecture;
4. prototype the 70..280 BPM logarithmic mapping with 140 BPM at centre and compare it on-device against the existing 30..240 mapping;
5. keep Wobble and Chop deterministic in the first implementation so the bank has reproducible phrase identity rather than becoming another random/generative bank;
6. do not market Growl as audio synthesis — it is a **timbral-motion CV source**;
7. require listening tests before freezing rate phrases, Chop onset order, Growl component weights or Build phrase lengths.

The bank is therefore **technically viable and musically justified**, but not yet implementation-frozen.

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../../assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
