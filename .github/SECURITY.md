# Security Policy

## Supported versions

Security fixes are maintained for the latest tagged release. Before the first tagged release, only the current development branch is maintained. Older releases may be superseded without backports.

| Version | Supported |
|---|---|
| Current development branch (before first release) | Yes |
| Latest tagged release (once available) | Yes |
| Older tagged releases | No |

## Reporting a vulnerability

> [!WARNING]
> Do **not** publish suspected security vulnerabilities in an issue, discussion, pull request or commit message. Use a private reporting route first.

Use GitHub private vulnerability reporting for this repository whenever available. Include the affected commit/release, relevant hardware configuration, impact, reproducible steps or minimal proof of concept, and any known mitigation.

If private vulnerability reporting is unavailable, open a public issue containing no vulnerability details and request a private contact channel from the maintainer.

## Scope

This policy covers the firmware and repository-controlled build, test and release automation. General component faults, analog tolerances, assembly mistakes, calibration drift, third-party toolchain vulnerabilities and external-service vulnerabilities are outside the direct scope unless this repository materially contributes to the issue.

## Response

Reports are reviewed on a best-effort basis. Valid issues will normally be investigated and fixed privately before disclosure when practical. Security advisories and release notes may be published once a correction is available.

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="../docs/assets/drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
