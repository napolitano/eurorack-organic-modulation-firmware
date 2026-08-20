#!/usr/bin/env python3
"""Validate the maintained Drift manual ODT publication contract.

SPDX-License-Identifier: GPL-3.0-or-later
"""
from __future__ import annotations

import argparse
import sys
import zipfile
import xml.etree.ElementTree as ET
from pathlib import Path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--source", default="docs/manual/drift-user-manual.odt")
    return parser.parse_args()


def require(fragment: str, text: str, description: str) -> None:
    if fragment not in text:
        raise ValueError(f"missing {description}: {fragment!r}")


def normalised_text(element: ET.Element) -> str:
    return " ".join("".join(element.itertext()).split())


def validate(source: Path) -> None:
    if not source.is_file():
        raise FileNotFoundError(f"manual source not found: {source}")
    if not zipfile.is_zipfile(source):
        raise ValueError("manual source is not a valid ODT/ZIP container")

    namespaces = {
        "draw": "urn:oasis:names:tc:opendocument:xmlns:drawing:1.0",
        "xlink": "http://www.w3.org/1999/xlink",
        "text": "urn:oasis:names:tc:opendocument:xmlns:text:1.0",
    }
    draw_name = f"{{{namespaces['draw']}}}name"
    xlink_href = f"{{{namespaces['xlink']}}}href"
    text_style = f"{{{namespaces['text']}}}style-name"

    with zipfile.ZipFile(source) as archive:
        names = set(archive.namelist())
        for required in ("mimetype", "content.xml", "styles.xml", "meta.xml"):
            if required not in names:
                raise ValueError(f"ODT is missing required member {required}")
        mimetype = archive.read("mimetype").decode("ascii", errors="strict")
        if mimetype != "application/vnd.oasis.opendocument.text":
            raise ValueError(f"unexpected ODT mimetype: {mimetype}")
        content_bytes = archive.read("content.xml")
        content = content_bytes.decode("utf-8", errors="strict")
        styles = archive.read("styles.xml").decode("utf-8", errors="strict")
        meta = archive.read("meta.xml").decode("utf-8", errors="strict")
        root = ET.fromstring(content_bytes)

        # Organic algorithm figures are maintained as standalone deterministic SVGs.
        embedded_organic: dict[str, str] = {}
        embedded_generative: dict[str, str] = {}
        embedded_ambient: dict[str, str] = {}
        embedded_electronica: dict[str, str] = {}
        embedded_percussion: dict[str, str] = {}
        embedded_dubstep: dict[str, str] = {}
        embedded_bank_config: dict[str, str] = {}
        embedded_bank_slot: dict[str, str] = {}
        embedded_manual: dict[str, str] = {}
        for frame in root.findall(".//draw:frame", namespaces):
            frame_name = frame.attrib.get(draw_name, "")
            image = frame.find("draw:image", namespaces)
            if image is None or xlink_href not in image.attrib:
                continue
            href = image.attrib[xlink_href]
            if frame_name.startswith("Organic-"):
                embedded_organic[frame_name.removeprefix("Organic-")] = href
            elif frame_name.startswith("Generative-"):
                embedded_generative[frame_name.removeprefix("Generative-")] = href
            elif frame_name.startswith("Ambient-"):
                embedded_ambient[frame_name.removeprefix("Ambient-")] = href
            elif frame_name.startswith("Electronica-"):
                embedded_electronica[frame_name.removeprefix("Electronica-")] = href
            elif frame_name.startswith("Percussion-"):
                embedded_percussion[frame_name.removeprefix("Percussion-")] = href
            elif frame_name.startswith("Dubstep-"):
                embedded_dubstep[frame_name.removeprefix("Dubstep-")] = href
            elif frame_name.startswith("BankConfig-"):
                embedded_bank_config[frame_name.removeprefix("BankConfig-")] = href
            elif frame_name.startswith("BankSlot-"):
                embedded_bank_slot[frame_name.removeprefix("BankSlot-")] = href
            elif frame_name.startswith("Manual-"):
                embedded_manual[frame_name.removeprefix("Manual-")] = href

        required_organic_assets = (
            "fractal-texture.svg",
            "vector-flow.svg",
            "rain-density.svg",
            "attractor-henon.svg",
        )
        for asset_name in required_organic_assets:
            href = embedded_organic.get(asset_name)
            if href is None:
                raise ValueError(f"manual is missing Organic SVG figure {asset_name}")
            standalone = Path("docs/manual/assets") / asset_name
            if not standalone.is_file():
                raise ValueError(f"standalone Organic SVG is missing: {standalone}")
            if archive.read(href) != standalone.read_bytes():
                raise ValueError(f"manual embeds a stale copy of Organic SVG {asset_name}")

        required_generative_assets = (
            "turing-mutation.svg",
            "markov-vocabulary.svg",
            "motif-evolution.svg",
            "urn-reinforcement.svg",
        )
        for asset_name in required_generative_assets:
            href = embedded_generative.get(asset_name)
            if href is None:
                raise ValueError(f"manual is missing Generative SVG figure {asset_name}")
            standalone = Path("docs/manual/assets") / asset_name
            if not standalone.is_file():
                raise ValueError(f"standalone Generative SVG is missing: {standalone}")
            if archive.read(href) != standalone.read_bytes():
                raise ValueError(f"manual embeds a stale copy of Generative SVG {asset_name}")


        required_ambient_assets = (
            "current-long-form.svg",
            "anchor-mean-reversion.svg",
            "breath-cycle-variation.svg",
            "fog-cloudlets.svg",
        )
        for asset_name in required_ambient_assets:
            href = embedded_ambient.get(asset_name)
            if href is None:
                raise ValueError(f"manual is missing Ambient SVG figure {asset_name}")
            standalone = Path("docs/manual/assets") / asset_name
            if not standalone.is_file():
                raise ValueError(f"standalone Ambient SVG is missing: {standalone}")
            if archive.read(href) != standalone.read_bytes():
                raise ValueError(f"manual embeds a stale copy of Ambient SVG {asset_name}")

        required_electronica_assets = (
            "pump-ducking.svg",
            "acid-contour.svg",
            "shuffle-timing.svg",
            "polymeter-cycle.svg",
        )
        for asset_name in required_electronica_assets:
            href = embedded_electronica.get(asset_name)
            if href is None:
                raise ValueError(f"manual is missing Electronica SVG figure {asset_name}")
            standalone = Path("docs/manual/assets") / asset_name
            if not standalone.is_file():
                raise ValueError(f"standalone Electronica SVG is missing: {standalone}")
            if archive.read(href) != standalone.read_bytes():
                raise ValueError(f"manual embeds a stale copy of Electronica SVG {asset_name}")

        required_percussion_assets = (
            "euclid-pattern.svg",
            "repeat-ratchets.svg",
            "probability-grid.svg",
            "humanize-timing.svg",
        )
        for asset_name in required_percussion_assets:
            href = embedded_percussion.get(asset_name)
            if href is None:
                raise ValueError(f"manual is missing Percussion SVG figure {asset_name}")
            standalone = Path("docs/manual/assets") / asset_name
            if not standalone.is_file():
                raise ValueError(f"standalone Percussion SVG is missing: {standalone}")
            if archive.read(href) != standalone.read_bytes():
                raise ValueError(f"manual embeds a stale copy of Percussion SVG {asset_name}")

        required_dubstep_assets = (
            "wobble-rate-phrase.svg",
            "growl-contour.svg",
            "chop-phrase.svg",
            "build-escalation.svg",
        )
        for asset_name in required_dubstep_assets:
            href = embedded_dubstep.get(asset_name)
            if href is None:
                raise ValueError(f"manual is missing Dubstep/Bass SVG figure {asset_name}")
            standalone = Path("docs/manual/assets") / asset_name
            if not standalone.is_file():
                raise ValueError(f"standalone Dubstep/Bass SVG is missing: {standalone}")
            if archive.read(href) != standalone.read_bytes():
                raise ValueError(f"manual embeds a stale copy of Dubstep/Bass SVG {asset_name}")

        required_bank_slot_assets = (
            "dip-slot-00.svg", "dip-slot-10.svg", "dip-slot-01.svg", "dip-slot-11.svg",
        )
        for asset_name in required_bank_slot_assets:
            href = embedded_bank_slot.get(asset_name)
            if href is None:
                raise ValueError(f"manual bank overview is missing DIP slot SVG {asset_name}")
            standalone = Path("docs/manual/assets") / asset_name
            if archive.read(href) != standalone.read_bytes():
                raise ValueError(f"manual embeds a stale DIP slot SVG {asset_name}")

        # Each bank owns its own four DIP diagrams; the manual must embed the
        # maintained SVGs directly rather than a comparison table/overview.
        required_config_assets = (
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
            "config-pump.svg",
            "config-acid.svg",
            "config-shuffle.svg",
            "config-polymeter.svg",
            "config-euclid.svg",
            "config-repeat.svg",
            "config-probability.svg",
            "config-humanize.svg",
            "config-wobble.svg",
            "config-growl.svg",
            "config-chop.svg",
            "config-build.svg",
        )
        for asset_name in required_config_assets:
            href = embedded_bank_config.get(asset_name)
            if href is None:
                raise ValueError(f"manual is missing bank-specific DIP SVG {asset_name}")
            standalone = Path("docs/manual/assets") / asset_name
            if not standalone.is_file():
                raise ValueError(f"standalone DIP SVG is missing: {standalone}")
            if archive.read(href) != standalone.read_bytes():
                raise ValueError(f"manual embeds a stale copy of DIP SVG {asset_name}")
        if "organic-bank-overview.svg" in embedded_organic:
            raise ValueError("manual still embeds the obsolete cross-bank DIP comparison")

        # Front and back covers are reusable vector assets. Stable frame names
        # prevent accidental regressions to stale or damaged cover artwork.
        for asset_name in ("drift-front-cover.svg", "drift-back-cover.svg"):
            href = embedded_manual.get(asset_name)
            if href is None:
                raise ValueError(f"manual is missing vector cover {asset_name}")
            standalone = Path("docs/manual/assets") / asset_name
            if archive.read(href) != standalone.read_bytes():
                raise ValueError(f"manual embeds a stale copy of cover SVG {asset_name}")

        # Lock the editorial hierarchy: section headings, algorithm headings and
        # mathematical subsections must use the same styles in both banks.
        paragraphs = root.findall(".//text:p", namespaces)
        paragraph_map: dict[str, list[ET.Element]] = {}
        for paragraph in paragraphs:
            paragraph_map.setdefault(normalised_text(paragraph), []).append(paragraph)

        expected_sections = (
            "4 Algorithm banks",
            "5 Classic bank",
            "6 Organic bank",
            "7 Generative bank",
            "8 Ambient bank",
            "9 Electronica bank",
            "10 Percussion bank",
            "11 Dubstep / Bass bank",
            "12 Signals at a glance",
        )
        for heading in expected_sections:
            matches = paragraph_map.get(heading, [])
            if len(matches) != 1 or matches[0].attrib.get(text_style) != "P18":
                raise ValueError(f"manual section heading is missing or mis-styled: {heading}")
            spans = matches[0].findall("text:span", namespaces)
            if [span.attrib.get(text_style) for span in spans] != ["T1", "T2"]:
                raise ValueError(f"manual section heading has inconsistent text styling: {heading}")

        mode_headings = (
            "Perlin mode (default)", "Bezier mode", "Brownian mode", "LFO mode",
            "Fractal mode", "Vector mode", "Rain mode", "Attractor mode",
            "Turing mode", "Markov mode", "Motif mode", "Urn mode",
            "Current mode", "Anchor mode", "Breath mode", "Fog mode",
            "Pump mode", "Acid mode", "Shuffle mode", "Polymeter mode",
            "Euclid mode", "Repeat mode", "Probability mode", "Humanize mode",
            "Wobble mode", "Growl mode", "Chop mode", "Build mode",
        )
        for heading in mode_headings:
            matches = paragraph_map.get(heading, [])
            if len(matches) != 1 or matches[0].attrib.get(text_style) != "P23":
                raise ValueError(f"algorithm heading is missing or mis-styled: {heading}")
            spans = matches[0].findall("text:span", namespaces)
            if len(spans) != 1 or spans[0].attrib.get(text_style) != "T15":
                raise ValueError(f"algorithm heading lacks the standard T15 style: {heading}")

        math_headings = paragraph_map.get("Mathematical foundations", [])
        if len(math_headings) != 28:
            raise ValueError("manual must contain exactly twenty-eight Mathematical foundations subsections")
        for heading in math_headings:
            spans = heading.findall("text:span", namespaces)
            if heading.attrib.get(text_style) != "P20" or len(spans) != 1 or spans[0].attrib.get(text_style) != "T15":
                raise ValueError("Mathematical foundations heading has inconsistent formatting")

        dip_headings = paragraph_map.get("DIP switch selection", [])
        if len(dip_headings) != 7:
            raise ValueError("manual must contain one DIP switch selection subsection per bank")
        for heading in dip_headings:
            spans = heading.findall("text:span", namespaces)
            if heading.attrib.get(text_style) != "P20" or len(spans) != 1 or spans[0].attrib.get(text_style) != "T15":
                raise ValueError("DIP switch selection heading has inconsistent formatting")

        if paragraph_map.get("5 Configuration"):
            raise ValueError("obsolete cross-bank Configuration section is still present")

    require('style:font-name="Ubuntu"', styles, "Ubuntu font style")
    require('style:font-name="Ubuntu Light"', styles, "Ubuntu Light font style")
    require("drift-user-manual", meta, "manual identifier")
    require("en-US", meta, "manual language")
    require("CC BY-NC 4.0", meta, "CC BY-NC 4.0 rights notice")

    require("Drift started with four algorithm concepts", content, "Algorithm banks introduction")
    require("Classic bank — DIP-switch positions", content, "Classic bank DIP caption")
    require("Organic bank — DIP-switch positions", content, "Organic bank DIP caption")
    require("Generative bank — DIP-switch positions", content, "Generative bank DIP caption")
    require("Ambient bank — DIP-switch positions", content, "Ambient bank DIP caption")
    require("Electronica bank — DIP-switch positions", content, "Electronica bank DIP caption")
    require("Percussion bank — DIP-switch positions", content, "Percussion bank DIP caption")
    require("Dubstep / Bass bank — DIP-switch positions", content, "Dubstep/Bass bank DIP caption")
    require("f(t) = 6t", content, "Perlin mathematical foundation")
    require("P(move) = c / 1024", content, "Brownian mathematical foundation")
    require("C(t,", content, "Bezier mathematical foundation")
    require("y(p) = p/a", content, "LFO mathematical foundation")
    require("F(t) = w", content, "Fractal mathematical foundation")
    require("φx[n+1]", content, "Vector mathematical foundation")
    require("cutoff(d) =", content, "Rain density law")
    require("α(s) = 4 + 8s", content, "Rain decay law")
    require("x[n+1] = 1 - a", content, "Attractor mathematical foundation")
    require("b' = b", content, "Turing mathematical foundation")
    require("Pτ =", content, "Markov mathematical foundation")
    require("P(edit) =", content, "Motif mathematical foundation")
    require("P(X = i)", content, "Urn mathematical foundation")
    require("f₀ : f₁ : f₂", content, "Current mathematical foundation")
    require("g(a) ≈ √(6(1 − a²))", content, "Anchor innovation normalisation")
    require("S(t) = 3t² − 2t³", content, "Breath smoothstep foundation")
    require("g(u) = 16u²(1 − u)²", content, "Fog cloudlet kernel")
    require("B(u) = 30 · 2^(3u) BPM", content, "Electronica tempo law")
    require("qₙ = (5n + 3) mod 16", content, "Acid permutation law")
    require("r(τ) = 1/2 + τ/4", content, "Shuffle timing law")
    require("P = lcm(4, b)", content, "Polymeter recurrence law")
    require("k = 2 + floor(12t / 1024)", content, "Euclid density law")
    require("P(repeat) = 3τ / 4", content, "Repeat probability law")
    require("Psecondary = τ", content, "Probability secondary law")
    require("Pghost = τ² / 2", content, "Probability ghost law")
    require("|δₙ| ≤ 12 ms", content, "Humanize timing bound")
    require("W(φ) = 1 − |2φ − 1|", content, "Wobble triangle foundation")
    require("G(φ,τ) =", content, "Growl compound-contour foundation")
    require("k(T) = floor(9T / 1024)", content, "Chop density law")
    require("M(u) = 3u² − 2u³", content, "Build macro-rise law")
    require("Y(u) = M(u)", content, "Build composite-output law")
    require("twenty-eight algorithms across seven", content, "seven-bank capability overview")
    require("Software can add new banks, but it cannot create additional physical switch states", content, "hardware selector limitation")
    require("WARNING — PERCUSSION / DUBSTEP CLOCK INPUT", content, "shared Percussion/Dubstep clock safety warning")
    require("Do not patch 10 V Eurorack triggers or clocks into Speed CV", content, "Percussion 10 V warning")
    require("return automatically to the Speed-knob clock", content, "Percussion/Dubstep clock-loss fallback")
    require("Musical character and origin", content, "musical value table section")
    require("flashed firmware image", content, "compile-time bank selection explanation")
    require("symmetric triangular random offset", content, "Bezier timing-distribution description")
    require("Brownian is deliberately different", content, "Brownian Speed semantic distinction")
    if "increasingly wider Gaussian distribution" in content:
        raise ValueError("manual still contains obsolete Gaussian Bezier timing claim")


def main() -> int:
    args = parse_args()
    try:
        validate(Path(args.source))
    except (FileNotFoundError, ValueError, zipfile.BadZipFile) as exc:
        print(f"manual-source error: {exc}", file=sys.stderr)
        return 2
    print("manual source contract: passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
