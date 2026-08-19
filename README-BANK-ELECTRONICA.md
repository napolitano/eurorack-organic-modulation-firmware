# Free Modular Drift — Electronica Algorithm Bank

[← Main README](README.md) · [Classic bank](README-BANK-CLASSIC.md) · [Organic bank](README-BANK-ORGANIC.md) · [Generative bank](README-BANK-GENERATIVE.md) · [Ambient bank](README-BANK-AMBIENT.md) · [Engineering design](docs/analysis/algorithm-banks/electronica-bank-design.md)

The **Electronica bank** is a compile-time Drift bank for rhythmically legible CV in house, acid, techno and adjacent electronic styles. It contains **Pump, Acid, Shuffle and Polymeter**. The bank remains free-running on the original hardware: there is no external clock input, so documented BPM values describe the internal scheduler rather than transport lock.

Electronica uses its own nominal Speed mapping:

$$
B(u)=30\cdot2^{3u}\ \text{BPM},
$$

covering **30–240 BPM** from the saturated Speed knob + CV control. Attenuation remains an analogue post-DAC depth control.

## DIP selection

| Rear DIP 1 | Rear DIP 2 | Algorithm |
|---|---|---|
| **OFF** | **OFF** | **Pump** |
| **ON** | **OFF** | **Acid** |
| **OFF** | **ON** | **Shuffle** |
| **ON** | **ON** | **Polymeter** |

The bank is selected by flashing an Electronica firmware image. Rear DIP switches select one algorithm inside that bank at startup.

## Controls at a glance

| Algorithm | Speed | Texture | Character |
|---|---|---|---|
| **Pump** | nominal quarter-note tempo | recovery endpoint 1/4..15/16 beat | sidechain-style duck/recovery contour |
| **Acid** | nominal 16th-note grid | accent + slide intensity | fixed 16-step electronic riff contour |
| **Shuffle** | nominal pair tempo | straight to 3:1 long/short timing | deterministic swing without random humanization |
| **Polymeter** | nominal 16th-note grid | secondary meter 3/5/7/9 | four-step anchor against a longer odd cycle |

## Pump

Pump resets to zero at every internal quarter-note boundary and recovers with cubic smoothstep. Texture maps the recovery endpoint exactly from `1/4` to `15/16` of the beat. The implementation caches a Q28 reciprocal when Texture changes so the per-sample path does not perform a general phase/end-point division.

Pump generates only the resulting modulation gesture. It is not a sidechain compressor and does not detect an external kick.

Engineering analysis: [Pump algorithm](docs/analysis/algorithms/pump-analysis.md).

## Acid

Acid uses the exact project-defined permutation

$$
q_n=(5n+3)\bmod16,
$$

with base target `1024 + 128*q_n`. Accents occur where `n mod 4 = 0` or `n mod 7 = 0`; slides occur where `(5*n mod 16) < 4`. Texture jointly increases accent contribution and slide interpolation. No random state is used.

This is a CV modulation grammar inspired by acid sequencing vocabulary, not a TB-303 emulator or note sequencer.

Engineering analysis: [Acid algorithm](docs/analysis/algorithms/acid-analysis.md).

## Shuffle

Shuffle works on pairs of sixteenth-note subdivisions. Texture moves the second onset from exactly `1/2` of the pair to `3/4`, producing an exact maximum long/short ratio of `3:1` while pair duration remains constant. The ratio is latched at each pair boundary so moving Texture cannot create a double or missing onset inside the active pair.

Each onset launches the same short smooth decay. The implemented decay occupies one eighth of the complete pair, safely shorter than the minimum second interval.

Engineering analysis: [Shuffle algorithm](docs/analysis/algorithms/shuffle-analysis.md).

## Polymeter

Polymeter keeps a fixed four-step primary meter against one Texture-selected secondary meter of `3`, `5`, `7` or `9` steps. The exact composite recurrence lengths are therefore `12`, `20`, `28` and `36` sixteenth steps.

Every sixteenth launches a short decay whose peak is one of four exact levels: base `1024`, primary `2559`, secondary `2560`, or coincidence `4095`. A meter change is applied only at the next primary boundary; when the meter changes, the new secondary cycle starts on that boundary. The first implementation deliberately uses no selector hysteresis.

Engineering analysis: [Polymeter algorithm](docs/analysis/algorithms/polymeter-analysis.md).

## Build

Current Arduino Nano bootloader:

```bash
pio run -e nanoatmega328new_electronica
```

Legacy Nano bootloader:

```bash
pio run -e nanoatmega328_electronica
```

Native verification:

```bash
pio test -e native_electronica
pio test -e native_electronica_sanitized
pio test -e native_electronica_coverage
```

Timing qualification uses `nanoatmega328new_electronica_timing`.

The complete bank-level design and duplication audit are documented in [Electronica bank architecture and control contract](docs/analysis/algorithm-banks/electronica-bank-design.md).

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="docs/assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
