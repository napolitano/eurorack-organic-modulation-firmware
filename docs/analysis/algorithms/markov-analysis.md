# Markov algorithm engineering analysis

## 1. Purpose and scope

Markov is a proposed Generative-bank mode for producing recurring stepped control voltages without storing or repeating a literal phrase. It combines a fixed per-boot voltage vocabulary with a finite-state stochastic transition grammar.

The aim is musical recurrence with probabilistic order: the listener should repeatedly encounter the same small set of voltage "places", while the route between those places changes.

There is no upstream Quinn Freedman Markov implementation.

## 2. Mathematical foundations

A finite time-homogeneous Markov chain has states

$$
S=\{0,1,\ldots,7\}
$$

and transition probabilities

$$
P_{ij}=P(X_{n+1}=j\mid X_n=i).
$$

The Markov property means that, conditioned on the current state, the next-state probability does not depend on the complete earlier path.

Drift defines a structured base transition $P_s$. From state $i$:

$$
P_s(i\rightarrow i)=\frac{1}{2},
$$

$$
P_s(i\rightarrow i+1)=\frac{1}{4},
$$

$$
P_s(i\rightarrow i-1)=\frac{1}{8},
$$

$$
P_s(i\rightarrow i+4)=\frac{1}{8},
$$

with state arithmetic modulo 8.

Texture does not directly alter these four weights. Instead it mixes the complete structured chain with the uniform transition matrix $U$, whose entries are all $1/8$:

$$
P_{\tau}=(1-\tau)P_s+\tau U,
\qquad 0\le\tau\le1.
$$

This has an implementation-friendly equivalent: with probability $\tau$, select the next state uniformly from all eight states; otherwise use one draw from the structured rule.

At $\tau=0$, the chain has a strong grammar. At $\tau=1$, the next state is uniform and independent of the current state.

For general Markov-chain terminology and transition-matrix treatment, see Seabrook and Wiskott, *A Tutorial on the Spectral Theory of Markov Chains*, 2022: <https://arxiv.org/abs/2207.02296>.

## 3. Voltage vocabulary

Each of the eight states maps to a 12-bit voltage that is generated once during initialization and remains fixed until reset/power cycle.

The implementation uses stratified generation: divide 0..4095 into eight 512-code bands, select one pseudorandom code from each band, then Fisher–Yates-shuffle the eight codes before assigning them to state labels. Startup shuffle indices use multiply-high scaling; for non-power-of-two range sizes the resulting source-bucket imbalance is bounded to one 16-bit random word.

This provides three useful properties:

1. the vocabulary covers the useful output range rather than accidentally clustering in one small region;
2. state index does not imply voltage order, so a "neighboring" Markov state is not equivalent to a Brownian voltage step;
3. a fixed seed reproduces the same vocabulary exactly.

The state sequence is therefore discrete and symbolic, while its actual voltage meaning is seed-defined.

## 4. Reference algorithm

For each processing sample:

1. advance the common exponential Speed phase;
2. retain the current output unless the phase wraps;
3. on wrap, combine Texture knob and CV to $\tau$;
4. decide whether this transition uses uniform exploration or the structured grammar;
5. select the next state accordingly;
6. output the fixed 12-bit vocabulary value assigned to that state.

Vocabulary generation happens only on initialization, not on every transition.

## 5. Relationship to existing Drift algorithms

Markov must not become a disguised Brownian random walk. The distinction is structural:

- Brownian's state *is the voltage*, and its next move is a local displacement of that voltage;
- Markov's state is a symbolic index, and each index points to an independently assigned vocabulary voltage.

Thus $i\rightarrow i+1$ is a local transition in the state graph but can correspond to a large upward or downward CV jump.

It is also distinct from Turing: Markov has no repeating bit loop and no accumulating bit corruption. The transition law remains fixed; only the visited states change.

## 6. Behavioral analysis

At low Texture, the $1/2$ self-transition creates dwell, the $1/4$ forward transition gives a weak sense of continuation, the $1/8$ backward transition creates local return, and the $1/8$ opposite transition provides occasional larger structural movement.

