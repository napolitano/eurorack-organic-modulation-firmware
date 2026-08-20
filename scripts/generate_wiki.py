#!/usr/bin/env python3
"""Generate the managed GitHub Wiki mirror from canonical repository Markdown.

The main repository remains the source of truth. This script rewrites local
links/images for GitHub's separate ``.wiki.git`` repository and emits a manifest
that allows the publisher to replace only pages owned by automation.
"""
from __future__ import annotations

import argparse
import json
import os
import re
import shutil
import sys
from pathlib import Path, PurePosixPath

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_REPOSITORY = "napolitano/eurorack-organic-modulation-firmware"
DEFAULT_BRANCH = "main"
MANAGED_MARKER = "<!-- fmd-wiki-managed -->"
MANIFEST = ".fmd-managed-pages"


def slug_words(value: str) -> str:
    words = [part for part in re.split(r"[^A-Za-z0-9]+", value) if part]
    return "-".join(word[:1].upper() + word[1:] for word in words)


def page_map() -> dict[Path, str]:
    mapping: dict[Path, str] = {
        Path("README.md"): "Project-Overview.md",
        Path("README_TESTING.md"): "Testing-and-Verification.md",
        Path("CONTRIBUTING.md"): "Contributing.md",
        Path("docs/installation/README.md"): "Firmware-Installation.md",
        Path("docs/installation/avrdudess/README.md"): "Windows-AVRDUDESS.md",
        Path("docs/development/code-documentation.md"): "Code-Documentation.md",
        Path("docs/development/source-code-reference.md"): "Source-Code-Reference.md",
        Path("docs/development/release-process.md"): "Release-Process.md",
        Path("docs/development/compatibility-plan.md"): "Compatibility-Plan.md",
        Path("docs/analysis/algorithms/README.md"): "Algorithm-Analysis-Index.md",
        Path("docs/analysis/original-firmware-analysis.md"): "Original-Firmware-Analysis.md",
    }

    for path in sorted(ROOT.glob("README-BANK-*.md")):
        bank = path.stem.removeprefix("README-BANK-")
        mapping[path.relative_to(ROOT)] = f"Bank-{slug_words(bank)}.md"

    for path in sorted((ROOT / "docs/analysis/algorithms").glob("*-analysis.md")):
        stem = path.stem.removesuffix("-analysis")
        mapping[path.relative_to(ROOT)] = f"Analysis-{slug_words(stem)}.md"

    for path in sorted((ROOT / "docs/analysis/algorithm-banks").glob("*-bank-design.md")):
        stem = path.stem.removesuffix("-bank-design")
        mapping[path.relative_to(ROOT)] = f"Bank-Design-{slug_words(stem)}.md"

    return mapping


def split_target(target: str) -> tuple[str, str]:
    """Split a Markdown target into path and optional #fragment."""
    if "#" in target:
        path, fragment = target.split("#", 1)
        return path, f"#{fragment}"
    return target, ""


def is_external(target: str) -> bool:
    lowered = target.lower()
    return (
        not target
        or target.startswith("#")
        or lowered.startswith(("http://", "https://", "mailto:", "data:", "tel:"))
    )


def resolve_repo_path(source: Path, target: str) -> Path | None:
    """Resolve one relative documentation target against the canonical repo."""
    path_part, _ = split_target(target)
    if not path_part or is_external(target):
        return None
    if path_part.startswith("/"):
        candidate = PurePosixPath(path_part.lstrip("/"))
    else:
        candidate = PurePosixPath(source.parent.as_posix()) / PurePosixPath(path_part)
    normalized = PurePosixPath(os.path.normpath(candidate.as_posix()))
    if normalized.as_posix().startswith("../"):
        return None
    return Path(normalized.as_posix())


def github_blob_url(repository: str, branch: str, path: Path, fragment: str = "") -> str:
    return f"https://github.com/{repository}/blob/{branch}/{path.as_posix()}{fragment}"


def github_raw_url(repository: str, branch: str, path: Path) -> str:
    return f"https://raw.githubusercontent.com/{repository}/{branch}/{path.as_posix()}"


