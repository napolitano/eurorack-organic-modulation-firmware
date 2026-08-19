# Probability algorithm engineering analysis

## 1. Purpose and scope

Probability is the proposed third Percussion-bank mode. It creates a metrically legible 16-step rhythm in which strong quarter-note anchors are retained while eighth-note subdivisions and weaker sixteenth-note ghost events become increasingly probable as Texture rises.

The design is intentionally not a flat Bernoulli clock. Different grid positions have different musical roles, and the final bar of each phrase increases optional-event probability to produce a stochastic fill. There is no Quinn Freedman Probability mode to preserve.

## 2. Mathematical foundations

The 16 steps of each bar are partitioned into three metric classes.

Primary steps:

$$
P_0=\{0,4,8,12\}.
$$

Secondary steps:

$$
P_1=\{2,6,10,14\}.
$$

Ghost steps:

$$
P_2=\{1,3,5,7,9,11,13,15\}.
$$

Let normalized Texture be $\tau\in[0,1]$.

Primary events are deterministic:

$$
P(hit\mid P_0)=1.
$$

Secondary events use

$$
P(hit\mid P_1)=\tau.
$$

Ghost events use a slower nonlinear rise:

$$
P(hit\mid P_2)=\frac{\tau^2}{2}.
$$

This hierarchy means Texture first fills musically strong eighth-note positions while weaker sixteenth ghosts remain more restrained.

At Texture zero the bar contains exactly four primary hits. At Texture one, all four secondary positions are guaranteed and each of the eight ghost positions still has probability $1/2$, for an expected twelve hits per normal bar.

## 3. Phrase-fill probability boost

The shared fill level is $F\in\{0,1,2,3,4\}$. Define

$$
b=\frac{F}{8}.
$$

For any optional event with base probability $p$ on the final phrase bar,

$$
p' = \min(1,p+b).
$$

For the final quarter of the bar, steps 12..15 receive a second equal boost:

$$
p''=\min(1,p+2b).
$$

Primary steps remain guaranteed and require no boost.

The result is a gradual phrase-end increase in density rather than a completely separate random fill generator.

### Percussion clock-source contract

In this bank, Speed CV is repurposed as a **0..5 V quarter-note clock input** rather than being summed with the Speed knob. Two valid rising edges acquire external timing; loss for more than 2.5 measured periods returns automatically to the Speed-knob clock. The original hardware is not specified for 10 V trigger inputs, so 10 V clocks are explicitly unsupported until the analogue input stage is revised.

## 4. Reference algorithm

At every sixteenth boundary:

1. determine the current step class;
2. if primary, emit a pulse unconditionally;
3. otherwise compute/use the latched fixed-point threshold for the current Texture and class;
4. if the bar is the final phrase bar, add the documented fill boost, with the extra tail boost on steps 12..15;
5. draw one deterministic seeded RNG value;
6. emit a 4095 pulse for exactly 25 samples if the draw is below threshold.

Texture is latched at the start of each bar. Phrase length is latched only when a new phrase starts.

The square in the ghost probability should be implemented with bounded fixed-point multiplication. No floating-point operation is required.

## 5. Relationship to prior art and upstream Drift

Per-step probability is a common sequencer concept, but this exact three-level metric hierarchy and phrase-fill boost are project-defined. The design does not copy Mutable Instruments Grids or any stored drum-pattern map; it contains no learned pattern database and no multi-voice interpolation.

Ableton Live exposes note probability as a separate event property in its MIDI workflow, illustrating the general musical distinction between a nominal grid event and the probability that it occurs. Drift's implementation is independent and much smaller: <https://www.ableton.com/en/manual/live-concepts/>.

There is no corresponding upstream Drift mode.

## 6. Behavioral analysis

Probability has a deliberately stable floor. The quarter-note skeleton never disappears, so even high stochastic activity remains metrically understandable.

At low Texture, the pattern is nearly four-on-the-floor with occasional secondary events. Mid Texture fills more eighth-note positions while ghost events remain sparse. High Texture approaches a busy hat/percussion texture, especially in the phrase-ending bar where optional-event thresholds rise.

