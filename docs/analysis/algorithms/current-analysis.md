# Current algorithm engineering analysis

## 1. Purpose and scope

Current is the implemented Ambient-bank algorithm for deterministic long-form modulation. It combines several independent slow motions whose rates are intentionally non-harmonic, producing a trajectory with a much longer practical recurrence than a single LFO.

The initial working name was **Tide**, but this was rejected because Mutable Instruments already used **Tides** for a prominent Eurorack function generator. The official Tides manual documents cyclic/envelope waveform generation over a wide frequency range: <https://pichenettes.github.io/mutable-instruments-documentation/modules/tides_2018/manual/>. Current avoids unnecessary naming and provenance confusion.

There is no Quinn Freedman Current mode to preserve.

## 2. Mathematical foundations

Current's ideal model uses three phases with frequency ratios

$$
1:\sqrt{2}:\varphi,
$$

where

$$
\varphi=\frac{1+\sqrt5}{2}.
$$

Incommensurate frequencies are the defining setting for quasi-periodic forcing. Cubero, Casado-Pascual and Renzoni discuss the distinction between periodic and quasi-periodic response under irrational frequency ratios in *Irrationality and Quasiperiodicity in Driven Nonlinear Systems*, Physical Review Letters 112, 174102 (2014), DOI: <https://doi.org/10.1103/PhysRevLett.112.174102>.

A finite 32-bit phase machine cannot represent irrational ratios exactly. The implementation must therefore use fixed rational approximations and the documentation must say **quasi-periodic-inspired** or **very long practical recurrence**, not claim exact mathematical aperiodicity.

Each phase is projected through a soft bipolar triangle. Let $q(\phi)$ be the normal unipolar triangle in $[0,1]$ and

$$
S(x)=3x^2-2x^3.
$$

Then

$$
h(\phi)=2S(q(\phi))-1.
$$

The cubic has

$$
S'(0)=S'(1)=0,
$$

so the triangle's turning points become slope-continuous.

The combined signal is

$$
y=\frac{w_0h(\phi_0)+w_1h(\phi_1)+w_2h(\phi_2)}{1024}.
$$

Texture controls the constant-sum weights. With normalized Texture $\tau$:

$$
w_0=768-256\tau,
$$

$$
w_1=192+128\tau,
$$

$$
w_2=64+128\tau,
$$

so

$$
w_0+w_1+w_2=1024.
$$

Even at minimum Texture the two secondary currents remain present, which prevents the endpoint from collapsing to a plain Classic LFO.

## 3. Reference algorithm

At every 2.5 kHz processing sample:

1. derive the established Drift phase increment from Speed knob and CV;
2. divide that increment by sixteen for the Ambient macro time scale;
3. advance three phase accumulators using documented fixed-point approximations to $1$, $\sqrt2$ and $\varphi$ rate ratios;
4. evaluate the soft bipolar triangle for each phase;
5. combine Texture knob and Texture CV in the saturated 10-bit domain;
6. derive the three constant-sum weights;
7. form the weighted signed sum;
8. bias/scale the result into the 0..4095 DAC domain.

Startup phases should be fixed and intentionally offset so the three components do not begin in perfect phase alignment. They are deterministic, not random per boot.

## 4. Relationship to prior art and upstream Drift

Current is not an implementation of a named synthesizer algorithm. Its mathematics is the elementary superposition of independent oscillations with non-harmonic rate ratios.

There is no corresponding Quinn Freedman Drift mode. The implementation may reuse the corrected common Speed-to-phase conversion and a verified cubic smoothstep primitive, but the three-phase state and weight law are new.

Current must also remain distinct from Mutable Instruments Tides. No Tides firmware, circuit, panel model or waveform algorithm is used.

## 5. Behavioral analysis

Current has no random state. For fixed controls and initial phases it is fully deterministic.

The perceived evolution comes from beating between the three rates. At low Texture, the base current dominates and the secondary rates gently deform it. At high Texture, the secondary components carry half of the total weight and the output develops longer and less obviously periodic macro-shapes.

