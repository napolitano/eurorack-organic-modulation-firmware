# Free Modular Drift — Percussion Algorithm Bank

[← Main README](README.md) · [Classic bank](README-BANK-CLASSIC.md) · [Organic bank](README-BANK-ORGANIC.md) · [Generative bank](README-BANK-GENERATIVE.md) · [Ambient bank](README-BANK-AMBIENT.md) · [Electronica bank](README-BANK-ELECTRONICA.md) · [Engineering design](docs/analysis/algorithm-banks/percussion-bank-design.md)

The **Percussion bank** is a compile-time bank for trigger/CV patterns. It contains **Euclid, Repeat, Probability and Humanize**. With no valid clock on **Speed CV**, the bank runs internally from the **Speed knob** at the Electronica-derived **30–240 BPM** tempo range. In this bank only, Speed CV is repurposed as an optional external clock input: one accepted rising edge represents one quarter note. Two valid edges are required for lock; if the clock disappears for more than 2.5 measured periods, timing automatically falls back to the current Speed-knob tempo without resetting the running bar/phrase counters.

> [!WARNING]
> **Original-hardware clock input limit: 0–5 V only. Do not patch 10 V triggers/clocks into Speed CV.** The current Drift input stage was designed as a 0–5 V CV input and is not protected/specified as a general Eurorack 10 V trigger input. A higher trigger can overstress the input circuitry. This firmware feature does not make the existing hardware electrically 10 V tolerant.

On external-clock acquisition, the second accepted edge becomes a deterministic new phrase origin (bar 1, beat 1). There is still no separate reset input. Percussion otherwise uses a 4/4-oriented internal grid. Euclid, Repeat and Probability share a phrase model with 16 steps per bar and Texture-selected **4/8/12/16-bar phrases**. The last bar can act as a fill. Humanize deliberately keeps its event count fixed and does not add fills.

## DIP selection

| Rear DIP 1 | Rear DIP 2 | Algorithm |
|---|---|---|
| **OFF** | **OFF** | **Euclid** |
| **ON** | **OFF** | **Repeat** |
| **OFF** | **ON** | **Probability** |
| **ON** | **ON** | **Humanize** |

## Controls at a glance

| Algorithm | Speed | Texture | Character |
|---|---|---|---|
| **Euclid** | Speed knob internal tempo; Speed CV quarter-note clock when present | hit density + phrase/fill activity | evenly distributed 16-step rhythm with deterministic phrase-end fills |
| **Repeat** | Speed knob internal tempo; Speed CV quarter-note clock when present | repeat probability + ratchet depth + phrase activity | quarter-note anchors with intermittent flams/ratchets and tail rolls |
| **Probability** | Speed knob internal tempo; Speed CV quarter-note clock when present | secondary/ghost probability + phrase activity | stable metric skeleton with weighted stochastic additions |
| **Humanize** | Speed knob internal tempo; Speed CV quarter-note clock when present | bounded microtiming + pulse amplitude variation | fixed eighth-note grid with no cumulative timing drift |

## Clock source and fallback

Speed CV is not added to the Speed knob in Percussion. The firmware applies hysteresis at approximately 1 V (LOW) and 2 V (HIGH), measures the interval between rising edges, and requires two valid edges before switching to external timing. Each edge represents a quarter note; sixteenth/eighth subdivisions and Repeat ratchets are derived from the measured period. Steady DC on Speed CV does not lock the clock.

When external timing is lost for more than 2.5 periods, the bank immediately returns to the internal Speed-knob source. The current bar and phrase counters continue; they are not reset by clock loss. A later reacquisition starts a new deterministic phrase origin because the hardware has no reset input with which to recover the external bar position.

## Shared phrase engine

Texture maps phrase length to 16, 12, 8 or 4 bars and fill strength to five levels. Phrase length is latched only at the start of a phrase, so a live Texture/CV move cannot move an already scheduled phrase ending. Euclid, Repeat and Probability interpret the shared fill flag differently.

## Euclid

Euclid selects one of the verified canonical `E(k,16)` masks for `k = 2..13`. The masks are stored as constants; Bjorklund generation is not performed in the real-time path. On the final phrase bar, fill strength only adds hits in steps 12–15. Existing Euclidean hits are never removed.

Engineering analysis: [Euclid algorithm](docs/analysis/algorithms/euclid-analysis.md).

## Repeat

Repeat guarantees a quarter-note anchor. Texture raises repeat probability up to 75% and selects clusters of two to four pulses distributed through the first half of the quarter note. Phrase fills force progressively stronger tail ratchets; the highest fill level forces four-pulse clusters on both final half-bar beats.

Engineering analysis: [Repeat algorithm](docs/analysis/algorithms/repeat-analysis.md).

## Probability

Probability keeps primary steps 0/4/8/12 deterministic. Secondary eighth-note positions rise linearly with Texture; odd ghost positions rise quadratically to a maximum probability of one half. The final phrase bar boosts only optional probabilities, preserving the metric skeleton.

Engineering analysis: [Probability algorithm](docs/analysis/algorithms/probability-analysis.md).

## Humanize

Humanize keeps exactly eight nominal eighth-note events per bar. Texture adds bounded timing deviation up to ±12 ms and pulse-level variation around the nominal level. Every event is displaced relative to its independent nominal grid position; jitter is never accumulated into the following interval, so random tempo drift cannot build up.

Engineering analysis: [Humanize algorithm](docs/analysis/algorithms/humanize-analysis.md).

## Build

```bash
pio run -e nanoatmega328new_percussion
pio run -e nanoatmega328_percussion
pio test -e native_percussion
pio test -e native_percussion_sanitized
pio test -e native_percussion_coverage
```

Timing qualification uses `nanoatmega328new_percussion_timing`.

The complete mathematical, musical and duplication contracts are documented in [Percussion bank architecture and control contract](docs/analysis/algorithm-banks/percussion-bank-design.md).

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="docs/assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
