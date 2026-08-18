#!/usr/bin/env python3
"""Validate the maintained Drift manual ODT publication contract.

SPDX-License-Identifier: GPL-3.0-or-later
"""
from __future__ import annotations

import argparse
import sys
import zipfile
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
        content = archive.read("content.xml").decode("utf-8", errors="strict")
        styles = archive.read("styles.xml").decode("utf-8", errors="strict")
        meta = archive.read("meta.xml").decode("utf-8", errors="strict")

    require('style:font-name="Ubuntu"', styles, "Ubuntu font style")
    require('style:font-name="Ubuntu Light"', styles, "Ubuntu Light font style")
    require("drift-user-manual", meta, "manual identifier")
    require("en-US", meta, "manual language")
    require("CC BY-NC 4.0", meta, "CC BY-NC 4.0 rights notice")

    if content.count("Mathematical foundations") != 4:
        raise ValueError("manual must contain exactly four Mathematical foundations subsections")
    require("f(t) = 6t", content, "Perlin mathematical foundation")
    require("P(move) = c / 1024", content, "Brownian mathematical foundation")
    require("C(t,", content, "Bezier mathematical foundation")
    require("y(p) = p/a", content, "LFO mathematical foundation")
    require("symmetric triangular random offset", content, "Bezier timing-distribution description")
    require("Brownian mode is deliberately different", content, "Brownian Speed semantic distinction")
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