Because the output is a linear weighted combination of bounded projections with total weight 1024, its signed result is bounded by construction before DAC biasing.

The digital implementation eventually repeats because every phase accumulator and rational rate relationship is finite. The engineering goal is therefore not infinite non-repetition but a recurrence time far longer than musically relevant observation windows.

## 6. Findings and classification

- **Naming/provenance decision:** use Current, not Tide, to avoid confusion with Mutable Instruments Tides.
- **Musical design choice:** secondary oscillators remain audible at minimum Texture.
- **Mathematical constraint:** documentation must distinguish ideal irrational ratios from finite fixed-point approximations.
- **Numerical requirement:** ratio approximation must not overflow the 32-bit phase-increment domain at maximum Speed.
- **Sound-design choice:** the soft triangle is chosen for smooth turning points without a trigonometric LUT.
- **Compatibility status:** there is no upstream behavior to preserve.

## 7. Improvement strategy

The first implementation should keep the three fixed rate ratios and fixed phase offsets. Random phase resets, Texture-dependent frequency ratios, additional voices or sine tables would make the behavior harder to reason about and should be separate revisions.

The implementation freezes the rate approximations at `362/256` for $\sqrt2$ and `414/256` for $\varphi$. Both use multiply/add/shift arithmetic and are tested against the ideal ratios. Startup phases are fixed at `0`, `0x55555555` and `0xAAAAAAAA`.

## 8. Computational cost on ATmega328P

Per sample the expected work is:

- one shared Speed mapping;
- one /16 phase-rate scaling;
- two fixed-ratio increment calculations in addition to the base rate;
- three 32-bit phase additions;
- three triangle/smoothstep evaluations;
- three weighted products and one power-of-two normalization.

Persistent state is three 32-bit phase accumulators.

This is likely one of the two Ambient timing worst cases because all three voices are evaluated every sample.

## 9. Optimization opportunities

- Choose rational rate approximations with power-of-two denominators.
- Keep weight total exactly 1024 so normalization is a shift.
- Reuse the existing monotone cubic smoothstep implementation if its domain and rounding contract match.
- Avoid 64-bit division in rate conversion; use bounded multiply/shift approximations with independently verified error.
- Recompute Texture weights only when the effective 10-bit Texture code changes if measurement shows this is useful.

## 10. Verification and test strategy

Required evidence:

- every Texture code produces weights summing exactly to 1024;
- $w_0$ is monotone non-increasing and $w_1,w_2$ are monotone non-decreasing;
- weight endpoints match the documented values;
- soft-wave endpoints, midpoint, branch monotonicity and turning-point continuity match an independent reference;
- fixed-point frequency ratios remain within a documented relative error from $\sqrt2$ and $\varphi$;
- no ratio calculation overflows at maximum phase increment;
- fixed initial state and controls are bit-for-bit deterministic;
- output remains inside 0..4095 for dense phase/Texture sweeps;
- Current differs measurably from a single LFO over representative long windows;
- AVR timing-probe build remains below the 400 µs sample deadline.

A recurrence test should validate the chosen finite-state implementation, not assert true mathematical aperiodicity.

## 11. Musical assessment

**Musical value: very high.**

Current is useful when a patch needs motion that feels intentional and coherent but does not advertise a short repeated cycle. Unlike stochastic modulation, it can be used where exact deterministic recall matters.

Particularly strong uses are:

- slow filter and resonance movement;
- wavetable/wavefolder scanning;
- stereo position or crossfade control;
- reverb/delay parameter movement;
- subtle macro modulation in drones and long-form generative patches.

Its central musical strength is **long deterministic evolution without randomness**.

The main limitation is conceptual honesty: the AVR implementation is ultimately periodic. The value is the length and complexity of the practical recurrence, not mathematical infinity.

## 12. Engineering assessment

Current is mathematically bounded and conceptually clean. The two areas requiring real measurement are execution time and fixed-point ratio quality. Neither is a reason to reject the mode. The host mathematical suite now verifies constant-sum weights, ratio accuracy, soft-triangle extrema and deterministic bounded output; AVR timing qualification remains a release requirement.

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../../assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
