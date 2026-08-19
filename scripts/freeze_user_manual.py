#!/usr/bin/env python3
"""Freeze or verify a release-specific Drift user-manual source snapshot.

The maintained source remains docs/manual/drift-user-manual.odt during normal
work. Release preparation copies the final source byte-for-byte into
``docs/manual/releases/X.Y.Z/drift-user-manual.X.Y.Z.odt`` before the release
commit is tagged. Existing snapshots are immutable by default.

Historical releases that predate this mechanism can be backfilled exactly from
an existing Git tag with ``--from-git-ref``; the tag is read with ``git show``
and is never modified.

SPDX-License-Identifier: GPL-3.0-or-later
"""
from __future__ import annotations

import argparse
import hashlib
import re
import subprocess
import sys
from pathlib import Path

VERSION_RE = re.compile(r"^(?:v)?(?P<version>\d+\.\d+\.\d+(?:-[0-9A-Za-z.-]+)?)$")
DEFAULT_SOURCE = Path("docs/manual/drift-user-manual.odt")
DEFAULT_RELEASE_ROOT = Path("docs/manual/releases")


def normalize_version(value: str) -> str:
    match = VERSION_RE.fullmatch(value.strip())
    if match is None:
        raise ValueError(f"invalid release version: {value!r}")
    return match.group("version")


def frozen_source_path(version: str, release_root: Path = DEFAULT_RELEASE_ROOT) -> Path:
    normalized = normalize_version(version)
    return release_root / normalized / f"drift-user-manual.{normalized}.odt"


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def read_worktree_source(source: Path) -> bytes:
    if not source.is_file():
        raise FileNotFoundError(f"manual source not found: {source}")
    data = source.read_bytes()
    if not data:
        raise ValueError(f"manual source is empty: {source}")
    return data


def read_git_source(git_ref: str, source: Path) -> bytes:
    if not git_ref.strip():
        raise ValueError("git ref must not be empty")
    spec = f"{git_ref}:{source.as_posix()}"
    completed = subprocess.run(
        ["git", "show", spec],
        check=False,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    if completed.returncode != 0:
        detail = completed.stderr.decode("utf-8", errors="replace").strip()
        raise RuntimeError(f"cannot read historical manual source {spec}: {detail}")
    if not completed.stdout:
        raise ValueError(f"historical manual source is empty: {spec}")
    return completed.stdout


def freeze(*, data: bytes, destination: Path) -> str:
    destination.parent.mkdir(parents=True, exist_ok=True)
    if destination.exists():
        existing = destination.read_bytes()
        if existing != data:
            raise ValueError(
                "frozen manual already exists with different content; "
                f"release snapshots are immutable: {destination}"
            )
        return sha256_bytes(existing)

    destination.write_bytes(data)
    return sha256_bytes(data)


def verify(*, source: Path, destination: Path) -> str:
    current = read_worktree_source(source)
    if not destination.is_file():
        raise FileNotFoundError(f"frozen manual source not found: {destination}")
    frozen = destination.read_bytes()
    if current != frozen:
        raise ValueError(
            "maintained manual source differs from the frozen release snapshot; "
            "restore the final source or, before any tag exists, deliberately remove the incorrect snapshot and freeze again"
        )
    return sha256_bytes(frozen)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--version", required=True, help="Release version, e.g. 0.2.0 or v0.2.0")
    parser.add_argument("--source", default=str(DEFAULT_SOURCE))
    parser.add_argument("--release-root", default=str(DEFAULT_RELEASE_ROOT))
    parser.add_argument(
        "--from-git-ref",
        default="",
        help="Backfill from the exact source stored at an existing Git ref/tag instead of the worktree",
    )
    parser.add_argument(
        "--check",
        action="store_true",
        help="Verify that the frozen snapshot exists and is byte-identical to the maintained source",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    try:
        version = normalize_version(args.version)
        source = Path(args.source)
        destination = frozen_source_path(version, Path(args.release_root))

        if args.check:
            if args.from_git_ref:
                raise ValueError("--check and --from-git-ref are mutually exclusive")
            digest = verify(source=source, destination=destination)
            print(f"frozen manual verified: {destination} sha256={digest}")
            return 0

        data = read_git_source(args.from_git_ref, source) if args.from_git_ref else read_worktree_source(source)
        digest = freeze(data=data, destination=destination)
        origin = args.from_git_ref or str(source)
        print(f"frozen manual: {origin} -> {destination} sha256={digest}")
        return 0
    except (FileNotFoundError, RuntimeError, ValueError) as exc:
        print(f"manual-freeze error: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
