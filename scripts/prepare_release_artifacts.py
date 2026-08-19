#!/usr/bin/env python3
"""Collect both Drift algorithm banks into an unambiguous release artifact set.

SPDX-License-Identifier: GPL-3.0-or-later
"""
from __future__ import annotations

import argparse
import re
import shutil
from dataclasses import dataclass
from pathlib import Path

VERSION_RE = re.compile(r"^[0-9]+\.[0-9]+\.[0-9]+$")


@dataclass(frozen=True)
class FirmwareImage:
    bank: str
    bootloader: str
    environment: str


IMAGES = (
    FirmwareImage("classic", "new", "nanoatmega328new"),
    FirmwareImage("classic", "old", "nanoatmega328"),
    FirmwareImage("organic", "new", "nanoatmega328new_organic"),
    FirmwareImage("organic", "old", "nanoatmega328_organic"),
)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--version", required=True, help="Release version without leading v")
    parser.add_argument("--build-root", default=".pio/build")
    parser.add_argument("--output-dir", default="dist")
    return parser.parse_args()


def artifact_name(image: FirmwareImage, version: str, extension: str) -> str:
    return (
        f"fm-drift-{image.bank}-nano-{image.bootloader}-bootloader."
        f"{version}.{extension}"
    )


def copy_firmware_images(build_root: Path, output_dir: Path, version: str) -> list[str]:
    copied: list[str] = []
    for image in IMAGES:
        environment_dir = build_root / image.environment
        for extension in ("hex", "elf"):
            source = environment_dir / f"firmware.{extension}"
            if not source.is_file():
                raise FileNotFoundError(
                    f"missing {image.bank}/{image.bootloader} firmware image: {source}"
                )
            destination_name = artifact_name(image, version, extension)
            shutil.copy2(source, output_dir / destination_name)
            copied.append(destination_name)
    return copied


def write_firmware_manifest(output_dir: Path, version: str) -> str:
    name = f"FIRMWARE-ARTIFACTS.{version}.md"
    lines = [
        f"# FM Drift firmware artifacts - {version}",
        "",
        "Each release contains two independent compile-time algorithm banks. "
        "Flash exactly one HEX image for the bank and Arduino Nano bootloader used by the module.",
        "",
        "| Bank | Nano bootloader | HEX image | ELF image | Rear DIP slots |",
        "|---|---|---|---|---|",
    ]
    dip = {
        "classic": "OFF/OFF Perlin; ON/OFF Brownian; OFF/ON Bezier; ON/ON LFO",
        "organic": "OFF/OFF Fractal; ON/OFF Vector; OFF/ON Rain; ON/ON Attractor",
    }
    for image in IMAGES:
        hex_name = artifact_name(image, version, "hex")
        elf_name = artifact_name(image, version, "elf")
        lines.append(
            f"| **{image.bank.title()}** | {image.bootloader} | `{hex_name}` | "
            f"`{elf_name}` | {dip[image.bank]} |"
        )
    lines.extend(
        [
            "",
            "## Which file should I flash?",
            "",
            "- **Classic** preserves the four Drift algorithms derived from the original firmware.",
            "- **Organic** provides Fractal, Vector, Rain and Attractor in the same four physical DIP slots.",
            "- Choose **new bootloader** for a current Arduino Nano bootloader and **old bootloader** for the legacy Nano bootloader.",
            "- The rear DIP switches select an algorithm **inside the flashed bank**; they do not switch banks.",
            "",
            "The `.elf` files are provided for engineering/debug/provenance use. Normal module flashing uses the `.hex` image.",
            "",
            "## Companion release files",
            "",
            f"- `drift-user-manual.{version}.pdf` — versioned user manual covering both banks.",
            f"- `BUILD-INFO-classic-nano-new.{version}.txt` — Classic/new-bootloader build provenance.",
            f"- `BUILD-INFO-classic-nano-old.{version}.txt` — Classic/old-bootloader build provenance.",
            f"- `BUILD-INFO-organic-nano-new.{version}.txt` — Organic/new-bootloader build provenance.",
            f"- `BUILD-INFO-organic-nano-old.{version}.txt` — Organic/old-bootloader build provenance.",
            "- `SHA256SUMS.txt` and `MD5SUMS.txt` — checksums for all release files.",
            "",
        ]
    )
    (output_dir / name).write_text("\n".join(lines), encoding="utf-8")
    return name


def main() -> int:
    args = parse_args()
    if not VERSION_RE.fullmatch(args.version):
        raise SystemExit(f"release-artifact error: invalid version {args.version!r}")

    build_root = Path(args.build_root)
    output_dir = Path(args.output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)

    copied = copy_firmware_images(build_root, output_dir, args.version)
    for documentation in ("README.md", "CHANGELOG.md", "LICENSE"):
        source = Path(documentation)
        if not source.is_file():
            raise FileNotFoundError(f"missing release documentation: {source}")
        shutil.copy2(source, output_dir / source.name)

    manifest = write_firmware_manifest(output_dir, args.version)
    print(f"release artifacts: {len(copied)} firmware files + {manifest}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
