# Windows build

MSVC only (ONNX + RNBO). Install **MSVC v143 build tools (Latest)** / toolset **14.51** alongside older toolsets — required to link [anira-project/backends](https://github.com/anira-project/backends) ONNX; other projects can stay on 14.44.

```powershell
.\cmake\windows\configure.ps1 -Preset release
.\cmake\windows\build.ps1 -BuildPreset release
```

Scripts pin `cl` (not LLVM on `PATH`) and `vcvars64 -vcvars_ver=14.51`. First run copies [`CMakeUserPresets.json.example`](../../cmake/windows/CMakeUserPresets.json.example) to `CMakeUserPresets.json` at the repo root.

Debug, tests, sanitizers: [test/README.md](../../test/README.md).
