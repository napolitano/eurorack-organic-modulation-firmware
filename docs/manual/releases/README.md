<!-- SPDX-FileCopyrightText: 2026 Axel Napolitano -->
<!-- SPDX-License-Identifier: CC-BY-NC-4.0 -->

# Frozen user-manual sources

This directory contains the immutable ODT source snapshot for each prepared firmware release.

Normal editing happens only in `../drift-user-manual.odt`. During release preparation, after the manual content and layout are final, freeze that exact source with:

```bash
python scripts/freeze_user_manual.py --version X.Y.Z
python scripts/freeze_user_manual.py --version X.Y.Z --check
```

The resulting file is:

```text
X.Y.Z/drift-user-manual.X.Y.Z.odt
```

The release commit and tag must contain that snapshot. The tag workflow builds the PDF from the frozen ODT and publishes both the versioned ODT and PDF. Once a snapshot exists, the freeze tool refuses to replace it with different bytes.

## Historical backfill

Releases created before this archive mechanism are not reconstructed from the current manual. If the old Git tag still contains the original unversioned source, recover it byte-for-byte from that tag:

```bash
python scripts/freeze_user_manual.py --version 0.1.0 --from-git-ref v0.1.0
```

This reads `docs/manual/drift-user-manual.odt` directly from the requested Git ref and does not move or rewrite the tag.

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../../assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
