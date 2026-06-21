# Windows build

Scyclone on Windows requires **MSVC** (ONNX Runtime and RNBO). Shared presets in `CMakePresets.json` do not pin a toolset — use the helpers below (or the IDE presets) so configure picks **14.51** from normal PowerShell and can install it if missing. They load `vcvars64 -vcvars_ver=14.51` and pass explicit **`cl`** paths.

**Toolset:** MSVC **14.51** (v143, Visual Studio 2026). Needed to link [anira-project/backends](https://github.com/anira-project/backends) ONNX. If it is missing, `configure.ps1` can offer to install **Build Tools for Visual Studio 2026**.

```powershell
.\cmake\windows\configure.ps1 -Preset release
.\cmake\windows\build.ps1 -BuildPreset release
```

After the first `configure.ps1`, builds can use presets directly (`cmake --build --preset release`) from a shell with the same MSVC env; `build.ps1` loads that for you. Re-configure with `configure.ps1` (not bare `cmake --preset`) to keep toolset **14.51** pinned.

Output: `build-release/Scyclone_artefacts/Release/` (VST3 and Standalone).

**IDE:** copy [`CMakeUserPresets.json.example`](../../cmake/windows/CMakeUserPresets.json.example) to `CMakeUserPresets.json` at the repo root; use presets `windows-release` or `windows-default`.

Debug, tests, and sanitizers: [test/README.md](../../test/README.md).