Unlike Repeat, Probability changes **whether an optional grid event exists**. It never creates multiple sub-events from one onset. Unlike Euclid, the optional hit distribution is stochastic and metrically weighted rather than evenly distributed.

## 7. Findings and classification

- **Musical design choice:** primary quarter-note anchors are always present.
- **Probability design choice:** ghost probability is quadratic and capped at 1/2 on normal bars.
- **Phrase requirement:** fill boosts probabilities but does not alter the metric class of a step.
- **Statistical requirement:** finite test runs must validate thresholds/tolerance rather than demand exact hit counts.
- **Reproducibility requirement:** fixed seed and controls must reproduce the same event sequence.
- **Hardware limitation:** one output provides one probability-driven rhythm, not independent kick/snare/hat probability lanes.

## 8. Improvement strategy

The first implementation should not introduce per-class rotations, pattern memories or user-programmable probability tables. Those features would require more controls and would erode the distinction from a sequencer.

If listening tests show that always-on primary steps make the mode too kick-centric, the primary probability could later become slightly less than one at extreme Texture. That would be a significant musical contract change and must not be introduced as an optimization.

The fill boost should be auditioned specifically at high Texture. Because high base probabilities already create dense bars, the phrase-end boost must remain audible without collapsing every final bar into a constant gate.

## 9. Computational cost on ATmega328P

Only sixteenth boundaries perform stochastic work. A normal bar contains:

- four unconditional primary events;
- four secondary Bernoulli decisions;
- eight ghost Bernoulli decisions.

This is twelve RNG comparisons per bar, negligible at 30..240 BPM.

The only nontrivial math is the Texture-square used for the ghost threshold, which can be computed once when Texture is latched at the bar boundary.

## 10. Verification and test strategy

Required deterministic tests:

- primary mask contains exactly steps 0/4/8/12;
- secondary mask contains exactly steps 2/6/10/14;
- ghost mask contains exactly the eight odd steps;
- Texture zero gives secondary and ghost threshold zero;
- Texture maximum gives secondary probability one and ghost probability one-half before fill boost;
- thresholds are monotone over all 1024 Texture codes;
- fill boost $b$ maps exactly from $F=0..4$;
- final-quarter threshold is never lower than the rest of the fill bar;
- all probabilities saturate at one rather than overflow;
- fixed seed produces identical event vectors;
- every emitted pulse lasts exactly 25 samples;
- output remains in 0..4095.

Required statistical tests:

- secondary hit rate matches representative target probabilities within a predeclared confidence/tolerance band;
- ghost hit rate tracks $\tau^2/2$ at representative Texture values;
- fill-bar optional hit rate is measurably greater than normal-bar rate for the same base Texture;
- primary events are never lost over long runs.

## 11. Musical assessment

**Musical value: very high.**

Probability offers a different kind of variation from the Generative bank. It does not remember and mutate a phrase; it keeps a clear metric skeleton while deciding which optional percussion events are allowed to appear.

Strong uses include:

- hi-hat and shaker variation;
- ghost-snare or rimshot-style trigger streams;
- auxiliary percussion around a stable kick pulse;
- sample-switch or envelope triggers that should become denser toward phrase endings;
- generative techno where the listener should retain a clear beat despite stochastic detail.

The phrase fill is particularly valuable because randomness alone often lacks arrangement. Here the stochastic density is still biased toward a predictable multi-bar endpoint.

The main limitation is the single output. The three internal metric classes suggest kick/eighth/ghost roles, but they are all emitted on the same jack and therefore cannot independently address three drum voices.

## 12. Engineering assessment

Probability is low-risk computationally and moderate-risk statistically. The principal engineering task is defining tests that verify the actual Bernoulli thresholds without treating normal finite random variation as a failure.

Because primary events are deterministic and all optional decisions occur on a fixed sixteenth grid, the mode remains substantially easier to reason about than continuous-time Rain and should fit comfortably within the existing AVR budget.

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../../assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
