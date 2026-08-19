# Contributing

Contributions are welcome when they preserve the musical identity, mathematical contracts, hardware constraints and maintainability goals of the project.

> [!IMPORTANT]
> Firmware work targets the supported Free Modular Drift Arduino Nano / ATmega328P platform. Hardware-revision work should be kept explicit and must not be smuggled into a firmware-only change.

Read [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md) before participating. Suspected security vulnerabilities must follow [.github/SECURITY.md](.github/SECURITY.md), not a public issue.

## Before opening an issue

Use the repository issue forms where possible. Bug reports should identify the commit/release, affected algorithm or subsystem, exact reproduction steps, expected result, observed result, and relevant vectors/build output/measurements. For algorithmic defects, distinguish mathematical expectation from historical upstream behavior.

Feature requests should describe the user-facing goal first. New behavior must fit the ATmega328P real-time and resource envelope unless the proposal explicitly belongs to a later hardware revision.

## Development environment

The project uses PlatformIO and C++17. Primary environments are:

- `nanoatmega328new` — Nano/ATmega328P, newer bootloader;
- `nanoatmega328` — Nano/ATmega328P, legacy bootloader;
- `native` — Unity host tests;
- `native_coverage` — host coverage;
- `native_sanitized` — ASan/UBSan;
- `nanoatmega328new_timing` — Classic timing-qualification image;
- `nanoatmega328new_organic` / `nanoatmega328_organic` — Organic-bank Nano images;
- `native_organic`, `native_organic_coverage`, `native_organic_sanitized` — Organic host verification;
- `nanoatmega328new_organic_timing` — Organic timing-qualification image.

Normal verification:

```sh
pio test -e native
pio test -e native_sanitized
pio run -e nanoatmega328new
pio run -e nanoatmega328
pio test -e native_organic
pio test -e native_organic_sanitized
pio run -e nanoatmega328new_organic
pio run -e nanoatmega328_organic
python scripts/check_requirement_traceability.py
python scripts/check_markdown_footer.py
python scripts/check_avr_resource_budget.py .pio/build/nanoatmega328new/firmware.elf
python scripts/check_avr_resource_budget.py .pio/build/nanoatmega328/firmware.elf
python scripts/check_avr_resource_budget.py .pio/build/nanoatmega328new_organic/firmware.elf
python scripts/check_avr_resource_budget.py .pio/build/nanoatmega328_organic/firmware.elf
```

The resource guards intentionally stop growth before the MCU is full: application flash must remain **<= 26,112 bytes (85% of the 30,720-byte budget)** and static SRAM **<= 1,331 bytes (65% of 2,048 bytes)**. Do not relax these limits merely to merge a feature.

Coverage:

```sh
pio test -e native_coverage
pio test -e native_organic_coverage
mkdir -p coverage
gcovr --root . --filter lib/fmd/src --exclude test --xml-pretty --output coverage/coverage.xml
python scripts/check_native_coverage.py coverage/coverage.xml
```

The current portable-code floor is 95% lines / 75% branches. Do not lower it merely to make a change pass.

## Coding requirements

- Use C++17 as configured by PlatformIO.
- Keep Arduino/AVR APIs out of `lib/fmd`; hardware access belongs under `src/platform/nano_atmega328p/`.
- Keep bank-owned algorithm classes and bank-specific math under `lib/fmd/include/fmd/domain/<bank>/` and `lib/fmd/src/domain/<bank>/`; only genuinely cross-bank domain support belongs directly under `domain/`.
- Keep `src/main.cpp` a thin composition entry point.
- Avoid dynamic allocation in the real-time firmware path.
- Keep code explicit enough that state transitions, fixed-point widths and timing costs remain reviewable.
- Keep host warnings clean under `-Wall -Wextra -Werror -Wconversion -Wsign-conversion -Wshadow -Wpedantic`.
- Add or update tests for every behavioral or numerical change.
- Do not weaken mathematical assertions to accommodate an implementation defect; fix the implementation or justify the contract.
- Performance optimizations must retain a tested numerical contract or document the accepted error bound.

## Algorithm changes

Each algorithm has a developer analysis under `docs/analysis/algorithms/`. Changes to an algorithm should update the corresponding document when they alter mathematics, control mapping, statistical behavior, computational cost or verification assumptions.

Correctness tests should prefer independent equations, exhaustive integer domains where practical, and strong invariants. Regression tests should make intentional differences from Quinn Freedman's upstream implementation explicit rather than silently changing behavior.

## Documentation style

Documentation targets GitHub rendering first. Use clear heading hierarchies, relative repository links, fenced code blocks, tables for compact comparisons, Mermaid where architecture/state flow benefits, and GitHub math notation for equations. Use alerts sparingly for materially important information. Long documents should include a focused contents index.

Repository Markdown documentation ends with the shared `From Munich with` Drift footer. Run `python scripts/check_markdown_footer.py` after adding or moving Markdown files; the pull-request template is intentionally excluded because it is copied into PR bodies.

Purely editorial wording/formatting changes do not need changelog entries unless they form part of a substantial documentation release.

## Pull requests

Keep each pull request focused. Before submission:

1. run the native test suites and sanitizers for all six compile-time banks (Classic, Organic, Generative, Ambient, Electronica and Percussion);
2. build both Nano bootloader environments for every compile-time bank;
3. update algorithm/reference tests for changed behavior;
4. update requirement traceability;
5. update technical documentation and `CHANGELOG.md` when release-relevant;
6. verify that build artifacts, coverage files, Python caches, editor files and local paths are not committed.

CI is expected to pass before merge. Use GitHub closing keywords only when merging the pull request should actually close the referenced issue.

## Changelog policy

`CHANGELOG.md` records release-relevant changes. New work belongs under `Unreleased`; no versioned section is created until release preparation. Do not rewrite historical release sections after publication.

A versioned release section must begin with `### Release summary` followed by one concise prose paragraph. The release workflow uses that paragraph verbatim as the opening of generated GitHub Release notes.

## Releases

Release preparation is maintainer-controlled. Version assignment, the changelog release section, tag, validation, firmware artifacts, checksums, build provenance and generated release notes are handled by the repository release workflow. Do not create or move release tags in an ordinary contribution.

## Licensing

By contributing, you agree that your contribution may be distributed under the licenses applicable to the files/components you modify. Check the relevant license before adding third-party material.

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="docs/assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
