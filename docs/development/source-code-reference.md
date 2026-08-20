# Source-code reference

This document is the developer map of the production firmware. It complements the user-facing bank guides and the per-algorithm engineering analyses by explaining **where behavior lives in code, which units cross each boundary, and which invariants must remain true during maintenance**.

## Execution model

The ATmega328P firmware is deliberately split into a portable C++17 core and a thin AVR composition root.

```text
src/main.cpp
  -> AVR ports/adapters
  -> DriftRuntime
  -> DriftEngine
  -> exactly one compile-time algorithm bank
  -> exactly one DIP-selected algorithm (or one developer-forced algorithm)
```

The audio-rate/modulation scheduler runs at **2.5 kHz**. Stateful algorithms therefore receive one `ControlFrame` and produce one 12-bit DAC-domain value per scheduler sample. Algorithm classes own only their persistent mathematical state; hardware access belongs outside the domain layer.

## Units and numeric domains

The code intentionally uses names that expose representation instead of relying on comments alone.

| Suffix / type | Meaning |
|---|---|
| `*Adc` | 10-bit hardware/control ADC code, normally 0..1023 |
| `*Dac12` | 12-bit DAC-domain value, normally 0..4095 |
| `phase_`, `*Phase` | unsigned 32-bit wraparound phase accumulator |
| `Q0F12` | unsigned fixed point with 12 fractional bits; unity is 4096 |
| `Q0F16` | unsigned fixed point with 16 fractional bits; unity is 65536 |
| `Q1F15` | signed/unsigned quantity scaled by 2^15 as documented by the specific API |
| `Q0F24` | fixed point with 24 fractional bits, used for sub-LSB residuals/coefficients |
| `Q10` | multiplier with 1024 representing unity |

Public mathematical helpers document clamping behavior explicitly. Callers must not infer that every function accepts arbitrary integer values merely because its C++ parameter type is wider than the hardware range.

## Domain root

`lib/fmd/include/fmd/domain/` and `lib/fmd/src/domain/` contain reusable cross-bank infrastructure.

### `DriftEngine`

`DriftEngine` is the compile-time bank boundary. A normal bank image contains the four algorithms belonging to exactly one `FMD_ALGORITHM_BANK`. Developer-only named targets may compile the engine down to a single algorithm through `FMD_FORCE_ALGORITHM`; release images must never use that override.

The rear DIP switches select one of the four slots only in normal bank images. Bank identity is a build-time choice, not a runtime setting.

### `AlgorithmMath`, `FixedMath`, `FrequencyMapping`

These files contain reusable integer/fixed-point primitives and the common control-to-frequency mapping. Their functions are intentionally pure where possible so numerical behavior can be exhaustively unit-tested without constructing stateful algorithms.

### `ParallelLfsr`

`ParallelLfsr` is the deterministic pseudo-random source used by stochastic algorithms. Tests rely on deterministic seed-to-sequence behavior. Changing its recurrence therefore changes algorithm output streams even when higher-level logic is untouched.

### `ClockSource`

`clock::ClockSource` is shared only by the rhythm-oriented Percussion and Dubstep/Bass banks. It implements:

- hysteretic LOW/HIGH detection on Speed CV;
- two-valid-edge acquisition;
- one accepted external edge = one quarter note;
- measured-period conversion to a 32-bit quarter-note phase increment;
- 2.5-period clock-loss timeout;
- immediate fallback to the caller-provided internal tempo.

It does **not** make the hardware input electrically safe. On the current Drift revision the repurposed Speed-CV clock is specified for **0..5 V only**; raw 10 V Eurorack clocks/triggers remain unsupported.

Because `ClockSource` is shared by only two banks, coverage treats it as an explicit shared-clock scope rather than silently counting it as Classic/Core code.

## Bank-owned code

Every algorithm bank owns a directory under `domain/<bank>/`. Headers define the behavioral contract; implementation files contain state transitions; the corresponding `*AlgorithmMath` file exposes reusable pure primitives.

### Classic

`domain/classic/` contains Perlin, Brownian, Bezier and LFO. This remains the corrected baseline closest to the original Drift behavior and receives a separate Classic coverage contract.

### Organic

`domain/organic/` contains Fractal, Vector, Rain and Attractor. The bank mixes deterministic nonlinear motion, multiscale noise and stochastic impulses. `OrganicAlgorithmMath` carries the fixed-point contracts used to keep those algorithms AVR-feasible.

### Generative

