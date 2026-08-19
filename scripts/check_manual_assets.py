#!/usr/bin/env python3
"""Validate the reusable SVG assets derived from the Drift user manual."""

from __future__ import annotations

from pathlib import Path
import sys
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[1]
ASSET_DIR = ROOT / "docs" / "manual" / "assets"

REQUIRED = {
    "drift-front-cover.svg",
    "drift-back-cover.svg",
    "drift-front-panel.svg",
    "config-perlin.svg",
    "config-brownian.svg",
    "config-bezier.svg",
    "config-lfo.svg",
    "config-fractal.svg",
    "config-vector.svg",
    "config-rain.svg",
    "config-attractor.svg",
    "config-turing.svg",
    "config-markov.svg",
    "config-motif.svg",
    "config-urn.svg",
    "config-current.svg",
    "config-anchor.svg",
    "config-breath.svg",
    "config-fog.svg",
    "dip-slot-00.svg",
    "dip-slot-10.svg",
    "dip-slot-01.svg",
    "dip-slot-11.svg",
    "turing-mutation.svg",
    "markov-vocabulary.svg",
    "motif-evolution.svg",
    "urn-reinforcement.svg",
    "current-long-form.svg",
    "anchor-mean-reversion.svg",
    "breath-cycle-variation.svg",
    "fog-cloudlets.svg",
    "perlin-low-texture.svg",
    "perlin-medium-texture.svg",
    "bezier-smooth.svg",
    "bezier-inverse.svg",
    "brownian-texture-comparison.svg",
    "lfo-skew-texture.svg",
    "drift-output-example.svg",
    "organic-bank-overview.svg",
    "fractal-texture.svg",
    "vector-flow.svg",
    "rain-density.svg",
    "attractor-henon.svg",
}

SVG_NS = "http://www.w3.org/2000/svg"


def fail(message: str) -> None:
    print(f"manual asset check: FAIL: {message}", file=sys.stderr)
    raise SystemExit(1)


def main() -> None:
    if not ASSET_DIR.is_dir():
        fail(f"missing asset directory: {ASSET_DIR}")

    present = {path.name for path in ASSET_DIR.glob("*.svg")}
    missing = sorted(REQUIRED - present)
    if missing:
        fail("missing required SVGs: " + ", ".join(missing))

    for name in sorted(REQUIRED):
        path = ASSET_DIR / name
        try:
            root = ET.parse(path).getroot()
        except ET.ParseError as exc:
            fail(f"{name}: invalid XML: {exc}")

        if root.tag != f"{{{SVG_NS}}}svg":
            fail(f"{name}: root element is not SVG")
        if not root.get("viewBox"):
            fail(f"{name}: missing viewBox")

        # A technical SVG asset must remain true vector content. Embedded raster
        # images would defeat the purpose of keeping these figures as SVGs.
        images = root.findall(f".//{{{SVG_NS}}}image")
        if images:
            fail(f"{name}: contains embedded raster/image elements")

        title = root.find(f"{{{SVG_NS}}}title")
        if title is None or not (title.text or "").strip():
            fail(f"{name}: missing accessible <title>")

    for readme in (ROOT / "README.md", ROOT / "docs" / "manual" / "README.md"):
        text = readme.read_text(encoding="utf-8")
        refs = [part.split('"', 1)[0] for part in text.split('src="')[1:]]
        for ref in refs:
            if "docs/manual/assets/" in ref:
                target = ROOT / ref
            elif ref.startswith("assets/") and readme.parent == ROOT / "docs" / "manual":
                target = readme.parent / ref
            else:
                continue
            if not target.is_file():
                fail(f"{readme.relative_to(ROOT)} references missing asset: {ref}")

    print(f"manual asset check: PASS ({len(REQUIRED)} SVG assets)")


if __name__ == "__main__":
    main()
