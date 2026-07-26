# Windows build

Scyclone on Windows requires **MSVC 14.51** (v143, Visual Studio 2026) to link [anira-project/backends](https://github.com/anira-project/backends) ONNX Runtime. Shared CMake presets inherit a Windows-only toolchain that reads a generated environment snapshot — plain `cmake --preset` and `cmake --build` work like on Linux and macOS after bootstrap.

## Bootstrap (once per machine, or after a VS update)

```powershell
.\cmake\windows\ensure-msvc.ps1
```

`ensure-msvc.ps1` installs the MSVC toolset if missing, runs `vcvars64 -vcvars_ver=14.51`, and writes `cmake/windows/generated/msvc-env.cmake`. It does **not** run CMake.

## Configure and build

```powershell
cmake --preset release          # plugin
cmake --build --preset release

cmake --preset default          # Debug / tests
cmake --build --preset test
ctest --test-dir build -L default --output-on-failure
```

Re-run `ensure-msvc.ps1` only after a Visual Studio or toolset update. Switching presets uses normal `cmake --preset …`.

Output: `build-release/Scyclone_artefacts/Release/` (VST3 and Standalone).

Debug, tests, and sanitizers: [test/README.md](../../test/README.md).

## CI

GitHub Actions loads vcvars via [`.github/actions/setup-msvc`](../../.github/actions/setup-msvc/action.yml). The Windows toolchain detects `$LIB`/`$INCLUDE` and skips the generated snapshot — no `ensure-msvc.ps1` or interactive prompts in CI.
