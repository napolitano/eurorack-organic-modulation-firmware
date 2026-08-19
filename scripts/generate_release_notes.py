#!/usr/bin/env python3
"""Generate deterministic GitHub Release notes from CHANGELOG.md.

A tagged release must have a matching versioned changelog section. That
section must contain ``### Release summary`` followed by one prose paragraph.
The public notes are emitted as summary, detailed changelog excerpt, artifact
integrity note and compare/commit link.

SPDX-License-Identifier: GPL-3.0-or-later
"""
from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

VERSION_HEADING_RE = re.compile(r"^##\s+([^\s]+)\s+—\s+.+$")
SUBHEADING_RE = re.compile(r"^###\s+(.+?)\s*$")
FOOTER_START = "<!-- drift-footer:start -->"


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--changelog", default="CHANGELOG.md")
    parser.add_argument("--tag", required=True, help="Release tag, e.g. v0.1.0")
    parser.add_argument("--repository", required=True, help="OWNER/REPO")
    parser.add_argument("--server-url", default="https://github.com")
    parser.add_argument("--previous-tag", default="")
    return parser.parse_args()


def extract_version_section(text: str, version: str) -> list[str]:
    lines = text.splitlines()
    start: int | None = None
    for index, line in enumerate(lines):
        match = VERSION_HEADING_RE.match(line)
        if match and match.group(1) == version:
            start = index + 1
            break
    if start is None:
        raise ValueError(f"CHANGELOG.md has no release section for {version}")

    end = len(lines)
    for index in range(start, len(lines)):
        if lines[index].startswith("## ") or lines[index].strip() == FOOTER_START:
            end = index
            break
    return lines[start:end]


def split_summary(section: list[str]) -> tuple[str, list[str]]:
    summary_heading: int | None = None
    for index, line in enumerate(section):
        match = SUBHEADING_RE.match(line)
        if match and match.group(1).strip().lower() == "release summary":
            summary_heading = index
            break
    if summary_heading is None:
        raise ValueError("release section is missing '### Release summary'")

    summary_lines: list[str] = []
    index = summary_heading + 1
    while index < len(section) and not section[index].strip():
        index += 1
    while index < len(section) and not section[index].startswith("### "):
        if section[index].strip():
            summary_lines.append(section[index].strip())
        elif summary_lines:
            break
        index += 1

    summary = " ".join(summary_lines).strip()
    if not summary:
        raise ValueError("'### Release summary' must contain one prose paragraph")
    if len(summary_lines) > 7:
        raise ValueError("'### Release summary' must not exceed 7 non-empty source lines")

    detail = section[:summary_heading] + section[index:]
    while detail and not detail[0].strip():
        detail.pop(0)
    while detail and not detail[-1].strip():
        detail.pop()
    return summary, detail


def render_notes(*, text: str, tag: str, repository: str, server_url: str, previous_tag: str = "") -> str:
    version = tag[1:] if tag.startswith("v") else tag
    section = extract_version_section(text, version)
    summary, detail = split_summary(section)

    output = [summary, "", "## Changelog", ""]
    if detail:
        output.extend(["\n".join(detail), ""])
    output.extend([
        "## Artifact integrity",
        "",
        "Release assets include versioned Classic and Organic firmware images for both Nano bootloaders, the versioned user manual, `FIRMWARE-ARTIFACTS.X.Y.Z.md`, per-image build provenance, and `SHA256SUMS.txt` / `MD5SUMS.txt` covering every generated release file. Use the SHA-256 manifest for normal integrity verification; the MD5 manifest is provided only as an additional compatibility checksum.",
        "",
        "## Diff",
        "",
    ])
    base = server_url.rstrip("/")
    if previous_tag:
        output.append(f"[{previous_tag}...{tag}]({base}/{repository}/compare/{previous_tag}...{tag})")
    else:
        output.append(f"[{tag}]({base}/{repository}/commits/{tag})")
    return "\n".join(output) + "\n"


def main() -> int:
    args = parse_args()
    text = Path(args.changelog).read_text(encoding="utf-8")
    try:
        notes = render_notes(
            text=text,
            tag=args.tag,
            repository=args.repository,
            server_url=args.server_url,
            previous_tag=args.previous_tag,
        )
    except ValueError as exc:
        print(f"release-notes error: {exc}", file=sys.stderr)
        return 2
    print(notes, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
