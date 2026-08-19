# Anchor algorithm engineering analysis

## 1. Purpose and scope

Anchor is a proposed Ambient-bank stochastic modulation mode designed around **mean reversion**. It should wander continuously but retain a statistical home around the DAC midpoint rather than behaving like a random walk that must eventually be constrained by rails.

The mathematical inspiration is the Ornstein-Uhlenbeck family of mean-reverting stochastic processes. Drift's embedded implementation must remain honest about any approximation used for Gaussian noise or boundary handling.

There is no Quinn Freedman Anchor mode to preserve.

## 2. Mathematical foundations

The continuous Ornstein-Uhlenbeck process is commonly written

$$
dX_t=-\theta(X_t-\mu)dt+\sigma dW_t.
$$

For Drift,

$$
\mu=0
$$

in the internal bipolar domain. The deterministic term pulls the process toward the centre while the noise term keeps it moving.

The historical reference is G. E. Uhlenbeck and L. S. Ornstein, *On the Theory of the Brownian Motion*, Physical Review 36, 823 (1930), DOI: <https://doi.org/10.1103/PhysRev.36.823>.

For fixed sample interval $\Delta t$, an exact discrete transition for the Gaussian OU process can be written

$$
X_{n+1}=\mu+a(X_n-\mu)+s\sqrt{1-a^2}Z_n,
$$

with

$$
a=e^{-\theta\Delta t},
$$

$Z_n\sim\mathcal N(0,1)$ and $s$ the desired stationary standard deviation.

This formulation exposes the key control-design requirement: if Speed changes $a$, the innovation scale must change with $\sqrt{1-a^2}$ if Texture is intended to represent roughly the same long-run spread.

## 3. Proposed Drift contract

Speed controls the mean-reversion/correlation rate $\theta$ using the Ambient macro-time scale. Texture controls target excursion $s$.

The ideal control semantics are therefore orthogonal:

- increasing Speed makes the process forget deviations and generate new movement faster;
- increasing Texture increases the typical distance from centre;
- changing Speed alone should not massively change the stationary excursion width.

The DAC mapping is centred:

$$
y=2048+KX,
$$

with saturation only as a final safety measure.

The initial state is exactly centre.

## 4. Embedded innovation model

Generating an exact continuous Gaussian random variable cheaply on ATmega328P is not automatic. Three implementation families are technically defensible:

1. a compact precomputed inverse-normal table driven by one uniform RNG word;
2. a small bounded approximation such as a triangular innovation;
3. a sum-of-uniforms approximation.

The first option best preserves the OU interpretation but consumes flash and interpolation work. The second is cheapest and bounded but makes the process **OU-inspired AR(1)** rather than exact Gaussian OU.

The implementation analysis must choose one explicitly before code is written. The firmware and user documentation must use terminology matching that choice.

## 5. Relationship to Brownian and upstream Drift

Classic Brownian updates its position through stochastic motion and smoothing. Its defining musical behaviour is local random movement, not attraction to a statistical centre.

Anchor adds an explicit restoring term. Two trajectories at the same non-zero position differ conceptually:

- Brownian has no inherent reason to move toward centre;
- Anchor has a deterministic expected drift toward centre.

This is the essential non-duplication test.

There is no corresponding upstream Anchor mode. Reuse is limited to Drift's control frame, RNG infrastructure, fixed-point helpers and DAC output path.

## 6. Behavioral analysis

At Texture zero, Anchor should decay smoothly to centre and remain there. This gives a useful deterministic boundary condition.

At low Texture, it performs small, correlated excursions around centre. At high Texture, excursions become broader but should still exhibit a visible restoring tendency.

At low Speed, the correlation time is long and the motion can spend many seconds or minutes on one side of centre. At high Speed, the same statistical spread is traversed more rapidly.

If innovation scaling is not compensated for Speed, this control model breaks: slow settings can become nearly static while fast settings become excessively noisy, or vice versa. Preserving approximate stationary spread is therefore a mathematical requirement rather than mere tuning.

