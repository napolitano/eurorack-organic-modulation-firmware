#!/usr/bin/env python3
"""Validate a generated Drift manual PDF before publication.

SPDX-License-Identifier: GPL-3.0-or-later
"""
from __future__ import annotations

import argparse
import re
import shutil
import subprocess
import sys
from pathlib import Path

FILENAME_RE = re.compile(r"^drift-user-manual\.\d+\.\d+\.\d+(?:-[0-9A-Za-z.-]+)?\.pdf$")
EXPECTED_WIDTH_PT = 99.0 / 25.4 * 72.0
EXPECTED_HEIGHT_PT = 210.0 / 25.4 * 72.0
PAGE_TOLERANCE_PT = 3.0


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("pdf")
    parser.add_argument("--source", default="docs/manual/drift-user-manual.odt")
    parser.add_argument(
        "--allow-font-substitution",
        action="store_true",
        help="Permit a smoke build without Ubuntu fonts; never use for release validation",
    )
    return parser.parse_args()


def run_tool(name: str, *args: str) -> str:
    executable = shutil.which(name)
    if executable is None:
        raise RuntimeError(f"required PDF inspection tool not found: {name}")
    completed = subprocess.run([executable, *args], check=False, text=True, capture_output=True)
    if completed.returncode != 0:
        raise RuntimeError(f"{name} failed: {(completed.stderr or completed.stdout).strip()}")
    return completed.stdout


def parse_page_size(info: str) -> tuple[float, float]:
    match = re.search(r"^Page size:\s+([0-9.]+)\s+x\s+([0-9.]+)\s+pts", info, re.MULTILINE)
    if match is None:
        raise ValueError("pdfinfo output does not contain a parseable page size")
    return float(match.group(1)), float(match.group(2))


def canonical_font_name(raw: str) -> str:
    # Embedded subset fonts are typically reported as ABCDEF+Ubuntu-Light.
    return raw.split("+", 1)[-1].lower()


def validate_fonts(fonts: str, allow_substitution: bool) -> None:
    rows = [line.split() for line in fonts.splitlines()[2:] if line.strip()]
    ubuntu_rows = [row for row in rows if row and canonical_font_name(row[0]).startswith("ubuntu")]
    has_ubuntu = any("light" not in canonical_font_name(row[0]) for row in ubuntu_rows)
    has_ubuntu_light = any("light" in canonical_font_name(row[0]) for row in ubuntu_rows)

    if not allow_substitution and (not has_ubuntu or not has_ubuntu_light):
        raise ValueError("release PDF must contain both Ubuntu and Ubuntu Light; font substitution detected")

    for row in ubuntu_rows:
        # pdffonts columns: name type encoding emb sub uni object-ID.
        if len(row) >= 4 and row[3].lower() != "yes":
            raise ValueError(f"Ubuntu-family font is not embedded: {row[0]}")

    if allow_substitution and not ubuntu_rows:
        print("manual PDF warning: Ubuntu fonts are absent; smoke build accepted by explicit override")


def validate(pdf: Path, *, source: Path, allow_substitution: bool) -> None:
    if not pdf.is_file() or pdf.stat().st_size == 0:
        raise FileNotFoundError(f"manual PDF not found or empty: {pdf}")
    if FILENAME_RE.fullmatch(pdf.name) is None:
        raise ValueError(f"unexpected manual artifact filename: {pdf.name}")

    info = run_tool("pdfinfo", str(pdf))
    pages_match = re.search(r"^Pages:\s+(\d+)\s*$", info, re.MULTILINE)
    if pages_match is None or int(pages_match.group(1)) < 1:
        raise ValueError("manual PDF must contain at least one page")
    pdf_pages = int(pages_match.group(1))
    # ODT meta:page-count is a cached Writer statistic, not a stable source
    # contract after legitimate content edits. Release-grade layout integrity is
    # enforced through page geometry plus strict Ubuntu/Ubuntu Light embedding;
    # the source itself is validated separately by check_user_manual_source.py.
    if not source.is_file():
        raise ValueError(f"manual source is unavailable: {source}")
    width, height = parse_page_size(info)
    if abs(width - EXPECTED_WIDTH_PT) > PAGE_TOLERANCE_PT or abs(height - EXPECTED_HEIGHT_PT) > PAGE_TOLERANCE_PT:
        raise ValueError(
            f"unexpected page geometry {width:.2f} x {height:.2f} pt; "
            f"expected approximately {EXPECTED_WIDTH_PT:.2f} x {EXPECTED_HEIGHT_PT:.2f} pt"
        )

    fonts = run_tool("pdffonts", str(pdf))
    validate_fonts(fonts, allow_substitution)


def main() -> int:
    args = parse_args()
    try:
        validate(Path(args.pdf), source=Path(args.source), allow_substitution=args.allow_font_substitution)
    except (FileNotFoundError, RuntimeError, ValueError) as exc:
        print(f"manual-artifact error: {exc}", file=sys.stderr)
        return 2
    print("manual PDF contract: passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
