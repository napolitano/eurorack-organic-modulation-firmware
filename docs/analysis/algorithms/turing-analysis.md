# Turing algorithm engineering analysis

## 1. Purpose and scope

Turing is a proposed algorithm for the optional Generative bank. It is intended to produce stepped control-voltage loops that can move continuously from exact repetition to progressive mutation.

The mode is based on a 16-bit shift register with probabilistic feedback corruption. It is conceptually related to shift-register random sequencers such as Music Thing Modular's Turing Machine, but this document defines an independent project implementation for Drift. There is no Quinn Freedman Turing mode to preserve.

## 2. Mathematical foundations

Let the 16-bit register at step $n$ be $R_n$. One bit leaves the active end of the register and is used as the feedback bit $b_n$. Texture controls a mutation probability $p$ in the range

$$
0\le p\le\frac{1}{2}.
$$

A Bernoulli random variable $M_n\sim\operatorname{Bernoulli}(p)$ decides whether that feedback bit is inverted:

$$
b_n'=b_n\oplus M_n.
$$

The register then shifts by one position and $b_n'$ enters at the opposite end.

At $p=0$, every bit is recycled unchanged. The register therefore rotates and the complete 16-step state sequence repeats exactly, subject only to shorter periods in degenerate bit patterns.

At $p=1/2$, $M_n$ is a fair random bit. XOR with a fair independent bit makes $b_n'$ fair and independent of the old feedback value, giving the maximum useful write randomness for this construction.

Texture must not map $p$ to $1$. A value of $p=1$ would make every feedback bit invert deterministically and would therefore be *less* random than $p=1/2$.

The DAC output is a deterministic 12-bit projection of the current register, for example

$$
y_n = R_n \gg 4,
$$

which maps the upper twelve register bits directly to the 0..4095 DAC domain.

## 3. Reference algorithm

At every 2.5 kHz processing sample:

1. combine Speed knob and Speed CV through the established Drift exponential mapping;
2. advance the 32-bit step phase;
3. if no phase wrap occurs, retain the previous DAC value;
4. on phase wrap, derive Texture from saturated knob + CV;
5. map Texture monotonically to $p\in[0,1/2]$;
6. read the feedback bit from the 16-bit register;
7. probabilistically invert it with probability $p$;
8. shift the register and insert the resulting bit;
9. project the register to a 12-bit output value.

A fixed seed and identical control sequence must reproduce the same register evolution. The implementation initializes the register deterministically from the shared RNG and remaps only the two degenerate all-zero/all-one initial states so Texture zero does not start as a trivial constant loop.

## 4. Relationship to prior art and upstream Drift

The official Music Thing Modular description calls the Turing Machine a binary sequencer based around a 16-bit shift register that produces clocked, stepped, randomly changing control voltages and can be locked into repeating loops: <https://musicthing.co.uk/Turing-Machine/>.

Drift's proposed mode adopts only the broad musical idea of probabilistically corrupted shift-register feedback. It does not reproduce the Turing Machine circuit, analogue DAC weighting, panel behavior, expanders or source code.

There is no corresponding mode in Quinn Freedman's Drift firmware. The only reused Drift contract is the corrected shared Speed-to-phase mapping and the existing 12-bit output path.

## 5. Behavioral analysis

The mode has two distinct time scales:

- **step time**, set by Speed;
- **pattern persistence**, set statistically by Texture.

At zero Texture, the pattern is fixed. A single mutation does not simply replace one output step: because one altered bit keeps circulating through the register, its effect propagates through subsequent 12-bit projections. The result is gradual structural change rather than independent sample-and-hold voltages.

At low non-zero Texture, long recognisable passages should recur with occasional altered steps. Near maximum Texture, new feedback information is effectively random at every shift and the register continuously forgets its previous loop.

The algorithm is intentionally stepped. Applying smoothing internally would weaken the phrase/sequence character and move the result toward Bézier-style continuous transitions.

## 6. Findings and classification

- **Musical design choice:** register length is fixed at 16 bits to provide a useful loop length without another front-panel parameter.
- **Mathematical requirement:** maximum mutation probability is $1/2$, not $1$.
- **Sound-design choice:** output uses twelve register bits directly rather than reducing the state to a smaller quantized code.
- **Implementation requirement:** the initial register must not accidentally depend on undefined/uninitialised memory; it is seeded explicitly.
- **Naming/provenance concern:** "Turing" should be documented as a shift-register-inspired mode and must not imply affiliation with Music Thing Modular.

## 7. Improvement strategy

The first implementation should remain deliberately small: one fixed 16-bit length, one mutation control and one 12-bit projection.

Potential future variants such as variable register length, rotating bit taps, pulse extraction or quantized voltage tables would materially change the instrument behavior and should be treated as separate revisions rather than silent optimizations.

If product naming becomes a concern, **Register** or **Shift** is a technically accurate alternative user-facing name while retaining this document's provenance discussion.

## 8. Computational cost on ATmega328P

Per processing sample the mode needs only the common Speed/phase update and a wrap test. On a step event it additionally performs:

- one random comparison for mutation;
- one bit extraction/XOR;
- one 16-bit shift and insertion;
- one 12-bit register projection.

Persistent algorithm state is one 32-bit phase accumulator, one 16-bit register, one RNG state and the last 12-bit output.

This should be substantially cheaper than Organic Fractal's three gradient-noise evaluations per sample.

## 9. Optimization opportunities

- Express the Texture-to-mutation mapping with integer scaling to a 15- or 16-bit random threshold.
- Keep register length fixed at 16 so rotation and masking compile to cheap AVR operations.
- Perform RNG work only on phase wrap, never on every 2.5 kHz sample.
- Avoid a division when mapping the register to the DAC; a simple shift is sufficient.

## 10. Verification and test strategy

Required mathematical and state tests:

- Texture 0 produces zero mutations under every RNG draw;
- maximum Texture implements a threshold corresponding to $p=1/2$ within the chosen integer random domain;
- with Texture 0, a non-degenerate register returns exactly to its initial state after 16 shifts;
- a known register produces known 12-bit DAC projections for each shift;
- a forced mutation changes only the entering feedback bit at that transition;
- output always remains in 0..4095;
- no phase wrap means no register state/output change;
- identical seed and controls are bit-for-bit deterministic;
- long-run high-Texture output exercises both bit values without register lockup;
- timing-probe build remains below the 400 µs sample deadline.

Statistical tests should verify the mutation decision itself, not demand a particular finite output histogram from the correlated register sequence.

## 11. Musical assessment

**Musical value: very high.**

Turing addresses a gap in the current Drift repertoire: repeatable stepped modulation that can slowly rewrite itself. Brownian remembers only its current continuous position; Bézier remembers two random endpoints; Turing remembers an entire binary loop.

Particularly strong uses are:

- external-quantizer melodies and bass lines;
- filter cut-off sequences that repeat long enough to become recognizable;
- wavetable/wavefolder position patterns;
- changing envelope times or effect parameters;
- generative modulation in self-running patches where exact clock synchronization is not essential.

The central musical virtue is **controllable persistence**. Texture does not merely add noise; it changes how long musical information survives.

The main limitation comes from the existing hardware rather than the algorithm: there is no dedicated external clock input, so the sequence cannot yet be promised as transport-synchronous to another Eurorack clock source.

## 12. Engineering assessment

Turing is low-risk mathematically and computationally. Its important engineering detail is the correct mutation range: the intuitive but wrong mapping of Texture to a 0..100% bit-flip probability would make maximum Texture deterministic again. With $p\in[0,1/2]$, the control has a clean monotonic meaning from locked memory to maximally random feedback.

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../../assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
