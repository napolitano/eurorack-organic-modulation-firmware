#!/usr/bin/env python3
"""Regression tests for named Drift bank/algorithm developer targets."""
from __future__ import annotations

import argparse
import os
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SCRIPTS = ROOT / "scripts"
sys.path.insert(0, str(SCRIPTS))

from drift_targets import ALGORITHMS, BANKS, algorithm_bank, algorithm_id, canonical_algorithm, platformio_environment
from flash_drift import build_command


def assert_mapping() -> None:
    assert len(BANKS) == 6
    assert len(ALGORITHMS) == 24
    assert sorted(identifier for _, identifier in ALGORITHMS.values()) == list(range(24))
    for name, (bank, identifier) in ALGORITHMS.items():
        assert canonical_algorithm(name.upper()) == name
        assert algorithm_bank(name) == bank
        assert algorithm_id(name) == identifier
    assert canonical_algorithm("Bézier") == "bezier"


def assert_environment_names() -> None:
    assert platformio_environment("classic", "new") == "nanoatmega328new"
    assert platformio_environment("classic", "old") == "nanoatmega328"
    assert platformio_environment("ambient", "new") == "nanoatmega328new_ambient"
    assert platformio_environment("percussion", "old") == "nanoatmega328_percussion"


def args(mode: str, name: str, bootloader: str = "new") -> argparse.Namespace:
    return argparse.Namespace(mode=mode, name=name, bootloader=bootloader, port=None, build_only=False, dry_run=True)


def assert_wrapper_commands() -> None:
    command, child_env, bank, algorithm = build_command(args("bank", "ambient"))
    assert command == ["pio", "run", "-e", "nanoatmega328new_ambient", "-t", "upload"]
    assert bank == "ambient" and algorithm is None
    assert "FMD_FORCE_ALGORITHM" not in child_env

    command, child_env, bank, algorithm = build_command(args("algorithm", "breath", "old"))
    assert command == ["pio", "run", "-e", "nanoatmega328_ambient", "-t", "upload"]
    assert bank == "ambient" and algorithm == "breath"
    assert child_env["FMD_FORCE_ALGORITHM"] == "breath"


def compile_config(bank: int, algorithm: int, should_succeed: bool) -> None:
    source = '#include "fmd/config/AlgorithmTargetConfig.h"\nint main(){return 0;}\n'
    with tempfile.TemporaryDirectory() as directory:
        path = Path(directory) / "probe.cpp"
        path.write_text(source, encoding="utf-8")
        command = [
            "g++", "-std=c++17", "-I", str(ROOT / "lib/fmd/include"),
            f"-DFMD_ALGORITHM_BANK={bank}", f"-DFMD_FORCED_ALGORITHM={algorithm}",
            "-c", str(path), "-o", str(Path(directory) / "probe.o")
        ]
        result = subprocess.run(command, capture_output=True, text=True)
        if should_succeed:
            assert result.returncode == 0, result.stderr
        else:
            assert result.returncode != 0
            assert "Forced algorithm does not belong" in result.stderr


def assert_compile_guards() -> None:
    for algorithm in range(24):
        compile_config(algorithm // 4, algorithm, True)
    compile_config(0, 14, False)
    compile_config(5, 0, False)



def assert_forced_engine_references_only_named_algorithm() -> None:
    cases = (
        (0, 0, "Perlin", ("Brownian", "Bezier", "Lfo")),
        (1, 4, "Fractal", ("Vector", "Rain", "Attractor")),
        (2, 9, "Markov", ("Turing", "Motif", "Urn")),
        (3, 14, "Breath", ("Current", "Anchor", "Fog")),
        (4, 19, "Polymeter", ("Pump", "Acid", "Shuffle")),
        (5, 20, "Euclid", ("Repeat", "Probability", "Humanize")),
    )
    with tempfile.TemporaryDirectory() as directory:
        for bank, algorithm, expected, absent_names in cases:
            obj = Path(directory) / f"engine-{bank}.o"
            command = [
                "g++", "-std=c++17", "-O2", "-ffunction-sections", "-fdata-sections",
                "-I", str(ROOT / "lib/fmd/include"),
                f"-DFMD_ALGORITHM_BANK={bank}", f"-DFMD_FORCED_ALGORITHM={algorithm}",
                "-c", str(ROOT / "lib/fmd/src/domain/DriftEngine.cpp"), "-o", str(obj),
            ]
            result = subprocess.run(command, capture_output=True, text=True)
            assert result.returncode == 0, result.stderr
            symbols = subprocess.run(["nm", "-C", "-u", str(obj)], capture_output=True, text=True, check=True).stdout
            assert f"fmd::{expected}Algorithm" in symbols, symbols
            for absent in absent_names:
                assert f"fmd::{absent}Algorithm" not in symbols, symbols


def main() -> int:
    assert_mapping()
    assert_environment_names()
    assert_wrapper_commands()
    assert_compile_guards()
    assert_forced_engine_references_only_named_algorithm()
    print("firmware target tooling: PASS (6 banks, 24 named algorithms, single-target linkage, guardrails)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
