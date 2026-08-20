# Release process

Releases are maintainer-controlled and tag-driven. Development work has no public version until release preparation.

## 1. Development state

All release-relevant work is recorded under:

```markdown
## Unreleased
```

Do not invent a versioned changelog heading during ordinary development. During explicit release preparation, `lib/fmd/library.json` is set to the release version and a matching versioned changelog section is created. Release `0.3.0` is the current seven-bank release target; `0.2.0` remains the six-bank compatibility baseline and `0.1.0` the Classic-only baseline. New work after 0.3.0 returns to `Unreleased` until the next release is explicitly prepared.

## 2. Prepare a release

When a release is explicitly approved:

1. choose the release version;
2. set `lib/fmd/library.json` to `X.Y.Z`;
3. create a new changelog section `## X.Y.Z — YYYY-MM-DD`;
4. move the relevant `Unreleased` entries into that section;
5. start the section with `### Release summary`;
6. write one concise prose paragraph under that heading (maximum seven non-empty source lines);
7. leave a fresh `## Unreleased` section above the release history;
8. validate the maintained user-manual source and publication tooling;
9. freeze the final manual source with `python scripts/freeze_user_manual.py --version X.Y.Z`, then verify it with `--check`;
10. run the complete native regression suite once, then bank-filtered wiring tests plus targeted coverage/sanitizer qualification for every bank present in the tagged source, followed by both bootloader builds, resource-budget checks and the corresponding timing-probe builds;
11. commit the prepared release state, including `docs/manual/releases/X.Y.Z/drift-user-manual.X.Y.Z.odt`;
12. create and push the version tag `vX.Y.Z`.

The AVR resource-budget check reserves deliberate headroom: release builds must remain at or below **85% of the 30,720-byte application flash budget (26,112 bytes)** and **65% of the 2,048-byte static SRAM budget (1,331 bytes)**. PlatformIO still enforces the absolute MCU limits; these repository gates fail earlier by design.

For any prepared release, the final operations use the chosen version, for example:

```bash
git tag -a vX.Y.Z -m "Release X.Y.Z"
git push origin main vX.Y.Z
```

Do not create the tag until the prepared release commit is the exact commit intended for publication.

## 3. Automated release notes

`scripts/generate_release_notes.py` derives the public GitHub Release text deterministically from the matching changelog section. It emits:

1. the `Release summary` paragraph;
2. the remaining detailed changelog excerpt;
3. artifact-integrity/checksum information;
4. a compare link to the previous tag, or a commit-history link for the first release.

The maintained editing source remains `docs/manual/drift-user-manual.odt` while development is unreleased. Release preparation freezes the final bytes into `docs/manual/releases/X.Y.Z/drift-user-manual.X.Y.Z.odt`; that immutable snapshot is committed before tagging. The tag-driven workflow publishes the frozen ODT and generates `drift-user-manual.X.Y.Z.pdf` from it. Normal pushes and pull requests do not generate publication PDFs or release snapshots.

Release packaging follows the capabilities of the tagged source. Release `0.3.0` packages **fourteen independently flashable bank/bootloader variants**, yielding **28 firmware files** because its seven banks are each published for both Nano bootloaders as HEX and ELF, accompanied by **fourteen BUILD-INFO provenance files**. Release `0.2.0` remains the historical six-bank package with twelve bank/bootloader variants, 24 firmware files and twelve BUILD-INFO files. Historical Classic-only tags package the two Classic bootloader variants only; intermediate tags package only the banks actually present in their PlatformIO environments. Filenames encode bank, bootloader and release version, for example `fm-drift-generative-nano-new-bootloader.X.Y.Z.hex`. `FIRMWARE-ARTIFACTS.X.Y.Z.md` is generated with the release and maps each filename to its bank, bootloader and rear-DIP algorithm slots. A separate `BUILD-INFO-*.X.Y.Z.txt` accompanies every generated bank/bootloader variant and records its exact PlatformIO environment and toolchain provenance.

The workflow intentionally does not use generic GitHub auto-generated notes because the changelog is the reviewed release narrative.

## 4. Release workflow

