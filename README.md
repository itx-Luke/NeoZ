# Neo-Z

[![Build Neo-Z](https://github.com/itx-Luke/test/actions/workflows/build.yml/badge.svg)](https://github.com/itx-Luke/test/actions/workflows/build.yml)
[![Tests](https://github.com/itx-Luke/test/actions/workflows/tests.yml/badge.svg)](https://github.com/itx-Luke/test/actions/workflows/tests.yml)
[![Lint](https://github.com/itx-Luke/test/actions/workflows/lint.yml/badge.svg)](https://github.com/itx-Luke/test/actions/workflows/lint.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![GitHub release](https://img.shields.io/github/v/release/itx-Luke/test?include_prereleases)](https://github.com/itx-Luke/test/releases)

**High-performance gaming sensitivity optimization and emulator control suite for Windows.**

Neo-Z is a Qt-based desktop application that provides advanced sensitivity calibration, AI-driven recommendations, and seamless ADB integration for Android emulators.

---

## ✨ Features

- **Dynamic Resolution & Calibration System (DRCS)** — Auto-adjust sensitivity based on display resolution
- **AI-Powered Recommendations** — Get intelligent sensitivity suggestions via Gemini AI
- **Emulator Integration** — Connect to HD Player and other Android emulators via ADB
- **Script Runner** — Execute shell scripts on connected emulators
- **Modern QML UI** — Sleek, responsive interface with theme support

---

## 📋 System Requirements

| Requirement | Version |
|-------------|---------|
| **OS** | Windows 10/11 (64-bit) |
| **Qt** | 6.6.0 or higher |
| **CMake** | 3.16 or higher |
| **Compiler** | MinGW 11+ or MSVC 2022 |
| **ADB** | Android Debug Bridge (bundled) |

---

## 🚀 Quick Start

### Building from Source

```powershell
# Clone the repository
git clone https://github.com/itx-Luke/test.git
cd test

# Configure and build
cmake -B build -DCMAKE_BUILD_TYPE=Release -G "MinGW Makefiles"
cmake --build build --config Release --parallel

# Run the application
.\build\appNeo-Z.exe
```

### Running Tests

```powershell
cmake -B build -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

---

## 📁 Project Structure

```
Neo-Z/
├── src/                    # Main source code
│   ├── backend/            # Core controllers
│   ├── core/               # Business logic modules
│   │   ├── adb/            # ADB connectivity
│   │   ├── ai/             # Gemini AI integration
│   │   ├── aim/            # Crosshair detection
│   │   ├── input/          # Input handling
│   │   ├── sensitivity/    # DRCS & calibration
│   │   └── managers/       # Service managers
│   └── ui/                 # QML interface
├── tests/                  # Unit and integration tests
├── assets/                 # Images, icons, videos
├── config/                 # Configuration profiles
├── docs/                   # Documentation
└── .github/                # CI/CD workflows
```

---

## 🤝 Contributing

We welcome contributions! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines on:

- Setting up your development environment
- Code style and commit conventions
- Submitting pull requests

---

## 📄 License

This project is licensed under the MIT License — see the [LICENSE](LICENSE) file for details.

---

## 📞 Support

- **Issues**: [GitHub Issues](https://github.com/itx-Luke/test/issues)
- **Discussions**: [GitHub Discussions](https://github.com/itx-Luke/test/discussions)

---

Made with ❤️ by [itx-Luke](https://github.com/itx-Luke)
