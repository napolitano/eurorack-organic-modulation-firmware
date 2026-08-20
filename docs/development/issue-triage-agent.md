# Issue triage agent

The repository uses a deliberately conservative, documentation-aware agent for the **first response to newly opened GitHub issues**. Its purpose is not to reject reports or replace maintainer judgement. It is a one-shot helper that tries to connect a report with existing documentation, offers useful first checks when the situation is ambiguous, and prepares likely defects for human investigation.

## Design goals

The agent follows five non-negotiable rules:

1. **Run once:** only the `issues.opened` event triggers the workflow. A hidden marker in the agent comment prevents a manual workflow re-run from posting a second triage response.
2. **Never close or invalidate an issue:** the agent does not close, lock, mark `invalid`, or otherwise dispose of reports.
3. **Evidence before confidence:** a high-confidence feature classification requires a concrete, matching **user-facing canonical documentation section** from the repository. Model confidence alone is never sufficient.
4. **Escalate risk:** safety/hardware-damage reports, crashes/hangs, build/flash failures, explicit regressions and reports that contradict the documentation are forced into maintainer triage.
5. **Be useful:** every visible response must either explain a relevant documented behaviour, provide targeted first-aid references/checks, or prepare a defect dossier with likely code/tests and missing diagnostic information.

The visible comment always identifies itself as the **Free Modular Drift Triage-Agent** and states that it is an automated first-pass review rather than a final bug/feature decision.

## Routing policy

The model produces a `feature_likelihood` score from 0 to 100 plus an independent evidence-quality score. These values are inputs to a deterministic policy engine, not direct GitHub actions.

| Effective feature likelihood | Route | Agent action |
|---:|---|---|
| `75–100` | Documented feature | Explain the matching behaviour, link the concrete documentation and ask the reporter to compare it with the observed result. The issue remains open. |
| `50–74` | First aid / ambiguous | Do not claim feature or bug. Provide the most relevant documentation references and a short list of useful checks. |
| `0–49` | Likely bug | Prepare expected/observed behaviour, plausible source and test entry points, missing information, apply the likely-bug label and assign the configured maintainer. |

### High-feature evidence gate

A score of 75 or more is **not sufficient** on its own. The high-feature route additionally requires:

- at least one documentation candidate selected by the model;
- that candidate to come from user-facing canonical documentation such as the root README, a bank README or the installation/manual documentation;
- evidence quality of at least 70.

If any of these requirements is missing, the effective feature score is capped at 74 and the issue receives the first-aid response instead.

Engineering analyses can help the model understand a report but **cannot by themselves trigger the high-feature response**. This prevents a user from being redirected to internal design notes when the behaviour was never properly documented for users.

### Hard escalations

The policy engine caps the feature score below 50 when the issue contains a strong signal in one of these classes:

- safety or possible hardware damage;
- crash, hang or similar runtime failure;
- compile, build, upload or flashing failure;
- explicit regression from an earlier release/working state;
- explicit mismatch between the documentation/manual and observed behaviour.

These are deterministic repository rules. The model cannot override them.

## Retrieval

`scripts/issue_triage_agent.py` performs local retrieval before inference.

### Documentation

Markdown is split by headings and ranked against the issue title/body. The candidate set includes:

- `README.md`;
- all `README-BANK-*.md` guides;
- installation and manual documentation;
- testing/development documentation;
- engineering analyses under `docs/analysis/`.

The model receives stable candidate IDs such as `D01`, never an unrestricted request to invent a file path. Returned IDs are validated against the retrieved set before they can appear in a comment.

### Source and tests

For likely-defect preparation, the same ticket text is matched against:

- portable and platform source under `lib/fmd/` and `src/`;
- tests under `test/`.

The model may select only the supplied `Sxx` and `Txx` candidates. These links are described as **plausible investigation starting points**, never as proof that a particular file is defective or that a test has passed/failed.

