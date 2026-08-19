# Percussion bank architecture and control contract

## 1. Purpose

The proposed **Percussion** bank is intended to turn Drift into a compact source of internally timed rhythmic control voltage for drum voices, percussion modules, samplers, envelopes and other triggerable destinations.

The bank must not duplicate Electronica. Electronica shapes continuous or stepped CV around club-music timing concepts; Percussion instead treats the DAC output primarily as a **pulse/event stream**. The existing hardware still provides only one analogue output and only one active algorithm at a time, so this is not a multi-track drum sequencer. One Percussion algorithm generates one rhythmic stream; the musical role — kick, snare, hi-hat, shaker, auxiliary percussion or modulation pulse — is determined by the chosen algorithm, Texture setting and patch destination.

The proposed modes are:

- **Euclid** — a 16-step Euclidean rhythm with phrase-aware deterministic fills;
- **Repeat** — quarter-note anchor events with probabilistic ratchet clusters and phrase-end repeat fills;
- **Probability** — a metrically weighted stochastic 16-step pulse pattern with phrase-aware density boosts;
- **Humanize** — a stable eighth-note pulse train with bounded random microtiming and amplitude variation.

There is no Quinn Freedman implementation of these four modes. The complete bank is project-defined.

## 2. Bank slot mapping

| Slot | DIP 1 | DIP 2 | Percussion algorithm |
|---:|---|---|---|
| 0 | OFF | OFF | Euclid |
| 1 | ON | OFF | Repeat |
| 2 | OFF | ON | Probability |
| 3 | ON | ON | Humanize |

The bank itself is selected at build/flash time. The rear DIP switches select one of these four algorithms at startup, exactly as in the other banks.

## 3. Shared tempo and pulse contract

Percussion uses the same nominal tempo scale already defined for Electronica. With normalized combined Speed control $u\in[0,1]$,

$$
B(u)=30\cdot 2^{3u}\ \text{BPM}.
$$

This gives

$$
30\ \text{BPM}\le B\le240\ \text{BPM}.
$$

The bank uses a 4/4-oriented internal grid with sixteen sixteenth-note steps per bar. The sixteenth-note frequency is

$$
f_{16}=4\frac{B}{60}.
$$

At the 240 BPM endpoint this is 16 Hz, leaving approximately 156 processing samples per sixteenth at Drift's 2.5 kHz scheduler rate.

Euclid, Repeat and Probability output full-scale pulse events. The initial pulse-width contract is fixed at **10 ms**, corresponding to 25 processing samples at 2.5 kHz. Fixed-duration pulses are preferred to tempo-relative gates because they remain trigger-like across the whole 30..240 BPM range. The analogue Attenuation control remains available after the DAC for reducing external pulse/CV level.

Humanize uses the same 10 ms pulse duration but intentionally varies pulse amplitude as part of its musical contract.

### External synchronization limitation

The current Drift hardware has no dedicated clock or reset input. Percussion is therefore **free-running**. The firmware can count its own steps, bars and phrases exactly, but it cannot know where bar 1 of an external sequencer, DAW or modular clock begins.

Consequently, wording such as "fill every eight bars" means every eight internally generated Drift bars after the algorithm starts. It must not be documented as transport-synchronized behavior.

## 4. Shared phrase engine

The main extension beyond a simple trigger generator is a common phrase hierarchy:

```text
pulse/substep -> step -> 16-step bar -> 4/8/12/16-bar phrase -> fill bar
```

The phrase engine maintains:

- sixteenth-step index $s\in\{0,\ldots,15\}$;
- bar index $b$ inside the current phrase;
- latched phrase length $N\in\{4,8,12,16\}$;
- `isFillBar`, true only for the final bar of the phrase.

Texture is a single macro control, so phrase length cannot be independently adjusted. The design therefore couples increasing Texture with increasing phrase activity and more frequent fills. For saturated 10-bit Texture $T\in[0,1023]$,

$$
N(T)=
\begin{cases}
16,&0\le T<256\\
12,&256\le T<512\\
8,&512\le T<768\\
4,&768\le T\le1023.
\end{cases}
$$

Phrase length is **latched only at the start of a phrase**. A Texture/CV change during an active phrase therefore does not suddenly move or cancel the upcoming fill. The new phrase length takes effect when the next phrase starts.

