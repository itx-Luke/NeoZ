# Neo-Z Wiki

Welcome to the Neo-Z project wiki! This is your central resource for understanding, developing, and contributing to Neo-Z.

---

## Quick Links

| Resource | Description |
|----------|-------------|
| [📖 README](../README.md) | Project overview and quick start |
| [🏗️ Architecture](../ARCHITECTURE.md) | System design and components |
| [🚀 Deployment](../DEPLOYMENT.md) | Build and deploy instructions |
| [🔐 Security](../SECURITY.md) | Security policy and reporting |
| [🛠️ Troubleshooting](../TROUBLESHOOTING.md) | Common issues and solutions |
| [📚 API Reference](../docs/api/README.md) | Complete API documentation |

---

## Getting Started

### For Users

1. Download the latest release from [GitHub Releases](https://github.com/itx-Luke/test/releases)
2. Extract and run `appNeo-Z.exe`
3. Connect your Android emulator
4. Configure sensitivity settings

### For Developers

1. Clone the repository
2. Install Qt 6.6+ and CMake 3.16+
3. Build: `cmake -B build && cmake --build build`
4. Run tests: `ctest --test-dir build`

See [CONTRIBUTING.md](../CONTRIBUTING.md) for detailed setup instructions.

---

## Project Structure

```
Neo-Z/
├── src/
│   ├── backend/      # NeoController (QML bridge)
│   ├── core/         # Business logic
│   └── ui/           # QML interface
├── tests/            # Unit and integration tests
├── docs/             # Documentation
└── .github/          # CI/CD workflows
```

---

## Key Concepts

### DRCS (Dynamic Resolution Calibration System)

Automatically adjusts sensitivity based on display resolution to maintain consistent aim feel across different setups.

### AI Advisor

Uses Google Gemini AI to analyze your sensitivity settings and recommend optimizations based on your play style.

### ADB Integration

Connects to Android emulators via ADB for real-time communication and script execution.

---

## FAQ

**Q: Which emulators are supported?**
A: Primarily HD Player, but any emulator with ADB support should work.

**Q: Do I need an API key?**
A: Only for AI features. Set `GEMINI_API_KEY` environment variable if you want AI recommendations.

**Q: Is this safe to use?**
A: Yes, Neo-Z runs at user level with no kernel modifications.

---

## Support

- [GitHub Issues](https://github.com/itx-Luke/test/issues) - Report bugs
- [GitHub Discussions](https://github.com/itx-Luke/test/discussions) - Ask questions