`domain/generative/` contains Turing, Markov, Motif and Urn. Persistent sequence/state memory is central here. Changes to mutation probability, transition grammar, motif transformations or reinforcement decay are user-visible algorithm changes and must be reflected in engineering analyses and golden/property tests.

### Ambient

`domain/ambient/` contains Current, Anchor, Breath and Fog.

- Current advances three rationally related long-form phases and mixes them with constant-sum Texture weights.
- Anchor is an OU-inspired mean-reverting process using a signed Q1.15 state plus Q0.24 fractional reversion residual.
- Breath latches duration, amplitude and skew only at cycle rollover; reciprocal segment data is cached outside the sample hot path.
- Fog owns a fixed pool of four cloudlet voices and never allocates dynamically.

### Electronica

`domain/electronica/` contains Pump, Acid, Shuffle and Polymeter and the shared 30..240 BPM tempo mapping.

- Pump caches Texture-dependent recovery normalization.
- Acid advances a deterministic 16-step grammar with project-defined accent/slide predicates.
- Shuffle preserves total pair duration while moving the second onset from straight to 3:1 timing; Texture is pair-latched.
- Polymeter runs a 4-step anchor against 3/5/7/9-step secondary meters on a common sixteenth grid.

### Percussion

`domain/percussion/` contains Euclid, Repeat, Probability and Humanize plus phrase-level rhythm primitives.

The common phrase state latches 4/8/12/16-bar form only at phrase boundaries. Euclid, Repeat and Probability interpret the final phrase bar differently; Humanize intentionally does not add fill events. External-clock acquisition re-establishes a deterministic grid origin because the hardware has no dedicated Reset input.

Humanize is timing-sensitive: jitter is applied relative to the ideal eighth-note grid, never by adding random error to the previous actual event. This invariant prevents cumulative tempo drift.

### Dubstep / Bass

`domain/dubstep/` contains Wobble, Growl, Chop and Build. The bank uses a 70..280 BPM internal tempo with approximately 140 BPM at Speed midpoint and shares the Percussion external-clock contract.

- Wobble stores a deterministic rate phrase; the helper vocabulary is larger than the set actually selected by the released phrase tables.
- Growl is a compound CV contour, not an audio growl synthesizer. Its Q0.12 component weights are normalized so the integer weights sum exactly to unity.
- Chop constructs a bar-latched 16-step onset mask and uses an intentionally hard onset followed by hold/decay articulation.
- Build combines a multi-bar smoothstep macro rise with quarter/eighth/sixteenth/thirty-second micro-rate stages. Internal phrase wrap currently discards sub-sample phase overshoot; external quarter edges re-anchor timing.

## Application layer

`DriftRuntime` coordinates control acquisition, engine stepping and output publication through ports. It should not contain algorithm mathematics. Keeping this boundary narrow allows native integration/system tests to run the same domain behavior without Arduino/AVR dependencies.

## Ports and AVR adapters

Interfaces under `lib/fmd/include/fmd/ports/` express hardware services needed by the portable core. AVR implementations live outside the domain tree and may use direct register access where timing requires it.

Hardware-specific comments should explain **why** ordering, register access or ISR behavior is necessary. They should not duplicate datasheet prose or narrate obvious assignments.

## State-transition documentation rules

For each stateful algorithm header, Doxygen documentation must identify:

1. the musical/mathematical responsibility of the class;
2. the units and role of constructor dependencies;
3. what `step()` advances and what it returns;
4. when Texture/Speed values are sampled or latched;
5. the meaning of persistent state members;
6. non-obvious reset/acquisition/rollover behavior;
7. any hardware-safety limitation that software cannot enforce.

Pure math APIs additionally document fixed-point formats, clamping, exact endpoints and saturation behavior.

## Verification map

Source documentation is checked independently from algorithm correctness:

```bash
python scripts/check_code_documentation.py
```

The guard validates file metadata, source-line readability and minimum semantic Doxygen coverage for every algorithm class, every bank math API and the shared clock contract. It is intentionally lightweight enough to run without Doxygen.

The rendered API reference can be generated locally with:

```bash
doxygen Doxyfile
```

The generated HTML lives under `.doxygen/html/` and is not a release artifact. Doxygen warnings remain useful during local review, while the repository checker is the deterministic CI gate.

For mathematical correctness, coverage and release qualification, see [README_TESTING.md](../../README_TESTING.md). For design rationale, see the engineering analyses under `docs/analysis/`.

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
