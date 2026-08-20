"""PlatformIO pre-script: optionally lock an AVR build to one named algorithm."""
Import("env")  # type: ignore[name-defined]  # PlatformIO/SCons injects this helper.

import os
import sys
from pathlib import Path

project_dir = Path(env.subst("$PROJECT_DIR"))
sys.path.insert(0, str(project_dir / "scripts"))

from drift_targets import algorithm_bank, algorithm_id, bank_from_platformio_environment, canonical_algorithm

requested = os.environ.get("FMD_FORCE_ALGORITHM", "").strip()
if requested:
    name = canonical_algorithm(requested)
    expected_bank = algorithm_bank(name)
    actual_bank = bank_from_platformio_environment(env.subst("$PIOENV"))
    if actual_bank != expected_bank:
        raise RuntimeError(
            f"FMD_FORCE_ALGORITHM={name} belongs to bank '{expected_bank}', "
            f"but PlatformIO environment '{env.subst('$PIOENV')}' builds bank '{actual_bank}'"
        )

    identifier = algorithm_id(name)
    env.Append(CPPDEFINES=[("FMD_FORCED_ALGORITHM", identifier)])
    print(
        f"Drift developer target: forced algorithm '{name}' "
        f"(bank={expected_bank}, id={identifier}); rear DIP switches ignored"
    )
