# Free Modular Drift — Ambient Algorithm Bank

[← Main README](README.md) · [Classic bank](README-BANK-CLASSIC.md) · [Organic bank](README-BANK-ORGANIC.md) · [Generative bank](README-BANK-GENERATIVE.md) · [Engineering design](docs/analysis/algorithm-banks/ambient-bank-design.md)

The **Ambient bank** is a compile-time Drift bank for slow, continuously evolving modulation. It contains **Current, Anchor, Breath and Fog**. The bank is deliberately separated from Classic, Organic and Generative: it focuses on long-form deterministic beating, mean-reverting stochastic motion, recurrent macro-gestures and smooth stochastic clouds rather than short loops, stepped state vocabularies or ordinary noise trajectories.

All four modes use the established Drift **Speed** and **Texture** controls. Their primary time scale is slowed by a factor of sixteen relative to the common Drift phase-rate mapping. **Attenuation** remains an analogue post-DAC control and is not visible to firmware.

## DIP selection

The bank is selected by flashing an Ambient firmware image. The rear DIP switches then select one of four algorithms and are sampled only at startup.

| Rear DIP 1 | Rear DIP 2 | Algorithm |
|---|---|---|
| **OFF** | **OFF** | **Current** |
| **ON** | **OFF** | **Anchor** |
| **OFF** | **ON** | **Breath** |
| **ON** | **ON** | **Fog** |

## Controls at a glance

| Algorithm | Speed | Texture | Character |
|---|---|---|---|
| **Current** | shared macro rate | redistribution across three non-harmonic currents | deterministic long-form beating without a short obvious loop |
| **Anchor** | mean-reversion / correlation rate | target stationary spread | stochastic wandering with an explicit statistical home |
| **Breath** | nominal swell rate | cycle-to-cycle duration, height and skew variation | recognizable recurring swells without mechanical repetition |
| **Fog** | cloudlet duration | target mean cloud occupancy | smooth bipolar stochastic cloudlets with controlled overlap |

## Current

Current advances three independent phases at fixed rate approximations of

$$
1:\sqrt{2}:\varphi.
$$

The AVR implementation uses the rational ratios `362/256` and `414/256`, giving small bounded errors while avoiding wide division in the sample path. Each phase is projected through a cubic-softened triangle. Texture redistributes a constant total weight of 1024 from `768/192/64` to `512/320/192`.

Current is fully deterministic for fixed controls. Its musical strength is long coherent evolution with repeatable recall.

Engineering analysis: [Current algorithm](docs/analysis/algorithms/current-analysis.md).

## Anchor

Anchor is implemented as an **OU-inspired bounded AR(1) process**, not as an exact Gaussian Ornstein-Uhlenbeck process. The state has an explicit restoring term toward zero. Random innovation uses the repository's existing symmetric triangular inverse-CDF table.

Speed controls the restoring/correlation rate. A generated flash lookup table scales the triangular innovation according to

$$
g \approx \sqrt{6\left(1-a^2\right)},
$$

where the triangular innovation has variance $1/6$. This keeps stationary excursion broadly stable when Speed changes. Texture maps target spread from zero to 0.30 in signed Q1.15.

Musically, Anchor explores a region without behaving like Brownian motion that merely encounters implementation boundaries.

Engineering analysis: [Anchor algorithm](docs/analysis/algorithms/anchor-analysis.md).

## Breath

Breath always follows one invariant gesture:

$$
\text{baseline}\rightarrow\text{one peak}\rightarrow\text{baseline}.
$$

The curve uses cubic smoothstep. At each completed cycle only, three new parameters are latched: duration multiplier `0.75..1.25`, peak amplitude `0.65..1.00`, and peak position `0.25..0.50` of the cycle. Texture zero locks all three to their nominal values `1.0`, `0.825`, and `0.375`.

The hot path uses rollover-cached reciprocals for the attack and release branches; random draws and reciprocal setup occur only at cycle boundaries.

Musically, Breath is intended for macro-dynamics, filter/reverb movement and other destinations where a clear swell is useful but an exact LFO cycle is too mechanical.

Engineering analysis: [Breath algorithm](docs/analysis/algorithms/breath-analysis.md).

## Fog

Fog contains four statically allocated cloudlet voices. Every voice uses the finite-support kernel

$$
g(u)=16u^2(1-u)^2,
$$

with signed amplitude around the DAC midpoint. Texture maps target mean occupancy from **0.125 to 3.0 voices**. Event probability is compensated against Speed-controlled cloud duration, so changing duration does not automatically destroy the intended density.

The four-voice limit is deliberate. When all voices are active, a new event attempt is dropped rather than allocating memory or stealing an existing voice. This keeps CPU and SRAM usage bounded.

Musically, Fog is the bank's soft stochastic-density mode: overlapping movements rather than steps, drops or continuous noise.

Engineering analysis: [Fog algorithm](docs/analysis/algorithms/fog-analysis.md).

## Build

Current Arduino Nano bootloader:

```bash
pio run -e nanoatmega328new_ambient
```

Legacy Nano bootloader:

```bash
pio run -e nanoatmega328_ambient
```

Native verification:

```bash
pio test -e native_ambient
pio test -e native_ambient_sanitized
pio test -e native_ambient_coverage
```

Timing qualification uses `nanoatmega328new_ambient_timing`.

The complete bank-level design and duplication audit are documented in [Ambient bank architecture and control contract](docs/analysis/algorithm-banks/ambient-bank-design.md).

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="docs/assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
