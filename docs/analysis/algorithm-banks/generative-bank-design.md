# Generative bank architecture and control contract

## 1. Purpose

The optional **Generative** bank is intended to add a kind of modulation that is deliberately under-represented in the Classic and Organic banks: discrete structure with memory.

Classic already covers continuous gradient noise, a bounded random walk, random destinations joined by curves, and a periodic LFO. Organic adds multi-scale noise, a two-dimensional flow, stochastic impulses and a deterministic nonlinear attractor. A third bank should therefore not be four more ways of producing continuously wandering randomness.

Generative instead treats the Drift output as a sequence of **internally timed 12-bit control-voltage states**. Its four algorithms explore four different forms of musical memory:

- **Turing** — a repeating binary shift-register pattern that can mutate;
- **Markov** — a finite vocabulary navigated by a state-dependent stochastic grammar;
- **Motif** — an explicit phrase whose ordering is transformed between repetitions;
- **Urn** — a reinforced stochastic vocabulary that develops temporary preferences.

The bank is project-defined. There is no Quinn Freedman implementation of these four modes to preserve.

## 2. Bank slot mapping

The Generative firmware image uses the same four rear-DIP slots as every other bank:

| Slot | DIP 1 | DIP 2 | Generative algorithm |
|---:|---|---|---|
| 0 | OFF | OFF | Turing |
| 1 | ON | OFF | Markov |
| 2 | OFF | ON | Motif |
| 3 | ON | ON | Urn |

The bank itself remains a compile-time firmware choice. The rear DIP switches select only one of the four algorithms in the flashed bank and are sampled at startup.

## 3. Shared timing model

All four algorithms use the verified Drift exponential Speed mapping to advance an internal 32-bit phase accumulator. A state transition occurs only when that phase wraps.

This makes Speed a **step rate**, not a continuous trajectory rate. With the present Drift mapping, the useful range is approximately the same 12-octave span already used by Classic Perlin/Bézier/LFO and Organic Fractal/Vector/Attractor.

The output is intentionally stepped between state transitions. Adding automatic slew would collapse the conceptual distinction from Bézier and other continuous-motion modes.

The existing hardware has no dedicated clock input, so the initial Generative contract is **internally clocked**. Speed CV changes the internal step rate; it is not documented as an external clock input.

## 4. Control contract

| Algorithm | Speed | Texture | Texture CV | Analog Attenuation |
|---|---|---|---|---|
| **Turing** | shift/step rate | mutation from locked loop to maximally random write bit | adds mutation | output depth |
| **Markov** | transition rate | exploration: structured transition grammar to uniform next-state choice | adds exploration | output depth |
| **Motif** | phrase step rate | probability of one structural edit after each completed phrase | adds variation | output depth |
| **Urn** | draw rate | reinforcement strength / preference formation | adds reinforcement | output depth |

Texture knob and Texture CV are summed and saturated in the normal 10-bit Drift control domain. Attenuation remains a post-DAC analogue level control and is not available to firmware.

## 5. Turing: evolving shift-register loop

Turing keeps a 16-bit register. At each step, the outgoing feedback bit is recycled into the register unless a Texture-controlled Bernoulli mutation flips it.

If $p$ is the mutation probability,

$$
b_{new}=b_{feedback}\oplus B(p),
$$

where $B(p)$ is a Bernoulli random variable. Texture maps

$$
0\le p\le \frac{1}{2}.
$$

At $p=0$, the register rotates without corruption and therefore repeats. At $p=1/2$, XOR with a fair random bit makes the incoming bit independent of the feedback bit, giving the maximum randomness required by this control contract. Mapping beyond $1/2$ would begin moving back toward deterministic inversion rather than adding useful randomness.

The 12-bit output is derived directly from twelve register bits. The musical result is a repeatable stepped CV loop whose content gradually changes as Texture rises.

The concept is related to the family of shift-register random sequencers exemplified by Music Thing Modular's Turing Machine, whose official documentation describes a 16-bit shift-register sequencer that can move between repeating loops and randomly changing stepped voltages: <https://musicthing.co.uk/Turing-Machine/>. The Drift implementation is independent and does not copy its circuit or source code.

## 6. Markov: stochastic state grammar