`.github/workflows/release.yml` validates traceability and release-note generation, runs the complete native regression suite once plus filtered bank-specific native/sanitizer qualification and separate Classic/core, shared-ClockSource and bank-owned coverage gates, builds both Nano bootloader targets for every bank present in that tag, checks each AVR image against the flash/SRAM engineering budgets, compiles the corresponding timing-probe images, installs the publication toolchain and the classic static Ubuntu font package, validates that `Ubuntu-R.ttf` and `Ubuntu-L.ttf` resolve before export, verifies the release-specific frozen ODT, builds and validates the versioned user-manual PDF from that snapshot, publishes both ODT and PDF, packages the bank/bootloader variants with unambiguous versioned filenames, generates `FIRMWARE-ARTIFACTS.X.Y.Z.md` plus per-image build provenance, validates the exact expected bank/bootloader HEX/ELF set before publication, writes SHA-256 and MD5 manifests, generates the changelog-based notes and publishes or refreshes the GitHub Release for the selected tag.

The manual check is strict for release builds: the PDF must have the expected page geometry and contain embedded Ubuntu and Ubuntu Light fonts. Font substitution is not accepted for a tagged release. On Ubuntu 24.04 the workflow deliberately installs `fonts-ubuntu-classic` rather than the mixed legacy/variable `fonts-ubuntu` package, and fails before LibreOffice export unless fontconfig resolves `Ubuntu-R.ttf` and `Ubuntu-L.ttf`. If publication still fails, the workflow prints LibreOffice, fontconfig, `pdfinfo` and `pdffonts` diagnostics in the failing step.

### Manual rebuild of an existing release

The release workflow also exposes `workflow_dispatch` for **existing version tags only**. This is maintenance tooling, not an alternative versioning path: the requested tag is fetched and checked out in detached-HEAD state, its package version is revalidated, and all firmware/manual artifacts are rebuilt from that exact tagged source. The workflow never creates, moves or rewrites the Git tag.

In **Actions → Release firmware → Run workflow**, enter `vX.Y.Z` (the leading `v` may be omitted) and choose one operation:

- `refresh` — preserve the GitHub Release object and its metadata, rebuild the tagged source and replace same-named generated assets with `gh release upload --clobber`. Unrelated assets and the existing release text are left untouched. If the Release object is missing, it is created from the existing tag.
- `recreate` — delete the GitHub Release object **without deleting the tag**, then create it again with regenerated changelog-based notes and the complete new artifact set. The workflow preserves whether an existing Release was marked Latest. This mode is the appropriate choice when stale/renamed assets or release text must be regenerated completely.

A published immutable GitHub Release cannot be refreshed or recreated; the workflow detects this state and fails before attempting a destructive operation. For a manual rebuild, release-orchestration tooling is taken from the current default branch while firmware, changelog and manual source remain pinned to the requested tag. Tags created after the freeze mechanism was introduced must contain `docs/manual/releases/X.Y.Z/drift-user-manual.X.Y.Z.odt` and the workflow refuses to substitute the current development manual. Historical tags that predate the mechanism use the unversioned `docs/manual/drift-user-manual.odt` stored inside that exact tag as a legacy source; `v0.1.0` is such a release. The historical source can also be backfilled into the archive byte-for-byte with `python scripts/freeze_user_manual.py --version 0.1.0 --from-git-ref v0.1.0` without moving the tag. Historical tags are detected from their PlatformIO environments and package only the banks that existed at that tag. Release `0.3.0` supports Classic, Organic, Generative, Ambient, Electronica, Percussion and Dubstep/Bass. The release workflow checks bank-specific manual coverage before publication for banks added after the original Classic/Organic documentation baseline: Generative requires Turing/Markov/Motif/Urn, Ambient requires Current/Anchor/Breath/Fog, Electronica requires Pump/Acid/Shuffle/Polymeter, Percussion requires Euclid/Repeat/Probability/Humanize, and Dubstep/Bass requires Wobble/Growl/Chop/Build in the frozen ODT. The maintained manual and the frozen `0.3.0` snapshot cover Dubstep/Bass, while the guard prevents any tag from publishing that bank unless its frozen manual snapshot contains the same bank and algorithm coverage. This prevents a selected firmware bank from being published with an obsolete user manual.

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
