# Fog algorithm engineering analysis

## 1. Purpose and scope

Fog is the implemented Ambient-bank stochastic modulation mode that creates a slowly changing bipolar cloud from multiple overlapping smooth pulse events.

The conceptual reference is filtered pulse/shot-noise modeling, but Drift deliberately uses a finite number of voices, discrete event timing and a compact polynomial pulse shape to remain bounded and practical on ATmega328P.

There is no Quinn Freedman Fog mode to preserve.

## 2. Mathematical foundations

A general filtered pulse process represents a signal as a superposition of events:

$$
x(t)=\sum_i A_i g(t-t_i).
$$

Filtered Poisson processes are widely used as models of intermittent fluctuations built from random pulse arrivals; see A. Theodorsen, O. E. Garcia and M. Rypdal, *Statistical properties of a filtered Poisson process with additive random noise*, Physica Scripta 92 (2017), preprint: <https://arxiv.org/abs/1609.01607>.

Drift's proposed cloudlet kernel is the compact polynomial

$$
g(u)=16u^2(1-u)^2,
\qquad 0\le u\le1,
$$

with $g(u)=0$ outside that range.

It has the useful properties

$$
g(0)=g(1)=0,
$$

$$
g'(0)=g'(1)=0,
$$

and

$$
g(1/2)=1.
$$

Each event receives a signed amplitude $A_i$, so the internal process is bipolar around zero. The DAC projection is centred around midpoint.

## 3. Finite Drift process

The first implementation should use a fixed array of four cloud voices.

Each active voice stores:

- phase/age within the cloudlet;
- signed amplitude;
- active flag.

A new event can start only if at least one voice is free. If all voices are occupied, the event attempt is dropped. No heap allocation, stealing or variable-length voice list is permitted.

This makes the algorithm a **bounded filtered-event process**, not an exact unrestricted Poisson superposition.

## 4. Speed and Texture semantics

Speed controls the common cloud duration $D$ through the Ambient macro-time scale. Faster Speed means shorter cloudlets; slower Speed means longer cloudlets.

Texture controls **expected occupancy**, not merely a raw per-sample trigger probability. Let the target mean active-voice count be $\lambda(\tau)$ for normalized Texture $\tau$.

The ideal event rate is chosen approximately as

$$
r=\frac{\lambda}{D},
$$

so expected overlap remains broadly comparable when cloud duration changes.

In discrete time at sample rate $f_s$,

$$
p\approx\frac{r}{f_s}=\frac{\lambda}{Df_s}
$$

for sparse events.

The implementation may use a Bernoulli threshold approximation, but the compensation relationship between duration and event probability is part of the control contract.

The implementation freezes the target occupancy range at `1..24` eighths of a voice, i.e. $0.125..3.0$ expected voices. This remains below the four-voice hard cap across the panel range while still allowing dense overlap.

## 5. Relationship to Rain and upstream Drift

Rain is the closest existing mode, so the distinction must be explicit.

Rain:

- makes discrete positive event decisions;
- adds positive impulses into one shared aggregate envelope;
- uses an exponential/leaky decay;
- is naturally unipolar;
- Texture directly controls Density.

Fog:

- creates independent concurrent pulse voices;
- each pulse has smooth attack and release;
- amplitudes are positive or negative around centre;
- pulses have finite support and disappear completely at the end;
- Texture targets overlap/occupancy while Speed changes duration.

Thus Fog should sound like overlapping drifting cloudlets rather than rainfall feeding a common energy reservoir.

There is no corresponding upstream Quinn Freedman mode.

## 6. Behavioral analysis

At low Texture, isolated positive and negative swells appear around the midpoint. At medium Texture, several pulses overlap and create slow irregular motion with no hard onset. At high Texture, the output becomes a dense continuously changing cloud.

The compact pulse kernel guarantees that adding or removing a completed voice does not create an amplitude discontinuity because every voice reaches exactly zero at the end of its life.

The finite voice cap changes the statistics at very high occupancy. This is acceptable if documented and deliberately tuned; it is not acceptable to claim exact Poisson superposition under saturation.

