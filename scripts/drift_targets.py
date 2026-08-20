#!/usr/bin/env python3
"""Shared named firmware-target mapping for Drift developer tooling."""
from __future__ import annotations

import unicodedata

BANKS = ("classic", "organic", "generative", "ambient", "electronica", "percussion")

ALGORITHMS = {
    "perlin": ("classic", 0),
    "brownian": ("classic", 1),
    "bezier": ("classic", 2),
    "lfo": ("classic", 3),
    "fractal": ("organic", 4),
    "vector": ("organic", 5),
    "rain": ("organic", 6),
    "attractor": ("organic", 7),
    "turing": ("generative", 8),
    "markov": ("generative", 9),
    "motif": ("generative", 10),
    "urn": ("generative", 11),
    "current": ("ambient", 12),
    "anchor": ("ambient", 13),
    "breath": ("ambient", 14),
    "fog": ("ambient", 15),
    "pump": ("electronica", 16),
    "acid": ("electronica", 17),
    "shuffle": ("electronica", 18),
    "polymeter": ("electronica", 19),
    "euclid": ("percussion", 20),
    "repeat": ("percussion", 21),
    "probability": ("percussion", 22),
    "humanize": ("percussion", 23),
}


def normalize_name(value: str) -> str:
    """Return a lowercase ASCII target name suitable for CLI/environment use."""
    normalized = unicodedata.normalize("NFKD", value.strip()).encode("ascii", "ignore").decode("ascii")
    return normalized.lower().replace("_", "-")


def canonical_bank(value: str) -> str:
    name = normalize_name(value)
    if name not in BANKS:
        raise ValueError(f"unknown bank '{value}'; choose one of: {', '.join(BANKS)}")
    return name


def canonical_algorithm(value: str) -> str:
    name = normalize_name(value)
    if name not in ALGORITHMS:
        raise ValueError(f"unknown algorithm '{value}'; choose one of: {', '.join(ALGORITHMS)}")
    return name


def algorithm_bank(value: str) -> str:
    return ALGORITHMS[canonical_algorithm(value)][0]


def algorithm_id(value: str) -> int:
    return ALGORITHMS[canonical_algorithm(value)][1]


def platformio_environment(bank: str, bootloader: str = "new") -> str:
    bank = canonical_bank(bank)
    if bootloader not in ("new", "old"):
        raise ValueError("bootloader must be 'new' or 'old'")
    base = "nanoatmega328new" if bootloader == "new" else "nanoatmega328"
    return base if bank == "classic" else f"{base}_{bank}"


def bank_from_platformio_environment(environment: str) -> str:
    env = normalize_name(environment)
    for bank in BANKS[1:]:
        if f"-{bank}" in env or f"_{bank}" in environment.lower():
            return bank
    return "classic"
