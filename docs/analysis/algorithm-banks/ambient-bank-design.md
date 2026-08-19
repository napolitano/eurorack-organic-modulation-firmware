# Ambient bank architecture and control contract

## 1. Purpose

The proposed **Ambient** bank is intended to extend Drift into a different musical time domain: slow, continuous modulation that can sustain interest for long patches without behaving like a conventional repeating LFO or another form of random walk.

Classic already provides gradient noise, bounded Brownian motion, random Bézier destinations and a periodic LFO. Organic adds multi-scale noise, coupled two-dimensional flow, stochastic impulses and a deterministic nonlinear attractor. Generative adds discrete musical memory through shift-register loops, Markov state transitions, phrase mutation and reinforced preference.

Ambient should therefore not be four slower versions of existing algorithms. Its four modes are selected around four different forms of **long-form continuity**:

- **Current** — deterministic quasi-periodic superposition with extremely long practical recurrence;
- **Anchor** — stochastic movement with an explicit statistical home position;
- **Breath** — recurrent smooth swells whose cycle shape evolves from one cycle to the next;
- **Fog** — a bipolar cloud of overlapping smooth stochastic pulses.

The bank is project-defined. There is no Quinn Freedman implementation of these four modes to preserve.

## 2. Naming decision: Current rather than Tide

The initial working name for the first algorithm was **Tide**. That name is rejected for the user-facing design because Mutable Instruments already used **Tides** for a well-known Eurorack digital function generator capable of cyclic and envelope operation. The official Mutable Instruments documentation describes Tides as a waveform/function generator spanning very slow cyclic modulation through audio rates: <https://pichenettes.github.io/mutable-instruments-documentation/modules/tides_2018/manual/>.

Using a near-identical name for another Eurorack modulation algorithm would create unnecessary provenance and product confusion. **Current** retains the intended water/flow metaphor without implying a relationship to Mutable Instruments.

## 3. Bank slot mapping

The Ambient firmware image uses the same four rear-DIP slots as every other bank:

| Slot | DIP 1 | DIP 2 | Ambient algorithm |
|---:|---|---|---|
| 0 | OFF | OFF | Current |
| 1 | ON | OFF | Anchor |
| 2 | OFF | ON | Breath |
| 3 | ON | ON | Fog |

The bank itself remains a compile-time firmware choice. The rear DIP switches select only one of the four algorithms in the flashed bank and are sampled at startup.

## 4. Shared Ambient time scale

The existing Drift Speed mapping spans roughly 1/40 Hz to 100 Hz in the Classic LFO path. That range is deliberately too wide and too fast for a bank whose identity is long-form modulation.

Ambient therefore defines a common macro-time base by dividing the established phase rate by sixteen:

$$
f_A=\frac{f_{Drift}}{16}.
$$

This gives an intended base range of approximately

$$
\frac{1}{640}\text{ Hz}\;\text{to}\;6.25\text{ Hz},
$$

corresponding to characteristic periods from about 10 minutes 40 seconds down to 160 ms.

The exact meaning of this base differs by algorithm:

- Current: base oscillator frequency;
- Anchor: mean-reversion/correlation rate;
- Breath: nominal swell-cycle frequency;
- Fog: inverse cloud duration/time scale.

The division by sixteen is part of the proposed bank contract rather than an accidental implementation detail. It also has a cheap fixed-point implementation as a phase-increment shift, subject to the existing minimum-rate non-zero invariant.

## 5. Shared control contract

| Algorithm | Speed | Texture | Texture CV | Analog Attenuation |
|---|---|---|---|---|
| **Current** | macro oscillation rate | contribution of secondary incommensurate motions | adds complexity | output depth |
| **Anchor** | mean-reversion/correlation rate | stationary excursion/spread | adds freedom | output depth |
| **Breath** | nominal swell-cycle rate | cycle-to-cycle irregularity | adds irregularity | output depth |
| **Fog** | cloud/pulse duration scale | expected cloud occupancy/density | adds density | output depth |

Texture knob and Texture CV are summed and saturated in the normal 10-bit Drift control domain. Attenuation remains a post-DAC analogue control and is not available to firmware.

## 6. Current: quasi-periodic long-form motion

Current is a deterministic sum of three independent soft oscillators. In the ideal mathematical model their frequency ratios are

$$
1:\sqrt{2}:\varphi,
$$

where

$$
\varphi=\frac{1+\sqrt{5}}{2}.
$$

The irrational ratios prevent a finite exact common period in the continuous idealization. A signal driven by incommensurate frequencies is the standard setting for quasi-periodicity; see D. Cubero, J. Casado-Pascual and F. Renzoni, *Irrationality and Quasiperiodicity in Driven Nonlinear Systems*, Physical Review Letters 112, 174102 (2014), DOI: <https://doi.org/10.1103/PhysRevLett.112.174102>.

A finite integer phase machine cannot represent irrational ratios exactly, so the AVR implementation must use documented rational fixed-point approximations. The user-facing claim is therefore **quasi-periodic-inspired motion with a very long practical recurrence**, not mathematical aperiodicity.

