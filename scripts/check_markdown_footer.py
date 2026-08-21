#!/usr/bin/env python3
"""Validate the shared Drift footer on repository Markdown documentation.

The pull-request template is intentionally excluded because its body is copied
into every newly opened pull request. GitHub Agentic Workflow Markdown sources
under .github/workflows are executable workflow definitions rather than project
documentation and are excluded for the same reason.

SPDX-License-Identifier: GPL-3.0-or-later
"""
from __future__ import annotations

import os
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
HEART = ROOT / "docs" / "assets" / "drift-heart.svg"
START = "<!-- drift-footer:start -->"
END = "<!-- drift-footer:end -->"
EXCLUDED = {ROOT / ".github" / "pull_request_template.md"}


def excluded(path: Path) -> bool:
    if path in EXCLUDED:
        return True
    try:
        relative = path.relative_to(ROOT)
    except ValueError:
        return False
    return len(relative.parts) >= 3 and relative.parts[:2] == (".github", "workflows")


def expected_footer(path: Path) -> str:
    heart = os.path.relpath(HEART, path.parent).replace("\\", "/")
    return (
        f"{START}\n"
        '<p align="center">\n'
        f'  From Munich with <img src="{heart}" alt="an orange-red heart" width="16" height="16">\n'
        "</p>\n"
        f"{END}"
    )


def main() -> int:
    failures: list[str] = []
    files = sorted(p for p in ROOT.rglob("*.md") if not excluded(p))
    for path in files:
        text = path.read_text(encoding="utf-8").rstrip()
        footer = expected_footer(path)
        if not text.endswith(footer):
            failures.append(str(path.relative_to(ROOT)))

    if failures:
        print("markdown-footer: missing or incorrect footer:")
        for item in failures:
            print(f"  - {item}")
        return 1

    if not HEART.is_file():
        print("markdown-footer: shared heart asset is missing")
        return 1

    print(f"markdown-footer: {len(files)} documents passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