As Texture rises, more transitions bypass that grammar and choose any of the eight states uniformly. The result moves from "habitual" to exploratory without changing the voltage vocabulary itself.

Because the vocabulary is fixed, even high Texture retains a recognizable set of levels. This is a crucial musical difference from ordinary sample-and-hold noise, which redraws values from a continuous range.

## 7. Findings and classification

- **Sound-design choice:** eight states balance recognizable vocabulary against sufficient variety.
- **Sound-design choice:** the structured row weights are simple binary fractions so the behavior is explainable and cheap to implement.
- **Musical requirement:** voltage assignment must not be monotonically tied to state index; otherwise the local transition graph would drift toward Brownian-like voltage adjacency.
- **Implementation trade-off:** Texture mixes between two complete kernels rather than continuously recalculating an 8x8 matrix.
- **Terminology constraint:** Texture is best described as **Exploration** or transition randomness, not as a rigorously measured "entropy amount" unless an entropy contract is explicitly tested.

## 8. Improvement strategy

The first implementation should keep the transition grammar fixed. User-selectable matrices, learned transitions or evolving vocabularies would create much larger state and documentation surfaces and would overlap Motif/Urn.

If later musical testing finds the base grammar too stationary, the weights can be revised only as a documented sound-design change with updated golden/statistical tests.

## 9. Computational cost on ATmega328P

Per normal processing sample, Markov has only the shared phase update and wrap test.

On a transition event:

- one Texture exploration decision;
- one uniform state draw or one 3-bit-equivalent structured draw;
- one table lookup into eight 12-bit vocabulary values.

Initialization requires generating and permuting eight values. Persistent state is an eight-element 16-bit vocabulary, the current state, phase accumulator, RNG state and output value.

## 10. Optimization opportunities

- Encode the structured distribution with an 0..7 random draw: 0..3 stay, 4..5 forward, 6 backward, 7 opposite.
- Use the saturated 10-bit Texture code directly as the threshold for the uniform-exploration decision.
- Generate vocabulary only once at initialization.
- Store the vocabulary as `uint16_t[8]`; no matrix needs to be stored in SRAM.

## 11. Verification and test strategy

Required tests:

- every base transition row sums to exactly one in the integer decision representation;
- base transition outcomes have exact 4/8, 2/8, 1/8, 1/8 partitioning over all 3-bit source values;
- Texture 0 never enters the uniform-exploration branch;
- maximum Texture always enters the uniform branch;
- all next states remain in 0..7;
- stratified vocabulary generation yields one value in each 512-code source band before permutation;
- permutation preserves all eight generated values exactly;
- fixed seed yields fixed vocabulary and state sequence;
- output always remains in 0..4095;
- no phase wrap means no state transition;
- statistical tests at an intermediate Texture match the documented structured/uniform mixture within a justified tolerance.

A test must not assume that adjacent state numbers produce adjacent voltages.

## 12. Musical assessment

**Musical value: high.**

Markov is useful when a patch should have a recurring vocabulary but not a repeating phrase. An external quantizer can turn the eight levels into a recurring pitch collection; without pitch use, the same behavior is strong for timbral zones, resonator settings, wavefolder depth, delay feedback and effect macro parameters.

The $1/2$ self-transition is musically important because it allows a value to persist for multiple decision steps. That creates phrasing and space rather than a mandatory change at every clock event.

Compared with Turing, Markov feels less like a loop being rewritten and more like a system moving among familiar places. Compared with Urn, its preferences are **structural and fixed** rather than learned through reinforcement.

Its main risk is aesthetic rather than numerical: a poorly chosen transition matrix can sound arbitrary. The simple four-outcome grammar is intentionally conservative and understandable.

## 13. Engineering assessment

Markov is computationally inexpensive and mathematically straightforward. The strongest design decision is the separation between symbolic state topology and voltage ordering; that prevents accidental duplication of Brownian behavior and gives the mode its own musical identity.

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../../assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
