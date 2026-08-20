# Urn algorithm engineering analysis

## 1. Purpose and scope

Urn is a proposed Generative-bank algorithm for stochastic modulation with **reinforced preference**. Instead of storing a loop or using a fixed current-state transition matrix, it remembers how strongly each output state has recently been favoured and feeds that history back into future selection probabilities.

The result is intended to form temporary habits: clusters of repeated or frequently revisited voltages emerge, persist for a while, and then can fade.

The model is inspired by Pólya urn processes but deliberately adds bounded weights and relaxation. It is therefore not an exact classical Pólya urn.

## 2. Mathematical foundations

Let there be eight output states with non-negative selection weights

$$
w_0,w_1,\ldots,w_7.
$$

All weights begin at the same baseline $b$, so the initial categorical distribution is uniform.

At each step, state $i$ is chosen with probability

$$
P(X=i)=\frac{w_i}{W},
\qquad
W=\sum_{j=0}^{7}w_j.
$$

Before reinforcement, every weight relaxes toward the common baseline:

$$
w_i' = b + \rho(w_i-b),
\qquad 0<\rho<1.
$$

The selected state $s$ then receives Texture-controlled reinforcement:

$$
w_s''=\min(w_{max},w_s'+R(\tau)),
$$

while all other states remain at their relaxed values.

$R(\tau)$ is monotone with

$$
R(0)=0.
$$

At zero Texture, the system therefore remains at equal weights and every draw is uniform. Increasing Texture causes recent selections to become more likely to recur.

## 3. Relationship to Pólya urn processes

In the classical Pólya urn mechanism, drawing a colour causes additional mass of that colour to be returned to the urn, increasing its future draw probability. This is a canonical example of stochastic reinforcement.

Robin Pemantle surveys generalized Pólya urns and other reinforced random processes in *A survey of random processes with reinforcement*, Probability Surveys 4 (2007), DOI: <https://doi.org/10.1214/07-PS094>.

Drift intentionally differs from the classical process in two ways:

1. weights are bounded;
2. weights decay/relax toward a baseline.

Those differences prevent indefinite numeric growth and, more importantly, prevent an early random advantage from becoming musically permanent. The correct description is therefore **Pólya-inspired leaky reinforcement**, not "a Pólya urn" without qualification.

## 4. Output vocabulary

The initial implementation should use eight fixed 12-bit output levels. An evenly spread vocabulary is preferred for Urn because the evolving probability distribution—not random voltage placement—is the musical subject.

One simple mapping is

$$
v_i=\mathrm{round}\left(\frac{4095i}{7}\right),
\qquad i=0\ldots7.
$$

This guarantees eight distinct ordered levels spanning the full DAC range. Analog Attenuation can reduce the physical span when a narrower modulation range is wanted.

Using a fixed vocabulary also differentiates Urn from Markov's seed-defined shuffled voltage vocabulary.

## 5. Reference algorithm

For each processing sample:

1. advance the common Speed phase;
2. keep the current output when no phase wrap occurs;
3. on wrap, relax each of eight weights toward baseline;
4. sum the weights;
5. select one state categorically in proportion to the relaxed weights;
6. add Texture-controlled reinforcement to the selected state's weight with saturation;
7. output the fixed 12-bit voltage corresponding to that state.

Texture CV is summed with the Texture knob in the standard saturated 10-bit control domain.

## 6. Behavioral analysis

At Texture 0, all weights remain equal and successive output states are independent uniform draws from the eight-level vocabulary.

At low reinforcement, accidental repetitions create only weak preferences and decay quickly. At medium settings, small clusters form: a selected state becomes slightly more likely, repeated selection strengthens it further, and the system can spend time orbiting a few favoured values.

At high Texture, reinforcement can create strong local fixation. The relaxation term is therefore essential. Without it, the mode can become progressively less exploratory as old history accumulates.

Unlike Markov, the probability of the next state is not determined by the identity of only the current state. It depends on the complete current weight vector, which summarizes earlier reinforcement history.

## 7. Findings and classification

- **Musical design choice:** eight fixed, evenly spread output states keep the focus on changing preference rather than vocabulary generation.
- **Musical requirement:** zero Texture must reduce exactly to an equal-weight baseline process.
- **Musical requirement:** reinforcement must decay; permanent lock-in is not desirable for Drift.
- **Numerical safety requirement:** weights and total weight must have explicit finite bounds.
- **Terminology constraint:** the model is Pólya-inspired but not the exact classical urn process.
- **Statistical implementation concern:** weighted categorical selection must have a documented integer method and measurable bias bound.

## 8. Implemented fixed-point parameterization

The first AVR implementation fixes the integer contract to:

- baseline $b=32$;
- maximum weight $w_{max}=1023$;
- retention $\rho=31/32$ per draw;
- reinforcement $R$ mapped monotonically from exactly 0 at minimum Texture to exactly 64 at maximum Texture.

The relaxation can then be implemented with shifts rather than general division:

$$
w_i' = b + \left\lfloor\frac{31(w_i-b)}{32}\right\rfloor.
$$

These constants are part of the behavior released in `0.2.0`. Future changes to them are sound-design changes and require updated deterministic/statistical tests plus a changelog entry.

## 9. Improvement strategy

The primary tuning problem is the relationship among baseline, reinforcement and retention. These should be adjusted together and documented, not hidden behind ad-hoc clamps.

The algorithm should not add separate "negative reinforcement", evolving vocabulary or state-to-state transition rules in its first version; those would blur the distinction from Markov and Motif.

If high Texture proves too sticky, reducing retention or reinforcement is preferable to injecting unrelated random resets.

## 10. Computational cost on ATmega328P

Normal 2.5 kHz samples require only the shared phase update. Work occurs on step events:

- eight small integer relaxation updates;
- sum of eight bounded weights;
- one weighted categorical selection;
- one saturating reinforcement update;
- one eight-entry voltage lookup.

Persistent state is eight weights, one phase accumulator, RNG state, selected state and output value. Even with 16-bit weights this is modest SRAM use.

The weighted draw is the only operation likely to require careful implementation/timing review.

## 11. Optimization opportunities

- Choose baseline/retention parameters whose relaxation denominator is a power of two.
- Use 16-bit weights and a bounded total that fits comfortably below 65536.
- Perform all eight-weight work only on phase wrap.
- The implemented weighted draw uses multiply-high scaling. For totals that do not divide 65536, source-bucket sizes differ by at most one 16-bit random word; this bounded bias is documented and avoids runtime division.
- Avoid 32-bit division in the 2.5 kHz hot path; event-only division may still be acceptable if timing measurements prove margin.

## 12. Verification and test strategy

Required deterministic tests:

- all weights initialize exactly to baseline;
- Texture 0 produces zero reinforcement;
- equal weights produce an equal categorical partition in the integer selection implementation;
- reinforcement is monotone with Texture;
- relaxation moves every non-baseline weight toward baseline without crossing it;
- baseline weight remains baseline under relaxation;
- selected weight never exceeds $w_{max}$;
- total weight cannot overflow its declared integer domain;
- fixed RNG script produces exact known selections and weights;
- output lookup stays in 0..4095;
- no phase wrap means no weight or output-state update.

Required statistical tests:

- at Texture 0, long-run state frequencies are consistent with uniform selection within a predeclared tolerance/confidence rule;
- after artificially reinforcing one state, its empirical reselection frequency is higher than an otherwise identical baseline state;
- with reinforcement stopped, that preference decays toward baseline over repeated draws;
- high Texture does not create an absorbing state under long-run simulation.

Statistical tests must use fixed seeds and explicit pass criteria to avoid flaky CI.

## 13. Musical assessment

**Musical value: high, but more experimental than Turing or Motif.**

Urn's attraction is emergent preference. The module can appear to "discover" a few favourite regions, return to them for a while, then move elsewhere without a programmed loop or fixed transition graph.

Good targets include:

- resonator or filter bands where temporary focus is useful;
- macro timbre states;
- external quantizers, where reinforced levels become temporary pitch favourites;
- effect sends and feedback paths that should develop statistical habits;
- long autonomous generative patches.

It is especially distinct from Rain: Rain changes the probability of independent impulse arrivals, whereas Urn changes the probability of **which state will be selected next based on reinforced history**.

The risk is musical stagnation. Too much reinforcement or too little decay will create one dominant level and destroy the value of the mode. For that reason the leaky relaxation is part of the sound design contract, not merely an overflow-control trick.

## 14. Engineering assessment

Urn is the most statistically interesting and least immediately predictable algorithm in the bank. It remains feasible on ATmega328P because the state space is only eight weights and all expensive work is event-driven. Its success depends more on parameter tuning and statistical verification than on raw DSP complexity.

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../../assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
