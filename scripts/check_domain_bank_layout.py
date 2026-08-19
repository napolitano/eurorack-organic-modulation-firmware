#!/usr/bin/env python3
"""Validate the bank-oriented portable domain directory contract."""

from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
INCLUDE_DOMAIN = ROOT / "lib/fmd/include/fmd/domain"
SOURCE_DOMAIN = ROOT / "lib/fmd/src/domain"

BANK_MEMBERS = {
    "classic": (
        "PerlinAlgorithm",
        "BrownianAlgorithm",
        "BezierAlgorithm",
        "LfoAlgorithm",
    ),
    "organic": (
        "FractalAlgorithm",
        "VectorAlgorithm",
        "RainAlgorithm",
        "AttractorAlgorithm",
        "OrganicAlgorithmMath",
    ),
    "generative": (
        "TuringAlgorithm",
        "MarkovAlgorithm",
        "MotifAlgorithm",
        "UrnAlgorithm",
        "GenerativeAlgorithmMath",
    ),
    "ambient": (
        "CurrentAlgorithm",
        "AnchorAlgorithm",
        "BreathAlgorithm",
        "FogAlgorithm",
        "AmbientAlgorithmMath",
    ),
    "electronica": (
        "PumpAlgorithm", "AcidAlgorithm", "ShuffleAlgorithm", "PolymeterAlgorithm", "ElectronicaAlgorithmMath",
    ),
    "percussion": (
        "EuclidAlgorithm", "RepeatAlgorithm", "ProbabilityAlgorithm", "HumanizeAlgorithm", "PercussionAlgorithmMath",
    ),
}

TEXT_SUFFIXES = {".h", ".hpp", ".c", ".cc", ".cpp", ".md", ".py", ".yml", ".yaml", ".ini", ".json"}


def fail(message: str) -> None:
    print(f"domain-bank-layout: {message}", file=sys.stderr)
    raise SystemExit(1)


def main() -> None:
    expected_names = {name for names in BANK_MEMBERS.values() for name in names}

    for bank, names in BANK_MEMBERS.items():
        for name in names:
            header = INCLUDE_DOMAIN / bank / f"{name}.h"
            source = SOURCE_DOMAIN / bank / f"{name}.cpp"
            if not header.is_file():
                fail(f"missing bank-owned header: {header.relative_to(ROOT)}")
            if not source.is_file():
                fail(f"missing bank-owned source: {source.relative_to(ROOT)}")
            if (INCLUDE_DOMAIN / f"{name}.h").exists():
                fail(f"bank-owned header leaked back to domain root: {name}.h")
            if (SOURCE_DOMAIN / f"{name}.cpp").exists():
                fail(f"bank-owned source leaked back to domain root: {name}.cpp")

    # DriftEngine is the intended compile-time dispatch boundary and must name
    # each bank explicitly in its public includes.
    engine = (INCLUDE_DOMAIN / "DriftEngine.h").read_text(encoding="utf-8")
    for bank, names in BANK_MEMBERS.items():
        for name in names:
            if name.endswith("AlgorithmMath"):
                continue
            include = f'#include "fmd/domain/{bank}/{name}.h"'
            if include not in engine:
                fail(f"DriftEngine missing bank-qualified include: {include}")

    stale = re.compile(
        r'fmd/domain/(' + '|'.join(re.escape(name) for name in sorted(expected_names)) + r')\.h'
    )
    for path in ROOT.rglob("*"):
        if not path.is_file() or path.suffix.lower() not in TEXT_SUFFIXES:
            continue
        text = path.read_text(encoding="utf-8", errors="ignore")
        match = stale.search(text)
        if match:
            fail(f"stale unqualified bank include in {path.relative_to(ROOT)}: {match.group(0)}")

    print("domain-bank-layout: six bank directories and qualified includes passed")


if __name__ == "__main__":
    main()
