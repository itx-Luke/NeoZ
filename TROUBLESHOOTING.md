# Troubleshooting Guide

Common issues and solutions for Neo-Z.

---

## Build Issues

### MOC Errors

**Symptom:** Build fails with MOC (Meta-Object Compiler) errors

**Solutions:**
1. Clean build folder:
   ```powershell
   Remove-Item -Recurse -Force build
   cmake -B build -G "MinGW Makefiles"
   cmake --build build
   ```

2. Ensure Qt is in PATH:
   ```powershell
   $env:PATH += ";C:\Qt\6.6.0\mingw_64\bin"
   ```

### CMake Configuration Fails

**Symptom:** `Qt6 not found`

**Solution:**
```powershell
cmake -B build -DCMAKE_PREFIX_PATH="C:/Qt/6.6.0/mingw_64"
```

### hidapi Build Issues

**Symptom:** hidapi target builds instead of appNeo-Z

**Solution:** Build explicitly:
```powershell
cmake --build build --target appNeo-Z
```

---

## Runtime Issues

### Application Crashes on Start

**Check:**
1. Qt DLLs present (run `windeployqt`)
2. Correct Qt version (6.6.0+)
3. View logs: `%APPDATA%/Neo-Z/logs/`

### ADB Connection Fails

**Symptom:** "No emulators found"

**Solutions:**
1. Ensure emulator is running
2. Check ADB:
   ```powershell
   adb devices
   ```
3. Restart ADB server:
   ```powershell
   adb kill-server
   adb start-server
   ```

### Emulator Not Detected

**Check:**
1. HD Player or compatible emulator is running
2. ADB is enabled in emulator settings
3. Try manual connection:
   ```powershell
   adb connect 127.0.0.1:5555
   ```

---

## Sensitivity Issues

### Sensitivity Not Applying

**Check:**
1. Device is connected (green status)
2. DRCS is enabled in settings
3. Game is running on emulator

### Erratic Mouse Movement

**Try:**
1. Lower sensitivity multipliers
2. Increase smoothing value
3. Adjust DRCS thresholds

---

## AI Features

### AI Recommendations Not Working

**Check:**
1. `GEMINI_API_KEY` environment variable is set
2. Internet connection is available
3. API key is valid

**Set API Key:**
```powershell
$env:GEMINI_API_KEY = "your-api-key-here"
```

---

## Debug Logging

Enable verbose logging:

```powershell
$env:NEO_LOG_LEVEL = "DEBUG"
.\appNeo-Z.exe
```

Log location: `%APPDATA%/Neo-Z/logs/`

---

## Known Limitations

| Issue | Workaround |
|-------|------------|
| Only Windows supported | Use Windows 10/11 |
| Requires Qt 6.6+ | Update Qt installation |
| Some emulators not detected | Use HD Player or check ADB compatibility |

---

## Getting Help

1. Check [GitHub Issues](https://github.com/itx-Luke/test/issues)
2. Search existing issues before creating new ones
3. Include logs and system info when reporting bugs
