#!/usr/bin/env python3
"""Collect the selected Drift algorithm banks into release artifacts.

The collector is intentionally bank-aware so current multi-bank releases and
older Classic-only tags can be regenerated with the same release workflow.

SPDX-License-Identifier: GPL-3.0-or-later
"""
from __future__ import annotations

import argparse
import re
import shutil
from dataclasses import dataclass
from pathlib import Path

VERSION_RE = re.compile(r"^[0-9]+\.[0-9]+\.[0-9]+$")
VALID_BANKS = ("classic", "organic", "generative", "ambient", "electronica", "percussion")


@dataclass(frozen=True)
class FirmwareImage:
    bank: str
    bootloader: str
    environment: str


ALL_IMAGES = (
    FirmwareImage("classic", "new", "nanoatmega328new"),
    FirmwareImage("classic", "old", "nanoatmega328"),
    FirmwareImage("organic", "new", "nanoatmega328new_organic"),
    FirmwareImage("organic", "old", "nanoatmega328_organic"),
    FirmwareImage("generative", "new", "nanoatmega328new_generative"),
    FirmwareImage("generative", "old", "nanoatmega328_generative"),
    FirmwareImage("ambient", "new", "nanoatmega328new_ambient"),
    FirmwareImage("ambient", "old", "nanoatmega328_ambient"),
    FirmwareImage("electronica", "new", "nanoatmega328new_electronica"),
    FirmwareImage("electronica", "old", "nanoatmega328_electronica"),
    FirmwareImage("percussion", "new", "nanoatmega328new_percussion"),
    FirmwareImage("percussion", "old", "nanoatmega328_percussion"),
)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--version", required=True, help="Release version without leading v")
    parser.add_argument(
        "--banks",
        default="classic,organic,generative,ambient,electronica,percussion",
        help="Comma-separated banks to package (classic, organic, generative, ambient, electronica, percussion)",
    )
    parser.add_argument("--build-root", default=".pio/build")
    parser.add_argument("--output-dir", default="dist")
    return parser.parse_args()


def parse_banks(raw: str) -> tuple[str, ...]:
    banks = tuple(dict.fromkeys(part.strip().lower() for part in raw.split(",") if part.strip()))
    if not banks:
        raise ValueError("at least one algorithm bank must be selected")
    invalid = [bank for bank in banks if bank not in VALID_BANKS]
    if invalid:
        raise ValueError(f"unknown algorithm bank(s): {', '.join(invalid)}")
    if "classic" not in banks:
        raise ValueError("Classic must remain part of every published Drift release")
    return banks


def images_for_banks(banks: tuple[str, ...]) -> tuple[FirmwareImage, ...]:
    selected = tuple(image for image in ALL_IMAGES if image.bank in banks)
    if not selected:
        raise ValueError("selected bank set produced no firmware images")
    return selected


def artifact_name(image: FirmwareImage, version: str, extension: str) -> str:
    return (
        f"fm-drift-{image.bank}-nano-{image.bootloader}-bootloader."
        f"{version}.{extension}"
    )


def copy_firmware_images(
    build_root: Path,
    output_dir: Path,
    version: str,
    images: tuple[FirmwareImage, ...],
) -> list[str]:
    copied: list[str] = []
    for image in images:
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


