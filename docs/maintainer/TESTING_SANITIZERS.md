# Sanitizer CI — maintainers

Policy and wiring for sanitizer builds. **Source of truth:** [`cmake/ScycloneSanitizers.cmake`](../../cmake/ScycloneSanitizers.cmake) (presets, flags, FATAL_ERROR rules), [`.github/workflows/sanitizers.yml`](../../.github/workflows/sanitizers.yml) (jobs, env vars, probes).

Local configure/build: [test/README.md](../../test/README.md) and [`CMakePresets.json`](../../CMakePresets.json). Only **`Test`** is built/run — not plugin formats.

## CI jobs

| Job | Preset | Gate |
|-----|--------|------|
| `sanitize-asan-ubsan-linux` | `ASAN_UBSAN` | **required** |
| `sanitize-asan-ubsan-macos` | `ASAN_UBSAN` | **required** |
| `sanitize-tsan-linux` / `sanitize-tsan-macos` | `THREAD` | **required** |
| `sanitize-asan-msvc-windows` | `ASAN` | `continue-on-error` |
| `sanitize-msan-linux` | `MEMORY` | `continue-on-error` |
| `sanitize-leak-macos` | `LEAK` | `continue-on-error` |

All jobs: `ctest -L default` (see [test/README.md](../../test/README.md) for labels).

## Decisions not obvious from CMake alone

**Linux ASan vs macOS ASan leaks** — workflow sets `ASAN_OPTIONS=detect_leaks=1` on Linux only. Apple Clang aborts with `detect_leaks is not supported on this platform`; macOS leak signal is `sanitize-leak-macos` (Homebrew Clang, `-fsanitize=leak`, warn-only via `LSAN_OPTIONS=exitcode=0`).

**MSan + ONNX stub** — prebuilt ORT is not MSan-instrumented. `MEMORY` forces stub ([`InferenceThreadStub.cpp`](../../source/dsp/onnx/InferenceThreadStub.cpp)); `PluginIntegrationTest` skips under `SCYCLONE_ONNX_STUB`. FetchContent **gtest** links `scyclone_sanitizer_flags` for MSan (see [`setup_tests_and_benchmarks.cmake`](../../cmake/setup_tests_and_benchmarks.cmake)).

**Linux ASAN_UBSAN + PluginIntegrationTest** — prebuilt ORT + Linux UBSan hits invalid-vptr in ORT during model load; `PluginIntegrationTest` skips (`SCYCLONE_SKIP_PLUGIN_INTEGRATION_TEST`). macOS ASan still runs it.

**Windows MSVC ASan** — `ASAN` forces the same ONNX stub (LNK2038/LNK1319 without it); DSP/resampling tests run, `PluginIntegrationTest` skips.

**macOS LEAK job** — pins Homebrew **`llvm@18`** (unpinned `llvm` 22 breaks JUCE 7.0.5); `-fsanitize=leak`, warn-only. `LEAK` forces ONNX stub (prebuilt ORT does not link with Homebrew Clang); DSP/resampling tests run, `PluginIntegrationTest` skips.

**SNR floors** — Linux/macOS ASan jobs run `PrintSnrMeasurements` (`continue-on-error`) to calibrate per-OS floors in [`ResamplingSignalUtils.h`](../../test/scyclone/resampling/ResamplingSignalUtils.h). Procedure: [test/scyclone/calibration/README.md](../../test/scyclone/calibration/README.md).

## When CI breaks

| Symptom | Check |
|---------|--------|
| Linux configure / `juceaide` | [`.github/actions/juce-linux-deps`](../../.github/actions/juce-linux-deps/action.yml) ran |
| macOS ASan aborts before tests | `detect_leaks` must not be set on macOS (see workflow) |
| No sanitizer in stack traces | `Test` links `scyclone_sanitizer_flags`; `-fsanitize=` in `compile_commands.json` |
| Windows tests fail after link | MSVC ASan DLL on `PATH` (CI: `VCToolsInstallDir` — see workflow Setup MSVC step) |
