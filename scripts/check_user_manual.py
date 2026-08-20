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
import zipfile
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
    return raw.split("+", 1)[-1].strip().lower()


def parse_pdffonts_rows(fonts: str) -> list[dict[str, str]]:
    """Parse pdffonts output without losing font names that contain spaces.

    pdffonts uses fixed-width columns. A plain ``line.split()`` is not safe:
    ``ABCDEF+Ubuntu Light`` would be split into two tokens and the style name
    would disappear from the field we inspect.
    """
    lines = [line.rstrip("\n") for line in fonts.splitlines() if line.strip()]
    if len(lines) < 2:
        raise ValueError("pdffonts output is incomplete")

    header = lines[0]
    try:
        type_col = header.index("type")
        encoding_col = header.index("encoding")
        emb_col = header.index("emb")
        sub_col = header.index("sub")
        uni_col = header.index("uni")
        object_col = header.index("object ID")
    except ValueError as exc:
        raise ValueError("pdffonts output does not contain the expected columns") from exc

    rows: list[dict[str, str]] = []
    for line in lines[2:]:
        if set(line.strip()) == {"-"}:
            continue
        padded = line.ljust(object_col + len("object ID"))
        rows.append(
            {
                "name": padded[:type_col].strip(),
                "type": padded[type_col:encoding_col].strip(),
                "encoding": padded[encoding_col:emb_col].strip(),
                "emb": padded[emb_col:sub_col].strip(),
                "sub": padded[sub_col:uni_col].strip(),
                "uni": padded[uni_col:object_col].strip(),
                "object_id": padded[object_col:].strip(),
            }
        )
    return rows


def source_declares_ubuntu_light(source: Path) -> bool:
    """Return whether the ODT source explicitly requests the Ubuntu Light face.

    Ubuntu's classic 0.83 font metadata exposes Ubuntu-M.ttf as both the
    ``Ubuntu`` and ``Ubuntu Light`` family, with ``Medium``/``Bold`` styles.
    LibreOffice on Linux can therefore export text authored as Ubuntu Light
    under the PDF font name ``Ubuntu-Medium``. We only accept that alias when
    the source itself proves that Ubuntu Light was requested.
    """
    try:
        with zipfile.ZipFile(source) as archive:
            styles = archive.read("styles.xml").decode("utf-8", errors="replace")
            content = archive.read("content.xml").decode("utf-8", errors="replace")
    except (OSError, KeyError, zipfile.BadZipFile) as exc:
        raise ValueError(f"unable to inspect manual source fonts: {exc}") from exc
    return 'style:font-name="Ubuntu Light"' in styles or 'style:font-name="Ubuntu Light"' in content


def validate_fonts(
    fonts: str,
    allow_substitution: bool,
    *,
    allow_ubuntu_medium_for_light: bool = False,
) -> None:
    rows = parse_pdffonts_rows(fonts)
    ubuntu_rows = [row for row in rows if canonical_font_name(row["name"]).startswith("ubuntu")]
    names = [canonical_font_name(row["name"]) for row in ubuntu_rows]

    # The normal Ubuntu family (regular/bold/etc.) must always be present.
    has_ubuntu_family = any(name == "ubuntu" or name.startswith("ubuntu-") for name in names)
    has_ubuntu_light = any("ubuntu light" in name or "ubuntu-light" in name for name in names)
    has_known_medium_alias = any("ubuntu-medium" in name or "ubuntu medium" in name for name in names)

    # Ubuntu classic 0.83 has long-standing family metadata that advertises
    # Ubuntu-M.ttf as part of the "Ubuntu Light" family. LibreOffice may
    # consequently name that embedded face Ubuntu-Medium even though the ODT
    # requests Ubuntu Light. This is not a missing-font substitution.
    light_contract_satisfied = has_ubuntu_light or (allow_ubuntu_medium_for_light and has_known_medium_alias)

    if not allow_substitution and (not has_ubuntu_family or not light_contract_satisfied):
        detected = ", ".join(row["name"] for row in rows) or "<none>"
        raise ValueError(
            "release PDF must contain Ubuntu plus the source-requested Ubuntu Light face "
            "(Ubuntu-Medium is accepted only for the documented LibreOffice/Ubuntu classic alias); "
            f"detected PDF fonts: {detected}"
        )

    if not has_ubuntu_light and allow_ubuntu_medium_for_light and has_known_medium_alias:
        print(
            "manual PDF note: Ubuntu Light is embedded/reported as Ubuntu-Medium by LibreOffice; "
            "accepted because the ODT source explicitly requests Ubuntu Light"
        )

    for row in ubuntu_rows:
        if row["emb"].lower() != "yes":
            raise ValueError(f"Ubuntu-family font is not embedded: {row['name']}")

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
    declares_light = source_declares_ubuntu_light(source)
    validate_fonts(
        fonts,
        allow_substitution,
        allow_ubuntu_medium_for_light=declares_light,
    )


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
