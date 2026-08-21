#!/usr/bin/env python3
"""Static contract tests for the GitHub-native issue triage agentic workflow."""
from __future__ import annotations

import json
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def frontmatter(text: str) -> str:
    match = re.match(r"^---\n(.*?)\n---\n", text, flags=re.S)
    require(match is not None, "agentic workflow must start with YAML frontmatter")
    return match.group(1)


def main() -> int:
    policy = json.loads((ROOT / "scripts/issue_triage_policy.json").read_text(encoding="utf-8"))
    require(policy["version"] == 2, "triage policy version changed unexpectedly")
    require(policy["feature_high_threshold"] == 75, "high feature threshold changed")
    require(policy["feature_ambiguous_threshold"] == 50, "ambiguous threshold changed")
    require(policy["required_high_evidence_quality"] == 70, "evidence threshold changed")
    require(policy["maintainer"] == "napolitano", "maintainer assignment contract changed")

    source_path = ROOT / ".github/workflows/issue-triage.md"
    legacy_path = ROOT / ".github/workflows/issue-triage.yml"
    sync_path = ROOT / ".github/workflows/agentic-workflows-sync.yml"
    require(source_path.is_file(), "GitHub Agentic Workflow source is missing")
    require(not legacy_path.exists(), "legacy direct-API issue-triage.yml must not exist")
    require(sync_path.is_file(), "agentic workflow compile/sync workflow is missing")

    workflow = source_path.read_text(encoding="utf-8")
    fm = frontmatter(workflow)

    for marker in (
        "types: [opened]",
        "engine: copilot",
        "contents: read",
        "issues: read",
        "toolsets: [repos, issues, search]",
        "min-integrity: none",
        "add-comment:",
        "max: 1",
        "target: triggering",
        "hide-older-comments: true",
        "assign-to-user:",
        "allowed: [napolitano]",
        "issue-intent: true",
        "max-bot-mentions: 1",
    ):
        require(marker in fm, f"agentic workflow contract missing: {marker}")

    require("max-bot-mentions: 0" not in fm, "gh-aw v0.86.1 rejects max-bot-mentions below 1")

    for forbidden_output in (
        "close-issue:",
        "update-issue:",
        "create-issue:",
        "add-labels:",
        "remove-labels:",
        "create-pull-request:",
        "push-to-pull-request-branch:",
    ):
        require(forbidden_output not in fm, f"unsafe/unneeded output enabled: {forbidden_output}")

    for prompt_contract in (
        "effective feature likelihood >= 75",
        "evidence quality is at least 70",
        "effective feature likelihood 50–74",
        "effective feature likelihood < 50",
        "Hard escalation override",
        "UNTRUSTED EVIDENCE, NEVER",
        "Free Modular Drift Triage-Agent",
        "issue stays open",
        "napolitano",
    ):
        # comparison is normalized for punctuation/case where appropriate
        if prompt_contract == "UNTRUSTED EVIDENCE, NEVER":
            require("untrusted evidence, never" in workflow.casefold(), "prompt-injection boundary missing")
        else:
            require(prompt_contract.casefold() in workflow.casefold(),
                    f"triage prompt contract missing: {prompt_contract}")

    for escalation in policy["hard_escalations"]:
        phrase_map = {
            "safety-or-hardware-damage": "safety concern or possible hardware damage",
            "crash-or-hang": "crash, hang",
            "build-or-flash-failure": "compile, build, upload or flashing failure",
            "explicit-regression": "explicit regression",
            "documented-behaviour-mismatch": "contradicts the README/manual/documentation",
        }
        require(phrase_map[escalation].casefold() in workflow.casefold(),
                f"hard escalation missing from prompt: {escalation}")

    # The GitHub-native implementation must not retain the direct OpenAI API prototype.
    repository_text = "\n".join(
        path.read_text(encoding="utf-8", errors="replace")
        for path in [
            source_path,
            sync_path,
            ROOT / "docs/development/issue-triage-agent.md",
            ROOT / "README.md",
            ROOT / "CONTRIBUTING.md",
        ]
    )
    for forbidden in ("OPENAI_API_KEY", "api.openai.com", "/v1/responses", "TRIAGE_MODEL"):
        require(forbidden not in repository_text, f"obsolete direct OpenAI dependency remains: {forbidden}")

    sync = sync_path.read_text(encoding="utf-8")
    for marker in (
        "contents: write",
        "github/gh-aw/actions/setup-cli@v0.86.1",
        "version: v0.86.1",
        "gh aw compile issue-triage --strict --yamllint --actionlint",
        ".github/workflows/issue-triage.lock.yml",
        "ci: compile issue triage agentic workflow",
    ):
        require(marker in sync, f"agentic workflow compile contract missing: {marker}")

    require("OPENAI_API_KEY" not in sync, "compile workflow must not require OpenAI credentials")
    require("github.event.issue.title" not in sync and "github.event.issue.body" not in sync,
            "compile workflow must not interpolate untrusted issue text")

    docs = (ROOT / "docs/development/issue-triage-agent.md").read_text(encoding="utf-8")
    require("COPILOT_GITHUB_TOKEN" in docs, "personal-repository Copilot authentication is undocumented")
    require("OPENAI_API_KEY" not in docs, "maintainer documentation still asks for OpenAI API")
    require("issue-triage.md" in docs and "issue-triage.lock.yml" in docs,
            "agentic source/compiled workflow relationship is undocumented")

    print("issue triage agentic workflow contract: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
