---
description: How to build the Neo-Z application correctly
---

# Building Neo-Z

The main application target is `appNeo-Z`. Qt Creator may sometimes try to build 
other targets (like `hidapi_winapi`) by default. Here's how to ensure you always 
build the correct target.

## Quick Build Commands

// turbo-all

1. Clean the build (optional, if having issues):
```powershell
cmake --build "D:/NeoZ/Neo-Z/build/Desktop_Qt_6_10_1_MinGW_64_bit-Debug" --target clean
```

2. Build appNeo-Z:
```powershell
cmake --build "D:/NeoZ/Neo-Z/build/Desktop_Qt_6_10_1_MinGW_64_bit-Debug" --target appNeo-Z
```

3. Run the application:
```powershell
& "D:/NeoZ/Neo-Z/build/Desktop_Qt_6_10_1_MinGW_64_bit-Debug/appNeo-Z.exe"
```

## Qt Creator Configuration

If Qt Creator is building the wrong target:

1. Go to **Projects** (left sidebar) → **Build Settings**
2. Under **Build Steps**, click on the CMake build step
3. In the **Targets** dropdown, select `appNeo-Z` instead of `all` or `hidapi_winapi`
4. Apply the changes

## Full Rebuild (Clean Slate)

If you're having persistent caching issues:

1. Close Qt Creator
2. Delete the entire build folder:
```powershell
Remove-Item -Recurse -Force "D:/NeoZ/Neo-Z/build/Desktop_Qt_6_10_1_MinGW_64_bit-Debug"
```
3. Reopen Qt Creator
4. Configure the project again
5. Build with **Build → Build Project "Neo-Z"**

## CMake Configuration

The CMakeLists.txt is configured with:
- `appNeo-Z` as the default startup project (VS_STARTUP_PROJECT)
- `hidapi` targets set to EXCLUDE_FROM_ALL to prevent them being built by default
- A `build_all` target that depends on `appNeo-Z` for the "all" build
