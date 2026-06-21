# Windows build

Scyclone on Windows requires **MSVC 14.51** (v143, Visual Studio 2026) to link [anira-project/backends](https://github.com/anira-project/backends) ONNX Runtime. Shared CMake presets inherit a Windows-only toolchain that bakes include/lib paths from a generated snapshot — plain `cmake --build` works after the first configure.

## First-time setup

```powershell
.\cmake\windows\configure.ps1 -Preset release   # plugin
# or
.\cmake\windows\configure.ps1                   # Debug / tests
```

`configure.ps1` installs the MSVC toolset if missing, runs `vcvars64 -vcvars_ver=14.51`, writes `cmake/windows/generated/msvc-env.cmake`, and configures the chosen preset.

## Day-to-day builds

After configure, use normal CMake preset commands from any shell (no Developer Prompt, no build wrapper):

```powershell
cmake --build --preset release    # VST3 + Standalone
cmake --build --preset test       # Test target only
ctest --test-dir build -L default --output-on-failure
```

Re-run `configure.ps1` (not bare `cmake --preset`) when changing presets, toolset version, or after a VS update.

Output: `build-release/Scyclone_artefacts/Release/` (VST3 and Standalone).

Debug, tests, and sanitizers: [test/README.md](../../test/README.md).

## CI

GitHub Actions loads vcvars via [`.github/actions/setup-msvc`](../../.github/actions/setup-msvc/action.yml). The Windows toolchain detects `$LIB`/`$INCLUDE` and skips the generated snapshot — no `configure.ps1` or interactive prompts in CI.
