#!/usr/bin/env python3
"""Self-tests for release-specific Drift manual snapshots.

SPDX-License-Identifier: GPL-3.0-or-later
"""
from __future__ import annotations

import os
import subprocess
import tempfile
from pathlib import Path

import freeze_user_manual as freeze


def main() -> int:
    assert freeze.normalize_version("0.1.0") == "0.1.0"
    assert freeze.normalize_version("v1.2.3") == "1.2.3"
    assert freeze.frozen_source_path("v1.2.3", Path("archive")) == Path(
        "archive/1.2.3/drift-user-manual.1.2.3.odt"
    )

    with tempfile.TemporaryDirectory() as temp_name:
        root = Path(temp_name)
        source = root / "docs/manual/drift-user-manual.odt"
        source.parent.mkdir(parents=True)
        source.write_bytes(b"manual-v1")
        release_root = root / "docs/manual/releases"
        destination = freeze.frozen_source_path("1.2.3", release_root)

        digest = freeze.freeze(data=freeze.read_worktree_source(source), destination=destination)
        assert digest == freeze.sha256_bytes(b"manual-v1")
        assert destination.read_bytes() == b"manual-v1"
        assert freeze.verify(source=source, destination=destination) == digest

        # Idempotent when the bytes are unchanged.
        assert freeze.freeze(data=b"manual-v1", destination=destination) == digest

        try:
            freeze.freeze(data=b"manual-v2", destination=destination)
        except ValueError:
            pass
        else:
            raise AssertionError("changing an existing frozen snapshot must fail")

        source.write_bytes(b"manual-v2")
        try:
            freeze.verify(source=source, destination=destination)
        except ValueError:
            pass
        else:
            raise AssertionError("source drift after freezing must fail verification")

    # Historical backfill must read the exact bytes from the requested Git ref.
    with tempfile.TemporaryDirectory() as temp_name:
        root = Path(temp_name)
        old_cwd = Path.cwd()
        try:
            os.chdir(root)
            subprocess.run(["git", "init", "-q"], check=True)
            subprocess.run(["git", "config", "user.name", "Manual Test"], check=True)
            subprocess.run(["git", "config", "user.email", "manual@example.invalid"], check=True)
            source = Path("docs/manual/drift-user-manual.odt")
            source.parent.mkdir(parents=True)
            source.write_bytes(b"historical-manual")
            subprocess.run(["git", "add", str(source)], check=True)
            subprocess.run(["git", "commit", "-qm", "historical"], check=True)
            subprocess.run(["git", "tag", "v0.1.0"], check=True)
            source.write_bytes(b"current-manual")

            historical = freeze.read_git_source("v0.1.0", source)
            assert historical == b"historical-manual"
        finally:
            os.chdir(old_cwd)

    for invalid in ("", "0.1", "release-0.1.0", "1.2.3+build"):
        try:
            freeze.normalize_version(invalid)
        except ValueError:
            pass
        else:
            raise AssertionError(f"expected invalid version to fail: {invalid!r}")

    print("manual freeze tooling: passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
