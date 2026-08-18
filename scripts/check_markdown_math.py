#!/usr/bin/env python3
"""Validate GitHub-compatible mathematical markup in repository Markdown files."""

from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
LEGACY_PATTERNS = {
    "\\\\(": "legacy inline math opener \\\\(",
    "\\\\)": "legacy inline math closer \\\\)",
    "\\\\[": "legacy display math opener \\\\[",
    "\\\\]": "legacy display math closer \\\\]",
}


def strip_fenced_code(text: str) -> str:
    """Remove fenced code blocks before checking Markdown math delimiters."""
    out: list[str] = []
    in_fence = False
    fence_char = ""
    fence_len = 0

    for line in text.splitlines(keepends=True):
        match = re.match(r"^[ \\t]*(`{3,}|~{3,})", line)
        if match:
            marker = match.group(1)
            if not in_fence:
                in_fence = True
                fence_char = marker[0]
                fence_len = len(marker)
            elif marker[0] == fence_char and len(marker) >= fence_len:
                in_fence = False
                fence_char = ""
                fence_len = 0
            out.append("\n")
            continue
        out.append("\n" if in_fence else line)

    return "".join(out)


def main() -> int:
    errors: list[str] = []
    markdown_files = sorted(
        path for path in ROOT.rglob("*.md") if ".git" not in path.parts
    )

    for path in markdown_files:
        text = path.read_text(encoding="utf-8")
        visible = strip_fenced_code(text)
        rel = path.relative_to(ROOT)

        for needle, description in LEGACY_PATTERNS.items():
            if needle in visible:
                errors.append(
                    f"{rel}: contains {description}; use GitHub $...$ or $$...$$ math"
                )

        display_count = visible.count("$$")
        if display_count % 2 != 0:
            errors.append(
                f"{rel}: contains an unbalanced number of $$ display-math delimiters "
                f"({display_count})"
            )

    if errors:
        print("Markdown math validation failed:", file=sys.stderr)
        for error in errors:
            print(f"- {error}", file=sys.stderr)
        return 1

    print(f"Markdown math validation passed for {len(markdown_files)} files")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
