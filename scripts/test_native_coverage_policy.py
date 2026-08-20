#!/usr/bin/env python3
"""Regression tests for scoped native coverage enforcement."""

from __future__ import annotations

import subprocess
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CHECKER = ROOT / "scripts" / "check_native_coverage.py"


def write_report(
    path: Path,
    *,
    filename: str,
    line: float,
    branch: float,
    aggregate_line: float | None = None,
    aggregate_branch: float | None = None,
) -> None:
    root_line = line if aggregate_line is None else aggregate_line
    root_branch = branch if aggregate_branch is None else aggregate_branch
    path.write_text(
        f'''<?xml version="1.0" ?>
<coverage line-rate="{root_line}" branch-rate="{root_branch}">
  <packages><package name="bank"><classes>
    <class name="Algorithm" filename="{filename}" line-rate="{line}" branch-rate="{branch}">
      <lines><line number="1" hits="1" branch="true"/></lines>
    </class>
  </classes></package></packages>
</coverage>\n''',
        encoding="utf-8",
    )


def run(report: Path, scope: str) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        ["python", str(CHECKER), str(report), "--scope", scope],
        cwd=ROOT,
        text=True,
        capture_output=True,
        check=False,
    )


def main() -> int:
    with tempfile.TemporaryDirectory() as tmp:
        report = Path(tmp) / "coverage.xml"

        write_report(
            report,
            filename="lib/fmd/src/domain/ambient/AmbientAlgorithmMath.cpp",
            line=0.97,
            branch=0.90,
        )
        assert run(report, "ambient").returncode == 0

        write_report(
            report,
            filename="lib/fmd/src/domain/ambient/AmbientAlgorithmMath.cpp",
            line=0.94,
            branch=0.90,
        )
        assert run(report, "ambient").returncode == 1

        write_report(
            report,
            filename="lib/fmd/src/domain/ambient/AmbientAlgorithmMath.cpp",
            line=0.97,
            branch=0.84,
        )
        assert run(report, "ambient").returncode == 1

        # Aggregate coverage may be excellent while one production file is weak.
        write_report(
            report,
            filename="lib/fmd/src/domain/ambient/FogAlgorithm.cpp",
            line=0.94,
            branch=0.79,
            aggregate_line=0.99,
            aggregate_branch=0.99,
        )
        per_file = run(report, "ambient")
        assert per_file.returncode == 1
        assert "FogAlgorithm.cpp: line coverage" in per_file.stderr
        assert "FogAlgorithm.cpp: branch coverage" in per_file.stderr

        write_report(
            report,
            filename="lib/fmd/src/domain/FrequencyMapping.cpp",
            line=0.99,
            branch=0.99,
        )
        result = run(report, "ambient")
        assert result.returncode == 1
        assert "non-bank sources" in result.stderr

        unknown = run(report, "unknown")
        assert unknown.returncode == 2

    print("native coverage policy tests: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
