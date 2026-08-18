# Release process

Releases are maintainer-controlled and tag-driven. Development work has no public version until release preparation.

## 1. Development state

All release-relevant work is recorded under:

```markdown
## Unreleased
```

Do not invent a versioned changelog heading during ordinary development. The `0.0.0` value in `lib/fmd/library.json` is only a local PlatformIO package-manifest placeholder required for library metadata; it is not a firmware release version.

## 2. Prepare a release

When a release is explicitly approved:

1. choose the release version;
2. set `lib/fmd/library.json` from its development placeholder to `X.Y.Z`;
3. create a new changelog section `## X.Y.Z — YYYY-MM-DD`;
4. move the relevant `Unreleased` entries into that section;
5. start the section with `### Release summary`;
6. write one concise prose paragraph under that heading (maximum seven non-empty source lines);
7. leave a fresh `## Unreleased` section above the release history;
8. validate the maintained user-manual source and publication tooling;
9. run all tests, coverage, both AVR builds, resource-budget checks and timing-probe build;

The AVR resource-budget check reserves deliberate headroom: release builds must remain at or below **85% of the 30,720-byte application flash budget (26,112 bytes)** and **65% of the 2,048-byte static SRAM budget (1,331 bytes)**. PlatformIO still enforces the absolute MCU limits; these repository gates fail earlier by design.
10. commit the prepared release state;
11. create and push tag `vX.Y.Z`.

## 3. Automated release notes

`scripts/generate_release_notes.py` derives the public GitHub Release text deterministically from the matching changelog section. It emits:

1. the `Release summary` paragraph;
2. the remaining detailed changelog excerpt;
3. artifact-integrity/checksum information;
4. a compare link to the previous tag, or a commit-history link for the first release.

The release assets include a PDF generated from the maintained `docs/manual/drift-user-manual.odt` source. The repository source remains unversioned; tag `vX.Y.Z` produces `drift-user-manual.X.Y.Z.pdf`.

The workflow intentionally does not use generic GitHub auto-generated notes because the changelog is the reviewed release narrative.

## 4. Release workflow

`.github/workflows/release.yml` validates traceability and release-note generation, runs native tests/sanitizers/coverage, builds both Nano bootloader targets, checks AVR flash/SRAM engineering budgets, compiles the timing-probe image, installs the publication toolchain and Ubuntu fonts, builds and validates the versioned user-manual PDF, creates firmware/provenance artifacts, writes SHA-256 and MD5 manifests, generates the changelog-based notes and publishes the GitHub Release for the pushed tag.

The manual check is strict for release builds: the PDF must have the expected page geometry and contain embedded Ubuntu and Ubuntu Light fonts. Font substitution is not accepted for a tagged release.

A manual `workflow_dispatch` performs the build/validation artifact path but does not assign a version or publish a GitHub Release.

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
