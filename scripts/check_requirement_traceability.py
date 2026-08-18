#!/usr/bin/env python3
from __future__ import annotations

import json
import re
import sys
from pathlib import Path

RUN_TEST_RE = re.compile(r"RUN_TEST\(\s*([A-Za-z_][A-Za-z0-9_]*)\s*\)")
AC_ID_RE = re.compile(r"^AC-(\d{2})$")


def main() -> int:
    data = json.loads(Path("test/requirements-traceability.json").read_text(encoding="utf-8"))
    entries = data["acceptance_criteria"]
    failures: list[str] = []

    numbers: list[int] = []
    seen_ids: set[str] = set()
    for entry in entries:
        criterion_id = entry.get("id", "")
        match = AC_ID_RE.match(criterion_id)
        if not match:
            failures.append(f"invalid acceptance-criterion ID: {criterion_id!r}")
            continue
        if criterion_id in seen_ids:
            failures.append(f"duplicate acceptance-criterion ID: {criterion_id}")
        seen_ids.add(criterion_id)
        numbers.append(int(match.group(1)))

    expected = list(range(1, len(entries) + 1))
    if sorted(numbers) != expected:
        failures.append(f"acceptance-criterion numbering must be consecutive AC-01..AC-{len(entries):02d}")

    for entry in entries:
        criterion_id = entry.get("id", "<unknown>")
        tests = entry.get("tests", [])
        if not tests:
            failures.append(f"{criterion_id}: no tests")
            continue
        for ref in tests:
            path = Path(ref["file"])
            if not path.is_file():
                failures.append(f"{criterion_id}: missing {path}")
                continue
            cases = set(RUN_TEST_RE.findall(path.read_text(encoding="utf-8")))
            if ref["case"] not in cases:
                failures.append(f"{criterion_id}: case not RUN_TEST'd: {ref['case']}")

    if failures:
        for failure in failures:
            print(f"requirement-traceability: {failure}", file=sys.stderr)
        return 1

    print(f"requirement-traceability: {len(entries)} acceptance criteria: passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