def rewrite_target(
    source: Path,
    target: str,
    mapping: dict[Path, str],
    repository: str,
    branch: str,
    image: bool,
) -> str:
    target = target.strip()
    if target.startswith("<") and target.endswith(">"):
        target = target[1:-1]
    if is_external(target):
        return target

    path_part, fragment = split_target(target)
    resolved = resolve_repo_path(source, path_part)
    if resolved is None:
        return target

    if resolved in mapping and not image:
        page = mapping[resolved].removesuffix(".md")
        return page + fragment

    absolute = ROOT / resolved
    if image or absolute.suffix.lower() in {".svg", ".png", ".jpg", ".jpeg", ".gif", ".webp"}:
        return github_raw_url(repository, branch, resolved)
    return github_blob_url(repository, branch, resolved, fragment)


MARKDOWN_LINK = re.compile(r"(!?)\[([^\]]*)\]\(([^)]+)\)")
HTML_ATTR = re.compile(r"(?P<prefix>\b(?:src|href)=)(?P<quote>[\"'])(?P<target>[^\"']+)(?P=quote)")


def rewrite_markdown(
    source: Path,
    text: str,
    mapping: dict[Path, str],
    repository: str,
    branch: str,
) -> str:
    def markdown_repl(match: re.Match[str]) -> str:
        bang, label, raw_target = match.groups()
        # Preserve optional Markdown title strings. Repository docs currently use
        # simple targets; splitting on whitespace keeps the generator conservative.
        parts = raw_target.split(maxsplit=1)
        target = parts[0]
        suffix = f" {parts[1]}" if len(parts) == 2 else ""
        rewritten = rewrite_target(
            source, target, mapping, repository, branch, image=(bang == "!")
        )
        return f"{bang}[{label}]({rewritten}{suffix})"

    result = MARKDOWN_LINK.sub(markdown_repl, text)

    def html_repl(match: re.Match[str]) -> str:
        prefix = match.group("prefix")
        quote = match.group("quote")
        target = match.group("target")
        image = prefix.lower().startswith("src=")
        rewritten = rewrite_target(source, target, mapping, repository, branch, image=image)
        return f"{prefix}{quote}{rewritten}{quote}"

    return HTML_ATTR.sub(html_repl, result)


def add_managed_banner(text: str, source: Path, repository: str, branch: str) -> str:
    source_url = github_blob_url(repository, branch, source)
    banner = (
        f"{MANAGED_MARKER}\n"
        f"> **Managed page.** Generated from [`{source.as_posix()}`]({source_url}). "
        "Edit the canonical repository file; direct wiki edits to this page are overwritten.\n\n"
    )
    lines = text.splitlines(keepends=True)
    if lines and lines[0].startswith("# "):
        return lines[0] + "\n" + banner + "".join(lines[1:])
    return banner + text


def display_bank(bank: str) -> str:
    return "Dubstep / Bass" if bank == "dubstep" else bank.title()


def home_page(repository: str, branch: str) -> str:
    sys.path.insert(0, str(ROOT / "scripts"))
    from drift_targets import ALGORITHMS, BANKS  # pylint: disable=import-outside-toplevel

    metadata = json.loads((ROOT / "lib/fmd/library.json").read_text(encoding="utf-8"))
    version = metadata["version"]
    rows = []
    for bank in BANKS:
        algorithms = [name.title() for name, (owner, _) in ALGORITHMS.items() if owner == bank]
        rows.append(f"| [{display_bank(bank)}](Bank-{slug_words(bank)}) | {' · '.join(algorithms)} |")

    return f"""# Free Modular Drift Wiki

{MANAGED_MARKER}
> **Managed wiki.** Canonical documentation lives in the main repository and is mirrored here automatically. Do not maintain duplicate content in generated wiki pages.

Current repository version: **{version}** · **{len(BANKS)} banks** · **{len(ALGORITHMS)} algorithms**

This wiki is the long-form navigation layer for users and developers. The root README remains the quick project overview, while the wiki groups installation, bank guides, verification and engineering analysis into a browsable structure.

## Start here

- [Project overview](Project-Overview)
- [Firmware installation and bank switching](Firmware-Installation)
- [Windows / AVRDUDESS flashing](Windows-AVRDUDESS)
- [Testing and verification](Testing-and-Verification)
- [Source-code reference](Source-Code-Reference)
- [Code-documentation conventions](Code-Documentation)
- [Algorithm analysis index](Algorithm-Analysis-Index)

## Algorithm banks

| Bank | Algorithms |
|---|---|
{chr(10).join(rows)}

## Engineering

- [Original firmware analysis](Original-Firmware-Analysis)
- [Release process](Release-Process)
- [Compatibility plan](Compatibility-Plan)
- [Contributing](Contributing)

Repository: <https://github.com/{repository}>  
Canonical branch mirrored by this wiki: `{branch}`
"""


