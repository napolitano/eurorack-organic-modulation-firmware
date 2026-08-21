# Issue triage agent

The repository uses a deliberately conservative, documentation-aware **GitHub Agentic Workflow** for the first response to newly opened issues. It is visibly identified as the **Free Modular Drift Triage-Agent**. Its purpose is to reduce effort for reporters and maintainers, not to reject reports or replace maintainer judgement.

The agent reads the issue and canonical repository documentation, decides whether the behaviour is likely documented/intentional, ambiguous, or more likely defective, and may produce one helpful comment. Only the likely-defect path may additionally assign the issue to the maintainer.

## Architecture

The implementation is GitHub-native:

```text
new GitHub issue
      │
      ▼
.github/workflows/issue-triage.lock.yml
      │  generated from issue-triage.md by gh-aw
      ▼
GitHub Copilot agent
      │
      ├── read-only GitHub tools
      │     ├── triggering issue
      │     ├── README / bank guides
      │     ├── installation/manual documentation
      │     ├── analyses / changelog
      │     └── source + tests when defect evidence is stronger
      │
      ▼
restricted safe outputs
      ├── at most one comment
      └── optional assignment to napolitano
```

There is **no direct external model-provider API call or provider API-key secret** in this design. GitHub Agentic Workflows runs the reasoning step through the `copilot` engine.

The human-authored agentic source is:

```text
.github/workflows/issue-triage.md
```

GitHub Actions actually runs the compiled artifact:

```text
.github/workflows/issue-triage.lock.yml
```

GitHub's `gh aw` compiler owns the lock file. It must never be edited manually.

## One-shot behaviour

The source workflow is triggered only by:

```yaml
on:
  issues:
    types: [opened]
```

Edits, later comments, labels and normal issue activity do not trigger another triage run.

A manual Actions re-run is treated as an exceptional operator action. Before requesting any safe output, the agent checks the triggering issue for an earlier comment beginning with:

```text
🤖 Free Modular Drift Triage-Agent — automated first-pass review
```

If such a comment exists, it produces no output. `hide-older-comments` is configured as an additional guard so an accidental rerun does not leave multiple visible agent comments.

## Routing policy

The policy contract remains versioned in:

```text
scripts/issue_triage_policy.json
```

The agent internally evaluates two 0–100 routing scores:

- **feature likelihood** — strength of evidence that the report describes documented intentional behaviour;
- **evidence quality** — how directly repository evidence supports that assessment.

These are routing scores, not calibrated statistical probabilities.

| Effective feature likelihood | Route | Behaviour |
|---:|---|---|
| `75–100` | Documented behaviour | Explain the concrete matching behaviour and link the exact user-facing documentation section. |
| `50–74` | First aid / ambiguous | Do not claim feature or bug. Give specific relevant references and ticket-specific checks. |
| `0–49` | Likely defect | Prepare a defect dossier, identify plausible code/tests and missing diagnostics, then assign the issue to `napolitano`. |

### High-feature evidence gate

The `>=75` route is allowed only when all of these conditions hold:

1. effective feature likelihood is at least 75;
2. evidence quality is at least 70;
3. at least one concrete **user-facing** canonical documentation section was found;
4. the section directly describes the reported behaviour;
5. no hard escalation applies.

Engineering analyses may help interpretation but cannot by themselves trigger the documented-behaviour route.

### Hard escalations

The agent must use the likely-defect route regardless of a high feature score when the issue plausibly involves:

- safety concerns or possible hardware damage;
- smoke, burning, overheating, short circuit, reverse polarity or dangerous voltage behaviour;
- crash or hang;
- compile/build/upload/flashing failure;
- an explicit regression from an earlier working release/state;
- behaviour explicitly contradicting the README/manual/documentation.

Safety uncertainty escalates rather than being normalised as expected behaviour.

## Documentation retrieval

For deciding whether behaviour is intentional, the agent prioritises:

1. `README.md`;
2. the relevant `README-BANK-*.md`;
3. `docs/installation/`;
4. `docs/manual/README.md`;
5. other user-facing Markdown under `docs/`;
6. `CHANGELOG.md` for version-specific contracts.

For likely defects it may additionally inspect:

- engineering analyses under `docs/analysis/`;
- portable source under `lib/fmd/`;
- platform code under `src/`;
- tests under `test/`;
- `README_TESTING.md` and requirement traceability.