def write_firmware_manifest(
    output_dir: Path,
    version: str,
    banks: tuple[str, ...],
    images: tuple[FirmwareImage, ...],
) -> str:
    name = f"FIRMWARE-ARTIFACTS.{version}.md"
    bank_names = " and ".join(bank.title() for bank in banks)
    lines = [
        f"# FM Drift firmware artifacts - {version}",
        "",
        f"This release contains the {bank_names} algorithm bank"
        f"{'s' if len(banks) > 1 else ''}. Flash exactly one HEX image for the bank "
        "and Arduino Nano bootloader used by the module.",
        "",
        "| Bank | Nano bootloader | HEX image | ELF image | Rear DIP slots |",
        "|---|---|---|---|---|",
    ]
    dip = {
        "classic": "OFF/OFF Perlin; ON/OFF Brownian; OFF/ON Bezier; ON/ON LFO",
        "organic": "OFF/OFF Fractal; ON/OFF Vector; OFF/ON Rain; ON/ON Attractor",
        "generative": "OFF/OFF Turing; ON/OFF Markov; OFF/ON Motif; ON/ON Urn",
        "ambient": "OFF/OFF Current; ON/OFF Anchor; OFF/ON Breath; ON/ON Fog",
        "electronica": "OFF/OFF Pump; ON/OFF Acid; OFF/ON Shuffle; ON/ON Polymeter",
        "percussion": "OFF/OFF Euclid; ON/OFF Repeat; OFF/ON Probability; ON/ON Humanize",
    }
    for image in images:
        hex_name = artifact_name(image, version, "hex")
        elf_name = artifact_name(image, version, "elf")
        lines.append(
            f"| **{image.bank.title()}** | {image.bootloader} | `{hex_name}` | "
            f"`{elf_name}` | {dip[image.bank]} |"
        )

    lines.extend(["", "## Which file should I flash?", ""])
    if "classic" in banks:
        lines.append("- **Classic** contains Perlin, Brownian, Bezier and LFO.")
    if "organic" in banks:
        lines.append("- **Organic** contains Fractal, Vector, Rain and Attractor.")
    if "generative" in banks:
        lines.append("- **Generative** contains Turing, Markov, Motif and Urn.")
    if "ambient" in banks:
        lines.append("- **Ambient** contains Current, Anchor, Breath and Fog.")
    if "electronica" in banks:
        lines.append("- **Electronica** contains Pump, Acid, Shuffle and Polymeter.")
    if "percussion" in banks:
        lines.append("- **Percussion** contains Euclid, Repeat, Probability and Humanize.")
    lines.extend(
        [
            "- Choose **new bootloader** for a current Arduino Nano bootloader and **old bootloader** for the legacy Nano bootloader.",
            "- The rear DIP switches select an algorithm **inside the flashed bank**; they do not switch banks.",
            "",
            "The `.elf` files are provided for engineering/debug/provenance use. Normal module flashing uses the `.hex` image.",
            "",
            "## Companion release files",
            "",
            f"- `drift-user-manual.{version}.odt` — frozen user-manual source for this release.",
            f"- `drift-user-manual.{version}.pdf` — PDF generated from that frozen source.",
        ]
    )
    for image in images:
        lines.append(
            f"- `BUILD-INFO-{image.bank}-nano-{image.bootloader}.{version}.txt` — "
            f"{image.bank.title()}/{image.bootloader}-bootloader build provenance."
        )
    lines.extend(
        [
            "- `SHA256SUMS.txt` and `MD5SUMS.txt` — checksums for all release files.",
            "",
        ]
    )
    (output_dir / name).write_text("\n".join(lines), encoding="utf-8")
    return name


def copy_release_documentation(output_dir: Path, banks: tuple[str, ...]) -> list[str]:
    required = ("README.md", "CHANGELOG.md", "LICENSE")
    copied: list[str] = []
    for documentation in required:
        source = Path(documentation)
        if not source.is_file():
            raise FileNotFoundError(f"missing release documentation: {source}")
        shutil.copy2(source, output_dir / source.name)
        copied.append(source.name)

    # Bank guides were introduced after the first Classic-only release. Copy
    # them when the source tag contains them, but remain compatible with older
    # tags that predate the split documentation.
    for bank in banks:
        source = Path(f"README-BANK-{bank.upper()}.md")
        if source.is_file():
            shutil.copy2(source, output_dir / source.name)
            copied.append(source.name)
    return copied


def main() -> int:
    args = parse_args()
    if not VERSION_RE.fullmatch(args.version):
        raise SystemExit(f"release-artifact error: invalid version {args.version!r}")
    try:
        banks = parse_banks(args.banks)
    except ValueError as exc:
        raise SystemExit(f"release-artifact error: {exc}") from exc

    images = images_for_banks(banks)
    build_root = Path(args.build_root)
    output_dir = Path(args.output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)

    copied = copy_firmware_images(build_root, output_dir, args.version, images)
    documentation = copy_release_documentation(output_dir, banks)
    manifest = write_firmware_manifest(output_dir, args.version, banks, images)
    print(
        f"release artifacts: {len(copied)} firmware files + {len(documentation)} documentation files + {manifest}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
