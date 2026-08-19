#!/usr/bin/env python3
"""Self-test the bank-aware release artifact collector.

SPDX-License-Identifier: GPL-3.0-or-later
"""
from __future__ import annotations

import os
import tempfile
from pathlib import Path

import prepare_release_artifacts as release


def create_builds(build_root: Path, images: tuple[release.FirmwareImage, ...]) -> None:
    for image in images:
        env = build_root / image.environment
        env.mkdir(parents=True)
        (env / "firmware.hex").write_text(f"hex-{image.bank}-{image.bootloader}", encoding="ascii")
        (env / "firmware.elf").write_text(f"elf-{image.bank}-{image.bootloader}", encoding="ascii")


def main() -> int:
    with tempfile.TemporaryDirectory() as temp_dir:
        root = Path(temp_dir)
        old_cwd = Path.cwd()
        try:
            os.chdir(root)
            for name in ("README.md", "CHANGELOG.md", "LICENSE"):
                Path(name).write_text(name, encoding="utf-8")
            Path("README-BANK-CLASSIC.md").write_text("Classic", encoding="utf-8")
            Path("README-BANK-ORGANIC.md").write_text("Organic", encoding="utf-8")

            build_root = root / "build"
            output_dir = root / "dist"
            output_dir.mkdir()
            banks = release.parse_banks("classic,organic")
            images = release.images_for_banks(banks)
            create_builds(build_root, images)

            copied = release.copy_firmware_images(build_root, output_dir, "1.2.3", images)
            assert len(copied) == 8
            assert len(set(copied)) == 8
            docs = release.copy_release_documentation(output_dir, banks)
            assert "README-BANK-CLASSIC.md" in docs
            assert "README-BANK-ORGANIC.md" in docs

            manifest = release.write_firmware_manifest(output_dir, "1.2.3", banks, images)
            text = (output_dir / manifest).read_text(encoding="utf-8")
            assert "Classic" in text and "Organic" in text
            assert "OFF/OFF Perlin" in text
            assert "OFF/OFF Fractal" in text
            assert "fm-drift-classic-nano-new-bootloader.1.2.3.hex" in text
            assert "fm-drift-organic-nano-old-bootloader.1.2.3.hex" in text
            assert "BUILD-INFO-classic-nano-old.1.2.3.txt" in text
            assert "BUILD-INFO-organic-nano-new.1.2.3.txt" in text
            assert "drift-user-manual.1.2.3.odt" in text
            assert "PDF generated from that frozen source" in text

            classic_output = root / "dist-classic"
            classic_output.mkdir()
            classic_banks = release.parse_banks("classic")
            classic_images = release.images_for_banks(classic_banks)
            release.copy_firmware_images(build_root, classic_output, "0.1.0", classic_images)
            classic_manifest = release.write_firmware_manifest(
                classic_output, "0.1.0", classic_banks, classic_images
            )
            classic_text = (classic_output / classic_manifest).read_text(encoding="utf-8")
            assert "Organic" not in classic_text
            assert "Fractal" not in classic_text
            assert "fm-drift-classic-nano-old-bootloader.0.1.0.hex" in classic_text
        finally:
            os.chdir(old_cwd)

    print("release artifact tooling: passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
