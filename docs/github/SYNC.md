# Git Sync Guide

How to keep local and GitHub in sync.

---

## Daily Workflow

### Start of Day - Pull Latest
```powershell
cd d:\NeoZ\Neo-Z
git pull origin main
```

### After Making Changes - Push
```powershell
git add -A
git status                    # Review changes
git commit -m "feat: your change"
git push origin main
```

---

## View Differences

### See What Changed Locally
```powershell
git status                    # Files changed
git diff                      # Line-by-line changes
git diff --cached             # Staged changes
```

### See What's on GitHub
```powershell
git fetch origin
git diff main origin/main     # Compare local vs remote
git log origin/main --oneline -5  # Recent remote commits
```

---

## Sync Conflicts

If push is rejected (remote has changes):
```powershell
git pull --rebase origin main
git push origin main
```

---

## GitHub Actions Status

Check workflow status:
- [Build Status](https://github.com/itx-Luke/test/actions/workflows/build.yml)
- [Test Status](https://github.com/itx-Luke/test/actions/workflows/tests.yml)
- [All Workflows](https://github.com/itx-Luke/test/actions)

---

## Quick Reference

| Action | Command |
|--------|---------|
| Pull latest | `git pull origin main` |
| Push changes | `git push origin main` |
| See status | `git status` |
| Create release | `git tag v0.x.x && git push --tags` |
| View log | `git log --oneline -10` |
| Undo last commit | `git reset --soft HEAD~1` |
