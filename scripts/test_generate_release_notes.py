#!/usr/bin/env python3
"""Self-tests for deterministic changelog-based release-note generation."""
from __future__ import annotations

from generate_release_notes import extract_version_section, render_notes, split_summary


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def main() -> int:
    changelog = """# Changelog

## Unreleased

### Added
- Work in progress.

## 0.2.0 — 2026-09-01

### Release summary

This release corrects the core algorithms and establishes the verified firmware baseline.

### Added
- Mathematical tests.

### Fixed
- Brownian smoothing.

## 0.1.0 — 2026-08-20

### Release summary

Initial public release.

### Added
- Initial firmware.

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="docs/assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
"""
    section = extract_version_section(changelog, "0.2.0")
    summary, detail = split_summary(section)
    require(summary.startswith("This release corrects"), "summary extraction failed")
    require(any("Mathematical tests" in line for line in detail), "detail extraction failed")
    require(not any("Release summary" in line for line in detail), "summary leaked into detail")

    notes = render_notes(
        text=changelog,
        tag="v0.2.0",
        previous_tag="v0.1.0",
        repository="napolitano/eurorack-organic-modulation-firmware",
        server_url="https://github.com",
    )
    require("v0.1.0...v0.2.0" in notes, "compare link missing")
    require("## Artifact integrity" in notes, "integrity section missing")
    require("Classic and Organic firmware images" in notes, "dual-bank artifact description missing")
    require("FIRMWARE-ARTIFACTS.X.Y.Z.md" in notes, "firmware manifest description missing")
    require("frozen versioned user-manual ODT" in notes, "frozen manual source description missing")
    require("drift-footer" not in notes, "documentation footer leaked into release notes")
    require("From Munich with" not in notes, "documentation footer text leaked into release notes")

    classic_notes = render_notes(
        text=changelog,
        tag="v0.1.0",
        repository="napolitano/eurorack-organic-modulation-firmware",
        server_url="https://github.com",
        banks=("classic",),
    )
    require("versioned Classic firmware images" in classic_notes, "Classic-only artifact description missing")
    require("Organic firmware images" not in classic_notes, "Classic-only notes mention Organic artifacts")

    try:
        extract_version_section(changelog, "9.9.9")
    except ValueError:
        pass
    else:
        raise AssertionError("missing version must fail")

    try:
        split_summary(["", "### Added", "- item"])
    except ValueError:
        pass
    else:
        raise AssertionError("missing release summary must fail")

    print("release-notes-tests: passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