Because positive and negative amplitudes are symmetric by construction, the unsaturated long-run process should remain centred around the DAC midpoint.

## 7. Findings and classification

- **Mathematical design choice:** use a quartic compact bump rather than exponential decay.
- **Musical design choice:** cloud amplitudes are bipolar.
- **Real-time constraint:** fixed four-voice cap bounds CPU and SRAM.
- **Control requirement:** density should compensate for Speed-controlled cloud duration.
- **Statistical limitation:** voice saturation biases high-density behavior away from an unrestricted filtered Poisson process.
- **Compatibility status:** no upstream behavior exists to preserve.

## 8. Improvement strategy

The first implementation should remain four voices with a common duration and one fixed pulse shape. Variable-duration events, more voices or multiple kernel families would increase both tuning and CPU complexity.

The first implementation selects a linear `0.125..3.0` target-occupancy law. Event creation compares a composed 32-bit RNG word against a 32-bit cutoff proportional to Ambient phase increment and target occupancy; this preserves sparse event probabilities that would disappear in a 16-bit threshold at very slow settings.

## 9. Computational cost on ATmega328P

Every sample requires:

- one event-probability/threshold decision;
- inspection/update of up to four fixed voices;
- for each active voice, phase advance and quartic-kernel evaluation;
- signed accumulation;
- midpoint bias and saturation.

Two 16-bit RNG draws are composed into the per-sample 32-bit arrival word. A further RNG draw is consumed only when a free voice accepts a new event, selecting a symmetric signed peak amplitude with magnitude `512..1023` DAC codes.

Persistent state is four small voice records plus RNG state.

Fog is likely the Ambient bank's worst-case CPU mode when all four voices are simultaneously active and must be qualified with a dedicated timing image.

## 10. Optimization opportunities

- Express $g(u)=16[u(1-u)]^2$ to reduce polynomial operations.
- Use a fixed-point phase where voice completion is detected by wrap/limit without division.
- The current implementation computes the 32-bit cutoff directly from Ambient increment and the small integer occupancy target; caching remains optional if AVR timing data shows a need.
- Keep the voice array statically allocated and unrolled only if measurement proves worthwhile.
- Use symmetric amplitude generation around zero without division.

## 11. Verification and test strategy

Required deterministic tests:

- kernel is exactly zero at both endpoints and reaches exactly the documented peak at midpoint;
- kernel is non-negative and symmetric: $g(u)=g(1-u)$;
- discrete kernel rises then falls monotonically around the midpoint;
- completed voices contribute exactly zero and become free;
- no more than four voices can be active;
- an event attempt with all voices active cannot corrupt existing voice state;
- positive/negative amplitude mapping is symmetric within one code where integer representation requires rounding;
- output always remains in 0..4095;
- fixed seed/control sequence is deterministic.

Required statistical/control tests:

- event threshold increases monotonically with Texture;
- at fixed Texture, changing cloud duration adjusts event rate in the compensating direction;
- representative Speed values produce broadly similar mean occupancy when not voice-saturated;
- long-run unsaturated mean stays near midpoint;
- high-Texture tests quantify, rather than hide, the fraction of dropped events caused by the voice cap.

## 12. Musical assessment

**Musical value: very high.**

Fog fills a gap that neither Rain nor smooth noise fully covers: discrete stochastic events that are individually inaudible as events because every pulse fades in and out smoothly.

Strong applications include:

- stereo position and spatial diffusion;
- subtle filter or formant wandering;
- reverb/delay diffusion parameters;
- wavefolder or wavetable motion;
- modulation of several correlated-sounding timbral destinations through mults/attenuators.

Its central musical quality is **soft stochastic density**. The output should feel like overlapping pockets of movement rather than steps, drops or a continuous noise trajectory.

## 13. Engineering assessment

Fog is conceptually strong and numerically bounded. The implementation fixes four voices, `0.125..3.0` target occupancy and signed `512..1023`-code cloud amplitudes. The host suite verifies the compact kernel, occupancy law, duration-compensated event cutoff, signed amplitude bounds, voice completion and fixed-seed bounded behavior. Four polynomial voices at 2.5 kHz still require the dedicated AVR timing image before release.

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../../assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
