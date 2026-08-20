# GitHub Wiki publication

The GitHub Wiki is a **generated navigation and long-form reading layer**, not a second documentation source of truth. User guides, bank guides, development documentation and engineering analyses remain maintained in the main repository and are transformed into wiki pages by `scripts/generate_wiki.py`.

## Why this model

GitHub stores a repository wiki as a separate Git repository. Maintaining the same technical content manually in both repositories would inevitably create version drift. GitHub documents local wiki cloning/editing after the first page is created: <https://docs.github.com/en/communities/documenting-your-project-with-wikis/adding-or-editing-wiki-pages>. The publishing workflow therefore owns a defined set of wiki pages and regenerates them from canonical Markdown after documentation changes reach `main`.

Generated pages carry a visible warning and the source-file link. GitHub uses `_Sidebar.md` and `_Footer.md` as the custom wiki navigation surfaces: <https://docs.github.com/en/communities/documenting-your-project-with-wikis/creating-a-footer-or-sidebar-for-your-wiki>. Manual wiki pages are allowed, but the publisher deletes/replaces **only** files recorded in `.fmd-managed-pages`; unrelated hand-written pages are preserved.

## Managed content

The generated wiki includes:

- Home and navigation/sidebar/footer;
- project overview and contribution guidance;
- installation and AVRDUDESS guides;
- all seven bank guides;
- testing/verification documentation;
- source-code reference, code-documentation rules, compatibility and release process;
- original-firmware analysis;
- all 28 per-algorithm engineering analyses;
- all bank-level design analyses present under `docs/analysis/algorithm-banks/`.

Relative repository links are converted either to another generated wiki page or to the canonical file on GitHub. Images are rewritten to raw files on the configured canonical branch because the separate wiki repository does not contain `docs/assets/` or `docs/manual/assets/`.

## Local preview

Generate the exact managed wiki tree without touching GitHub:

```bash
python scripts/generate_wiki.py --output .wiki-preview
python scripts/test_wiki_generation.py
```

`.wiki-preview/` is disposable generated output and must not be committed.

## One-time GitHub setup

GitHub exposes the wiki Git repository only after the wiki has an initial page. Create/enable the repository Wiki and create its first page once through GitHub's Wiki UI. After that, the corresponding repository can be cloned as:

```text
https://github.com/napolitano/eurorack-organic-modulation-firmware.wiki.git
```

The automation deliberately fails with a clear initialization message if this wiki repository is not yet cloneable; it does not attempt to invent a second initialization mechanism.

## Authentication

`.github/workflows/wiki.yml` requests only `contents: write` and first attempts publication with the job's repository-scoped `GITHUB_TOKEN`. GitHub recommends granting `GITHUB_TOKEN` only the minimum permissions required by a workflow: <https://docs.github.com/en/actions/tutorials/authenticate-with-github_token>. Repository/organization policy can restrict that token. If `.wiki.git` rejects the built-in token, define a repository secret named `WIKI_PUSH_TOKEN` containing a token with write access to this repository; the workflow uses it preferentially.

Do not put a token into the generated documentation, command line output or committed configuration.

## Publication behavior

The workflow runs on relevant documentation/tooling changes pushed to `main` and can also be started manually. It:

1. checks out the canonical repository;
2. runs the deterministic wiki-generation regression test;
3. generates the managed pages into a temporary directory;
4. clones the existing `.wiki.git` repository;
5. removes only pages listed by the previous `.fmd-managed-pages` manifest;
6. copies the new generated pages and manifest;
7. commits only when content actually changed;
8. pushes the wiki default branch.

This gives the Wiki useful navigation without creating an independent documentation lifecycle.

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
