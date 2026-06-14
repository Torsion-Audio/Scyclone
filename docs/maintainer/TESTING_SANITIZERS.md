# Sanitizer CI — maintainers only

Sanitizer builds use **`RelWithDebInfo`**, **`-DSCYCLONE_SANITIZERS=<PRESET>`**, and the **`Test`** target only (not plugin formats). Presets and policy live in [`cmake/ScycloneSanitizers.cmake`](../../cmake/ScycloneSanitizers.cmake).

Workflow: [`.github/workflows/sanitizers.yml`](../../.github/workflows/sanitizers.yml) — push/PR to `develop`, plus manual dispatch.

## Presets

| Preset | Where | CI job |
|--------|-------|--------|
| `ASAN_UBSAN` | Linux, macOS | `sanitize-asan-ubsan-*` |
| `ASAN` | Windows (MSVC) | `sanitize-asan-msvc-windows` |
| `THREAD` | Linux, macOS | `sanitize-tsan-*` |
| `MEMORY` | Linux + Clang | `sanitize-msan-linux` |
| `LEAK` | macOS + Homebrew Clang | `sanitize-leak-macos` |

Default: `NONE`.

## CI gates

| Status | Jobs |
|--------|------|
| **Required** | ASAN_UBSAN (Linux/macOS), TSan (Linux/macOS) |
| **`continue-on-error: true`** | MSVC ASan (Windows), MSan, LeakSan |

All jobs run `ctest -L default` (excludes the `extended-matrix` suite).

## Local run

Prefer CMake presets from [`CMakePresets.json`](../../CMakePresets.json) (see [test/README.md](../../test/README.md)):

```bash
# Linux/macOS — ASan + UBSan
cmake --preset asan-ubsan
cmake --build --preset asan-ubsan
export ASAN_OPTIONS=detect_leaks=1:abort_on_error=1
export UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1
ctest --test-dir build-asan-ubsan -L default --output-on-failure

# Linux/macOS — ThreadSanitizer (preset sets CC/CXX to clang; override in CMakeUserPresets.json if needed)
cmake --preset tsan
cmake --build --preset tsan
ctest --test-dir build-tsan -L default --output-on-failure

# Windows MSVC — ASan only (UBSan not supported)
cmake --preset asan
cmake --build --preset asan
ctest --test-dir build-asan -L default --output-on-failure
```

Equivalent raw configure (no presets):

```bash
cmake -G Ninja -B build-asan-ubsan -DCMAKE_BUILD_TYPE=RelWithDebInfo -DSCYCLONE_SANITIZERS=ASAN_UBSAN
cmake --build build-asan-ubsan --target Test
```

TSan (raw configure): `-DSCYCLONE_SANITIZERS=THREAD` with `CC=clang CXX=clang++`.  
Windows MSVC ASan: `-DSCYCLONE_SANITIZERS=ASAN` (not `ASAN_UBSAN`).

## Known limits

- **Prebuilt ONNX** is not instrumented. Fine for ASan/UBSan/TSan; MSan may false-positive in `PluginIntegrationTest`.
- **Windows MSVC ASan** fails link with **LNK2038** (`annotate_string` mismatch vs `onnxruntime-win-x64.lib`). CI job is informational until an ASan-built ORT exists.
- **JUCE LTO** is disabled when any sanitizer preset is active.
- **libsamplerate** gets matching ASan/UBSan flags when preset is `ASAN` or `ASAN_UBSAN`.

## If something breaks

- No sanitizer output → check `compile_commands.json` for `-fsanitize=` / `/fsanitize=address`; confirm `Test` links `scyclone_sanitizer_flags`.
- Bad stack traces → install `llvm-symbolizer`; set `ASAN_OPTIONS=symbolize=1`.
- Windows `0xc0000135` after a successful link → MSVC ASan DLL must be on `PATH` (CI derives it from `VCToolsInstallDir` in the Setup MSVC step).
