# Organic algorithm bank — engineering design

## Status and scope

The Organic bank is an **optional compile-time alternative** to Drift's original four-algorithm bank. It does not replace or modify the Classic algorithms. A firmware image contains one bank only, selected by `FMD_ALGORITHM_BANK`; the existing two rear configuration inputs continue to select one of four slots at power-up.

The default build remains the Classic bank. This is deliberate: existing hardware behavior and the original `0.1.0` firmware remain the compatibility baseline, while experimental algorithms can evolve without consuming flash or SRAM in Classic images.

## Hardware constraint: only two algorithm controls are digital

The ATmega328P reads **Speed**, **Texture**, **Speed CV** and **Texture CV**. The front-panel **Attenuation** control is after the DAC in the analog output path and is not measurable by the firmware. Consequently, an algorithm may describe Attenuation musically as *depth* or *intensity*, but it cannot use that knob as an internal parameter such as pitch spread, probability or seed.

This constraint is especially important for future pitch-oriented banks: analog output attenuation would scale the entire 1 V/oct signal and therefore cannot act as a firmware pitch-spread control.

## Compile-time selection

`lib/fmd/include/fmd/config/AlgorithmBankConfig.h` defines two banks:

```text
FMD_BANK_CLASSIC = 0
FMD_BANK_ORGANIC = 1
```

If `FMD_ALGORITHM_BANK` is not defined, the firmware compiles as Classic. PlatformIO provides explicit Organic environments, for example:

```bash
pio run -e nanoatmega328new_organic
pio test -e native_organic
```

The `DriftEngine` member layout is conditional at compile time. This prevents four inactive-bank algorithm objects from occupying SRAM merely because multiple banks exist in the source tree.

## DIP mapping

The electrical CONFIG truth table is unchanged. The user-facing mapping below uses the **physical rear DIP numbering** documented in the manual:

| Rear DIP 1 | Rear DIP 2 | Classic bank | Organic bank |
|---|---|---|---|
| OFF | OFF | Perlin | **Fractal** |
| ON | OFF | Brownian | **Vector** |
| OFF | ON | Bézier | **Rain** |
| ON | ON | LFO | **Attractor** |

A bank change therefore changes only what occupies each of the four existing slots; it does not introduce another runtime UI state.

## Control contract

| Algorithm | Speed | Texture | Texture CV | Analog Attenuation |
|---|---|---|---|---|
| **Fractal** | traversal speed | roughness / fine-scale weight | adds roughness | modulation depth |
| **Vector** | flow speed | cross-axis coupling | adds coupling | modulation depth |
| **Rain** | drop-tail speed / decay | event density | adds density | output intensity |
| **Attractor** | travel rate between map points | Hénon structure parameter $a$ | adds to $a$ control | modulation depth |

Speed CV contributes to Speed in every Organic mode. Texture CV contributes to the same secondary parameter as the Texture knob. This keeps the alternate bank predictable from the existing panel labeling.

## Fractal

Fractal is a procedural multi-scale gradient-noise sum built from the same verified one-dimensional gradient primitive used by Perlin. Three octaves run at relative rates $1$, $4$ and $16$:

$$
F(t)=w_0 n(t)+w_1 n(4t)+w_2 n(16t)
$$

with

$$
w_0+w_1+w_2=1.
$$

Texture redistributes a fixed total integer weight of 1024 from the macro octave toward the two finer scales. At the endpoints the weights are:

$$
(1024,0,0) \rightarrow (512,320,192).
$$

This preserves large-scale motion even at maximum Texture instead of turning the algorithm into only high-frequency noise.

The implementation is intentionally described as **procedural fractal noise**, not as an exact fractional Brownian-motion process. Fractional Brownian motion has a specific stochastic definition and covariance/self-similarity contract; the classic reference is Mandelbrot and Van Ness, *Fractional Brownian Motions, Fractional Noises and Applications*, SIAM Review 10(4), 1968, DOI: <https://doi.org/10.1137/1010093>.

## Vector

Vector is a project-defined two-dimensional flow on a torus. Two phase coordinates have different base velocities and perturb one another through a bipolar triangle projection:

$$
\dot{\phi}_x = \omega + \kappa T(\phi_y)
$$

