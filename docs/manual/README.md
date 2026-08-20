<!-- SPDX-FileCopyrightText: 2026 Axel Napolitano -->
<!-- SPDX-License-Identifier: CC-BY-NC-4.0 -->

# Drift User Manual

`drift-user-manual.odt` is the maintained editing source for the end-user manual. During release preparation, the final source is frozen byte-for-byte under [`releases/`](releases/) before the release commit is tagged.

For release `X.Y.Z` the committed snapshot is:

```text
docs/manual/releases/X.Y.Z/drift-user-manual.X.Y.Z.odt
```

The tag-driven workflow publishes that ODT unchanged and generates the matching PDF:

```text
drift-user-manual.X.Y.Z.odt
drift-user-manual.X.Y.Z.pdf
```

Normal development continues to edit only `drift-user-manual.odt`; existing frozen snapshots are release records and must not be rewritten.

## Typography

The manual is designed for the classic **Ubuntu** and **Ubuntu Light** typefaces. Font binaries are not stored in this repository. The Ubuntu Font Family is distributed under the Ubuntu Font Licence 1.0; the font licence permits embedding in documents without changing the licence of the document itself. See <https://ubuntu.com/legal/font-licence>.

The tag-driven release workflow installs the `fonts-ubuntu-classic` package in the ephemeral Ubuntu 24.04 CI environment before exporting the ODT source. The strict artifact check requires Ubuntu-family fonts to be present in the generated PDF and embedded in the file. A release must not silently substitute another typeface for Ubuntu.

For a Debian/Ubuntu workstation, the publication dependencies are:

```bash
sudo apt-get update
sudo apt-get install libreoffice-writer fonts-ubuntu-classic poppler-utils
```

During explicit release preparation, freeze and verify the final source first:

```bash
python scripts/freeze_user_manual.py --version X.Y.Z
python scripts/freeze_user_manual.py --version X.Y.Z --check
```

Then build and verify a local versioned PDF from the frozen source:

```bash
python scripts/build_user_manual.py \
  --version X.Y.Z \
  --source docs/manual/releases/X.Y.Z/drift-user-manual.X.Y.Z.odt \
  --output-dir dist
python scripts/check_user_manual.py \
  dist/drift-user-manual.X.Y.Z.pdf \
  --source docs/manual/releases/X.Y.Z/drift-user-manual.X.Y.Z.odt
```

A local visual smoke build may use `--allow-font-substitution` with the checker when Ubuntu is deliberately unavailable. Such a PDF is **not release-grade**.

On Ubuntu 24.04, the classic Ubuntu 0.83 font metadata has a long-standing interoperability quirk: `Ubuntu-M.ttf` is also advertised through fontconfig as part of the `Ubuntu Light` family. LibreOffice can therefore report/embed source text authored as **Ubuntu Light** under the PDF font name **Ubuntu-Medium**. Release validation accepts that specific alias only when the ODT source explicitly declares `Ubuntu Light`; generic font substitution remains a hard failure.

The PDF validator intentionally does **not** treat the ODT `meta:page-count` field as a release contract. That value is a cached Writer statistic and can be stale after legitimate source edits. Release validation instead checks page geometry and requires embedded Ubuntu/Ubuntu Light fonts; the ODT content contract is validated separately.

## Reusable vector assets

The explanatory graphics from the manual are maintained as standalone SVGs in [`assets/`](assets/). The current inventory contains **59 true-vector assets**: the shared/Classic material, bank-specific Organic, Generative, Ambient, Electronica and Percussion DIP diagrams, four compact DIP-slot symbols for the bank overview, and deterministic explanatory figures for all added banks. They are vector artwork rather than SVG wrappers around raster images, keeping diagrams sharp on GitHub, in generated documentation and in downstream layouts.

<p align="center">
  <img src="assets/drift-front-panel.svg" alt="Drift front panel controls" width="220">
</p>

The asset inventory and source mapping are documented in [`assets/README.md`](assets/README.md). Organic DIP diagrams and Organic algorithm figures are regenerated deterministically with `scripts/generate_organic_manual_assets.py`. Generative DIP diagrams, compact bank-slot symbols and Generative algorithm figures are regenerated with `scripts/generate_generative_manual_assets.py`. Ambient DIP diagrams and Ambient algorithm figures are regenerated with `scripts/generate_ambient_manual_assets.py`. Electronica DIP diagrams and Electronica algorithm figures are regenerated with `scripts/generate_electronica_manual_assets.py`. Percussion DIP diagrams, rhythm figures and the 6-bank cover update are regenerated with `scripts/generate_percussion_manual_assets.py`; generated geometry should not be edited by hand. These SVGs are covered by the same CC BY-NC 4.0 manual licence.

## Behavioural source of truth

The manual describes the behavior of the firmware in this repository, including intentional corrections to verified upstream numerical/control issues. It therefore does not reproduce historical upstream prose where that prose conflicts with the implemented and tested behavior. In particular:

