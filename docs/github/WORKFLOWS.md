# Workflow Details

Detailed documentation for each GitHub Actions workflow.

---

## build.yml - Main Build

**Triggers:** Push to `main`/`develop`, Pull requests to `main`

**What it does:**
1. Checkout code
2. Install Qt 6.6.0 with MinGW
3. Configure CMake with Release build
4. Build the application
5. Run tests
6. Upload `appNeo-Z.exe` artifact

**Key configuration:**
```yaml
tools: 'tools_mingw90'           # Installs MinGW compiler
arch: 'win64_mingw'              # Windows 64-bit MinGW
CMAKE_CXX_COMPILER: g++.exe      # Explicit compiler path
```

---

## tests.yml - Test Execution

**Triggers:** Push to `main`/`develop`, Pull requests to `main`

**Test suites:**
- `tst_neocontroller` - Controller integration
- `tst_logger` - Logging unit tests
- `tst_sensitivity` - Sensitivity calculations
- `tst_drcs` - DRCS algorithm
- `tst_e2e` - End-to-end tests

---

## lint.yml - Code Style

**Triggers:** Pull requests (only C++/H files)

**Checks:**
- clang-format (C++ formatting)
- qmllint (QML validation)

---

## release.yml - Automated Releases

**Triggers:** Push tags matching `v*.*.*`

**What it does:**
1. Build release version
2. Run `windeployqt` for dependencies
3. Package as `.zip`
4. Create GitHub Release

**To trigger:**
```powershell
git tag v0.1.0
git push --tags
```

---

## security.yml - Security Scanning

**Triggers:** Push, PR, Weekly (Monday 00:00 UTC)

**Scans:**
- CodeQL static analysis
- Dependency review (PRs only)

---

## coverage.yml - Code Coverage

**Triggers:** Push to `main`, Pull requests

**What it does:**
1. Build with `--coverage` flag
2. Run tests
3. Generate report with `gcovr`
4. Upload to Codecov

**Requires:** `CODECOV_TOKEN` secret

---

## labels.yml - Label Sync

**Triggers:** Changes to `.github/labels.yml`

**Labels defined:**
- Priority: critical, high, medium, low
- Type: bug, feature, enhancement, docs
- Status: in-progress, blocked, review-needed
- Component: ui, backend, adb, sensitivity, ai
