# Fractal algorithm engineering analysis

## 1. Purpose and scope

Fractal is a project-defined algorithm in the optional Organic bank. It is designed to extend Drift's continuous gradient-noise character into several simultaneous time scales while remaining practical on an ATmega328P.

This document defines the mathematical contract first, then evaluates the concrete fixed-point implementation, resource cost and verification strategy. There is no Quinn Freedman Fractal implementation to preserve; upstream Drift remains relevant only because the new mode reuses the verified one-dimensional gradient-noise primitive and existing Speed mapping.

## 2. Mathematical foundations

The algorithm is a weighted sum of three continuous one-dimensional gradient-noise processes:

$$
F(t)=w_0 n(t)+w_1 n(4t)+w_2 n(16t).
$$

Each $n(\cdot)$ uses the same lattice-gradient interpolation and canonical quintic fade already verified for Classic Perlin. Texture changes the octave weights, not the underlying noise definition.

The integer weights always satisfy

$$
w_0+w_1+w_2=1024.
$$

At minimum Texture:

$$
(w_0,w_1,w_2)=(1024,0,0).
$$

At maximum Texture:

$$
(w_0,w_1,w_2)=(512,320,192).
$$

The constant sum makes Texture primarily a redistribution of scale content rather than an amplitude control.

The name **Fractal** refers to the explicit multi-scale/self-similar construction. It is not an assertion that the output is an exact fractional Brownian motion. Fractional Brownian motion has a specific stochastic self-similarity and covariance definition; see Mandelbrot and Van Ness, SIAM Review 10(4), 1968, DOI: <https://doi.org/10.1137/1010093>.

## 3. Reference algorithm

For each sample:

1. derive the base phase increment from Speed knob and Speed CV;
2. advance three gradient-noise octaves at $1\times$, $4\times$ and $16\times$ the base increment;
3. combine Texture knob and Texture CV in the saturated 10-bit domain;
4. derive constant-sum octave weights;
5. compute the weighted signed Q1.15 sum;
6. bias and scale the result to the 12-bit DAC domain.

A fixed random seed must reproduce the same octave-gradient sequence.

## 4. Relationship to upstream Drift

There is no upstream Fractal mode. The implementation deliberately reuses two already-verified Classic contracts:

- `perlinmath::gradientFromRandom()` and `perlinmath::segmentQ1F15()` for lattice gradient noise;
- `phaseIncrementFromControls()` for the established exponential Speed/CV mapping.

This reduces the amount of new mathematics that must be trusted while preserving a recognisably Drift-like response to Speed.

## 5. Behavioral analysis

Low Texture produces one broad continuous landscape. Increasing Texture adds smaller-scale deviations without removing the slower motion. Because all octaves are continuous at their lattice boundaries, adding detail does not create sample-and-hold discontinuities.

At very high Speed, octave multipliers are subject to the same discrete-time phase aliasing limitations as Classic Perlin's faster octave. That is a sampling limitation, not a unique Fractal defect.

## 6. Findings and classification

- **Sound-design choice:** the macro octave retains 50% of total weight at maximum Texture.
- **Implementation trade-off:** three octaves are used instead of an arbitrary octave count to bound CPU cost and state.
- **Terminology constraint:** the mode must not be documented as exact fractional Brownian motion.
- **Performance concern:** three gradient-noise evaluations per sample make Fractal the likely Organic-bank timing worst case; this requires timing qualification rather than assumption.

## 7. Improvement strategy

The first implementation intentionally avoids runtime-selectable octave counts, floating-point Hurst parameters or normalisation divisions. If later measurements show timing margin, additional scale laws can be evaluated, but any change should preserve a constant-gain control contract and receive a separate mathematical test.

## 8. Computational cost on ATmega328P

Per sample the mode performs:

- one shared exponential Speed lookup/mapping;
- three phase advances;
- up to three gradient rotations on lattice rollover;
- three quintic gradient-noise evaluations;
- three 16-by-16 weighted products and one constant division by 1024.

Persistent state is three 32-bit phases, six 16-bit gradients and one paired LFSR plus the table reference.

## 9. Optimization opportunities

- Keep octave ratios powers of two so phase-rate scaling is multiplication/shift friendly.
- Keep total weight 1024 so the final normalisation is a power-of-two operation after compiler optimisation.
- Avoid an independent RNG per octave; one deterministic paired LFSR supplies all gradient transitions.
- Measure before attempting to replace the already-verified quintic evaluator with a lower-order approximation.

## 10. Verification and test strategy

Required evidence:

- all 1024 Texture values produce weights summing exactly to 1024;
- macro weight is non-increasing while meso/detail weights are non-decreasing;
- endpoint weights match the documented values;
- weighted mixing matches an independent integer reference;
- fixed seeds are deterministic;
- long dynamic-control runs remain in 0..4095;
- Texture changes the trajectory for an otherwise identical seed and Speed;
- Organic timing-probe build meets the hardware sample deadline.

## 11. Engineering assessment

Fractal is a low-risk extension because the stochastic primitive is already verified and only the scale composition is new. Its principal engineering uncertainty is execution time, not mathematical stability.

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../../assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
