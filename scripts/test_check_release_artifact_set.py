#!/usr/bin/env python3
"""Self-test the exact release-artifact-set validator.

SPDX-License-Identifier: GPL-3.0-or-later
"""
from __future__ import annotations

import tempfile
from pathlib import Path

import check_release_artifact_set as checker


def populate(root: Path, version: str, banks: tuple[str, ...]) -> None:
    firmware = checker.expected_firmware_names(version, banks)
    build_info = checker.expected_build_info_names(version, banks)
    for name in firmware | build_info:
        (root / name).write_text(name, encoding="utf-8")
    manifest = root / f"FIRMWARE-ARTIFACTS.{version}.md"
    manifest.write_text("\n".join(sorted(firmware | build_info)), encoding="utf-8")


def main() -> int:
    version = "1.2.3"
    banks = ("classic", "organic", "generative", "ambient", "electronica", "percussion")
    with tempfile.TemporaryDirectory() as temp_dir:
        root = Path(temp_dir)
        populate(root, version, banks)
        assert checker.validate_release_artifacts(root, version, banks) == []
        assert len(checker.expected_firmware_names(version, banks)) == 24
        assert len(checker.expected_build_info_names(version, banks)) == 12

        ambient_elf = root / f"fm-drift-ambient-nano-old-bootloader.{version}.elf"
        ambient_elf.unlink()
        errors = checker.validate_release_artifacts(root, version, banks)
        assert any("ambient-nano-old-bootloader" in error for error in errors)

        populate(root, version, banks)
        ambient_build_info = root / f"BUILD-INFO-ambient-nano-new.{version}.txt"
        ambient_build_info.unlink()
        errors = checker.validate_release_artifacts(root, version, banks)
        assert any("BUILD-INFO-ambient-nano-new" in error for error in errors)

        populate(root, version, banks)
        electronica_hex = root / f"fm-drift-electronica-nano-old-bootloader.{version}.hex"
        electronica_hex.unlink()
        errors = checker.validate_release_artifacts(root, version, banks)
        assert any("electronica-nano-old-bootloader" in error for error in errors)

        populate(root, version, banks)
        electronica_build_info = root / f"BUILD-INFO-electronica-nano-new.{version}.txt"
        electronica_build_info.unlink()
        errors = checker.validate_release_artifacts(root, version, banks)
        assert any("BUILD-INFO-electronica-nano-new" in error for error in errors)

        populate(root, version, banks)
        electronica_elf = root / f"fm-drift-electronica-nano-new-bootloader.{version}.elf"
        electronica_elf.unlink()
        errors = checker.validate_release_artifacts(root, version, banks)
        assert any("electronica-nano-new-bootloader" in error for error in errors)

        populate(root, version, banks)
        electronica_old_build_info = root / f"BUILD-INFO-electronica-nano-old.{version}.txt"
        electronica_old_build_info.unlink()
        errors = checker.validate_release_artifacts(root, version, banks)
        assert any("BUILD-INFO-electronica-nano-old" in error for error in errors)

        populate(root, version, banks)
        percussion_hex = root / f"fm-drift-percussion-nano-old-bootloader.{version}.hex"
        percussion_hex.unlink()
        errors = checker.validate_release_artifacts(root, version, banks)
        assert any("percussion-nano-old-bootloader" in error for error in errors)

        populate(root, version, banks)
        percussion_info = root / f"BUILD-INFO-percussion-nano-new.{version}.txt"
        percussion_info.unlink()
        errors = checker.validate_release_artifacts(root, version, banks)
        assert any("BUILD-INFO-percussion-nano-new" in error for error in errors)

        populate(root, version, banks)
        generative_hex = root / f"fm-drift-generative-nano-new-bootloader.{version}.hex"
        generative_hex.unlink()
        errors = checker.validate_release_artifacts(root, version, banks)
        assert any("generative-nano-new-bootloader" in error for error in errors)

        populate(root, version, banks)
        extra = root / f"fm-drift-experimental-nano-new-bootloader.{version}.hex"
        extra.write_text("unexpected", encoding="utf-8")
        errors = checker.validate_release_artifacts(root, version, banks)
        assert any("unexpected release firmware" in error for error in errors)

    with tempfile.TemporaryDirectory() as temp_dir:
        root = Path(temp_dir)
        classic = ("classic",)
        populate(root, "0.1.0", classic)
        assert checker.validate_release_artifacts(root, "0.1.0", classic) == []

    with tempfile.TemporaryDirectory() as temp_dir:
        root = Path(temp_dir)
        classic_ambient = ("classic", "ambient")
        populate(root, version, classic_ambient)
        assert checker.validate_release_artifacts(root, version, classic_ambient) == []
        assert len(checker.expected_firmware_names(version, classic_ambient)) == 8
        assert len(checker.expected_build_info_names(version, classic_ambient)) == 4

    print("release artifact set tooling: passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
