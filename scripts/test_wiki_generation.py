#!/usr/bin/env python3
"""Regression tests for deterministic GitHub Wiki generation."""
from __future__ import annotations

import hashlib
import tempfile
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))

from generate_wiki import MANAGED_MARKER, MANIFEST, generate, page_map  # noqa: E402


def digest_tree(root: Path) -> dict[str, str]:
    return {
        path.relative_to(root).as_posix(): hashlib.sha256(path.read_bytes()).hexdigest()
        for path in sorted(root.rglob("*"))
        if path.is_file()
    }


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def main() -> int:
    repository = "napolitano/eurorack-organic-modulation-firmware"
    with tempfile.TemporaryDirectory() as tmp:
        first = Path(tmp) / "first"
        second = Path(tmp) / "second"
        managed = generate(first, repository, "main")
        generate(second, repository, "main")

        require(digest_tree(first) == digest_tree(second), "wiki output is not deterministic")
        mapping = page_map()
        require(len(list((ROOT / "docs/analysis/algorithms").glob("*-analysis.md"))) == 28,
                "expected 28 canonical algorithm analyses")
        require(len([p for p in mapping if p.name.startswith("README-BANK-")]) == 7,
                "expected seven generated bank pages")

        expected = {path.name for path in first.glob("*.md")}
        require(set(managed) == expected, "managed manifest does not match generated Markdown pages")
        manifest = set((first / MANIFEST).read_text().splitlines())
        require(manifest == expected, "written managed-page manifest is incomplete")

        home = (first / "Home.md").read_text()
        require("7 banks" in home and "28 algorithms" in home, "Home page platform counts are stale")
        require("Bank-Dubstep" in home and "Wobble" in home, "Dubstep/Bass is missing from Home page")

        source_page = (first / "Source-Code-Reference.md").read_text()
        require(MANAGED_MARKER in source_page, "generated source reference lacks managed marker")
        require("raw.githubusercontent.com" not in source_page or "drift-heart.svg" in source_page,
                "unexpected raw URL rewrite")

        overview = (first / "Project-Overview.md").read_text()
        require("raw.githubusercontent.com" in overview, "README images were not rewritten for wiki repository")
        require("Firmware-Installation" in overview, "mapped README link was not rewritten to wiki page")

        for path in first.glob("*.md"):
            text = path.read_text()
            require('src="docs/' not in text and 'src="../' not in text,
                    f"{path.name} retains a repository-relative HTML image")

        workflow = (ROOT / ".github/workflows/wiki.yml").read_text(encoding="utf-8")
        for marker in (
            "workflow_dispatch:",
            "contents: write",
            ".wiki.git",
            "WIKI_PUSH_TOKEN",
            ".fmd-managed-pages",
            "scripts/test_wiki_generation.py",
            "scripts/generate_wiki.py --output .wiki-build",
        ):
            require(marker in workflow, f"wiki workflow lost required contract marker: {marker}")
        require("rm -rf .wiki-repo" not in workflow,
                "wiki publisher must not replace the entire wiki and destroy manual pages")

    print("wiki generation contract: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