Each phase is converted to a bipolar soft-triangle projection. Let $q(\phi)$ be a unipolar triangle in $[0,1]$ and let

$$
S(x)=3x^2-2x^3.
$$

Then

$$
h(\phi)=2S(q(\phi))-1.
$$

This retains cheap piecewise-linear phase geometry while giving zero slope at the extrema.

Texture redistributes constant total gain among the three motions. With normalized Texture $\tau\in[0,1]$, the intended integer weights are

$$
w_0=768-256\tau,
$$

$$
w_1=192+128\tau,
$$

$$
w_2=64+128\tau,
$$

with

$$
w_0+w_1+w_2=1024.
$$

At low Texture the fundamental current dominates but secondary motions are already present; at high Texture the slower visual impression becomes a more complex beat pattern. This deliberately avoids making the zero-Texture endpoint merely a duplicate of Classic LFO.

## 7. Anchor: bounded mean-reverting stochastic motion

Anchor is based on the mean-reversion principle associated with the Ornstein-Uhlenbeck process. The continuous ideal model is

$$
dX_t=-\theta X_t\,dt+\sigma\,dW_t,
$$

where the deterministic drift pulls the process toward zero while the stochastic term continuously perturbs it. The historical foundation is G. E. Uhlenbeck and L. S. Ornstein, *On the Theory of the Brownian Motion*, Physical Review 36, 823 (1930), DOI: <https://doi.org/10.1103/PhysRev.36.823>.

For fixed sampling interval, the exact Gaussian OU transition can be written

$$
X_{n+1}=aX_n+s\sqrt{1-a^2}\,Z_n,
$$

where $a=e^{-\theta\Delta t}$, $Z_n$ is standard normal and $s$ is the stationary standard deviation.

Drift's embedded implementation may approximate the Gaussian innovation with a bounded, documented fixed-point distribution, but if it does so the mode must be described as **OU-inspired mean reversion**, not as an exact Gaussian OU process.

Speed controls $\theta$ through the Ambient macro-time scale. Texture controls target stationary spread $s$. The critical design requirement is that changing Speed should primarily change correlation time, not accidentally collapse or explode the long-run excursion width. Innovation scaling must therefore compensate for the change in $a$.

Unlike Brownian, Anchor has an explicit statistical home. It can leave the centre for long periods, but the process does not have Brownian's unconstrained tendency to wander until bounded by implementation rails.

## 8. Breath: stochastic recurrent swell

Breath is a project-defined cycle process rather than an implementation of a named physiological model. Every cycle starts at baseline, rises smoothly to a cycle-specific peak, and returns smoothly to baseline.

For normalized phase $p\in[0,1)$ and cycle skew $s\in(0,1)$, define

$$
E(p;s)=
\begin{cases}
S(p/s), & p<s,\\
1-S((p-s)/(1-s)), & p\ge s,
\end{cases}
$$

where again

$$
S(x)=3x^2-2x^3.
$$

This gives zero slope at baseline and peak, avoiding hard corners.

At each cycle boundary the algorithm draws a new amplitude, duration variation and skew variation. Texture scales all three deviations from a stable nominal breath. At Texture zero, the cycle is deterministic. Increasing Texture makes successive swells differ without changing the fundamental fact that every cycle returns to baseline.

The proposed maximum-Texture bounds are deliberately moderate:

- nominal-period multiplier: 0.75..1.25;
- peak amplitude: 0.65..1.00 of full algorithmic scale;
- peak location/skew: 0.25..0.50 of the cycle.

These are project musical parameters and require listening validation before implementation is frozen.

Breath is distinct from Bézier because it does not choose arbitrary random destinations. Its invariant topology is always baseline → peak → baseline; randomness modifies the shape of the next complete gesture rather than the destination graph itself.

## 9. Fog: smooth bipolar stochastic cloud

Fog uses a bounded superposition of stochastic pulse voices. Conceptually this belongs to the family of shot-noise or filtered-Poisson models in which a signal is represented as a superposition of pulses arriving at random times. A modern analysis is A. Theodorsen, O. E. Garcia and M. Rypdal, *Statistical properties of a filtered Poisson process with additive random noise*, Physica Scripta 92 (2017), preprint: <https://arxiv.org/abs/1609.01607>.

The Drift mode deliberately differs from a classical filtered Poisson process in three ways:

1. event decisions occur on the discrete 2.5 kHz processing grid;
2. only a fixed small number of pulse voices exists;
3. pulse amplitudes may be positive or negative around the DAC midpoint.

Each cloudlet uses a compact smooth polynomial kernel

$$
g(u)=16u^2(1-u)^2,
\qquad 0\le u\le1,
$$

and $g(u)=0$ outside that interval. The kernel has zero amplitude and zero first derivative at both boundaries and reaches one at $u=1/2$.

For active events $i$,

$$
x(t)=\sum_i A_i g\left(\frac{t-t_i}{D}\right),
$$

