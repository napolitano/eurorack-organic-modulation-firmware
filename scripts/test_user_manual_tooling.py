#!/usr/bin/env python3
"""Unit checks for Drift manual publication naming/version logic.

SPDX-License-Identifier: GPL-3.0-or-later
"""
from build_user_manual import artifact_name, normalize_version


def main() -> int:
    assert normalize_version("0.1.0") == "0.1.0"
    assert normalize_version("v0.1.0") == "0.1.0"
    assert normalize_version("1.2.3-rc.1") == "1.2.3-rc.1"
    assert artifact_name("v0.1.0") == "drift-user-manual.0.1.0.pdf"

    for invalid in ("", "0.1", "release-0.1.0", "v1", "1.2.3+build"):
        try:
            normalize_version(invalid)
        except ValueError:
            pass
        else:
            raise AssertionError(f"expected invalid version to fail: {invalid!r}")

    print("manual tooling tests: passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