$$
\dot{\phi}_y = \frac{3}{4}\omega - \kappa T(\phi_x).
$$

The discrete implementation bounds full-scale cross-coupling to approximately $\pm25\%$ of each axis's base increment, so neither axis reverses solely because Texture is increased. The scalar output is the average of the two bipolar projections, remapped to 12-bit unipolar DAC space.

The goal is not random noise. It is a continuous path through a two-dimensional state space: uncoupled settings produce two related but non-identical motions, while increasing Texture bends each axis according to the current state of the other.

## Rain

Rain is a discrete-time, shot-noise-inspired process. A Bernoulli event decision is made once per 2.5 kHz processing sample. Texture/Density maps to a 16-bit event cutoff through

$$
C(d)=\left\lfloor\frac{d^2}{64}\right\rfloor,
\qquad 0\le d\le1023.
$$

An event occurs when a uniform 16-bit random word is below $C(d)$. The quadratic mapping provides much finer physical control over sparse events than a linear full-range probability mapping.

Each event adds a random positive impulse to an aggregate envelope. Between events the state follows a leaky decay:

$$
E_{n+1}=E_n-\alpha E_n,
$$

implemented with a retained fractional residual so low-level tails cannot freeze solely because integer truncation makes one decay step smaller than one code.

This is a discrete Bernoulli approximation to the impulse-arrival idea commonly associated with shot noise; it is not claimed to be a continuous-time Poisson process. A foundational treatment of random/shot noise is S. O. Rice, *Mathematical Analysis of Random Noise*, Bell System Technical Journal 23(3), 1944, DOI: <https://doi.org/10.1002/j.1538-7305.1944.tb00874.x>.

## Attractor

Attractor uses Michel Hénon's two-dimensional map:

$$
x_{n+1}=1-a x_n^2+y_n
$$

$$
y_{n+1}=b x_n.
$$

The original Hénon paper studies the well-known $a=1.4$, $b=0.3$ case: M. Hénon, *A two-dimensional mapping with a strange attractor*, Communications in Mathematical Physics 50, 69–77 (1976), DOI: <https://doi.org/10.1007/BF01608556>.

Drift fixes $b=0.30$ and maps Texture continuously over

$$
1.20\le a\le1.40.
$$

This range remains bounded from the chosen origin in the verified fixed-point implementation, but it must **not** be described as a monotonic "chaos amount" control. The Hénon family contains periodic windows as parameters change. Musically, that is useful: Texture navigates different structures rather than merely turning one effect up.

Speed determines how quickly the map advances. To avoid sample-and-hold jumps, Drift linearly travels from the previous $x$ coordinate to the next $x$ coordinate during each Speed cycle.

## Fixed-point and real-time constraints

The Organic bank follows the same engineering constraints as Classic:

- no heap allocation in the portable DSP path;
- no floating-point arithmetic in the AVR hot path;
- 12-bit output invariant for every algorithm;
- deterministic output for a fixed stochastic seed;
- strict host compilation with conversion/sign/shadow warnings promoted to errors;
- AVR application flash budget at or below 85%;
- static SRAM budget at or below 65%;
- timing-probe builds for the 2.5 kHz processing deadline.

Hénon iteration is performed only when an interpolation segment rolls over. Vector uses only bounded 32-bit cross-coupling arithmetic. Rain uses one random event draw per sample and a second draw only when an event actually occurs. Fractal is expected to be the most computationally expensive Organic mode because it evaluates three gradient-noise octaves each sample; the dedicated Organic timing image exists to qualify this on hardware/CI rather than infer timing from source inspection alone.

## Verification contract

The Organic bank adds dedicated tests for:

- exact DIP-slot mapping under the compile-time bank selector;
- Fractal constant-sum octave weights, deterministic output and DAC bounds;
- Vector triangle continuity/monotonic branches, bounded cross-coupling and deterministic flow;
- Rain density/decay monotonicity, impulse bounds, saturating accumulation, fractional tail decay and seed determinism;
- Hénon equation reference points, complete Texture-range boundedness, interpolation and DAC mapping;
- full end-to-end Runtime/LED/DAC invariants under `native_organic`;
- Organic native coverage, sanitizers, AVR resource limits and timing-image compilation in CI.

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../../assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
