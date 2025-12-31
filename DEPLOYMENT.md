# Deployment Guide

Instructions for building and deploying Neo-Z.

---

## Prerequisites

| Requirement | Version | Notes |
|-------------|---------|-------|
| Windows | 10/11 64-bit | Primary supported platform |
| Qt | 6.6.0+ | With Qt Quick, Qt Network |
| CMake | 3.16+ | Build system |
| MinGW | 11.0+ | Or MSVC 2022 |
| ADB | Latest | Android Debug Bridge |

---

## Build Instructions

### Development Build

```powershell
# Clone repository
git clone https://github.com/itx-Luke/test.git
cd test

# Configure
cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug

# Build
cmake --build build --parallel
```

### Release Build

```powershell
cmake -B build_release -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build_release --config Release --parallel
```

### With Tests

```powershell
cmake -B build -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

---

## Deployment

### Windows Deployment

1. **Build Release:**
   ```powershell
   cmake --build build_release --config Release
   ```

2. **Deploy Qt Dependencies:**
   ```powershell
   cd build_release
   windeployqt --qmldir ../src/ui appNeo-Z.exe
   ```

3. **Include ADB:**
   - Copy `adb.exe` and related DLLs to deployment folder
   - Or ensure ADB is in system PATH

4. **Package:**
   ```
   Neo-Z/
   ├── appNeo-Z.exe
   ├── Qt6Core.dll
   ├── Qt6Quick.dll
   ├── Qt6Network.dll
   ├── qml/
   ├── platforms/
   └── runtime/
       └── adb.exe
   ```

---

## Environment Variables

| Variable | Purpose | Default |
|----------|---------|---------|
| `GEMINI_API_KEY` | AI recommendations API key | (none) |
| `NEO_LOG_LEVEL` | Logging verbosity (DEBUG/INFO/WARN/ERROR) | INFO |

---

## Configuration

Default configuration is stored in:
- **Windows:** `%APPDATA%/Neo-Z/config.json`

---

## Rollback Procedure

1. Stop the application
2. Replace executable with previous version
3. Clear config if needed: `%APPDATA%/Neo-Z/`
4. Restart application

---

## Health Checks

After deployment, verify:

- [ ] Application starts without crash
- [ ] ADB connection works with emulator
- [ ] Sensitivity adjustments apply correctly
- [ ] AI features respond (if API key configured)

---

## Troubleshooting Deployment

| Issue | Solution |
|-------|----------|
| Missing DLLs | Run `windeployqt` again |
| ADB not found | Ensure `runtime/adb.exe` exists or ADB in PATH |
| QML errors | Verify `qml/` folder is deployed |
| Crashes on start | Check logs in `%APPDATA%/Neo-Z/logs/` |