A shared fill-strength level is

$$
F(T)=\left\lfloor\frac{5T}{1024}\right\rfloor,
$$

so

$$
F\in\{0,1,2,3,4\}.
$$

Euclid, Repeat and Probability interpret $F$ differently. Humanize intentionally ignores fill state because changing event count would contradict its role as a timing/velocity humanizer.

The phrase counter starts from zero when the algorithm starts. There is no external reset source on current hardware.

## 5. Shared control contract

| Algorithm | Speed | Texture | Phrase behavior | Analog Attenuation |
|---|---|---|---|---|
| **Euclid** | nominal tempo | Euclidean hit density; also shortens phrase and strengthens fill | deterministic tail fill on final bar | final pulse level |
| **Repeat** | nominal tempo | repeat probability and ratchet depth; also shortens phrase | forced repeat escalation near phrase end | final pulse level |
| **Probability** | nominal tempo | probability of secondary/ghost hits; also shortens phrase | optional-hit probabilities increase on final bar | final pulse level |
| **Humanize** | nominal tempo | microtiming + pulse-amplitude deviation | no fill; event count remains fixed | final pulse level |

Texture knob and Texture CV are summed and saturated in the normal 10-bit Drift control domain. For the first three algorithms, rhythm parameters are latched at musically safe boundaries rather than allowing mid-step pattern tearing.

## 6. Euclid: evenly distributed rhythm with phrase-end fill

Godfried Toussaint's 2005 paper describes **Euclidean rhythms** $E(k,n)$ as binary onset patterns with $k$ attacks distributed as evenly as possible over $n$ time intervals, relating Bjorklund's sequence-generation algorithm to the structure of the Euclidean algorithm: <https://cgm.cs.mcgill.ca/~godfried/publications/banff.pdf>.

Drift fixes

$$
n=16
$$

and lets Texture choose

$$
k(T)=2+\left\lfloor\frac{12T}{1024}\right\rfloor,
$$

which yields

$$
k\in\{2,\ldots,13\}.
$$

The deliberately restricted range avoids the musically degenerate all-rest/all-hit endpoints while covering sparse kick/snare-like figures through dense hi-hat/percussion patterns.

The AVR implementation should not run Bjorklund's algorithm in the 2.5 kHz hot path. Canonical 16-bit masks for every supported $k$ should be generated and verified offline, stored in flash, and rotated so step 0 is an onset.

### Euclid fill transformation

On normal bars the canonical $E(k,16)$ mask is emitted unchanged. On the final phrase bar, the base mask is ORed with an additive final-quarter tail according to $F$:

| Fill level | Additional tail candidates |
|---:|---|
| 0 | none |
| 1 | step 15 |
| 2 | steps 14, 15 |
| 3 | steps 13, 14, 15 |
| 4 | steps 12, 13, 14, 15 |

Existing Euclidean hits are never removed. The fill therefore becomes a deterministic densification of the current groove rather than an unrelated random pattern.

## 7. Repeat: ratchets, flams and short rolls

Repeat keeps an unambiguous anchor structure: one guaranteed event at every quarter-note boundary. Texture does **not** decide whether the anchor exists. It decides whether the anchor sprouts a short repeated cluster.

Let normalized Texture be $\tau$. On each non-forced anchor, a ratchet is enabled with probability

$$
p_r=\frac{3}{4}\tau.
$$

If no ratchet is selected, the anchor produces one pulse. If a ratchet is selected, the total pulse count is

$$
r(T)=2+\left\lfloor\frac{3T}{1024}\right\rfloor,
$$

so

$$
r\in\{2,3,4\}.
$$

The $r$ pulses are distributed evenly across the first half of the quarter note. If quarter duration is $Q$, cluster window $W=Q/2$ and pulse index $j\in\{0,\ldots,r-1\}$,

$$
t_j=\frac{jW}{r}.
$$

At 240 BPM and $r=4$, adjacent repeats remain about 31.25 ms apart, comfortably longer than the fixed 10 ms output pulse.

### Repeat fill transformation

The final bar of the phrase forces stronger repeats at the tail:

