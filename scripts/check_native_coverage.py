#!/usr/bin/env python3
"""Enforce aggregate and per-file native coverage policy."""

from __future__ import annotations

import argparse
import json
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

POLICY_PATH = Path("scripts/native_coverage_policy.json")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("coverage_xml", type=Path)
    parser.add_argument("--scope", required=True)
    return parser.parse_args()


def percentage(rate: str) -> float:
    return float(rate) * 100.0


def class_has_branches(class_element: ET.Element) -> bool:
    return any(
        line.attrib.get("branch", "false").lower() == "true"
        for line in class_element.findall("./lines/line")
    )


def main() -> int:
    args = parse_args()
    policy = json.loads(POLICY_PATH.read_text(encoding="utf-8"))
    scopes = policy.get("scopes", {})
    if args.scope not in scopes:
        print(f"coverage-policy error: unknown scope {args.scope!r}", file=sys.stderr)
        return 2

    selected = scopes[args.scope]
    root = ET.parse(args.coverage_xml).getroot()
    line = percentage(root.attrib["line-rate"])
    branch = percentage(root.attrib["branch-rate"])
    print(
        f"coverage[{args.scope}]: lines={line:.2f}% branches={branch:.2f}% "
        f"(required {selected['line_min_percent']:.2f}%/{selected['branch_min_percent']:.2f}%)"
    )

    failures: list[str] = []
    if line < selected["line_min_percent"]:
        failures.append(
            f"aggregate line coverage {line:.2f}% < {selected['line_min_percent']:.2f}%"
        )
    if branch < selected["branch_min_percent"]:
        failures.append(
            f"aggregate branch coverage {branch:.2f}% < {selected['branch_min_percent']:.2f}%"
        )

    classes = root.findall("./packages/package/classes/class")
    prefix = selected.get("path_prefix")
    if prefix:
        unexpected = sorted(
            c.attrib.get("filename", "")
            for c in classes
            if not c.attrib.get("filename", "").startswith(prefix)
        )
        if unexpected:
            failures.append(
                "scope contains non-bank sources: " + ", ".join(unexpected)
            )

    allowed_prefixes = selected.get("allowed_path_prefixes")
    if allowed_prefixes:
        unexpected = sorted(
            c.attrib.get("filename", "")
            for c in classes
            if not any(
                c.attrib.get("filename", "").startswith(allowed)
                for allowed in allowed_prefixes
            )
        )
        if unexpected:
            failures.append(
                "scope contains non-allowlisted sources: " + ", ".join(unexpected)
            )

    for class_element in classes:
        filename = class_element.attrib.get("filename", "<unknown>")
        file_line = percentage(class_element.attrib.get("line-rate", "0"))
        if file_line < selected["file_line_min_percent"]:
            failures.append(
                f"{filename}: line coverage {file_line:.2f}% < "
                f"{selected['file_line_min_percent']:.2f}%"
            )
        if class_has_branches(class_element):
            file_branch = percentage(class_element.attrib.get("branch-rate", "0"))
            if file_branch < selected["file_branch_min_percent"]:
                failures.append(
                    f"{filename}: branch coverage {file_branch:.2f}% < "
                    f"{selected['file_branch_min_percent']:.2f}%"
                )

    if failures:
        for failure in failures:
            print(f"coverage-policy error: {failure}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
