# Rain algorithm engineering analysis

## 1. Purpose and scope

Rain is a project-defined Organic-bank event process intended to create isolated drops, showers and dense stochastic activity using the existing Drift panel. Speed controls event-tail time scale, Texture controls Density, and the hardware Attenuation control acts naturally as final Intensity.

## 2. Mathematical foundations

The conceptual model is shot noise: random events add impulses whose tails overlap. A foundational random-noise treatment is S. O. Rice, *Mathematical Analysis of Random Noise*, Bell System Technical Journal 23(3), 1944, DOI: <https://doi.org/10.1002/j.1538-7305.1944.tb00874.x>.

Drift uses a deliberately discrete model. Once per 2.5 kHz sample, a 16-bit uniform random value $r$ is compared with a Texture-derived cutoff:

$$
C(d)=\left\lfloor\frac{d^2}{64}\right\rfloor.
$$

An event occurs when

$$
r<C(d).
$$

This is Bernoulli event generation in discrete time; it is not claimed to be an exact continuous-time Poisson process.

The aggregate envelope decays approximately as

$$
E_{n+1}=E_n-\alpha E_n,
$$

where $\alpha$ increases with Speed. A fractional residual retains sub-code decay so integer quantisation cannot create a permanent one-code tail.

## 3. Reference algorithm

For each sample:

1. combine Speed knob and CV to derive $\alpha$;
2. decay the current 16-bit aggregate envelope while retaining the fractional remainder;
3. combine Texture knob and CV to derive event cutoff $C(d)$;
4. draw one random event word;
5. if an event occurs, draw a second random word for impulse amplitude and add it with saturation;
6. emit the upper 12 bits of the aggregate envelope.

## 4. Relationship to upstream Drift

There is no upstream Rain mode. The paired LFSR is reused as the project's deterministic pseudo-random source, which keeps seed behavior consistent with other stochastic Drift modes.

## 5. Behavioral analysis

The quadratic Density law is intentional. A linear 0..65535 probability mapping would devote too little panel travel to sparse drops and would approach one event per sample too quickly. The selected law reaches a maximum cutoff of 16352, roughly one quarter of the random domain, while giving much finer control near zero.

Dense events overlap in the same envelope rather than being rendered as independent voices. This keeps state and CPU cost small while producing a natural transition from isolated impulses to a continuous downpour-like voltage.

## 6. Findings and classification

- **Sound-design choice:** output is unipolar because each event adds positive energy.
- **Sound-design choice:** event density uses a quadratic panel law.
- **Implementation trade-off:** overlapping events share one aggregate envelope rather than multiple per-drop envelope objects.
- **Numerical safety choice:** impulse accumulation saturates at the 16-bit state rail.
- **Numerical requirement:** fractional decay must be retained to avoid a truncation deadband.

## 7. Improvement strategy

Possible future extensions include multiple decay populations or bipolar rain, but those should be separate behavioral revisions. The first implementation deliberately keeps Density and Speed orthogonal: Density decides *when* events happen; Speed decides *how long their energy remains*.

## 8. Computational cost on ATmega328P

Every sample performs one LFSR advance, one 16-by-16 decay multiply and simple threshold arithmetic. A second LFSR advance occurs only on an event. There is no exponential table lookup, 64-bit division or per-drop dynamic state.

Persistent state is one 16-bit envelope, one 16-bit residual and one paired LFSR.

## 9. Optimization opportunities

The hot path is already small. Any optimization should preserve the fractional residual and exact event law. Replacing the quadratic cutoff with a lookup table would save little while consuming flash/PROGMEM.

## 10. Verification and test strategy

Required evidence:

- cutoff is monotonic and matches exact reference values;
- decay coefficient is monotonic with exact endpoints;
- all 65536 random words map to the documented impulse range;
- saturating addition is exact;
- a one-code tail eventually reaches zero at minimum decay;
- zero Density remains silent from reset;
- fixed seed/control sequences are deterministic;
- long runs remain in the 12-bit DAC domain.

## 11. Engineering assessment

Rain has the smallest mathematical and CPU risk of the new stochastic modes. Its main tuning surface is musical calibration of Density and decay endpoints, which can be adjusted later with explicit regression vectors.

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../../assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