- $F=0$: no forced repeat;
- $F=1$: final quarter is at least a double;
- $F=2$: final quarter is at least a triple;
- $F=3$: final quarter is a four-pulse ratchet;
- $F=4$: both the third and fourth quarters are forced four-pulse ratchets.

Normal-bar probabilistic repeats still occur between phrase endings, providing the requested intermittent stutters. Phrase-end forcing adds a legible arrangement-scale fill without inventing another front-panel control.

## 8. Probability: metrically weighted stochastic pattern

Probability uses a fixed 16-step metric hierarchy rather than assigning the same probability to every sixteenth. This is deliberate: completely independent equal-probability hits tend to erase the distinction between strong beats, subdivisions and ghost positions.

The steps are divided into three classes:

- **primary:** 0, 4, 8, 12;
- **secondary:** 2, 6, 10, 14;
- **ghost:** 1, 3, 5, 7, 9, 11, 13, 15.

Primary steps always fire. With normalized Texture $\tau$,

$$
P(secondary)=\tau,
$$

and

$$
P(ghost)=\frac{\tau^2}{2}.
$$

Thus Texture zero is a stable four-on-the-floor skeleton. Increasing Texture first adds musically strong eighth-note subdivisions; weaker sixteenth-note ghosts grow more slowly and remain probabilistic even at maximum Texture.

### Probability fill transformation

On the final bar, let

$$
b=\frac{F}{8}.
$$

Optional-event probabilities become

$$
P' = \min(1,P+b),
$$

with an additional $+b$ boost on steps 12..15. The last quarter of a high-intensity fill can therefore become nearly or completely populated while the rest of the phrase remains recognizably related to the underlying metric hierarchy.

Every stochastic choice is made from the project's deterministic seeded RNG so host tests can reproduce exact event streams.

## 9. Humanize: bounded microtiming and pulse-level variation

Humanize deliberately does not add or remove events. Its neutral source pattern is a fixed eighth-note pulse train: steps 0, 2, 4, 6, 8, 10, 12 and 14 of every bar.

Ableton's Groove Pool documentation separates **random timing fluctuation** from **velocity** influence and notes that subtle timing randomization can humanize highly quantized electronic loops: <https://www.ableton.com/en/manual/using-grooves/>. Drift follows that conceptual separation internally, but one Texture macro scales both dimensions together because no additional control exists.

For each nominal event $n$,

$$
t_n=t_n^{0}+\delta_n,
$$

where the timing offset is independently drawn from

$$
\delta_n\in[-J(T),J(T)]
$$

with

$$
J(T)=\left\lfloor\frac{30T}{1023}\right\rfloor\ \text{samples}.
$$

At 2.5 kHz this is a maximum absolute offset of 12 ms.

Pulse amplitude is centered at DAC code 3840. With

$$
V(T)=\left\lfloor\frac{255T}{1023}\right\rfloor,
$$

an independent signed deviation $\epsilon_n\in[-V,V]$ gives

$$
A_n=\operatorname{clamp}(3840+\epsilon_n,0,4095).
$$

At maximum Texture the pulse therefore remains approximately 8.75..10 V before analogue attenuation, keeping it useful as a trigger while still exposing a velocity-like CV difference to compatible destinations.

The key timing invariant is that every event is positioned relative to its own **nominal grid location**, not relative to the previously jittered event. Jitter can therefore never accumulate into tempo drift.

Because the minimum eighth-note spacing at 240 BPM is 125 ms and the maximum pairwise timing contraction is 24 ms, event ordering is preserved by a wide margin.

## 10. Duplication audit

| Existing/proposed algorithm | Why Percussion does not duplicate it |
|---|---|
| Electronica / Polymeter | Polymeter combines deterministic meter cycles; Euclid distributes a chosen number of onsets evenly and adds phrase-level fills |
| Electronica / Shuffle | Shuffle deterministically warps long/short timing; Humanize uses bounded random offsets around an unchanged nominal grid |
| Electronica / Pump | Pump is a continuous duck/recovery contour, not discrete trigger generation |
| Organic / Rain | Rain generates stochastic continuous-time envelope events; Probability decisions occur only on a fixed musical grid |
| Generative / Turing | Turing mutates a stored binary loop; Euclid has no evolving memory and Probability redraws independent optional events |
| Generative / Motif | Motif edits an explicit eight-value CV phrase; Percussion never performs phrase rotation/reversal/substitution |
| planned Repeat vs Probability | Repeat never removes its quarter-note anchors; randomness controls sub-event multiplication. Probability changes whether optional primary-grid events exist at all |
| Euclid vs Probability | Euclid is deterministic and even-distribution based; Probability follows a fixed metric hierarchy with stochastic occupancy |

