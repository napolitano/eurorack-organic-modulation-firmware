# Vector algorithm engineering analysis

## 1. Purpose and scope

Vector is a project-defined Organic-bank modulation mode whose state is explicitly two-dimensional. The aim is continuous, directed motion that is neither a conventional one-dimensional LFO nor a stochastic random walk.

## 2. Mathematical foundations

Two wrapped phase coordinates $\phi_x$ and $\phi_y$ form a state on a two-dimensional torus. Their velocities are cross-coupled through a bipolar triangle projection $T$:

$$
\dot{\phi}_x=\omega+\kappa T(\phi_y),
$$

$$
\dot{\phi}_y=\frac{3}{4}\omega-\kappa T(\phi_x).
$$

The implementation is a discrete Euler-style phase update with unsigned modular phase accumulators. Texture controls $\kappa$. Full-scale coupling is intentionally limited so the perturbation remains approximately within $\pm25\%$ of the corresponding base rate.

The scalar output is

$$
y=\frac{T(\phi_x)+T(\phi_y)}{2},
$$

then biased to the unipolar 12-bit output domain.

This vector field is a project-defined musical system, not an implementation of a named external dynamical model.

## 3. Reference algorithm

For each sample:

1. map Speed knob/CV to the first-axis base increment;
2. derive the second-axis base increment as $3/4$ of the first;
3. evaluate both current bipolar triangle projections;
4. scale cross-coupling by Texture knob/CV;
5. advance both wrapped phases with opposite coupling signs;
6. evaluate the new projections and average them to one DAC value.

## 4. Relationship to upstream Drift

There is no upstream Vector mode. It reuses the Classic exponential Speed mapping and the same four-channel `ControlFrame`, but its two-dimensional state and coupling law are new.

## 5. Behavioral analysis

At zero Texture the two axes move independently at related rates, already producing a more complex projection than a single LFO. As Texture rises, each coordinate changes the instantaneous velocity of the other. The phase space remains bounded by construction because both coordinates wrap modulo $2^{32}$.

The output triangle projection is continuous at phase wrap to within one fixed-point code, avoiding the hard discontinuity of a sawtooth projection.

## 6. Findings and classification

- **Sound-design choice:** the second axis runs at $3/4$ of the first base rate.
- **Sound-design choice:** opposite-sign coupling creates circulation rather than both axes accelerating together.
- **Numerical safety choice:** coupling is capped at about 25% of base rate, so Texture does not reverse an axis.
- **Implementation trade-off:** a piecewise-linear triangle field is preferred to sine/cosine because it avoids trigonometric LUTs or expensive runtime functions on AVR.

## 7. Improvement strategy

Alternative projection ratios or vector fields should be treated as new sound-design revisions, not silent refactors. Any future field must define boundedness, continuity and maximum arithmetic width before implementation.

## 8. Computational cost on ATmega328P

Per sample:

- one shared Speed mapping;
- four inexpensive triangle evaluations;
- two 16-by-16 cross-coupling products;
- two bounded 32-bit modulation products;
- two 32-bit phase additions;
- one two-axis output projection.

Persistent state is only two 32-bit phase accumulators plus the reference-table pointer/reference.

## 9. Optimization opportunities

The present model already uses power-of-two scaling for the second-axis rate and coupling conversion. The principal optimization rule is to avoid replacing the triangle field with floating-point trigonometry.

## 10. Verification and test strategy

Required evidence:

- triangle endpoints and branch monotonicity;
- zero Texture returns the uncoupled base increments exactly;
- full Texture keeps both increments positive and within the documented perturbation bound;
- scalar projection always remains in 0..4095;
- identical initial state/control sequences are deterministic;
- Texture measurably changes the trajectory;
- dynamic end-to-end Runtime tests pass under the Organic build.

## 11. Engineering assessment

Vector is computationally cheap and numerically bounded by construction. Its main review question is musical usefulness of the chosen field, which can be iterated later without affecting the compile-time bank architecture.

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../../assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
