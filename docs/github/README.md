# GitHub Configuration Overview

Local reference for all GitHub Actions, templates, and automation.

---

## Quick Links

| File | Purpose | GitHub Link |
|------|---------|-------------|
| [build.yml](../../.github/workflows/build.yml) | Main build for Windows | [View on GitHub](https://github.com/itx-Luke/test/blob/main/.github/workflows/build.yml) |
| [tests.yml](../../.github/workflows/tests.yml) | Test suite execution | [View on GitHub](https://github.com/itx-Luke/test/blob/main/.github/workflows/tests.yml) |
| [lint.yml](../../.github/workflows/lint.yml) | Code formatting checks | [View on GitHub](https://github.com/itx-Luke/test/blob/main/.github/workflows/lint.yml) |
| [release.yml](../../.github/workflows/release.yml) | Auto-release on tags | [View on GitHub](https://github.com/itx-Luke/test/blob/main/.github/workflows/release.yml) |
| [security.yml](../../.github/workflows/security.yml) | CodeQL scanning | [View on GitHub](https://github.com/itx-Luke/test/blob/main/.github/workflows/security.yml) |
| [coverage.yml](../../.github/workflows/coverage.yml) | Code coverage | [View on GitHub](https://github.com/itx-Luke/test/blob/main/.github/workflows/coverage.yml) |
| [labels.yml](../../.github/workflows/labels.yml) | Auto-sync labels | [View on GitHub](https://github.com/itx-Luke/test/blob/main/.github/workflows/labels.yml) |

---

## Workflow Summary

```
┌─────────────────────────────────────────────────────────────┐
│                    GitHub Actions CI/CD                      │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│   push/PR to main ──► build.yml ──► Build + Artifacts       │
│                   └──► tests.yml ──► Run Test Suite         │
│                   └──► lint.yml ──► clang-format check      │
│                   └──► security.yml ──► CodeQL scan         │
│                   └──► coverage.yml ──► Codecov report      │
│                                                              │
│   git tag v*.*.* ──► release.yml ──► GitHub Release         │
│                                                              │
│   Weekly ──► security.yml ──► Scheduled security scan       │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

---

## Sync Commands

### Pull Latest from GitHub
```powershell
git pull origin main
```

### Push Local Changes
```powershell
git add -A
git commit -m "your message"
git push origin main
```

### Create a Release
```powershell
git tag v0.1.0
git push --tags
```

---

## Secrets Required

| Secret | Purpose | Where to Set |
|--------|---------|--------------|
| `CODECOV_TOKEN` | Coverage uploads | Repo Settings → Secrets |

---

## Status Badges

Copy these for your README:

```markdown
[![Build](https://github.com/itx-Luke/test/actions/workflows/build.yml/badge.svg)](https://github.com/itx-Luke/test/actions/workflows/build.yml)
[![Tests](https://github.com/itx-Luke/test/actions/workflows/tests.yml/badge.svg)](https://github.com/itx-Luke/test/actions/workflows/tests.yml)
[![Security](https://github.com/itx-Luke/test/actions/workflows/security.yml/badge.svg)](https://github.com/itx-Luke/test/actions/workflows/security.yml)
```

---

## File Locations

```
.github/
├── dependabot.yml          # Dependency updates config
├── labels.yml              # Project labels definition
├── PULL_REQUEST_TEMPLATE.md
├── ISSUE_TEMPLATE/
│   ├── bug_report.md
│   └── feature_request.md
└── workflows/
    ├── build.yml           # Main CI build
    ├── tests.yml           # Test execution
    ├── lint.yml            # Code style
    ├── release.yml         # Auto releases
    ├── security.yml        # CodeQL
    ├── coverage.yml        # Codecov
    └── labels.yml          # Label sync
```

---

*Last updated: 2025-12-31*