Markov has eight discrete internal states. Each state maps to one member of a fixed per-boot eight-voltage vocabulary. The voltage vocabulary is generated once from the seed and then retained; the sequence changes because the state transitions change, not because every output value is redrawn.

At minimum Texture, the structured transition law from state $i$ is:

- $1/2$ remain in $i$;
- $1/4$ move to $(i+1)\bmod 8$;
- $1/8$ move to $(i-1)\bmod 8$;
- $1/8$ move to $(i+4)\bmod 8$.

Texture mixes this structured transition law with a uniform transition to any of the eight states. With normalized Texture $\tau$,

$$
P_{\tau}=(1-\tau)P_{structured}+\tau U,
$$

where every entry of $U$ is $1/8$.

At full Texture, the next state is therefore uniform and independent of the current state. At lower settings, the process exhibits a recognisable grammar of repetition, continuation, small backtracking and occasional large change.

This is a finite time-homogeneous Markov chain: the probability of the next state depends on the current state and the fixed transition rule, not the complete prior path. Standard finite-state Markov-chain terminology and transition-matrix definitions are discussed, for example, by Seabrook and Wiskott, *A Tutorial on the Spectral Theory of Markov Chains*, 2022: <https://arxiv.org/abs/2207.02296>.

## 7. Motif: structural phrase mutation

Motif stores an explicit eight-step phrase of 12-bit output values. The phrase plays unchanged for one full cycle. At the cycle boundary, Texture determines whether exactly one structural edit is made.

With normalized Texture $\tau$,

$$
P(edit)=\tau.
$$

When an edit occurs, one of four operations is selected with equal probability:

1. rotate the complete phrase by one position left or right;
2. swap one adjacent pair;
3. reverse one circular three-step span;
4. replace one position with a newly generated 12-bit value.

Three of the four operations preserve the complete voltage vocabulary. Even the replacement operation preserves seven of eight values. The algorithm therefore evolves by **variation of a recognisable phrase**, rather than by redrawing a sequence from scratch.

Rotation, reversal, permutation and substitution are established classes of sequence transformation in algorithmic composition. The exact operation set and probabilities above are project-defined rather than an implementation of a named composition system. General algorithmic-composition context is discussed by Michael Edwards, *Algorithmic Composition: Computational Thinking in Music*, Communications of the ACM 54(7), 2011: <https://www.pure.ed.ac.uk/ws/files/16205214/algorithmic_composition_AM.pdf>.

## 8. Urn: reinforced preference process

Urn uses eight output states with mutable selection weights. All states begin with the same baseline weight, so selection is initially uniform. After each selected state is drawn, its weight is reinforced. Between draws all weights relax slowly toward the common baseline.

A simplified conceptual update is

$$
w_i' = w_0 + \rho(w_i-w_0),
$$

followed for the selected state $s$ by

