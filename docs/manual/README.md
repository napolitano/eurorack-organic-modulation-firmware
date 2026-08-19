<!-- SPDX-FileCopyrightText: 2026 Axel Napolitano -->
<!-- SPDX-License-Identifier: CC-BY-NC-4.0 -->

# Drift User Manual

`drift-user-manual.odt` is the maintained source for the end-user manual shipped with tagged firmware releases.

The source file intentionally has **no release version in its filename**. Only a tagged release `vX.Y.Z` publishes a generated PDF as:

```text
drift-user-manual.X.Y.Z.pdf
```

For example, tag `v0.1.0` produces `drift-user-manual.0.1.0.pdf`.

## Typography

The manual is designed for the classic **Ubuntu** and **Ubuntu Light** typefaces. Font binaries are not stored in this repository. The Ubuntu Font Family is distributed under the Ubuntu Font Licence 1.0; the font licence permits embedding in documents without changing the licence of the document itself. See <https://ubuntu.com/legal/font-licence>.

The tag-driven release workflow installs the `fonts-ubuntu` package in the ephemeral CI environment before exporting the ODT source. The strict artifact check requires Ubuntu-family fonts to be present in the generated PDF and embedded in the file. A release must not silently substitute another typeface for Ubuntu.

For a Debian/Ubuntu workstation, the publication dependencies are:

```bash
sudo apt-get update
sudo apt-get install libreoffice fonts-ubuntu poppler-utils
```

Then build and verify a local versioned PDF with:

```bash
python scripts/build_user_manual.py --version 0.1.0 --output-dir dist
python scripts/check_user_manual.py dist/drift-user-manual.0.1.0.pdf
```

A local visual smoke build may use `--allow-font-substitution` with the checker when Ubuntu is deliberately unavailable. Such a PDF is **not release-grade**.

The PDF validator intentionally does **not** treat the ODT `meta:page-count` field as a release contract. That value is a cached Writer statistic and can be stale after legitimate source edits. Release validation instead checks page geometry and requires embedded Ubuntu/Ubuntu Light fonts; the ODT content contract is validated separately.

## Reusable vector assets

The explanatory graphics from the manual are maintained as standalone SVGs in [`assets/`](assets/). The current inventory contains **19 true-vector assets**: the Classic-bank/front-panel material plus five deterministic Organic-bank figures. They are vector artwork rather than SVG wrappers around raster images, keeping diagrams sharp on GitHub, in generated documentation and in downstream layouts.

<p align="center">
  <img src="assets/drift-front-panel.svg" alt="Drift front panel controls" width="220">
</p>

The asset inventory and source mapping are documented in [`assets/README.md`](assets/README.md). Organic figures are regenerated deterministically with `scripts/generate_organic_manual_assets.py`; generated geometry should not be edited by hand. These SVGs are covered by the same CC BY-NC 4.0 manual licence.

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

The manual covers **both compile-time banks and all eight modes**. Each mode ends with a **Mathematical foundations** subsection that states the core equations and explains the musical consequence. The detailed engineering derivations remain in [`../analysis/algorithms/`](../analysis/algorithms/) and [`../analysis/algorithm-banks/`](../analysis/algorithm-banks/).

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