def sidebar(mapping: dict[Path, str]) -> str:
    def page(source: str) -> str:
        return mapping[Path(source)].removesuffix(".md")

    bank_lines = []
    for source, wiki in mapping.items():
        if source.name.startswith("README-BANK-"):
            label = source.stem.removeprefix("README-BANK-").title()
            if label == "Dubstep":
                label = "Dubstep / Bass"
            bank_lines.append((label, wiki.removesuffix(".md")))
    bank_lines.sort()

    return "\n".join([
        "# Navigation",
        "",
        "- [Home](Home)",
        f"- [Project overview]({page('README.md')})",
        f"- [Installation]({page('docs/installation/README.md')})",
        f"- [Testing]({page('README_TESTING.md')})",
        "",
        "## Banks",
        *[f"- [{label}]({target})" for label, target in bank_lines],
        "",
        "## Development",
        f"- [Source code]({page('docs/development/source-code-reference.md')})",
        f"- [Code documentation]({page('docs/development/code-documentation.md')})",
        f"- [Release process]({page('docs/development/release-process.md')})",
        f"- [Contributing]({page('CONTRIBUTING.md')})",
        "",
        "## Analysis",
        f"- [Algorithm index]({page('docs/analysis/algorithms/README.md')})",
        f"- [Original firmware]({page('docs/analysis/original-firmware-analysis.md')})",
        "",
    ])


def footer(repository: str) -> str:
    return (
        "Generated from the canonical documentation in "
        f"[{repository}](https://github.com/{repository}). "
        "Direct edits to managed pages are overwritten by automation."
    )


def generate(output: Path, repository: str, branch: str) -> list[str]:
    mapping = page_map()
    if output.exists():
        shutil.rmtree(output)
    output.mkdir(parents=True)

    managed: list[str] = []
    for source, page_name in sorted(mapping.items(), key=lambda item: item[1]):
        absolute = ROOT / source
        if not absolute.exists():
            raise FileNotFoundError(f"wiki source does not exist: {source}")
        text = absolute.read_text(encoding="utf-8")
        text = rewrite_markdown(source, text, mapping, repository, branch)
        text = add_managed_banner(text, source, repository, branch)
        (output / page_name).write_text(text, encoding="utf-8")
        managed.append(page_name)

    extras = {
        "Home.md": home_page(repository, branch),
        "_Sidebar.md": sidebar(mapping),
        "_Footer.md": footer(repository),
    }
    for name, text in extras.items():
        (output / name).write_text(text.rstrip() + "\n", encoding="utf-8")
        managed.append(name)

    managed = sorted(set(managed))
    (output / MANIFEST).write_text("\n".join(managed) + "\n", encoding="utf-8")
    return managed


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--repository", default=os.environ.get("GITHUB_REPOSITORY", DEFAULT_REPOSITORY))
    parser.add_argument("--branch", default=DEFAULT_BRANCH)
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    try:
        managed = generate(args.output, args.repository, args.branch)
    except (OSError, ValueError) as exc:
        print(f"wiki-generation error: {exc}", file=sys.stderr)
        return 2
    print(f"wiki generation: {len(managed)} managed Markdown pages written to {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