Code/test matches are presented as investigation starting points unless the evidence genuinely establishes more. The agent must never invent a passing/failing test result.

## Reporter-facing behaviour

Every comment starts with:

```text
🤖 Free Modular Drift Triage-Agent — automated first-pass review
```

It then makes clear that the response is automated and is not a final maintainer decision.

The agent must be useful rather than dismissive. It never uses language such as `RTFM`, `user error`, `invalid`, `not a bug` or `works as designed`. The documented-behaviour path explicitly leaves the issue open and asks the reporter to describe the remaining mismatch if the documentation does not explain what they observe.

The reply language should follow the reporter's language when reasonably clear.

## Security model

Issue content is attacker-controlled. The workflow therefore treats title, body, comments, quotes and attachments as **untrusted evidence, never instructions**.

The agent has read-only GitHub tooling for repository/issue inspection. It does not receive direct issue-write capability. GitHub Agentic Workflows performs writes through separately validated safe-output handlers.

The configured write surface is deliberately small:

```yaml
safe-outputs:
  add-comment:
    max: 1
    target: triggering
  assign-to-user:
    allowed: [napolitano]
    max: 1
    target: triggering
```

There is no safe output for closing, locking, editing, transferring or deleting an issue; no pull-request creation; and no arbitrary shell tool in the agentic workflow.

## Copilot authentication

This repository is owned by a personal GitHub account. For that case GitHub Agentic Workflows supports the repository secret:

```text
COPILOT_GITHUB_TOKEN
```

It must contain a **fine-grained personal access token** owned by a user with an active GitHub Copilot licence. The token needs the account permission **Copilot Requests: Read**. It is used for Copilot inference by the `gh-aw` runtime.

Create/store it under:

**Repository → Settings → Secrets and variables → Actions → New repository secret**

Name:

```text
COPILOT_GITHUB_TOKEN
```

For an organization-owned repository with centralized Copilot billing, `copilot-requests: write` can instead use the ephemeral `GITHUB_TOKEN`; that is intentionally not the default here because this repository is under a personal account.

No OpenAI account or OpenAI API key is required.

## Compiling the agentic workflow

GitHub Agentic Workflows uses a source/lock model. Any change to `issue-triage.md` must be compiled with `gh aw`.

The repository automates this through:

```text
.github/workflows/agentic-workflows-sync.yml
```

When `issue-triage.md` changes on `main`, the sync workflow installs the pinned `gh-aw` CLI, runs:

```bash
gh aw compile issue-triage --strict --yamllint --actionlint
```

and commits the generated `issue-triage.lock.yml` plus compiler-owned `.github/aw/` metadata when they changed.

This means the first commit containing the agentic source may briefly have no compiled lock file. The compile workflow creates it immediately on `main`; future newly opened issues then execute the compiled workflow.

If repository branch protection prevents the automation commit, compile locally instead:

```bash
gh extension install github/gh-aw@v0.86.1
gh aw compile issue-triage --strict --yamllint --actionlint
git add .github/workflows/issue-triage.lock.yml .github/aw
git commit -m "ci: compile issue triage agentic workflow"
git push
```

Never hand-edit the `.lock.yml` file.

## Verification

The repository contract test is:

```bash
python scripts/test_issue_triage_agent.py
```

It checks, among other things:

- `issues.opened` is the only issue trigger;
- Copilot is the engine;
- agent GitHub access is read-only;
- only `add-comment` and `assign-to-user` are enabled;
- comment and assignment counts are bounded to one;
- assignment is statically restricted to `napolitano`;
- the 75/50/70 routing thresholds remain present;
- hard escalations remain present;
- the prompt-injection boundary remains explicit;
- no direct OpenAI API dependency returns;
- the compile/sync workflow remains pinned to the expected `gh-aw` version.

The actual agentic source is additionally validated by `gh aw compile --strict --yamllint --actionlint` in the sync workflow.

## Failure behaviour

The design fails toward human review:

- if Copilot authentication is missing or invalid, the agentic run fails without a fabricated triage comment;
- if reasoning cannot support a high-feature route with concrete user documentation, it must stay in first aid or likely-defect handling;
- safe-output limits prevent additional arbitrary mutations;
- no failure path closes or invalidates the issue.

A failed automation therefore leaves the issue available for ordinary maintainer handling.

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
