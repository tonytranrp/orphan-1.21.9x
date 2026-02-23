# orphan 1.21.9x

## CMake Build (Visual Studio)

This repository is now configured for CMake-first builds.

1. Install Visual Studio 2022 (Desktop development with C++).
2. Open this repository root in Visual Studio.
3. Select the `vs2022-x64` configure preset.
4. Let CMake fetch dependencies (CPM) during configure.
5. Build using `build-debug` or `build-release`.

From terminal:

```powershell
cmake --preset vs2022-x64
cmake --build --preset build-debug
```