## 7. Findings and classification

- **Mathematical requirement:** mean reversion must be explicit and testable from any non-zero state.
- **Control requirement:** Texture should primarily control spread while Speed controls correlation time.
- **Implementation decision still required:** exact/approximate Gaussian innovation method.
- **Terminology constraint:** if the innovation is not Gaussian, do not call the implementation an exact Ornstein-Uhlenbeck process.
- **Numerical requirement:** output saturation must not become the normal mechanism defining the stationary distribution.
- **Compatibility status:** no upstream behavior exists to preserve.

## 8. Improvement strategy

The first implementation should prioritize control orthogonality and bounded numerical behavior over exact stochastic purity.

A practical sequence is:

1. select a documented innovation distribution;
2. generate offline coefficient tables for $a$ and innovation scale if needed;
3. keep the nominal maximum spread low enough that final DAC saturation remains rare;
4. verify stationary moments across several Speed values;
5. only then tune the Texture-to-spread law musically.

A nonlinear centre position or movable mean would be a different feature and is not part of the first bank contract.

## 9. Computational cost on ATmega328P

Every sample is expected to require:

- conversion of Speed to a mean-reversion coefficient or table index;
- one RNG innovation;
- one state-retention multiply;
- one innovation-scale multiply;
- one signed accumulation;
- final bias/saturation into 12-bit DAC range.

Persistent state is one signed process value plus RNG state and any cached coefficients.

A table-based Gaussian approximation would add flash use and interpolation work but should still be much cheaper than multi-octave gradient noise if designed carefully.

## 10. Optimization opportunities

- Precompute coefficient pairs $(a,b)$ indexed by a reduced Speed domain.
- Use a power-of-two fixed-point format for state and coefficient products.
- Recompute coefficient lookup only when the effective Speed code changes.
- If an inverse-normal table is selected, reuse interpolation infrastructure already proven elsewhere rather than introducing floating point.
- Keep all stochastic operations deterministic for a fixed seed.

## 11. Verification and test strategy

Required deterministic tests:

- Texture zero from any signed non-zero state moves monotonically toward centre and never away from it;
- exact centre plus Texture zero remains exactly centre;
- coefficient endpoints and monotonicity match the documented mapping;
- identical seed and control sequence are bit-for-bit deterministic;
- DAC output stays in 0..4095;
- extreme signed states cannot overflow intermediate fixed-point arithmetic.

Required statistical tests:

- mean remains close to centre over a sufficiently long fixed-seed ensemble/run;
- measured stationary spread increases monotonically with Texture;
- representative low/mid/high Speed settings produce broadly comparable stationary spread at a fixed Texture;
- autocorrelation decays faster as Speed increases;
- saturation/rail-hit rate stays below an explicitly chosen tolerance under normal Texture settings.

Statistical tolerances must be derived from the selected innovation model, not copied from an ideal Gaussian process if the implementation uses a different distribution.

## 12. Musical assessment

**Musical value: very high.**

Anchor addresses one of the most useful forms of ambient modulation: movement that explores but does not permanently escape its useful range.

Strong applications include:

- filter cutoff around a carefully tuned sweet spot;
- stereo position around centre;
- reverb mix or decay around a nominal setting;
- subtle oscillator timbre and FM-index animation;
- feedback parameters where unbounded wandering would be musically dangerous.

Its core musical virtue is **freedom with memory of home**.

Compared with Brownian, the result should feel less like a drunk walk and more like a suspended object repeatedly displaced from equilibrium.

## 13. Engineering assessment

Anchor is musically strong but has the highest mathematical-definition risk in the Ambient bank because careless discretization can make Speed alter amplitude as much as time scale. The analysis must therefore be treated as a real contract: mean reversion, stationary-spread behavior and innovation model all need independent verification before release.

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../../assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