- Brownian Speed is a direct stochastic activity control, not a 1 V/oct frequency control.
- Bezier timing variation uses a symmetric triangular random offset in logarithmic speed space; it is not a Gaussian interval distribution.
- Bezier curve shape morphs continuously from inverse easing through linear to smooth easing and is controlled by the Texture knob only.
- LFO mode is periodic and morphs between falling saw, triangle and rising saw.
- Fractal combines three continuous gradient-noise scales at 1x/4x/16x; Texture redistributes a constant total weight toward finer detail.
- Vector is deterministic two-axis toroidal phase flow; Texture controls bounded cross-coupling rather than randomness.
- Rain maps Texture to event **Density**, Speed to envelope-decay speed, and the physical post-DAC Attenuation control naturally acts as final output **Intensity**.
- Attractor uses a fixed-point Hénon map with Texture-controlled parameter `a`; because the digital state space is finite, its trajectories are ultimately periodic rather than mathematically infinite chaotic orbits.
- Turing uses a 16-bit feedback shift register whose Texture-controlled mutation probability is limited to 0..1/2; full inversion would be deterministic, not maximally random.
- Markov uses an eight-state fixed voltage vocabulary and mixes a structured transition grammar with uniform exploration.
- Motif preserves an explicit eight-step phrase and applies at most one structural edit at each completed phrase boundary.
- Urn uses bounded, leaky reinforcement over eight fixed output states; it is Pólya-inspired rather than an exact classical Pólya urn.
- Current combines three deterministic slow currents at fixed non-harmonic rate approximations; Texture redistributes a constant total weight and does not act as another level control.
- Anchor is an OU-inspired bounded AR(1) mean-reverting process driven by the firmware's symmetric triangular innovation; it is deliberately not described as an exact Gaussian Ornstein-Uhlenbeck process.
- Breath always follows baseline → one peak → baseline; Texture introduces cycle-to-cycle variation in duration, amplitude and peak position only at rollover.
- Fog sums at most four finite-support bipolar cloudlets; Texture targets average occupancy while the static voice limit keeps CPU and SRAM bounded.
- Pump is a free-running duck/recovery contour at an internal 30–240 BPM quarter-note reference; it is not an audio sidechain compressor and has no external trigger lock.
- Acid is a deterministic project-defined 16-step CV grammar with accent and slide masks; it is not a TB-303 emulator or copied factory pattern.
- Shuffle preserves each two-sixteenth pair duration while moving the second onset from 1/2 to 3/4 of the pair, giving straight through exact 3:1 timing without random humanization.
- Polymeter keeps a four-step anchor against a Texture-selected 3/5/7/9-step cycle and realigns after exact 12/20/28/36-step least-common-multiple periods.
- Percussion uses the Speed knob for its internal 30–240 BPM quarter-note clock and repurposes Speed CV as an optional **0–5 V only** external quarter-note clock; two valid edges acquire sync and loss after 2.5 measured periods falls back automatically to the Speed-knob clock.
- Euclid selects verified `E(k,16)` masks for `k=2..13`; phrase-end fills only add hits in steps 12–15.
- Repeat keeps quarter-note anchors and adds Texture-controlled doubles/triples/four-pulse ratchets, with stronger forced ratchets in phrase-end fills.
- Probability guarantees primary quarter notes, gives secondary hits linear Texture probability and ghost hits quadratic half-scale probability, and only boosts optional hits during fills.
- Humanize keeps exactly eight nominal eighth-note events per bar and adds bounded timing/level variation without accumulating jitter into tempo drift.

The manual covers **all six compile-time banks and all twenty-four modes**. A dedicated **Algorithm banks** section explains the distinction between flashing a bank and selecting an algorithm. Each bank then has its own four DIP-switch diagrams before its mode descriptions. The Algorithm banks introduction provides a compact six-bank slot table and a separate origin/musical-value table, while the bank sections themselves remain bank-specific rather than using a cross-bank DIP comparison. Each mode ends with a **Mathematical foundations** subsection that states the core equations and explains the musical consequence. The detailed engineering derivations remain in [`../analysis/algorithms/`](../analysis/algorithms/) and [`../analysis/algorithm-banks/`](../analysis/algorithm-banks/).

## Source contract

The publication source is expected to retain these document-level properties:

- identifier: `drift-user-manual`
- language: `en-US`
- licence notice: CC BY-NC 4.0
- document styles referencing `Ubuntu` and `Ubuntu Light`

Run:

```bash
python scripts/check_user_manual_source.py
```

before publication.

## Licence scope

The manual and the files in this directory are licensed separately from the firmware. See [LICENSE](LICENSE).

The firmware source in the repository remains GPL-3.0-or-later. The original Free Modular Drift design and upstream firmware are Quinn Freedman's work and remain subject to their respective upstream licensing terms.

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
