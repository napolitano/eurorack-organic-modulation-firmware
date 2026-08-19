<!-- SPDX-FileCopyrightText: 2026 Axel Napolitano -->
<!-- SPDX-License-Identifier: CC-BY-NC-4.0 -->

# Manual vector assets

This directory contains reusable vector redraws of explanatory artwork embedded in `../drift-user-manual.odt`.

The SVGs are intentionally maintained as real vector geometry. They do not contain embedded PNG/JPEG images. Text remains SVG text using the manual's `Ubuntu` / `Ubuntu Light` typography with generic sans-serif fallbacks for environments where Ubuntu is unavailable.

## Asset inventory

| SVG | Manual source | Purpose |
|---|---|---|
| `drift-front-cover.svg` | Vector replacement for the original composed cover raster | User-manual cover covering both firmware banks |
| `drift-back-cover.svg` | Vector replacement for the original composed back-cover raster | Credits, licence and Classic/Organic bank summary |
| `drift-front-panel.svg` | `Pictures/10000000000004D000000764D6EF55BF.png` | Numbered front-panel control reference |
| `config-perlin.svg` | `Pictures/10000000000001C2000001203D7620D3.png` | Perlin configuration-switch position |
| `config-brownian.svg` | `Pictures/10000000000001C200000120799504B3.png` | Brownian configuration-switch position |
| `config-bezier.svg` | `Pictures/10000000000001C200000120E424DA35.png` | Bézier configuration-switch position |
| `config-lfo.svg` | `Pictures/10000000000001C20000012006867D0F.png` | LFO configuration-switch position |
| `config-fractal.svg` | Generated from the Organic bank DIP mapping | Fractal configuration-switch position |
| `config-vector.svg` | Generated from the Organic bank DIP mapping | Vector configuration-switch position |
| `config-rain.svg` | Generated from the Organic bank DIP mapping | Rain configuration-switch position |
| `config-attractor.svg` | Generated from the Organic bank DIP mapping | Attractor configuration-switch position |
| `perlin-low-texture.svg` | `Pictures/1000000000000420000001686E61D248.png` | Low-texture Perlin example |
| `perlin-medium-texture.svg` | `Pictures/1000000000000420000001683DE711E2.png` | Medium-texture Perlin example |
| `bezier-smooth.svg` | `Pictures/1000000000000420000001680A4A5BF2.png` | Smooth Bézier easing example |
| `bezier-inverse.svg` | `Pictures/1000000000000420000001680461B15D.png` | Inverse Bézier easing example |
| `brownian-texture-comparison.svg` | `Pictures/100000000000042000000193C5D884ED.png` | Brownian full-texture vs. smoothed comparison |
| `lfo-skew-texture.svg` | `Pictures/10000000000004200000019356D9371B.png` | Skewed-triangle LFO texture comparison |
| `drift-output-example.svg` | `Pictures/10000000000003A800000186D21A4305.png` | Generic Drift output trace |
| `organic-bank-overview.svg` | Generated from the compile-time bank/DIP mapping | Classic vs Organic bank slot overview |
| `fractal-texture.svg` | Generated from the documented 1x/4x/16x gradient-noise mix | Fractal Texture comparison |
| `vector-flow.svg` | Generated from the documented coupled toroidal phase model | Vector internal trajectory and scalar projection |
| `rain-density.svg` | Generated from the production Bernoulli/decay control laws with the paired LFSR | Rain Density comparison |
| `attractor-henon.svg` | Generated from the production Q2.14 Hénon iteration | Attractor fixed-point orbit and output projection |

The Organic-bank configuration diagrams and the five Organic-bank explanatory figures are generated deterministically by `scripts/generate_organic_manual_assets.py`. They are explanatory visualisations of the implemented control laws rather than oscilloscope captures.

The ODT now embeds the reusable vector front/back covers from this directory. Creative Commons marks, callout-number circles and small decorative symbols remain embedded document artwork because they do not add reusable technical information outside the composed manual pages.

## Editing rules

- Keep SVGs self-contained and raster-free.
- Use an explicit `viewBox` and no fixed physical page size.
- Keep explanatory text as text, not outlined glyph paths.
- Prefer the manual palette and Ubuntu-family typography.
- Do not add generated release-version strings to asset filenames.
- If the corresponding figure in the ODT changes materially, update the SVG and this source mapping together.
- Regenerate the Organic figures with `python scripts/generate_organic_manual_assets.py`; do not hand-edit generated geometry.

Run `python scripts/check_manual_assets.py` to verify the vector-asset contract.

## Licence

These assets are part of the end-user manual and are licensed under CC BY-NC 4.0. See [`../LICENSE`](../LICENSE).

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../../assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
