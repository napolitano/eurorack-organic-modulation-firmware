#!/usr/bin/env python3
"""Self-test the dual-bank release artifact collector.

SPDX-License-Identifier: GPL-3.0-or-later
"""
from __future__ import annotations

import tempfile
from pathlib import Path

import prepare_release_artifacts as release


def main() -> int:
    with tempfile.TemporaryDirectory() as temp_dir:
        root = Path(temp_dir)
        build_root = root / "build"
        output_dir = root / "dist"
        output_dir.mkdir()

        for image in release.IMAGES:
            env = build_root / image.environment
            env.mkdir(parents=True)
            (env / "firmware.hex").write_text(f"hex-{image.bank}-{image.bootloader}", encoding="ascii")
            (env / "firmware.elf").write_text(f"elf-{image.bank}-{image.bootloader}", encoding="ascii")

        copied = release.copy_firmware_images(build_root, output_dir, "1.2.3")
        assert len(copied) == 8
        assert len(set(copied)) == 8
        for image in release.IMAGES:
            for extension in ("hex", "elf"):
                expected = release.artifact_name(image, "1.2.3", extension)
                assert (output_dir / expected).is_file(), expected

        manifest = release.write_firmware_manifest(output_dir, "1.2.3")
        text = (output_dir / manifest).read_text(encoding="utf-8")
        assert "Classic" in text and "Organic" in text
        assert "OFF/OFF Perlin" in text
        assert "OFF/OFF Fractal" in text
        assert "fm-drift-classic-nano-new-bootloader.1.2.3.hex" in text
        assert "fm-drift-organic-nano-old-bootloader.1.2.3.hex" in text
        assert "BUILD-INFO-classic-nano-old.1.2.3.txt" in text
        assert "BUILD-INFO-organic-nano-new.1.2.3.txt" in text
        assert "drift-user-manual.1.2.3.pdf" in text

    print("release artifact tooling: passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
