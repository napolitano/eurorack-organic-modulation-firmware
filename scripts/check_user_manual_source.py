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


def validate(source: Path) -> None:
    if not source.is_file():
        raise FileNotFoundError(f"manual source not found: {source}")
    if not zipfile.is_zipfile(source):
        raise ValueError("manual source is not a valid ODT/ZIP container")

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

        # Organic figures are maintained as standalone deterministic SVG assets.
        # Verify that the publication source embeds the exact current files rather
        # than stale copies from an earlier manual edit.
        namespaces = {
            "draw": "urn:oasis:names:tc:opendocument:xmlns:drawing:1.0",
            "xlink": "http://www.w3.org/1999/xlink",
        }
        root = ET.fromstring(content_bytes)
        embedded_organic: dict[str, str] = {}
        draw_name = f"{{{namespaces['draw']}}}name"
        xlink_href = f"{{{namespaces['xlink']}}}href"
        for frame in root.findall(".//draw:frame", namespaces):
            frame_name = frame.attrib.get(draw_name, "")
            if not frame_name.startswith("Organic-"):
                continue
            image = frame.find("draw:image", namespaces)
            if image is None or xlink_href not in image.attrib:
                raise ValueError(f"Organic manual figure has no embedded image: {frame_name}")
            embedded_organic[frame_name.removeprefix("Organic-")] = image.attrib[xlink_href]

        required_organic_assets = (
            "organic-bank-overview.svg",
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

        # The composed front/back covers are reusable vector assets as well.
        # Their frame names are stable so stale Classic-only cover art cannot
        # silently return to a dual-bank manual.
        embedded_manual: dict[str, str] = {}
        for frame in root.findall(".//draw:frame", namespaces):
            frame_name = frame.attrib.get(draw_name, "")
            if not frame_name.startswith("Manual-"):
                continue
            image = frame.find("draw:image", namespaces)
            if image is None or xlink_href not in image.attrib:
                raise ValueError(f"manual cover frame has no embedded image: {frame_name}")
            embedded_manual[frame_name.removeprefix("Manual-")] = image.attrib[xlink_href]
        for asset_name in ("drift-front-cover.svg", "drift-back-cover.svg"):
            href = embedded_manual.get(asset_name)
            if href is None:
                raise ValueError(f"manual is missing vector cover {asset_name}")
            standalone = Path("docs/manual/assets") / asset_name
            if archive.read(href) != standalone.read_bytes():
                raise ValueError(f"manual embeds a stale copy of cover SVG {asset_name}")

    require('style:font-name="Ubuntu"', styles, "Ubuntu font style")
    require('style:font-name="Ubuntu Light"', styles, "Ubuntu Light font style")
    require("drift-user-manual", meta, "manual identifier")
    require("en-US", meta, "manual language")
    require("CC BY-NC 4.0", meta, "CC BY-NC 4.0 rights notice")

    if content.count("Mathematical foundations") != 8:
        raise ValueError("manual must contain exactly eight Mathematical foundations subsections")
    require("f(t) = 6t", content, "Perlin mathematical foundation")
    require("P(move) = c / 1024", content, "Brownian mathematical foundation")
    require("C(t,", content, "Bezier mathematical foundation")
    require("y(p) = p/a", content, "LFO mathematical foundation")
    require("Organic bank", content, "Organic bank section")
    require("Fractal mode", content, "Fractal mode section")
    require("F(t) = w", content, "Fractal mathematical foundation")
    require("Vector mode", content, "Vector mode section")
    require("φx[n+1]", content, "Vector mathematical foundation")
    require("Rain mode", content, "Rain mode section")
    require("cutoff(d) =", content, "Rain density law")
    require("α(s) = 4 + 8s", content, "Rain decay law")
    require("Attractor mode", content, "Attractor mode section")
    require("x[n+1] = 1 - a", content, "Attractor mathematical foundation")
    require("Fractal (default)", content, "Organic DIP default mapping")
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
