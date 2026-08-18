#!/usr/bin/env python3
"""Build the versioned Drift end-user manual PDF from the maintained ODT source.

The repository keeps an unversioned ODT source while development is unreleased.
A release version is injected only into the generated artifact filename.

SPDX-License-Identifier: GPL-3.0-or-later
"""
from __future__ import annotations

import argparse
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

VERSION_RE = re.compile(r"^(?:v)?(?P<version>\d+\.\d+\.\d+(?:-[0-9A-Za-z.-]+)?)$")


def normalize_version(value: str) -> str:
    match = VERSION_RE.fullmatch(value.strip())
    if match is None:
        raise ValueError(f"invalid release version: {value!r}")
    return match.group("version")


def artifact_name(version: str) -> str:
    return f"drift-user-manual.{normalize_version(version)}.pdf"


def find_libreoffice() -> str:
    executable = shutil.which("libreoffice") or shutil.which("soffice")
    if executable is None:
        raise RuntimeError("LibreOffice/soffice is required to build the user manual")
    return executable


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--version", required=True, help="Release version, e.g. 0.1.0 or v0.1.0")
    parser.add_argument("--source", default="docs/manual/drift-user-manual.odt")
    parser.add_argument("--output-dir", default="dist")
    return parser.parse_args()


def build(*, source: Path, output_dir: Path, version: str) -> Path:
    normalized = normalize_version(version)
    if not source.is_file():
        raise FileNotFoundError(f"manual source not found: {source}")

    office = find_libreoffice()
    output_dir.mkdir(parents=True, exist_ok=True)
    final_pdf = output_dir / artifact_name(normalized)

    with tempfile.TemporaryDirectory(prefix="drift-manual-") as temp_name:
        temp_dir = Path(temp_name)
        versioned_source = temp_dir / f"drift-user-manual.{normalized}.odt"
        shutil.copy2(source, versioned_source)

        command = [
            office,
            "--headless",
            "--convert-to",
            "pdf:writer_pdf_Export",
            "--outdir",
            str(temp_dir),
            str(versioned_source),
        ]
        completed = subprocess.run(command, check=False, text=True, capture_output=True)
        if completed.returncode != 0:
            detail = (completed.stderr or completed.stdout).strip()
            raise RuntimeError(f"LibreOffice PDF export failed: {detail}")

        generated = temp_dir / f"drift-user-manual.{normalized}.pdf"
        if not generated.is_file() or generated.stat().st_size == 0:
            detail = (completed.stdout + "\n" + completed.stderr).strip()
            raise RuntimeError(f"LibreOffice did not create the expected PDF: {detail}")
        shutil.copy2(generated, final_pdf)

    return final_pdf


def main() -> int:
    args = parse_args()
    try:
        output = build(
            source=Path(args.source),
            output_dir=Path(args.output_dir),
            version=args.version,
        )
    except (FileNotFoundError, RuntimeError, ValueError) as exc:
        print(f"manual-build error: {exc}", file=sys.stderr)
        return 2
    print(output)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
