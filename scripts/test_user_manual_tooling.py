#!/usr/bin/env python3
"""Unit checks for Drift manual publication naming/version logic.

SPDX-License-Identifier: GPL-3.0-or-later
"""
from build_user_manual import artifact_name, normalize_version
from check_user_manual import parse_pdffonts_rows, validate_fonts


def test_pdffonts_parser() -> None:
    spaced = """name                                 type              encoding         emb sub uni object ID
------------------------------------ ----------------- ---------------- --- --- --- ---------
AAAAAA+Ubuntu                        TrueType          WinAnsi          yes yes yes     10  0
BBBBBB+Ubuntu Light                  TrueType          WinAnsi          yes yes yes     11  0
"""
    rows = parse_pdffonts_rows(spaced)
    assert [row["name"] for row in rows] == ["AAAAAA+Ubuntu", "BBBBBB+Ubuntu Light"]
    validate_fonts(spaced, False)

    hyphenated = """name                                 type              encoding         emb sub uni object ID
------------------------------------ ----------------- ---------------- --- --- --- ---------
AAAAAA+Ubuntu                        TrueType          WinAnsi          yes yes yes     10  0
BBBBBB+Ubuntu-Light                  TrueType          WinAnsi          yes yes yes     11  0
"""
    validate_fonts(hyphenated, False)

    libreoffice_noble = """name                                 type              encoding         emb sub uni object ID
------------------------------------ ----------------- ---------------- --- --- --- ---------
BAAAAA+Ubuntu                        TrueType          WinAnsi          yes yes yes   2101  0
CAAAAA+Ubuntu-Bold                   TrueType          WinAnsi          yes yes yes   2086  0
DAAAAA+Ubuntu-Medium                 TrueType          WinAnsi          yes yes yes   2091  0
EAAAAA+DejaVuSans                    TrueType          WinAnsi          yes yes yes   2096  0
"""
    # LibreOffice on Ubuntu/Noble may report source-requested Ubuntu Light as
    # Ubuntu-Medium because of the classic Ubuntu font metadata. This alias is
    # accepted only when the caller has verified that the ODT requests Light.
    validate_fonts(libreoffice_noble, False, allow_ubuntu_medium_for_light=True)
    try:
        validate_fonts(libreoffice_noble, False, allow_ubuntu_medium_for_light=False)
    except ValueError:
        pass
    else:
        raise AssertionError("expected Ubuntu-Medium alias to require source proof of Ubuntu Light")

    substituted = """name                                 type              encoding         emb sub uni object ID
------------------------------------ ----------------- ---------------- --- --- --- ---------
AAAAAA+Ubuntu                        TrueType          WinAnsi          yes yes yes     10  0
BBBBBB+LiberationSans                TrueType          WinAnsi          yes yes yes     11  0
"""
    try:
        validate_fonts(substituted, False)
    except ValueError:
        pass
    else:
        raise AssertionError("expected missing Ubuntu Light to fail")


def main() -> int:
    assert normalize_version("0.1.0") == "0.1.0"
    assert normalize_version("v0.1.0") == "0.1.0"
    assert normalize_version("1.2.3-rc.1") == "1.2.3-rc.1"
    assert artifact_name("v0.1.0") == "drift-user-manual.0.1.0.pdf"
    test_pdffonts_parser()

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
