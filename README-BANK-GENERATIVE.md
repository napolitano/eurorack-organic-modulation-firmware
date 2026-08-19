# Free Modular Drift — Generative Algorithm Bank

[← Main README](README.md) · [Classic bank](README-BANK-CLASSIC.md) · [Organic bank](README-BANK-ORGANIC.md) · [Ambient bank](README-BANK-AMBIENT.md) · [Electronica bank](README-BANK-ELECTRONICA.md) · [Engineering design](docs/analysis/algorithm-banks/generative-bank-design.md)

The **Generative bank** is a compile-time alternative bank for Drift focused on stepped modulation with explicit memory. It contains **Turing, Markov, Motif and Urn**. Unlike Classic Perlin/Brownian/Bézier and Organic Fractal/Vector/Attractor, these modes do not primarily model continuous trajectories. They produce internally timed 12-bit states whose future behavior depends on retained sequence, state, phrase or reinforcement memory.

The bank uses the original Drift hardware unchanged. **Speed** controls the internal step/draw rate, **Texture** controls the algorithm-specific evolution mechanism, and **Attenuation** remains a purely analogue post-DAC output-depth control.

## DIP selection

The bank is selected by flashing a Generative firmware image. Rear DIP switches then choose one of its four algorithms and are sampled only at startup.

| Rear DIP 1 | Rear DIP 2 | Algorithm |
|---|---|---|
| **OFF** | **OFF** | **Turing** |
| **ON** | **OFF** | **Markov** |
| **OFF** | **ON** | **Motif** |
| **ON** | **ON** | **Urn** |

## Controls at a glance

| Algorithm | Speed | Texture | Character |
|---|---|---|---|
| **Turing** | shift rate | mutation probability, 0 → 1/2 | repeating 16-bit loop that gradually rewrites itself |
| **Markov** | transition rate | exploration from fixed grammar → uniform next-state choice | recurring eight-state vocabulary without a fixed phrase |
| **Motif** | phrase step rate | probability of one structural edit per completed phrase | recognizable eight-step phrase with controlled variation |
| **Urn** | draw rate | reinforcement strength | temporary statistical preferences that emerge and decay |

Texture CV is added to the Texture knob with the same saturated 10-bit control law used elsewhere in Drift. Speed CV contributes to the established exponential step-rate mapping. The first Generative implementation is internally clocked; the existing Speed CV input is not documented as a digital clock input.

## Turing

Turing stores a 16-bit register. At each internal step the outgoing bit is recycled into the opposite end, optionally inverted by a Texture-controlled Bernoulli mutation.

If $p$ is the mutation probability,

$$
b_{new}=b_{feedback}\oplus B(p),
\qquad 0\le p\le\frac{1}{2}.
$$

At Texture zero the state rotates exactly, so a non-degenerate register returns after 16 shifts. At maximum Texture, $p=1/2$; the entering bit is then maximally independent of the recycled feedback bit. Mapping Texture to $p=1$ would be wrong because that would produce deterministic inversion rather than maximum randomness.

The upper twelve register bits are sent directly to the DAC:

$$
y=R\gg4.
$$

Musically, Turing is the bank's strongest **persistent-loop** mode: recognizable stepped CV repeats can survive for long periods and then gradually mutate.

Engineering analysis: [Turing algorithm](docs/analysis/algorithms/turing-analysis.md).

## Markov

Markov uses eight symbolic states and a fixed seed-defined voltage vocabulary. The vocabulary is generated once from eight 512-code DAC bands and shuffled so neighboring symbolic states are not automatically neighboring voltages.

At minimum Texture, state $i$ follows the fixed grammar:

- $1/2$ stay at $i$;
- $1/4$ move to $(i+1)\bmod8$;
- $1/8$ move to $(i-1)\bmod8$;
- $1/8$ move to $(i+4)\bmod8$.

Texture mixes that grammar with a uniform next-state choice:

$$
P_{\tau}=(1-\tau)P_s+\tau U.
$$

At full Texture the next state is uniformly selected from all eight states. The voltage vocabulary itself remains fixed, preserving a recognizable set of modulation levels even at high exploration.

Musically, Markov is the bank's **recurring-vocabulary** mode: familiar places, changing route.

Engineering analysis: [Markov algorithm](docs/analysis/algorithms/markov-analysis.md).

## Motif

Motif stores eight explicit 12-bit values and plays them as a phrase. Texture controls whether exactly one structural edit is applied at the boundary between complete phrase cycles:

$$
P(edit)=\tau.
$$

When an edit occurs, one of four equally likely transformations is selected:

1. rotate the phrase one step left or right;
2. swap one adjacent circular pair;
3. reverse one circular three-step span;
4. replace one value with a new 12-bit value.

Three of the four edit classes preserve the complete eight-value vocabulary. Even replacement changes only one value. At maximum Texture there is still at most one edit per complete phrase; the algorithm never redraws the whole phrase merely because Texture is high.

The implementation applies the edit **before emitting step 0 of the new cycle**, so every audible eight-step cycle is internally consistent.

Musically, Motif is the bank's strongest **identity-through-variation** mode.

Engineering analysis: [Motif algorithm](docs/analysis/algorithms/motif-analysis.md).

## Urn

Urn uses eight fixed, evenly spaced DAC levels with mutable selection weights. Every state starts at baseline weight $b=32$. Before each draw, all weights relax toward baseline by a factor of $31/32$:

$$
w_i'=b+\left\lfloor\frac{31(w_i-b)}{32}\right\rfloor.
$$

The selected state then receives Texture-controlled reinforcement from 0 to 64, saturating at $w_{max}=1023$. Future draw probability is proportional to current weight:

$$
P(X=i)=\frac{w_i}{\sum_j w_j}.
$$

The fixed output vocabulary is

$$
v_i=585i,
\qquad i=0\ldots7,
$$

which spans exactly 0..4095.

This is deliberately a **bounded, leaky, Pólya-inspired reinforcement process**, not an exact classical Pólya urn. Relaxation prevents an early random advantage from becoming permanently dominant.

Musically, Urn is the bank's **emergent-preference** mode: some voltage regions become temporary favorites and later lose that advantage.

Engineering analysis: [Urn algorithm](docs/analysis/algorithms/urn-analysis.md).

## Build

Current Arduino Nano bootloader:

```bash
pio run -e nanoatmega328new_generative
```

Legacy Nano bootloader:

```bash
pio run -e nanoatmega328_generative
```

Native verification:

```bash
pio test -e native_generative
pio test -e native_generative_sanitized
pio test -e native_generative_coverage
```

Timing qualification uses `nanoatmega328new_generative_timing`.

The complete bank-level design and duplication audit are documented in [Generative bank architecture and control contract](docs/analysis/algorithm-banks/generative-bank-design.md).

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="docs/assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