with random signed amplitude $A_i$. The DAC output is the centered and saturated projection of $x(t)$.

Speed controls the common cloud duration $D$. Texture controls target expected occupancy rather than raw per-sample probability. This is important: if pulse duration changes, event probability should be compensated so a chosen Texture setting represents roughly the same visual/musical cloud density across Speed.

A fixed voice cap, proposed initially as four concurrent cloudlets, bounds CPU and SRAM. If all voices are active, additional event attempts are dropped rather than allocating memory or changing the real-time cost.

## 10. Duplication audit

| Existing algorithm | Why Ambient does not duplicate it |
|---|---|
| Perlin | stochastic gradient landscape; Current is deterministic multi-rate superposition, Anchor has explicit mean reversion |
| Brownian | bounded random walk with local positional memory; Anchor has a restoring force and stationary home distribution |
| Bézier | arbitrary random endpoints joined by curves; Breath always returns to baseline and randomizes whole-cycle gesture parameters |
| LFO | single exactly periodic oscillator; Current deliberately combines independent non-harmonic rates and Breath varies cycle parameters |
| Fractal | stochastic multi-scale gradient noise; Current uses deterministic independent phases rather than noise octaves |
| Vector | two coordinates cross-couple their instantaneous velocities; Current's oscillators do not interact and are linearly superposed |
| Rain | positive abrupt stochastic impulses feeding one shared decay envelope; Fog uses bipolar smooth finite-support voices with explicit overlap |
| Attractor | deterministic nonlinear map; none of the Ambient modes iterates a chaotic map |
| Turing | mutating discrete loop; Ambient modes are continuous and do not retain a binary phrase |
| Markov | discrete state vocabulary and transition grammar; Ambient has no categorical state grammar |
| Motif | explicit phrase edits at cycle boundaries; Breath varies gesture parameters but does not reorder a stored phrase |
| Urn | reinforced categorical preferences; Fog events have no self-reinforcing state probability |

The nearest conceptual overlaps are Current↔LFO/Vector, Anchor↔Brownian, Breath↔LFO/Bézier and Fog↔Rain. Those boundaries must remain explicit in implementation and tests.

## 11. Musical assessment

| Algorithm | Musical value | Strongest musical role | Main limitation |
|---|---|---|---|
| **Current** | very high | slowly evolving deterministic motion that can run for long periods without an obvious short loop | still deterministic; finite digital phases ultimately repeat |
| **Anchor** | very high | subtle modulation that wanders but repeatedly returns to a useful centre region | aggressive Texture can still require saturation/bound handling |
| **Breath** | high | evolving swells for amplitude, timbre, reverb depth and macro-dynamics | low Texture approaches conventional cyclic modulation |
| **Fog** | very high | bipolar drifting cloudlets for spatial, spectral and effects modulation | fixed voice cap makes the process only an approximation to unrestricted pulse superposition |

As a bank, Ambient should be strongest when the user wants modulation that remains alive without sounding like a sequencer, a short loop or raw random noise.

## 12. ATmega328P feasibility

All four modes fit the existing fixed-memory architecture in principle:

- Current: three 32-bit phases and three soft-wave evaluations per sample;
- Anchor: one signed state, one random innovation and fixed-point mean-reversion arithmetic per sample;
- Breath: one phase plus three random draws only at cycle rollover;
- Fog: fixed array of up to four cloud voices, one event decision per sample and up to four polynomial-envelope evaluations.

Likely worst cases are Current and Fog because their full work executes every 2.5 kHz sample. Neither should be declared safe solely from source inspection; both require the same AVR timing-probe qualification used by the other banks.

No heap allocation or floating-point arithmetic is acceptable in the AVR hot path.

## 13. Verification contract

The Ambient bank should add dedicated tests for:

- exact DIP-slot mapping under the compile-time bank selector;
- common Ambient /16 time-scale mapping and non-zero minimum increment;
- Current constant-sum weights, phase-ratio approximations, soft-wave continuity, deterministic recurrence and DAC bounds;
- Anchor mean-reversion direction, stationary-spread invariance across representative Speed values, deterministic seeded innovations and bounded output;
- Breath exact baseline/peak endpoints, zero-slope smoothstep boundaries, Texture-zero periodic repeat, variation bounds and one parameter-set update per cycle;
- Fog kernel endpoints/peak, bipolar amplitude symmetry, density compensation versus duration, voice-cap behavior, deterministic fixed-seed output and DAC bounds;
- native sanitizer/coverage qualification;
- AVR flash/SRAM resource limits;
- timing-probe qualification against the 400 µs processing deadline.

Statistical tests must validate properties of the stochastic contract with tolerances and fixed seeds; they must not require an arbitrary finite random stream to match an ideal asymptotic distribution exactly.

## 14. Design status

This document defines the proposed Ambient mathematical and musical contract. It does **not** claim that the current firmware implements the bank. Implementation should follow these contracts or explicitly revise the analysis before code is changed.

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../../assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