The nearest regression boundaries are Euclid↔Polymeter, Repeat↔Probability, Probability↔Rain and Humanize↔Shuffle.

## 11. Musical assessment

| Algorithm | Musical value | Strongest roles | Main limitation |
|---|---|---|---|
| **Euclid** | very high | kick/snare/percussion ostinatos, sparse-to-dense hats, long self-running grooves with deterministic fills | fixed 16-step bar and no independent rotation control |
| **Repeat** | very high | hi-hat ratchets, snare rolls, flam-like percussion, phrase-ending stutters | quarter-note anchors are intentionally fixed because no trigger input exists |
| **Probability** | very high | ghost notes, evolving hats/percussion, stable groove skeleton with controlled surprise | one output cannot express separate kick/snare/hat probabilities simultaneously |
| **Humanize** | high | hats, shakers, hand-percussion-like pulse trains and velocity/accent modulation | it humanizes an internal eighth-note train rather than an external rhythm |

The bank's main musical contribution is **time hierarchy**. Existing Drift algorithms mostly work at sample, cycle or phrase-state level. Percussion explicitly introduces sub-events, steps, bars and multi-bar phrases, allowing the output to imply arrangement rather than merely motion.

The 4/8/12/16-bar fill structure should be especially useful in self-running house, techno and generative patches. It is intentionally deterministic in *where* the phrase ending occurs even when the events inside it are stochastic.

## 12. ATmega328P feasibility

The proposed bank is computationally modest:

- **Euclid:** one 16-bit mask lookup plus bit test per step; fill is one mask OR;
- **Repeat:** four anchor decisions per bar plus at most four scheduled pulse events per quarter;
- **Probability:** at most twelve Bernoulli decisions per bar because primary hits are unconditional;
- **Humanize:** two bounded RNG draws per eighth-note event plus event scheduling.

No heap allocation is required. Phrase state consists of small counters and latched control values. Euclidean masks should live in flash rather than being generated in the realtime path.

The most important timing constraint is pulse scheduling, not arithmetic. The implementation must guarantee that the 10 ms pulse state machine cannot lose a later scheduled event and that ratchet pulses remain non-overlapping at the 240 BPM endpoint.

## 13. Verification strategy

Bank-level tests must cover:

- exact 30..240 BPM tempo endpoints;
- exactly 16 sixteenth steps per internal bar;
- phrase lengths exactly 4/8/12/16 bars;
- `isFillBar` true only on the final bar;
- phrase-length changes latched only at phrase boundaries;
- fill strength $F\in\{0,1,2,3,4\}$ across the full Texture domain;
- full-scale fixed pulses exactly 25 samples long for Euclid/Repeat/Probability;
- no phase-overshoot loss at beat/step/bar/phrase boundaries;
- deterministic replay for fixed seeds;
- 0..4095 DAC invariants;
- sanitizer, coverage, AVR resource and 400 µs timing-probe qualification.

Algorithm-specific suites are defined in the individual engineering analyses.

## 14. Implementation decision gate

Before coding, the following are treated as explicit design decisions:

1. retain the shared 30..240 BPM tempo range;
2. use a fixed 16-step bar and fixed 10 ms pulse duration;
3. couple Texture to phrase length as 16/12/8/4 bars and latch that length at phrase start;
4. use canonical pre-generated $E(k,16)$ masks for $k=2..13$;
5. keep Euclid fills additive and confined to the final quarter;
6. keep Repeat anchors fixed to quarter notes and use randomness only for sub-event multiplication;
7. keep Probability primary beats unconditional and ghost probabilities weaker than secondary probabilities;
8. keep Humanize event count fixed and timing offsets relative to an independent nominal grid;
9. preserve the explicit no-external-clock/no-reset limitation in all user documentation.

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../../assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
