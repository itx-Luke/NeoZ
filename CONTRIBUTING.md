# Contributing to Neo-Z

Thank you for your interest in contributing to Neo-Z! This guide will help you get started.

---

## 🛠️ Development Setup

### Prerequisites

1. **Qt 6.6+** — Install via [Qt Online Installer](https://www.qt.io/download)
2. **CMake 3.16+** — [Download here](https://cmake.org/download/)
3. **MinGW 11+** or **MSVC 2022** — Included with Qt or Visual Studio
4. **Git** — [Download here](https://git-scm.com/)

### Clone & Build

```powershell
git clone https://github.com/itx-Luke/test.git
cd test
cmake -B build -G "MinGW Makefiles"
cmake --build build
```

### Running Tests

```powershell
cmake -B build -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

---

## 📝 Code Style

### C++ Guidelines

- **Standard**: C++20
- **Indentation**: 4 spaces (no tabs)
- **Naming**:
  - Classes: `PascalCase` (e.g., `NeoController`)
  - Methods/Functions: `camelCase` (e.g., `calculateSensitivity`)
  - Constants: `SCREAMING_SNAKE_CASE` (e.g., `MAX_RETRY_COUNT`)
  - Member variables: `m_camelCase` (e.g., `m_isConnected`)

### QML Guidelines

- **Indentation**: 4 spaces
- **Component naming**: `PascalCase`
- Use `Style.qml` for all colors and theming

---

## 💬 Commit Messages

We follow [Conventional Commits](https://www.conventionalcommits.org/):

```
<type>(<scope>): <short description>

[optional body]

[optional footer]
```

### Types

| Type | Description |
|------|-------------|
| `feat` | New feature |
| `fix` | Bug fix |
| `docs` | Documentation only |
| `style` | Code formatting (no logic change) |
| `refactor` | Code restructuring |
| `test` | Adding or updating tests |
| `chore` | Build, CI, or tooling changes |

### Examples

```
feat(adb): add automatic reconnection on disconnect
fix(ui): resolve dropdown closing on click
docs: update README with build instructions
```

---

## 🔄 Pull Request Process

1. **Fork** the repository
2. **Create a branch** from `main`:
   ```bash
   git checkout -b feat/your-feature-name
   ```
3. **Make your changes** with clear, atomic commits
4. **Run tests** to ensure nothing is broken
5. **Push** to your fork and open a PR

### PR Checklist

- [ ] Code follows the style guidelines
- [ ] Tests pass locally
- [ ] New code has appropriate test coverage
- [ ] Documentation updated if needed
- [ ] Commit messages follow conventions

---

## 🐛 Reporting Bugs

Use the [bug report template](https://github.com/itx-Luke/test/issues/new?template=bug_report.md) and include:

- Steps to reproduce
- Expected vs actual behavior
- System information (OS, Qt version)
- Screenshots if applicable

---

## 💡 Feature Requests

Use the [feature request template](https://github.com/itx-Luke/test/issues/new?template=feature_request.md) and describe:

- The problem you're trying to solve
- Your proposed solution
- Alternatives you've considered

---

## 📜 Code of Conduct

Please read and follow our [Code of Conduct](CODE_OF_CONDUCT.md).

---

Thank you for contributing! 🎮