## Model boundary

The workflow currently uses the OpenAI Responses API. GitHub Models is intentionally not used: GitHub retired the standalone GitHub Models service, including its inference API, on 30 July 2026.

The workflow sends the following to the model:

- issue title and body;
- selected repository documentation excerpts;
- compact source/test candidate hints;
- the fixed analysis instructions and JSON schema.

The API request sets `store: false`. No repository secret is included in the prompt. The API key is used only in the HTTP `Authorization` header.

The default model is `gpt-5.6-terra`; it can be replaced with the `TRIAGE_MODEL` repository variable without changing the policy code.

## Prompt-injection boundary

Issue content is fully untrusted. GitHub explicitly warns that issue titles and bodies can contain attacker-controlled data and should not be injected into executable workflow scripts.

The implementation therefore:

- reads the original JSON payload through `GITHUB_EVENT_PATH` in Python;
- never substitutes `github.event.issue.title` or `github.event.issue.body` into a shell command;
- tells the model that issue content is evidence and **never instructions**;
- validates every returned documentation/source/test ID;
- keeps routing and GitHub mutations outside the model;
- gives the workflow only `contents: read` and `issues: write` permissions.

The model cannot ask the workflow to close an issue, run commands, modify source code, or choose arbitrary GitHub API operations.

## GitHub mutations

The policy engine can perform only these issue changes:

- add `triage: agent-reviewed`;
- add `triage: docs-relevant` for the documented-feature or first-aid routes;
- add `triage: likely-bug` for the likely-bug route;
- assign the configured maintainer for the likely-bug route;
- post exactly one triage comment.

The labels are created automatically if they do not exist. `TRIAGE_MAINTAINER` can be set as a repository variable; if unset, the repository owner is used. For this repository that resolves to `napolitano`.

The comment is deliberately posted **last**. A visible agent response therefore means that the required labels and, for a likely bug, the assignee mutation have already succeeded.

## One-shot protection

The workflow is triggered only by:

```yaml
on:
  issues:
    types: [opened]
```

The posted comment additionally contains:

```html
<!-- fmd-triage-agent:v1 -->
```

Before spending a model request, the script inspects existing issue comments for this marker. A manual job re-run therefore exits without a second comment.

## Repository setup

The workflow needs one repository secret:

- `OPENAI_API_KEY` — API key used only for the inference request.

Optional repository variables:

- `TRIAGE_MODEL` — model override; empty means `gpt-5.6-terra`;
- `TRIAGE_MAINTAINER` — GitHub login for likely-bug assignment; empty means repository owner.

GitHub path:

**Settings → Secrets and variables → Actions**

Create the secret under **Secrets** and the optional settings under **Variables**.

The workflow itself is `.github/workflows/issue-triage.yml`.

## Verification

The deterministic policy, retrieval expectations, safety overrides, wording constraints and workflow permissions are covered by:

```bash
python scripts/test_issue_triage_agent.py
```

A local end-to-end dry run can use a captured `issues.opened` JSON payload and a fixture model result:

```bash
python scripts/issue_triage_agent.py \
  --event /path/to/issues-opened.json \
  --analysis-fixture /path/to/analysis.json \
  --dry-run
```

This performs retrieval, policy routing and comment rendering without GitHub writes or a model request.

The test contract also forbids the retired GitHub Models permission, direct issue-title/body interpolation in workflow shell, and issue-closing/locking mutations in the agent implementation.

## Failure behaviour

The agent fails closed from an automation perspective:

- missing `OPENAI_API_KEY` causes the workflow to fail without posting a misleading comment;
- invalid model output is rejected before routing;
- unknown candidate IDs are discarded;
- a failed required label/assignment mutation prevents the final comment from being posted;
- no fallback path guesses a bug/feature result when inference fails.

A failed triage workflow therefore leaves the issue untouched for normal human review rather than posting a low-quality automated answer.

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