$$
w_s''=\min(w_{max},w_s'+R(\tau)).
$$

Here $0<\rho<1$ is a fixed retention factor and $R(\tau)$ is a monotone Texture-controlled reinforcement amount with $R(0)=0$.

The next state is chosen categorically with probability

$$
P(X=i)=\frac{w_i}{\sum_j w_j}.
$$

The reinforcement mechanism is inspired by Pólya urn processes, in which previous selections increase the probability of future selections. The deliberate relaxation toward baseline makes Drift's implementation **leaky and bounded**, so it must not be documented as an exact classical Pólya urn. Reinforced stochastic processes and generalized Pólya urns are surveyed by Robin Pemantle, *A survey of random processes with reinforcement*, Probability Surveys 4 (2007), DOI: <https://doi.org/10.1214/07-PS094>.

Musically, this produces temporary favourites: a voltage that happens to be selected can become increasingly likely, form a local cluster of recurrence, then gradually lose its advantage when it stops being reinforced.

## 9. Duplication audit

The four algorithms were selected specifically to avoid re-implementing existing Drift behavior under new names.

| Existing algorithm | Why Generative does not duplicate it |
|---|---|
| **Perlin / Fractal** | continuous correlated gradient noise; Generative is discrete state/phrase logic |
| **Brownian** | next voltage is a locally displaced continuous state; Generative Turing/Markov/Motif/Urn use explicit symbolic/discrete memory structures |
| **Bézier** | random destinations connected by continuous cubic trajectories; Generative intentionally preserves steps |
| **LFO** | deterministic periodic waveform; only locked Turing/Motif repeat, and their actual values are stored/mutable sequences rather than a fixed analytic waveform |
| **Vector** | continuous coupled phase flow; Markov's transition graph is discrete and stochastic |
| **Rain** | independent stochastic event arrivals feeding an envelope; Urn changes future selection probability through reinforcement |
| **Attractor** | deterministic nonlinear map; Generative processes either mutate stored material or make stochastic categorical decisions |

There is also a deliberate distinction **inside** the new bank:

- Turing remembers a binary loop and corrupts its feedback;
- Markov remembers only the current symbolic state while the transition grammar remains fixed;
- Motif remembers the complete phrase and edits its structure at phrase boundaries;
- Urn remembers accumulated preferences across the vocabulary rather than a fixed ordering.

## 10. Musical assessment

### Turing — very high musical value

Turing is the most immediately useful generative mode. At low Texture it behaves like a lockable sequence source; increasing Texture creates controlled evolution rather than a hard switch from repetition to randomness. It is especially strong for filter cut-off, wavetable position, envelope times and—through an external quantizer—melodic or bass-line generation.

Its limitation is equally clear: without an external clock input, its loop is internally timed rather than tempo-locked to another sequencer.

### Markov — high musical value

Markov creates recurrence without a literal loop. The fixed eight-value vocabulary provides identity, while the transition grammar determines how those values are revisited. This is well suited to generative timbre, pitch-class-like control through an external quantizer, and parameter systems where repeated "places" are desirable but a fixed phrase is not.

The main design risk is making the transition matrix too arbitrary. The deliberately simple structured law above is preferred because its musical behavior is explainable and testable.

### Motif — very high musical value

Motif has the strongest phrase-level identity of the bank. The user hears a pattern, then hears recognizable transformations of the same pattern. Because most edits preserve all eight values, it can change for a long time without losing its basic character.

It is particularly valuable for slow generative patches and external quantizer use. Its limitation is that structural change occurs only after full phrase cycles, so at very low Speed evolution can become intentionally very slow.

### Urn — high but more experimental musical value

Urn is less predictable than the other three but occupies genuinely new territory. Instead of loops or a fixed transition graph, it creates temporary statistical habits. This can generate convincing clusters, refrains and periods of fixation without explicitly programming any of them.

Its risk is excessive reinforcement: if the decay/reinforcement balance is poorly chosen, one state can dominate for too long and the output becomes dull. The bounded leaky contract is therefore a musical requirement, not merely a numerical safety measure.

## 11. Resource assessment

All four modes are expected to be cheaper per 2.5 kHz processing sample than Organic Fractal because most non-trivial work occurs only on phase wrap:

- Turing: one register shift and one probabilistic mutation decision per step;
- Markov: one categorical transition decision per step;
- Motif: one output index advance per step and at most one small array edit per eight steps;
- Urn: one eight-weight decay/reinforcement/selection operation per step.

The largest persistent state is still small for ATmega328P: an eight-value phrase or vocabulary plus eight small Urn weights is on the order of tens of bytes, not hundreds.

No floating-point arithmetic, heap allocation or dynamic containers are required.

## 12. Verification contract

A future implementation must add dedicated mathematical suites proving at least:

- exact DIP-slot mapping for the Generative bank;
- Turing lock behavior, 16-step rotation recurrence, endpoint mutation probabilities and deterministic seeded behavior;
- Markov row probabilities, structured/uniform Texture endpoints, state bounds and fixed-vocabulary behavior;
- Motif exact repeat at zero Texture, one-edit-per-cycle maximum, operation invariants and deterministic seeded mutation;
- Urn equal-weight uniform baseline, monotone reinforcement control, bounded weights, relaxation toward baseline and statistically increased reselection probability after reinforcement;
- 12-bit output bounds for all four modes;
- no output update between phase-wrap step events;
- native sanitizer/coverage qualification and AVR flash/SRAM/timing qualification for the third bank.

## 13. Design status

This document defines the proposed Generative mathematical and musical contract. It does **not** claim that the current firmware already implements the bank. Implementation should follow only these documented contracts or explicitly revise the analysis first.

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../../assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
